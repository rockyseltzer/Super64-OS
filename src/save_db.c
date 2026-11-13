#include "save_db.h"

#include <string.h>

const save_db_entry_t save_db[] = {
    {"NK4J",  0, 1, 0}, // Kirby 64: The Crystal Shards [Hoshi no Kirby 64 (J)]
    {"NK4J",  1, 1, 0}, // Kirby 64: The Crystal Shards [Hoshi no Kirby 64 (J)]
    {"NK4", -1, 3, 0}, // Kirby 64: The Crystal Shards [Hoshi no Kirby 64 (J)]
    {"NSMJ",  3, 3, 0}, // Super Mario 64 Shindou Edition
    {"NSM", -1, 3, 0}, // Super Mario 64
    {"NWRJ",  2, 3, 1}, // Wave Race 64 Shindou Edition
    {"NWR", -1, 3, 1}, // Wave Race 64
    {"N3HJ", -1, 1, 0}, // Ganbare! Nippon! Olympics 2000
    {"N3H", -1, 0, 1}, // International Track & Field 2000
    {"ND3J", -1, 4, 0}, // Akumajou Dracula Mokushiroku (J)
    {"ND3", -1, 0, 1}, // Castlevania
    {"ND4J", -1, 4, 0}, // Akumajou Dracula Mokushiroku Gaiden: Legend of Cornell (J)
    {"ND4", -1, 0, 1}, // Castlevania - Legacy of Darkness
    {"NDKJ", -1, 3, 0}, // Dark Rift [Space Dynamites (J)]
    {"NPDJ", -1, 4, 1}, // Perfect Dark (J)
    {"NPD", -1, 4, 1}, // Perfect Dark
    {"NSVE", -1, 3, 0}, // Space Station Silicon Valley
    {"NSV", -1, 3, 0}, // Space Station Silicon Valley
    {"NWTJ", -1, 3, 0}, // Wetrix
    {"NWT", -1, 0, 1}, // Wetrix
    {"ZSE", -1, 0, 0}, // Super Real Mahjong VS (Aleck64 conversion)
    {"ZSA", -1, 0, 0}, // Vivid Dolls (Aleck64 conversion)
    {"CLB", -1, 3, 0}, // Mario Party (NTSC)
    {"NAB", -1, 3, 1}, // Air Boarder 64
    {"NAD", -1, 3, 0}, // Worms Armageddon (U)
    {"NAG", -1, 3, 1}, // AeroGauge
    {"NB6", -1, 3, 1}, // Super B-Daman: Battle Phoenix 64
    {"NBC", -1, 3, 1}, // Blast Corps
    {"NBD", -1, 3, 0}, // Bomberman Hero [Mirian Ojo o Sukue! (J)]
    {"NBH", -1, 3, 0}, // Body Harvest
    {"NBK", -1, 3, 0}, // Banjo-Kazooie [Banjo to Kazooie no Daiboken (J)]
    {"NBM", -1, 3, 1}, // Bomberman 64 [Baku Bomberman (J)]
    {"NBN", -1, 3, 1}, // Bakuretsu Muteki Bangaioh
    {"NBV", -1, 3, 1}, // Bomberman 64: The Second Attack! [Baku Bomberman 2 (J)]
    {"NCG", -1, 3, 1}, // Choro Q 64 II - Hacha Mecha Grand Prix Race (J)
    {"NCH", -1, 3, 0}, // Chopper Attack
    {"NCR", -1, 3, 1}, // Penny Racers [Choro Q 64 (J)]
    {"NCT", -1, 3, 0}, // Chameleon Twist
    {"NCU", -1, 3, 1}, // Cruis'n USA
    {"NCX", -1, 3, 0}, // Custom Robo
    {"NDQ", -1, 3, 1}, // Disney's Donald Duck - Goin' Quackers [Quack Attack (E)]
    {"NDR", -1, 3, 0}, // Doraemon: Nobita to 3tsu no Seireiseki
    {"NDU", -1, 3, 0}, // Duck Dodgers starring Daffy Duck
    {"NDY", -1, 3, 1}, // Diddy Kong Racing
    {"NEA", -1, 3, 0}, // PGA European Tour
    {"NER", -1, 3, 0}, // Aero Fighters Assault [Sonic Wings Assault (J)]
    {"NF2", -1, 3, 0}, // F-1 World Grand Prix II
    {"NFG", -1, 3, 1}, // Fighter Destiny 2
    {"NFH", -1, 3, 0}, // In-Fisherman Bass Hunter 64
    {"NFW", -1, 3, 0}, // F-1 World Grand Prix
    {"NFX", -1, 3, 0}, // Star Fox 64 [Lylat Wars (E)]
    {"NFY", -1, 3, 1}, // Kakutou Denshou: F-Cup Maniax
    {"NGE", -1, 3, 0}, // GoldenEye 007
    {"NGL", -1, 3, 1}, // Getter Love!!
    {"NGU", -1, 3, 0}, // Tsumi to Batsu: Hoshi no Keishousha (Sin and Punishment)
    {"NGV", -1, 3, 0}, // Glover
    {"NHA", -1, 3, 1}, // Bomberman 64: Arcade Edition (J)
    {"NHF", -1, 3, 0}, // 64 Hanafuda: Tenshi no Yakusoku
    {"NHP", -1, 3, 0}, // Heiwa Pachinko World 64
    {"NIC", -1, 3, 0}, // Indy Racing 2000
    {"NIJ", -1, 3, 0}, // Indiana Jones and the Infernal Machine
    {"NIR", -1, 3, 0}, // Utchan Nanchan no Hono no Challenger: Denryuu Ira Ira Bou
    {"NJK", -1, 3, 0}, // Viewpoint 2064 (Final Prototype) (J)
    {"NJM", -1, 3, 0}, // Earthworm Jim 3D
    {"NK2", -1, 3, 0}, // Snowboard Kids 2 [Chou Snobow Kids (J)]
    {"NKA", -1, 3, 1}, // Fighters Destiny [Fighting Cup (J)]
    {"NKI", -1, 3, 1}, // Killer Instinct Gold
    {"NKT", -1, 3, 1}, // Mario Kart 64
    {"NLB", -1, 3, 0}, // Mario Party (PAL)
    {"NLL", -1, 3, 0}, // Last Legion UX
    {"NLR", -1, 3, 0}, // Lode Runner 3-D
    {"NMG", -1, 3, 1}, // Monaco Grand Prix [Racing Simulation 2 (G)]
    {"NMI", -1, 3, 0}, // Mission: Impossible
    {"NML", -1, 3, 0}, // Mickey's Speedway USA [Mickey no Racing Challenge USA (J)]
    {"NMO", -1, 3, 0}, // Monopoly
    {"NMR", -1, 3, 1}, // Multi-Racing Championship
    {"NMS", -1, 3, 1}, // Morita Shougi 64
    {"NMU", -1, 3, 1}, // Big Mountain 2000
    {"NMW", -1, 3, 0}, // Mario Party 2
    {"NMZ", -1, 3, 0}, // Zool - Majou Tsukai Densetsu (J)
    {"NN6", -1, 3, 0}, // Dr. Mario 64
    {"NNA", -1, 3, 0}, // Star Wars Episode I: Battle for Naboo
    {"NOS", -1, 3, 1}, // 64 Oozumou
    {"NO2", -1, 3, 1}, // 64 Oozumou 2
    {"NP2", -1, 3, 1}, // Chou Kuukan Night Pro Yakyuu King 2 (J)
    {"NPG", -1, 3, 0}, // Hey You, Pikachu! [Pikachu Genki Dechu (J)]
    {"NPT", -1, 3, 0}, // Puyo Puyon Party
    {"NPW", -1, 3, 0}, // Pilotwings 64
    {"NPY", -1, 3, 0}, // Puyo Puyo Sun 64
    {"NRA", -1, 3, 1}, // Rally '99 (J)
    {"NRC", -1, 3, 0}, // Top Gear Overdrive
    {"NRS", -1, 3, 0}, // Star Wars: Rogue Squadron [Shutsugeki! Rogue Chuutai (J)]
    {"NS3", -1, 3, 1}, // AI Shougi 3
    {"NS6", -1, 3, 0}, // Star Soldier: Vanishing Earth
    {"NSA", -1, 3, 0}, // Sonic Wings Assault (J)
    {"NSC", -1, 3, 0}, // Starshot: Space Circus Fever
    {"NSN", -1, 3, 1}, // Snow Speeder (J)
    {"NSS", -1, 3, 0}, // Super Robot Spirits
    {"NSU", -1, 3, 0}, // Rocket: Robot on Wheels
    {"NSW", -1, 3, 0}, // Star Wars: Shadows of the Empire [Teikoku no Kage (J)]
    {"NT6", -1, 3, 0}, // Tetris 64
    {"NTB", -1, 3, 0}, // Transformers: Beast Wars Metals 64 (J)
    {"NTC", -1, 3, 0}, // 64 Trump Collection
    {"NTJ", -1, 3, 0}, // Tom & Jerry in Fists of Fury
    {"NTM", -1, 3, 0}, // Mischief Makers [Yuke Yuke!! Trouble Makers (J)]
    {"NTN", -1, 3, 0}, // All Star Tennis '99
    {"NTP", -1, 3, 0}, // Tetrisphere
    {"NTR", -1, 3, 1}, // Top Gear Rally (J + E)
    {"NTW", -1, 3, 1}, // 64 de Hakken!! Tamagotchi
    {"NTX", -1, 3, 0}, // Taz Express
    {"NVL", -1, 3, 0}, // V-Rally Edition '99
    {"NVY", -1, 3, 0}, // V-Rally Edition '99 (J)
    {"NWC", -1, 3, 0}, // Wild Choppers
    {"NWQ", -1, 3, 1}, // Rally Challenge 2000
    {"NWU", -1, 3, 0}, // Worms Armageddon (E)
    {"NXO", -1, 3, 0}, // Cruis'n Exotica
    {"NYK", -1, 3, 0}, // Yakouchuu II: Satsujin Kouro
    {"N3D", -1, 4, 0}, // Doraemon 3: Nobita no Machi SOS!
    {"NB7", -1, 4, 0}, // Banjo-Tooie [Banjo to Kazooie no Daiboken 2 (J)]
    {"NCW", -1, 4, 0}, // Cruis'n World
    {"NCZ", -1, 4, 0}, // Custom Robo V2
    {"ND2", -1, 4, 0}, // Doraemon 2: Nobita to Hikari no Shinden
    {"ND6", -1, 4, 0}, // Densha de Go! 64
    {"NDO", -1, 4, 0}, // Donkey Kong 64
    {"NEP", -1, 4, 0}, // Star Wars Episode I: Racer
    {"NEV", -1, 4, 0}, // Neon Genesis Evangelion
    {"NFU", -1, 4, 0}, // Conker's Bad Fur Day
    {"NGC", -1, 4, 1}, // GT 64: Championship Edition
    {"NGT", -1, 4, 1}, // City Tour GrandPrix - Zen Nihon GT Senshuken
    {"NIM", -1, 4, 0}, // Ide Yosuke no Mahjong Juku
    {"NM8", -1, 4, 0}, // Mario Tennis
    {"NMV", -1, 4, 0}, // Mario Party 3
    {"NMX", -1, 4, 1}, // Excitebike 64
    {"NNB", -1, 4, 1}, // Kobe Bryant in NBA Courtside
    {"NPP", -1, 4, 1}, // Parlor! Pro 64: Pachinko Jikki Simulation Game
    {"NR7", -1, 4, 0}, // Robot Poncots 64: 7tsu no Umi no Caramel
    {"NRZ", -1, 4, 0}, // Ridge Racer 64
    {"NUB", -1, 4, 1}, // PD Ultraman Battle Collection 64
    {"NYS", -1, 4, 0}, // Yoshi's Story
    {"CFZ", -1, 1, 0}, // F-Zero X (J)
    {"CPS", -1, 1, 0}, // Pocket Monsters Stadium (J)
    {"CZL", -1, 1, 0}, // Legend of Zelda: Ocarina of Time [Zelda no Densetsu - Toki no Ocarina (J)]
    {"NA2", -1, 1, 1}, // Virtual Pro Wrestling 2
    {"NAL", -1, 1, 0}, // Super Smash Bros. [Nintendo All-Star! Dairantou Smash Brothers (J)]
    {"NB5", -1, 1, 0}, // Biohazard 2 (J)
    {"NDD", -1, 1, 0}, // 64DD Conversion Rom
    {"NFZ", -1, 1, 0}, // F-Zero X (U + E)
    {"NG6", -1, 1, 0}, // Ganmare Goemon: Dero Dero Douchuu Obake Tenkomori
    {"NGP", -1, 1, 1}, // Goemon: Mononoke Sugoroku
    {"NHY", -1, 1, 1}, // Hybrid Heaven (J)
    {"NIB", -1, 1, 0}, // Itoi Shigesato no Bass Tsuri No. 1 Kettei Ban!
    {"NJ5", -1, 1, 1}, // Jikkyou Powerful Pro Yakyuu 5
    {"NJG", -1, 1, 0}, // Jinsei Game 64
    {"NKG", -1, 1, 1}, // Major League Baseball featuring Ken Griffey Jr.
    {"NMF", -1, 1, 0}, // Mario Golf 64
    {"NOB", -1, 1, 0}, // Ogre Battle 64: Person of Lordly Caliber
    {"NP4", -1, 1, 1}, // Jikkyou Powerful Pro Yakyuu 4
    {"NP6", -1, 1, 1}, // Jikkyou Powerful Pro Yakyuu 6
    {"NPA", -1, 1, 1}, // Jikkyou Powerful Pro Yakyuu 2000
    {"NPE", -1, 1, 1}, // Jikkyou Powerful Pro Yakyuu Basic Ban 2001
    {"NPM", -1, 1, 1}, // Premier Manager 64
    {"NPS", -1, 1, 1}, // Jikkyou J.League 1999: Perfect Striker 2
    {"NRE", -1, 1, 0}, // Resident Evil 2
    {"NRI", -1, 1, 1}, // New Tetris, The
    {"NS4", -1, 1, 1}, // Super Robot Taisen 64
    {"NSI", -1, 1, 1}, // Fushigi no Dungeon: Fuurai no Shiren 2
    {"NT3", -1, 1, 1}, // Shin Nihon Pro Wrestling - Toukon Road 2 - The Next Generation (J)
    {"NTE", -1, 1, 0}, // 1080 Snowboarding
    {"NUM", -1, 1, 0}, // Nushi Zuri 64: Shiokaze ni Notte
    {"NUT", -1, 1, 1}, // Nushi Zuri 64
    {"NVB", -1, 1, 0}, // Bass Rush - ECOGEAR PowerWorm Championship (J)
    {"NVP", -1, 1, 1}, // Virtual Pro Wrestling 64
    {"NW2", -1, 1, 0}, // WCW-nWo Revenge
    {"NWL", -1, 1, 0}, // Waialae Country Club: True Golf Classics
    {"NWX", -1, 1, 1}, // WWF WrestleMania 2000
    {"NYW", -1, 1, 0}, // Harvest Moon 64
    {"NZL", -1, 1, 0}, // Legend of Zelda: Ocarina of Time (E)
    {"CDZ", -1, 6, 0}, // Dezaemon 3D
    {"CP2", -1, 5, 0}, // Pocket Monsters Stadium 2 (J)
    {"NAF", -1, 5, 1}, // Doubutsu no Mori
    {"NCC", -1, 5, 0}, // Command & Conquer
    {"NCV", -1, 5, 0}, // Cubivore (Translation)
    {"NCK", -1, 5, 0}, // NBA Courtside 2 featuring Kobe Bryant
    {"NDA", -1, 5, 1}, // Derby Stallion 64
    {"NDP", -1, 5, 0}, // Dinosaur Planet (Unlicensed)
    {"NJF", -1, 5, 0}, // Jet Force Gemini [Star Twins (J)]
    {"NKJ", -1, 5, 0}, // Ken Griffey Jr.'s Slugfest
    {"NM6", -1, 5, 0}, // Mega Man 64
    {"NMQ", -1, 5, 0}, // Paper Mario
    {"NPF", -1, 5, 0}, // Pokemon Snap [Pocket Monsters Snap (J)]
    {"NPN", -1, 5, 0}, // Pokemon Puzzle League
    {"NPO", -1, 5, 0}, // Pokemon Stadium
    {"NRH", -1, 5, 0}, // Rockman Dash - Hagane no Boukenshin (J)
    {"NSQ", -1, 5, 0}, // StarCraft 64
    {"NT9", -1, 5, 0}, // Tigger's Honey Hunt
    {"NW4", -1, 5, 1}, // WWF No Mercy
    {"NZS", -1, 5, 0}, // Legend of Zelda: Majora's Mask [Zelda no Densetsu - Mujura no Kamen (J)]
    {"NP3", -1, 5, 0}, // Pokemon Stadium 2 [Pocket Monsters Stadium - Kin Gin (J)]
    {"N22", -1, 0, 1}, // Ready 2 Rumble Boxing - Round 2
    {"N2M", -1, 0, 1}, // Madden Football 2002
    {"N2P", -1, 0, 1}, // Rampage 2 - Universal Tour
    {"N2V", -1, 0, 1}, // Chameleon Twist 2
    {"N32", -1, 0, 1}, // Army Men - Sarge's Heroes 2
    {"N3P", -1, 0, 1}, // Triple Play 2000
    {"N3T", -1, 0, 1}, // Tony Hawk's Pro Skater 3
    {"N4W", -1, 0, 1}, // 40 Winks
    {"N64", -1, 0, 1}, // Kira to Kaiketsu! 64 Tanteidan
    {"N7I", -1, 0, 1}, // FIFA Soccer 64 [FIFA 64 (E)]
    {"N8I", -1, 0, 1}, // FIFA - Road to World Cup 98 [World Cup e no Michi (J)]
    {"N8M", -1, 0, 1}, // Madden Football 64
    {"N8W", -1, 0, 1}, // World Cup '98
    {"N9B", -1, 0, 1}, // NBA Live '99
    {"N9C", -1, 0, 1}, // Nascar '99
    {"N9F", -1, 0, 1}, // FIFA 99
    {"N9H", -1, 0, 1}, // NHL '99
    {"N9M", -1, 0, 1}, // Madden Football '99
    {"NAC", -1, 0, 1}, // Army Men - Air Combat
    {"NAH", -1, 0, 1}, // Asteroids Hyper 64
    {"NAI", -1, 0, 1}, // Midway's Greatest Arcade Hits Volume 1
    {"NAM", -1, 0, 1}, // Army Men - Sarge's Heroes
    {"NAR", -1, 0, 1}, // Armorines - Project S.W.A.R.M.
    {"NAS", -1, 0, 1}, // All-Star Baseball 2001
    {"NAY", -1, 0, 1}, // Aidyn Chronicles - The First Mage
    {"NB2", -1, 0, 1}, // NBA In the Zone '99 [NBA Pro '99 (E)]
    {"NB3", -1, 0, 1}, // Bust-A-Move '99 [Bust-A-Move 3 DX (E)]
    {"NB4", -1, 0, 1}, // Bass Masters 2000
    {"NB8", -1, 0, 1}, // Beetle Adventure Racing (J)
    {"NB9", -1, 0, 1}, // NBA Jam '99
    {"NBA", -1, 0, 1}, // NBA In the Zone '98 [NBA Pro '98 (E)]
    {"NBE", -1, 0, 1}, // All-Star Baseball 2000
    {"NBF", -1, 0, 1}, // Bio F.R.E.A.K.S.
    {"NBI", -1, 0, 1}, // NFL Blitz 2000
    {"NBJ", -1, 0, 1}, // Bakushou Jinsei 64 - Mezase! Resort Ou
    {"NBL", -1, 0, 1}, // Buck Bumble
    {"NBO", -1, 0, 1}, // Bottom of the 9th
    {"NBP", -1, 0, 1}, // Blues Brothers 2000
    {"NBQ", -1, 0, 1}, // Battletanx - Global Assault
    {"NBR", -1, 0, 1}, // Milo's Astro Lanes
    {"NBS", -1, 0, 1}, // All-Star Baseball '99
    {"NBU", -1, 0, 1}, // Bust-A-Move 2 - Arcade Edition
    {"NBW", -1, 0, 1}, // Super Bowling
    {"NBX", -1, 0, 1}, // Battletanx
    {"NBY", -1, 0, 1}, // Bug's Life, A
    {"NBZ", -1, 0, 1}, // NFL Blitz
    {"NCB", -1, 0, 0}, // Charlie Blast's Territory
    {"NCD", -1, 0, 1}, // Carmageddon 64
    {"NCE", -1, 0, 1}, // Nuclear Strike 64
    {"NCL", -1, 0, 1}, // California Speed
    {"NCO", -1, 0, 1}, // Jeremy McGrath Supercross 2000
    {"NCS", -1, 0, 1}, // S.C.A.R.S.
    {"NDC", -1, 0, 1}, // SD Hiryuu no Ken Densetsu (J)
    {"NDE", -1, 0, 1}, // Destruction Derby 64
    {"NDF", -1, 0, 0}, // Dance Dance Revolution - Disney Dancing Museum
    {"NDH", -1, 0, 1}, // Duel Heroes
    {"NDM", -1, 0, 1}, // Doom 64
    {"NDN", -1, 0, 1}, // Duke Nukem 64
    {"NDQ", -1, 0, 1}, // Disney's Donald Duck - Goin' Quackers [Quack Attack (E)]
    {"NDS", -1, 0, 1}, // J.League Dynamite Soccer 64
    {"NDT", -1, 0, 1}, // South Park
    {"NDW", -1, 0, 1}, // Daikatana, John Romero's
    {"NDZ", -1, 0, 1}, // Duke Nukem - Zero Hour
    {"NEG", -1, 0, 1}, // Extreme-G
    {"NET", -1, 0, 1}, // Quest 64 [Eltale Monsters (J) Holy Magic Century (E)]
    {"NF9", -1, 0, 1}, // Fox Sports College Hoops '99
    {"NFB", -1, 0, 1}, // NFL Blitz 2001
    {"NFD", -1, 0, 1}, // Flying Dragon
    {"NFF", -1, 0, 1}, // Fighting Force 64
    {"NFL", -1, 0, 1}, // Madden Football 2001
    {"NFO", -1, 0, 1}, // Forsaken 64
    {"NFQ", -1, 0, 1}, // Razor Freestyle Scooter
    {"NFR", -1, 0, 1}, // F-1 Racing Championship
    {"NFS", -1, 0, 1}, // Famista 64
    {"NG2", -1, 0, 1}, // Extreme-G XG2
    {"NG5", -1, 0, 1}, // Ganbare Goemon - Neo Momoyama Bakufu no Odori [Mystical Ninja Starring Goemon]
    {"NGA", -1, 0, 1}, // Deadly Arts [G.A.S.P!! Fighter's NEXTream (E-J)]
    {"NGB", -1, 0, 1}, // Top Gear Hyper Bike
    {"NGD", -1, 0, 1}, // Gauntlet Legends (J)
    {"NGM", -1, 0, 1}, // Goemon's Great Adventure [Mystical Ninja 2 Starring Goemon]
    {"NGN", -1, 0, 1}, // Golden Nugget 64
    {"NGR", -1, 0, 1}, // Top Gear Rally (U)
    {"NGS", -1, 0, 1}, // Jikkyou G1 Stable
    {"NGX", -1, 0, 1}, // Gauntlet Legends
    {"NH5", -1, 0, 1}, // Nagano Winter Olympics '98 [Hyper Olympics in Nagano (J)]
    {"NH9", -1, 0, 1}, // NHL Breakaway '99
    {"NHC", -1, 0, 1}, // Hercules - The Legendary Journeys
    {"NHG", -1, 0, 1}, // F-1 Pole Position 64
    {"NHG", -1, 0, 1}, // Human Grand Prix - New Generation
    {"NHK", -1, 0, 1}, // Hiryuu no Ken Twin
    {"NHL", -1, 0, 1}, // NHL Breakaway '98
    {"NHM", -1, 0, 1}, // Mia Hamm Soccer 64
    {"NHN", -1, 0, 1}, // Olympic Hockey Nagano '98
    {"NHO", -1, 0, 1}, // NHL Blades of Steel '99 [NHL Pro '99 (E)]
    {"NHS", -1, 0, 1}, // Hamster Monogatari 64
    {"NHT", -1, 0, 1}, // Hydro Thunder
    {"NHV", -1, 0, 1}, // Hybrid Heaven (U + E)
    {"NHW", -1, 0, 1}, // Hot Wheels Turbo Racing
    {"NHX", -1, 0, 1}, // Hexen
    {"NIS", -1, 0, 1}, // International Superstar Soccer 2000
    {"NIV", -1, 0, 1}, // Space Invaders
    {"NJ2", -1, 0, 1}, // Wonder Project J2 - Koruro no Mori no Jozet (J)
    {"NJ3", -1, 0, 1}, // Jikkyou World Soccer 3
    {"NJA", -1, 0, 1}, // NBA Jam 2000
    {"NJE", -1, 0, 1}, // J.League Eleven Beat 1997
    {"NJL", -1, 0, 1}, // J.League Live 64
    {"NJP", -1, 0, 1}, // International Superstar Soccer 64 [Jikkyo J-League Perfect Striker (J)]
    {"NJQ", -1, 0, 0}, // Batman Beyond - Return of the Joker [Batman of the Future - Return of the Joker (E)]
    {"NKE", -1, 0, 0}, // Knife Edge - Nose Gunner
    {"NKK", -1, 0, 1}, // Knockout Kings 2000
    {"NKM", -1, 0, 1}, // Pro Mahjong Kiwame 64 (J)
    {"NKR", -1, 0, 1}, // Rakuga Kids (E)
    {"NL2", -1, 0, 1}, // Top Gear Rally 2
    {"NLC", -1, 0, 1}, // Automobili Lamborghini [Super Speed Race 64 (J)]
    {"NLG", -1, 0, 1}, // LEGO Racers
    {"NM3", -1, 0, 0}, // Monster Truck Madness 64
    {"NM4", -1, 0, 1}, // Mortal Kombat 4
    {"NM9", -1, 0, 1}, // Harukanaru Augusta Masters 98
    {"NMA", -1, 0, 1}, // Jangou Simulation Mahjong Do 64
    {"NMB", -1, 0, 1}, // Mike Piazza's Strike Zone
    {"NMD", -1, 0, 1}, // Madden Football 2000
    {"NMJ", -1, 0, 1}, // Mahjong 64
    {"NMM", -1, 0, 1}, // Mahjong Master
    {"NMT", -1, 0, 0}, // Magical Tetris Challenge
    {"NMY", -1, 0, 1}, // Mortal Kombat Mythologies - Sub-Zero
    {"NN2", -1, 0, 1}, // Nascar 2000
    {"NNC", -1, 0, 1}, // Nightmare Creatures
    {"NNL", -1, 0, 1}, // NBA Live 2000
    {"NNM", -1, 0, 1}, // Namco Museum 64
    {"NNR", -1, 0, 1}, // Pro Mahjong Tsuwamono 64 - Jansou Battle ni Chousen (J)
    {"NNS", -1, 0, 1}, // Beetle Adventure Racing
    {"NO7", -1, 0, 1}, // The World Is Not Enough
    {"NOF", -1, 0, 1}, // Offroad Challenge
    {"NOH", -1, 0, 0}, // Transformers Beast Wars - Transmetals
    {"NOM", -1, 0, 1}, // Onegai Monsters
    {"NOW", -1, 0, 1}, // Brunswick Circuit Pro Bowling
    {"NP9", -1, 0, 1}, // Ms. Pac-Man - Maze Madness
    {"NPB", -1, 0, 1}, // Puzzle Bobble 64 (J)
    {"NPC", -1, 0, 1}, // Pachinko 365 Nichi (J)
    {"NPK", -1, 0, 1}, // Chou Kuukan Night Pro Yakyuu King (J)
    {"NPL", -1, 0, 1}, // Power League 64 (J)
    {"NPR", -1, 0, 1}, // South Park Rally
    {"NPU", -1, 0, 1}, // Power Rangers - Lightspeed Rescue
    {"NPX", -1, 0, 1}, // Polaris SnoCross
    {"NPZ", -1, 0, 1}, // Susume! Taisen Puzzle Dama Toukon! Marumata Chou (J)
    {"NQ2", -1, 0, 1}, // Quake 2
    {"NQ8", -1, 0, 1}, // NFL Quarterback Club '98
    {"NQ9", -1, 0, 1}, // NFL Quarterback Club '99
    {"NQB", -1, 0, 1}, // NFL Quarterback Club 2000
    {"NQC", -1, 0, 1}, // NFL Quarterback Club 2001
    {"NQK", -1, 0, 1}, // Quake 64
    {"NR2", -1, 0, 1}, // Rush 2 - Extreme Racing USA
    {"NR3", -1, 0, 1}, // Stunt Racer 64
    {"NR6", -1, 0, 1}, // Tom Clancy's Rainbow Six
    {"NRD", -1, 0, 1}, // Ready 2 Rumble Boxing
    {"NRG", -1, 0, 0}, // Rugrats - Scavenger Hunt [Treasure Hunt (E)]
    {"NRK", -1, 0, 1}, // Rugrats in Paris - The Movie
    {"NRO", -1, 0, 1}, // Road Rash 64
    {"NRP", -1, 0, 1}, // Rampage - World Tour
    {"NRR", -1, 0, 1}, // Roadster's Trophy
    {"NRT", -1, 0, 1}, // Rat Attack
    {"NRU", -1, 0, 1}, // San Francisco Rush 2049
    {"NRV", -1, 0, 1}, // Re-Volt
    {"NRW", -1, 0, 1}, // Turok: Rage Wars
    {"NRX", -1, 0, 1}, // Robotron 64
    {"NS2", -1, 0, 1}, // Simcity 2000
    {"NSB", -1, 0, 1}, // Twisted Edge - Extreme Snowboarding [King Hill 64 - Extreme Snowboarding (J)]
    {"NSD", -1, 0, 1}, // Shadow Man
    {"NSF", -1, 0, 1}, // San Francisco Rush - Extreme Racing
    {"NSG", -1, 0, 1}, // Shadowgate 64 - Trials Of The Four Towers
    {"NSH", -1, 0, 1}, // Saikyou Habu Shougi (J)
    {"NSJ", -1, 0, 1}, // J.League Tactics Soccer
    {"NSK", -1, 0, 1}, // Snowboard Kids [Snobow Kids (J)]
    {"NSL", -1, 0, 1}, // Spider-Man
    {"NSO", -1, 0, 1}, // NBA Showtime - NBA on NBC
    {"NSP", -1, 0, 1}, // Superman
    {"NST", -1, 0, 1}, // Eikou no Saint Andrews
    {"NSX", -1, 0, 1}, // Supercross 2000
    {"NSY", -1, 0, 1}, // Scooby-Doo! - Classic Creep Capers
    {"NSZ", -1, 0, 1}, // NFL Blitz - Special Edition
    {"NT2", -1, 0, 1}, // Turok 2 - Seeds of Evil [Violence Killer - Turok New Generation (J)]
    {"NT4", -1, 0, 1}, // CyberTiger
    {"NTA", -1, 0, 1}, // Disney's Tarzan
    {"NTD", -1, 0, 1}, // O.D.T. [Prototype]
    {"NTF", -1, 0, 1}, // Tony Hawk's Pro Skater
    {"NTH", -1, 0, 1}, // Toy Story 2 - Buzz Lightyear to the Rescue!
    {"NTI", -1, 0, 1}, // WWF: Attitude
    {"NTK", -1, 0, 1}, // Turok 3 - Shadow of Oblivion
    {"NTO", -1, 0, 1}, // Shin Nihon Pro Wrestling - Toukon Road - Brave Spirits (J)
    {"NTQ", -1, 0, 1}, // Tony Hawk's Pro Skater 2
    {"NTS", -1, 0, 1}, // Centre Court Tennis [Let's Smash (J)]
    {"NTT", -1, 0, 1}, // Tonic Trouble
    {"NTU", -1, 0, 1}, // Turok: Dinosaur Hunter [Turok: Jikuu Senshi (J)]
    {"NV3", -1, 0, 1}, // Micro Machines 64 Turbo
    {"NV8", -1, 0, 1}, // Vigilante 8
    {"NVC", -1, 0, 1}, // Virtual Chess 64
    {"NVG", -1, 0, 1}, // Vigilante 8 - Second Offense
    {"NVR", -1, 0, 1}, // Virtual Pool 64
    {"NW3", -1, 0, 1}, // WCW: Nitro
    {"NW8", -1, 0, 1}, // Wayne Gretzky's 3D Hockey '98
    {"NWB", -1, 0, 1}, // Iggy's Reckin' Balls [Iggy-kun no Bura Bura Poyon (J)]
    {"NWD", -1, 0, 1}, // Winback - Covert Operations
    {"NWF", -1, 0, 0}, // Wheel of Fortune
    {"NWG", -1, 0, 1}, // Wayne Gretzky's 3D Hockey
    {"NWI", -1, 0, 1}, // ECW Hardcore Revolution
    {"NWK", -1, 0, 1}, // Michael Owens WLS 2000 [World League Soccer 2000 (E) / Telefoot Soccer 2000 (F)]
    {"NWM", -1, 0, 1}, // WCW: Mayhem
    {"NWN", -1, 0, 1}, // WCW vs. nWo - World Tour
    {"NWO", -1, 0, 1}, // World Driver Championship
    {"NWP", -1, 0, 1}, // Wipeout 64
    {"NWS", -1, 0, 1}, // International Superstar Soccer '98 [Jikkyo World Soccer - World Cup France '98 (J)]
    {"NWV", -1, 0, 1}, // WCW: Backstage Assault
    {"NWW", -1, 0, 1}, // WWF: War Zone
    {"NWZ", -1, 0, 1}, // NBA In the Zone 2000
    {"NX2", -1, 0, 1}, // Gex 64 - Enter the Gecko
    {"NX3", -1, 0, 1}, // Gex 3 - Deep Cover Gecko
    {"NXF", -1, 0, 1}, // Xena Warrior Princess - The Talisman of Fate
    {"NXG", -1, 0, 1}, // NBA Hangtime
    {"NY2", -1, 0, 1}, // Rayman 2 - The Great Escape
    {"NYP", -1, 0, 1}, // Paperboy
    {"NYW", -1, 0, 1}, // Bokujou Monogatari 2
    {"NZO", -1, 0, 1}, // Battlezone - Rise of the Black Dogs
};
static const int save_db_count = sizeof(save_db) / sizeof(save_db[0]);

int save_db_lookup(const unsigned char *id4, unsigned char ver, int *save, int *cpak)
{
    int i;
    for (i = 0; i < save_db_count; i++)
    {
        const save_db_entry_t *e = &save_db[i];
        if (e->id[3] == 0)
        {
            if (memcmp(e->id, id4, 3) != 0)
                continue;
        }
        else
        {
            if (memcmp(e->id, id4, 4) != 0)
                continue;
            if (e->ver >= 0 && e->ver != ver)
                continue;
        }
        *save = e->save;
        *cpak = e->cpak;
        return 1;
    }
    return 0;
}
