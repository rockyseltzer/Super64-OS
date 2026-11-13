//
// Adapted from ED64-IO, the EverDrive-64 I/O library developed by Krikzz
//

#ifndef MEM_H
#define MEM_H

#include "types.h"

#define SPI_SPEED_INIT 2
#define SPI_SPEED_25   1
#define SPI_SPEED_50   0

void memSpiBusy(void);
u8 memSpiRead(void *dst, u16 slen);
u8 memSpiWrite(const void *src);
void memSpiSetDma(u8 mode);
u32 memRomRead32(u32 addr);

#endif /* MEM_H */
