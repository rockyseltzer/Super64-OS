#ifndef SAVE_DB_H
#define SAVE_DB_H

typedef struct
{
    char id[5];
    signed char ver;
    unsigned char save;
    unsigned char cpak;
} save_db_entry_t;

extern const save_db_entry_t save_db[];

int save_db_lookup(const unsigned char *id4, unsigned char ver, int *save, int *cpak);

#endif /* SAVE_DB_H */
