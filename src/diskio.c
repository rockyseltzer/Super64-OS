/*-----------------------------------------------------------------------*/
/* Low-level disk I/O glue for FatFs: the SD card                        */
/* Based on the FatFs R0.16 diskio.c skeleton, (C)ChaN, 2025             */
/*-----------------------------------------------------------------------*/
/* FatFs has a single drive (FF_VOLUMES == 1): physical drive 0 is the   */
/* SD card, accessed through the cart's SD driver in src/sd.c.           */
/*-----------------------------------------------------------------------*/

#include "ff.h"     /* Basic definitions of FatFs (diskio.h needs them first) */
#include "diskio.h" /* Declarations of the disk functions */

#include "sd.h" /* SD card driver: sdInit(), sdRead(), sdWrite() */

#define DEV_SD 0 /* Map the SD card to physical drive 0 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv /* Physical drive number to identify the drive */
)
{
    if (pdrv != DEV_SD)
        return STA_NOINIT;
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv /* Physical drive number to identify the drive */
)
{
    if (pdrv != DEV_SD)
        return STA_NOINIT;
    return sdInit() == 0 ? 0 : STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv,    /* Physical drive number to identify the drive */
                  BYTE *buff,   /* Data buffer to store read data */
                  LBA_t sector, /* Start sector in LBA */
                  UINT count    /* Number of sectors to read */
)
{
    if (pdrv != DEV_SD || !count)
        return RES_PARERR;
    return sdRead(sector, buff, count) == 0 ? RES_OK : RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv,        /* Physical drive number to identify the drive */
                   const BYTE *buff, /* Data to be written */
                   LBA_t sector,     /* Start sector in LBA */
                   UINT count        /* Number of sectors to write */
)
{
    if (pdrv != DEV_SD || !count)
        return RES_PARERR;
    return sdWrite(sector, buff, count) == 0 ? RES_OK : RES_ERROR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, /* Physical drive number (0..) */
                   BYTE cmd,  /* Control code */
                   void *buff /* Buffer to send/receive control data */
)
{
    if (pdrv != DEV_SD)
        return RES_PARERR;

    switch (cmd)
    {
        case CTRL_SYNC: /* sdWrite() completes each write before returning */
            return RES_OK;
        case GET_SECTOR_SIZE: /* Fixed 512-byte sectors (FF_MIN_SS == FF_MAX_SS) */
            *(WORD *) buff = 512;
            return RES_OK;
        case GET_SECTOR_COUNT: /* Only used by f_mkfs(), which is disabled */
        case GET_BLOCK_SIZE:
            return RES_OK;
        default:
            break;
    }
    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Timestamp for new and modified files (FF_FS_NORTC == 0)               */
/*-----------------------------------------------------------------------*/

DWORD get_fattime(void)
{
    return 0; /* The N64 has no real-time clock */
}
