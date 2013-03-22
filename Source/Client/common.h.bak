#define MNAME	"Asguard"
#define MHELP	"http://mercaston.dnsalias.net/help.html"
#define MNEWS	"http://mercaston.dnsalias.net/news.html"
#define MHOST	"mercaston.dnsalias.net"

/*#define MNAME	"Revolution"
#define MHELP	"http://192.168.0.5/help.html"
#define MNEWS	"http://192.168.0.5/news.html"
#define MHOST	"192.168.0.5"*/

//#define DOCONVERT	// enable sprite packer


#define TILEX			34
#define TILEY			34

#define TICKS			18

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
#define WN_LRING		10
#define WN_RRING    11

// placement bits
#define PL_HEAD		1
#define PL_NECK		2
#define PL_BODY		4
#define PL_ARMS		8
#define PL_BELT		32
#define PL_LEGS		64
#define PL_FEET		128
#define PL_WEAPON	256
#define PL_SHIELD	512
#define PL_CLOAK		1024
#define PL_TWOHAND	2048
#define PL_RING     4096

#define DX_RIGHT	1
#define DX_LEFT	2
#define DX_UP		3
#define DX_DOWN	4

#define INJURED                 (1u<<0)
#define INJURED1                (1u<<1)
#define INJURED2                (1u<<2)
#define STONED                  (1u<<3)
#define INFRARED                (1u<<4)
#define UWATER                  (1u<<5)

#define ISUSABLE                (1u<<7)
#define ISITEM                  (1u<<8)
#define ISCHAR                  (1u<<9)
#define INVIS						  (1u<<10)
#define STUNNED 					  (1u<<11)

#define TOMB                    ((1u<<12)|(1u<<13)|(1u<<14)|(1u<<15)|(1u<<16))
#define TOMB1                   (1u<<12)
#define DEATH                   ((1u<<17)|(1u<<18)|(1u<<19)|(1u<<20)|(1u<<21))
#define DEATH1                  (1u<<17)

#define EMAGIC                  ((1U<<22)|(1U<<23)|(1U<<24))
#define EMAGIC1                 (1U<<22)
#define GMAGIC                  ((1U<<25)|(1U<<26)|(1U<<27))
#define GMAGIC1                 (1U<<25)
#define CMAGIC                  ((1U<<28)|(1U<<29)|(1U<<30))
#define CMAGIC1                 (1U<<28)

#define MF_MOVEBLOCK    (1U<<0)
#define MF_SIGHTBLOCK   (1U<<1)
#define MF_INDOORS      (1U<<2)
#define MF_UWATER       (1U<<3)
#define MF_NOLAG        (1U<<4)
#define MF_NOMONST      (1U<<5)
#define MF_BANK         (1U<<6)
#define MF_TAVERN       (1U<<7)
#define MF_NOMAGIC      (1U<<8)
#define MF_DEATHTRAP    (1U<<9)

#define MF_ARENA        (1U<<11)

#define MF_NOEXPIRE     (1U<<13)

struct cmap {
	// common:

	unsigned short x,y;			// position

	// for background
	short int ba_sprite;			// background image
	unsigned char light;
	unsigned int flags;
	unsigned int flags2;

	// for character
	unsigned short ch_sprite;			// basic sprite of character
	unsigned char ch_status;	// what the character is doing, animation-wise
	unsigned char ch_stat_off;
	unsigned char ch_speed;		// speed of animation
	unsigned short ch_nr;			// character id
	unsigned short ch_id;			// character 'crc'
	unsigned char ch_proz;

	// for item
	short int it_sprite;			// basic sprite of item
	unsigned char it_status;	// for items with animation (burning torches etc)

	// for local computation -- client only:
	int back;	// background
	int obj1;	// item
	int obj2;	// character

	int obj_xoff,obj_yoff;
	int ovl_xoff,ovl_yoff;

	int idle_ani;
};

struct cplayer {
	// informative stuff
	char name[40];

	int mode;	// 0 = slow, 1 = medium, 2 = fast

	// character attributes+abilities
	// [0]=bare value, [1]=modifier, [2]=total value
	int attrib[5][6];
	int skill[100][6];
	int hp[6];
	int end[6];
	int mana[6];

	// temporary attributes
	int a_hp;
	int a_end;
	int a_mana;

	int points;
	int points_tot;
	int kindred;

	// posessions
	int gold;

	// items carried
	int item[40];
	int item_p[40];

	// items worn
	int worn[20];
	int worn_p[20];

	// spells ready
	int spell[20];
	int active[20];

	int armor;
	int weapon;

	int citem,citem_p;

	int attack_cn;
	int goto_x,goto_y;
	int misc_action,misc_target1,misc_target2;
	int dir;
};


// client message types (unsigned char):

#define CL_EMPTY				0
#define CL_NEWLOGIN			1
#define CL_LOGIN				2
#define CL_CHALLENGE		3
#define CL_PERF_REPORT		4
#define CL_CMD_MOVE			5
#define CL_CMD_PICKUP		6
#define CL_CMD_ATTACK		7
#define CL_CMD_MODE			8
#define CL_CMD_INV			9
#define CL_CMD_STAT			10
#define CL_CMD_DROP			11
#define CL_CMD_GIVE			12
#define CL_CMD_LOOK			13
#define CL_CMD_INPUT1		14
#define CL_CMD_INPUT2		15
#define CL_CMD_INV_LOOK		16
#define CL_CMD_LOOK_ITEM	17
#define CL_CMD_USE			18
#define CL_CMD_SETUSER		19
#define CL_CMD_TURN			20
#define CL_CMD_AUTOLOOK		21
#define CL_CMD_INPUT3		22
#define CL_CMD_INPUT4		23
#define CL_CMD_RESET			24
#define CL_CMD_SHOP			25
#define CL_CMD_SKILL			26
#define CL_CMD_INPUT5		27
#define CL_CMD_INPUT6		28
#define CL_CMD_INPUT7		29
#define CL_CMD_INPUT8		30
#define CL_CMD_EXIT       31
#define CL_CMD_UNIQUE       32
#define CL_PASSWD			33
#define CL_CMD_CTICK      255


// server message types (unsigned char):

#define SV_EMPTY						0
#define SV_CHALLENGE					1
#define SV_NEWPLAYER					2
#define SV_SETCHAR_NAME1			3
#define SV_SETCHAR_NAME2			4
#define SV_SETCHAR_NAME3			5
#define SV_SETCHAR_MODE				6

#define SV_SETCHAR_ATTRIB			7
#define SV_SETCHAR_SKILL			8

#define SV_SETCHAR_HP				12
#define SV_SETCHAR_ENDUR			13
#define SV_SETCHAR_MANA				14

#define SV_SETCHAR_AHP				20
#define SV_SETCHAR_PTS				21
#define SV_SETCHAR_GOLD				22
#define SV_SETCHAR_ITEM				23
#define SV_SETCHAR_WORN				24
#define SV_SETCHAR_OBJ				25

#define SV_TICK						27

#define SV_LOOK1						29
#define SV_SCROLL_RIGHT				30
#define SV_SCROLL_LEFT				31
#define SV_SCROLL_UP					32
#define SV_SCROLL_DOWN				33
#define SV_LOGIN_OK					34
#define SV_SCROLL_RIGHTUP       	35
#define SV_SCROLL_RIGHTDOWN     	36
#define SV_SCROLL_LEFTUP        	37
#define SV_SCROLL_LEFTDOWN		  	38
#define SV_LOOK2						39
#define SV_LOOK3						40
#define SV_LOOK4						41
#define SV_SETTARGET					42
#define SV_SETMAP2					43
#define SV_SETORIGIN	 			  	44
#define SV_SETMAP3					45
#define SV_SETCHAR_SPELL			46
#define SV_PLAYSOUND					47
#define SV_EXIT						48
#define SV_MSG							49
#define SV_LOOK5						50
#define SV_LOOK6						51

#define SV_LOG							52
#define SV_LOG0							52
#define SV_LOG1							53
#define SV_LOG2							54
#define SV_LOG3							55

#define SV_LOAD                 56
#define SV_CAP			57
#define SV_MOD1			58
#define SV_MOD2			59
#define SV_MOD3			60
#define SV_MOD4			61
#define SV_MOD5			62
#define SV_MOD6			63
#define SV_MOD7			64
#define SV_MOD8			65
#define SV_SETMAP4		66
#define SV_SETMAP5		67
#define SV_SETMAP6		68
#define SV_SETCHAR_AEND		69
#define SV_SETCHAR_AMANA	70
#define SV_SETCHAR_DIR		71
#define SV_UNIQUE		72
#define SV_IGNORE		73

#define SV_SETMAP	128

#define SPR_INVISIBLE			0
#define SPR_E1						1
#define SPR_E2						2
#define SPR_E3						3
#define SPR_E4						4
#define SPR_INV_MAP_PTR			5
#define SPR_MAP_PTR				6
#define SPR_FIGHT_PTR	  		7
#define SPR_INV_FIGHT_PTR		8
#define SPR_TAKE_PTR				9
#define SPR_INV_TAKE_PTR		10
#define SPR_INVALID_PTR			11
#define SPR_MISSION_SELECT		12		// fills 8 slots
#define SPR_FIGHT_INT			20		// fills 48 slots
#define SPR_INV_FIGHT_INT 		68		// fills 24 slots
#define SPR_LIGHT1				92
#define SPR_LIGHT2				93
#define SPR_LIGHT3				94
#define SPR_LIGHT4				95
#define SPR_LIGHT5				96
#define SPR_LIGHT6				97
#define SPR_EXIT					98
#define SPR_WALL_LB				99			// fills 2 slots
#define SPR_WALL_RB		  		101		// fills 2 slots
#define SPR_WALL_LT		  		103		// fills 2 slots
#define SPR_WALL_RT		  		105		// fills 2 slots
#define SPR_WALL_VERT	  		107
#define SPR_WALL_HORIZ			108		// fills 2 slots
#define SPR_DUNGEON_INT			110		// fills 48 slots
#define SPR_INV_DUNGEON_INT 	158		// fills 24 slots
#define SPR_INVENTORY_INT		182		// fills 36 slots
#define SPR_MISSION_INT			218		// fills 48 slots
#define SPR_INV_MISSION_INT 	266		// fills 24 slots
#define SPR_STAT_INT 			290		// fills 120 slots
#define SPR_RANK0					410		// fills 8 slots
#define SPR_RANK1					418		// fills 8 slots
#define SPR_RANK2					426		// fills 8 slots
#define SPR_RANK3					434		// fills 8 slots
#define SPR_RANK4					442		// fills 8 slots
#define SPR_RANK5					450		// fills 8 slots
#define SPR_RANK6					458		// fills 8 slots
#define SPR_RANK7					466		// fills 8 slots
#define SPR_RANK8					474		// fills 8 slots
#define SPR_RANK9					482		// fills 8 slots
#define SPR_RANK10				490		// fills 8 slots
#define SPR_RANK11				498		// fills 8 slots
#define SPR_RANK12				506		// fills 8 slots
#define SPR_RANK13				514		// fills 8 slots
#define SPR_RANK14				522		// fills 8 slots
#define SPR_RANK15				530		// fills 8 slots
#define SPR_RANK16				538		// fills 8 slots
#define SPR_RANK17				546		// fills 8 slots
#define SPR_RANK18				554		// fills 8 slots
#define SPR_RANK19				562		// fills 8 slots
#define SPR_RANK20				570		// fills 8 slots
#define SPR_SHOP_INT				578		// fills 48 slots
#define SPR_INV_SHOP_INT 		626		// fills 24 slots
#define SPR_BUTTONBOX3			660		// fills 2 slots

#define SPR_FONT					700		// fills 16 slots
#define SPR_LOOK_INT				716		// fills 120 slots

#define SPR_EMPTY					 999
#define SPR_TORCH3				1000
#define SPR_GROUND2				1002
#define SPR_HELMET				1003
#define SPR_BODY_ARMOR 			1004
#define SPR_LEG_ARMOR			1005
#define SPR_SWORD					1006
#define SPR_DAGGER				1007
#define SPR_GROUND1				1008
#define SPR_CHEST_KEY			1009
#define SPR_STONE_GROUND1		1010
#define SPR_TORCH1				1011
#define SPR_PACKET0				1012
#define SPR_PACKET1				1013
#define SPR_PACKET2				1014
#define SPR_PACKET3				1015
#define SPR_PACKET4				1016
#define SPR_PACKET5				1017
#define SPR_PACKET6				1018
#define SPR_PACKET7				1019
#define SPR_PACKET8				1020
#define SPR_PACKET9				1021
#define SPR_PACKET_OVR			1022
#define SPR_PACKET_UDR			1023
#define SPR_PACKET_BRAKES		1024
#define SPR_INT_BOX				1025
#define SPR_TORCH2				1026	// fills eight slots

#define SPR_NEWBIE				2048
#define SPR_CHAR0					2048		// fills 256 slots
#define SPR_CHAR1					2048		// fills 256 slots

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

extern int ctick;
