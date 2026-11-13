#include "sram.h"

#include <malloc.h>
#include <string.h>

#include "rom.h"
#include "sys.h"
#include "utils.h"

static void PI_DMAWait(void)
{
    while (IO_READ(PI_STATUS_REG) & (PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY))
        ;
}

static void PI_DMAFromCart(void *dest, void *src, u32 size)
{
    PI_DMAWait();

    IO_WRITE(PI_STATUS_REG, 0x03);
    IO_WRITE(PI_DRAM_ADDR_REG, K1_TO_PHYS(dest));
    IO_WRITE(PI_CART_ADDR_REG, K0_TO_PHYS(src));
    IO_WRITE(PI_WR_LEN_REG, (size - 1));
}
