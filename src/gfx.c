#include "gfx.h"

#include "font.h"

extern void *__safe_buffer[];
extern uint32_t __width, __height;
#define FB(disp) ((uint32_t *) __safe_buffer[((int) (disp)) - 1])

display_context_t gfx_lock_display(void)
{
    display_context_t disp;
    do
    {
        disp = display_lock();
    } while (!disp);
    return disp;
}

int gfx_has_expansion_pak(void)
{
    return *(volatile uint32_t *) 0x80000318 >= 0x800000;
}

void s64_dim(display_context_t disp)
{
    if (!disp)
        return;
    uint32_t *fb = FB(disp);
    uint32_t n = __width * __height;
    for (uint32_t i = 0; i < n; i++)
        fb[i] = ((fb[i] >> 2) & 0x3F3F3F00u) | 0xFFu;
}

void s64_draw_cell(display_context_t disp, int x, int y, unsigned char ch, uint32_t fg, uint32_t bg, int opaque)
{
    if (!disp)
        return;
    uint32_t *fb = FB(disp);
    for (int row = 0; row < 8; row++)
    {
        unsigned char c = __font_data[(ch * 8) + row];
        int py = y + row;
        if (py < 0 || py >= (int) __height)
            continue;
        for (int col = 0; col < 8; col++, c <<= 1)
        {
            int on = c & 0x80, px = x + col;
            if ((!on && !opaque) || px < 0 || px >= (int) __width)
                continue;
            fb[py * __width + px] = on ? fg : bg;
        }
    }
}
