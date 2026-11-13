#ifndef UI_CELLS_H
#define UI_CELLS_H

#include <libdragon.h>

#include "types.h"

#define UI_COLS 40
#define UI_ROWS 30

#define PAL_BLACK  0
#define PAL_GRAY   1
#define PAL_WHITE  2
#define PAL_YELLOW 3
#define PAL_DARK   4
#define PAL_RED    5
#define PAL_GREEN  6
#define PAL_GRAY2  7

#define UI_LIST_COL   3
#define UI_LIST_ROW   4
#define UI_LIST_COLS  34
#define UI_PAGE_ROW   2
#define UI_BAR_ROW    26
#define UI_CENTER_ROW 14

#define UI_SEP '\x01'

extern u32 ui_pal[8];

void ui_cell(display_context_t d, int col, int row, unsigned char ch, int fg, int bg);
void ui_print(display_context_t d, int col, int row, const char *s, int fg, int bg);
void ui_print_center(display_context_t d, int row, const char *s, int fg, int bg);
void ui_print_n(display_context_t d, int col, int row, const char *s, int maxcols, int fg, int bg);
void ui_fill(display_context_t d, int c0, int r0, int c1, int r1, unsigned char ch, int fg, int bg);
void ui_hline(display_context_t d, int col, int row, int n, unsigned char ch, int fg, int bg);

void ui_black(display_context_t d);
void ui_frame(display_context_t d);
void ui_frame_title(display_context_t d, const char *t);
void ui_page_line(display_context_t d, int page, int pages);
void ui_bottom_name(display_context_t d, const char *name);
void ui_box(display_context_t d, int c0, int r0, int c1, int r1, const char *title);
void ui_menu(display_context_t d, const char *title, const char *const *items, int n, int sel, int (*enabled)(int));
void ui_kv(display_context_t d, const char *title, const char **labels, const char **values, int n, int sel);

void ui_kv_scroll(display_context_t d, const char *title, const char **labels, const char **values, int n, int sel,
                  int first, int maxrows);
void ui_print_center_px(display_context_t d, int y, const char *s, int fg, int bg);
void ui_yesno(display_context_t d, const char *title, int sel);
void ui_message(display_context_t d, const char *l1, const char *l2);
void ui_dashed(display_context_t d, int row);
void ui_text_marked(display_context_t d, int col, int row, const char *s);

#endif /* UI_CELLS_H */
