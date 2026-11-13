//
// Adapted from ED64-IO, the EverDrive-64 I/O library developed by Krikzz
//

#include "sys.h"

#include <stdint.h>

#include <dma.h>
#include <n64sys.h>

#include "rom.h"

u32 asm_date;

void dma_read_s(void *ram_address, unsigned long pi_address, unsigned long len)
{
    u32 buff[256];

    u32 *bptr;

    u32 *rptr = (u32 *) ram_address;

    u16 i;

    IO_WRITE(PI_STATUS_REG, 3);
    while (len)
    {
        dma_read(buff, pi_address, 512);
        while ((IO_READ(PI_STATUS_REG) & 3) != 0)
            ;
        data_cache_hit_invalidate(buff, 512);
        bptr = buff;
        for (i = 0; i < 512 && i < len; i += 4)
            *rptr++ = *bptr++;
        len = len < 512 ? 0 : len - 512;
        pi_address += 512;
    }
}

void dma_write_s(void *ram_address, unsigned long pi_address, unsigned long len)
{
    data_cache_hit_writeback(ram_address, len);
    dma_write(ram_address, pi_address, len);
}

void sleep(u32 ms)
{
    u32 current_ms = get_ticks_ms();

    while (get_ticks_ms() - current_ms < ms)
        ;
}

static u32 ii;
void clean();

#define MEM32(addr) *((volatile u32 *) addr)

static u8 STR_intToDecString(u32 val_arg, u8 *str)
{
    u32 val = val_arg;

    int len;

    if (val < 10)
        len = 1;
    else if (val < 100)
        len = 2;
    else if (val < 1000)
        len = 3;
    else if (val < 10000)
        len = 4;
    else if (val < 100000)
        len = 5;
    else if (val < 1000000)
        len = 6;
    else if (val < 10000000)
        len = 7;
    else if (val < 100000000)
        len = 8;
    else if (val < 1000000000)
        len = 9;
    else
        len = 10;

    str += len;
    str[0] = 0;
    if (val == 0)
        *--str = '0';
    while (val)
    {
        *--str = '0' + val % 10;
        val /= 10;
    }

    return len;
}

static void STR_intToDecStringMin(u32 val_arg, u8 *str, u8 min_size)
{
    u32 val = val_arg;

    int len;
    u8 i;

    if (val < 10)
        len = 1;
    else if (val < 100)
        len = 2;
    else if (val < 1000)
        len = 3;
    else if (val < 10000)
        len = 4;
    else if (val < 100000)
        len = 5;
    else if (val < 1000000)
        len = 6;
    else if (val < 10000000)
        len = 7;
    else if (val < 100000000)
        len = 8;
    else if (val < 1000000000)
        len = 9;
    else
        len = 10;

    if (len < min_size)
    {
        i = min_size - len;
        while (i--)
            str[i] = '0';
        len = min_size;
    }
    str += len;
    str[0] = 0;
    if (val == 0)
        *--str = '0';
    while (val)
    {
        *--str = '0' + val % 10;
        val /= 10;
    }
}
