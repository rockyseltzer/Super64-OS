#ifndef DIAG_H
#define DIAG_H

#include "types.h"

#define DIAG_MAX_LINES 96
#define DIAG_LINE_LEN  72

extern char diag_lines[DIAG_MAX_LINES][DIAG_LINE_LEN];
extern int diag_nlines;
extern int diag_failed;

void diag_run(void (*progress)(const char *));

#endif /* DIAG_H */
