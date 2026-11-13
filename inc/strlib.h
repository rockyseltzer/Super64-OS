#ifndef STRLIB_H
#define STRLIB_H

#include "types.h"

enum strtrim_mode_t
{
    STRLIB_MODE_ALL = 0,
    STRLIB_MODE_RIGHT = 0x01,
    STRLIB_MODE_LEFT = 0x02,
    STRLIB_MODE_BOTH = 0x03
};

char *trim(char *s);

void strhicase(char *str, u8 len);
u8 slen(const char *str);
u8 scopy(const char *src, char *dst);

u8 streql(const char *str1, const char *str2, u8 len);

#endif /* STRLIB_H */
