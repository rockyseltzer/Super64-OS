#include "menu.h"

#include <libdragon.h>

#include "debug.h"
#include "gfx.h"
#include "types.h"

static short int gCursorX;
static short int gCursorY;

int text_offset = 0;

void printText(char *msg, int x_arg, int y, display_context_t dcon)
{
    int x = x_arg;
    x = x + text_offset;

    if (x != -1)
        gCursorX = x;
    if (y != -1)
        gCursorY = y;

    if (dcon)
        graphics_draw_text(dcon, gCursorX * 8, gCursorY * 8, msg);

    gCursorY++;
    if (gCursorY > 29)
    {
        gCursorY = 0;
        gCursorX++;
    }
}
