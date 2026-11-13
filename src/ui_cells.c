#include "ui_cells.h"

#include <stdio.h>
#include <string.h>

#include <libdragon.h>

#include "gfx.h"
#include "types.h"

#define C(r, g, b) ((u32) (((u32) (r) << 24) | ((u32) (g) << 16) | ((u32) (b) << 8) | 0xFF))

u32 ui_pal[8] = {
    C(0x00, 0x00, 0x00), C(0x83, 0x83, 0x83), C(0xFF, 0xFF, 0xFF), C(0xE6, 0xE6, 0x10),
    C(0x41, 0x41, 0x41), C(0xFF, 0x00, 0x00), C(0x00, 0xFF, 0x00), C(0x62, 0x62, 0x62),
};

#define G_HLINE 0xC4
#define G_VLINE 0xB3
#define G_TL    0xDA
#define G_TR    0xBF
#define G_BL    0xC0
#define G_BR    0xD9

static unsigned char ascii_only(unsigned char ch)
{
    static const char fold[] = "CueaaaaceeeiiiAAEaAooouuyOUcLYPfaiounN";
    switch (ch)
    {
        case 0x18:
        case 0x19:
        case 0xC4:
        case 0xB3:
        case 0xDA:
        case 0xBF:
        case 0xC0:
        case 0xD9:
        case 0xFE:
            return ch;
        default:
            if (ch >= 0x20 && ch < 0x7F)
                return ch;
            if (ch >= 0x80 && ch <= 0xA5)
                return (unsigned char) fold[ch - 0x80];
            return '?';
    }
}

void ui_cell(display_context_t d, int col, int row, unsigned char ch, int fg, int bg)
{
    if (!d || col < 0 || row < 0 || col >= UI_COLS || row >= UI_ROWS)
        return;
    s64_draw_cell(d, col * 8, row * 8, ascii_only(ch), ui_pal[fg & 7], ui_pal[bg & 7], bg != PAL_BLACK);
}

void ui_print(display_context_t d, int col, int row, const char *s, int fg, int bg)
{
    if (!s)
        return;
    for (; *s && col < UI_COLS; s++, col++)
        ui_cell(d, col, row, (unsigned char) *s, fg, bg);
}

void ui_print_n(display_context_t d, int col, int row, const char *s, int maxcols, int fg, int bg)
{
    if (!s)
        return;
    for (int i = 0; i < maxcols && col + i < UI_COLS && s[i]; i++)
        ui_cell(d, col + i, row, (unsigned char) s[i], fg, bg);
}

void ui_print_center(display_context_t d, int row, const char *s, int fg, int bg)
{
    int len = (int) strlen(s);
    ui_print(d, (UI_COLS - len) / 2, row, s, fg, bg);
}

void ui_fill(display_context_t d, int c0, int r0, int c1, int r1, unsigned char ch, int fg, int bg)
{
    for (int r = r0; r <= r1; r++)
        for (int c = c0; c <= c1; c++)
            ui_cell(d, c, r, ch, fg, bg);
}

void ui_hline(display_context_t d, int col, int row, int n, unsigned char ch, int fg, int bg)
{
    for (int i = 0; i < n; i++)
        ui_cell(d, col + i, row, ch, fg, bg);
}

void ui_black(display_context_t d)
{
    graphics_fill_screen(d, ui_pal[PAL_BLACK]);
}

static void rect_px(display_context_t d, int x0, int y0, int x1, int y1, u32 col)
{
    graphics_draw_line(d, x0, y0, x1, y0, col);
    graphics_draw_line(d, x0, y1, x1, y1, col);
    graphics_draw_line(d, x0, y0, x0, y1, col);
    graphics_draw_line(d, x1, y0, x1, y1, col);
}

void ui_frame(display_context_t d)
{
    rect_px(d, 20, 28, 299, 195, ui_pal[PAL_WHITE]);
    rect_px(d, 20, 204, 299, 227, ui_pal[PAL_WHITE]);
}

void ui_frame_title(display_context_t d, const char *t)
{
    int len = (int) strlen(t);
    int c0 = (UI_COLS - len) / 2;
    ui_fill(d, c0 - 1, 3, c0 + len, 3, ' ', PAL_GRAY, PAL_DARK);
    ui_print(d, c0, 3, t, PAL_GRAY, PAL_DARK);
}

static void clamp_rows(int *r0, int h)
{
    if (*r0 < 3)
        *r0 = 3;
    if (*r0 + h - 1 > 26)
        *r0 = 26 - h + 1;
    if (*r0 < 3)
        *r0 = 3;
}

void ui_page_line(display_context_t d, int page, int pages)
{
    char l[32];
    snprintf(l, sizeof(l), "Page: %d of %d", page, pages);
    ui_print(d, UI_LIST_COL, UI_PAGE_ROW, l, PAL_GRAY, PAL_BLACK);
}

void ui_bottom_name(display_context_t d, const char *name)
{
    int n = (int) strlen(name);
    ui_print_n(d, UI_LIST_COL, UI_BAR_ROW, name, UI_LIST_COLS, PAL_WHITE, PAL_BLACK);
    if (n > UI_LIST_COLS)
        ui_print_n(d, UI_LIST_COL, UI_BAR_ROW + 1, &name[UI_LIST_COLS], UI_LIST_COLS, PAL_WHITE, PAL_BLACK);
}

void ui_box(display_context_t d, int c0, int r0, int c1, int r1, const char *title)
{
    ui_fill(d, c0, r0, c1, r1, ' ', PAL_GRAY, PAL_DARK);
    ui_hline(d, c0, r0, c1 - c0 + 1, G_HLINE, PAL_GRAY, PAL_DARK);
    ui_hline(d, c0, r1, c1 - c0 + 1, G_HLINE, PAL_GRAY, PAL_DARK);
    for (int r = r0 + 1; r < r1; r++)
    {
        ui_cell(d, c0, r, G_VLINE, PAL_GRAY, PAL_DARK);
        ui_cell(d, c1, r, G_VLINE, PAL_GRAY, PAL_DARK);
    }
    ui_cell(d, c0, r0, G_TL, PAL_GRAY, PAL_DARK);
    ui_cell(d, c1, r0, G_TR, PAL_GRAY, PAL_DARK);
    ui_cell(d, c0, r1, G_BL, PAL_GRAY, PAL_DARK);
    ui_cell(d, c1, r1, G_BR, PAL_GRAY, PAL_DARK);
    if (title && *title)
    {
        int len = (int) strlen(title);
        int tc = (c0 + c1 + 1) / 2 - len / 2;
        ui_print(d, tc, r0, title, PAL_GRAY, PAL_DARK);
    }
}

static void cell_px(display_context_t d, int col, int y, unsigned char ch, int fg, int bg)
{
    if (!d || col < 0 || col >= UI_COLS || y < 0 || y > 232)
        return;
    s64_draw_cell(d, col * 8, y, ascii_only(ch), ui_pal[fg & 7], ui_pal[bg & 7], bg != PAL_BLACK);
}
static void print_px(display_context_t d, int col, int y, const char *s, int fg, int bg)
{
    for (; *s && col < UI_COLS; s++, col++)
        cell_px(d, col, y, (unsigned char) *s, fg, bg);
}
static void box_px(display_context_t d, int c0, int y0, int w, int h, const char *title)
{
    graphics_draw_box(d, c0 * 8, y0, w * 8, h, ui_pal[PAL_DARK]);
    for (int c = 0; c < w; c++)
    {
        cell_px(d, c0 + c, y0, G_HLINE, PAL_GRAY, PAL_DARK);
        cell_px(d, c0 + c, y0 + h - 8, G_HLINE, PAL_GRAY, PAL_DARK);
    }
    for (int y = y0 + 8; y <= y0 + h - 16; y += 8)
    {
        cell_px(d, c0, y, G_VLINE, PAL_GRAY, PAL_DARK);
        cell_px(d, c0 + w - 1, y, G_VLINE, PAL_GRAY, PAL_DARK);
    }
    cell_px(d, c0, y0, G_TL, PAL_GRAY, PAL_DARK);
    cell_px(d, c0 + w - 1, y0, G_TR, PAL_GRAY, PAL_DARK);
    cell_px(d, c0, y0 + h - 8, G_BL, PAL_GRAY, PAL_DARK);
    cell_px(d, c0 + w - 1, y0 + h - 8, G_BR, PAL_GRAY, PAL_DARK);
    if (title && *title)
    {
        int len = (int) strlen(title);
        print_px(d, c0 + (w - len) / 2, y0, title, PAL_GRAY, PAL_DARK);
    }
}
static int menu_geom(int n, int *pitch, int *h)
{
    *pitch = 8;
    for (int p = 12; p > 8; p--)
        if (10 + p * n + 6 <= 192)
        {
            *pitch = p;
            break;
        }
    int hh = 10 + n * *pitch + 6;
    hh = (hh + 7) & ~7;
    int y0 = 120 - hh / 2;
    if (y0 + hh > 216)
        y0 = 216 - hh;
    if (y0 < 24)
        y0 = 24;
    *h = hh;
    return y0;
}

static int widest(const char *title, const char *const *items, int n)
{
    int m = title ? (int) strlen(title) : 0;
    for (int i = 0; i < n; i++)
        if (items[i] && (int) strlen(items[i]) > m)
            m = (int) strlen(items[i]);
    return m;
}

void ui_menu(display_context_t d, const char *title, const char *const *items, int n, int sel, int (*enabled)(int))
{
    int w = widest(title, items, n) + 2;
    int pitch, h;
    int y0 = menu_geom(n, &pitch, &h);
    int c0 = (UI_COLS - w) / 2;
    if (c0 < 0)
        c0 = 0;
    box_px(d, c0, y0, w, h, title);
    for (int i = 0; i < n; i++)
    {
        int en = enabled ? enabled(i) : 1;
        int fg = (i == sel) ? PAL_WHITE : en ? PAL_YELLOW : PAL_GRAY2;
        int len = (int) strlen(items[i]);
        print_px(d, c0 + (w - len) / 2, y0 + 8 + (pitch - 8) / 2 + (h - 16 - n * pitch) / 2 + i * pitch, items[i], fg,
                 PAL_DARK);
    }
}

void ui_kv_scroll(display_context_t d, const char *title, const char **labels, const char **values, int n, int sel,
                  int first, int maxrows)
{
    int lw = 0, vw = 0, solo = 0;
    for (int i = 0; i < n; i++)
    {
        if (labels[i][0] == UI_SEP)
            continue;
        int len = (int) strlen(labels[i]);
        if (values && values[i])
        {
            if (len > lw)
                lw = len;
            if ((int) strlen(values[i]) > vw)
                vw = (int) strlen(values[i]);
        }
        else if (len > solo)
            solo = len;
    }
    int cols = vw ? lw + 3 + vw : lw;
    if (solo > cols)
        cols = solo;
    int tl = title ? (int) strlen(title) : 0;
    if (tl > cols)
        cols = tl;
    int w = cols + 2;
    int nvis = (n < maxrows) ? n : maxrows;
    int pitch, h;
    int y0 = menu_geom(nvis, &pitch, &h);
    int c0 = (UI_COLS - w) / 2;
    if (c0 < 0)
        c0 = 0;
    box_px(d, c0, y0, w, h, title);
    for (int r = 0; r < nvis; r++)
    {
        int i = first + r;
        if (i >= n)
            break;
        int y = y0 + 8 + (pitch - 8) / 2 + (h - 16 - nvis * pitch) / 2 + r * pitch;
        if (labels[i][0] == UI_SEP)
        {
            for (int c = 1; c < w - 1; c++)
                cell_px(d, c0 + c, y, G_HLINE, PAL_GRAY2, PAL_DARK);
            continue;
        }
        char line[48];
        if (values && values[i])
            snprintf(line, sizeof(line), "%-*s : %s", lw, labels[i], values[i]);
        else
            snprintf(line, sizeof(line), "%s", labels[i]);
        print_px(d, c0 + 1, y, line, (i == sel) ? PAL_WHITE : PAL_YELLOW, PAL_DARK);
    }
    if (first > 0)
        cell_px(d, c0 + w - 1, y0 + 8, 0x18, PAL_WHITE, PAL_DARK);
    if (first + nvis < n)
        cell_px(d, c0 + w - 1, y0 + h - 16, 0x19, PAL_WHITE, PAL_DARK);
}

void ui_kv(display_context_t d, const char *title, const char **labels, const char **values, int n, int sel)
{
    ui_kv_scroll(d, title, labels, values, n, sel, 0, n);
}

void ui_print_center_px(display_context_t d, int y, const char *s, int fg, int bg)
{
    print_px(d, (UI_COLS - (int) strlen(s)) / 2, y, s, fg, bg);
}

void ui_yesno(display_context_t d, const char *title, int sel)
{
    int w = (int) strlen(title) + 2;
    if (w < 12)
        w = 12;
    int c0 = (UI_COLS - w) / 2, r0 = UI_CENTER_ROW - 1;
    ui_box(d, c0, r0, c0 + w - 1, r0 + 2, title);
    ui_print(d, 16, UI_CENTER_ROW, "YES", (sel == 0) ? PAL_WHITE : PAL_GRAY, PAL_DARK);
    ui_print(d, 21, UI_CENTER_ROW, "NO", (sel == 1) ? PAL_WHITE : PAL_GRAY, PAL_DARK);
}

void ui_message(display_context_t d, const char *l1, const char *l2)
{
    int w = (int) strlen(l1) + 2;
    if (l2 && (int) strlen(l2) + 2 > w)
        w = (int) strlen(l2) + 2;
    int c0 = (UI_COLS - w) / 2;
    int r1 = l2 ? UI_CENTER_ROW + 1 : UI_CENTER_ROW;
    ui_fill(d, c0, UI_CENTER_ROW - 1, c0 + w - 1, r1, ' ', PAL_WHITE, PAL_DARK);
    ui_print_center(d, UI_CENTER_ROW - 1, l1, PAL_WHITE, PAL_DARK);
    if (l2)
        ui_print_center(d, UI_CENTER_ROW, l2, PAL_YELLOW, PAL_DARK);
}

void ui_dashed(display_context_t d, int row)
{
    ui_hline(d, 2, row, 36, '-', PAL_GRAY, PAL_BLACK);
}

void ui_text_marked(display_context_t d, int col, int row, const char *s)
{
    static const struct
    {
        const char *w;
        int pal;
    } tok[] = {{"PASSED", PAL_GREEN}, {"FAILED", PAL_RED}, {"CORRUPT", PAL_RED}, {"PASS", PAL_GREEN},
               {"OK", PAL_GREEN},     {"FAIL", PAL_RED},   {"ERROR", PAL_RED},   {"ERR", PAL_RED}};
    ui_print(d, col, row, s, PAL_GRAY, PAL_BLACK);
    char done[64] = {0};
    for (unsigned t = 0; t < sizeof(tok) / sizeof(tok[0]); t++)
    {
        const char *p = s;
        for (p = strstr(p, tok[t].w); p != NULL; p = strstr(p, tok[t].w))
        {
            int at = (int) (p - s), len = (int) strlen(tok[t].w);
            if (at < 64 && !done[at])
            {
                ui_print(d, col + at, row, tok[t].w, tok[t].pal, PAL_BLACK);
                for (int k = at; k < at + len && k < 64; k++)
                    done[k] = 1;
            }
            p += len;
        }
    }
}
