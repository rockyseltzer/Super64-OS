//
// Adapted from ED64-IO, the EverDrive-64 I/O library developed by Krikzz
//

#ifndef SD_H
#define SD_H

#include "types.h"

#define WAIT 1024

#define DISK_IFACE_SPI 0
#define DISK_IFACE_SD  1

extern u8 card_type;

u8 sdGetInterface(void);

u8 sdInit(void);
u8 sdRead(u32 sector, u8 *buff, u16 count);
u8 sdWrite(u32 sector, const u8 *buff, u16 count);

void sdSetInterface(u32 interface);

#endif /* SD_H */
