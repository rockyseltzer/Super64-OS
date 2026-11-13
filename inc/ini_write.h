#ifndef INI_WRITE_H
#define INI_WRITE_H

int ini_set_value(const char *path, const char *section, const char *key, const char *value);
int ini_get_value(const char *path, const char *section, const char *key, char *out, int outlen);

#endif /* INI_WRITE_H */
