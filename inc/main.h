#ifndef MAIN_H
#define MAIN_H

#include <libdragon.h>

#include "types.h"

extern u8 cart_speed;
extern u8 background_on, crc_check, fast_reboot, game_id_on;
extern u32 boot_counter, game_counter;
extern u8 page_display, tv_mode, quick_boot, ext_type, sd_speed, hide_sysfolder, save_backup;
extern short int gCheats, force_tv;
extern char pwd[];
extern int count, cursor;

#endif /* MAIN_H */
