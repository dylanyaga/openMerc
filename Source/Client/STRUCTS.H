#define GM_DIALOG		1
#define GM_FIGHT		2
#define GM_DUNGEON	3
#define GM_MISSION	4
#define GM_SHOP		5

extern int game_mode;
extern int map_nr;
extern int atime;

struct character
{
	// informative stuff
	char name[40];

	// id of driver-routine
	int driver;
	int drdata[10];

	// character attributes+abilities
	// [0]=bare value, [1]=modifier, [2]=total value, [3] value used by game
	int braveness[4];
	int willpower[4];
	int intuition[4];
	int agility[4];
	int strength[4];
	int hp[4];
	int endurance[4];
	int fighting[4];
	int bargaining[4];
	int lock_picking[4];
	int stealth[4];
	int perception[4];
	int sixth_sense[4];

	// temporary attributes
	int a_hp;
	int a_endurance;

	int speed;
	int time;		// current time for this character

	int points;
	int points_tot;

	// summary of weapons + armor
	int armor;
	int weapon;

	// temporary fighting stuff
	int ep_left;

	// fighting style
	int attack_level;
	int parry_level;

	// map stuff
	int mission,x,y;	// current mission, x,y
	int tox,toy;		// target coordinated, where the char will be next turn
	int status;			// what the character is doing, animation-wise
   int tick;			// tick within current animation

	// posessions
	int gold;

	// items carried
	int item[40];

	// items worn
	int worn[10];

	// display stuff
	int sprite;
	int dir;
};

#define MAXCHARS	8
extern struct character ch[];

// wear positions
#define WN_HEAD		0
#define WN_NECK		1
#define WN_BODY		2
#define WN_ARMS		3
#define WN_BELT		4
#define WN_LEGS		5
#define WN_FEET		6
#define WN_LHAND		7	// shield
#define WN_RHAND		8	// weapon
#define WN_CLOAK		9

// placement bits
#define PL_HEAD		1
#define PL_NECK		2
#define PL_BODY		4
#define PL_ARMS		8
#define PL_BELT		32
#define PL_LEGS		64
#define PL_FEET		128
#define PL_WEAPON		256
#define PL_2HANDED	512		// two-handed weapons
#define PL_SHIELD		1024		// not usable with two-handed weapons
#define PL_CLOAK		2048

struct item
{
	char name[40];
	int type;			// 1 = normal, 2=n charges
	int charges;		// only used for type 2
	int buy_value;		// first offer of merchant when bought
	int sell_value;	// first offer of merchant when sold
	int placement;		// see constants above
	int weight;			// weight. a character can carry 100xstrength without
							// loosing additional endurance. Above 100 he spends
							// weight% endurance.
	// add bulk?? I'd suggest no.

	// modifiers - modifiers apply only when the item is being worn.
	int braveness;
	int willpower;
	int intuition;
	int agility;
	int strength;
	int hp;
	int endurance;
	int fighting;
	int bargaining;
	int lock_picking;
	int stealth;
	int perception;
	int sixth_sense;
	int armor;
	int weapon;

	// map stuff
	int mission,x,y;
	int sprite,ovl;

};

#define MAXITEMS	7
extern struct item it[];


struct map
{
	// data for display. must be set by mapinit and all functions changing
	// objects
	int back;	// background image, painted first
	int obj;		// object image, painted next, overlays background
	int ovl;		// overlay image, painted last, overlays previous two
	// for game play
	int flags;	// bit 1 = lightsource, bit 2 = exit, bit 3 = LOS-block
	int dark;	// 0 = normal, 1-6, shadows to darkness
	int visible;
	// for fast access to objects
	int character;
	int item;
};

extern struct map *map;
extern int mapx,mapy;
extern int map_nr;

struct mission
{
	char name[80];
	char *briefing;
	char available;
	int mapnr;
	int pay;
	int sprite;

	// temporary values
	int x,y;
};

#define MAXMISSION	5
