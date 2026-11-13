#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "types.h"

#if !defined(MIN)
#define MIN(a, b)                                                                                                      \
    ({                                                                                                                 \
        __typeof__(a) _a = (a);                                                                                        \
        __typeof__(b) _b = (b);                                                                                        \
        _a < _b ? _a : _b;                                                                                             \
    })
#endif

extern int sram_last_bad, sram_last_pass;

void simulate_boot(u32 boot_cic, u8 bios_cic, u32 *cheat_list[2]);

int get_cic_save(const unsigned char *hdr, int *cic, int *save);
const char *saveTypeToExtension(int type, int etype);
int saveTypeToSize(int type);
int getSaveFromCart(int stype, uint8_t *buffer);
int pushSaveToCart(int stype, uint8_t *buffer);

int getSRAM(uint8_t *buffer, int size);

int setSRAM(uint8_t *buffer, int size);

#endif /* UTILS_H */
