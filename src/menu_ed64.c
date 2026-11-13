#include "menu_ed64.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"

#include "cheats_table.h"
#include "cic.h"
#include "constants.h"
#include "diag.h"
#include "everdrive.h"
#include "gfx.h"
#include "image.h"
#include "ini_write.h"
#include "main.h"
#include "mem.h"
#include "memorypak.h"
#include "rom.h"
#include "sd.h"
#include "sys.h"
#include "ui_cells.h"
#include "utils.h"

#define INI_PATH "/" ED64_FIRMWARE_PATH "/SYS/ALTRA64.INI"

enum Screen
{
    S_NONE,
    S_FILE_MENU,
    S_MAIN_MENU,
    S_DEVICE_INFO,
    S_DEVICE_DETAILS,
    S_HEX_VIEW,
    S_RECENT,
    S_OPTIONS,
    S_DIAG,
    S_CPAK_MENU,
    S_CPAK_PORT,
    S_CPAK_CONFIRM,
    S_ROM_INFO,
    S_FILE_INFO,
    S_DELETE_CONFIRM,
    S_ABOUT,
    S_OPT_HELP,
    S_MSG,
    S_CHEATS,
    S_CODE_MENU,
    S_CODE_EDIT,
    S_DESC_EDIT,
    S_SAVE_CHEATS,
    S_ROM_CONFIG,
    S_FAVORITES,
    S_IMAGE,
    S_TEXT
};

static enum Screen screen = S_NONE;
static int sel = 0;
static int dir_dirty = 0;

static char cur_path[MAX_SUPPORTED_PATH_LEN];
static char cur_name[64];
static int cur_is_rom = 0, cur_flags = 0;
static char sel_path[MAX_SUPPORTED_PATH_LEN];

static void scopyn(char *dst, const char *src, size_t n)
{
    strncpy(dst, src, n - 1);
    dst[n - 1] = 0;
}

enum
{
    FI_START,
    FI_ROMINFO,
    FI_ROMCFG,
    FI_HEX,
    FI_DELETE,
    FI_PATCH,
    FI_SETBG,
    FI_FILEINFO,
    FI_LOADCHEATS,
    FI_FILE2RAM,
    FI_RAM2FILE,
    FI_CPAK2FILE,
    FI_FILE2CPAK,
    FI_FAVORITE,
    FI_AUTOLOAD,
    FI_VIEWIMG,
    FI_VIEWTXT
};

struct fmenu
{
    const char *title;
    int n;
    const char *items[8];
    int map[8];
};
static struct fmenu fm_rom = {"File Menu",
                              6,
                              {"Start Game", "ROM Info", "ROM Config", "Delete", "Add to Favorites", "Set Autoload"},
                              {FI_START, FI_ROMINFO, FI_ROMCFG, FI_DELETE, FI_FAVORITE, FI_AUTOLOAD}};
static const struct fmenu fm_patch = {
    "File Menu", 3, {"Apply Patch", "Hex View", "Delete"}, {FI_PATCH, FI_HEX, FI_DELETE}};
static const struct fmenu fm_image = {"File Menu",
                                      5,
                                      {"Set Background", "View Image", "File Info", "Hex View", "Delete"},
                                      {FI_SETBG, FI_VIEWIMG, FI_FILEINFO, FI_HEX, FI_DELETE}};
static const struct fmenu fm_text = {"File Menu",
                                     5,
                                     {"Load Cheats", "View Text", "Hex View", "File Info", "Delete"},
                                     {FI_LOADCHEATS, FI_VIEWTXT, FI_HEX, FI_FILEINFO, FI_DELETE}};
static const struct fmenu fm_save = {"Save RAM",
                                     5,
                                     {"Copy File to RAM", "Copy RAM to File", "Hex View", "File Info", "Delete"},
                                     {FI_FILE2RAM, FI_RAM2FILE, FI_HEX, FI_FILEINFO, FI_DELETE}};
static const struct fmenu fm_mpk = {"Controller Pak Memory",
                                    5,
                                    {"CPak to File", "File to CPak", "Hex View", "File Info", "Delete"},
                                    {FI_CPAK2FILE, FI_FILE2CPAK, FI_HEX, FI_FILEINFO, FI_DELETE}};
static const struct fmenu fm_other = {
    "File Menu", 3, {"File Info", "Hex View", "Delete"}, {FI_FILEINFO, FI_HEX, FI_DELETE}};
static const struct fmenu *fm = &fm_other;

static const char *main_items[] = {"Recently Played", "Favorites", "CPak Manager", "System Folder",
                                   "Device Info",     "Options",   "About"};
#define MAIN_ITEMS 7
enum
{
    MI_RECENT,
    MI_FAVORITES,
    MI_CPAK,
    MI_SYSFOLDER,
    MI_DEVICE,
    MI_OPTIONS,
    MI_ABOUT
};

static const char *cpak_items[] = {"Save CPak as controller number", "Save CPak as last played game", "Format"};
#define CPAK_ITEMS 3
enum
{
    CA_SAVE_NUM,
    CA_SAVE_GAME,
    CA_FORMAT,
    CA_TO_FILE,
    CA_FROM_FILE
};
static int cpak_action = 0;

static void begin_browser(display_context_t *disp)
{
    *disp = gfx_lock_display();
    ed64_redraw_browser(*disp);
    s64_dim(*disp);
}

static void begin_black(display_context_t *disp)
{
    *disp = gfx_lock_display();
    ui_black(*disp);
}

static void begin_frame(display_context_t *disp)
{
    *disp = gfx_lock_display();
    clearScreen(*disp);
}

static int file_enabled(int i)
{
    int what = fm->map[i];
    if (what == FI_START || what == FI_ROMINFO || what == FI_ROMCFG)
        return cur_is_rom;
    return 1;
}

static void draw_file_menu(display_context_t disp)
{
    ui_menu(disp, fm->title, fm->items, fm->n, sel, file_enabled);
}

static void draw_main_menu(display_context_t disp)
{
    ui_menu(disp, "Main Menu", main_items, MAIN_ITEMS, sel, NULL);
}

static char di_val[12][24];

static void draw_device_info(display_context_t disp)
{
    static const char *labels[] = {"Cart Type",     "OS Version",   "OS Date",      "System Type",
                                   "Expansion Pak", "FPGA Core",    "SD Interface", "Assembly Date",
                                   "Game Counter",  "Boot Counter", "\x01",         "Press START for details"};
    const char *values[12];
    u16 firm = evd_getFirmVersion();
    u32 tv = *(volatile u32 *) 0x80000300;
    snprintf(di_val[0], 24, "Super 64 (ED64+)");
    snprintf(di_val[1], 24, "%s", Altra64_GetVersionString());
    snprintf(di_val[2], 24, "%s", Altra64_GetBuildDate());
    snprintf(di_val[3], 24, "%s", tv == 1 ? "NTSC" : tv == 0 ? "PAL" : tv == 2 ? "MPAL" : "?");
    snprintf(di_val[4], 24, "%s", gfx_has_expansion_pak() ? "Installed" : "Not Installed");
    snprintf(di_val[5], 24, "v%d.%02x", firm >> 8, firm & 0xff);
    snprintf(di_val[6], 24, "%s", sdGetInterface() == DISK_IFACE_SD ? "4-bit" : "1-bit SPI");
    if (asm_date)
        snprintf(di_val[7], 24, "%08lx", (unsigned long) asm_date);
    else
        snprintf(di_val[7], 24, "N/A");
    snprintf(di_val[8], 24, "%lu", (unsigned long) game_counter);
    snprintf(di_val[9], 24, "%lu", (unsigned long) boot_counter);
    for (int i = 0; i < 10; i++)
        values[i] = di_val[i];
    values[10] = NULL;
    values[11] = NULL;
    ui_kv(disp, "Device Info", labels, values, 12, -1);
}

static void draw_device_details(display_context_t disp)
{
    static const char *labels[] = {"DEVICE ID",  "CART DATE",   "OS DATE",   "BATTERY RAM",  "SERIAL",
                                   "RTC I2C",    "BOOT CONFIG", "LAST GAME", "SD INTERFACE", "SD SPEED",
                                   "CART SPEED", "MENU SIZE",   "MEMORY",    "RESET TYPE"};
    static char v[14][28];
    u32 mem = *(volatile u32 *) 0x80000318;
    char last[MAX_SUPPORTED_PATH_LEN] = "";
    FIL f;
    if (f_open(&f, "/" ED64_FIRMWARE_PATH "/SYS/LASTROM.CFG", FA_READ) == FR_OK)
    {
        f_gets(last, sizeof(last), &f);
        f_close(&f);
    }
    char *nl = strpbrk(last, "\r\n");
    if (nl)
        *nl = 0;
    const char *nm = strrchr(last, '/');
    nm = nm ? nm + 1 : last;
    snprintf(v[0], 28, "%04x (REG_VER)", evd_getFirmVersion());
    if (asm_date)
        snprintf(v[1], 28, "%08lx", (unsigned long) asm_date);
    else
        snprintf(v[1], 28, "N/A");
    snprintf(v[2], 28, "%s", Altra64_GetBuildDate());
    snprintf(v[3], 28, "None (FPGA SDRAM)");
    snprintf(v[4], 28, "N/A");
    snprintf(v[5], 28, "None");
    snprintf(v[6], 28, "%04x (REG_CFG)", (unsigned) evd_readReg(REG_CFG));
    snprintf(v[7], 28, "%.24s", *nm ? nm : "-");
    snprintf(v[8], 28, "%s", sdGetInterface() == DISK_IFACE_SD ? "4-bit" : "1-bit SPI");
    snprintf(v[9], 28, "%s", sd_speed == 2 ? "50 MHz" : "25 MHz");
    snprintf(v[10], 28, "%s", cart_speed == 2 ? "Fastest (08/20)" : cart_speed == 1 ? "Fast (PWD 0A)" : "Standard");
    snprintf(v[11], 28, "%s", "320x240");
    snprintf(v[12], 28, "%lu MB", (unsigned long) (mem >> 20));
    snprintf(v[13], 28, "%08lx (0x8000030C)", (unsigned long) *(volatile u32 *) 0x8000030C);
    ui_print_center(disp, 3, "Super 64 System Information", PAL_GRAY, PAL_BLACK);
    ui_dashed(disp, 4);
    for (int i = 0; i < 14; i++)
    {
        char l[16];
        snprintf(l, sizeof(l), "%-13s:", labels[i]);
        ui_print(disp, 2, 6 + i, l, PAL_GRAY, PAL_BLACK);
        ui_print(disp, 17, 6 + i, v[i], PAL_YELLOW, PAL_BLACK);
    }
    ui_dashed(disp, 21);
    ui_print(disp, 2, 23, "Press any key", PAL_GRAY, PAL_BLACK);
}

struct opt
{
    const char *label;
    const char *key;
    u8 *var;
    u8 nvals;
    const char *vals[4];
};

#define SEP_ROW {"\x01", "sep", NULL, 0, {0}}
static struct opt sopts[] = {
    {"TV Mode", "tv_mode", &tv_mode, 3, {"Automatic", "NTSC", "PAL"}},
    SEP_ROW,
    {"Background", "background", &background_on, 2, {"OFF", "ON"}},
    {"Page Number", "page_display", &page_display, 2, {"OFF", "ON"}},
    {"Hide Sys Dir", "hide_sysfolder", &hide_sysfolder, 2, {"OFF", "ON"}},
    SEP_ROW,
    {"Quick Boot", "quick_boot", &quick_boot, 2, {"OFF", "ON"}},
    {"Fast Reboot", "fast_reboot", &fast_reboot, 2, {"OFF", "ON"}},
    {"CRC Check", "crc_check", &crc_check, 2, {"OFF", "ON"}},
    {"HDMI Game ID", "game_id", &game_id_on, 2, {"OFF", "ON"}},
    {"Cart Speed", "cart_speed", &cart_speed, 3, {"Standard", "Fast", "Fastest"}},
    SEP_ROW,
    {"SD Speed", "sd_speed", &sd_speed, 2, {"-", "25 MHz", "50 MHz"}},
    {"Save Ext Type", "ext_type", &ext_type, 2, {"classic", "OS64"}},
    {"Save Backup", "save_backup", &save_backup, 2, {"OFF", "ON"}},
    SEP_ROW,
    {"[Diagnostics]", "diag", NULL, 0, {0}},
    {"Help", "help", NULL, 0, {0}},
};
#define OPT_IS_SEP(o)    ((o)->var == NULL && strcmp((o)->key, "sep") == 0)
#define OPT_IS_ACTION(o) ((o)->var == NULL && strcmp((o)->key, "sep") != 0)
#define OPT_VISIBLE      17
static int opt_first = 0;
#define NSOPTS (int) (sizeof(sopts) / sizeof(sopts[0]))
#define NOPTS  NSOPTS
static int opt_index(const char *key)
{
    for (int i = 0; i < NSOPTS; i++)
        if (!strcmp(sopts[i].key, key))
            return i;
    return 0;
}

static const char *opt_text(struct opt *o)
{
    u8 v = *o->var;
    if (o->var == &sd_speed)
        return (v == 2) ? "50 MHz" : "25 MHz";
    if (v >= o->nvals)
        v = 0;
    return o->vals[v];
}

static void opt_apply(struct opt *o)
{
    if (o->var == &sd_speed)
    {
        if (sd_speed == 2)
            bi_speed50();
        else
            bi_speed25();
    }
    else if (o->var == &tv_mode)
    {
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
    }
    else if (o->var == &hide_sysfolder)
        dir_dirty = 1;
}

static void opt_cycle(struct opt *o, int dir)
{
    char val[8];
    if (o->var == &sd_speed)
        sd_speed = (sd_speed == 2) ? 1 : 2;
    else
        *o->var = (u8) ((*o->var + o->nvals + dir) % o->nvals);
    snprintf(val, sizeof(val), "%d", *o->var);
    ini_set_value(INI_PATH, "ed64", o->key, val);
    opt_apply(o);
}

static void draw_options(display_context_t disp)
{
    const char *labels[NOPTS], *values[NOPTS];
    for (int i = 0; i < NSOPTS; i++)
    {
        labels[i] = sopts[i].label;
        values[i] = sopts[i].var ? opt_text(&sopts[i]) : NULL;
    }
    if (sel < opt_first)
        opt_first = sel;
    if (sel >= opt_first + OPT_VISIBLE)
        opt_first = sel - OPT_VISIBLE + 1;
    ui_kv_scroll(disp, "Options", labels, values, NOPTS, sel, opt_first, OPT_VISIBLE);
}

#define HELP_PER_PAGE 5
#define HELP_ITEMS    13
#define HELP_PAGES    ((HELP_ITEMS + HELP_PER_PAGE - 1) / HELP_PER_PAGE)
static int help_page = 0;
static const char *const help_text[HELP_ITEMS][3] = {
    {"TV Mode", "Video mode of the menu.", "Automatic follows the console."},
    {"Background", "Shows the wallpaper behind the", "menus."},
    {"Page Number", "Shows the page number in the", "file browser."},
    {"Hide Sys Dir", "Hides the ED64P system folder", "in the file browser."},
    {"Quick Boot", "START in the file browser starts", "the last played game."},
    {"Fast Reboot", "After a console reset, starts", "the last game. Hold B to skip."},
    {"CRC Check", "Fixes the ROM checksum at load.", "Needed for modified ROMs."},
    {"HDMI Game ID", "Sends the game ID to PixelFX", "HDMI mods at launch."},
    {"Cart Speed", "Cartridge bus timing in games.", "Faster shortens game loading."},
    {"SD Speed", "SD card clock. 50 MHz copies", "ROMs from the card faster."},
    {"Save Ext Type", "Save file names: classic", "(Altra64) or OS64 (official OS)."},
    {"Save Backup", "Keeps the three previous copies", "of each save on the card."},
    {"[Diagnostics]", "Board self-tests. Results go to", "SYS/SELFTEST.TXT."},
};

static void draw_opt_help(display_context_t disp)
{
    char t[24];
    snprintf(t, sizeof(t), "Options Help %d/%d", help_page + 1, HELP_PAGES);
    ui_box(disp, 2, 3, 37, 25, t);
    for (int r = 0; r < HELP_PER_PAGE; r++)
    {
        int i = help_page * HELP_PER_PAGE + r;
        if (i >= HELP_ITEMS)
            break;
        ui_print(disp, 4, 4 + r * 4, help_text[i][0], PAL_WHITE, PAL_DARK);
        ui_print(disp, 5, 5 + r * 4, help_text[i][1], PAL_YELLOW, PAL_DARK);
        ui_print(disp, 5, 6 + r * 4, help_text[i][2], PAL_YELLOW, PAL_DARK);
    }
    ui_print_center(disp, 24, "Up/Down: page   B: back", PAL_GRAY, PAL_DARK);
}

#define SELFTEST_LOG "/" ED64_FIRMWARE_PATH "/SYS/SELFTEST.TXT"
#define ST_PAGE      16
static int st_page = 0;

static void st_write_log(const char *stage)
{
    FIL f;
    if (f_open(&f, SELFTEST_LOG, FA_WRITE | FA_OPEN_ALWAYS) != FR_OK)
        return;
    f_lseek(&f, f_size(&f));
    f_puts("--- Super 64 OS ", &f);
    f_puts(Altra64_GetVersionString(), &f);
    f_puts(" diagnostic: ", &f);
    f_puts(stage, &f);
    f_puts(" ---\n", &f);
    for (int i = 0; i < diag_nlines; i++)
    {
        f_puts(diag_lines[i], &f);
        f_puts("\n", &f);
    }
    f_close(&f);
}

static void st_progress(const char *what)
{
    display_context_t d;
    char line[36];
    begin_black(&d);
    ui_print_center(d, 3, "Super 64 Diagnostics", PAL_GRAY, PAL_BLACK);
    ui_dashed(d, 4);
    snprintf(line, sizeof(line), "Testing %s... ", what);
    ui_print(d, 2, 6, line, PAL_GRAY, PAL_BLACK);
    display_show(d);
}

static void run_self_test(void)
{
    diag_nlines = 0;
    st_write_log("started");
    diag_run(st_progress);
    st_write_log(diag_failed ? "FAILED" : "PASSED");
    st_page = 0;
}

static void draw_diag(display_context_t disp)
{
    int pages = (diag_nlines + ST_PAGE - 1) / ST_PAGE;
    if (pages < 1)
        pages = 1;
    if (st_page >= pages)
        st_page = pages - 1;
    char l[40];
    ui_print_center(disp, 3, "Super 64 Diagnostics", PAL_GRAY, PAL_BLACK);
    ui_dashed(disp, 4);
    for (int i = st_page * ST_PAGE, r = 0; i < diag_nlines && r < ST_PAGE; i++, r++)
    {
        char l[37];
        snprintf(l, sizeof(l), "%.36s", diag_lines[i]);
        ui_text_marked(disp, 2, 5 + r, l);
    }
    ui_dashed(disp, 21);
    snprintf(l, sizeof(l), "Page %d of %d   L/R: page  (B) Back", st_page + 1, pages);
    ui_print(disp, 2, 23, l, PAL_GRAY, PAL_BLACK);
    ui_print(disp, 2, 24, "Result: ", PAL_GRAY, PAL_BLACK);
    ui_print(disp, 10, 24, diag_failed ? "FAIL" : "PASS", diag_failed ? PAL_RED : PAL_GREEN, PAL_BLACK);
    ui_print(disp, 2, 25, "Log: ED64P/SYS/SELFTEST.TXT", PAL_GRAY, PAL_BLACK);
}

static char cpak_port_label[4][40];

static void draw_cpak_menu(display_context_t disp)
{
    ui_menu(disp, "Controller Pak Manager", cpak_items, CPAK_ITEMS, sel, NULL);
}

static void draw_cpak_port(display_context_t disp)
{
    int present = get_controllers_present();
    const char *items[4];
    for (int i = 0; i < 4; i++)
    {
        const char *what = "No controller";
        if (present & (0xF000 >> (i * 4)))
        {
            int acc = ed64_detect_pak(i);
            what = (acc == ACCESSORY_MEMPAK)      ? "Controller Pak"
                   : (acc == ACCESSORY_RUMBLEPAK) ? "Rumble Pak"
                                                  : "No pak";
        }
        snprintf(cpak_port_label[i], 40, "controller-%d  %s", i + 1, what);
        items[i] = cpak_port_label[i];
    }
    ui_menu(disp, "Choose Controller Pak", items, 4, sel, NULL);
}

static void draw_cpak_confirm(display_context_t disp)
{
    char t[32];
    snprintf(t, sizeof(t), "Format Controller Pak %d?", cpak_port + 1);
    ui_yesno(disp, t, sel);
}

static u32 hex_off = 0, hex_size = 0;
static enum Screen hex_return = S_FILE_MENU;
#define HEX_LINES 14
#define HEX_BPL   16
#define HEX_PAGE  (HEX_LINES * HEX_BPL)

static void draw_hex_view(display_context_t disp)
{
    FIL f;
    UINT br = 0;
    static u8 buf[HEX_PAGE];
    char line[48];

    if (f_open(&f, cur_path, FA_READ) == FR_OK)
    {
        hex_size = f_size(&f);
        if (hex_off >= hex_size)
            hex_off = hex_size ? (hex_size - 1) / HEX_PAGE * HEX_PAGE : 0;
        f_lseek(&f, hex_off);
        f_read(&f, buf, sizeof(buf), &br);
        f_close(&f);
    }
    snprintf(line, sizeof(line), "%08lX of %08lX", (unsigned long) hex_off, (unsigned long) hex_size);
    ui_print(disp, 2, 3, line, PAL_GRAY, PAL_BLACK);
    for (int l = 0; l < HEX_LINES; l++)
    {
        int base = l * HEX_BPL;
        snprintf(line, sizeof(line), "%03lX", (unsigned long) ((hex_off + base) & 0xFFF));
        ui_print(disp, 2, 4 + l, line, PAL_GRAY, PAL_BLACK);
        for (int i = 0; i < HEX_BPL && base + i < (int) br; i++)
        {
            char h[3];
            sprintf(h, "%02X", buf[base + i]);
            ui_print(disp, 6 + 2 * i, 4 + l, h, buf[base + i] ? PAL_YELLOW : PAL_GRAY2, PAL_BLACK);
        }
    }
    ui_box(disp, 2, 18, 37, 26, NULL);
    for (int r = 0; r < 7; r++)
    {
        char *p = line;
        for (int i = 0; i < 32; i++)
        {
            int k = r * 32 + i;
            u8 c = (k < (int) br) ? buf[k] : ' ';
            *p++ = (c >= 0x20 && c < 0x7f) ? c : '.';
        }
        *p = 0;
        ui_print(disp, 4, 19 + r, line, PAL_GRAY, PAL_DARK);
    }
}

static char ri_val[12][24];
static const char *cic_name(int cic)
{
    switch (cic)
    {
        case 1:
            return "6101/7102";
        case 2:
            return "6102/7101";
        case 3:
            return "6103/7103";
        case 4:
            return "5101";
        case 5:
            return "6105/7105";
        case 6:
            return "6106/7106";
        case 7:
            return "5167";
        default:
            return "?";
    }
}
static const char *save_name(int s)
{
    switch (s)
    {
        case 0:
            return "OFF";
        case 1:
            return "SRAM";
        case 2:
            return "SRAM128K";
        case 3:
            return "EEP4K";
        case 4:
            return "EEP16K";
        case 5:
            return "FLASH";
        case 6:
            return "SRAM96K";
        default:
            return "?";
    }
}
static void fat_datetime(const FILINFO *fi, char *date, char *time)
{
    sprintf(date, "%02d.%02d.%04d", fi->fdate & 31, (fi->fdate >> 5) & 15, (fi->fdate >> 9) + 1980);
    sprintf(time, "%02d:%02d:%02d", fi->ftime >> 11, (fi->ftime >> 5) & 63, (fi->ftime & 31) * 2);
}

static const char *region_name(unsigned char cc)
{
    switch (cc)
    {
        case 'E':
        case 'J':
        case 'A':
        case 'G':
        case 'K':
        case 'N':
        case 'U':
        case 'Y':
            return "NTSC";
        case 'B':
            return "MPAL";
        case 'D':
        case 'F':
        case 'H':
        case 'I':
        case 'L':
        case 'P':
        case 'S':
        case 'W':
        case 'X':
        case 'Z':
        case 'C':
            return "PAL";
        default:
            return "WORLD";
    }
}

static int rom_info_ok = 0;
static void load_rom_info(void)
{
    static unsigned char hdr[4096];
    FIL f;
    UINT br = 0;
    FILINFO fi;
    rom_info_ok = 0;
    if (f_open(&f, cur_path, FA_READ) != FR_OK)
        return;
    u32 fsize = f_size(&f);
    f_read(&f, hdr, sizeof(hdr), &br);
    f_close(&f);
    if (br < 0x1000)
        return;
    int swapped = is_valid_rom(hdr) != 0;
    if (swapped)
        swap_header(hdr, 4096);
    int cic = get_cic(hdr), save = 0;
    get_cic_save(&hdr[0x3B], &cic, &save);
    snprintf(ri_val[0], 24, "%c%c%c%c", hdr[0x3B], hdr[0x3C], hdr[0x3D], hdr[0x3E]);
    snprintf(ri_val[1], 24, "%s", save_name(save));
    snprintf(ri_val[2], 24, "No");
    snprintf(ri_val[3], 24, "%s", region_name(hdr[0x3E]));
    snprintf(ri_val[4], 24, "%s", cic_name(cic));
    snprintf(ri_val[5], 24, "%s", swapped ? "Yes" : "No");
    snprintf(ri_val[6], 24, "%02X%02X%02X%02X", hdr[0x10], hdr[0x11], hdr[0x12], hdr[0x13]);
    snprintf(ri_val[7], 24, "%02X%02X%02X%02X", hdr[0x14], hdr[0x15], hdr[0x16], hdr[0x17]);
    snprintf(ri_val[8], 24, "%luMB", (unsigned long) ((fsize + 0xFFFFF) >> 20));
    if (f_stat(cur_path, &fi) == FR_OK)
        fat_datetime(&fi, ri_val[9], ri_val[10]);
    else
    {
        snprintf(ri_val[9], 24, "?");
        snprintf(ri_val[10], 24, "?");
    }
    rom_info_ok = 1;
}

static u8 rc_val[9];
static char rc_path[MAX_SUPPORTED_PATH_LEN];
static int rc_sel = 0;
static int rc_dirty = 0;
#define RC_ROWS   8
#define RC_EDITOR 4
#define RC_HEX    6

struct rc_row
{
    const char *label;
    signed char field;
    u8 n;
    u8 vals[8];
    const char *names[8];
};
static const struct rc_row rc_rows[RC_ROWS] = {
    {"Set CIC Type",
     0,
     8,
     {ROM_CFG_AUTO, 0, 1, 2, 4, 5, 3, 6},
     {"Automatic", "6101 / 7102", "6102 / 7101", "6103 / 7103", "6105 / 7105", "6106 / 7106", "5101 (Aleck 64)",
      "5167 (64DD conv)"}},
    {"Set Save Type",
     1,
     8,
     {ROM_CFG_AUTO, 0, 3, 4, 1, 6, 2, 5},
     {"Automatic", "None", "EEPROM 4kbit", "EEPROM 16kbit", "SRAM 256kbit", "SRAM 768kbit", "SRAM 1Mbit",
      "FlashRAM 1Mbit"}},
    {"Set TV Type", 2, 3, {0, 1, 2}, {"Automatic", "NTSC", "PAL"}},
    {"Use Cheats", 3, 2, {0, 1}, {"Off", "On"}},
    {"Cheat Editor", -1, 0, {0}, {0}},
    {"Use Patches", 7, 2, {0, 1}, {"Off", "On"}},
    {"Hex View", -1, 0, {0}, {0}},
    {"Reset Defaults", -1, 0, {0}, {0}},
};

static int rc_index(const struct rc_row *r)
{
    for (int i = 0; i < r->n; i++)
        if (r->vals[i] == rc_val[r->field])
            return i;
    return 0;
}

static void rc_defaults(void)
{
    for (int i = 0; i < RC_ROWS; i++)
        if (rc_rows[i].n)
            rc_val[rc_rows[i].field] = rc_rows[i].vals[0];
}

static void rc_cfg_name(char *out, size_t n)
{
    const char *dot = strrchr(cur_name, '.');
    if (dot && strlen(dot) == 4 && dot[2] == '6' && dot[3] == '4')
        snprintf(out, n, "/" ED64_FIRMWARE_PATH "/CFG/%.*s.CFG", (int) (dot - cur_name), cur_name);
    else
        snprintf(out, n, "/" ED64_FIRMWARE_PATH "/CFG/%s.CFG", dot ? dot + 1 : cur_name);
}

static void rc_load(void)
{
    FIL f;
    UINT br = 0;
    static u8 data[512];
    rc_cfg_name(rc_path, sizeof(rc_path));
    memset(rc_val, 0, sizeof(rc_val));
    rc_defaults();
    rc_dirty = 0;
    if (f_open(&f, rc_path, FA_READ) == FR_OK)
    {
        f_read(&f, data, sizeof(data), &br);
        f_close(&f);
        for (int i = 0; i < 9 && i < (int) br; i++)
            rc_val[i] = data[i];
    }
    for (int i = 0; i < RC_ROWS; i++)
        if (rc_rows[i].n)
            rc_val[rc_rows[i].field] = rc_rows[i].vals[rc_index(&rc_rows[i])];
}

static int rc_save(void)
{
    FIL f;
    UINT bw;
    static u8 data[512];
    memset(data, 0, sizeof(data));
    for (int i = 0; i < 9; i++)
        data[i] = rc_val[i];
    strncpy((char *) data + 32, cur_path, sizeof(data) - 33);
    f_mkdir("/" ED64_FIRMWARE_PATH "/CFG");
    if (f_open(&f, rc_path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return -1;
    f_write(&f, data, sizeof(data), &bw);
    f_close(&f);
    rc_dirty = 0;
    return 0;
}

static void draw_rom_config(display_context_t disp)
{
    const char *labels[RC_ROWS + 2], *values[RC_ROWS + 2];
    for (int i = 0; i < RC_ROWS; i++)
    {
        labels[i] = rc_rows[i].label;
        values[i] = rc_rows[i].n ? rc_rows[i].names[rc_index(&rc_rows[i])] : NULL;
    }
    labels[RC_ROWS] = "\x01";
    values[RC_ROWS] = NULL;
    labels[RC_ROWS + 1] = "(A) Select   (B) Back";
    values[RC_ROWS + 1] = NULL;
    ui_kv(disp, "ROM Config", labels, values, RC_ROWS + 2, rc_sel);
}

static void draw_rom_info(display_context_t disp)
{
    static const char *labels[] = {"ROM ID", "Save Type", "RTC",      "ROM Region", "CIC Type", "Byte Swap",
                                   "CRC HI", "CRC LO",    "ROM Size", "Date",       "Time"};
    const char *values[11];
    if (!rom_info_ok)
    {
        ui_message(disp, "File does not exist", NULL);
        return;
    }
    for (int i = 0; i < 11; i++)
        values[i] = ri_val[i];
    ui_kv(disp, "ROM Info", labels, values, 11, -1);
}

static void draw_file_info(display_context_t disp)
{
    static const char *labels[] = {"Name", "Size", "Date", "Time"};
    static char v[4][24];
    const char *values[4];
    FILINFO fi;
    if (f_stat(cur_path, &fi) != FR_OK)
    {
        ui_message(disp, "File does not exist", NULL);
        return;
    }
    snprintf(v[0], 24, "%.20s", cur_name);
    if (fi.fsize >= 1048576)
        snprintf(v[1], 24, "%lu MB", (unsigned long) (fi.fsize >> 20));
    else if (fi.fsize >= 1024)
        snprintf(v[1], 24, "%lu KB", (unsigned long) (fi.fsize >> 10));
    else
        snprintf(v[1], 24, "%lu B", (unsigned long) fi.fsize);
    fat_datetime(&fi, v[2], v[3]);
    for (int i = 0; i < 4; i++)
        values[i] = v[i];
    ui_kv(disp, "File Info", labels, values, 4, -1);
}

static void draw_delete_confirm(display_context_t disp)
{
    ui_yesno(disp, list[cursor].type == DT_DIR ? "Delete This Folder?" : "Delete This File?", sel);
}

#define RECENT_MAX  10
#define RECENT_FILE "/" ED64_FIRMWARE_PATH "/SYS/RECENT.LST"
static char recent[RECENT_MAX][MAX_SUPPORTED_PATH_LEN];
static int recent_count = 0;

static void recent_load(void)
{
    FIL f;
    recent_count = 0;
    if (f_open(&f, RECENT_FILE, FA_READ) != FR_OK)
        return;
    while (recent_count < RECENT_MAX && f_gets(recent[recent_count], MAX_SUPPORTED_PATH_LEN, &f))
    {
        char *nl = strpbrk(recent[recent_count], "\r\n");
        if (nl)
            *nl = 0;
        if (recent[recent_count][0])
            recent_count++;
    }
    f_close(&f);
}

static void recent_save(void)
{
    FIL f;
    if (f_open(&f, RECENT_FILE, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return;
    for (int i = 0; i < recent_count; i++)
    {
        f_puts(recent[i], &f);
        f_puts("\n", &f);
    }
    f_close(&f);
}

void ed64menu_recent_add(const char *fullpath)
{
    recent_load();
    int j = 0;
    char tmp[RECENT_MAX][MAX_SUPPORTED_PATH_LEN];
    scopyn(tmp[j++], fullpath, MAX_SUPPORTED_PATH_LEN);
    for (int i = 0; i < recent_count && j < RECENT_MAX; i++)
        if (strcmp(recent[i], fullpath) != 0)
            scopyn(tmp[j++], recent[i], MAX_SUPPORTED_PATH_LEN);
    recent_count = j;
    for (int i = 0; i < j; i++)
        scopyn(recent[i], tmp[i], MAX_SUPPORTED_PATH_LEN);
    recent_save();
}

static const char *basename_of(const char *p)
{
    const char *nm = strrchr(p, '/');
    return nm ? nm + 1 : p;
}

static void draw_recent(display_context_t disp)
{
    ui_frame_title(disp, "Recently Played");
    if (recent_count == 0)
        ui_print(disp, UI_LIST_COL, UI_LIST_ROW, "No game selected yet", PAL_GRAY, PAL_BLACK);
    for (int i = 0; i < recent_count; i++)
        ui_print_n(disp, UI_LIST_COL, UI_LIST_ROW + i, basename_of(recent[i]), UI_LIST_COLS,
                   (i == sel) ? PAL_WHITE : PAL_GRAY, PAL_BLACK);
    if (recent_count)
        ui_bottom_name(disp, basename_of(recent[sel]));
}

static sprite_t *logo = NULL;

static void draw_about(display_context_t disp)
{
    char ver[40];
    if (!logo)
        logo = read_sprite("rom://sprites/logo.sprite");
    if (logo)
        graphics_draw_sprite(disp, 0, 0, logo);
    snprintf(ver, sizeof(ver), "Version: %s", Altra64_GetVersionString());
    ui_print_center_px(disp, 136, "Super 64 (ED64P) Project", PAL_WHITE, PAL_BLACK);
    ui_print_center_px(disp, 148, ver, PAL_YELLOW, PAL_BLACK);
    ui_print_center_px(disp, 160, "Based on: Altra64, alt64, ED-64 IO", PAL_YELLOW, PAL_BLACK);
}

static char msg_l1[40], msg_l2[40];
static int msg_black = 0;
static enum Screen msg_return = S_NONE;
static void draw_cheats_frame(display_context_t disp);
static void draw_msg(display_context_t disp)
{
    if (msg_return != S_NONE)
        draw_cheats_frame(disp);
    ui_message(disp, msg_l1, msg_l2[0] ? msg_l2 : NULL);
}

#define FAV_MAX       20
#define FAV_FILE      "/" ED64_FIRMWARE_PATH "/SYS/FAVORITES.LST"
#define AUTOLOAD_FILE "/" ED64_FIRMWARE_PATH "/SYS/AUTOLOAD.CFG"
static char favs[FAV_MAX][MAX_SUPPORTED_PATH_LEN];
static int fav_count = 0;

static void fav_load(void)
{
    FIL f;
    fav_count = 0;
    if (f_open(&f, FAV_FILE, FA_READ) != FR_OK)
        return;
    while (fav_count < FAV_MAX && f_gets(favs[fav_count], MAX_SUPPORTED_PATH_LEN, &f))
    {
        char *nl = strpbrk(favs[fav_count], "\r\n");
        if (nl)
            *nl = 0;
        if (favs[fav_count][0])
            fav_count++;
    }
    f_close(&f);
}

static void fav_save(void)
{
    FIL f;
    if (f_open(&f, FAV_FILE, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return;
    for (int i = 0; i < fav_count; i++)
    {
        f_puts(favs[i], &f);
        f_puts("\n", &f);
    }
    f_close(&f);
}

static int fav_index(const char *path)
{
    for (int i = 0; i < fav_count; i++)
        if (!strcmp(favs[i], path))
            return i;
    return -1;
}

static void fav_toggle(const char *path)
{
    fav_load();
    int i = fav_index(path);
    if (i >= 0)
    {
        for (; i < fav_count - 1; i++)
            strcpy(favs[i], favs[i + 1]);
        fav_count--;
    }
    else if (fav_count < FAV_MAX)
    {
        strcpy(favs[fav_count++], path);
    }
    fav_save();
}

static int autoload_is(const char *path)
{
    FIL f;
    char ap[MAX_SUPPORTED_PATH_LEN] = "";
    if (f_open(&f, AUTOLOAD_FILE, FA_READ) != FR_OK)
        return 0;
    f_gets(ap, sizeof(ap), &f);
    f_close(&f);
    char *nl = strpbrk(ap, "\r\n");
    if (nl)
        *nl = 0;
    return !strcmp(ap, path);
}

static void autoload_toggle(const char *path)
{
    if (autoload_is(path))
    {
        f_unlink(AUTOLOAD_FILE);
        return;
    }
    FIL f;
    if (f_open(&f, AUTOLOAD_FILE, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
        f_puts(path, &f);
        f_close(&f);
    }
}

static void rom_menu_labels(void)
{
    fav_load();
    fm_rom.items[4] = (fav_index(cur_path) >= 0) ? "Remove from Favorites" : "Add to Favorites";
    fm_rom.items[5] = autoload_is(cur_path) ? "Clear Autoload" : "Set Autoload";
}

static void draw_favorites(display_context_t disp)
{
    ui_frame_title(disp, "Favorites");
    if (fav_count == 0)
        ui_print(disp, UI_LIST_COL, UI_LIST_ROW, "No favorites yet", PAL_GRAY, PAL_BLACK);
    for (int i = 0; i < fav_count; i++)
        ui_print_n(disp, UI_LIST_COL, UI_LIST_ROW + i, basename_of(favs[i]), UI_LIST_COLS,
                   (i == sel) ? PAL_WHITE : PAL_GRAY, PAL_BLACK);
    if (fav_count)
        ui_bottom_name(disp, basename_of(favs[sel]));
    ui_print(disp, UI_LIST_COL, UI_BAR_ROW + 1, "(A) Start  (R) Remove  (B) Back", PAL_GRAY, PAL_BLACK);
}

static sprite_t *view_img = NULL;
static void draw_image_view(display_context_t disp)
{
    if (!view_img)
        view_img = loadPng((char *) cur_path);
    if (!view_img)
    {
        ui_message(disp, "JPEG decoder error", NULL);
        return;
    }
    int x = (320 - view_img->width) / 2, y = (240 - view_img->height) / 2;
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    graphics_draw_sprite(disp, x, y, view_img);
}

#define TV_COLS 36
#define TV_ROWS 21
#define TV_MAX  61440
static char *tv_buf = NULL;
static int tv_len = 0, tv_lines = 0, tv_top = 0;
static int *tv_off = NULL;

static void tv_free(void)
{
    free(tv_buf);
    tv_buf = NULL;
    free(tv_off);
    tv_off = NULL;
    tv_lines = 0;
}

static int tv_open(const char *path)
{
    FIL f;
    UINT br = 0;
    tv_free();
    if (f_open(&f, path, FA_READ) != FR_OK)
        return 0;
    tv_buf = malloc(TV_MAX + 1);
    if (!tv_buf)
    {
        f_close(&f);
        return 0;
    }
    f_read(&f, tv_buf, TV_MAX, &br);
    f_close(&f);
    tv_len = (int) br;
    tv_buf[tv_len] = 0;
    tv_off = malloc(sizeof(int) * (tv_len / 1 + 2));
    if (!tv_off)
    {
        tv_free();
        return 0;
    }
    int i = 0;
    while (i < tv_len)
    {
        tv_off[tv_lines] = i;
        tv_lines++;
        int n = 0;
        while (i < tv_len && tv_buf[i] != '\n' && n < TV_COLS)
        {
            i++;
            n++;
        }
        if (i < tv_len && tv_buf[i] == '\n')
            i++;
    }
    tv_top = 0;
    return 1;
}

static void draw_text_view(display_context_t disp)
{
    char l[48];
    snprintf(l, sizeof(l), "%.28s  %d/%d", cur_name, tv_lines ? tv_top + 1 : 0, tv_lines);
    ui_print(disp, 2, 3, l, PAL_GRAY, PAL_BLACK);
    for (int r = 0; r < TV_ROWS && tv_top + r < tv_lines; r++)
    {
        int o = tv_off[tv_top + r];
        int e = (tv_top + r + 1 < tv_lines) ? tv_off[tv_top + r + 1] : tv_len;
        int n = 0;
        for (int k = o; k < e && n < TV_COLS; k++)
        {
            unsigned char c = (unsigned char) tv_buf[k];
            if (c == '\n' || c == '\r')
                break;
            if (c == '\t')
                c = ' ';
            if (c < 0x20)
                c = '.';
            ui_cell(disp, 2 + n, 4 + r, c, PAL_YELLOW, PAL_BLACK);
            n++;
        }
    }
    ui_print(disp, 2, 26, "Up/Down: line  L/R: page  (B) Back", PAL_GRAY, PAL_BLACK);
}

static void redraw(display_context_t disp);
static int ch_sel = 0;
static enum Screen ch_return = S_MAIN_MENU;
static int ce_cur = 0, ce_key = 0;
static char ce_hex[13];
static int de_cur = 0, de_key = 0;
static char de_text[CH_DESC_LEN];
static const char de_keys[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ+-. ";
static const char *code_menu_items[] = {"Edit Code", "Description", "Clear Code", "Disable"};

static void ch_game_from_name(const char *name, char *out, size_t n)
{
    strncpy(out, name, n - 1);
    out[n - 1] = 0;
    char *dot = strrchr(out, '.');
    if (dot)
        *dot = 0;
}

static int last_game_name(char *out, size_t n)
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
    const char *nm = strrchr(last, '/');
    nm = nm ? nm + 1 : last;
    if (!*nm)
        return 0;
    ch_game_from_name(nm, out, n);
    return 1;
}

static void draw_cheats_frame(display_context_t disp)
{
    ui_fill(disp, 2, 1, 37, 3, ' ', PAL_GRAY, PAL_DARK);
    ui_hline(disp, 2, 1, 36, 0xC4, PAL_GRAY, PAL_DARK);
    ui_hline(disp, 2, 3, 36, 0xC4, PAL_GRAY, PAL_DARK);
    const char *d = ch_codes[ch_sel].desc[0] ? ch_codes[ch_sel].desc : "No code description";
    ui_print_center(disp, 2, d, PAL_WHITE, PAL_DARK);
    ui_fill(disp, 2, 4, 37, 22, ' ', PAL_GRAY, PAL_DARK);
    ui_hline(disp, 2, 4, 36, 0xC4, PAL_GRAY, PAL_DARK);
    ui_hline(disp, 2, 22, 36, 0xC4, PAL_GRAY, PAL_DARK);
    ui_print_center(disp, 4, "Cheat Code Editor", PAL_GRAY, PAL_DARK);
    for (int i = 0; i < CH_MAX; i++)
    {
        char l[20];
        snprintf(l, sizeof(l), "%08lX %04X %s", (unsigned long) ch_codes[i].addr, ch_codes[i].val,
                 ch_codes[i].on ? "ON" : "OFF");
        ui_print(disp, (i < 17) ? 2 : 20, 5 + (i % 17), l, (i == ch_sel) ? PAL_WHITE : PAL_YELLOW, PAL_DARK);
    }
    ui_fill(disp, 2, 25, 37, 28, ' ', PAL_GRAY, PAL_DARK);
    ui_hline(disp, 2, 25, 36, 0xC4, PAL_GRAY, PAL_DARK);
    ui_hline(disp, 2, 28, 36, 0xC4, PAL_GRAY, PAL_DARK);
    ui_print_center(disp, 26, "These codes will be applied to", PAL_WHITE, PAL_DARK);
    char g[40];
    snprintf(g, sizeof(g), "%.34s", ch_game);
    ui_print_center(disp, 27, g, PAL_WHITE, PAL_DARK);
}

static void draw_code_menu(display_context_t disp)
{
    draw_cheats_frame(disp);
    code_menu_items[3] = ch_codes[ch_sel].on ? "Disable" : "Enable";
    ui_menu(disp, "Code Menu", code_menu_items, 4, sel, NULL);
}

static void draw_code_edit(display_context_t disp)
{
    draw_cheats_frame(disp);
    ui_box(disp, 12, 9, 26, 19, "Code Edit");
    for (int i = 0; i < 12; i++)
    {
        int col = 13 + i + (i >= 8 ? 1 : 0);
        ui_cell(disp, col, 10, (unsigned char) ce_hex[i], (i == ce_cur) ? PAL_WHITE : PAL_YELLOW,
                (i == ce_cur) ? PAL_GRAY : PAL_DARK);
    }
    for (int k = 0; k < 16; k++)
    {
        int col = 16 + 2 * (k % 4), row = 12 + 2 * (k / 4);
        ui_cell(disp, col, row, (unsigned char) "0123456789ABCDEF"[k], (k == ce_key) ? PAL_WHITE : PAL_YELLOW,
                (k == ce_key) ? PAL_GRAY : PAL_DARK);
    }
}

static void draw_desc_edit(display_context_t disp)
{
    draw_cheats_frame(disp);
    ui_box(disp, 2, 8, 37, 20, NULL);
    int len = (int) strlen(de_text);
    for (int i = 0; i <= len && i < CH_DESC_LEN - 1; i++)
    {
        unsigned char c = (i < len) ? (unsigned char) de_text[i] : ' ';
        ui_cell(disp, 3 + i, 9, c, (i == de_cur) ? PAL_WHITE : PAL_YELLOW, (i == de_cur) ? PAL_GRAY : PAL_DARK);
    }
    ui_print(disp, 10, 11, "<L", PAL_GRAY, PAL_DARK);
    ui_print(disp, 28, 11, "R>", PAL_GRAY, PAL_DARK);
    for (int k = 0; k < 40; k++)
    {
        int col = 10 + 2 * (k % 10), row = 13 + 2 * (k / 10);
        unsigned char c = (de_keys[k] == ' ') ? 0xFE : (unsigned char) de_keys[k];
        ui_cell(disp, col, row, c, (k == de_key) ? PAL_WHITE : PAL_YELLOW, (k == de_key) ? PAL_GRAY : PAL_DARK);
    }
}

static void draw_save_cheats(display_context_t disp)
{
    draw_cheats_frame(disp);
    ui_yesno(disp, "Save cheats?", sel);
}

static void open_cheats(display_context_t disp, const char *game, enum Screen back)
{
    cheats_load(game);
    ch_sel = 0;
    ch_return = back;
    screen = S_CHEATS;
    redraw(disp);
}

static void redraw(display_context_t disp)
{
    switch (screen)
    {
        case S_DEVICE_DETAILS:
        case S_HEX_VIEW:
        case S_DIAG:
        case S_ABOUT:
        case S_CHEATS:
        case S_CODE_MENU:
        case S_CODE_EDIT:
        case S_DESC_EDIT:
        case S_SAVE_CHEATS:
        case S_IMAGE:
        case S_TEXT:
            begin_black(&disp);
            break;
        case S_FAVORITES:
            begin_frame(&disp);
            break;
        case S_MSG:
            if (msg_black)
                begin_black(&disp);
            else
                begin_browser(&disp);
            break;
        case S_RECENT:
            begin_frame(&disp);
            break;
        default:
            begin_browser(&disp);
            break;
    }
    switch (screen)
    {
        case S_FILE_MENU:
            draw_file_menu(disp);
            break;
        case S_MAIN_MENU:
            draw_main_menu(disp);
            break;
        case S_DEVICE_INFO:
            draw_device_info(disp);
            break;
        case S_DEVICE_DETAILS:
            draw_device_details(disp);
            break;
        case S_HEX_VIEW:
            draw_hex_view(disp);
            break;
        case S_RECENT:
            draw_recent(disp);
            break;
        case S_OPTIONS:
            draw_options(disp);
            break;
        case S_DIAG:
            draw_diag(disp);
            break;
        case S_CPAK_MENU:
            draw_cpak_menu(disp);
            break;
        case S_CPAK_PORT:
            draw_cpak_port(disp);
            break;
        case S_CPAK_CONFIRM:
            draw_cpak_confirm(disp);
            break;
        case S_ROM_INFO:
            draw_rom_info(disp);
            break;
        case S_ROM_CONFIG:
            draw_rom_config(disp);
            break;
        case S_FILE_INFO:
            draw_file_info(disp);
            break;
        case S_DELETE_CONFIRM:
            draw_delete_confirm(disp);
            break;
        case S_ABOUT:
            draw_about(disp);
            break;
        case S_OPT_HELP:
            draw_opt_help(disp);
            break;
        case S_MSG:
            draw_msg(disp);
            break;
        case S_CHEATS:
            draw_cheats_frame(disp);
            break;
        case S_CODE_MENU:
            draw_code_menu(disp);
            break;
        case S_CODE_EDIT:
            draw_code_edit(disp);
            break;
        case S_DESC_EDIT:
            draw_desc_edit(disp);
            break;
        case S_SAVE_CHEATS:
            draw_save_cheats(disp);
            break;
        case S_FAVORITES:
            draw_favorites(disp);
            break;
        case S_IMAGE:
            draw_image_view(disp);
            break;
        case S_TEXT:
            draw_text_view(disp);
            break;
        default:
            break;
    }
    display_show(disp);
}

void ed64menu_message(const char *l1, const char *l2)
{
    scopyn(msg_l1, l1 ? l1 : "", sizeof(msg_l1));
    scopyn(msg_l2, l2 ? l2 : "", sizeof(msg_l2));
    msg_black = 0;
    msg_return = S_NONE;
    screen = S_MSG;
    redraw(0);
}

static int show_msg_here(display_context_t disp, const char *l1, const char *l2)
{
    scopyn(msg_l1, l1 ? l1 : "", sizeof(msg_l1));
    scopyn(msg_l2, l2 ? l2 : "", sizeof(msg_l2));
    msg_return = (screen == S_CODE_EDIT || screen == S_CHEATS || screen == S_SAVE_CHEATS) ? screen : S_NONE;
    msg_black = (msg_return != S_NONE);
    screen = S_MSG;
    redraw(disp);
    return ED64_ACT_NONE;
}

void ed64menu_open_file_menu(display_context_t disp, const char *fullpath, const char *shortname, int is_rom, int flags)
{
    scopyn(cur_path, fullpath, sizeof(cur_path));
    scopyn(cur_name, shortname, sizeof(cur_name));
    cur_is_rom = is_rom;
    cur_flags = flags;
    if (is_rom)
    {
        rom_menu_labels();
        fm = &fm_rom;
    }
    else if (flags & ED64_FILE_PATCH)
        fm = &fm_patch;
    else if (flags & ED64_FILE_IMAGE)
        fm = &fm_image;
    else if (flags & ED64_FILE_TEXT)
        fm = &fm_text;
    else if (flags & ED64_FILE_SAVE)
        fm = &fm_save;
    else if (flags & ED64_FILE_MPK)
        fm = &fm_mpk;
    else
        fm = &fm_other;
    screen = S_FILE_MENU;
    sel = 0;
    redraw(disp);
}

void ed64menu_open_main_menu(display_context_t disp)
{
    screen = S_MAIN_MENU;
    sel = 0;
    redraw(disp);
}

const char *ed64menu_selected_path(void)
{
    return sel_path;
}
static int items_for(enum Screen s)
{
    switch (s)
    {
        case S_FILE_MENU:
            return fm->n;
        case S_MAIN_MENU:
            return MAIN_ITEMS;
        case S_RECENT:
            return recent_count;
        case S_FAVORITES:
            return fav_count;
        case S_OPTIONS:
            return NOPTS;
        case S_CPAK_MENU:
            return CPAK_ITEMS;
        case S_CPAK_PORT:
            return 4;
        default:
            return 0;
    }
}

static int close_or_refresh(void)
{
    screen = S_NONE;
    if (dir_dirty)
    {
        dir_dirty = 0;
        return ED64_ACT_REFRESH;
    }
    return ED64_ACT_CLOSE;
}

static int handle_inner(display_context_t disp, struct controller_data *keys, struct controller_data *held);

int ed64menu_handle(display_context_t disp, struct controller_data *keys, struct controller_data *held)
{
    return handle_inner(disp, keys, held);
}

static int handle_inner(display_context_t disp, struct controller_data *keys, struct controller_data *held)
{
    int n = items_for(screen);
    static int stick_dir = 0;
    static u32 stick_next = 0;
    int sx = held->c[0].x, sy = held->c[0].y;
    int ax = sx < 0 ? -sx : sx, ay = sy < 0 ? -sy : sy;
    int want = 0;
    if (ay >= ax && ay >= 55)
        want = (sy > 0) ? 1 : 2;
    else if (ax > ay && ax >= 55)
        want = (sx < 0) ? 4 : 8;
    int edge = 0;
    u32 now = get_ticks_ms();
    if (stick_dir && ax < 30 && ay < 30)
        stick_dir = 0;
    if (want && want != stick_dir && !stick_dir)
    {
        stick_dir = want;
        edge = want;
        stick_next = now + 450;
    }
    else if (stick_dir && want == stick_dir && (s32) (now - stick_next) >= 0)
    {
        edge = want;
        stick_next = now + 160;
    }
    int up = keys->c[0].up || (edge & 1);
    int down = keys->c[0].down || (edge & 2);
    int left = keys->c[0].left || (edge & 4);
    int right = keys->c[0].right || (edge & 8);
    int A = keys->c[0].A, B = keys->c[0].B;

    if (screen == S_HEX_VIEW)
    {
        int hup = up || held->c[0].up, hdown = down || held->c[0].down;
        if (B)
        {
            screen = hex_return;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (hup)
        {
            hex_off = hex_off >= HEX_BPL ? hex_off - HEX_BPL : 0;
            redraw(disp);
        }
        if (hdown)
        {
            hex_off += HEX_BPL;
            redraw(disp);
        }
        if (keys->c[0].left)
        {
            hex_off = hex_off >= HEX_PAGE ? hex_off - HEX_PAGE : 0;
            redraw(disp);
        }
        if (keys->c[0].right)
        {
            hex_off += HEX_PAGE;
            redraw(disp);
        }
        if (keys->c[0].L)
        {
            hex_off = hex_off >= 4096 ? hex_off - 4096 : 0;
            redraw(disp);
        }
        if (keys->c[0].R)
        {
            hex_off += 4096;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_DIAG)
    {
        if (keys->c[0].left || keys->c[0].L)
        {
            if (st_page > 0)
                st_page--;
            redraw(disp);
        }
        if (keys->c[0].right || keys->c[0].R)
        {
            st_page++;
            redraw(disp);
        }
        if (B)
        {
            screen = S_OPTIONS;
            sel = opt_index("diag");
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_DEVICE_INFO)
    {
        if (keys->c[0].start)
        {
            screen = S_DEVICE_DETAILS;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B || A)
        {
            screen = S_MAIN_MENU;
            sel = MI_DEVICE;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_DEVICE_DETAILS)
    {
        if (B || A || keys->c[0].start)
        {
            screen = S_DEVICE_INFO;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_OPT_HELP)
    {
        if (B || A)
        {
            screen = S_OPTIONS;
            redraw(disp);
        }
        else if (up || left || keys->c[0].L)
        {
            help_page = (help_page + HELP_PAGES - 1) % HELP_PAGES;
            redraw(disp);
        }
        else if (down || right || keys->c[0].R)
        {
            help_page = (help_page + 1) % HELP_PAGES;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_ABOUT)
    {
        if (B || A)
        {
            screen = S_MAIN_MENU;
            sel = MI_ABOUT;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_ROM_CONFIG)
    {
        if (up)
        {
            rc_sel = (rc_sel + RC_ROWS - 1) % RC_ROWS;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (down)
        {
            rc_sel = (rc_sel + 1) % RC_ROWS;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        const struct rc_row *r = &rc_rows[rc_sel];
        if (r->n && (A || left || right || keys->c[0].L || keys->c[0].R))
        {
            int dir = (left || keys->c[0].L) ? -1 : 1;
            rc_val[r->field] = r->vals[(rc_index(r) + r->n + dir) % r->n];
            rc_dirty = 1;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (A && rc_sel == RC_EDITOR)
        {
            char g[64];
            ch_game_from_name(cur_name, g, sizeof(g));
            open_cheats(disp, g, S_ROM_CONFIG);
            return ED64_ACT_NONE;
        }
        if (A && rc_sel == RC_HEX)
        {
            screen = S_HEX_VIEW;
            hex_return = S_ROM_CONFIG;
            hex_off = 0;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (A)
        {
            rc_defaults();
            rc_dirty = 1;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B)
        {
            screen = S_FILE_MENU;
            if (!rc_dirty)
            {
                redraw(disp);
                return ED64_ACT_NONE;
            }
            if (rc_save() != 0)
                return show_msg_here(disp, "Disk I/O error", NULL);
            return show_msg_here(disp, "ROM Config saved", NULL);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_ROM_INFO || screen == S_FILE_INFO)
    {
        if (B || A)
        {
            screen = S_FILE_MENU;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_MSG)
    {
        if (B || A || keys->c[0].start)
        {
            if (msg_return != S_NONE)
            {
                screen = msg_return;
                msg_return = S_NONE;
                redraw(disp);
                return ED64_ACT_NONE;
            }
            return close_or_refresh();
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_OPTIONS)
    {
        struct opt *table = sopts;
        if (up)
        {
            do
                sel = (sel + n - 1) % n;
            while (OPT_IS_SEP(&table[sel]));
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (down)
        {
            do
                sel = (sel + 1) % n;
            while (OPT_IS_SEP(&table[sel]));
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (OPT_IS_ACTION(&table[sel]) && A)
        {
            if (!strcmp(table[sel].key, "diag"))
            {
                run_self_test();
                screen = S_DIAG;
                redraw(disp);
                return ED64_ACT_NONE;
            }
            if (!strcmp(table[sel].key, "help"))
            {
                help_page = 0;
                screen = S_OPT_HELP;
                redraw(disp);
                return ED64_ACT_NONE;
            }
        }
        if (table[sel].var && (A || keys->c[0].R || right || keys->c[0].L || left))
        {
            struct opt *o = &table[sel];
            opt_cycle(o, (keys->c[0].L || left) ? -1 : +1);
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B)
        {
            screen = S_MAIN_MENU;
            sel = MI_OPTIONS;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_DELETE_CONFIRM || screen == S_CPAK_CONFIRM)
    {
        if (left || right || up || down)
        {
            sel ^= 1;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B)
        {
            sel = 1;
        }
        if (A || B)
        {
            if (screen == S_DELETE_CONFIRM)
            {
                if (sel == 0)
                {
                    scopyn(sel_path, cur_path, sizeof(sel_path));
                    screen = S_NONE;
                    return ED64_ACT_DELETE;
                }
                screen = S_FILE_MENU;
                sel = fm->n - 1;
                redraw(disp);
                return ED64_ACT_NONE;
            }
            if (sel == 0)
            {
                screen = S_NONE;
                return ED64_ACT_CPAK_FORMAT;
            }
            screen = S_CPAK_PORT;
            sel = cpak_port;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        return ED64_ACT_NONE;
    }

    if (screen == S_IMAGE)
    {
        if (B || A || keys->c[0].start)
        {
            if (view_img)
            {
                free(view_img);
                view_img = NULL;
            }
            screen = S_FILE_MENU;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_TEXT)
    {
        int hup = up || held->c[0].up, hdown = down || held->c[0].down;
        if (B)
        {
            tv_free();
            screen = S_FILE_MENU;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (hup && tv_top > 0)
        {
            tv_top--;
            redraw(disp);
        }
        if (hdown && tv_top + TV_ROWS < tv_lines)
        {
            tv_top++;
            redraw(disp);
        }
        if ((keys->c[0].L || left) && tv_top > 0)
        {
            tv_top = tv_top >= TV_ROWS ? tv_top - TV_ROWS : 0;
            redraw(disp);
        }
        if ((keys->c[0].R || right) && tv_top + TV_ROWS < tv_lines)
        {
            tv_top += TV_ROWS;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_FAVORITES)
    {
        if (up && n)
        {
            sel = (sel + n - 1) % n;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (down && n)
        {
            sel = (sel + 1) % n;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (keys->c[0].R && fav_count)
        {
            fav_toggle(favs[sel]);
            fav_load();
            if (sel >= fav_count)
                sel = fav_count ? fav_count - 1 : 0;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B)
        {
            screen = S_MAIN_MENU;
            sel = MI_FAVORITES;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (A && fav_count)
        {
            scopyn(sel_path, favs[sel], sizeof(sel_path));
            screen = S_NONE;
            return ED64_ACT_LOAD_PATH;
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_CHEATS)
    {
        if (up)
        {
            ch_sel = (ch_sel % 17 == 0) ? ch_sel + 16 : ch_sel - 1;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (down)
        {
            ch_sel = (ch_sel % 17 == 16) ? ch_sel - 16 : ch_sel + 1;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (left || right)
        {
            ch_sel = (ch_sel + 17) % CH_MAX;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (A)
        {
            screen = S_CODE_MENU;
            sel = 0;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B)
        {
            if (ch_dirty)
            {
                screen = S_SAVE_CHEATS;
                sel = 0;
                redraw(disp);
                return ED64_ACT_NONE;
            }
            screen = ch_return;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_SAVE_CHEATS)
    {
        if (left || right || up || down)
        {
            sel ^= 1;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (A || B)
        {
            if (A && sel == 0 && cheats_save() != 0)
                return show_msg_here(disp, "Disk I/O error", NULL);
            ch_dirty = 0;
            screen = ch_return;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_CODE_MENU)
    {
        if (up)
        {
            sel = (sel + 3) % 4;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (down)
        {
            sel = (sel + 1) % 4;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B)
        {
            screen = S_CHEATS;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (A)
        {
            cheat_code_t *c = &ch_codes[ch_sel];
            switch (sel)
            {
                case 0:
                    snprintf(ce_hex, sizeof(ce_hex), "%08lX%04X", (unsigned long) c->addr, c->val);
                    ce_cur = 0;
                    ce_key = 0;
                    screen = S_CODE_EDIT;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case 1:
                    strncpy(de_text, c->desc, sizeof(de_text));
                    de_text[sizeof(de_text) - 1] = 0;
                    de_cur = (int) strlen(de_text);
                    de_key = 0;
                    screen = S_DESC_EDIT;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case 2:
                    cheats_clear(ch_sel);
                    screen = S_CHEATS;
                    redraw(disp);
                    return ED64_ACT_NONE;
                default:
                    c->on = !c->on;
                    ch_dirty = 1;
                    screen = S_CHEATS;
                    redraw(disp);
                    return ED64_ACT_NONE;
            }
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_CODE_EDIT)
    {
        if (up)
        {
            ce_key = (ce_key + 12) % 16;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (down)
        {
            ce_key = (ce_key + 4) % 16;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (left)
        {
            ce_key = (ce_key & ~3) | ((ce_key + 3) & 3);
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (right)
        {
            ce_key = (ce_key & ~3) | ((ce_key + 1) & 3);
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (keys->c[0].L)
        {
            if (ce_cur > 0)
                ce_cur--;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (keys->c[0].R)
        {
            if (ce_cur < 11)
                ce_cur++;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (A)
        {
            ce_hex[ce_cur] = "0123456789ABCDEF"[ce_key];
            if (ce_cur < 11)
                ce_cur++;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B || keys->c[0].start)
        {
            char tmp[9];
            memcpy(tmp, ce_hex, 8);
            tmp[8] = 0;
            u32 a = (u32) strtoul(tmp, NULL, 16);
            u16 v = (u16) strtoul(&ce_hex[8], NULL, 16);
            if ((a || v) && !cheats_code_supported(a))
            {
                screen = S_CODE_EDIT;
                return show_msg_here(disp, "Unsupported Code", NULL);
            }
            ch_codes[ch_sel].addr = a;
            ch_codes[ch_sel].val = v;
            ch_dirty = 1;
            screen = S_CHEATS;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }
    if (screen == S_DESC_EDIT)
    {
        int len = (int) strlen(de_text);
        if (up)
        {
            de_key = (de_key + 30) % 40;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (down)
        {
            de_key = (de_key + 10) % 40;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (left)
        {
            de_key = (de_key / 10) * 10 + (de_key % 10 + 9) % 10;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (right)
        {
            de_key = (de_key / 10) * 10 + (de_key % 10 + 1) % 10;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (keys->c[0].L)
        {
            if (de_cur > 0)
                de_cur--;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (keys->c[0].R)
        {
            if (de_cur < len && de_cur < CH_DESC_LEN - 2)
                de_cur++;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (keys->c[0].Z)
        {
            if (de_cur > 0)
            {
                memmove(de_text + de_cur - 1, de_text + de_cur, len - de_cur + 1);
                de_cur--;
            }
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (A)
        {
            if (de_cur < CH_DESC_LEN - 2)
            {
                if (de_cur == len)
                {
                    de_text[de_cur] = de_keys[de_key];
                    de_text[de_cur + 1] = 0;
                }
                else
                    de_text[de_cur] = de_keys[de_key];
                de_cur++;
            }
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (B || keys->c[0].start)
        {
            size_t e = strlen(de_text);
            while (e > 0 && de_text[e - 1] == ' ')
            {
                e--;
                de_text[e] = 0;
            }
            strncpy(ch_codes[ch_sel].desc, de_text, CH_DESC_LEN - 1);
            ch_codes[ch_sel].desc[CH_DESC_LEN - 1] = 0;
            ch_dirty = 1;
            screen = S_CHEATS;
            redraw(disp);
        }
        return ED64_ACT_NONE;
    }

    if (up && n)
    {
        sel = (sel + n - 1) % n;
        redraw(disp);
        return ED64_ACT_NONE;
    }
    if (down && n)
    {
        sel = (sel + 1) % n;
        redraw(disp);
        return ED64_ACT_NONE;
    }

    if (B)
    {
        switch (screen)
        {
            case S_RECENT:
                screen = S_MAIN_MENU;
                sel = MI_RECENT;
                redraw(disp);
                return ED64_ACT_NONE;
            case S_CPAK_MENU:
                screen = S_MAIN_MENU;
                sel = MI_CPAK;
                redraw(disp);
                return ED64_ACT_NONE;
            case S_CPAK_PORT:
                if (cpak_action >= CA_TO_FILE)
                {
                    screen = S_FILE_MENU;
                    redraw(disp);
                    return ED64_ACT_NONE;
                }
                screen = S_CPAK_MENU;
                sel = cpak_action;
                redraw(disp);
                return ED64_ACT_NONE;
            default:
                return close_or_refresh();
        }
    }

    if (A)
    {
        if (screen == S_FILE_MENU)
        {
            if (!file_enabled(sel))
            {
                return ED64_ACT_NONE;
            }
            scopyn(sel_path, cur_path, sizeof(sel_path));
            switch (fm->map[sel])
            {
                case FI_START:
                    screen = S_NONE;
                    return ED64_ACT_START;
                case FI_LOADCHEATS:
                {
                    char g[64];
                    if (!last_game_name(g, sizeof(g)))
                        return show_msg_here(disp, "No game selected", NULL);
                    int n = cheats_import(cur_path, g);
                    if (n < 0)
                        return show_msg_here(disp, "Disk I/O error", NULL);
                    char l2[40];
                    snprintf(l2, sizeof(l2), "%d codes -> %.24s", n, g);
                    return show_msg_here(disp, "Operation complete", l2);
                }
                case FI_ROMINFO:
                    load_rom_info();
                    screen = S_ROM_INFO;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_ROMCFG:
                    rc_load();
                    rc_sel = 0;
                    screen = S_ROM_CONFIG;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_HEX:
                    screen = S_HEX_VIEW;
                    hex_return = S_FILE_MENU;
                    hex_off = 0;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_DELETE:
                    screen = S_DELETE_CONFIRM;
                    sel = 1;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_PATCH:
                    screen = S_NONE;
                    return ED64_ACT_APPLY_PATCH;
                case FI_SETBG:
                    screen = S_NONE;
                    return ED64_ACT_SET_WALLPAPER;
                case FI_FILEINFO:
                    screen = S_FILE_INFO;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_FAVORITE:
                    fav_toggle(cur_path);
                    rom_menu_labels();
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_AUTOLOAD:
                    autoload_toggle(cur_path);
                    rom_menu_labels();
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_VIEWIMG:
                    screen = S_IMAGE;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_VIEWTXT:
                    if (!tv_open(cur_path))
                        return show_msg_here(disp, "File does not exist", NULL);
                    screen = S_TEXT;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_FILE2RAM:
                    screen = S_NONE;
                    return ED64_ACT_FILE_TO_RAM;
                case FI_RAM2FILE:
                    screen = S_NONE;
                    return ED64_ACT_RAM_TO_FILE;
                case FI_CPAK2FILE:
                    cpak_action = CA_TO_FILE;
                    screen = S_CPAK_PORT;
                    sel = cpak_port;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case FI_FILE2CPAK:
                    cpak_action = CA_FROM_FILE;
                    screen = S_CPAK_PORT;
                    sel = cpak_port;
                    redraw(disp);
                    return ED64_ACT_NONE;
                default:
                    return close_or_refresh();
            }
        }
        if (screen == S_MAIN_MENU)
        {
            switch (sel)
            {
                case MI_OPTIONS:
                    screen = S_OPTIONS;
                    sel = 0;
                    opt_first = 0;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case MI_RECENT:
                    recent_load();
                    screen = S_RECENT;
                    sel = 0;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case MI_FAVORITES:
                    fav_load();
                    screen = S_FAVORITES;
                    sel = 0;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case MI_SYSFOLDER:
                    screen = S_NONE;
                    return ED64_ACT_SYS_FOLDER;
                case MI_CPAK:
                    screen = S_CPAK_MENU;
                    sel = 0;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case MI_DEVICE:
                    screen = S_DEVICE_INFO;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case MI_ABOUT:
                    screen = S_ABOUT;
                    redraw(disp);
                    return ED64_ACT_NONE;
                default:
                    return close_or_refresh();
            }
        }
        if (screen == S_RECENT && recent_count)
        {
            scopyn(sel_path, recent[sel], sizeof(sel_path));
            screen = S_NONE;
            return ED64_ACT_LOAD_PATH;
        }
        if (screen == S_CPAK_MENU)
        {
            cpak_action = sel;
            screen = S_CPAK_PORT;
            sel = cpak_port;
            redraw(disp);
            return ED64_ACT_NONE;
        }
        if (screen == S_CPAK_PORT)
        {
            cpak_port = sel;
            switch (cpak_action)
            {
                case CA_FORMAT:
                    screen = S_CPAK_CONFIRM;
                    sel = 1;
                    redraw(disp);
                    return ED64_ACT_NONE;
                case CA_SAVE_NUM:
                    screen = S_NONE;
                    return ED64_ACT_CPAK_SAVE_NUM;
                case CA_SAVE_GAME:
                    screen = S_NONE;
                    return ED64_ACT_CPAK_SAVE_GAME;
                case CA_TO_FILE:
                    screen = S_NONE;
                    return ED64_ACT_CPAK_TO_FILE;
                case CA_FROM_FILE:
                    screen = S_NONE;
                    return ED64_ACT_FILE_TO_CPAK;
                default:
                    break;
            }
        }
    }
    return ED64_ACT_NONE;
}
