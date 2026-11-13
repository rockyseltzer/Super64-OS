//
// Adapted from ED64-IO, the EverDrive-64 I/O library developed by Krikzz
//

#ifndef SYS_H
#define SYS_H

#include "types.h"

extern u32 asm_date;

void dma_read_s(void *ram_address, unsigned long pi_address, unsigned long len);
void dma_write_s(void *ram_address, unsigned long pi_address, unsigned long len);

void sleep(u32 ms);

#endif /* SYS_H */
