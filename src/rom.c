#include "rom.h"

#include <string.h>

#include <libdragon.h>

#include "cic.h"
#include "everdrive.h"
#include "sram.h"
#include "sys.h"

int is_valid_rom(unsigned char *buffer)
{
    if ((buffer[0] == 0x80) && (buffer[1] == 0x37) && (buffer[2] == 0x12) && (buffer[3] == 0x40))
        return 0;
    else if ((buffer[0] == 0x37) && (buffer[1] == 0x80) && (buffer[2] == 0x40) && (buffer[3] == 0x12))
        return 1;
    else if ((buffer[0] == 0x40) && (buffer[1] == 0x12) && (buffer[2] == 0x37) && (buffer[3] == 0x80))
        return 2;
    else
        return 0;
}

void swap_header(unsigned char *header, int loadlength)
{
    unsigned char temp;
    int i;

    if (header[0] == 0x37)
    {
        for (i = 0; i < loadlength; i += 2)
        {
            temp = header[i];
            header[i] = header[i + 1];
            header[i + 1] = temp;
        }
    }
    else if (header[0] == 0x40)
    {
        for (i = 0; i < loadlength; i += 4)
        {
            temp = header[i];
            header[i] = header[i + 3];
            header[i + 3] = temp;
            temp = header[i + 1];
            header[i + 1] = header[i + 2];
            header[i + 2] = temp;
        }
    }
}

u8 getCicType(u8 bios_cic)
{
    static u8 cic_buff[4096] __attribute__((aligned(16)));
    volatile u8 cic_chip;
    volatile u32 val;
    if (bios_cic)
    {
        evd_setCfgBit(ED_CFG_SDRAM_ON, 0);
        sleep(10);
        val = *(u32 *) 0xB0000170;
        dma_read_s(cic_buff, 0xB0000000, 4096);
        cic_chip = get_cic(cic_buff);
        evd_setCfgBit(ED_CFG_SDRAM_ON, 1);
        sleep(10);
    }
    else
    {
        val = *(u32 *) 0xB0000170;
        dma_read_s(cic_buff, 0xB0000000, 4096);
        cic_chip = get_cic(cic_buff);
    }
    (void) val;
    if (cic_chip == 0)
        cic_chip = 2;
    return cic_chip;
}

void send_game_id(uint8_t *crc_hi, uint8_t *crc_lo, uint8_t media_format, uint8_t country_id)
{
#define N64_CONTROLLER_PORT 0
#define GAME_ID_COMMAND     0x1D

    uint8_t out[10];
    uint8_t dummy[1];

    memcpy(out, crc_hi, 4);
    memcpy(&out[4], crc_lo, 4);

    out[8] = media_format;
    out[9] = country_id;

    execute_raw_command(N64_CONTROLLER_PORT, GAME_ID_COMMAND, sizeof(out), sizeof(dummy), out, dummy);
}

void get_country_and_region(unsigned char ccode, char *region_string)
{
    switch (ccode)
    {
        case '7':
            strcpy(region_string, "Beta");
            break;
        case 'A':
            strcpy(region_string, "Asia-NTSC");
            break;
        case 'B':
            strcpy(region_string, "Brazil-MPAL");
            break;
        case 'C':
            strcpy(region_string, "China-PAL");
            break;
        case 'D':
            strcpy(region_string, "Germany-PAL");
            break;
        case 'E':
            strcpy(region_string, "North America-NTSC");
            break;
        case 'F':
            strcpy(region_string, "France-PAL");
            break;
        case 'G':
            strcpy(region_string, "Gateway64-NTSC");
            break;
        case 'H':
            strcpy(region_string, "Netherlands-PAL");
            break;
        case 'I':
            strcpy(region_string, "Italy-PAL");
            break;
        case 'J':
            strcpy(region_string, "Japan-NTSC");
            break;
        case 'K':
            strcpy(region_string, "Korea-NTSC");
            break;
        case 'L':
            strcpy(region_string, "Gateway64-PAL");
            break;
        case 'N':
            strcpy(region_string, "Canada-NTSC");
            break;
        case 'P':
            strcpy(region_string, "Europe-PAL");
            break;
        case 'S':
            strcpy(region_string, "Spain-PAL");
            break;
        case 'U':
            strcpy(region_string, "Australia-PAL");
            break;
        case 'W':
            strcpy(region_string, "Scandinavia-PAL");
            break;
        case 'X':
            strcpy(region_string, "Europe-PAL");
            break;
        case 'Y':
            strcpy(region_string, "Europe-PAL");
            break;
        case 'Z':
            strcpy(region_string, "Europe-PAL");
            break;
        default:
            strcpy(region_string, "No Country");
    }
}
