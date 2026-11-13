#ifndef MENU_ED64_H
#define MENU_ED64_H

#include <libdragon.h>

#include "constants.h"
#include "menu.h"
#include "types.h"
#include "version.h"

#define ED64_FILE_IMAGE 1
#define ED64_FILE_PATCH 2
#define ED64_FILE_TEXT  4
#define ED64_FILE_SAVE  8
#define ED64_FILE_MPK   16

typedef struct
{
    uint32_t type;
    char filename[MAX_FILENAME_LEN + 1];
} direntry_t;

enum Ed64Action
{
    ED64_ACT_NONE = 0,       // Still inside a menu screen
    ED64_ACT_CLOSE,          // Leave the menu, redraw the browser
    ED64_ACT_START,          // File Menu: Start Game
    ED64_ACT_SET_WALLPAPER,  // File Menu: Set Background (path in ed64menu_selected_path())
    ED64_ACT_CHEATS,         // ROM Config: the per-ROM config box
    ED64_ACT_LOAD_PATH,      // Load ed64menu_selected_path() (Recently Played)
    ED64_ACT_APPLY_PATCH,    // File Menu on .ips/.aps: queue for the next ROM start
    ED64_ACT_DELETE,         // Delete ed64menu_selected_path(), re-read the folder
    ED64_ACT_REFRESH,        // Re-read the folder, leave the menu
    ED64_ACT_CPAK_SAVE_NUM,  // CPak Manager: save pak on cpak_port as controller-N.mpk
    ED64_ACT_CPAK_SAVE_GAME, // CPak Manager: save pak on cpak_port as <last game>.mpk
    ED64_ACT_CPAK_FORMAT,    // CPak Manager: format pak on cpak_port
    ED64_ACT_SYS_FOLDER,     // Main Menu: System Folder (go to /ED64P)
    ED64_ACT_FILE_TO_RAM,    // Save RAM menu: copy the selected save file into the cart's save RAM
    ED64_ACT_RAM_TO_FILE,    // Save RAM menu: dump the cart's save RAM into the selected file
    ED64_ACT_CPAK_TO_FILE,   // Controller Pak memory menu: pak on cpak_port -> selected .mpk
    ED64_ACT_FILE_TO_CPAK,   // Controller Pak memory menu: selected .mpk -> pak on cpak_port
};

extern direntry_t *list;

void ed64menu_open_file_menu(display_context_t disp, const char *fullpath, const char *shortname, int is_rom,
                             int flags);
void ed64menu_open_main_menu(display_context_t disp);
void ed64menu_message(const char *l1, const char *l2);
int ed64menu_handle(display_context_t disp, struct controller_data *keys, struct controller_data *held);
const char *ed64menu_selected_path(void);

void ed64menu_recent_add(const char *fullpath);

void ed64_redraw_browser(display_context_t disp);
void clearScreen(display_context_t disp);
sprite_t *read_sprite(const char *const spritename);
sprite_t *loadPng(char *png_filename);

#endif /* MENU_ED64_H */
