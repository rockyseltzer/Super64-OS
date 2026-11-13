#ifndef ROM_H
#define ROM_H

#include "types.h"

#define DP_BASE_REG   0x04100000
#define VI_BASE_REG   0x04400000
#define PI_BASE_REG   0x04600000
#define PIF_RAM_START 0x1FC007C0

#define PI_STATUS_REG       (PI_BASE_REG + 0x10)
#define PI_DRAM_ADDR_REG    (PI_BASE_REG + 0x00)
#define PI_CART_ADDR_REG    (PI_BASE_REG + 0x04)
#define PI_RD_LEN_REG       (PI_BASE_REG + 0x08)
#define PI_WR_LEN_REG       (PI_BASE_REG + 0x0C)
#define PI_BSD_DOM1_LAT_REG (PI_BASE_REG + 0x14)
#define PI_BSD_DOM1_PWD_REG (PI_BASE_REG + 0x18)
#define PI_BSD_DOM1_PGS_REG (PI_BASE_REG + 0x1C)
#define PI_BSD_DOM1_RLS_REG (PI_BASE_REG + 0x20)
#define PI_BSD_DOM2_LAT_REG (PI_BASE_REG + 0x24)
#define PI_BSD_DOM2_PWD_REG (PI_BASE_REG + 0x28)
#define PI_BSD_DOM2_PGS_REG (PI_BASE_REG + 0x2C)
#define PI_BSD_DOM2_RLS_REG (PI_BASE_REG + 0x30)

#define PI_DOMAIN1_REG PI_BSD_DOM1_LAT_REG
#define PI_DOMAIN2_REG PI_BSD_DOM2_LAT_REG

#define PI_STATUS_ERROR    0x04
#define PI_STATUS_IO_BUSY  0x02
#define PI_STATUS_DMA_BUSY 0x01

#define DPC_START    (DP_BASE_REG + 0x00)
#define DPC_END      (DP_BASE_REG + 0x04)
#define DPC_CURRENT  (DP_BASE_REG + 0x08)
#define DPC_STATUS   (DP_BASE_REG + 0x0C)
#define DPC_CLOCK    (DP_BASE_REG + 0x10)
#define DPC_BUFBUSY  (DP_BASE_REG + 0x14)
#define DPC_PIPEBUSY (DP_BASE_REG + 0x18)
#define DPC_TMEM     (DP_BASE_REG + 0x1C)

#define VI_CONTROL     (VI_BASE_REG + 0x00)
#define VI_FRAMEBUFFER (VI_BASE_REG + 0x04)
#define VI_WIDTH       (VI_BASE_REG + 0x08)
#define VI_V_INT       (VI_BASE_REG + 0x0C)
#define VI_CUR_LINE    (VI_BASE_REG + 0x10)
#define VI_TIMING      (VI_BASE_REG + 0x14)
#define VI_V_SYNC      (VI_BASE_REG + 0x18)
#define VI_H_SYNC      (VI_BASE_REG + 0x1C)
#define VI_H_SYNC2     (VI_BASE_REG + 0x20)
#define VI_H_LIMITS    (VI_BASE_REG + 0x24)
#define VI_COLOR_BURST (VI_BASE_REG + 0x28)
#define VI_H_SCALE     (VI_BASE_REG + 0x2C)
#define VI_VSCALE      (VI_BASE_REG + 0x30)

#define PHYS_TO_K0(x) ((u32) (x) | 0x80000000)
#define K0_TO_PHYS(x) ((u32) (x) & 0x1FFFFFFF)
#define PHYS_TO_K1(x) ((u32) (x) | 0xA0000000)
#define K1_TO_PHYS(x) ((u32) (x) & 0x1FFFFFFF)

#define IO_READ(addr)        (*(volatile u32 *) PHYS_TO_K1(addr))
#define IO_WRITE(addr, data) (*(volatile u32 *) PHYS_TO_K1(addr) = (u32) (data))

#define SAVE_SIZE_SRAM    32768
#define SAVE_SIZE_SRAM128 131072
#define SAVE_SIZE_SRAM768 98304
#define SAVE_SIZE_EEP4k   512
#define SAVE_SIZE_EEP16k  2048
#define SAVE_SIZE_FLASH   131072

#define FRAM_EXECUTE_CMD      0xD2000000
#define FRAM_STATUS_MODE_CMD  0xE1000000
#define FRAM_ERASE_OFFSET_CMD 0x4B000000
#define FRAM_WRITE_OFFSET_CMD 0xA5000000
#define FRAM_ERASE_MODE_CMD   0x78000000
#define FRAM_WRITE_MODE_CMD   0xB4000000
#define FRAM_READ_MODE_CMD    0xF0000000

#define FRAM_STATUS_REG  0xA8000000
#define FRAM_COMMAND_REG 0xA8010000

#define REGION_UNKNOWN 0
#define REGION_NTSC    1
#define REGION_PAL     2
#define REGION_PAL_M   3

int is_valid_rom(unsigned char *buffer);
void swap_header(unsigned char *header, int loadlength);

u8 getCicType(u8 bios_cic);

void send_game_id(uint8_t *crc_hi, uint8_t *crc_lo, uint8_t media_format, uint8_t country_id);
void get_country_and_region(unsigned char ccode, char *region_string);

#endif /* ROM_H */
