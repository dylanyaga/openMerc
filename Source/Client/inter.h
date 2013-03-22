#define VERSION 0x020E06

#define MAXSPRITE 2000+(32*1024)

#define MODEX			800
#define MODEY			600

//#define DB_WINDOWED

#define MAPX			TILEX
#define MAPY			TILEY

#define YPOS			440
#define XPOS			0

#define MS_MOVE		    0
#define MS_LB_DOWN	    1
#define MS_RB_DOWN	    2
#define MS_LB_UP		3
#define MS_RB_UP		4

#define TICK			(1000/TICKS)

#define HIGH_VAL		(1<<30)

#define QSIZE			8

struct xbutton
{
   char name[8];
   int skill_nr;
//   int skill_strength;
};


struct pdata
{
	char cname[80];
	char ref[80];
	char desc[160];

	char changed;

  int hide;
  int show_names;
  int show_proz;
  struct xbutton xbutton[12];
};

extern struct pdata pdata;

extern struct cplayer pl;
extern struct cmap *map;

void mouse(int x,int y,int state);

void cmd1s(int cmd,int val);
void add_look(unsigned short nr,char *name,unsigned short id);
int attrib_needed(int n,int v);
int hp_needed(int v);
int end_needed(int v);
int mana_needed(int v);
int skill_needed(int n,int v);
void xlog(char font,char *format,...);
int play_sound(char *file,int vol,int pan);
void reset_block(void);

void save_options(void);
void load_options(void);
void options(void);

void xsend(unsigned char *buf);
void engine_tick(void);
void so_perf_report(int ticksize,int skip,int idle);
int game_loop(void);
int tick_do(void);
void init_engine(void);

void button_command(int nr);
void cmd_exit(void);
void cmds(int cmd,int x,int y);

struct key
{
	unsigned int usnr;
	unsigned int pass1,pass2;
	char name[40];
  int race;
};

struct look
{
	unsigned char autoflag;
	unsigned short worn[20];
	unsigned short sprite;
	unsigned int points;
	char name[40];
	unsigned int hp;
	unsigned int end;
	unsigned int mana;
	unsigned int a_hp;
	unsigned int a_end;
   unsigned int a_mana;
	unsigned short nr;
	unsigned short id;
	unsigned char extended;
	unsigned short item[62];
	unsigned int price[62];
   unsigned int pl_price;
};

extern struct look look;
extern struct look shop;
extern unsigned int show_shop;

#define HL_BUTTONBOX	1
#define HL_STATBOX	2
#define HL_BACKPACK	3
#define HL_EQUIPMENT	4
#define HL_SPELLBOX	5
#define HL_CITEM		6
#define HL_MONEY		7
#define HL_MAP			8
#define HL_SHOP		9
#define HL_STATBOX2	10

extern int hightlight;
extern int hightlight_sub;

#define CT_NONE		1
#define CT_TAKE		2
#define CT_DROP		3
#define CT_USE			4
#define CT_GIVE		5
#define CT_WALK		6
#define CT_HIT			7
#define CT_SWAP		8
#define CT_SEL			9

struct skilltab
{
		int nr;
		  char sortkey;
		  char name[40];
		  char desc[200];

		  int attrib[3];
};

extern struct skilltab *skilltab;
extern char *at_name[];

void dd_puttext(int x,int y,int font,char *text);
void dd_gputc(int xpos,int ypos,int font,int c);
void dd_gputtext(int xpos,int ypos,int font,char *text,int xoff,int yoff);
void dd_putc(int xpos,int ypos,char font,int c);
void dd_xputtext(int x,int y,int font,char *format,...);
void say(char *input);
void cmd1(int cmd,int x);
void init_xalloc(void);
void tlog(char *text,char font);
void do_msg(void);
