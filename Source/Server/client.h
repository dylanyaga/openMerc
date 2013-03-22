/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <zlib.h>

#define TILEX			34
#define TILEY			34

#define INJURED		(1u<<0)
#define INJURED1	(1u<<1)
#define INJURED2	(1u<<2)
#define STONED		(1u<<3)
#define INFRARED	(1u<<4)
#define UWATER		(1u<<5)

#define ISUSABLE	(1u<<7)
#define ISITEM		(1u<<8)
#define ISCHAR		(1u<<9)
#define INVIS		(1<<10)
#define STUNNED		(1<<11)
#define TOMB                    ((1u<<12)|(1u<<13)|(1u<<14)|(1u<<15)|(1u<<16))
#define TOMB1                   (1u<<12)
#define DEATH                   ((1u<<17)|(1u<<18)|(1u<<19)|(1u<<20)|(1u<<21))
#define DEATH1                  (1u<<17)
#define EMAGIC                  ((1U<<22)|(1U<<23)|(1U<<24))
#define EMAGIC1                 (1U<<22)
#define GMAGIC                  ((1U<<25)|(1U<<26)|(1U<<27))
#define GMAGIC1                 (1U<<25)
#define CMAGIC                  ((1U<<28)|(1U<<29)|(1U<<30))
#define CMAGIC1                 (1U<<28

struct cmap
{
	// for background
	short int ba_sprite;		// background image
	unsigned char light;
	unsigned int flags;
	unsigned int flags2;

	// for character
	short int ch_sprite;		// basic sprite of character
	unsigned char ch_status2;
	unsigned char ch_status;  	// what the character is doing, animation-wise
	unsigned char ch_speed;		// speed of animation
	unsigned short ch_nr;
	unsigned short ch_id;
	unsigned char ch_proz;		// health in percent

	// for item
	short int it_sprite;		// basic sprite of item
	unsigned char it_status;  	// for items with animation (burning torches etc)
} __attribute__ ((packed));

struct cplayer
{
	// informative stuff
	char name[40];

	int mode;		// 0 = slow, 1 = medium, 2 = fast

	// character stats
	// [0]=bare value, 0=unknown
	// [1]=preset modifier, is race/npc dependend
	// [2]=race specific maximum
	// [3]=race specific difficulty to raise (0=not raisable, 1=easy ... 10=hard)
	// [4]=dynamic modifier, depends on equipment and spells
	// [5]=total value

	unsigned char attrib[5][6];

	unsigned short hp[6];
	unsigned short end[6];
	unsigned short mana[6];

	unsigned char skill[100][6];

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

	int spell[20];
	char active[20];

	int weapon;
	int armor;

	int citem,citem_p;

	int attack_cn;
	int goto_x,goto_y;
	int misc_action,misc_target1,misc_target2;
	int dir;

	// server only:
	int x,y;
};


// client message types (unsigned char):

#define CL_EMPTY			0
#define CL_NEWLOGIN			1
#define CL_LOGIN			2
#define CL_CHALLENGE			3
#define CL_PERF_REPORT			4
#define CL_CMD_MOVE			5
#define CL_CMD_PICKUP			6
#define CL_CMD_ATTACK			7
#define CL_CMD_MODE			8
#define CL_CMD_INV			9
#define CL_CMD_STAT			10
#define CL_CMD_DROP			11
#define CL_CMD_GIVE			12
#define CL_CMD_LOOK			13
#define CL_CMD_INPUT1			14
#define CL_CMD_INPUT2			15
#define CL_CMD_INV_LOOK			16
#define CL_CMD_LOOK_ITEM		17
#define CL_CMD_USE			18
#define CL_CMD_SETUSER			19
#define CL_CMD_TURN			20
#define CL_CMD_AUTOLOOK			21
#define CL_CMD_INPUT3			22
#define CL_CMD_INPUT4			23
#define CL_CMD_RESET			24
#define CL_CMD_SHOP			25
#define CL_CMD_SKILL			26
#define CL_CMD_INPUT5			27
#define CL_CMD_INPUT6			28
#define CL_CMD_INPUT7			29
#define CL_CMD_INPUT8			30
#define CL_CMD_EXIT			31
#define CL_CMD_UNIQUE			32
#define CL_PASSWD			33

#define CL_CMD_CTICK			255

// server message types (unsigned char):

#define SV_EMPTY		0
#define SV_CHALLENGE		1
#define SV_NEWPLAYER		2
#define SV_SETCHAR_NAME1	3
#define SV_SETCHAR_NAME2	4
#define SV_SETCHAR_NAME3	5
#define SV_SETCHAR_MODE		6

#define SV_SETCHAR_ATTRIB	7
#define SV_SETCHAR_SKILL	8

#define SV_SETCHAR_HP		12
#define SV_SETCHAR_ENDUR	13
#define SV_SETCHAR_MANA		14

#define SV_SETCHAR_AHP		20
#define SV_SETCHAR_PTS		21
#define SV_SETCHAR_GOLD		22
#define SV_SETCHAR_ITEM		23
#define SV_SETCHAR_WORN		24
#define SV_SETCHAR_OBJ		25

#define SV_TICK			27

#define SV_LOOK1		29
#define SV_SCROLL_RIGHT		30
#define SV_SCROLL_LEFT		31
#define SV_SCROLL_UP		32
#define SV_SCROLL_DOWN		33
#define SV_LOGIN_OK		34
#define SV_SCROLL_RIGHTUP	35
#define SV_SCROLL_RIGHTDOWN	36
#define SV_SCROLL_LEFTUP	37
#define SV_SCROLL_LEFTDOWN	38
#define SV_LOOK2		39
#define SV_LOOK3		40
#define SV_LOOK4		41
#define SV_SETTARGET		42
#define SV_SETMAP2		43
#define SV_SETORIGIN		44
#define SV_SETMAP3		45
#define SV_SETCHAR_SPELL	46
#define SV_PLAYSOUND		47
#define SV_EXIT			48
#define SV_MSG			49
#define SV_LOOK5		50
#define SV_LOOK6		51

#define SV_LOG			52
#define SV_LOG0			52
#define SV_LOG1			53
#define SV_LOG2			54
#define SV_LOG3			55

#define SV_LOAD			56
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

#define SV_SETMAP		128	// 128-255 are used !!!

#define LO_CHALLENGE		1
#define LO_IDLE			2
#define LO_NOROOM		3
#define LO_PARAMS		4
#define LO_NONACTIVE		5
#define LO_PASSWORD		6
#define LO_SLOW			7
#define LO_FAILURE		8
#define LO_SHUTDOWN		9
#define LO_TAVERN		10
#define LO_VERSION		11
#define LO_EXIT			12
#define LO_USURP		13
#define LO_KICKED		14

#define SPR_INVISIBLE		0
#define SPR_GENERIC_INT		1
#define SPR_FONT		2
#define SPR_INV_INT		3
#define SPR_BLOCK_INV		4
#define SPR_MAGIC_INT		5
#define SPR_LOOK_INT		6

#define SPR_RANK0		10
#define SPR_RANK1		11
#define SPR_RANK2		12
#define SPR_RANK3		13
#define SPR_RANK4		14
#define SPR_RANK5		15
#define SPR_RANK6		16
#define SPR_RANK7		17
#define SPR_RANK8		18
#define SPR_RANK9		19
#define SPR_RANK10		20
#define SPR_RANK11		21
#define SPR_RANK12		22
#define SPR_RANK13		23
#define SPR_RANK14		24
#define SPR_RANK15		25
#define SPR_RANK16		26
#define SPR_RANK17		27
#define SPR_RANK18		28
#define SPR_RANK19		29
#define SPR_RANK20		30

#define SPR_COIN5		36
#define SPR_COIN2		37
#define SPR_COIN1		38
#define SPR_COIN3		39
#define SPR_COIN4		40

#define SPR_FONT_SMALL		701

#define SPR_EMPTY		999

#define SPR_TUNDRA_GROUND	1001
#define SPR_DESERT_GROUND	1002
#define SPR_HELMET		1003
#define SPR_BODY_ARMOR 		1004
#define SPR_LEG_ARMOR		1005
#define SPR_SWORD		1006
#define SPR_DAGGER		1007
#define SPR_GROUND1		1008
#define SPR_KEY			1009
#define SPR_STONE_GROUND1	1010
#define SPR_TORCH1		1011
#define SPR_LIZARD_POOL		1012
#define SPR_WOOD_GROUND		1013
#define SPR_CLOAK		1014
#define SPR_BELT		1015
#define SPR_AMULET		1016
#define SPR_BOOTS		1017
#define SPR_ARM_ARMOR		1018
#define SPR_TEMPLAR_POOL	1019

#define SPR_TORCH2		1026

#define SPR_TAVERN_GROUND	1034
#define SPR_GOLD_CLOAK		1035
#define SPR_ARM_GOLD_ARMOR	1036
#define SPR_STEEL_BOOTS		1037
#define SPR_GOLD_BOOTS		1038
#define SPR_LEG_GOLD_ARMOR	1039
#define SPR_BODY_GOLD_ARMOR	1040
#define SPR_GOLD_HELMET		1041
#define SPR_STEEL_CLOAK		1042
#define SPR_GOLD_SWORD		1043
#define SPR_LIGHT_GREEN		1044

#define SPR_STONE_GROUND2	1052
#define SPR_LEATHER_LEGGINGS	1053
#define SPR_LEATHER_SLEEVES	1054
#define SPR_BODY_LEATHER_ARMOR	1055
#define SPR_LEATHER_HELMET	1056
#define SPR_LEATHER_CLOAK	1057
#define SPR_CLUB		1058

#define SPR_LIGHT_GREEN2	1060
#define SPR_LIGHT_GREEN3	1068

#define SPR_WALL		1200
#define SPR_MOUNTAIN_WALL	1202
#define SPR_HIGH_MOUNTAIN	1204
#define SPR_MEDIUM_MOUNTAIN	1206
#define SPR_LOW_MOUNTAIN	1208
#define SPR_STONE_WALL		1210

#define SPR_TEMPLAR		2000
#define SPR_LIZARD		(SPR_TEMPLAR+1024)
#define SPR_HARAKIM		(SPR_LIZARD+1024)
#define SPR_MERCENARY		(SPR_HARAKIM+1024)

struct player
{
	int sock;
	unsigned int addr;
	int version;
	int race;

	unsigned char inbuf[256];	// all incoming packets have 16 bytes
	int in_len;
	unsigned char *tbuf;
	unsigned char *obuf;
	int iptr,optr,tptr;

	unsigned int challenge;
	unsigned int state;
	unsigned int lasttick,lasttick2;
	unsigned int usnr;
	unsigned int pass1,pass2;
	unsigned int ltick,rtick;

	int prio;

	struct cplayer cpl;
	struct cmap cmap[TILEX*TILEY];
	struct cmap smap[TILEX*TILEY];
	
	// copy of map for comparision
	struct map xmap[TILEX*TILEY];
	
	// copy of visibility map for comparision
	int vx,vy;
	char visi[40*40];
	
	char input[128];

	// for compression
        struct z_stream_s zs;
        
        int ticker_started;

	unsigned long long unique;

	char passwd[16];

	int changed_field[TILEX*TILEY];	// IDs of changed fields
};

#define MAXPLAYER		250

#define ST_CONNECT		0
#define ST_NEW_CHALLENGE	1
#define ST_LOGIN_CHALLENGE	2
#define ST_NEWLOGIN		3
#define ST_LOGIN		4
#define ST_NEWCAP		5
#define ST_CAP			6

#define ST_NORMAL		10
#define ST_CHALLENGE		11

#define ST_EXIT			12

extern struct player player[];

