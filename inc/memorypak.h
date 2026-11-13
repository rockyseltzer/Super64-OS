#ifndef MEMORYPAK_H
#define MEMORYPAK_H

#include <libdragon.h>

#include "types.h"

extern char *mempak_path;
extern int cpak_port;

int ed64_detect_pak(int port);

int file_to_mpk(display_context_t disp, char *filename);
int mpk_dump(int port, const char *base);
int mpk_dump_to(int port, const char *path);

#endif /* MEMORYPAK_H */
