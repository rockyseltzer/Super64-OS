Super 64 OS  (Altra64 3.6.9-s64.90 build, 2026-11-13)
=====================================================
Custom menu OS for the Super 64 (ED64Plus), a fork of Altra64.

Install
  1. Format the SD card as FAT32.
  2. Back up the card's existing ED64P folder, then copy this ED64P folder to
     the root of the card.
  3. The bootloader loads /ED64P/OS64P.v64 at every power-on. Nothing is
     written to the cartridge itself; restoring the backed-up OS64P.v64 undoes
     the install.

Folders
  ED64P/            OS64P.v64 and the folders below
  ED64P/SYS/        Settings (ALTRA64.INI), LASTROM.CFG, RECENT.LST,
                    FAVORITES.LST, AUTOLOAD.CFG, SELFTEST.TXT.
                    Delete the folder to reset the OS to defaults.
  ED64P/SAVES/      Game saves (.eep / .sra / .fla)
  ED64P/CFG/        Per-game settings (ROM Config)
  ED64P/CHEATS/     Cheat code files
  ED64P/MEMPAKS/    Controller Pak images
  ED64P/WALLPAPER/  Wallpapers
  ED64P/EMU/        Emulators (not included), named after the ROM extension
                    they handle: nes.v64, gb.v64, gbc.v64, sms.v64, gg.v64,
                    msx.v64. Download links are in the project README.

Controls
  A             Open folder / File Menu
  B             Back
  Z             Main Menu
  START         Start the last played game
  L / R         Previous / Next page
  Analog Stick  Navigation
  Hold B at Power-on: Skip Autoload

Menus
  File Menu (A on a file): Start Game, ROM Info, ROM Config, Delete, Add to
    Favorites, Set Autoload; save files: Copy File to RAM / Copy RAM to File;
    .mpk: CPak to File / File to CPak; images: Set Background / View Image;
    .txt: Load Cheats / View Text; .ips/.aps: Apply Patch.
  ROM Config (per game): Set CIC Type, Set Save Type, Set TV Type, Use
    Cheats, Cheat Editor, Use Patches, Hex View, Reset Defaults. CIC, save
    and TV type default to Automatic. Changes are saved when you press B.
  Main Menu (Z): Recently Played, Favorites, CPak Manager, System Folder,
    Device Info (START for details), Options, About.
  Options: TV Mode, Background, Page Number, Hide Sys Dir, Quick Boot, Fast
    Reboot, CRC Check, HDMI Game ID, Cart Speed, SD Speed, Save Ext Type,
    Save Backup, [Diagnostics], Help.

Notes
  General
    - Game saves are written to the SD card after a console reset (hardware
      limitation).
    - Set the NTSC / PAL switch on the cartridge to match your console; it
      selects the lockout chip region and is not read by the OS.
    - No FPGA core and no emulators are shipped.
    - A missing emulator shows "Emulator not found" when opening a ROM file.
  File Menu (A on a file)
    - Start Game: N64 ROMs start directly. NES, GB, GBC, SMS, GG and MSX ROMs
      start through an emulator in ED64P/EMU/ (not included).
    - ROM Config > Use Cheats: applies the game's GameShark codes when it
      starts. Needs an Expansion Pak.
    - ROM Config > Use Patches: applies an .ips or .aps file each time the
      game starts. The patch must be in the same folder as the ROM with
      exactly the same name (Game.z64 and Game.ips). The ROM file on the card
      is not changed.
    - Add to Favorites: adds the game to Main Menu > Favorites.
    - Set Autoload: the game starts at power-on (hold B to skip).
    - Set Background (on a PNG, BMP or JPG, 320x240 or 640x480): uses it as
      the wallpaper.
  Main Menu (Z)
    - Recently Played: the last 10 games started.
    - CPak Manager: back up, restore and format Controller Paks.
    - Device Info: cartridge, OS and console details.
  Options (Main Menu > Options)
    - Fast Reboot: relaunches the last played game after a console reset (hold
      B to skip).
    - HDMI Game ID: sends the game ID to PixelFX HDMI mods.
    - Cart Speed (Standard, Fast, Fastest): sets the cartridge bus timing the
      game reads the cart with. Fast and Fastest shorten in-game loading;
      Standard uses the timing in the ROM header. EXPERIMENTAL.
    - SD Speed (25 MHz, 50 MHz): the SD card clock, which sets how fast a ROM
      is copied from the card when a game starts.
    - Save Backup: keeps the three previous copies of each save (.0002 newest
      to .0004 oldest) each time a new save is written.
    - Diagnostics: board self-tests, written to ED64P/SYS/SELFTEST.TXT. The
      Self Test overwrites the ROM in cart RAM, so start a game afterwards
      normally.

Known Limitations
  - Diagnostics shows a warning for the save window's own read noise. This is
    expected; the OS no longer uses that path to load or back up saves.
  - Not supported on this hardware: 64DD disk images and anything that needs
    a different FPGA core.
  - Aleck 64 (CIC-5101): console-patched conversions work. Unmodified arcade
    ROM dumps start to a black screen.
