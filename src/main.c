#include "main.h"

#include <malloc.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libdragon.h>

#include "ff.h"
#include "ini.h"

#include "chksum64.h"
#include "cic.h"
#include "constants.h"
#include "debug.h"
#include "everdrive.h"
#include "gfx.h"
#include "image.h"
#include "mem.h"
#include "memorypak.h"
#include "menu.h"
#include "mp3.h"
#include "rom.h"
#include "sd.h"
#include "strlib.h"
#include "sys.h"
#include "types.h"
#include "utils.h"

static char *romloaded;
static char *loading;
static char *plgmp3;
static char *OpComsucc;
static char *updatelastgamerecord;
static char *cpyingram2SD;
static char *emunofound;
static char *errornoready;
static char *filenoexist;
static char *pathnoexist;
static char *invalidname;
static char *operdenied;
static char *filealrrexist;
static char *errtimeout;
static char *devicelocked;
static char *done3p;
static char *readystr;
static char *filopenerror;
static char *nosavfund;
#include "cheats_table.h"
#include "ini_write.h"
#include "menu_ed64.h"
#include "patch.h"
#include "ui_cells.h"

static int saveTypeToSd(display_context_t disp, char *rom_name, int stype);
static void drawShortInfoBox(display_context_t disp, char *text, u8 mode);
static void bootRom(display_context_t disp, int silent);

static char pending_patch[MAX_SUPPORTED_PATH_LEN] = "";
u8 cart_speed = 0; // 0 = header timings, 1 = Fast (PWD 0x0A), 2 = Fastest (PWD 0x08, LAT 0x20): INI cart_speed
u8 background_on = 1, crc_check = 0;
u8 fast_reboot = 0, game_id_on = 1;
u32 boot_counter = 0, game_counter = 0;

static void ed64_close_menu(display_context_t disp);
static void ed64_do_action(display_context_t disp, int act);
static void ed64_open_file_menu_for_cursor(display_context_t disp);
static void romInfoScreen(char *buff);
static void loadFile(display_context_t disp);
static void load_wallpaper(void);
static void readRomConfig(display_context_t disp, char *short_filename, char *full_filename);

#define MAX_LIST 20

typedef struct
{
    char *background_image;
    char *border_color_1;
    char *border_color_2;
    char *box_color;
    char *selection_color;
    char *list_font_color;
    char *list_dir_font_color;
    char *selection_font_color;
    char *mempak_path;
    char *save_path;
    int page_display;
    int tv_mode;
    int quick_boot;
    int ext_type;
    int cd_behaviour;
    int scroll_behaviour;
    int text_offset;
    int hide_sysfolder;
    int sd_speed;
    int save_backup;
} configuration;

static char rom_config[10];
static u32 emu_offset;

static int select_mode = 0;

static int boot_cic = 0;
static int boot_save = 0;

static int cursor_line = 0;
static u16 cursor_history[32];
static u16 cursor_history_pos = 0;

static u8 empty = 0;
static int mp3playing = 0;

static FATFS *fs;

enum InputMap
{
    file_manager,
    rom_loaded,
    mp3,
    ed64_menu,
};
static enum InputMap input_mapping = file_manager;

static int save_after_reboot = 0;

static char curr_dirname[64];
char pwd[64];
static TCHAR rom_filename[MAX_SUPPORTED_PATH_LEN];

static u8 gbload = 0;

short int gCheats = 0; /* 0 = off, 1 = select, 2 = all */
short int force_tv = 0;

static resolution_t res = RESOLUTION_320x240;

sprite_t *loadPng(char *png_filename);
static sprite_t *background;

static u32 border_color_1 = 0xFFFFFFFF;
static u32 border_color_2 = 0x3F3F3FFF;
static u32 box_color = 0x00000080;
static u32 selection_color = 0x6495ED60;
static u32 selection_font_color = 0xFFFFFFFF;
static u32 list_font_color = 0xA8A8A8FF;
static u32 list_dir_font_color = 0xF0E030FF;

static char *border_color_1_s;
static char *border_color_2_s;
static char *box_color_s;
static char *selection_color_s;
static char *selection_font_color_s;
static char *list_font_color_s;
static char *list_dir_font_color_s;

static char *save_path;

u8 page_display = 0;
u8 tv_mode = 0; // 1 = NTSC, 2 = PAL, 0 = automatic
u8 quick_boot = 0;
static u8 cd_behaviour = 0;     // 0 = first entry, 1 = last entry
static u8 scroll_behaviour = 0; // 1 = classic, 0 = page system
u8 ext_type = 0;                // 0 = classic, 1 = original OS
u8 sd_speed = 1;                // 1 = 25 MHz, 2 = 50 MHz
u8 hide_sysfolder = 1;
u8 save_backup = 1;
static char *background_image;

static int buf_size;
static char *buf_ptr;

int count = 0;
static int page = 0;
int cursor = 0;
direntry_t *list;

static int filesize(FILE *pFile)
{
    fseek(pFile, 0, SEEK_END);
    int lSize = ftell(pFile);
    rewind(pFile);

    return lSize;
}

sprite_t *read_sprite(const char *const spritename)
{
    FILE *fp = fopen(spritename, "r");

    if (fp)
    {
        sprite_t *sp = malloc(filesize(fp));
        if (sp)
            fread(sp, 1, filesize(fp), fp);
        fclose(fp);

        return sp;
    }
    else
    {
        return 0;
    }
}

static int compare(const void *a, const void *b)
{
    direntry_t *first = (direntry_t *) a;
    direntry_t *second = (direntry_t *) b;

    if (first->type == DT_DIR && second->type != DT_DIR)
    {
        return -1;
    }

    if (first->type != DT_DIR && second->type == DT_DIR)
    {
        return 1;
    }

    return strcmp(first->filename, second->filename);
}

static void new_scroll_pos(int *cursor, int *page, int max_arg, int count)
{
    int max = max_arg;
    if (max > count)
    {
        max = count;
    }

    if (*cursor >= count)
    {
        *cursor = count - 1;
    }

    if (*cursor < 0)
    {
        *cursor = 0;
    }

    if (*cursor < *page)
    {
        *page = *cursor;
        return;
    }

    if (*cursor >= (*page + max))
    {
        *page = (*cursor - max) + 1;
        return;
    }
}

static void display_dir(direntry_t *list, int cursor_arg, int page, int max_arg, int count, display_context_t disp)
{
    int cursor = cursor_arg;
    int max = max_arg;
    int pages = (count + MAX_LIST - 1) / MAX_LIST;
    if (pages < 1)
        pages = 1;
    if (page_display)
        ui_page_line(disp, page / MAX_LIST + 1, pages);
    else
    {
        char l[48];
        snprintf(l, sizeof(l), "SD:/%.34s", pwd);
        ui_print(disp, UI_LIST_COL, UI_PAGE_ROW, l, PAL_GRAY, PAL_BLACK);
    }

    if (max > count)
        max = count;
    if (cursor >= (page + max))
        cursor = (page + max) - 1;
    if (cursor < page)
        cursor = page;
    if (max == 0)
    {
        ui_print(disp, UI_LIST_COL, UI_LIST_ROW, "There are no files", PAL_GRAY, PAL_BLACK);
        empty = 1;
        return;
    }
    empty = 0;
    if (page == (count / MAX_LIST) * MAX_LIST)
        max = count % MAX_LIST;

    for (int i = page; i < (page + max); i++)
    {
        int pal = (i == cursor) ? PAL_WHITE : (list[i].type == DT_DIR) ? PAL_YELLOW : PAL_GRAY;
        ui_print_n(disp, UI_LIST_COL + text_offset, UI_LIST_ROW + (i - page), list[i].filename, UI_LIST_COLS, pal,
                   PAL_BLACK);
    }
    ui_bottom_name(disp, list[cursor].filename);
    graphics_set_color(graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF), graphics_make_color(0, 0, 0, 0));
}

static sprite_t *halve_sprite(sprite_t *src)
{
    int w = src->width / 2, h = src->height / 2;
    sprite_t *dst = malloc(sizeof(sprite_t) + w * h * 4);
    if (!dst)
        return NULL;
    *dst = *src;
    dst->width = w;
    dst->height = h;
    dst->hslices = w / 32;
    dst->vslices = h / 16;
    const u8 *sp = (const u8 *) src->data;
    u8 *dp = (u8 *) dst->data;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            const u8 *p0 = sp + ((2 * y) * src->width + 2 * x) * 4, *p1 = p0 + src->width * 4;
            u8 *q = dp + (y * w + x) * 4;
            for (int c = 0; c < 4; c++)
                q[c] = (p0[c] + p0[4 + c] + p1[c] + p1[4 + c] + 2) >> 2;
        }
    return dst;
}

static void load_wallpaper(void)
{
    char path[MAX_SUPPORTED_PATH_LEN];
    FIL wf;
    FILINFO fno;
    int have = 0;
    if (f_open(&wf, "/" ED64_FIRMWARE_PATH "/SYS/WALLPAPER.CFG", FA_READ) == FR_OK)
    {
        if (f_gets(path, sizeof(path), &wf))
        {
            char *nl = strpbrk(path, "\r\n");
            if (nl)
                *nl = 0;
            have = path[0] != 0;
        }
        f_close(&wf);
    }
    if (!have)
    {
        if (background_image && background_image[0])
        {
            snprintf(path, sizeof(path), "/" ED64_FIRMWARE_PATH "/wallpaper/%s", background_image);
            have = (f_stat(path, &fno) == FR_OK && !(fno.fattrib & AM_DIR));
        }
    }
    sprite_t *bg = have ? loadPng((char *) path) : NULL;
    if (!bg)
    {
        background = read_sprite("rom://sprites/background.sprite");
        return;
    }
    if (bg->width == 640 && bg->height == 480)
    {
        sprite_t *small = halve_sprite(bg);
        free(bg);
        background = small ? small : read_sprite("rom://sprites/background.sprite");
        return;
    }
    background = bg;
}

static void drawBg(display_context_t disp)
{
    if (!background_on || !background)
    {
        ui_black(disp);
        return;
    }
    graphics_draw_sprite(disp, 0, 0, background);
}

static void drawBox(short x, short y, short width, short height, display_context_t disp)
{
    x = x + (text_offset * 8);

    graphics_draw_line(disp, x, y, width + x, y, border_color_1);
    graphics_draw_line(disp, width + x, y, width + x, height + y, border_color_1);
    graphics_draw_line(disp, x, y, x, height + y, border_color_2);
    graphics_draw_line(disp, x, height + y, width + x, height + y, border_color_2);
    graphics_draw_box_trans(disp, x + 1, y + 1, width - 1, height - 1, box_color);
}

static void drawBoxNumber(display_context_t disp, int box)
{
    int old_color = box_color;

    switch (box)
    {
        case 1:
            ui_frame(disp);
            break;
        case 7:
            box_color = graphics_make_color(0x00, 0x00, 0x60, 0xC3);
            drawBox(60, 105, 197, 20, disp);
            break;
        case 8:
            box_color = graphics_make_color(0x60, 0x00, 0x00, 0xD3);
            drawBox(60, 105, 197, 20, disp);
            break;
        case 10:
            box_color = graphics_make_color(0x00, 0x60, 0x00, 0xC3);
            drawBox(60, 105, 197, 20, disp);
            break;
        default:
            break;
    }

    box_color = old_color;
}

static int configHandler(void *user, const char *section, const char *name, const char *value)
{
    configuration *pconfig = (configuration *) user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("ed64", "border_color_1"))
    {
        pconfig->border_color_1 = strdup(value);
    }
    else if (MATCH("ed64", "border_color_2"))
    {
        pconfig->border_color_2 = strdup(value);
    }
    else if (MATCH("ed64", "box_color"))
    {
        pconfig->box_color = strdup(value);
    }
    else if (MATCH("ed64", "selection_color"))
    {
        pconfig->selection_color = strdup(value);
    }
    else if (MATCH("ed64", "list_font_color"))
    {
        pconfig->list_font_color = strdup(value);
    }
    else if (MATCH("ed64", "list_dir_font_color"))
    {
        pconfig->list_dir_font_color = strdup(value);
    }
    else if (MATCH("ed64", "selection_font_color"))
    {
        pconfig->selection_font_color = strdup(value);
    }
    else if (MATCH("ed64", "mempak_path"))
    {
        pconfig->mempak_path = strdup(value);
    }
    else if (MATCH("ed64", "save_path"))
    {
        pconfig->save_path = strdup(value);
    }
    else if (MATCH("ed64", "page_display"))
    {
        pconfig->page_display = atoi(value);
    }
    else if (MATCH("ed64", "tv_mode"))
    {
        pconfig->tv_mode = atoi(value);
    }
    else if (MATCH("ed64", "quick_boot"))
    {
        pconfig->quick_boot = atoi(value);
    }
    else if (MATCH("ed64", "ext_type"))
    {
        pconfig->ext_type = atoi(value);
    }
    else if (MATCH("ed64", "cd_behaviour"))
    {
        pconfig->cd_behaviour = atoi(value);
    }
    else if (MATCH("ed64", "scroll_behaviour"))
    {
        pconfig->scroll_behaviour = atoi(value);
    }
    else if (MATCH("ed64", "text_offset"))
    {
        pconfig->text_offset = atoi(value);
    }
    else if (MATCH("ed64", "sd_speed"))
    {
        pconfig->sd_speed = atoi(value);
    }
    else if (MATCH("ed64", "hide_sysfolder"))
    {
        pconfig->hide_sysfolder = atoi(value);
    }
    else if (MATCH("ed64", "background_image"))
    {
        pconfig->background_image = strdup(value);
    }
    else if (MATCH("ed64", "save_backup"))
    {
        pconfig->save_backup = atoi(value);
    }
    else
    {
        return 0;
    }

    return 1;
}

static void configure()
{
    u16 msg = 0;
    u8 buff[16];
    u16 firm;

    evd_init();
    evd_setCfgBit(ED_CFG_SDRAM_ON, 0);
    dma_read_s(buff, ROM_ADDR + 0x20, 16);
    asm_date = memRomRead32(0x38);
    evd_setCfgBit(ED_CFG_SDRAM_ON, 1);

    firm = evd_getFirmVersion();

    if (firm >= 0x0200)
    {
        sleep(1);
        evd_setCfgBit(ED_CFG_SDRAM_ON, 0);
        sleep(1);

        msg = evd_readReg(REG_MAX_MSG);

        if (!(msg & (1 << 14)))
        {
            msg |= 1 << 14;
            evd_writeReg(REG_MAX_MSG, msg);

            sleep(1);
            evd_init();
        }
        sleep(1);

        evd_setCfgBit(ED_CFG_SDRAM_ON, 1);
    }

    if ((streql("ED64+ SD boot", (char *) buff, 13) || streql("ED64 SD boot", (char *) buff, 12)) && firm >= 0x0116)
    {
        sdSetInterface(DISK_IFACE_SD);
    }
    else
    {
        sdSetInterface(DISK_IFACE_SPI);
    }
    memSpiSetDma(0);
}

void clearScreen(display_context_t disp)
{
    drawBg(disp);
    drawBoxNumber(disp, 1);
}

static void romInfoScreen(char *buff)
{
    TCHAR filename[MAX_SUPPORTED_PATH_LEN];
    sprintf(filename, "%s", buff);

    FRESULT result;

    int fsize = 4096;
    unsigned char headerdata[fsize];

    FIL file;
    UINT bytesread;
    result = f_open(&file, filename, FA_READ);

    if (result == FR_OK)
    {
        result = f_read(&file, headerdata, fsize, &bytesread);

        f_close(&file);

        int sw_type = is_valid_rom(headerdata);

        if (sw_type != 0)
        {
            swap_header(headerdata, 4096);
        }

        int cic, save;

        cic = get_cic(headerdata);
        get_cic_save(&headerdata[0x3B], &cic, &save);

        rom_config[1] = cic - 1;
        rom_config[2] = save;
        rom_config[3] = 0;
        rom_config[4] = 0;
        rom_config[5] = 0;
        rom_config[6] = 0;
        rom_config[7] = 0;
        rom_config[8] = 0;
        rom_config[9] = 0;
    }
}

sprite_t *loadPng(char *png_filename)
{
    TCHAR *filename = (TCHAR *) malloc(slen(png_filename) + 1);
    sprintf(filename, "%s", png_filename);

    FRESULT result;
    FIL file;
    UINT bytesread;
    result = f_open(&file, filename, FA_READ);

    if (result == FR_OK)
    {
        int fsize = f_size(&file);
        u8 png_rawdata[fsize];

        result = f_read(&file, png_rawdata, fsize, &bytesread);

        f_close(&file);

        free(filename);
        return loadImage32(png_rawdata, fsize);
    }

    return 0;
}

static void loadgenericrom(display_context_t disp_arg, TCHAR *rom_path, char *extension)
{
    display_context_t disp = disp_arg;
    disp = gfx_lock_display();
    ui_black(disp);
    ui_print_center(disp, 13, "Loading...", PAL_GRAY, PAL_BLACK);
    display_show(disp);
    printText(" ", 3, 16, disp);
    FRESULT result;
    FIL emufile;
    UINT emubytesread;
    TCHAR tmp[MAX_SUPPORTED_PATH_LEN];
    snprintf(tmp, sizeof(tmp), "/" ED64_FIRMWARE_PATH "/EMU/%s.v64", extension);
    result = f_open(&emufile, tmp, FA_READ);

    if (result == FR_OK)
    {
        int emufsize = f_size(&emufile);
        result = f_read(&emufile, (void *) 0xb0000000, emufsize, &emubytesread);

        f_close(&emufile);

        FIL romfile;
        UINT rombytesread;
        result = f_open(&romfile, rom_path, FA_READ);

        if (result == FR_OK)
        {
            int romfsize = f_size(&romfile);

            result = f_read(&romfile, (void *) emu_offset, romfsize, &rombytesread);

            f_close(&romfile);

            boot_cic = rom_config[1] + 1;
            boot_save = rom_config[2];
            force_tv = (rom_config[3] <= 2) ? rom_config[3] : 0;

            bootRom(disp, 1);
        }
    }
    else
    {
        drawShortInfoBox(disp, emunofound, 1);
    }
}

static void loadrom(display_context_t disp, char *buff)
{
    if (sd_speed == 2)
        bi_speed50();
    else
        bi_speed25();
    {
        char cv[16];
        game_counter++;
        snprintf(cv, sizeof(cv), "%lu", (unsigned long) game_counter);
        ini_set_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "game_count", cv);
    }

    ui_black(disp);
    ui_print_center(disp, 13, "Loading...", PAL_GRAY, PAL_BLACK);
    display_show(disp);
    printText(" ", 3, 16, disp);

    TCHAR filename[MAX_SUPPORTED_PATH_LEN];
    sprintf(filename, "%s", buff);

    FRESULT result;
    FIL file;
    UINT bytesread = 0;
    result = f_open(&file, filename, FA_READ);

    if (result == FR_OK)
    {
        int swapped = 0;
        int headerfsize = 4096;
        unsigned char headerdata[headerfsize];
        int fsize = f_size(&file);
        int fsizeMB = fsize / 1048576;

        result = f_read(&file, headerdata, headerfsize, &bytesread);

        f_close(&file);

        int sw_type = is_valid_rom(headerdata);

        if (sw_type != 0)
        {
            swapped = 1;
            swap_header(headerdata, 4096);
        }
        if (game_id_on)
            send_game_id(&headerdata[0x10], &headerdata[0x14], headerdata[0x3B], headerdata[0x3E]);

        int cic, save;

        cic = get_cic(headerdata);

        get_cic_save(&headerdata[0x3B], &cic, &save);

        boot_cic = rom_config[1] + 1;
        boot_save = rom_config[2];
        force_tv = (rom_config[3] <= 2) ? rom_config[3] : 0;

        if (gbload == 1)
            boot_save = 1;

        if (swapped == 1)
        {
            while (evd_isDmaBusy())
                ;
            evd_mmcSetDmaSwap(1);

            TRACE(disp, "swapping on");
        }

        bytesread = 0;
        result = f_open(&file, filename, FA_READ);
        if (fsizeMB <= 32)
        {
            result = f_read(&file, (void *) 0xb0000000, fsize, &bytesread);
        }
        else
        {
            result = f_read(&file, (void *) 0xb0000000, 32 * 1048576, &bytesread);
            if (result == FR_OK)
            {
                result = f_read(&file, (void *) 0xb2000000, fsize - bytesread, &bytesread);
            }
        }

        if (result == FR_OK)
        {
            char patch_path[MAX_SUPPORTED_PATH_LEN] = "";
            if (pending_patch[0])
            {
                strncpy(patch_path, pending_patch, sizeof(patch_path) - 1);
                pending_patch[0] = 0;
            }
            else if (rom_config[8])
            {
                char base[MAX_SUPPORTED_PATH_LEN];
                strncpy(base, (char *) buff, sizeof(base) - 1);
                base[sizeof(base) - 1] = 0;
                char *dot = strrchr(base, '.');
                if (dot && dot > strrchr(base, '/'))
                    *dot = 0;
                FILINFO pfno;
                snprintf(patch_path, sizeof(patch_path), "%s.ips", base);
                if (f_stat(patch_path, &pfno) != FR_OK)
                {
                    snprintf(patch_path, sizeof(patch_path), "%s.aps", base);
                    if (f_stat(patch_path, &pfno) != FR_OK)
                        patch_path[0] = 0;
                }
            }
            if (patch_path[0])
            {
                printText("Patching...", 3, -1, disp);
                display_show(disp);
                int prc = patch_apply(patch_path, fsize, NULL);
                printText((char *) patch_error_text(prc), 3, -1, disp);
                display_show(disp);
                if (prc != PATCH_OK)
                    sleep(1500);
            }
            if (cart_speed)
            {
                static u8 hdr[512] __attribute__((aligned(16)));
                while (evd_isDmaBusy())
                    ;
                dma_read_s(hdr, ROM_ADDR, 512);
                hdr[2] = (cart_speed >= 2) ? 0x08 : 0x0A;
                hdr[3] = (cart_speed >= 2) ? 0x20 : 0x40;
                dma_write_s(hdr, ROM_ADDR, 512);
                printText((cart_speed >= 2) ? "Cart speed: fastest" : "Cart speed: fast", 3, -1, disp);
            }
            printText(romloaded, 3, -1, disp);

            bootRom(disp, 1);
        }
        else
        {
            printText(filopenerror, 3, -1, disp);
        }
    }
}

static int backupSaveData(display_context_t disp)
{
    TCHAR config_file_path[MAX_SUPPORTED_PATH_LEN];
    sprintf(config_file_path, "/" ED64_FIRMWARE_PATH "/%s/LASTROM.CFG", save_path);

    u8 save_format;
    u8 cfg_data[2];

    FRESULT result;
    FIL file;
    UINT bytesread;
    result = f_open(&file, config_file_path, FA_READ);

    if (result == FR_OK)
    {
        printText(updatelastgamerecord, 3, 4, disp);

        result = f_read(&file, &cfg_data, 2, &bytesread);

        save_format = cfg_data[0];

        f_gets(rom_filename, 256, &file);
        f_close(&file);

        if (save_format != 0)
        {
            result = f_open(&file, config_file_path, FA_WRITE | FA_OPEN_EXISTING);
            cfg_data[0] = 0;

            UINT bw;
            result = f_write(&file, &cfg_data, 1, &bw);

            f_close(&file);

            TRACE(disp, "Disabling save for subsequent system reboots");

            volatile u8 save_config_state = 0;
            save_config_state = evd_readReg(REG_SAV_CFG);

            if (save_config_state != 0 || evd_getFirmVersion() >= 0x0300)
            {
                if (save_config_state == 0)
                {
                    evd_writeReg(REG_SAV_CFG, 1);
                }
                save_after_reboot = 1;
            }
            else
            {
                TRACE(disp, "Save not required.");
                printText(readystr, 3, -1, disp);
                display_show(disp);
                return 1;
            }
        }
    }
    else
    {
        TRACE(disp, "No previous ROM loaded!");
        printText(readystr, 3, -1, disp);
        display_show(disp);
        return 0;
    }

    if (save_after_reboot)
    {
        printText(cpyingram2SD, 3, -1, disp);
        if (saveTypeToSd(disp, rom_filename, save_format))
        {
            printText(OpComsucc, 3, -1, disp);
        }
        else
        {
            TRACE(disp, "ERROR: the RAM was not successfully saved!");
        }
    }
    else
    {
        TRACE(disp, "No reset: save requested");
        printText(done3p, 3, -1, disp);
    }
    display_show(disp);
    return 1;
}

static int saveTypeFromSd(display_context_t disp, char *rom_name, int stype)
{
    TRACE(disp, rom_filename);

    const char *save_type_extension = saveTypeToExtension(stype, ext_type);
    TCHAR fname[256] = {0};
    sprintf(fname, "/" ED64_FIRMWARE_PATH "/%s/%s.%s", save_path, rom_name, save_type_extension);

    TCHAR fname1[50] = {0};
    sprintf(fname1, "/" ED64_FIRMWARE_PATH "/%s/", save_path);
    printText(fname1, 3, -1, disp);
    TCHAR fname2[50] = {0};
    sprintf(fname2, "%s.%s", rom_name, save_type_extension);
    printText(fname2, 3, -1, disp);

    int size = saveTypeToSize(stype);
    uint8_t cartsave_data[size];

    FIL file;
    FRESULT result;
    UINT bytesread;
    result = f_open(&file, fname, FA_READ);

    if (result == FR_OK)
    {
        result = f_read(&file, cartsave_data, size, &bytesread);

        f_close(&file);
    }
    else
    {
        switch (result)
        {
            case FR_NOT_READY:
                printText(errornoready, 3, -1, disp);
                break;
            case FR_NO_FILE:
                printText(filenoexist, 3, -1, disp);
                break;
            case FR_NO_PATH:
                printText(pathnoexist, 3, -1, disp);
                break;
            case FR_INVALID_NAME:
                printText(invalidname, 3, -1, disp);
                break;
            case FR_DENIED:
                printText(operdenied, 3, -1, disp);
                break;
            case FR_EXIST:
                printText(filealrrexist, 3, -1, disp);
                break;
            case FR_TIMEOUT:
                printText(errtimeout, 3, -1, disp);
                break;
            case FR_LOCKED:
                printText(devicelocked, 3, -1, disp);
                break;
            default:
                break;
        }
        printText(nosavfund, 3, -1, disp);

        memset(cartsave_data, 0xFF, size);
        if (pushSaveToCart(stype, cartsave_data))
            printText("Blank save written to cart", 3, -1, disp);
        else
            printText("Error blanking save area", 3, -1, disp);

        return 0;
    }

    if (pushSaveToCart(stype, cartsave_data))
    {
        printText("Transferred save data...", 3, -1, disp);
    }
    else
    {
        printText("Error transferring save data", 3, -1, disp);
    }

    return 1;
}

static int saveTypeToSd(display_context_t disp, char *rom_name, int stype)
{
    const char *save_type_extension = saveTypeToExtension(stype, ext_type);
    TCHAR fname[MAX_SUPPORTED_PATH_LEN];

    if (save_backup == 1)
    {
        int save_count = 0;
        int i;
        char currsave[256];
        char nextsave[256];
        FRESULT result;
        FILINFO fnoba;
        printText("Finding unused save slot...", 3, -1, disp);
        display_show(disp);
        sprintf(fname, "/" ED64_FIRMWARE_PATH "/%s/%s.%s", save_path, rom_name, save_type_extension);
        result = f_stat(fname, &fnoba);
        if (result != FR_OK)
        {
            printText("No saves found. Creating save...", 3, -1, disp);
            display_show(disp);
            sprintf(fname, "/" ED64_FIRMWARE_PATH "/%s/%s.%s", save_path, rom_name, save_type_extension);
            save_count = 1;
        }
        if (save_count == 0)
        {
            for (i = 2; i < 5; i++)
            {
                sprintf(fname, "/" ED64_FIRMWARE_PATH "/%s/%s.%04x.%s", save_path, rom_name, i, save_type_extension);
                result = f_stat(fname, &fnoba);
                if (result != FR_OK)
                {
                    printText("Saves found. Backing up...", 3, -1, disp);
                    display_show(disp);
                    for (int j = i; j > 1; j--)
                    {
                        sprintf(currsave, "/" ED64_FIRMWARE_PATH "/%s/%s.%04x.%s", save_path, rom_name, j,
                                save_type_extension);
                        sprintf(nextsave, "/" ED64_FIRMWARE_PATH "/%s/%s.%04x.%s", save_path, rom_name, j + 1,
                                save_type_extension);
                        f_rename(currsave, nextsave);
                    }
                    save_count = 2;
                    break;
                }
            }
        }
        if (save_count == 0)
        {
            printText("4 saves found. Deleting save #4...", 3, -1, disp);
            display_show(disp);
            char latestsave[256];
            sprintf(latestsave, "/" ED64_FIRMWARE_PATH "/%s/%s.%04x.%s", save_path, rom_name, 4, save_type_extension);
            f_unlink(latestsave);
            printText("Renaming remaining saves...", 3, -1, disp);
            display_show(disp);
            for (i = 2; i < 4; i++)
            {
                sprintf(currsave, "/" ED64_FIRMWARE_PATH "/%s/%s.%04x.%s", save_path, rom_name, i, save_type_extension);
                sprintf(nextsave, "/" ED64_FIRMWARE_PATH "/%s/%s.%04x.%s", save_path, rom_name, i + 1,
                        save_type_extension);
                f_rename(currsave, nextsave);
            }
            save_count = 3;
        }

        if (save_count != 1)
        {
            sprintf(currsave, "/" ED64_FIRMWARE_PATH "/%s/%s.%s", save_path, rom_name, save_type_extension);
            sprintf(nextsave, "/" ED64_FIRMWARE_PATH "/%s/%s.%04x.%s", save_path, rom_name, 2, save_type_extension);
            f_rename(currsave, nextsave);
        }
    }
    sprintf(fname, "/" ED64_FIRMWARE_PATH "/%s/%s.%s", save_path, rom_name, save_type_extension);

    int size = saveTypeToSize(stype);
    TRACEF(disp, "size for save=%i", size);

    FRESULT result;
    FIL file;
    result = f_open(&file, fname, FA_WRITE | FA_OPEN_ALWAYS);

    if (result == FR_OK)
    {
        uint8_t cartsave_data[size];

        TRACEF(disp, "cartsave_data=%p", &cartsave_data);

        printText("Transferring save data...", 3, -1, disp);
        if (getSaveFromCart(stype, cartsave_data))
        {
            UINT bw;
            result = f_write(&file, cartsave_data, size, &bw);
            f_close(&file);

            if (bw == 0)
            {
                printText("File was blank.", 3, -1, disp);
                f_unlink(fname);
            }
            else
            {
                printText("RAM area copied to SD card.", 3, -1, disp);
            }
            return 1;
        }
        else
        {
            f_close(&file);
            f_unlink(fname);
            printText("Could not retrieve save from SRAM.", 3, -1, disp);
            return 0;
        }
    }
    else
    {
        TRACE(disp, "Couldn't create file");
        printText("Error saving game to SD: could not create file.", 3, -1, disp);
    }
    return 0;
}

static int readConfigFile(void)
{
    TCHAR filename[MAX_SUPPORTED_PATH_LEN];
    sprintf(filename, "/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI");

    FRESULT result;
    FIL file;
    UINT bytesread;
    result = f_open(&file, filename, FA_READ);

    if (result == FR_OK)
    {
        int fsize = f_size(&file);

        char config_rawdata[fsize + 1];

        result = f_read(&file, config_rawdata, fsize, &bytesread);
        config_rawdata[bytesread] = '\0';

        f_close(&file);

        configuration config;
        config.border_color_1 = border_color_1_s;
        config.border_color_2 = border_color_2_s;
        config.box_color = box_color_s;
        config.selection_color = selection_color_s;
        config.selection_font_color = selection_font_color_s;
        config.list_font_color = list_font_color_s;
        config.list_dir_font_color = list_dir_font_color_s;
        config.mempak_path = mempak_path;
        config.page_display = page_display;
        config.tv_mode = tv_mode;
        config.quick_boot = quick_boot;
        config.ext_type = ext_type;
        config.cd_behaviour = cd_behaviour;
        config.scroll_behaviour = scroll_behaviour;
        config.text_offset = text_offset;
        config.hide_sysfolder = hide_sysfolder;
        config.sd_speed = sd_speed;
        config.background_image = background_image;
        config.save_backup = save_backup;

        if (ini_parse_string(config_rawdata, configHandler, &config) < 0)
        {
            return 0;
        }
        else
        {
            border_color_1_s = config.border_color_1;
            border_color_2_s = config.border_color_2;
            box_color_s = config.box_color;
            selection_color_s = config.selection_color;
            selection_font_color_s = config.selection_font_color;
            list_font_color_s = config.list_font_color;
            list_dir_font_color_s = config.list_dir_font_color;

            mempak_path = config.mempak_path;
            save_path = config.save_path ? config.save_path : "SAVES";
            page_display = config.page_display;
            tv_mode = (config.tv_mode <= 2) ? config.tv_mode : 0;
            quick_boot = config.quick_boot;
            ext_type = config.ext_type;
            cd_behaviour = config.cd_behaviour;
            scroll_behaviour = config.scroll_behaviour;
            text_offset = config.text_offset;
            hide_sysfolder = config.hide_sysfolder;
            sd_speed = config.sd_speed;
            background_image = config.background_image;
            save_backup = config.save_backup;

            return 1;
        }
    }
    return 0;
}

static int str2int(char data_arg)
{
    char data = data_arg;
    data -= '0';
    if (data > 9)
        data -= 7;

    return data;
}

static uint32_t translate_color(char *hexstring)
{
    int r = str2int(hexstring[0]) * 16 + str2int(hexstring[1]);
    int g = str2int(hexstring[2]) * 16 + str2int(hexstring[3]);
    int b = str2int(hexstring[4]) * 16 + str2int(hexstring[5]);
    int a = str2int(hexstring[6]) * 16 + str2int(hexstring[7]);

    return graphics_make_color(r, g, b, a);
}

static void initFilesystem(void)
{
    evd_ulockRegs();
    sleep(10);

    fs = malloc(sizeof(FATFS));
    if (f_mount(fs, "", 1) == FR_OK)
    {
        f_mkdir("/" ED64_FIRMWARE_PATH "/SYS");
    }
}

static int showThisFolder(int hide_sysfolder, char *fname)
{
    if (hide_sysfolder && strcmp(fname, "ED64") == 0)
        return 0;
    if (hide_sysfolder && strcmp(fname, "ED64P") == 0)
        return 0;
    if (hide_sysfolder && strcmp(fname, "System Volume Information") == 0)
        return 0;
    if (strcmp(fname, "menu") == 0)
        return 0;
    return 1;
}

static void readSDcard(display_context_t disp, char *directory)
{
    FRESULT res;
    DIR dir;
    static FILINFO fno;
    count = 1;

    clearScreen(disp);

    res = f_opendir(&dir, directory);
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
                break;
            if (showThisFolder(hide_sysfolder, fno.fname))
            {
                if (fno.fattrib & AM_DIR)
                {
                    list[count - 1].type = DT_DIR;
                }
                else
                {
                    list[count - 1].type = DT_REG;
                }
                strcpy(list[count - 1].filename, fno.fname);

                direntry_t *grown = realloc(list, sizeof(direntry_t) * (count + 1));
                if (!grown)
                    break;
                list = grown;
                count++;
            }
        }
        f_closedir(&dir);
        count--;
    }
    else
    {
        printText("Cannot open this folder", 3, -1, disp);
        sleep(3000);
    }

    page = 0;
    cursor = 0;
    select_mode = 1;

    if (count > 0)
    {
        qsort(list, count, sizeof(direntry_t), compare);
    }

    display_dir(list, cursor, page, MAX_LIST, count, disp);
}

static void bootRom(display_context_t disp_arg, int silent)
{
    display_context_t disp = disp_arg;
    if (boot_cic != 0)
    {
        u32 *cheat_lists[2] = {NULL, NULL};
        int table_codes = rom_config[4] ? cheats_build_lists(rom_filename, cheat_lists) : 0;
        if (table_codes > 0 && !gfx_has_expansion_pak())
        {
            printText("Cheats need an Expansion Pak", 3, -1, disp);
            free(cheat_lists[0]);
            cheat_lists[0] = NULL;
            cheat_lists[1] = NULL;
            table_codes = 0;
        }
        if (table_codes > 0)
        {
            gCheats = 1;
            printText("Cheats: codes loaded", 3, -1, disp);
        }
        else
        {
            gCheats = 0;
        }

        if (boot_save != 0)
        {
            TCHAR cfg_file[MAX_SUPPORTED_PATH_LEN];

            sprintf(cfg_file, "/" ED64_FIRMWARE_PATH "/%s/LASTROM.CFG", save_path);

            FRESULT result;
            FIL file;
            result = f_open(&file, cfg_file, FA_WRITE | FA_CREATE_ALWAYS);

            if (result == FR_OK)
            {
                uint8_t cfg_data[2] = {boot_save, boot_cic};

                UINT bw;
                result = f_write(&file, &cfg_data, 2, &bw);

                f_puts(rom_filename, &file);

                f_close(&file);

                evd_setSaveType(boot_save);

                saveTypeFromSd(disp, rom_filename, boot_save);
            }
        }

        TRACE(disp, "Cartridge save type set");
        TRACE(disp, "Information stored for the reboot save...");

        disable_interrupts();
        int bios_cic = getCicType(1);

        if (crc_check)
        {
            checksum_sdram();
        }

        evd_lockRegs();
        sleep(10);

        disp = gfx_lock_display();

        graphics_fill_screen(disp, 0x000000FF);
        display_show(disp);

        f_mount(0, "", 0);
        free(fs);
        simulate_boot(boot_cic, bios_cic, cheat_lists);
    }
}

static void drawShortInfoBox(display_context_t disp_arg, char *text, u8 mode)
{
    display_context_t disp = disp_arg;
    disp = gfx_lock_display();
    new_scroll_pos(&cursor, &page, MAX_LIST, count);
    clearScreen(disp);
    display_dir(list, cursor, page, MAX_LIST, count, disp);

    if (mode == 0)

        drawBoxNumber(disp, 7);
    else if (mode == 1)
        drawBoxNumber(disp, 8);
    else if (mode == 2)
        drawBoxNumber(disp, 10);

    printText(text, 9, 14, disp);

    display_show(disp);

    sleep(300);
}

static void readRomConfig(display_context_t disp, char *short_filename, char *full_filename)
{
    TCHAR cfg_filename[MAX_SUPPORTED_PATH_LEN];
    sprintf(rom_filename, "%s", short_filename);
    char *extension = &strrchr(rom_filename, '.')[1];
    char *ultracheck = &strrchr(rom_filename, '.')[2];
    if (!strcmp(ultracheck, "64"))
    {
        rom_filename[strlen(rom_filename) - 4] = '\0';
        sprintf(cfg_filename, "/" ED64_FIRMWARE_PATH "/CFG/%s.CFG", rom_filename);
    }
    else
    {
        sprintf(cfg_filename, "/" ED64_FIRMWARE_PATH "/CFG/%s.CFG", extension);
    }
    uint8_t rom_cfg_data[512];

    FRESULT result;
    FIL file;
    UINT bytesread;
    result = f_open(&file, cfg_filename, FA_READ);

    if (result == FR_OK)
    {
        result = f_read(&file, rom_cfg_data, 512, &bytesread);

        f_close(&file);

        if (rom_cfg_data[0] == ROM_CFG_AUTO || rom_cfg_data[1] == ROM_CFG_AUTO)
        {
            romInfoScreen(full_filename);
        }

        rom_config[0] = 1;
        if (rom_cfg_data[0] != ROM_CFG_AUTO)
        {
            rom_config[1] = rom_cfg_data[0];
        }
        if (rom_cfg_data[1] != ROM_CFG_AUTO)
        {
            rom_config[2] = rom_cfg_data[1];
        }
        rom_config[3] = rom_cfg_data[2];
        rom_config[4] = rom_cfg_data[3];
        rom_config[5] = rom_cfg_data[4];
        rom_config[6] = rom_cfg_data[5];
        rom_config[7] = rom_cfg_data[6];
        rom_config[8] = rom_cfg_data[7];
        rom_config[9] = rom_cfg_data[8];
    }
    else
    {
        romInfoScreen(full_filename);
    }
}
static void readEmuConfig(display_context_t disp, char *short_filename, char *full_filename)
{ /*
     * This feature is for emulator developers and emulators that do not use the standard EverDrive
     * loading specification. It defines custom ROM load offsets, stored as 4-byte files in the CFG
     * folder. Users should not need it; it is for edge cases.
     */
    TCHAR cfg_filename[MAX_SUPPORTED_PATH_LEN];
    sprintf(rom_filename, "%s", short_filename);
    char *extension = &strrchr(rom_filename, '.')[1];
    sprintf(cfg_filename, "/" ED64_FIRMWARE_PATH "/CFG/%s.OFFSET", extension);
    u32 emu_cfg_data[4];

    FRESULT result;
    FIL file;
    UINT bytesread;
    result = f_open(&file, cfg_filename, FA_READ);

    if (result == FR_OK)
    {
        result = f_read(&file, (void *) emu_cfg_data, 4, &bytesread);

        f_close(&file);

        emu_offset = emu_cfg_data[0];
    }
    else
    {
        emu_offset = 0xb0200000;
    }
}

static void loadFile(display_context_t disp_arg)
{
    display_context_t disp = disp_arg;
    char name_file[256];

    if (strcmp(pwd, "/") == 0)
        sprintf(name_file, "/%s", list[cursor].filename);
    else
        sprintf(name_file, "%s/%s", pwd, list[cursor].filename);

    int ft = 0;
    char _upper_name_file[256];

    strcpy(_upper_name_file, name_file);

    strhicase(_upper_name_file, strlen(_upper_name_file));

    char extension[4];
    char *pch;
    pch = strrchr(_upper_name_file, '.');
    sprintf(extension, "%s", (pch + 1));

    if (!strcmp(extension, "Z64") || !strcmp(extension, "V64") || !strcmp(extension, "N64"))
        ft = 1;
    else if (!strcmp(extension, "MP3"))
        ft = 7;
    else
        ft = 999;

    disp = gfx_lock_display();

    clearScreen(disp);
    evd_ulockRegs();
    sleep(10);
    sprintf(rom_filename, "%s", list[cursor].filename);
    display_show(disp);
    select_mode = 9;
    switch (ft)
    {
        case 1:
            if (quick_boot)
            {
                FRESULT result;
                FIL file;
                result = f_open(&file, "/" ED64_FIRMWARE_PATH "/SYS/LASTROM.CFG", FA_WRITE | FA_CREATE_ALWAYS);
                if (result == FR_OK)
                {
                    f_puts(name_file, &file);

                    f_close(&file);
                    ed64menu_recent_add(name_file);

                    readRomConfig(disp, rom_filename, name_file);

                    loadrom(disp, name_file);
                    display_show(disp);

                    input_mapping = rom_loaded;
                }
                else
                {
                    TRACE(disp, "Couldn't open file");
                }
            }
            else
            {
                readRomConfig(disp, rom_filename, name_file);

                loadrom(disp, name_file);
                display_show(disp);

                input_mapping = rom_loaded;
            }
            break;
        case 7:
        {
            disp = gfx_lock_display();
            clearScreen(disp);
            drawShortInfoBox(disp, loading, 0);
            display_show(disp);
            long long samples;
            int rate = 44100, channels = 2;

            audio_init(44100, 8);
            buf_size = audio_get_buffer_length() * 6;
            buf_ptr = malloc(buf_size);

            mp3_Start(name_file, &samples, &rate, &channels);
            mp3playing = 1;
            select_mode = 9;

            disp = gfx_lock_display();
            clearScreen(disp);
            drawShortInfoBox(disp, plgmp3, 0);
            display_show(disp);
            input_mapping = mp3;
        }
        break;

        case 999:
            readRomConfig(disp, rom_filename, name_file);
            readEmuConfig(disp, rom_filename, name_file);
            loadgenericrom(disp, name_file, extension);
            display_show(disp);
            break;

        default:
            break;
    }
}

void ed64_redraw_browser(display_context_t disp)
{
    new_scroll_pos(&cursor, &page, MAX_LIST, count);
    clearScreen(disp);
    display_dir(list, cursor, page, MAX_LIST, count, disp);
}

static void ed64_cursor_path(char *name_file, size_t n)
{
    if (strcmp(pwd, "/") == 0)
        snprintf(name_file, n, "/%s", list[cursor].filename);
    else
        snprintf(name_file, n, "%s/%s", pwd, list[cursor].filename);
}

static void ed64_extension(const char *name_file, char *ext, size_t n)
{
    char upper[MAX_SUPPORTED_PATH_LEN];
    strncpy(upper, name_file, sizeof(upper) - 1);
    upper[sizeof(upper) - 1] = 0;
    strhicase(upper, strlen(upper));
    char *pch = strrchr(upper, '.');
    snprintf(ext, n, "%s", pch ? pch + 1 : "");
}

static void ed64_close_menu(display_context_t disp_arg)
{
    display_context_t disp = disp_arg;
    disp = gfx_lock_display();
    ed64_redraw_browser(disp);
    display_show(disp);
    input_mapping = file_manager;
}

static void ed64_open_file_menu_for_cursor(display_context_t disp)
{
    char name_file[MAX_SUPPORTED_PATH_LEN];
    char ext[8];
    ed64_cursor_path(name_file, sizeof(name_file));
    ed64_extension(name_file, ext, sizeof(ext));
    int is_rom = (!strcmp(ext, "Z64") || !strcmp(ext, "V64") || !strcmp(ext, "N64"));
    int is_image = (!strcmp(ext, "PNG") || !strcmp(ext, "BMP") || !strcmp(ext, "JPG") || !strcmp(ext, "JPEG"));
    int is_patch = (!strcmp(ext, "IPS") || !strcmp(ext, "APS"));
    int is_text = (!strcmp(ext, "TXT") || !strcmp(ext, "YML") || !strcmp(ext, "INI") || !strcmp(ext, "CFG") ||
                   !strcmp(ext, "LST"));
    int is_save = (!strcmp(ext, "EEP") || !strcmp(ext, "SRA") || !strcmp(ext, "SRM") || !strcmp(ext, "FLA") ||
                   !strcmp(ext, "SAV"));
    int is_mpk = (!strcmp(ext, "MPK") || !strcmp(ext, "N64"));
    if (!strcmp(ext, "N64"))
        is_mpk = 0;
    if (!is_rom && !is_image && !is_text && !is_patch && !is_save && !is_mpk)
    {
        loadFile(disp);
        return;
    }
    ed64menu_open_file_menu(disp, name_file, list[cursor].filename, is_rom,
                            (is_image ? ED64_FILE_IMAGE : 0) | (is_patch ? ED64_FILE_PATCH : 0) |
                                (is_text ? ED64_FILE_TEXT : 0) | (is_save ? ED64_FILE_SAVE : 0) |
                                (is_mpk ? ED64_FILE_MPK : 0));
    input_mapping = ed64_menu;
}

static void ed64_load_path(display_context_t disp_arg, const char *path)
{
    display_context_t disp = disp_arg;
    char full[MAX_SUPPORTED_PATH_LEN];
    strncpy(full, path, sizeof(full) - 1);
    full[sizeof(full) - 1] = 0;
    char *file_name = strrchr(full, '/');
    if (!file_name)
    {
        ed64_close_menu(disp);
        return;
    }
    FIL file;
    if (f_open(&file, full, FA_READ) != FR_OK)
    {
        drawShortInfoBox(disp, "    ROM not found", 1);
        ed64_close_menu(disp);
        return;
    }
    f_close(&file);
    disp = gfx_lock_display();
    clearScreen(disp);
    evd_ulockRegs();
    sleep(10);
    select_mode = 9;
    readRomConfig(disp, &file_name[1], full);
    ed64menu_recent_add(full);
    loadrom(disp, full);
    display_show(disp);
    input_mapping = rom_loaded;
}

static void ed64_do_action(display_context_t disp_arg, int act)
{
    display_context_t disp = disp_arg;
    switch (act)
    {
        case ED64_ACT_NONE:
            break;
        case ED64_ACT_CLOSE:
            ed64_close_menu(disp);
            break;
        case ED64_ACT_START:
            input_mapping = file_manager;
            loadFile(disp);
            break;
        case ED64_ACT_SET_WALLPAPER:
        {
            sprite_t *bg = loadPng((char *) ed64menu_selected_path());
            if (bg)
            {
                FIL wf;
                free(bg);
                if (f_open(&wf, "/" ED64_FIRMWARE_PATH "/SYS/WALLPAPER.CFG", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
                {
                    f_puts(ed64menu_selected_path(), &wf);
                    f_close(&wf);
                }
                load_wallpaper();
                drawShortInfoBox(disp, "     Wallpaper set", 2);
            }
            else
                drawShortInfoBox(disp, " 320x240 or 640x480 PNG/BMP/JPG", 1);
            ed64_close_menu(disp);
            break;
        }
        case ED64_ACT_DELETE:
        {
            FRESULT fr = f_unlink(ed64menu_selected_path());
            disp = gfx_lock_display();
            clearScreen(disp);
            readSDcard(disp, pwd);
            display_show(disp);
            if (fr == FR_OK)
                ed64menu_message("Operation complete", NULL);
            else
                ed64menu_message("Disk I/O error", NULL);
            break;
        }
        case ED64_ACT_REFRESH:
            disp = gfx_lock_display();
            clearScreen(disp);
            readSDcard(disp, pwd);
            display_show(disp);
            input_mapping = file_manager;
            break;
        case ED64_ACT_CPAK_SAVE_NUM:
        case ED64_ACT_CPAK_SAVE_GAME:
        {
            char base[64];
            if (act == ED64_ACT_CPAK_SAVE_NUM)
                snprintf(base, sizeof(base), "controller-%d", cpak_port + 1);
            else
            {
                FIL f;
                char last[MAX_SUPPORTED_PATH_LEN] = "";
                if (f_open(&f, "/" ED64_FIRMWARE_PATH "/SYS/LASTROM.CFG", FA_READ) == FR_OK)
                {
                    f_gets(last, sizeof(last), &f);
                    f_close(&f);
                }
                char *nl = strpbrk(last, "\r\n");
                if (nl)
                    *nl = 0;
                char *nm = strrchr(last, '/');
                nm = nm ? nm + 1 : last;
                if (!*nm)
                {
                    ed64menu_message("No game selected", NULL);
                    break;
                }
                snprintf(base, sizeof(base), "%.59s", nm);
                char *dot = strrchr(base, '.');
                if (dot)
                    *dot = 0;
            }
            if (ed64_detect_pak(cpak_port) != ACCESSORY_MEMPAK)
            {
                ed64menu_message("Controller Pak not detected", NULL);
                break;
            }
            {
                char where[64];
                snprintf(where, sizeof(where), "%.40s.mpk", base);
                int r = mpk_dump(cpak_port, base);
                if (r == 0)
                    ed64menu_message("CPak saved to", where);
                else if (r == 1)
                    ed64menu_message("CPak saved to (read errors)", where);
                else
                    ed64menu_message("Disk I/O error", NULL);
            }
            break;
        }
        case ED64_ACT_CPAK_FORMAT:
            if (ed64_detect_pak(cpak_port) != ACCESSORY_MEMPAK)
                ed64menu_message("Controller Pak not detected", NULL);
            else if (format_mempak(cpak_port) == 0)
                ed64menu_message("Operation complete", NULL);
            else
                ed64menu_message("Format failed", NULL);
            break;
        case ED64_ACT_SYS_FOLDER:
            if (strcmp(pwd, "/" ED64_FIRMWARE_PATH) != 0)
            {
                sprintf(pwd, "/");
                readSDcard(disp, "/");
                cursor = 0;
                page = 0;
                cursor_line = 0;
                cursor_history_pos = 0;
                if (cd_behaviour == 1)
                {
                    cursor_history[cursor_history_pos] = 0;
                    cursor_history_pos++;
                }
                sprintf(curr_dirname, "%s", ED64_FIRMWARE_PATH);
                sprintf(pwd, "/" ED64_FIRMWARE_PATH);
            }
            disp = gfx_lock_display();
            clearScreen(disp);
            readSDcard(disp, pwd);
            display_show(disp);
            input_mapping = file_manager;
            break;
        case ED64_ACT_FILE_TO_RAM:
        case ED64_ACT_RAM_TO_FILE:
        {
            const char *p = ed64menu_selected_path();
            char ext[8];
            ed64_extension(p, ext, sizeof(ext));
            FILINFO fi;
            u32 fsize = (f_stat(p, &fi) == FR_OK) ? fi.fsize : 0;
            int stype;
            if (!strcmp(ext, "EEP"))
                stype = (fsize > 512) ? 4 : 3;
            else if (!strcmp(ext, "FLA"))
                stype = 5;
            else
                stype = (fsize > 32768) ? 2 : 1;
            int size = saveTypeToSize(stype);
            u8 *buf = malloc(size);
            if (!buf)
            {
                ed64menu_message("Unexpected error", NULL);
                break;
            }
            FIL f;
            UINT n = 0;
            if (act == ED64_ACT_FILE_TO_RAM)
            {
                memset(buf, 0xFF, size);
                if (f_open(&f, p, FA_READ) == FR_OK)
                {
                    f_read(&f, buf, size, &n);
                    f_close(&f);
                }
                if (n && pushSaveToCart(stype, buf))
                    ed64menu_message("Operation complete", NULL);
                else
                    ed64menu_message("Disk I/O error", NULL);
            }
            else
            {
                if (getSaveFromCart(stype, buf) && f_open(&f, p, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
                {
                    f_write(&f, buf, size, &n);
                    f_close(&f);
                    ed64menu_message("Operation complete", NULL);
                }
                else
                    ed64menu_message("Disk I/O error", NULL);
            }
            free(buf);
            break;
        }
        case ED64_ACT_CPAK_TO_FILE:
            if (ed64_detect_pak(cpak_port) != ACCESSORY_MEMPAK)
            {
                ed64menu_message("Controller Pak not detected", NULL);
                break;
            }
            {
                int r = mpk_dump_to(cpak_port, ed64menu_selected_path());
                if (r == 0)
                    ed64menu_message("Operation complete", NULL);
                else if (r == 1)
                    ed64menu_message("Operation complete", "(pak read errors)");
                else
                    ed64menu_message("Disk I/O error", NULL);
            }
            break;
        case ED64_ACT_FILE_TO_CPAK:
            if (ed64_detect_pak(cpak_port) != ACCESSORY_MEMPAK)
            {
                ed64menu_message("Controller Pak not detected", NULL);
                break;
            }
            disp = gfx_lock_display();
            ed64_redraw_browser(disp);
            s64_dim(disp);
            ui_message(disp, "Processing...", NULL);
            display_show(disp);
            printText(" ", 9, 17, disp);
            file_to_mpk(disp, (char *) ed64menu_selected_path());
            display_show(disp);
            ed64menu_message("Operation complete", NULL);
            break;
        case ED64_ACT_APPLY_PATCH:
            strncpy(pending_patch, ed64menu_selected_path(), sizeof(pending_patch) - 1);
            pending_patch[sizeof(pending_patch) - 1] = 0;
            drawShortInfoBox(disp, " Patch set for next ROM", 2);
            ed64_close_menu(disp);
            break;
        case ED64_ACT_LOAD_PATH:
            ed64_load_path(disp, ed64menu_selected_path());
            break;
        default:
            ed64_close_menu(disp);
            break;
    }
}

static void handleInput(display_context_t disp_arg)
{
    display_context_t disp = disp_arg;
    controller_scan();
    struct controller_data keys = get_keys_down();
    struct controller_data keys_held = get_keys_held();

    if (keys.c[0].err != ERROR_NONE)
        return;

    if (input_mapping == ed64_menu)
    {
        ed64_do_action(disp, ed64menu_handle(disp, &keys, &keys_held));
        return;
    }

    if (keys.c[0].up || keys_held.c[0].up || keys_held.c[0].y > +25)
    {
        switch (input_mapping)
        {
            case file_manager:

                if (select_mode)
                {
                    if (count != 0)
                    {
                        if (scroll_behaviour == 1)
                            cursor--;
                        else if (cursor != 0)
                        {
                            if (cursor % 20 == 0)
                            {
                                cursor--;
                                page -= 20;
                            }
                            else
                            {
                                cursor--;
                            }
                        }

                        disp = gfx_lock_display();

                        new_scroll_pos(&cursor, &page, MAX_LIST, count);

                        clearScreen(disp);
                        display_dir(list, cursor, page, MAX_LIST, count, disp);

                        display_show(disp);
                    }
                }
                break;
            default:
                break;
        }
    }

    if (keys.c[0].down || keys_held.c[0].down || keys_held.c[0].y < -25)
    {
        switch (input_mapping)
        {
            case file_manager:

                if (select_mode)
                {
                    if (count != 0)
                    {
                        if (scroll_behaviour == 1)
                            cursor++;
                        else if (cursor + 1 != count)
                        {
                            if ((cursor + 1) % 20 == 0)
                            {
                                cursor++;
                                page += 20;
                            }
                            else
                            {
                                cursor++;
                            }
                        }

                        disp = gfx_lock_display();

                        new_scroll_pos(&cursor, &page, MAX_LIST, count);
                        clearScreen(disp);
                        display_dir(list, cursor, page, MAX_LIST, count, disp);

                        display_show(disp);
                    }
                }
                break;
            default:
                break;
        }
    }
    else if (keys.c[0].left || keys_held.c[0].left || keys_held.c[0].x < -25 || keys.c[0].L)
    {
        switch (input_mapping)
        {
            case file_manager:
                if (select_mode)
                {
                    if (count != 0 && scroll_behaviour == 0 && cursor - 20 >= 0)
                    {
                        page -= 20;
                        cursor = page;
                    }

                    disp = gfx_lock_display();

                    new_scroll_pos(&cursor, &page, MAX_LIST, count);
                    clearScreen(disp);
                    display_dir(list, cursor, page, MAX_LIST, count, disp);

                    display_show(disp);
                }
                break;
            default:
                break;
        }
    }
    else if (keys.c[0].right || keys_held.c[0].right || keys_held.c[0].x > +25 || keys.c[0].R)
    {
        switch (input_mapping)
        {
            case file_manager:

                if (select_mode)
                {
                    if ((count != 0) && (scroll_behaviour == 0) && (page + 20 != count) &&
                        (cursor + 20 <= ((count / 20) * 20 + 19)))
                    {
                        page += 20;
                        cursor = page;
                    }

                    disp = gfx_lock_display();

                    new_scroll_pos(&cursor, &page, MAX_LIST, count);
                    clearScreen(disp);
                    display_dir(list, cursor, page, MAX_LIST, count, disp);

                    display_show(disp);
                }
                break;

            default:
                break;
        }
    }
    else if (keys.c[0].start)
    {
        switch (input_mapping)
        {
            case file_manager:

                if (quick_boot)
                {
                    FRESULT result;
                    FIL file;
                    result = f_open(&file, "/" ED64_FIRMWARE_PATH "/SYS/LASTROM.CFG", FA_READ);

                    if (result == FR_OK)
                    {
                        int fsize = f_size(&file) + 1;
                        char lastrom_cfg_data[fsize];

                        f_gets(lastrom_cfg_data, fsize, &file);

                        f_close(&file);

                        char *file_name = strrchr(lastrom_cfg_data, '/');

                        disp = gfx_lock_display();
                        clearScreen(disp);

                        evd_ulockRegs();
                        sleep(10);

                        select_mode = 9;
                        readRomConfig(disp, &file_name[1], lastrom_cfg_data);

                        loadrom(disp, lastrom_cfg_data);
                        display_show(disp);
                    }
                    else
                    {
                        drawShortInfoBox(disp, "    ROM not found", 0);
                    }
                }
                else if (list[cursor].type != DT_DIR && empty == 0)
                {
                    loadFile(disp);
                }
                break;

            case rom_loaded:

                bootRom(disp, 0);
                break;

            default:
                break;
        }
    }
    else if (keys.c[0].Z)
    {
        switch (input_mapping)
        {
            case file_manager:
                ed64menu_open_main_menu(disp);
                input_mapping = ed64_menu;
                break;

            default:
                break;
        }
    }
    else if (keys.c[0].A)
    {
        switch (input_mapping)
        {
            case file_manager:
            {
                disp = gfx_lock_display();

                if (list[cursor].type == DT_DIR && empty == 0)
                {
                    char name_dir[256];

                    if (strcmp(pwd, "/") == 0)
                        sprintf(name_dir, "/%s", list[cursor].filename);
                    else
                        sprintf(name_dir, "%s/%s", pwd, list[cursor].filename);

                    sprintf(curr_dirname, "%s", list[cursor].filename);
                    sprintf(pwd, "%s", name_dir);

                    cursor_line = 0;

                    if (cd_behaviour == 1)
                    {
                        cursor_history[cursor_history_pos] = cursor;
                        cursor_history_pos++;
                    }

                    readSDcard(disp, name_dir);
                    display_show(disp);
                }
                else if (list[cursor].type != DT_DIR && empty == 0)
                {
                    ed64_open_file_menu_for_cursor(disp);
                }
                break;
            }
            default:
                break;
        }
    }
    else if (keys.c[0].B)
    {
        switch (input_mapping)
        {
            case file_manager:

                if (!(strcmp(pwd, "/") == 0))
                {
                    disp = gfx_lock_display();

                    int slash_pos = 0;
                    int i = 0;
                    while (pwd[i] != '\0')
                    {
                        if (pwd[i] == '/')
                            slash_pos = i;

                        i++;
                    }
                    char new_pwd[64];

                    int j;
                    for (j = 0; j < slash_pos; j++)
                        new_pwd[j] = pwd[j];

                    if (j == 0)
                        j++;

                    new_pwd[j] = '\0';

                    sprintf(pwd, "%s", new_pwd);

                    cursor_line = 0;

                    readSDcard(disp, pwd);

                    if (cd_behaviour == 1)
                    {
                        cursor_history_pos--;
                        cursor = cursor_history[cursor_history_pos];

                        if (cursor_history[cursor_history_pos] > 0)
                        {
                            cursor_line = cursor_history[cursor_history_pos] - 1;
                            if (scroll_behaviour == 0)
                            {
                                int p = cursor_line / 20;
                                page = p * 20;
                            }
                            else
                            {
                                if (cursor_line > 19)
                                    cursor_line = 19;
                            }
                        }
                        else
                            cursor_line = 0;
                    }

                    disp = gfx_lock_display();

                    new_scroll_pos(&cursor, &page, MAX_LIST, count);

                    clearScreen(disp);
                    display_dir(list, cursor, page, MAX_LIST, count, disp);

                    display_show(disp);
                }
                break;

            case mp3:
                mp3_Stop();
                mp3playing = 0;
                audio_close();
                free(buf_ptr);
                buf_ptr = 0;

                disp = gfx_lock_display();

                graphics_set_color(graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF),
                                   graphics_make_color(0x00, 0x00, 0x00, 0x00));
                new_scroll_pos(&cursor, &page, MAX_LIST, count);
                clearScreen(disp);
                display_show(disp);

                display_dir(list, cursor, page, MAX_LIST, count, disp);
                input_mapping = file_manager;
                display_show(disp);
                break;

            default:
                disp = gfx_lock_display();

                clearScreen(disp);
                display_dir(list, cursor, page, MAX_LIST, count, disp);

                display_show(disp);

                input_mapping = file_manager;
                break;
        }
    }
}

int main(void)
{
    int fast_boot = 0;

    list = malloc(sizeof(direntry_t));

    if (dfs_init(DFS_DEFAULT_LOCATION) == DFS_ESUCCESS)
    {
        configure();

        int save_job = evd_readReg(REG_SAV_CFG);

        if (save_job != 0)
            fast_boot = 1;

        controller_init();

        initFilesystem();

        readConfigFile();
        {
            char v[8];
            if (ini_get_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "cart_speed", v, sizeof(v)) == 0)
                cart_speed = (u8) atoi(v);
            if (ini_get_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "background", v, sizeof(v)) == 0)
                background_on = (u8) atoi(v);
            if (ini_get_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "crc_check", v, sizeof(v)) == 0)
                crc_check = (u8) atoi(v);
            if (ini_get_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "fast_reboot", v, sizeof(v)) == 0)
                fast_reboot = (u8) atoi(v);
            if (ini_get_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "game_id", v, sizeof(v)) == 0)
                game_id_on = (u8) atoi(v);
            {
                char cv[16];
                if (ini_get_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "boot_count", cv, sizeof(cv)) == 0)
                    boot_counter = (u32) atoi(cv);
                if (ini_get_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "game_count", cv, sizeof(cv)) == 0)
                    game_counter = (u32) atoi(cv);
                boot_counter++;
                snprintf(cv, sizeof(cv), "%lu", (unsigned long) boot_counter);
                ini_set_value("/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI", "ed64", "boot_count", cv);
            }
        }

        if (sd_speed == 2)
        {
            bi_speed50();
        }
        else
        {
            bi_speed25();
        }

        /*
         * TV Mode from Config and libdragon differ:
         *
         * Config | libdragon | Mode
         * -------|-----------|-------
         * 0      | None      | Auto
         * 1      | 1         | NTSC
         * 2      | 0         | PAL
         */
        switch (tv_mode)
        {
            case 1:
                *(u32 *) 0x80000300 = 1U;
                break;
            case 2:
                *(u32 *) 0x80000300 = 0U;
                break;
            default:
                break;
        }

        switch (0)
        {
            default:
                done3p = "...Done";
                romloaded = "ROM loaded";
                loading = "      Loading...";
                plgmp3 = "    Playing MP3";
                OpComsucc = "Operation completed successfully...";
                updatelastgamerecord = "Updating last played game record...";
                cpyingram2SD = "Copying save RAM to SD card...";
                emunofound = "  Emulator not found";
                errornoready = "ERROR: Not ready.";
                filenoexist = "ERROR: File does not exist.";
                pathnoexist = "ERROR: Path does not exist.";
                invalidname = "ERROR: Invalid name.";
                operdenied = "ERROR: Operation denied.";
                filealrrexist = "ERROR: File already exists.";
                errtimeout = "ERROR: Timeout.";
                devicelocked = "ERROR: Device locked.";
                readystr = "Ready.";
                filopenerror = "File open error";
                nosavfund = "No save found.";
                break;
        }

        init_interrupts();

        display_init(res, DEPTH_32_BPP, 3, GAMMA_NONE, ANTIALIAS_RESAMPLE);

        static display_context_t disp;

        disp = gfx_lock_display();

        load_wallpaper();

        border_color_1 = translate_color(border_color_1_s);
        border_color_2 = translate_color(border_color_2_s);
        box_color = translate_color(box_color_s);
        selection_color = translate_color(selection_color_s);
        selection_font_color = translate_color(selection_font_color_s);
        list_font_color = translate_color(list_font_color_s);
        list_dir_font_color = translate_color(list_dir_font_color_s);

        disp = gfx_lock_display();

        drawBg(disp);
        drawBoxNumber(disp, 1);

        display_show(disp);

        if (fast_boot)
        {
            backupSaveData(disp);
            disp = gfx_lock_display();
        }

        sprintf(pwd, "%s", "/");
        readSDcard(disp, "/");

        display_show(disp);

        {
            controller_scan();
            struct controller_data held = get_keys_pressed();
            if (!held.c[0].B)
            {
                FIL af;
                char ap[MAX_SUPPORTED_PATH_LEN] = "";
                const char *cfg = (fast_boot && fast_reboot) ? "/" ED64_FIRMWARE_PATH "/SYS/LASTROM.CFG"
                                  : (!fast_boot)             ? "/" ED64_FIRMWARE_PATH "/SYS/AUTOLOAD.CFG"
                                                             : NULL;
                if (cfg && f_open(&af, cfg, FA_READ) == FR_OK)
                {
                    f_gets(ap, sizeof(ap), &af);
                    f_close(&af);
                    char *nl = strpbrk(ap, "\r\n");
                    if (nl)
                        *nl = 0;
                    if (ap[0])
                        ed64_load_path(disp, ap);
                }
            }
        }
        for (;;)
        {
            handleInput(disp);

            if (mp3playing && audio_can_write())
            {
                mp3playing = mp3_Update(buf_ptr, buf_size);

                if (mp3playing)
                {
                    audio_write((short *) buf_ptr);
                }
            }

            if (input_mapping == file_manager)
                sleep(60);
        }
    }
    else
    {
        printf("Filesystem failed to start!\n");
        f_mount(0, "", 0);
        free(fs);
        for (;;)
            ;
    }
}
