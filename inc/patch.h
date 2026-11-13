#ifndef PATCH_H
#define PATCH_H

#include "types.h"

enum
{
    PATCH_OK = 0,
    PATCH_ERR_OPEN = 1,
    PATCH_ERR_BAD_IPS = 2,
    PATCH_ERR_BAD_APS = 3,
    PATCH_ERR_UNKNOWN = 4,
    PATCH_ERR_RANGE = 5
};

int patch_apply(const char *path, u32 rom_size, void (*progress)(int pct));
const char *patch_error_text(int err);

#endif /* PATCH_H */
