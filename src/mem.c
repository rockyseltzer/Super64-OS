//
// Adapted from ED64-IO, the EverDrive-64 I/O library developed by Krikzz
//

#include "mem.h"

#include "errors.h"
#include "everdrive.h"
#include "sys.h"

static u8 spi_dma;

static u8 memSpiReadDma(void *dst, u16 slen);
static u8 memSpiReadPio(void *dst, u16 slen);
static u8 mem_buff[512];

void memSpiSetDma(u8 mode)
{
    spi_dma = mode;
}

u8 memSpiRead(void *dst, u16 slen)
{
    u8 copy_to_rom = 0;
    u32 addr = (u32) dst;
    if (addr >= ROM_ADDR && addr < ROM_END_ADDR)
        copy_to_rom = 1;

    if ((copy_to_rom || spi_dma) && addr % 4 == 0)
    {
        return memSpiReadDma(dst, slen);
    }
    else
    {
        return memSpiReadPio(dst, slen);
    }
}

static u8 memSpiReadPio(void *dst, u16 slen)
{
    u16 i;
    u16 u;
    u8 *ptr8 = (u8 *) dst;

    for (i = 0; i < slen; i++)
    {
        evd_SDdatReadMode(1);
        for (u = 0; u < 65535; u++)
            if ((evd_SPI(0xff) & 0xf1) == 0xf0)
                break;
        evd_SDdatReadMode(0);
        if (u == 65535)
        {
            evd_spiSSOff();
            return DISK_RD_FE_TOUT;
        }

        for (u = 0; u < 512; u++)
            *ptr8++ = evd_SPI(0xff);

        u = evd_isSDMode() ? 8 : 2;

        while (u--)
        {
            u--;
            evd_SPI(0xff);
            evd_SPI(0xff);
        }
    }

    return 0;
}

static u8 memSpiReadDma(void *dst, u16 slen)
{
    u8 resp = 0;
    u8 copy_to_rom = 0;
    u32 addr = (u32) dst;

    evd_SDdatReadMode(0);
    if (addr >= ROM_ADDR && addr < ROM_END_ADDR)
        copy_to_rom = 1;

    if (copy_to_rom)
    {
        return evd_mmcReadToCart(addr, slen);
    }
    else
    {
        resp = evd_mmcReadToCart(0, slen);
        dma_read_s(dst, ROM_ADDR, slen * 512);
    }

    return resp;
}

u8 memSpiWrite(const void *src)
{
    u16 i;

    u8 *ptr8 = (u8 *) src;

    if ((u32) src >= ROM_ADDR && (u32) src < ROM_END_ADDR)
    {
        dma_read_s(mem_buff, (u32) src, 512);
        for (i = 0; i < 512; i++)
            evd_SPI(mem_buff[i]);
    }
    else
    {
        for (i = 0; i < 512; i++)
            evd_SPI(*ptr8++);
    }

    return 0;
}

void memSpiBusy()
{
    while (evd_isSpiBusy())
        ;
}

u32 memRomRead32(u32 addr)
{
    vu32 *ptr = (u32 *) (addr + ROM_ADDR);
    vu8 tmp;
    vu32 val;

    tmp = *ptr;
    val = *ptr;

    return val;
}
