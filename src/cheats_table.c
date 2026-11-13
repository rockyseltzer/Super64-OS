#include "cheats_table.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"

#include "constants.h"
#include "types.h"

cheat_code_t ch_codes[CH_MAX];
char ch_game[64];
int ch_dirty = 0;

static void path_for(const char *game, char *out, size_t n)
{
    snprintf(out, n, "/" ED64_FIRMWARE_PATH "/CHEATS/%s.txt", game);
}

static int hexval(int c_arg)
{
    int c = c_arg;
    if (c >= '0' && c <= '9')
        return c - '0';
    c = toupper(c);
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static int parse_line(const char *l_arg, cheat_code_t *c)
{
    const char *l = l_arg;
    while (*l == ' ' || *l == '\t')
        l++;
    int on = 1;
    if (*l == '#' || *l == ';')
    {
        on = 0;
        l++;
        while (*l == ' ' || *l == '\t')
            l++;
    }
    u32 a = 0;
    int i;
    for (i = 0; i < 8; i++)
    {
        int h = hexval(l[i]);
        if (h < 0)
            return 0;
        a = (a << 4) | h;
    }
    l = &l[8];
    if (*l != ' ' && *l != '\t')
        return 0;
    while (*l == ' ' || *l == '\t')
        l++;
    u32 v = 0;
    for (i = 0; i < 4; i++)
    {
        int h = hexval(l[i]);
        if (h < 0)
            return 0;
        v = (v << 4) | h;
    }
    l = &l[4];
    while (*l == ' ' || *l == '\t')
        l++;
    c->addr = a;
    c->val = (u16) v;
    c->on = on;
    strncpy(c->desc, l, CH_DESC_LEN - 1);
    c->desc[CH_DESC_LEN - 1] = 0;
    size_t e = strlen(c->desc);
    while (e > 0 && (c->desc[e - 1] == '\r' || c->desc[e - 1] == '\n' || c->desc[e - 1] == ' '))
    {
        e--;
        c->desc[e] = 0;
    }
    return 1;
}

static int load_file_into(const char *path, cheat_code_t *tab)
{
    FIL f;
    char line[160];
    int n = 0;
    memset(tab, 0, sizeof(cheat_code_t) * CH_MAX);
    if (f_open(&f, path, FA_READ) != FR_OK)
        return 0;
    while (n < CH_MAX && f_gets(line, sizeof(line), &f))
        if (parse_line(line, &tab[n]))
            n++;
    f_close(&f);
    return n;
}

int cheats_load(const char *game)
{
    char path[MAX_SUPPORTED_PATH_LEN];
    strncpy(ch_game, game, sizeof(ch_game) - 1);
    ch_game[sizeof(ch_game) - 1] = 0;
    path_for(ch_game, path, sizeof(path));
    ch_dirty = 0;
    return load_file_into(path, ch_codes);
}

int cheats_save(void)
{
    char path[MAX_SUPPORTED_PATH_LEN];
    FIL f;
    path_for(ch_game, path, sizeof(path));
    f_mkdir("/" ED64_FIRMWARE_PATH "/CHEATS");
    if (f_open(&f, path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return -1;
    for (int i = 0; i < CH_MAX; i++)
    {
        cheat_code_t *c = &ch_codes[i];
        if (!c->addr && !c->val && !c->desc[0])
            continue;
        char line[64];
        snprintf(line, sizeof(line), "%s%08lX %04X %s\n", c->on ? "" : "#", (unsigned long) c->addr, c->val, c->desc);
        f_puts(line, &f);
    }
    f_close(&f);
    ch_dirty = 0;
    return 0;
}

int cheats_import(const char *txt_path, const char *game)
{
    int n = load_file_into(txt_path, ch_codes);
    strncpy(ch_game, game, sizeof(ch_game) - 1);
    ch_game[sizeof(ch_game) - 1] = 0;
    if (cheats_save() != 0)
        return -1;
    return n;
}

int cheats_code_supported(u32 addr)
{
    switch (addr >> 24)
    {
        case 0x80:
        case 0x81:
        case 0xA0:
        case 0xA1:
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0x50:
        case 0xEE:
        case 0xF0:
        case 0xF1:
        case 0xFF:
        case 0x20:
        case 0xCC:
        case 0xDE:
            return 1;
        default:
            return 0;
    }
}

void cheats_clear(int i)
{
    if (i >= 0 && i < CH_MAX)
    {
        memset(&ch_codes[i], 0, sizeof(cheat_code_t));
        ch_dirty = 1;
    }
}

int cheats_build_lists(const char *game, u32 *lists[2])
{
    static cheat_code_t tab[CH_MAX];
    char path[MAX_SUPPORTED_PATH_LEN];
    path_for(game, path, sizeof(path));
    int n = load_file_into(path, tab);
    lists[0] = NULL;
    lists[1] = NULL;
    if (n <= 0)
        return 0;
    u32 *l1 = calloc(2 * (CH_MAX + 1) * 2, sizeof(u32));
    if (!l1)
        return -2;
    u32 *l2 = l1 + 2 * (CH_MAX + 1);
    u32 *p1 = l1, *p2 = l2;
    int used = 0, repeater = 0, skip_next = 0, cond_used = 0;
    u32 *cond_pos = NULL;
    for (int i = 0; i < n; i++)
    {
        cheat_code_t *c = &tab[i];
        if (!c->on || (!c->addr && !c->val))
            continue;
        u32 a = c->addr, v = c->val;
        if (repeater)
        {
            repeater--;
            cond_pos = NULL;
            p2[0] = a;
            p2[1] = v;
            p2 = &p2[2];
            used++;
            continue;
        }
        int is_cond = ((a >> 24) >= 0xD0) && ((a >> 24) <= 0xD3);
        switch (a >> 24)
        {
            case 0x20:
            case 0xCC:
            case 0xDE:
                break;
            case 0xEE:
            case 0xF0:
            case 0xF1:
            case 0xFF:
                p1[0] = a;
                p1[1] = v;
                p1 = &p1[2];
                used++;
                break;
            case 0x50:
                if (a & 0x0000FF00)
                {
                    repeater = 1;
                    cond_pos = NULL;
                    p2[0] = a;
                    p2[1] = v;
                    p2 = &p2[2];
                    used++;
                }
                break;
            case 0xD0:
            case 0xD1:
            case 0xD2:
            case 0xD3:
            case 0x80:
            case 0x81:
            case 0xA0:
            case 0xA1:
                if (skip_next)
                {
                    skip_next = is_cond;
                    break;
                }
                if ((a & 0x01000001) == 0x01000001)
                {
                    if (is_cond)
                        skip_next = 1;
                    else if (cond_pos)
                    {
                        p2 = cond_pos;
                        used = cond_used;
                        cond_pos = NULL;
                    }
                    break;
                }
                if (!is_cond)
                    cond_pos = NULL;
                else if (!cond_pos)
                {
                    cond_pos = p2;
                    cond_used = used;
                }
                p2[0] = a;
                p2[1] = v;
                p2 = &p2[2];
                used++;
                break;
            default:
                break;
        }
    }
    p1[0] = 0;
    p1[1] = 0;
    p2[0] = 0;
    p2[1] = 0;
    if (!used)
    {
        free(l1);
        return 0;
    }
    lists[0] = l1;
    lists[1] = l2;
    return used;
}
