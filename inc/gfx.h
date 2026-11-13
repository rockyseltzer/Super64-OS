#ifndef GFX_H
#define GFX_H

#include <libdragon.h>

int gfx_has_expansion_pak(void);
display_context_t gfx_lock_display(void);
void s64_dim(display_context_t disp);
void s64_draw_cell(display_context_t disp, int x, int y, unsigned char ch, uint32_t fg, uint32_t bg, int opaque);

#endif /* GFX_H */
