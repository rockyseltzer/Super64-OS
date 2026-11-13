#ifndef CHEATS_TABLE_H
#define CHEATS_TABLE_H

#include "types.h"

#define CH_MAX      34
#define CH_DESC_LEN 32

typedef struct
{
    u32 addr;
    u16 val;
    u8 on;
    char desc[CH_DESC_LEN];
} cheat_code_t;

extern cheat_code_t ch_codes[CH_MAX];
extern char ch_game[64];
extern int ch_dirty;

int cheats_load(const char *game);
int cheats_save(void);
int cheats_import(const char *txt_path, const char *game);
int cheats_code_supported(u32 addr);
int cheats_build_lists(const char *game, u32 *lists[2]);
void cheats_clear(int i);

#endif /* CHEATS_TABLE_H */
