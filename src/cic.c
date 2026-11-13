#include "cic.h"

#include <stddef.h>
#include <stdint.h>

#define IPL3_CRC_6101    0x6170A4A1u
#define IPL3_CRC_7102    0x009E9EA3u // PAL version of the 6101
#define IPL3_CRC_6102    0x90BB6CB5u // Also the 7101 (PAL)
#define IPL3_CRC_6103    0x0B050EE0u // Also the 7103
#define IPL3_CRC_6105    0x98BC2C86u // Also the 7105
#define IPL3_CRC_6106    0xACC8580Au // Also the 7106
#define IPL3_CRC_8303    0x0E018159u // 64DD
#define IPL3_CRC_5101    0x587BD543u // Aleck 64, over bytes 0x40..0xBFF only
#define IPL3_CRC_5101_P1 0xE351F733u // Aleck 64 console conversion, seed set at 0x550
#define IPL3_CRC_5101_P2 0x3B46348Cu // Aleck 64 console conversion, seed set at 0x574

#define IPL3_START       0x40
#define IPL3_END         0x1000
#define IPL3_END_ALECK64 0xC00

static uint32_t crc32(const uint8_t *data_arg, size_t len)
{
    const uint8_t *data = data_arg;
    uint32_t crc = 0xFFFFFFFFu;
    while (len--)
    {
        crc ^= *data++;
        for (int bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1));
    }
    return ~crc;
}

int get_cic(unsigned char *rom_data)
{
    switch (crc32(&rom_data[IPL3_START], IPL3_END_ALECK64 - IPL3_START))
    {
        case IPL3_CRC_5101:
        case IPL3_CRC_5101_P1:
        case IPL3_CRC_5101_P2:
            return CIC_5101;
        default:
            break;
    }

    switch (crc32(&rom_data[IPL3_START], IPL3_END - IPL3_START))
    {
        case IPL3_CRC_6101:
        case IPL3_CRC_7102:
            return CIC_6101;
        case IPL3_CRC_6102:
            return CIC_6102;
        case IPL3_CRC_6103:
            return CIC_6103;
        case IPL3_CRC_6105:
            return CIC_6105;
        case IPL3_CRC_6106:
            return CIC_6106;
        case IPL3_CRC_8303:
            return CIC_8303;
        default:
            return 0;
    }
}
