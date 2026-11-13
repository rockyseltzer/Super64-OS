#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

#include <libdragon.h>

extern int text_offset;

void printText(char *msg, int x, int y, display_context_t dcon);

#endif /* MENU_H */
