//
// Super 64 board diagnostics
//

#include "diag.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libdragon.h>

#include "ff.h"

#include "constants.h"
#include "everdrive.h"
#include "main.h"
#include "mem.h"
#include "memorypak.h"
#include "sd.h"
#include "sys.h"
#include "utils.h"

char diag_lines[DIAG_MAX_LINES][DIAG_LINE_LEN];
int diag_nlines = 0;
int diag_failed = 0;
static int diag_warn = 0;

u8 diskCmdSPI(u8 cmd, u32 arg);

#define PI_STATUS   (*(volatile u32 *) 0xA4600010)
#define PI_DOM1_LAT (*(volatile u32 *) 0xA4600014)
#define PI_DOM1_PWD (*(volatile u32 *) 0xA4600018)
#define PI_DOM1_PGS (*(volatile u32 *) 0xA460001C)
#define PI_DOM1_RLS (*(volatile u32 *) 0xA4600020)
#define PI_DOM2_LAT (*(volatile u32 *) 0xA4600024)
#define PI_DOM2_PWD (*(volatile u32 *) 0xA4600028)
#define PI_DOM2_PGS (*(volatile u32 *) 0xA460002C)
#define PI_DOM2_RLS (*(volatile u32 *) 0xA4600030)

#define CHUNK 65536
static u8 buf[CHUNK] __attribute__((aligned(16)));
static u8 buf2[CHUNK] __attribute__((aligned(16)));

static void add(const char *fmt, ...)
{
    if (diag_nlines >= DIAG_MAX_LINES)
        return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(diag_lines[diag_nlines++], DIAG_LINE_LEN, fmt, ap);
    va_end(ap);
}
static u32 ms_since(u32 t0)
{
    return (u32) (get_ticks() - t0) / (COUNTS_PER_SECOND / 1000);
}
static u32 kbps(u32 bytes, u32 ms)
{
    if (!ms)
        ms = 1;
    return (bytes >> 10) * 1000 / ms;
}

static void cart_read(void *dst, u32 pi, u32 len)
{
    data_cache_hit_writeback_invalidate(dst, len);
    dma_read(dst, pi, len);
    data_cache_hit_invalidate(dst, len);
}
static void cart_write(const void *src, u32 pi, u32 len)
{
    data_cache_hit_writeback((void *) src, len);
    dma_write((void *) src, pi, len);
}
static u32 sum32(const u8 *p, u32 n, u32 acc)
{
    const u32 *w = (const u32 *) p;
    for (u32 i = 0; i < n / 4; i++)
        acc = (acc << 1 | acc >> 31) ^ w[i];
    return acc;
}
static inline u32 pattern(u32 seed, u32 addr)
{
    return (addr * 0x9E3779B1u) ^ seed ^ (addr >> 13);
}

#define PI_DRAM_ADDR (*(volatile u32 *) 0xA4600000)
#define PI_CART_ADDR (*(volatile u32 *) 0xA4600004)
#define PI_RD_LEN    (*(volatile u32 *) 0xA4600008)
#define PI_WR_LEN    (*(volatile u32 *) 0xA460000C)
static void pi_wait(void)
{
    while (PI_STATUS & 3)
        ;
}
static void pi_read_raw(void *dst, u32 pi, u32 len)
{
    data_cache_hit_writeback_invalidate(dst, len);
    pi_wait();
    PI_STATUS = 3;
    PI_DRAM_ADDR = (u32) dst & 0x1FFFFFFF;
    PI_CART_ADDR = pi & 0x1FFFFFFF;
    PI_WR_LEN = len - 1;
    pi_wait();
    data_cache_hit_invalidate(dst, len);
}
static void pi_write_raw(const void *src, u32 pi, u32 len)
{
    data_cache_hit_writeback((void *) src, len);
    pi_wait();
    PI_STATUS = 3;
    PI_DRAM_ADDR = (u32) src & 0x1FFFFFFF;
    PI_CART_ADDR = pi & 0x1FFFFFFF;
    PI_RD_LEN = len - 1;
    pi_wait();
}

// ---------------------------------------------------------------- 1. Board ---
static void sec_board(void)
{
    u16 firm = evd_readReg(REG_VER), cfg = evd_readReg(REG_CFG), st = evd_readReg(REG_STATUS),
        mm = evd_readReg(REG_MAX_MSG);
    u32 mem = *(volatile u32 *) 0x80000318, tv = *(volatile u32 *) 0x80000300;
    add("[board]");
    add("firmware v%x.%02x  RAM %luMB  TV %s", firm >> 8, firm & 0xff, (unsigned long) (mem >> 20),
        tv == 1   ? "NTSC"
        : tv == 0 ? "PAL"
                  : "MPAL");
    add("CFG=%04x STATUS=%04x MAXMSG=%04x", cfg, st, mm);
    add("status: dma%s tout%s txe%s rxf%s spi%s", (st & 1) ? "+" : "-", (st & 2) ? "+" : "-", (st & 4) ? "+" : "-",
        (st & 8) ? "+" : "-", (st & 16) ? "+" : "-");
    add("regs 0-11:");
    for (int r = 0; r < 12; r += 4)
        add(" %04x %04x %04x %04x", evd_readReg(r), evd_readReg(r + 1), evd_readReg(r + 2), evd_readReg(r + 3));
    add(" r16-19 %04x %04x %04x %04x", evd_readReg(16), evd_readReg(17), evd_readReg(18), evd_readReg(19));
    add("PI dom1 LAT %02lx PWD %02lx PGS %lx RLS %lx", (unsigned long) (PI_DOM1_LAT & 0xff),
        (unsigned long) (PI_DOM1_PWD & 0xff), (unsigned long) (PI_DOM1_PGS & 0xf), (unsigned long) (PI_DOM1_RLS & 3));
    add("PI dom2 LAT %02lx PWD %02lx PGS %lx RLS %lx", (unsigned long) (PI_DOM2_LAT & 0xff),
        (unsigned long) (PI_DOM2_PWD & 0xff), (unsigned long) (PI_DOM2_PGS & 0xf), (unsigned long) (PI_DOM2_RLS & 3));
    evd_setCfgBit(ED_CFG_SDRAM_ON, 0);
    sleep(2);
    cart_read(buf, ROM_ADDR, CHUNK);
    evd_setCfgBit(ED_CFG_SDRAM_ON, 1);
    char title[21];
    memcpy(title, buf + 0x20, 20);
    title[20] = 0;
    for (int i = 0; i < 20; i++)
        if (title[i] && (title[i] < 0x20 || title[i] > 0x7e))
        {
            title[i] = 0;
            break;
        }
    for (int i = 19; i >= 0 && (title[i] == ' ' || title[i] == 0); i--)
        title[i] = 0;
    add("NOR title: '%s'", title);
    add("NOR magic %02x%02x%02x%02x boot %02x%02x%02x%02x", buf[0], buf[1], buf[2], buf[3], buf[8], buf[9], buf[10],
        buf[11]);
    int shown = 0;
    for (u32 i = 0x40; i + 8 < CHUNK && shown < 3; i++)
    {
        if ((memcmp(buf + i, "ED64", 4) == 0 || memcmp(buf + i, "boot", 4) == 0 || memcmp(buf + i, "FW:", 3) == 0) &&
            i != 0x20)
        {
            char s[32];
            int n = 0;
            while (n < 31 && buf[i + n] >= 0x20 && buf[i + n] < 0x7f)
            {
                s[n] = buf[i + n];
                n++;
            }
            s[n] = 0;
            if (n >= 6)
            {
                add("NOR @%05lx '%s'", (unsigned long) i, s);
                shown++;
                i += n;
            }
        }
    }
}

// ------------------------------------------------------------- 2. Cart bus ---
static u32 read_mb_crc(u32 base, u32 mb, u32 *ms_out)
{
    u32 dma_ticks = 0, crc = 0;
    for (u32 off = 0; off < (mb << 20); off += CHUNK)
    {
        u32 t0 = get_ticks();
        cart_read(buf, ROM_ADDR + base + off, CHUNK);
        dma_ticks += (u32) (get_ticks() - t0);
        crc = sum32(buf, CHUNK, crc);
    }
    *ms_out = dma_ticks / (COUNTS_PER_SECOND / 1000);
    return crc;
}
static void sec_bus(void)
{
    add("[cart bus / PI timing]");
    u32 base = DMA_BUFF_ADDR - (4u << 20);
    for (u32 off = 0; off < (4u << 20); off += CHUNK)
    {
        u32 *w = (u32 *) buf;
        for (u32 i = 0; i < CHUNK / 4; i++)
            w[i] = pattern(0x1234, off + i * 4);
        cart_write(buf, ROM_ADDR + base + off, CHUNK);
    }
    u32 lat0 = PI_DOM1_LAT & 0xff, pwd0 = PI_DOM1_PWD & 0xff, pgs0 = PI_DOM1_PGS & 0xf, rls0 = PI_DOM1_RLS & 3;
    u32 ms, ref = read_mb_crc(base, 4, &ms);
    add("SDRAM read 4MB default: %lu KB/s", (unsigned long) kbps(4u << 20, ms));
    {
        evd_setCfgBit(ED_CFG_SDRAM_ON, 0);
        sleep(2);
        u32 nms;
        read_mb_crc(0, 1, &nms);
        evd_setCfgBit(ED_CFG_SDRAM_ON, 1);
        add("NOR read 1MB default: %lu KB/s", (unsigned long) kbps(1u << 20, nms));
    }
    static const u32 pwds[] = {0x12, 0x0e, 0x0a, 0x08, 0x06, 0x04, 0x03};
    u32 best_pwd = pwd0, best_kb = kbps(4u << 20, ms);
    for (int i = 0; i < 7; i++)
    {
        if (pwds[i] >= pwd0)
            continue;
        PI_DOM1_PWD = pwds[i];
        u32 c = read_mb_crc(base, 4, &ms);
        u32 kb = kbps(4u << 20, ms);
        add(" PWD %02lx: %lu KB/s %s", (unsigned long) pwds[i], (unsigned long) kb, c == ref ? "ok" : "CORRUPT");
        if (c != ref)
            break;
        best_pwd = pwds[i];
        best_kb = kb;
    }
    PI_DOM1_PWD = best_pwd;
    static const u32 lats[] = {0x20, 0x10, 0x08, 0x04};
    u32 best_lat = lat0;
    for (int i = 0; i < 4; i++)
    {
        if (lats[i] >= lat0)
            continue;
        PI_DOM1_LAT = lats[i];
        u32 c = read_mb_crc(base, 4, &ms);
        u32 kb = kbps(4u << 20, ms);
        add(" LAT %02lx: %lu KB/s %s", (unsigned long) lats[i], (unsigned long) kb, c == ref ? "ok" : "CORRUPT");
        if (c != ref)
            break;
        best_lat = lats[i];
        best_kb = kb;
    }
    PI_DOM1_LAT = lat0;
    PI_DOM1_PWD = pwd0;
    PI_DOM1_PGS = pgs0;
    PI_DOM1_RLS = rls0;
    add("best stable LAT %02lx PWD %02lx = %lu KB/s", (unsigned long) best_lat, (unsigned long) best_pwd,
        (unsigned long) best_kb);
    add("(timings restored to %02lx/%02lx)", (unsigned long) lat0, (unsigned long) pwd0);
}

// ---------------------------------------------------------------- 3. SDRAM ---
static void sec_sdram(void)
{
    add("[SDRAM]");
    u32 end = DMA_BUFF_ADDR;
    u32 errors = 0, first_err = 0xFFFFFFFF;
    u32 t0 = get_ticks();
    for (u32 off = 0; off < end; off += CHUNK)
    {
        u32 *w = (u32 *) buf;
        for (u32 i = 0; i < CHUNK / 4; i++)
            w[i] = pattern(0xA5A5, off + i * 4);
        cart_write(buf, ROM_ADDR + off, CHUNK);
    }
    u32 wr_ms = ms_since(t0);
    t0 = get_ticks();
    u32 region_err[16] = {0};
    for (u32 off = 0; off < end; off += CHUNK)
    {
        cart_read(buf, ROM_ADDR + off, CHUNK);
        u32 *w = (u32 *) buf;
        for (u32 i = 0; i < CHUNK / 4; i++)
            if (w[i] != pattern(0xA5A5, off + i * 4))
            {
                errors++;
                region_err[off >> 22]++;
                if (first_err == 0xFFFFFFFF)
                    first_err = off + i * 4;
            }
    }
    u32 rd_ms = ms_since(t0);
    u32 err2 = 0;
    for (u32 off = 0; off < (8u << 20); off += CHUNK)
    {
        u32 *w = (u32 *) buf;
        for (u32 i = 0; i < CHUNK / 4; i++)
            w[i] = ~pattern(0xA5A5, off + i * 4);
        cart_write(buf, ROM_ADDR + off, CHUNK);
    }
    for (u32 off = 0; off < (8u << 20); off += CHUNK)
    {
        cart_read(buf2, ROM_ADDR + off, CHUNK);
        u32 *w = (u32 *) buf2;
        for (u32 i = 0; i < CHUNK / 4; i++)
            if (w[i] != ~pattern(0xA5A5, off + i * 4))
                err2++;
    }
    add("%luMB tested: %lu errors (+%lu inv)", (unsigned long) (end >> 20), (unsigned long) errors,
        (unsigned long) err2);
    if (errors)
    {
        add("first error @%08lx", (unsigned long) first_err);
        for (int r = 0; r < 16; r++)
            if (region_err[r])
                add(" region %2dMB: %lu", r * 4, (unsigned long) region_err[r]);
    }
    add("write %lu KB/s  read %lu KB/s", (unsigned long) kbps(end, wr_ms), (unsigned long) kbps(end, rd_ms));
    if (errors || err2)
        diag_failed = 1;
}

// ------------------------------------------------------------- 4. Save RAM ---
static void sram_timings_std(void)
{
    PI_DOM2_LAT = 0x05;
    PI_DOM2_PWD = 0x0C;
    PI_DOM2_PGS = 0x0D;
    PI_DOM2_RLS = 0x02;
}
static void sram_timings_def(void)
{
    PI_DOM2_LAT = 0x40;
    PI_DOM2_PWD = 0x12;
    PI_DOM2_PGS = 0x07;
    PI_DOM2_RLS = 0x03;
}

static u32 sram_base = 0xA8000000;
static u32 sram_variant(u32 seed, u32 size, u32 wchunk, u32 rchunk, int show)
{
    u32 errors = 0;
    for (u32 off = 0; off < size; off += wchunk)
    {
        u32 *w = (u32 *) buf;
        for (u32 i = 0; i < wchunk / 4; i++)
            w[i] = pattern(seed, off + i * 4);
        pi_write_raw(buf, sram_base + off, wchunk);
    }
    pi_wait();
    for (u32 off = 0; off < size; off += rchunk)
    {
        memset(buf2, 0, rchunk);
        pi_read_raw(buf2, sram_base + off, rchunk);
        u32 *w = (u32 *) buf2;
        for (u32 i = 0; i < rchunk / 4; i++)
            if (w[i] != pattern(seed, off + i * 4))
            {
                if (show && errors == 0)
                {
                    u32 e0 = pattern(seed, off + i * 4), e1 = pattern(seed, off + i * 4 + 4);
                    add("  exp %08lx %08lx", (unsigned long) e0, (unsigned long) e1);
                    add("  got %08lx %08lx", (unsigned long) w[i], (unsigned long) (i + 1 < rchunk / 4 ? w[i + 1] : 0));
                }
                errors++;
            }
    }
    return errors;
}

static void hexline(const char *tag, const u8 *p)
{
    add("%s %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", tag, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
        p[8], p[9], p[10], p[11]);
}
static int find_sig(u32 pi_base, u32 len, const char *sig, u32 *where)
{
    size_t sl = strlen(sig);
    for (u32 off = 0; off < len; off += CHUNK)
    {
        cart_read(buf, pi_base + off, CHUNK);
        for (u32 i = 0; i + sl <= CHUNK; i++)
            if (buf[i] == (u8) sig[0] && memcmp(buf + i, sig, sl) == 0)
            {
                *where = off + i;
                return 1;
            }
    }
    return 0;
}

static void sec_saveram(void)
{
    add("[save RAM (raw PI DMA)]");
    u32 l2 = PI_DOM2_LAT, p2 = PI_DOM2_PWD, g2 = PI_DOM2_PGS, r2 = PI_DOM2_RLS;
    sram_timings_std();
    evd_setSaveType(SAVE_TYPE_SRAM128);
    sleep(5);
    pi_read_raw(buf, 0xA8000000, 512);
    hexline("A8000000:", buf);
    pi_read_raw(buf, 0xA8000000 + 0x1000, 512);
    hexline("A8001000:", buf);
    pi_read_raw(buf, 0xA7FF0000, 512);
    hexline("A7FF0000:", buf);
    {
        u32 w, found = 0;
        for (u32 off = 0; off < (128u << 10) && !found; off += CHUNK)
        {
            pi_read_raw(buf, 0xA8000000 + off, CHUNK);
            for (u32 i = 0; i + 6 <= CHUNK; i++)
                if (memcmp(buf + i, "ZELDAZ", 6) == 0)
                {
                    w = off + i;
                    found = 1;
                    break;
                }
        }
        if (found)
            add("'ZELDAZ' @ A8000000+%05lx", (unsigned long) w);
        else
            add("'ZELDAZ' not in A8000000 window");
    }
    u32 e;
    sram_base = 0xA8000000;
    e = sram_variant(0x5EED, 32u << 10, 512, 512, 1);
    add("A8000000 512B DMA SRAM128: %lu err", (unsigned long) e);
    e = sram_variant(0x5EEE, 32u << 10, 32768, 512, 0);
    add("A8000000 32KB DMA SRAM128: %lu err", (unsigned long) e);
    evd_setSaveType(SAVE_TYPE_SRAM);
    sleep(5);
    e = sram_variant(0x5EF0, 32u << 10, 512, 512, 0);
    add("A8000000 512B DMA SRAM32: %lu err", (unsigned long) e);
    evd_setSaveType(SAVE_TYPE_FLASH);
    sleep(5);
    e = sram_variant(0x5EF1, 32u << 10, 512, 512, 0);
    add("FLASH mode plain DMA write: %lu err (info)", (unsigned long) e);
    add(" (FlashRAM ignores plain writes; the OS writes .fla in SRAM128 mode)");
    evd_setSaveType(SAVE_TYPE_SRAM128);
    sleep(5);
    sram_base = 0xA7FF0000;
    e = sram_variant(0x5EF2, 32u << 10, 512, 512, 0);
    add("A7FF0000 (old Altra64): %lu err (info)", (unsigned long) e);
    sram_base = 0xA8000000;
    e = sram_variant(0x5EF3, 128u << 10, 512, 512, 1);
    add("A8000000 128KB 512B chunks: %lu err", (unsigned long) e);
    if (e)
    {
        u32 reg[8] = {0};
        u32 first = 0xFFFFFFFF;
        for (u32 off = 0; off < (128u << 10); off += 512)
        {
            pi_read_raw(buf2, 0xA8000000 + off, 512);
            u32 *w = (u32 *) buf2;
            for (u32 i = 0; i < 128; i++)
                if (w[i] != pattern(0x5EF3, off + i * 4))
                {
                    reg[off >> 14]++;
                    if (first == 0xFFFFFFFF)
                        first = off + i * 4;
                }
        }
        add(" re-read: first err @+%05lx; per 16KB: %lu %lu %lu %lu %lu %lu %lu %lu", (unsigned long) first,
            (unsigned long) reg[0], (unsigned long) reg[1], (unsigned long) reg[2], (unsigned long) reg[3],
            (unsigned long) reg[4], (unsigned long) reg[5], (unsigned long) reg[6], (unsigned long) reg[7]);
    }
    {
        u8 *big = malloc(128u << 10), *big2 = malloc(128u << 10);
        u32 e1 = 0, e2a = 0, e2b = 0, e2d = 0, e3 = 0, e4 = 0;
        if (big && big2)
        {
            u32 *w = (u32 *) big, *r = (u32 *) big2;
            sram_timings_def();
            for (u32 i = 0; i < (128u << 10) / 4; i++)
                w[i] = pattern(0x5EF4, i * 4);
            setSRAM(big, 128u << 10);
            memset(big2, 0, 128u << 10);
            getSRAM(big2, 128u << 10);
            for (u32 i = 0; i < (128u << 10) / 4; i++)
                if (r[i] != pattern(0x5EF4, i * 4))
                    e1++;
            for (u32 i = 0; i < (128u << 10) / 4; i++)
                w[i] = pattern(0x5EF5, i * 4);
            pi_write_raw(big, 0xA8000000, 128u << 10);
            pi_wait();
            static u8 bad_a[128u << 10 >> 7];
            memset(bad_a, 0, sizeof(bad_a));
            for (int pass = 0; pass < 2; pass++)
            {
                for (u32 off = 0; off < (128u << 10); off += 512)
                {
                    pi_read_raw(buf2, 0xA8000000 + off, 512);
                    u32 *q = (u32 *) buf2;
                    int bad = 0;
                    for (u32 i = 0; i < 128; i++)
                        if (q[i] != pattern(0x5EF5, off + i * 4))
                        {
                            bad = 1;
                            if (pass)
                                e2b++;
                            else
                                e2a++;
                        }
                    if (!pass)
                        bad_a[off >> 9] = bad;
                    else if (bad != bad_a[off >> 9])
                        e2d++;
                }
            }
            for (u32 i = 0; i < (128u << 10) / 4; i++)
                w[i] = pattern(0x5EF6, i * 4);
            pi_write_raw(big, 0xA8000000, 128u << 10);
            pi_wait();
            pi_write_raw(big, 0xA8000000, 128u << 10);
            pi_wait();
            for (u32 off = 0; off < (128u << 10); off += 512)
            {
                pi_read_raw(buf2, 0xA8000000 + off, 512);
                u32 *q = (u32 *) buf2;
                for (u32 i = 0; i < 128; i++)
                    if (q[i] != pattern(0x5EF6, off + i * 4))
                        e3++;
            }
            for (u32 i = 0; i < (128u << 10) / 4; i++)
                w[i] = pattern(0x5EF7, i * 4);
            for (u32 off = 0; off < (128u << 10); off += (32u << 10))
            {
                pi_write_raw(big + off, 0xA8000000 + off, 32u << 10);
                pi_wait();
                sleep(20);
            }
            for (u32 off = 0; off < (128u << 10); off += 512)
            {
                pi_read_raw(buf2, 0xA8000000 + off, 512);
                u32 *q = (u32 *) buf2;
                for (u32 i = 0; i < 128; i++)
                    if (q[i] != pattern(0x5EF7, off + i * 4))
                        e4++;
            }
            add("E1 OS save path 128KB: %lu err, %d retries, %d unverified", (unsigned long) e1, sram_last_pass,
                sram_last_bad);
            add("E5: bad words per 16KB after rewriting region #");
            {
                for (u32 off = 0; off < (128u << 10); off += 512)
                {
                    u32 *q = (u32 *) buf;
                    for (u32 i = 0; i < 128; i++)
                        q[i] = pattern(0x5EF8, off + i * 4);
                    pi_write_raw(buf, 0xA8000000 + off, 512);
                }
                pi_wait();
                for (int k = 0; k < 8; k++)
                {
                    u32 hit[8] = {0};
                    for (u32 off = k * 16384u; off < (k + 1) * 16384u; off += 512)
                    {
                        u32 *q = (u32 *) buf;
                        for (u32 i = 0; i < 128; i++)
                            q[i] = pattern(0x5EF9, off + i * 4);
                        pi_write_raw(buf, 0xA8000000 + off, 512);
                    }
                    pi_wait();
                    for (u32 off = 0; off < (128u << 10); off += 512)
                    {
                        int r = off >> 14;
                        pi_read_raw(buf2, 0xA8000000 + off, 512);
                        u32 *q = (u32 *) buf2;
                        for (u32 i = 0; i < 128; i++)
                        {
                            u32 want = (r <= k) ? pattern(0x5EF9, off + i * 4) : pattern(0x5EF8, off + i * 4);
                            if (q[i] != want)
                                hit[r]++;
                        }
                    }
                    add("E5 w#%d: %lu %lu %lu %lu %lu %lu %lu %lu", k, (unsigned long) hit[0], (unsigned long) hit[1],
                        (unsigned long) hit[2], (unsigned long) hit[3], (unsigned long) hit[4], (unsigned long) hit[5],
                        (unsigned long) hit[6], (unsigned long) hit[7]);
                }
            }
            {
                u32 mis = 0;
                for (u32 i = 0; i < (128u << 10) / 4; i++)
                    w[i] = pattern(0x5EFA, i * 4);
                setSRAM(big, 128u << 10);
                sram_timings_std();
                for (u32 off = 0; off < (128u << 10); off += 512)
                {
                    pi_read_raw(buf2, 0xA8000000 + off, 512);
                    u32 *q = (u32 *) buf2;
                    for (u32 i = 0; i < 128; i++)
                        if (q[i] != pattern(0x5EFA, off + i * 4))
                            mis++;
                }
                add("E6 SDRAM top vs save window: %lu of 32768 words differ", (unsigned long) mis);
            }
            add("E2 raw write, read twice: %lu / %lu err, %lu blocks differ", (unsigned long) e2a, (unsigned long) e2b,
                (unsigned long) e2d);
            add("E3 written twice: %lu err", (unsigned long) e3);
            add("E4 4 x 32KB with pauses: %lu err", (unsigned long) e4);
        }
        else
            add("128KB experiments: no memory");
        free(big);
        free(big2);
        if (e1)
            diag_failed = 1;
        else if (e || e2a || e3 || e4)
            diag_warn = 1;
    }
    memset(buf, 0, CHUNK);
    for (u32 off = 0; off < (128u << 10); off += CHUNK)
        pi_write_raw(buf, 0xA8000000 + off, CHUNK);
    evd_setSaveType(SAVE_TYPE_OFF);
    PI_DOM2_LAT = l2;
    PI_DOM2_PWD = p2;
    PI_DOM2_PGS = g2;
    PI_DOM2_RLS = r2;
}

// --------------------------------------------------------------- 5. SD card ---
static int spi_read_reg16(u8 cmd, u8 *out)
{
    if (sdGetInterface() != DISK_IFACE_SPI)
        return -1;
    if (diskCmdSPI(cmd, 0) != 0)
        return -2;
    evd_spiSSOn();
    int ok = 0;
    for (int i = 0; i < 2000; i++)
        if (evd_SPI(0xff) == 0xFE)
        {
            ok = 1;
            break;
        }
    if (ok)
    {
        for (int i = 0; i < 16; i++)
            out[i] = evd_SPI(0xff);
        evd_SPI(0xff);
        evd_SPI(0xff);
    }
    evd_spiSSOff();
    return ok ? 0 : -3;
}
static void sec_sd(void)
{
    add("[SD card]");
    u8 cid[16], csd[16];
    add("type: %s%s  iface %s", (card_type & 2) ? "SDv2" : "SDv1/MMC", (card_type & 1) ? " HC" : "",
        sdGetInterface() == DISK_IFACE_SD ? "4-bit" : "1-bit SPI");
    if (spi_read_reg16(0x40 + 10, cid) == 0)
    {
        char pnm[6];
        memcpy(pnm, cid + 3, 5);
        pnm[5] = 0;
        add("CID mfr %02x oem %c%c '%s' rev %d.%d", cid[0], cid[1], cid[2], pnm, cid[8] >> 4, cid[8] & 15);
        add("serial %02x%02x%02x%02x date %d/%02d", cid[9], cid[10], cid[11], cid[12],
            2000 + ((cid[13] & 15) << 4 | (cid[14] >> 4)), cid[14] & 15);
    }
    else
        add("CID: not readable");
    if (spi_read_reg16(0x40 + 9, csd) == 0)
    {
        u32 cap_mb = 0;
        if ((csd[0] >> 6) == 1)
            cap_mb = (((u32) (csd[7] & 0x3f) << 16 | csd[8] << 8 | csd[9]) + 1) / 2;
        else
        {
            u32 c = ((csd[6] & 3) << 10) | (csd[7] << 2) | (csd[8] >> 6);
            u32 mult = ((csd[9] & 3) << 1) | (csd[10] >> 7);
            u32 bl = csd[5] & 15;
            cap_mb = ((c + 1) << (mult + 2)) >> (20 - bl);
        }
        add("CSD v%d cap %lu MB tran %02x cls %02x%02x", (csd[0] >> 6) + 1, (unsigned long) cap_mb, csd[3], csd[4],
            csd[5] >> 4);
    }
    else
        add("CSD: not readable");
    evd_setSpiSpeed(sd_speed == 2 ? SPI_SPEED_50 : SPI_SPEED_25);
    u32 base = DMA_BUFF_ADDR - (4u << 20);
    static const u16 per[] = {64, 512, 2048};
    for (int i = 0; i < 3; i++)
    {
        u32 t0 = get_ticks(), got = 0, err = 0;
        for (u32 s = 0; s < 4096; s += per[i])
        {
            if (sdRead(s, (u8 *) (ROM_ADDR + base + s * 512), per[i]) != 0)
            {
                err = 1;
                break;
            }
            got += per[i];
        }
        u32 ms = ms_since(t0);
        add("read x%4d: %lu KB/s%s", per[i], (unsigned long) kbps(got * 512, ms), err ? " ERR" : "");
    }
    u32 t0 = get_ticks(), seed = 12345, n = 0;
    for (int i = 0; i < 32; i++)
    {
        seed = seed * 1103515245u + 12345u;
        u32 s = (seed >> 8) & 0xFFFF;
        if (sdRead(s * 8, buf, 8) == 0)
            n++;
    }
    u32 ms = ms_since(t0);
    add("random 4KB read: %lu ms avg (%lu ok)", (unsigned long) (ms / 32), (unsigned long) n);
    FIL f;
    UINT bw;
    u32 wrote = 0;
    memset(buf, 0x5A, CHUNK);
    t0 = get_ticks();
    if (f_open(&f, "/" ED64_FIRMWARE_PATH "/SYS/SDTEST.TMP", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
        for (int i = 0; i < 16; i++)
        {
            if (f_write(&f, buf, CHUNK, &bw) != FR_OK || bw != CHUNK)
                break;
            wrote += bw;
        }
        f_close(&f);
        f_unlink("/" ED64_FIRMWARE_PATH "/SYS/SDTEST.TMP");
    }
    ms = ms_since(t0);
    add("write 1MB (f_write): %lu KB/s", (unsigned long) kbps(wrote, ms));
    add("spi_cfg=%02x sd_speed=%u", spi_cfg, sd_speed);
}

// ------------------------------------------------------------ 6. Controllers --
static void sec_pads(void)
{
    add("[controllers]");
    int present = get_controllers_present();
    for (int i = 0; i < 4; i++)
    {
        if (!(present & (0xF000 >> (i * 4))))
        {
            add("port %d: none", i + 1);
            continue;
        }
        int acc = ed64_detect_pak(i);
        add("port %d: pad, %s", i + 1,
            acc == ACCESSORY_MEMPAK      ? "Controller Pak"
            : acc == ACCESSORY_RUMBLEPAK ? "Rumble Pak"
                                         : "no pak");
    }
}

void diag_run(void (*progress)(const char *))
{
    diag_nlines = 0;
    diag_failed = 0;
    diag_warn = 0;
    while (evd_isDmaBusy())
        ;
    if (progress)
        progress("board & core");
    sec_board();
    if (progress)
        progress("save RAM");
    sec_saveram();
    if (progress)
        progress("cart bus timing");
    sec_bus();
    if (progress)
        progress("SDRAM (takes a while)");
    sec_sdram();
    if (progress)
        progress("SD card");
    sec_sd();
    if (progress)
        progress("controllers");
    sec_pads();
    add(diag_failed ? "[result] FAILED"
        : diag_warn ? "[result] PASSED (with a warning: see save RAM)"
                    : "[result] PASSED");
}
