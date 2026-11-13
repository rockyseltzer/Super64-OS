#include "memorypak.h"

#include <stdio.h>
#include <string.h>

#include <libdragon.h>

#include "ff.h"

#include "constants.h"
#include "debug.h"
#include "mempak.h"
#include "menu.h"
#include "strlib.h"
#include "sys.h"
#include "types.h"

enum MemoryPakFormat
{
    None,
    DexDrive,
    Z64
};

static uint8_t mempak_data[128 * 256];
char *mempak_path;
int cpak_port = 0;

static void mempak_dir(char *out, size_t n)
{
    const char *p = mempak_path ? mempak_path : "MEMPAKS";
    if (p[0] == '/')
        snprintf(out, n, "%s", p);
    else
        snprintf(out, n, "/" ED64_FIRMWARE_PATH "/%s", p);
    size_t l = strlen(out);
    while (l > 1 && out[l - 1] == '/')
        out[--l] = 0;
}

int ed64_detect_pak(int port)
{
    int acc = identify_accessory(port);
    if (acc != ACCESSORY_RUMBLEPAK)
        return acc;
    uint8_t blk[32];
    if (read_mempak_address(port, 0x0000, blk) == 0)
    {
        int uniform = 1;
        for (int i = 1; i < 32; i++)
            if (blk[i] != blk[0])
            {
                uniform = 0;
                break;
            }
        if (!uniform || (blk[0] != 0x00 && blk[0] != 0x80 && blk[0] != 0xFF))
            return ACCESSORY_MEMPAK;
    }
    return ACCESSORY_RUMBLEPAK;
}

int file_to_mpk(display_context_t disp, char *filename)
{
    enum MemoryPakFormat memorypak_format;
    char buff[64];

    FRESULT result;
    FIL file;
    UINT bytesread;
    result = f_open(&file, filename, FA_READ);

    if (result == FR_OK)
    {
        char *pch;
        pch = strrchr(filename, '.');
        sprintf(buff, "%s", (pch + 2));

        if (strcmp(buff, "64") == 0)
        {
            TRACE(disp, "DexDrive format");
            memorypak_format = DexDrive;
            result = f_lseek(&file, 4160);
        }

        TRACE(disp, "Z64 format");
        memorypak_format = Z64;

        result = f_read(&file, mempak_data, 32768, &bytesread);

        f_close(&file);

        for (int j = 0; j < 128; j++)
        {
            write_mempak_sector(cpak_port, j, &mempak_data[j * MEMPAK_BLOCK_SIZE]);
        }
    }
    else
    {
        memorypak_format = None;
    }

    return (int) memorypak_format;
}

int mpk_dump(int port, const char *base)
{
    char dir[MAX_SUPPORTED_PATH_LEN], path[MAX_SUPPORTED_PATH_LEN];
    mempak_dir(dir, sizeof(dir));
    f_mkdir(dir);
    snprintf(path, sizeof(path), "%s/%s.mpk", dir, base);
    return mpk_dump_to(port, path);
}

int mpk_dump_to(int port, const char *path)
{
    controller_init();
    int err = 0;
    for (int j = 0; j < 128; j++)
        err |= read_mempak_sector(port, j, &mempak_data[j * 256]);
    FIL file;
    if (f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return -1;
    UINT bw;
    f_write(&file, mempak_data, 32768, &bw);
    f_close(&file);
    return err ? 1 : 0;
}
