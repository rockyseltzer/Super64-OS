#include "strlib.h"

#include "types.h"

static char *strcpytrim(char *d_arg, char *s_arg, int mode, char *delim_arg)
{
    char *d = d_arg;
    char *s = s_arg;
    char *delim = delim_arg;
    char *o = d;
    char *e = 0;
    char dtab[256] = {0};
    if (!s || !d)
        return 0;

    if (!delim)
        delim = " \t\n\f";
    for (; *delim; delim++)
        dtab[(unsigned char) *delim] = 1;

    for (*d = *s, s++; *d != 0; *d = *s, s++)
    {
        if (!dtab[(unsigned char) *d])
        {
            e = 0;
        }
        else
        {
            if (!e)
                e = d;

            if (mode == STRLIB_MODE_ALL || ((mode != STRLIB_MODE_RIGHT) && (d == o)))
                continue;
        }
        d++;
    }
    if (mode != STRLIB_MODE_LEFT && e)
    {
        *e = 0;
    }
    return o;
}

char *trim(char *s)
{
    return strcpytrim(s, s, STRLIB_MODE_BOTH, 0);
}

u8 streql(const char *str1_arg, const char *str2_arg, u8 len)
{
    const char *str1 = str1_arg;
    const char *str2 = str2_arg;

    u8 s1;
    u8 s2;
    while (len--)
    {
        s1 = *str1;
        s2 = *str2;
        str1++;
        str2++;
        if (s1 >= 'a' && s1 <= 'z')
            s1 -= 0x20;
        if (s2 >= 'a' && s2 <= 'z')
            s2 -= 0x20;

        if (s1 != s2)
            return 0;
    }

    return 1;
}

u8 slen(const char *str)
{
    u8 len = 0;
    while (*str++)
        len++;
    return len;
}

u8 scopy(const char *src_arg, char *dst_arg)
{
    char *dst = dst_arg;
    const char *src = src_arg;

    u8 len = 0;
    while (*src != 0)
    {
        *dst = *src;
        dst++;
        src++;
        len++;
    }
    *dst = 0;
    return len;
}

void strhicase(char *str_arg, u8 len)
{
    char *str = str_arg;

    if (len)
    {
        while (len--)
        {
            if (*str >= 'a' && *str <= 'z')
                *str -= 0x20;
            str++;
        }
    }
    else
    {
        while (*str != 0)
        {
            if (*str >= 'a' && *str <= 'z')
                *str -= 0x20;
            str++;
        }
    }
}
