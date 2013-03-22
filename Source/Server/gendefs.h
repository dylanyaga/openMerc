/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))
#define RANDOM(a)       (random()%(a))
#define DATDIR          ".dat"

#define VERSION         0x020E07
#define MINVERSION      0x020E06

#define TICKS                   18
#define TICK                    (1000000/TICKS)

#define MAPX                      1024
#define MAPY                      1024
#define MAXCHARS                  8192
#define MAXITEM                  (96*1024)
#define MAXEFFECT                 4096
#define MAXMISSION                1024
#define MAXSKILL                    50

#define MAXTCHARS                 4548
#define MAXTITEM                  4548

#define LIGHTDIST                   10
#define LENDESC                    200

#define HOME_MERCENARY_X        512
#define HOME_MERCENARY_Y        512

#define CNTSAY			(TICKS)
#define MAXSAY			(TICKS*7)

#define GODPASSWORD "xxxxxxxxxxxx"

// wear positions
#define WN_HEAD                 0
#define WN_NECK                 1
#define WN_BODY                 2
#define WN_ARMS                 3
#define WN_BELT                 4
#define WN_LEGS                 5
#define WN_FEET                 6
#define WN_LHAND                7       // shield
#define WN_RHAND                8       // weapon
#define WN_CLOAK                9
#define WN_LRING                10
#define WN_RRING                11

// placement bits
#define PL_HEAD                 1
#define PL_NECK                 2
#define PL_BODY                 4
#define PL_ARMS                 8
#define PL_BELT                 32
#define PL_LEGS                 64
#define PL_FEET                 128
#define PL_WEAPON               256
#define PL_SHIELD               512             // not usable with two-handed weapons
#define PL_CLOAK                1024
#define PL_TWOHAND              2048
#define PL_RING                 4096

#define DX_RIGHT                1
#define DX_LEFT                 2
#define DX_UP                   3
#define DX_DOWN                 4
#define DX_LEFTUP               5
#define DX_LEFTDOWN             6
#define DX_RIGHTUP              7
#define DX_RIGHTDOWN            8

// driver stuff
#define DR_IDLE         0
#define DR_DROP         1
#define DR_PICKUP       2
#define DR_GIVE         3
#define DR_USE          4
#define DR_BOW          5
#define DR_WAVE         6
#define DR_TURN         7
#define DR_SINGLEBUILD  8
#define DR_AREABUILD1   9
#define DR_AREABUILD2   10

#define NT_NONE         0
#define NT_GOTHIT       1
#define NT_GOTMISS      2
#define NT_DIDHIT       3
#define NT_DIDMISS      4
#define NT_DIDKILL      5
#define NT_GOTEXP       6
#define NT_SEEHIT       7
#define NT_SEEMISS      8
#define NT_SEEKILL      9
#define NT_GIVE         11
#define NT_SEE          12
#define NT_DIED         13
#define NT_SHOUT        14
#define NT_HITME        15

#define SP_LIGHT        (1u<<0)
#define SP_PROTECT      (1u<<1)
#define SP_ENHANCE      (1u<<2)
#define SP_BLESS        (1u<<3)
#define SP_HEAL         (1u<<4)
#define SP_CURSE        (1u<<5)
#define SP_STUN         (1u<<6)
#define SP_DISPEL       (1u<<7)

extern char *at_name[];
