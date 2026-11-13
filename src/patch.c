#include "patch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"

#include "everdrive.h"
#include "sys.h"

#define WIN_SIZE 4096
#define RD_SIZE  16384

static u8 win[WIN_SIZE] __attribute__((aligned(16)));
static u32 win_base = 0xFFFFFFFF;
static int win_dirty = 0;
static u32 patch_rom_size = 0;

static FIL pf;
static u8 rdbuf[RD_SIZE];
static u32 rd_len = 0, rd_pos = 0;
static int rd_eof = 0;

static int rd_fill(void)
{
    UINT br = 0;
    if (f_read(&pf, rdbuf, RD_SIZE, &br) != FR_OK || br == 0)
    {
        rd_eof = 1;
        return 0;
    }
    rd_len = br;
    rd_pos = 0;
    return 1;
}
static int rd_byte(void)
{
    if (rd_pos >= rd_len && !rd_fill())
        return -1;
    return rdbuf[rd_pos++];
}
static int rd_bytes(u8 *dst, u32 n)
{
    for (u32 i = 0; i < n; i++)
    {
        int c = rd_byte();
        if (c < 0)
            return 0;
        dst[i] = (u8) c;
    }
    return 1;
}

static void win_flush(void)
{
    if (win_dirty && win_base != 0xFFFFFFFF)
        dma_write_s(win, ROM_ADDR + win_base, WIN_SIZE);
    win_dirty = 0;
}
static void win_load(u32 base)
{
    if (base == win_base)
        return;
    win_flush();
    win_base = base;
    dma_read_s(win, ROM_ADDR + base, WIN_SIZE);
}
static int put_byte(u32 off, u8 v)
{
    if (off >= patch_rom_size)
        return 0;
    win_load(off & ~(WIN_SIZE - 1));
    win[off & (WIN_SIZE - 1)] = v;
    win_dirty = 1;
    return 1;
}
static int put_run(u32 off, const u8 *data, u32 n, int swapped)
{
    for (u32 i = 0; i < n; i++)
        if (!put_byte(swapped ? ((off + i) ^ 1) : (off + i), data[i]))
            return 0;
    return 1;
}
static int put_fill(u32 off, u8 v, u32 n, int swapped)
{
    for (u32 i = 0; i < n; i++)
        if (!put_byte(swapped ? ((off + i) ^ 1) : (off + i), v))
            return 0;
    return 1;
}

static int apply_ips(void (*progress)(int))
{
    u8 h[5];
    if (!rd_bytes(h, 5) || memcmp(h, "PATCH", 5) != 0)
        return PATCH_ERR_BAD_IPS;
    u32 fsize = f_size(&pf), done = 5, next_pct = 10;
    u8 data[65536];
    for (;;)
    {
        u8 r[3];
        if (!rd_bytes(r, 3))
            return PATCH_ERR_BAD_IPS;
        if (memcmp(r, "EOF", 3) == 0)
            break;
        u32 off = ((u32) r[0] << 16) | ((u32) r[1] << 8) | r[2];
        u8 sz[2];
        if (!rd_bytes(sz, 2))
            return PATCH_ERR_BAD_IPS;
        u32 size = ((u32) sz[0] << 8) | sz[1];
        done += 5;
        if (size == 0)
        {
            u8 rl[3];
            if (!rd_bytes(rl, 3))
                return PATCH_ERR_BAD_IPS;
            u32 n = ((u32) rl[0] << 8) | rl[1];
            if (!put_fill(off, rl[2], n, 0))
                return PATCH_ERR_RANGE;
            done += 3;
        }
        else
        {
            if (!rd_bytes(data, size))
                return PATCH_ERR_BAD_IPS;
            if (!put_run(off, data, size, 0))
                return PATCH_ERR_RANGE;
            done += size;
        }
        if (progress && fsize && done * 100 / fsize >= next_pct)
        {
            progress(done * 100 / fsize);
            next_pct += 10;
        }
    }
    return PATCH_OK;
}

static u32 rd32(const u8 *p, int le)
{
    return le ? ((u32) p[3] << 24 | (u32) p[2] << 16 | (u32) p[1] << 8 | p[0])
              : ((u32) p[0] << 24 | (u32) p[1] << 16 | (u32) p[2] << 8 | p[3]);
}

static int aps_probe_le(u32 hdr_len, u32 size_le, u32 size_be)
{
    int ok[2] = {1, 1};
    u32 last[2] = {0, 0};
    u32 dsize[2] = {size_le, size_be};
    f_lseek(&pf, hdr_len);
    rd_len = 0;
    rd_pos = 0;
    rd_eof = 0;
    for (int n = 0; n < 4096; n++)
    {
        u8 r[5];
        if (!rd_bytes(r, 5))
            break;
        for (int e = 0; e < 2; e++)
        {
            u32 off = rd32(r, e == 0);
            if (off < last[e] || off >= dsize[e] || dsize[e] == 0 || dsize[e] > (64u << 20))
                ok[e] = 0;
            last[e] = off;
        }
        u32 skip = r[4] ? r[4] : 2;
        u8 tmp[256];
        if (!rd_bytes(tmp, skip))
            break;
        if (!ok[0] && !ok[1])
            break;
    }
    if (ok[0] && !ok[1])
        return 1;
    if (ok[1] && !ok[0])
        return 0;
    return 1;
}

static int apply_aps(void (*progress)(int))
{
    u8 h[78];
    if (!rd_bytes(h, 7) || memcmp(h, "APS10", 5) != 0)
        return PATCH_ERR_BAD_APS;
    u8 type = h[5], enc = h[6];
    if (enc != 0 || type > 1)
        return PATCH_ERR_BAD_APS;
    u32 hdr_len = (type == 1) ? 78 : 61;
    if (!rd_bytes(h + 7, hdr_len - 7))
        return PATCH_ERR_BAD_APS;
    int swapped = (type == 1 && h[57] == 0);
    const u8 *szp = (type == 1) ? h + 74 : h + 57;
    int le = aps_probe_le(hdr_len, rd32(szp, 1), rd32(szp, 0));
    f_lseek(&pf, hdr_len);
    rd_len = 0;
    rd_pos = 0;
    rd_eof = 0;
    u32 fsize = f_size(&pf), done = hdr_len, next_pct = 10;
    u8 data[256];
    for (;;)
    {
        u8 r[5];
        if (!rd_bytes(r, 5))
        {
            if (rd_eof)
                break;
            return PATCH_ERR_BAD_APS;
        }
        u32 off = rd32(r, le);
        u8 len = r[4];
        done += 5;
        if (len == 0)
        {
            u8 rl[2];
            if (!rd_bytes(rl, 2))
                return PATCH_ERR_BAD_APS;
            if (!put_fill(off, rl[0], rl[1], swapped))
                return PATCH_ERR_RANGE;
            done += 2;
        }
        else
        {
            if (!rd_bytes(data, len))
                return PATCH_ERR_BAD_APS;
            if (!put_run(off, data, len, swapped))
                return PATCH_ERR_RANGE;
            done += len;
        }
        if (progress && fsize && done * 100 / fsize >= next_pct)
        {
            progress(done * 100 / fsize);
            next_pct += 10;
        }
    }
    return PATCH_OK;
}

int patch_apply(const char *path, u32 rom_size, void (*progress)(int pct))
{
    int rc;
    size_t n = strlen(path);
    if (f_open(&pf, path, FA_READ) != FR_OK)
        return PATCH_ERR_OPEN;
    rd_len = 0;
    rd_pos = 0;
    rd_eof = 0;
    win_base = 0xFFFFFFFF;
    win_dirty = 0;
    patch_rom_size = rom_size;
    while (evd_isDmaBusy())
        ;
    if (n > 4 && (strcasecmp(path + n - 4, ".ips") == 0))
        rc = apply_ips(progress);
    else if (n > 4 && (strcasecmp(path + n - 4, ".aps") == 0))
        rc = apply_aps(progress);
    else
        rc = PATCH_ERR_UNKNOWN;
    win_flush();
    f_close(&pf);
    if (progress && rc == PATCH_OK)
        progress(100);
    return rc;
}

const char *patch_error_text(int err)
{
    switch (err)
    {
        case PATCH_OK:
            return "Patch applied";
        case PATCH_ERR_OPEN:
            return "Patch file not found";
        case PATCH_ERR_BAD_IPS:
            return "Bad .ips file";
        case PATCH_ERR_BAD_APS:
            return "Bad .aps file";
        case PATCH_ERR_RANGE:
            return "Patch exceeds ROM size";
        default:
            return "Unknown file type";
    }
}
