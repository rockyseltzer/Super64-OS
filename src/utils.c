#include "utils.h"

#include <string.h>

#include <libdragon.h>

#include "everdrive.h"
#include "main.h"
#include "save_db.h"
#include "sys.h"
#include "types.h"

static int get_cic_save_alt64(char *cartid, int *cic, int *save);
#include "ff.h"

#include "cic.h"
#include "rom.h"
#include "sram.h"

static int setSRAM768(uint8_t *buffer);
static int setSRAM32(uint8_t *buffer);
static int setSRAM128(uint8_t *buffer);
static void setSDTiming(void);
static int setFlashRAM(uint8_t *buffer);
static int setEeprom4k(uint8_t *buffer);
static int setEeprom16k(uint8_t *buffer);
static int getSRAM768(uint8_t *buffer);
static int getSRAM32(uint8_t *buffer);
static int getSRAM128(uint8_t *buffer);
static int getFlashRAM(uint8_t *buffer);
static int getEeprom4k(uint8_t *buffer);
static int getEeprom16k(uint8_t *buffer);

static u8 __attribute__((aligned(16))) dmaBuf[128 * 1024];
static volatile struct _PI_regs_s *const _PI_regs = (struct _PI_regs_s *) 0xa4600000;

int get_cic_save(const unsigned char *hdr, int *cic, int *save)
{
    char cartid[3] = {hdr[1], hdr[2], 0};
    int db_save, db_cpak;
    int detected = *cic;
    int db_hit = save_db_lookup(hdr, hdr[4], &db_save, &db_cpak);
    int old_hit = get_cic_save_alt64(cartid, cic, save);
    if (detected)
        *cic = detected;
    else if (!old_hit)
        *cic = 2;
    if (db_hit)
    {
        *save = db_save;
        return 1;
    }
    return old_hit;
}

static int get_cic_save_alt64(char *cartid, int *cic, int *save)
{
    int NUM_CARTS = 179;
    int i;

    char
        *cartIDs[] =
            {
                "DZ", "B6", "ZY", "ZZ", "AD", "AL", "B7", "BC", "BD", "BH", "BK", "BM", "BV", "CC", "CH", "CK", "CR",
                "CT", "CU", "CW", "DL", "DO", "DP", "DQ", "DU", "DY", "EA", "EP", "ER", "FH", "FU", "FW", "FX", "FZ",
                "GC", "GE", "GV", "HA", "IC", "IJ", "JD", "JF", "JM", "K2", "K4", "KA", "KG", "KI", "KJ", "KT", "LB",
                "LR", "M6", "M8", "MF", "MI", "ML", "MO", "MQ", "MR", "MU", "MV", "MW", "MX", "N6", "NA", "NB", "NX",
                "OB", "P2", "P3", "PD", "PF", "PG", "PH", "PN", "PO", "PS", "PW", "RC", "RE", "RI", "RS", "RZ", "S6",
                "SA", "SC", "SM", "SQ", "SU", "SV", "SW", "T9", "TE", "TJ", "TM", "TN", "TP", "VL", "VY", "W2", "W4",
                "WL", "WR", "WU", "WX", "XO", "YS", "YW", "ZL", "ZS", "AB", "BN", "CG", "CX", "CZ", "D6", "DR", "DZ",
                "OH", "TB", "TC", "VB", "WI", "4W", "AG", "AY", "DA", "D2", "3D", "F2", "SI", "HP", "EV", "MG", "GU",
                "SA", "VP", "A2", "WC", "GF", "TR", "TX", "CA", "HB", "SE", "DD", "DD", "DD", "DD", "DD", "DD", "DD",
                "DD", "AF", "PT", "PY", "MZ", "GL", "JG", "SB", "PM", "R7", "OS", "O2", "HF", "FG", "G6", "IM", "IB",
                "LL", "MS", "UB", "RH", "T3", "T6", "IR", "GP", "X7"
    };

    int saveTypes[] = {
        0x02, 0x03, 0x05, 0x01, 0x03, 0x01, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x05, 0x03, 0x05, 0x03, 0x03,
        0x03, 0x04, 0x05, 0x04, 0x05, 0x03, 0x03, 0x03, 0x03, 0x04, 0x03, 0x03, 0x04, 0x03, 0x03, 0x01, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x05, 0x05, 0x03, 0x03, 0x03, 0x03, 0x01, 0x03, 0x05, 0x03, 0x03, 0x03, 0x05, 0x04,
        0x01, 0x03, 0x03, 0x03, 0x05, 0x03, 0x03, 0x04, 0x03, 0x04, 0x03, 0x03, 0x04, 0x04, 0x01, 0x05, 0x05, 0x04,
        0x05, 0x03, 0x05, 0x05, 0x05, 0x05, 0x03, 0x03, 0x01, 0x01, 0x03, 0x04, 0x03, 0x03, 0x03, 0x03, 0x05, 0x03,
        0x03, 0x03, 0x05, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x05, 0x03, 0x03, 0x03, 0x01, 0x03, 0x04,
        0x01, 0x01, 0x05, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x03, 0x01, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x03,
        0x02, 0x02, 0x04, 0x04, 0x04, 0x02, 0x03, 0x04, 0x03, 0x03, 0x03, 0x01, 0x01, 0x03, 0x03, 0x03, 0x03, 0X03,
        0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x05, 0x03, 0x03, 0x03, 0x03, 0x02, 0x03, 0x03,
        0x04, 0X03, 0X03, 0x03, 0x03, 0x02, 0x04, 0x02, 0x03, 0X03, 0x04, 0x05, 0x02, 0x03, 0x03, 0x02, 0x04};

    int cicTypes[] = {
        0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x05, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x06, 0x05, 0x05, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x05, 0x02, 0x01, 0x06, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x05, 0x05, 0x02, 0x02, 0x03, 0x02, 0x03, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x05, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x03, 0x02, 0x03, 0x03, 0x05,
        0x03, 0x02, 0x03, 0x02, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x06,
        0x02, 0x05, 0x05, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0X04,
        0X04, 0X04, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x02, 0x02, 0x07, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0X02, 0X02, 0X02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

    for (i = 0; i < NUM_CARTS; i++)
        if (strcmp(cartid, cartIDs[i]) == 0)
            break;

    if (i == NUM_CARTS)
    {
        *cic = 2;
        *save = 0;
        return 0;
    }

    *cic = cicTypes[i];
    *save = saveTypes[i];

    return 1;
}

const char *saveTypeToExtension(int type, int etype)
{
    static char *str = "SAV";

    if (etype == 0)
    {
        switch (type)
        {
            case 0:
                str = "OFF";
                break;
            case 1:
                str = "srm";
                break;
            case 2:
                str = "128";
                break;
            case 3:
                str = "e4k";
                break;
            case 4:
                str = "e16";
                break;
            case 5:
                str = "fla";
                break;
            case 6:
                str = "768";
                break;
            default:
                str = "sav";
        }
    }
    else
    {
        switch (type)
        {
            case 0:
                str = "OFF";
                break;
            case 1:
                str = "sra";
                break;
            case 2:
                str = "sra";
                break;
            case 3:
                str = "eep";
                break;
            case 4:
                str = "eep";
                break;
            case 5:
                str = "fla";
                break;
            case 6:
                str = "sra";
                break;
            default:
                str = "sav";
        }
    }

    return str;
}

int saveTypeToSize(int type)
{
    switch (type)
    {
        case 0:
            return 0;
            break;
        case 1:
            return SAVE_SIZE_SRAM;
            break;
        case 2:
            return SAVE_SIZE_SRAM128;
            break;
        case 3:
            return SAVE_SIZE_EEP4k;
            break;
        case 4:
            return SAVE_SIZE_EEP16k;
            break;
        case 5:
            return SAVE_SIZE_FLASH;
            break;
        case 6:
            return SAVE_SIZE_SRAM768;
            break;
        default:
            return 0;
    }
}

int getSaveFromCart(int stype, uint8_t *buffer)
{
    int ret = 0;

    switch (stype)
    {
        case 0:
            return 0;
        case 1:
            ret = getSRAM32(buffer);
            break;
        case 2:
            ret = getSRAM128(buffer);
            break;
        case 3:
            ret = getEeprom4k(buffer);
            break;
        case 4:
            ret = getEeprom16k(buffer);
            break;
        case 5:
            ret = getFlashRAM(buffer);
            break;
        case 6:
            ret = getSRAM768(buffer);
            break;
        default:
            return 0;
    }

    return ret;
}

int pushSaveToCart(int stype, uint8_t *buffer)
{
    int ret = 0;

    switch (stype)
    {
        case 0:
            return 0;
        case 1:
            ret = setSRAM32(buffer);
            break;
        case 2:
            ret = setSRAM128(buffer);
            break;
        case 3:
            ret = setEeprom4k(buffer);
            break;
        case 4:
            ret = setEeprom16k(buffer);
            break;
        case 5:
            ret = setFlashRAM(buffer);
            break;
        case 6:
            ret = setSRAM768(buffer);
            break;
        default:
            return 0;
    }

    return ret;
}

#define SRAM_WINDOW_OFFSET 0

#define SRAM_BLK   512
#define SRAM_TRIES 8
static u8 sram_blk_a[SRAM_BLK] __attribute__((aligned(16)));
static u8 sram_blk_b[SRAM_BLK] __attribute__((aligned(16)));
int sram_last_bad = 0;
int sram_last_pass = 0;

static void sram_timing_save(void)
{
    while (dma_busy())
        ;
    IO_WRITE(PI_BSD_DOM2_LAT_REG, 0x05);
    IO_WRITE(PI_BSD_DOM2_PWD_REG, 0x0C);
    IO_WRITE(PI_BSD_DOM2_PGS_REG, 0x0D);
    IO_WRITE(PI_BSD_DOM2_RLS_REG, 0x02);
}
static void sram_timing_restore(void)
{
    while (dma_busy())
        ;
    IO_WRITE(PI_BSD_DOM2_LAT_REG, 0x40);
    IO_WRITE(PI_BSD_DOM2_PWD_REG, 0x12);
    IO_WRITE(PI_BSD_DOM2_PGS_REG, 0x07);
    IO_WRITE(PI_BSD_DOM2_RLS_REG, 0x03);
}
static void sram_blk_read(u8 *dst, u32 off)
{
    data_cache_hit_writeback_invalidate(dst, SRAM_BLK);
    while (dma_busy())
        ;
    IO_WRITE(PI_STATUS_REG, 3);
    IO_WRITE(PI_DRAM_ADDR_REG, (u32) dst & 0x1FFFFFFF);
    IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + off) & 0x1FFFFFFF);
    IO_WRITE(PI_WR_LEN_REG, SRAM_BLK - 1);
    while (dma_busy())
        ;
    data_cache_hit_invalidate(dst, SRAM_BLK);
}
static void sram_blk_write(const u8 *src, u32 off)
{
    data_cache_hit_writeback((void *) src, SRAM_BLK);
    while (dma_busy())
        ;
    IO_WRITE(PI_STATUS_REG, 3);
    IO_WRITE(PI_DRAM_ADDR_REG, (u32) src & 0x1FFFFFFF);
    IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + off) & 0x1FFFFFFF);
    IO_WRITE(PI_RD_LEN_REG, SRAM_BLK - 1);
    while (dma_busy())
        ;
}
static int sram_blk_read_stable(u8 *dst, u32 off)
{
    sram_blk_read(sram_blk_a, off);
    for (int t = 0; t < SRAM_TRIES; t++)
    {
        sram_blk_read(sram_blk_b, off);
        if (memcmp(sram_blk_a, sram_blk_b, SRAM_BLK) == 0)
        {
            memcpy(dst, sram_blk_a, SRAM_BLK);
            return 1;
        }
        memcpy(sram_blk_a, sram_blk_b, SRAM_BLK);
    }
    memcpy(dst, sram_blk_a, SRAM_BLK);
    return 0;
}

static u32 save_rom_addr(int size)
{
    return ROM_ADDR + ROM_LEN - (u32) size;
}

int getSRAM(uint8_t *buffer, int size)
{
    static u8 a[2048] __attribute__((aligned(16))), b2[2048] __attribute__((aligned(16)));
    u32 base = save_rom_addr(size);
    sram_last_bad = 0;
    evd_setCfgBit(ED_CFG_SDRAM_ON, 1);
    for (int off = 0; off < size; off += 2048)
    {
        int n = (size - off < 2048) ? size - off : 2048, ok = 0;
        dma_read_s(a, base + off, n);
        for (int t = 0; t < SRAM_TRIES && !ok; t++)
        {
            dma_read_s(b2, base + off, n);
            ok = memcmp(a, b2, n) == 0;
            if (!ok)
                memcpy(a, b2, n);
        }
        if (!ok)
            sram_last_bad++;
        memcpy(buffer + off, a, n);
    }
    return 1;
}

static int getSRAM32(uint8_t *buffer)
{
    return getSRAM(buffer, SAVE_SIZE_SRAM);
}

static int getSRAM128(uint8_t *buffer)
{
    return getSRAM(buffer, SAVE_SIZE_SRAM128);
}

static int getSRAM768(uint8_t *buffer)
{
    return getSRAM(buffer, SAVE_SIZE_SRAM768);
}

static int getEeprom4k(uint8_t *buffer)
{
    if (eeprom_present())
    {
        int blocks = SAVE_SIZE_EEP4k / 8;
        for (int b = 0; b < blocks; b++)
        {
            eeprom_read(b, &buffer[b * 8]);
        }
        return 1;
    }

    return 0;
}

static int getEeprom16k(uint8_t *buffer)
{
    int blocks = SAVE_SIZE_EEP16k / 8;
    for (int b = 0; b < blocks; b++)
    {
        eeprom_read(b, &buffer[b * 8]);
    }

    return 1;
}

static int getFlashRAM(uint8_t *buffer)
{
    evd_setSaveType(SAVE_TYPE_SRAM128);
    sleep(FF_MAX_SS);

    getSRAM(buffer, SAVE_SIZE_SRAM128);
    data_cache_hit_writeback_invalidate(buffer, SAVE_SIZE_SRAM128);

    sleep(FF_MAX_SS);
    evd_setSaveType(SAVE_TYPE_FLASH);

    return 1;
}

int setSRAM(uint8_t *buffer, int size)
{
    static u8 src[2048] __attribute__((aligned(16))), chk[2048] __attribute__((aligned(16)));
    u32 base = save_rom_addr(size);
    sram_last_bad = 0;
    sram_last_pass = 0;
    evd_setCfgBit(ED_CFG_SDRAM_ON, 1);
    for (int off = 0; off < size; off += 2048)
    {
        int n = (size - off < 2048) ? size - off : 2048, ok = 0;
        memcpy(src, buffer + off, n);
        for (int t = 0; t < SRAM_TRIES && !ok; t++)
        {
            dma_write_s(src, base + off, n);
            dma_read_s(chk, base + off, n);
            ok = memcmp(chk, src, n) == 0;
            if (t)
                sram_last_pass++;
        }
        if (!ok)
            sram_last_bad++;
    }
    setSDTiming();
    return sram_last_bad == 0;
}

static int setSRAM32(uint8_t *buffer)
{
    return setSRAM(buffer, SAVE_SIZE_SRAM);
}

static int setSRAM128(uint8_t *buffer)
{
    return setSRAM(buffer, SAVE_SIZE_SRAM128);
}

static int setSRAM768(uint8_t *buffer)
{
    return setSRAM(buffer, SAVE_SIZE_SRAM768);
}

static int setEeprom4k(uint8_t *buffer)
{
    if (eeprom_present())
    {
        int blocks = SAVE_SIZE_EEP4k / 8;
        for (int b = 0; b < blocks; b++)
        {
            eeprom_write(b, &buffer[b * 8]);
        }
        return 1;
    }

    return 0;
}

static int setEeprom16k(uint8_t *buffer)
{
    int blocks = SAVE_SIZE_EEP16k / 8;
    for (int b = 0; b < blocks; b++)
    {
        eeprom_write(b, &buffer[b * 8]);
    }

    return 1;
}

static int setFlashRAM(uint8_t *buffer)
{
    evd_setSaveType(SAVE_TYPE_SRAM128);
    sleep(FF_MAX_SS);

    setSRAM(buffer, SAVE_SIZE_SRAM128);
    data_cache_hit_writeback_invalidate(buffer, SAVE_SIZE_SRAM128);

    sleep(FF_MAX_SS);
    evd_setSaveType(SAVE_TYPE_FLASH);

    return 1;
}

static void setSDTiming(void)
{
    IO_WRITE(PI_BSD_DOM1_LAT_REG, 0x40);
    IO_WRITE(PI_BSD_DOM1_PWD_REG, 0x12);
    IO_WRITE(PI_BSD_DOM1_PGS_REG, 0x07);
    IO_WRITE(PI_BSD_DOM1_RLS_REG, 0x03);

    IO_WRITE(PI_BSD_DOM2_LAT_REG, 0x40);
    IO_WRITE(PI_BSD_DOM2_PWD_REG, 0x12);
    IO_WRITE(PI_BSD_DOM2_PGS_REG, 0x07);
    IO_WRITE(PI_BSD_DOM2_RLS_REG, 0x03);
}

static void _data_cache_invalidate_all(void)
{
    asm("li $8,0x80000000;"
        "li $9,0x80000000;"
        "addu $9,$9,0x1FF0;"
        "cacheloop:;"
        "cache 1,0($8);"
        "cache 1,16($8);"
        "cache 1,32($8);"
        "cache 1,48($8);"
        "cache 1,64($8);"
        "cache 1,80($8);"
        "cache 1,96($8);"
        "addu $8,$8,112;"
        "bne $8,$9,cacheloop;"
        "cache 1,0($8);"
        :
        :
        : "$8", "$9"
    );
}

#define ROM        ((vu32 *) 0xB0000000)
#define EXE_START  0xB0001000
#define EXE_SIZE   0x00100004
#define TV_TYPE    *(vu32 *) 0x80000300
#define RAM_SIZE_1 *(vu32 *) 0x80000318
#define RAM_SIZE_2 *(vu32 *) 0x800003F0

void simulate_boot(u32 cic_chip, u8 gBootCic, u32 *cheat_lists[2])
{
    IO_WRITE(VI_V_INT, 0x3FF);
    IO_WRITE(VI_H_LIMITS, 0);
    IO_WRITE(VI_CUR_LINE, 0);

    IO_WRITE(PI_STATUS_REG, 0x03);

    u32 lat = ROM[0];
    IO_WRITE(PI_BSD_DOM1_LAT_REG, lat & 0xFF);
    IO_WRITE(PI_BSD_DOM1_PWD_REG, lat >> 8);
    IO_WRITE(PI_BSD_DOM1_PGS_REG, lat >> 16);
    IO_WRITE(PI_BSD_DOM1_RLS_REG, lat >> 20);

    vu32 *ram_size = (cic_chip == CIC_6105) ? &RAM_SIZE_2 : &RAM_SIZE_1;
    *ram_size = (gBootCic == CIC_6105) ? RAM_SIZE_2 : RAM_SIZE_1;

    if (force_tv)
    {
        TV_TYPE = MIN((~force_tv - 1) & 3, 2);
    }

    if (gCheats)
    {
        void *patcher = (void *) 0x80700000;
        u32 patcher_length = &&patcher_end - &&patcher_start;
        memcpy(patcher, &&patcher_start, patcher_length);

        u32 *p = patcher + patcher_length;
        *(u32 **) 0xA06FFFFC = p;
        for (int i = 0; i < 2; i++)
        {
            int j = -2;
            do
            {
                j += 2;
                patcher_length += 8;

                *p++ = cheat_lists[i][j];
                *p++ = cheat_lists[i][j + 1];
            } while (cheat_lists[i][j] || cheat_lists[i][j + 1]);
        }

        data_cache_hit_writeback(patcher, patcher_length);
        inst_cache_hit_invalidate(patcher, patcher_length);
    }

    asm __volatile__ (
        ".set noreorder;"

        "lui    $t0, 0x8000;"

        "move   $s3, $zero;"
        "lw     $s4, 0x0300($t0);"
        "move   $s5, $zero;"
        "lui    $s6, %%hi(cic_ids);"
        "addu   $s6, $s6, %0;"
        "lbu    $s6, %%lo(cic_ids)($s6);"
        "lw     $s7, 0x0314($t0);"

        "lui    $a0, 0xA400;"
        "lui    $a1, %%hi(imem_start);"
        "ori    $a2, $zero, 0x0008;"
    "1:"
        "lw     $t0, %%lo(imem_start)($a1);"
        "addiu  $a1, $a1, 4;"
        "sw     $t0, 0x1000($a0);"
        "addiu  $a2, $a2, -1;"
        "bnez   $a2, 1b;"
        "addiu  $a0, $a0, 4;"

        "lui    $t3, 0xA400;"
        "ori    $t3, $t3, 0x0040;"
        "move   $a0, $t3;"
        "lui    $a1, 0xB000;"
        "ori    $a2, 0x0FC0;"
    "1:"
        "lw     $t0, 0x0040($a1);"
        "addiu  $a1, $a1, 4;"
        "sw     $t0, 0x0000($a0);"
        "addiu  $a2, $a2, -4;"
        "bnez   $a2, 1b;"
        "addiu  $a0, $a0, 4;"

        "beqz   %1, 2f;"

        "lui    $a1, %%hi(cic_patch_offsets);"
        "addu   $a1, $a1, %0;"
        "lbu    $a1, %%lo(cic_patch_offsets)($a1);"
        "addu   $a0, $t3, $a1;"
        "lui    $a1, 0x081C;"
        "ori    $a2, $zero, 0x06;"
        "bne    %0, $a2, 1f;"
        "lui    $a2, 0x8188;"
        "ori    $a2, $a2, 0x764A;"
        "xor    $a1, $a1, $a2;"
    "1:"
        "sw     $a1, 0x0700($a0);"

        "ori    $a2, $zero, 0x05;"
        "beql   %0, $a2, 2f;"
        "sw     $zero, 0x06CC($a0);"

    "2:"
        "lui    $sp, 0xA400;"
        "ori    $ra, $sp, 0x1550;"
        "jr     $t3;"
        "ori    $sp, $sp, 0x1FF0;"

    "cic_ids:"
        ".byte  0x00;"
        ".byte  0x3F;"
        ".byte  0x3F;"
        ".byte  0x78;"
        ".byte  0xAC;"
        ".byte  0x91;"
        ".byte  0x85;"
        ".byte  0xDD;"

    "cic_patch_offsets:"
        ".byte  0x00;"
        ".byte  0x30;"
        ".byte  0x2C;"
        ".byte  0x20;"
        ".byte  0x30;"
        ".byte  0x8C;"
        ".byte  0x60;"
        ".byte  0x30;"

    "imem_start:"
        "lui    $t5, 0xBFC0;"
    "1:"
        "lw     $t0, 0x07FC($t5);"
        "addiu  $t5, $t5, 0x07C0;"
        "andi   $t0, $t0, 0x0080;"
        "bnezl  $t0, 1b;"
        "lui    $t5, 0xBFC0;"
        "lw     $t0, 0x0024($t5);"
        "lui    $t3, 0xB000;"

        :
        : "r" (cic_chip),
          "r" (gCheats)
        : "$4", "$5", "$6", "$8",
          "$11", "$19", "$20", "$21",
          "$22", "$23", "memory"
    );

patcher_start:
    asm __volatile__ (
        ".set noat;"
        ".set noreorder;"

    "patcher:"
        "lui    $t5, 0x8070;"
        "lui    $t6, 0x8000;"
        "li     $t7, 0x007FFFFF;"
        "li     $t8, 0x807C5C00;"
        "addiu  $t9, $t5, -4;"
        "lw     $t9, 0x0000($t9);"

    "1:"
        "lw     $v0, 0x0000($t9);"
        "bnez   $v0, 2f;"
        "lw     $v1, 0x0004($t9);"
        "beqz   $v1, 1f;"

    "2:"
        "addiu  $t9, $t9, 0x0008;"
        "srl    $t2, $v0, 24;"
        "li     $at, 0xEE;"
        "beq    $t2, $at, 5f;"
        "li     $at, 0xF0;"
        "and    $v0, $v0, $t7;"
        "beq    $t2, $at, 4f;"
        "or     $v0, $v0, $t6;"
        "li     $at, 0xF1;"
        "beq    $t2, $at, 3f;"
        "nop;"

        "li     $at, 0xFFFFFFFC;"
        "b      1b;"
        "and    $t8, $v0, $at;"

    "3:"
        "b      1b;"
        "sh     $v1, 0x0000($v0);"

    "4:"
        "b      1b;"
        "sb     $v1, 0x0000($v0);"

    "5:"
        "lui    $v0, 0x0040;"
        "sw     $v0, 0x0318($t6);"
        "b      1b;"
        "sw     $v0, 0x03F0($t6);"

    "1:"
        "srl    $at, $t8, 2;"
        "and    $v0, $at, $t7;"
        "lui    $at, 0x0800;"
        "or     $v0, $v0, $at;"
        "sw     $v0, 0x0180($t6);"
        "sw     $zero, 0x0184($t6);"

        "sw     $t8, 0x0188($t6);"
        "la     $at, %%lo(patcher);"
        "la     $v0, %%lo(code_engine_start);"
        "la     $v1, %%lo(code_engine_end);"
        "subu   $at, $v0, $at;"
        "subu   $v1, $v1, $v0;"
        "addu   $v0, $t5, $at;"
    "1:"
        "lw     $at, 0x0000($v0);"
        "addiu  $v1, $v1, -4;"
        "sw     $at, 0x0000($t8);"
        "addiu  $v0, $v0, 4;"
        "bgtz   $v1, 1b;"
        "addiu  $t8, $t8, 4;"
        "sw     $t8, 0x018C($t6);"

    "1:"
        "lw     $v0, 0x0000($t9);"
        "lw     $v1, 0x0004($t9);"
        "addiu  $t9, $t9, 8;"
        "sw     $v0, 0x0000($t8);"
        "sw     $v1, 0x0004($t8);"
        "bnez   $v0, 1b;"
        "addiu  $t8, $t8, 8;"
        "bnez   $v1, 1b;"
        "nop;"

        "ori    $t0, $t6, 0x0180;"
        "li     $at, 0x0010;"
    "1:"
        "cache  0x19, 0x0000($t0);"
        "cache  0x10, 0x0000($t0);"
        "addiu  $at, $at, -4;"
        "bnez   $at, 1b;"
        "addiu  $t0, $t0, 4;"

        "lw     $t0, 0x0188($t6);"
        "subu   $at, $t8, $t0;"
    "1:"
        "cache  0x19, 0x0000($t0);"
        "cache  0x10, 0x0000($t0);"
        "addiu  $at, $at, -4;"
        "bnez   $at, 1b;"
        "addiu  $t0, $t0, 4;"

        "li     $t0, 0x0181;"
        "mtc0   $t0, $18;"
        "nop;"
        "mtc0   $zero, $19;"

        "jr     $t1;"
        "nop;"

    "code_engine_start:"
        "mfc0   $k0, $13;"
        "andi   $k1, $k0, 0x1000;"
        "bnezl  $k1, 1f;"
        "mtc0   $zero, $18;"
    "1:"

        "andi   $k0, $k0, 0x7C;"
        "li     $k1, 0x5C;"
        "bne    $k0, $k1, 1f;"

        "mfc0   $k1, $14;"
        "lw     $k1, 0x0000($k1);"
        "lui    $k0, 0x03E0;"
        "and    $k1, $k1, $k0;"
        "srl    $k1, $k1, 5;"
        "lui    $k0, 0x3740;"
        "or     $k1, $k1, $k0;"
        "ori    $k1, $k1, 0x0120;"
        "lui    $k0, 0x8000;"
        "lw     $k0, 0x0188($k0);"
        "sw     $k1, 0x0060($k0);"
        "cache  0x19, 0x0060($k0);"
        "cache  0x10, 0x0060($k0);"
        "lui    $k0, 0x8000;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        "eret;"

    "1:"
        "lui    $k0, 0x8000;"
        "lw     $k0, 0x0188($k0);"
        "addiu  $k0, $k0, -0x38;"
        "sd     $v1, 0x0000($k0);"
        "sd     $v0, 0x0008($k0);"
        "sd     $t9, 0x0010($k0);"
        "sd     $t8, 0x0018($k0);"
        "sd     $t7, 0x0020($k0);"
        "sd     $t6, 0x0028($k0);"
        "sd     $t5, 0x0030($k0);"

        "lui    $t9, 0x8000;"
        "lw     $t9, 0x018C($t9);"
    "1:"
        "lw     $v0, 0x0000($t9);"
        "bnez   $v0, 2f;"
        "lw     $v1, 0x0004($t9);"
        "beqz   $v1, 4f;"
        "nop;"

        "b      1b;"

    "2:"
        "addiu  $t9, $t9, 0x0008;"
        "srl    $t7, $v0, 24;"
        "sltiu  $k1, $t7, 0xD0;"
        "sltiu  $t8, $t7, 0xD4;"
        "xor    $k1, $k1, $t8;"
        "li     $t8, 0x50;"
        "bne    $t7, $t8, 3f;"

        "srl    $t8, $v0, 8;"
        "andi   $t8, $t8, 0x00FF;"
        "andi   $t7, $v0, 0x00FF;"
        "lw     $v0, 0x0000($t9);"
        "lw     $k1, 0x0004($t9);"
        "addiu  $t9, $t9, 0x0008;"

        "lui    $t6, 0x0100;"
        "and    $t6, $v0, $t6;"
        "li     $t5, 0xA07FFFFF;"
        "and    $v0, $v0, $t5;"
        "lui    $t5, 0x8000;"
        "or     $v0, $v0, $t5;"

    "2:"
        "beqzl  $t6, 5f;"
        "sb     $k1, 0x0000($v0);"
        "sh     $k1, 0x0000($v0);"
    "5:"
        "addiu  $t8, $t8, -1;"
        "addu   $v0, $v0, $t7;"
        "bnez   $t8, 2b;"
        "addu   $k1, $k1, $v1;"
        "b      1b;"

    "3:"
        "lui    $t7, 0x0300;"
        "and    $t7, $v0, $t7;"
        "li     $t8, 0xA07FFFFF;"
        "and    $v0, $v0, $t8;"
        "lui    $t8, 0x8000;"
        "beqz   $k1, 2f;"
        "or     $v0, $v0, $t8;"

        "sll    $k1, $t7, 7;"
        "beqzl  $k1, 3f;"
        "lbu    $t8, 0x0000($v0);"
        "lhu    $t8, 0x0000($v0);"
    "3:"
        "srl    $t7, $t7, 22;"
        "andi   $t7, $t7, 8;"
        "beql   $v1, $t8, 1b;"
        "add    $t9, $t9, $t7;"
        "xori   $t7, $t7, 8;"
        "b      1b;"
        "add    $t9, $t9, $t7;"

    "2:"
        "sll    $k1, $t7, 7;"
        "beqzl  $k1, 3f;"
        "sb     $v1, 0x0000($v0);"
        "sh     $v1, 0x0000($v0);"
    "3:"
        "b      1b;"

    "4:"
        "ld     $t5, 0x0030($k0);"
        "ld     $t6, 0x0028($k0);"
        "ld     $t7, 0x0020($k0);"
        "ld     $t8, 0x0018($k0);"
        "ld     $t9, 0x0010($k0);"
        "ld     $v0, 0x0008($k0);"
        "j      0x80000120;"
        "ld     $v1, 0x0000($k0);"
    "code_engine_end:"

        :
        :
        : "memory"
    );
patcher_end:

    return;
}
