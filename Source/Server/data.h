/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

// For all .used:
#define USE_EMPTY               ((unsigned char)0)
#define USE_ACTIVE              ((unsigned char)1)
#define USE_NONACTIVE           ((unsigned char)2)

/*********/
/* Globs */
/*********/

/* CS, 991113: SIZEs in one header */
#define GLOBSIZE                (sizeof(struct global))

#define GF_LOOTING              (1<<0)
#define GF_MAYHEM               (1<<1)
#define GF_CLOSEENEMY		(1<<2)
#define GF_CAP			(1<<3)
#define GF_SPEEDY		(1<<4)
#define GF_DIRTY		(1<<5)

struct global
{
        int mdtime;
        int mdday;
        int mdyear;
        int dlight;

        int players_created;
        int npcs_created;
        int players_died;
        int npcs_died;

        int character_cnt;
        int item_cnt;
        int effect_cnt;

        int expire_cnt;
        int expire_run;

        int gc_cnt;
        int gc_run;

        int lost_cnt;
        int lost_run;

        int reset_char;
        int reset_item;

        int ticker;

        long long total_online_time;
        long long online_per_hour[24];

        int flags;

        long long uptime;
        long long uptime_per_hour[24];

        int awake;
        int body;

        int players_online;
        int queuesize;

        long long recv;
        long long send;

        int transfer_reset_time;
        int load_avg;

        long load;

        int max_online;
        int max_online_per_hour[24];

        char fullmoon;
        char newmoon;

	unsigned long long unique;

	int cap;
};


/*******/
/* Map */
/*******/

// 32 bits for flag (static)
#define MF_MOVEBLOCK    (1ULL<<0)
#define MF_SIGHTBLOCK   (1ULL<<1)
#define MF_INDOORS      (1ULL<<2)
#define MF_UWATER       (1ULL<<3)
#define MF_NOLAG        (1ULL<<4)
#define MF_NOMONST      (1ULL<<5)
#define MF_BANK         (1ULL<<6)
#define MF_TAVERN       (1ULL<<7)
#define MF_NOMAGIC      (1ULL<<8)
#define MF_DEATHTRAP    (1ULL<<9)

#define MF_ARENA        (1ULL<<11)

#define MF_NOEXPIRE     (1ULL<<13)
/* CS, 991204: NOFIGHT */
#define MF_NOFIGHT      (1Ull<<14)

// plus 32 bits for dynamic flags
#define MF_GFX_INJURED  (1ULL<<32)
#define MF_GFX_INJURED1 (1ULL<<33)
#define MF_GFX_INJURED2 (1ULL<<34)

#define MF_GFX_TOMB     ((1ULL<<35)|(1ULL<<36)|(1ULL<<37)|(1ULL<<38)|(1ULL<<39))
#define MF_GFX_TOMB1    (1ULL<<35)
#define MF_GFX_DEATH    ((1ULL<<40)|(1ULL<<41)|(1ULL<<42)|(1ULL<<43)|(1ULL<<44))
#define MF_GFX_DEATH1   (1ULL<<40)

#define MF_GFX_EMAGIC   ((1ULL<<45)|(1ULL<<46)|(1ULL<<47))
#define MF_GFX_EMAGIC1  (1ULL<<45)
#define MF_GFX_GMAGIC   ((1ULL<<48)|(1ULL<<49)|(1ULL<<50))
#define MF_GFX_GMAGIC1  (1ULL<<48)
#define MF_GFX_CMAGIC   ((1ULL<<51)|(1ULL<<52)|(1ULL<<53))
#define MF_GFX_CMAGIC1  (1ULL<<51)

/* CS, 991113: SIZEs in one header */
#define MAPSIZE         (sizeof(struct map)*MAPX*MAPY)

struct map_old
{
        unsigned short sprite;          // background image
        unsigned short fsprite;         // foreground sprite

        // for fast access to objects & characters
        unsigned short ch,to_ch;
        unsigned short it;

        unsigned short dlight;          // percentage of dlight
        short light;                    // strength of light (objects only, daylight is computed independendly)

        unsigned long long flags;       // s.a.

} __attribute__ ((packed));

struct map
{
        unsigned short sprite;          // background image
        unsigned short fsprite;         // foreground sprite

        // for fast access to objects & characters
        unsigned int ch,to_ch;
        unsigned int it;

        unsigned short dlight;          // percentage of dlight
        short light;                    // strength of light (objects only, daylight is computed independendly)

        unsigned long long flags;       // s.a.

} __attribute__ ((packed));

/* CS, 991113: Support for new pathfinder */
extern unsigned int mapmark[MAPX][MAPY];
extern unsigned int mapmarker;

/**************/
/* Characters */
/**************/

#define KIN_MERCENARY   (1u<<0)
#define KIN_SEYAN_DU    (1u<<1)
#define KIN_PURPLE      (1u<<2)
#define KIN_MONSTER     (1u<<3)
#define KIN_TEMPLAR     (1u<<4)
#define KIN_ARCHTEMPLAR (1u<<5)
#define KIN_HARAKIM     (1u<<6)
#define KIN_MALE        (1u<<7)
#define KIN_FEMALE      (1u<<8)
#define KIN_ARCHHARAKIM (1u<<9)
#define KIN_WARRIOR     (1u<<10)        // arch-merc, warrior
#define KIN_SORCERER    (1u<<11)        // arch-merc, sorcerer

#define CF_IMMORTAL     (1ull<<0)       // will not suffer any damage
#define CF_GOD          (1ull<<1)       // may issue #god commands
#define CF_CREATOR      (1ull<<2)       // may use #build
#define CF_BUILDMODE    (1ull<<3)       // does use #build
#define CF_RESPAWN      (1ull<<4)       // will respawn after death - not for players
#define CF_PLAYER       (1ull<<5)       // is a player
#define CF_NEWUSER      (1ull<<6)       // new account created. player may change name
#define CF_NOTELL       (1ull<<8)       // tell will only work on him if used by a god
#define CF_NOSHOUT      (1ull<<9)       // shout will only work in him if used by a god
#define CF_MERCHANT     (1ull<<10)      // will sell his inventory if looked at
#define CF_STAFF        (1ull<<11)      // member of the staff
#define CF_NOHPREG      (1ull<<12)      // no hp regeneration
#define CF_NOENDREG     (1ull<<13)
#define CF_NOMANAREG    (1ull<<14)
#define CF_INVISIBLE    (1ull<<15)      // character is completely invisible
#define CF_INFRARED     (1ull<<16)      // sees in the dark
#define CF_BODY         (1ull<<17)      // dead body
#define CF_NOSLEEP      (1ull<<18)      // stay awake all the time
#define CF_UNDEAD       (1ull<<19)      // is undead, can be killed with holy water
#define CF_NOMAGIC      (1ull<<20)      // no magic zone
#define CF_STONED       (1ull<<21)      // turned to stone due to lag
#define CF_USURP        (1ull<<22)      // NPC is being played by player
#define CF_IMP          (1ull<<23)      // may impersonate monsters
#define CF_SHUTUP       (1ull<<24)      // player is unable to talk till next day
#define CF_NODESC       (1ull<<25)      // player cannot change his description
#define CF_PROF         (1ull<<26)      // profiler listing
#define CF_SIMPLE       (1ull<<27)      // uses simple animation system (move, turn, 1 attack)
#define CF_KICKED       (1ull<<28)      // player got kicked, may not login again for a certain time
#define CF_NOLIST       (1ull<<29)      // dont list character in top ten
#define CF_NOWHO        (1ull<<30)      // don't list character in #WHO
#define CF_SPELLIGNORE  (1ull<<31)      // ignore spells cast on me
#define CF_CCP          (1ull<<32)      // Computer Controlled Player, does NOT log out and may have some extra logic
#define CF_SAFE         (1ull<<33)      // safety measures for gods
#define CF_NOSTAFF      (1ull<<34)       // #stell will only work if flag off
#define CF_POH          (1ull<<35)      // clan purples of honor
#define CF_POH_LEADER   (1ull<<36)      // clan purples of honor
#define CF_THRALL       (1ull<<37)      // is enthralled NPC
#define CF_LABKEEPER    (1ull<<38)      // is labkeeper
#define CF_ISLOOTING	(1ull<<39)	// is currently looting a grave
#define CF_GOLDEN	(1ull<<40)	// is on "golden list" aka good player
#define CF_BLACK	(1ull<<41)	// is on "black list" aka bad player
#define CF_PASSWD	(1ull<<42)	// has passwd set
#define CF_UPDATE	(1ull<<43)	// client side update needed
#define CF_SAVEME	(1ull<<44)	// save this player to disk
#define CF_GREATERGOD	(1ull<<45)	// greater god
#define CF_GREATERINV	(1ull<<46)	// no one sees me, ever

#define AT_BRAVE        0
#define AT_WILL         1
#define AT_INT          2
#define AT_AGIL         3
#define AT_STREN        4

#define SK_HAND         0
#define SK_KARATE       1
#define SK_SWORD        3
#define SK_AXE          4
#define SK_DAGGER       2
#define SK_STAFF        5
#define SK_TWOHAND      6       // two handed weapon
#define SK_LOCK         7
#define SK_STEALTH      8
#define SK_PERCEPT      9
#define SK_SWIM         10
#define SK_MSHIELD      11
#define SK_BARTER       12
#define SK_REPAIR       13
#define SK_LIGHT        14
#define SK_RECALL       15
#define SK_WIMPY        16
#define SK_PROTECT      17
#define SK_ENHANCE      18
#define SK_STUN         19
#define SK_CURSE        20
#define SK_BLESS        21
#define SK_IDENT        22
#define SK_RESIST       23
#define SK_BLAST        24
#define SK_DISPEL       25
#define SK_HEAL         26
#define SK_GHOST        27
#define SK_REGEN        28
#define SK_REST         29
#define SK_MEDIT        30
#define SK_SENSE        31
#define SK_IMMUN        32
#define SK_SURROUND     33
#define SK_CONCEN	34
#define SK_WARCRY	35
#define SK_WARCRY2	(SK_WARCRY+100)

/* ch.data[] definitions */
/* (this list is growing very slowly;
   see definitions at the beginning of driver.c for NPCs and
   README for player characters. */
#define CHD_AFK          0
#define CHD_MINGROUP     1
#define CHD_MAXGROUP     9
#define CHD_FIGHTBACK   11
#define CHD_GROUP       42
#define CHD_MASTER      63
#define CHD_COMPANION   64
#define CHD_ALLOW       65
#define CHD_CORPSEOWNER 66
#define CHD_RIDDLER     67
#define CHD_ATTACKTIME  68
#define CHD_ATTACKVICT  69
#define CHD_TALKATIVE   71
#define CHD_ENEMY1ST    80
#define CHD_ENEMYZZZ    91

#define RANKS           24

/* level differences permitted for attack / group */
#define ATTACK_RANGE	 3
#define GROUP_RANGE	 	 3

/* CS, 991113: SIZEs in one header */
#define CHARSIZE (sizeof(struct character)*MAXCHARS)
#define TCHARSIZE (sizeof(struct character)*MAXTCHARS)

struct character
{
        unsigned char used;             // 1
        // general

        char name[40];                  // 41
        char reference[40];             // 81
        char description[LENDESC];      // 281

        int kindred;                    // 285

        int player;                     // 289
        unsigned int pass1,pass2;       // 297

        unsigned short sprite;          // 299, sprite base value, 1024 dist
        unsigned short sound;           // 301, sound base value, 64 dist

        unsigned long long flags;       // 309

        short int alignment;            // 311

        unsigned short temple_x;        // 313, position of temple for recall and dying
        unsigned short temple_y;        // 315

        unsigned short tavern_x;        // 317, position of last temple for re-login
        unsigned short tavern_y;        // 319

        unsigned short temp;            // 321, created from template n

        // character stats
        // [0]=bare value, 0=unknown
        // [1]=preset modifier, is race/npc dependend
        // [2]=race specific maximum
        // [3]=race specific difficulty to raise (0=not raisable, 1=easy ... 10=hard)
        // [4]=dynamic modifier, depends on equipment and spells (this one is currently not used)
        // [5]=total value

        unsigned char attrib[5][6];     // 351

        unsigned short hp[6];           // 363
        unsigned short end[6];          // 375
        unsigned short mana[6];         // 387

        unsigned char skill[50][6];     // 687

        unsigned char weapon_bonus;
        unsigned char armor_bonus;

        // temporary attributes
        int a_hp;
        int a_end;
        int a_mana;

        unsigned char light;    // strength of lightsource
        unsigned char mode;     // 0 = slow, 1 = medium, 2 = fast
        short int speed;

        int points;
        int points_tot;

        // summary of weapons + armor
        short int armor;
        short int weapon;

        // map stuff
        short int x,y;          // current position x,y NOTE: x=-1, y=-1 = void
        short int tox,toy;      // target coordinated, where the char will be next turn
        short int frx,fry;      // where the char was last turn
        short int status;       // what the character is doing, animation-wise
        short int status2;      // for plr_misc(): what is misc?
        unsigned char dir;      // direction character is facing

        // posessions
        int gold;

        // items carried
        unsigned int item[40];

        // items worn
        unsigned int worn[20];

        // spells active on character
        unsigned int spell[20];

        // item currently in hand (mouse cursor)
        unsigned int citem;

        time_t creation_date;
        time_t login_date;

        unsigned int addr;

        // misc
        unsigned int current_online_time;
        unsigned int total_online_time;
        unsigned int comp_volume;
        unsigned int raw_volume;
        unsigned int idle;

        // generic driver data
        unsigned short attack_cn;       // target for attacks, will attack if set (prio 4)
        unsigned short skill_nr;        // skill to use/spell to cast, will cast if set (prio 2)
        unsigned short skill_target1;   // target for skills/spells
        unsigned short skill_target2;   // target for skills/spells
        unsigned short goto_x;          // will goto x,y if set (prio 3)
        unsigned short goto_y;
        unsigned short use_nr;          // will use worn item nr if set (prio 1)

        unsigned short misc_action;     // drop, pickup, use, whatever (prio 5)
        unsigned short misc_target1;    // item for misc_action
        unsigned short misc_target2;    // location for misc_action

        unsigned short cerrno;          // error/success indicator for last action (svr_act level)

        unsigned short escape_timer;    // can try again to escape in X ticks
        unsigned short enemy[4];        // currently being fought against by these
        unsigned short current_enemy;   // currently fighting against X

        unsigned short retry;           // retry current action X times

        unsigned short stunned;         // is stunned for X ticks

        // misc stuff added later:
        char speed_mod;                 // race dependand speed modification
        char last_action;               // last action was success/failure (driver_generic level)
        char unused;
        char depot_sold;                // items from depot where sold to pay for the rent

        char gethit_dam;                // damage for attacker when hitting this char
        char gethit_bonus;              // race specific bonus for above

        unsigned char light_bonus;      // char emits light all the time

	char passwd[16];

	char lastattack;		// neater display: remembers the last attack animation
        char future1[25];               // space for future expansion

        short int sprite_override;

        short future2[49];

        unsigned int depot[62];

        int depot_cost;

        int luck;

        int unreach,unreachx,unreachy;

        int class;                      // monster class

        int future3[12];

        time_t logout_date;

        // driver data
        int data[100];
        char text[10][160];

} __attribute__ ((packed));

/*********/
/* Items */
/*********/

#define IF_MOVEBLOCK    (1ull<<0)
#define IF_SIGHTBLOCK   (1ull<<1)
#define IF_TAKE         (1ull<<2)
#define IF_MONEY        (1ull<<3)
#define IF_LOOK         (1ull<<4)
#define IF_LOOKSPECIAL  (1ull<<5)
#define IF_SPELL        (1ull<<6)
#define IF_NOREPAIR     (1ull<<7)
#define IF_ARMOR        (1ull<<8)       // is a piece of armor
#define IF_USE          (1ull<<9)
#define IF_USESPECIAL   (1ull<<10)
#define IF_SINGLEAGE    (1ull<<11)      // don't use age[1] even if it is active
#define IF_SHOPDESTROY  (1ull<<12)
#define IF_UPDATE       (1ull<<13)
#define IF_ALWAYSEXP1   (1ull<<14)      // expire even if not laying in the open and when non-active
#define IF_ALWAYSEXP2   (1ull<<15)      // expire ... when active
#define IF_WP_SWORD     (1ull<<16)      // is a weapon - sword
#define IF_WP_DAGGER    (1ull<<17)      // is a weapon - dagger
#define IF_WP_AXE       (1ull<<18)      // is a weapon - axe
#define IF_WP_STAFF     (1ull<<19)      // is a weapon - staff
#define IF_WP_TWOHAND   (1ull<<20)      // is a weapon - two-handed sword
#define IF_USEDESTROY   (1ull<<21)      // using it destroys the object
#define IF_USEACTIVATE  (1ull<<22)      // may be turned on (activated)
#define IF_USEDEACTIVATE (1ull<<23)     // may be turned off (deactivated)
#define IF_MAGIC        (1ull<<24)      // is magical
#define IF_MISC         (1ull<<25)      // is neither weapon nor armor nor magical
#define IF_REACTIVATE   (1ull<<26)      // reactive item whenever it expires
#define IF_PERMSPELL    (1ull<<27)      // permanent spell (may take mana to keep up)
#define IF_UNIQUE       (1ull<<28)      // unique item
#define IF_DONATE       (1ull<<29)      // auto-donate this item
#define IF_LABYDESTROY  (1ull<<30)      // destroy when leaving labyrinth
#define IF_NOMARKET     (1ull<<31)      // dont change the price for this item
#define IF_HIDDEN       (1ull<<32)      // hard to see, uses data[9] for difficulty
#define IF_STEPACTION   (1ull<<33)      // special routine to call when stepped on
#define IF_NODEPOT      (1ull<<34)      // not storable in depot
#define IF_LIGHTAGE     (1ull<<35)      // ages faster when exposed to light
#define IF_EXPIREPROC   (1ull<<36)      // special procedure for expire
#define IF_IDENTIFIED   (1ull<<37)      // item has been identified
#define IF_NOEXPIRE     (1ull<<38)      // dont expire item
#define IF_SOULSTONE	(1ull<<39)	// item was enhanced by a soulstone

#define IF_WEAPON       (IF_WP_SWORD|IF_WP_DAGGER|IF_WP_AXE|IF_WP_STAFF|IF_WP_TWOHAND)
#define IF_SELLABLE     (IF_WEAPON|IF_MISC|IF_MAGIC|IF_ARMOR)

#define ITEMSIZE (sizeof(struct item)*MAXITEM)
#define TITEMSIZE (sizeof(struct item)*MAXTITEM)

struct item
{
        unsigned char used;             // 1
        char name[40];                  // 41
        char reference[40];             // 81, a pair of boots
        char description[200];          // 281, A pair of studded leather boots.

        unsigned long long flags;       // 289, s.a.

        unsigned int value;             // 293, value to a merchant
        unsigned short placement;       // 295, see constants above

        unsigned short temp;            // 297, created from template temp

        unsigned char damage_state;     // 298, has reached damage level X of 5, 0=OK, 4=almost destroyed, 5=destroyed

        // states for non-active [0] and active[1]
        unsigned int max_age[2];        // 306, maximum age per state
        unsigned int current_age[2];    // 314, current age in current state

        unsigned int max_damage;        // 318, maximum damage per state
        unsigned int current_damage;    // 322, current damage in current state

        // modifiers - modifiers apply only when the item is being
        // worn (wearable objects) or when spell is cast. After duration expires,
        // the effects are removed.

        // modifiers - modifier [0] applies when the item is being
        // worn (wearable objects) or is added to the powers (spells) for permanent spells
        // modifier [1] applies when it is active
        // modifier [2] is not a modifier but the minimum value that attibute/skill must have to wear or use
        // the item

        char attrib[5][3];              // 337

        short hp[3];                    // 343
        short end[3];                   // 349
        short mana[3];                  // 355

        char skill[50][3];              // 505

        char armor[2];                  // 506
        char weapon[2];                 // 507

        short light[2];                 // 511

        unsigned int duration;          // 515
        unsigned int cost;              // 519
        unsigned int power;             // 523
        unsigned int active;            // 527

        // map stuff
        unsigned short int x,y;         // 531, current position        NOTE: x=0, y=0 = void
        unsigned short carried;         // 533, carried by character carried
        unsigned short sprite_override; // 535, used for potions/spells which change the character sprite

        short int sprite[2];            // 543
        unsigned char status[2];        // 545

        char gethit_dam[2];             // 547, damage for hitting this item

	char min_rank;			// minimum rank to wear the item
	char future[3];			
        int future3[9];             	// 587

        int t_bought;                   // 591
        int t_sold;                     // 595

        unsigned char driver;           // 596, special routines for LOOKSPECIAL and USESPECIAL
        unsigned int data[10];          // 634, driver data

} __attribute__ ((packed));

/***********/
/* Effects */
/***********/

#define EF_MOVEBLOCK    1
#define EF_SIGHTBLOCK   2

#define FX_INJURED      1

/* CS, 991113: SIZEs in one header */
#define EFFECTSIZE (sizeof(struct effect)*MAXEFFECT)

struct effect
{
        unsigned char used;
        unsigned char flags;

        unsigned char type;             // what type of effect (FX_)

        unsigned int duration;          // time effect will stay

        unsigned int data[10];          // some data
} __attribute__ ((packed));

struct s_skilltab
{
        int nr;
        char sortkey;
        char name[40];
        char desc[200];

        int attrib[3];
};

struct see_map
{
        int x,y;
        char vis[40*40];
};

extern struct s_skilltab skilltab[MAXSKILL];
extern struct global *globs;
extern struct map *map;
extern struct character *ch;
extern struct item *it;
extern struct character *ch_temp;
extern struct item *it_temp;
extern struct effect *fx;
extern struct see_map *see;
