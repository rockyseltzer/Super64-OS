#include "ini_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"

#define INI_MAX 8192

static int starts_key(const char *line, const char *key)
{
    size_t i = 0;
    size_t n = strlen(key);
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    if (strncmp(&line[i], key, n) != 0)
        return 0;
    i += n;
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    return line[i] == '=';
}

static size_t line_len(const char *buf, size_t pos)
{
    size_t len = 0;
    while (buf[pos + len] != '\0' && buf[pos + len] != '\n')
        len++;
    if (buf[pos + len] == '\n')
        len++;
    return len;
}

static size_t skip_blanks(const char *buf, size_t pos)
{
    while (buf[pos] == ' ' || buf[pos] == '\t')
        pos++;
    return pos;
}

static int is_section(const char *buf, size_t pos, const char *section)
{
    size_t n = strlen(section);
    return strncmp(&buf[pos + 1], section, n) == 0 && buf[pos + 1 + n] == ']';
}

int ini_set_value(const char *path, const char *section, const char *key, const char *value)
{
    FIL f;
    UINT n = 0;
    char *buf = malloc(INI_MAX + 1);
    char *out = malloc(INI_MAX + 256);
    if (!buf || !out)
    {
        free(buf);
        free(out);
        return -1;
    }

    if (f_open(&f, path, FA_READ) == FR_OK)
    {
        f_read(&f, buf, INI_MAX, &n);
        f_close(&f);
    }
    buf[n] = 0;

    size_t on = 0;
    size_t pos = 0;
    int in_section = 0, done = 0;
    while (buf[pos] != '\0')
    {
        size_t len = line_len(buf, pos);
        size_t k = skip_blanks(buf, pos);
        if (buf[k] == '[')
        {
            if (in_section && !done)
            {
                on += (size_t) sprintf(&out[on], "%s=%s\n", key, value);
                done = 1;
            }
            in_section = is_section(buf, k, section);
            memcpy(&out[on], &buf[pos], len);
            on += len;
        }
        else if (in_section && !done && starts_key(&buf[k], key))
        {
            size_t c = 0;
            while (c < len && buf[pos + c] != ';')
                c++;
            on += (size_t) sprintf(&out[on], "%s=%s", key, value);
            if (c < len)
            {
                out[on] = '\t';
                on++;
                memcpy(&out[on], &buf[pos + c], len - c);
                on += len - c;
            }
            else
            {
                out[on] = '\n';
                on++;
            }
            done = 1;
        }
        else
        {
            memcpy(&out[on], &buf[pos], len);
            on += len;
        }
        pos += len;
    }
    if (!done)
    {
        if (!in_section)
            on += (size_t) sprintf(&out[on], "\n[%s]\n", section);
        on += (size_t) sprintf(&out[on], "%s=%s\n", key, value);
    }

    int rc = -1;
    if (f_open(&f, path, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
        UINT w = 0;
        f_write(&f, out, (UINT) on, &w);
        f_close(&f);
        rc = (w == (UINT) on) ? 0 : -1;
    }
    free(buf);
    free(out);
    return rc;
}

int ini_get_value(const char *path, const char *section, const char *key, char *out, int outlen)
{
    FIL f;
    UINT n = 0;
    char *buf = malloc(INI_MAX + 1);
    if (!buf)
        return -1;
    if (f_open(&f, path, FA_READ) != FR_OK)
    {
        free(buf);
        return -1;
    }
    f_read(&f, buf, INI_MAX, &n);
    f_close(&f);
    buf[n] = 0;
    int in_section = 0, rc = -1;
    size_t pos = 0;
    while (buf[pos] != '\0' && rc != 0)
    {
        size_t len = line_len(buf, pos);
        size_t k = skip_blanks(buf, pos);
        if (buf[k] == '[')
            in_section = is_section(buf, k, section);
        else if (in_section && starts_key(&buf[k], key))
        {
            size_t v = k;
            while (buf[v] != '=')
                v++;
            v = skip_blanks(buf, v + 1);
            int i = 0;
            while (i < outlen - 1 && buf[v + i] && buf[v + i] != ';' && buf[v + i] != '\r' && buf[v + i] != '\n' &&
                   buf[v + i] != ' ' && buf[v + i] != '\t')
            {
                out[i] = buf[v + i];
                i++;
            }
            out[i] = 0;
            rc = 0;
        }
        pos += len;
    }
    free(buf);
    return rc;
}
