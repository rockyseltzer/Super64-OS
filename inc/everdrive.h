//
// Adapted from ED64-IO, the EverDrive-64 I/O library developed by Krikzz
//

#ifndef EVERDRIVE_H
#define EVERDRIVE_H

#include "types.h"

#define ROM_LEN      0x4000000
#define ROM_ADDR     0xb0000000
#define ROM_END_ADDR (0xb0000000 + 0x4000000)

#define SAVE_TYPE_OFF     0
#define SAVE_TYPE_SRAM    1
#define SAVE_TYPE_SRAM128 2
#define SAVE_TYPE_EEP4k   3
#define SAVE_TYPE_EEP16k  4
#define SAVE_TYPE_FLASH   5
#define SAVE_TYPE_SRAM768 6
#define DMA_BUFF_ADDR     (ROM_LEN - 0x100000)

#define REG_CFG          0
#define REG_STATUS       1
#define REG_DMA_LEN      2
#define REG_DMA_RAM_ADDR 3
#define REG_MSG          4
#define REG_DMA_CFG      5
#define REG_SPI          6
#define REG_SPI_CFG      7
#define REG_KEY          8
#define REG_SAV_CFG      9
#define REG_SEC          10
#define REG_VER          11

#define REG_CFG_CNT 16
#define REG_CFG_DAT 17
#define REG_MAX_MSG 18
#define REG_CRC     19

#define DCFG_SD_TO_RAM   1
#define DCFG_RAM_TO_SD   2
#define DCFG_FIFO_TO_RAM 3
#define DCFG_RAM_TO_FIFO 4

#define ED_CFG_SDRAM_ON     0
#define ED_CFG_SWAP         1
#define ED_CFG_WR_MOD       2
#define ED_CFG_WR_ADDR_MASK 3

#define REGS_BASE 0xA8040000

extern volatile u8 spi_cfg;

extern u32 bi_reg_rd(u32 reg);
extern void bi_reg_wr(u32 reg, u32 data);

void evd_setCfgBit(u8 option, u8 state);

void evd_init(void);
u8 evd_fifoRxf(void);
u8 evd_fifoTxe(void);
u8 evd_isDmaBusy(void);
u8 evd_isDmaTimeout(void);
u8 evd_fifoRd(void *buff, u16 blocks);
u8 evd_fifoWr(void *buff, u16 blocks);
u8 evd_fifoRdToCart(u32 cart_addr, u16 blocks);
u8 evd_fifoWrFromCart(u32 cart_addr, u16 blocks);

u8 evd_SPI(u8 dat);
void evd_mmcSetDmaSwap(u8 state);
u8 evd_mmcReadToCart(u32 cart_addr, u32 len);

void evd_ulockRegs(void);
void evd_lockRegs(void);
u16 evd_readReg(u8 reg);
void evd_writeReg(u8 reg, u16 val);
void evd_setSaveType(u8 type);

void evd_writeMsg(u8 dat);
u8 evd_readMsg(void);
u16 evd_getFirmVersion(void);

void evd_spiSSOn(void);
void evd_spiSSOff(void);
u8 evd_isSpiBusy(void);
void evd_setSpiSpeed(u8 speed);

void evd_SDcmdWriteMode(u8 bit1_mode);
void evd_SDcmdReadMode(u8 bit1_mode);
void evd_SDdatWriteMode(u8 bit4_mode);
void evd_SDdatReadMode(u8 bit4_mode);
void evd_enableSDMode(void);
void evd_enableSPIMode(void);
u8 evd_isSDMode(void);
void evd_setDmaCallback(void (*callback)());

void bi_init(void);
void bi_load_firmware(u8 *firm);
void bi_speed25(void);
void bi_speed50(void);

#endif /* EVERDRIVE_H */
