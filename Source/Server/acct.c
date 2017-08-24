/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "cgi-lib.h" /* include the cgi-lib.h header file */
#include "html-lib.h" /* include the html-lib.h header file */
#include "gendefs.h"
#include "data.h"



/* CS, 991113: TCHARSIZE and TITEMSIZE now in data.h */

struct character *ch_temp;
struct character *ch;
struct item *it;
struct item *it_temp;
struct global *globs;


// struct character *ch_temp;
// struct item *it_temp;
// struct character *ch_temp;
// struct item *it_temp;

#if 0
unsigned long long atoll(char *string)
{
	unsigned long long val = 0;

	while (isspace(*string))
	{
		string++;
	}

	while (isdigit(*string))
	{
		val *= 10;
		val += *string - '0';
		string++;
	}
	return(val);
}
#endif

char *at_name[5] = {
	"Braveness",
	"Willpower",
	"Intuition",
	"Agility",
	"Strength"
};

static char *weartext[20] = {
	"Head",
	"Neck",
	"Body",
	"Arms",
	"Belt",
	"Legs",
	"Feet",
	"Left Hand",
	"Right Hand",
	"Cloak",
	"Left Ring",
	"Right Ring"
};

static char *text_name[10] = {
	"Killed Enemy $1", //0
	"Will attack $1", //1
	"Greeting $1",  //2
	"Killed by $1", //3
	"Shout Help against $1",//4
	"Going to Help $1", //5
	"Keyword",      //6
	"Rection to keyword", //7
	"Warning about attack", //8
	"Unused", //9
};

static char *player_text_name[10] = {
	"(Clobbered by Login) AFK Message", //0
	"(Clobbered by Login)", //1
	"(Clobbered by Login)",  //2
	"Mark Message", //3
	"Unused",//4
	"Unused", //5
	"Unused",      //6
	"Unused", //7
	"Unused", //8
	"Unused", //9
};

static char *player_data_name[100] = {
	"Away from Keyboard", //0
	"Group With", //1
	"Group With", //2
	"Group With", //3
	"Group With", //4
	"Group With", //5
	"Group With", //6
	"Group With", //7
	"Group With", //8
	"Group With", //9
	"Following",   //10
	"No Fightback",   //11
	"Follow Suspension Timeout",   //12
	"Money in Bank (in silver)",   //13
	"Number of Deaths",   //14
	"Killed By X",   //15
	"Date",   //16
	"Area",   //17
	"Current Pentagram Experience",   //18
	"Lag Timer", //19
	"Highest Gorge Solved", //20
	"Seyan'Du Sword Shrines Used", //21
	"Current Monster in Arena", //22
	"Overall Kill Counter (Below Rank)", //23
	"Overall Kill Counter (At Rank)", //24
	"Overall Kill Counter (Above Rank)", //25
	"BS Kill Counter (Below Rank)", //26
	"BS Kill Counter (At Rank)", //27
	"BS Kill Counter (Above Rank)",   //28
	"Other Players Killed Outside Arena", //29
	"Soft Ignore List", //30
	"Protect Templ", //31
	"Soft Ignore List", //32
	"Soft Ignore List", //33
	"Soft Ignore List", //34
	"Soft Ignore List", //35
	"Soft Ignore List", //36
	"Soft Ignore List", //37
	"Soft Ignore List", //38
	"Soft Ignore List", //39
	"Shop Keepers Etc. Killed", //40
	"BS Points Already Spent", //41
	"Group",    //42
	"Black Candles Returned", //43
	"Number of Times Saved by the Gods", //44
	"Level X Got HP/END/MANA Bonus For", //45
	"Flags of Explorer Points Visited", //46
	"Flags of Explorer Points Visited", //47
	"Flags of Explorer Points Visited", //48
	"Flags of Explorer Points Visited", //49
	"Hard Ignore List", //50
	"Hard Ignore List", //51
	"Hard Ignore List", //52
	"Hard Ignore List", //53
	"Hard Ignore List", //54
	"Hard Ignore List", //55
	"Hard Ignore List", //56
	"Hard Ignore List", //57
	"Hard Ignore List", //58
	"Hard Ignore List", //59
	"Flags of Killed NPC of Certain Class", //60
	"Flags of Killed NPC of Certain Class", //61
	"Flags of Killed NPC of Certain Class", //62
	"Flags of Killed NPC of Certain Class", //63
	"Current Ghost Companion", //64
	"Player #ALLOWed to Search Corpse", //65
	"Corpse's Owner", //66
	"Riddle Giver (Lab 9)", //67
	"Date of Last Player Attack (CHD_ATTACKTIME)", //68
	"Last Player Attackeed (CHD_ATTACKVICT)", //69
	"Number of Fire Points", //70
	"Number of Says/Tells/Looks/Lights", //71
	"Bad Word Score", //72
	"Unused", //73
	"Unused", //74
	"Unused", //75
	"Unused", //76
	"Unused", //77
	"Unused", //78
	"Unused", //79
	"Seen Loggin in from these Class C nets", //80
	"Seen Loggin in from these Class C nets", //81
	"Seen Loggin in from these Class C nets", //82
	"Seen Loggin in from these Class C nets", //83
	"Seen Loggin in from these Class C nets", //84
	"Seen Loggin in from these Class C nets", //85
	"Seen Loggin in from these Class C nets", //86
	"Seen Loggin in from these Class C nets", //87
	"Seen Loggin in from these Class C nets", //88
	"Seen Loggin in from these Class C nets", //89
	"Number in Database", //90
	"Unused", //91
	"Reserved", //92
	"Unused", //93
	"Unused", //94
	"Unused", //95
	"Queued Spells", //96
	"Computation Time Used Last Action",   //97
	"Computation Time Used",   //98
	"Used by Populate"  //99
};

static char *data_name[100] = {
	"Generic 1",    //0
	"Generic 2",    //1
	"Generic 3",    //2
	"Generic 4",    //3
	"Generic 5",    //4
	"Generic 6",    //5
	"Generic 7",    //6
	"Generic 8",    //7
	"Generic 9",    //8
	"Generic 10",   //9
	"Patrol",       //10
	"Patrol",       //11
	"Patrol",       //12
	"Patrol",       //13
	"Patrol",       //14
	"Patrol",       //15
	"Patrol",       //16
	"Patrol",       //17
	"Patrol",       //18
	"Reserved",     //19
	"Close Door",   //20
	"Close Door",   //21
	"Close Door",   //22
	"Close Door",   //23
	"Prevent Fights", //24
	"Special Driver", //25
	"Special Sub-Driver", //26
	"Reserved",     //27
	"Unused",       //28
	"Resting Position", //29
	"Resting Dir",  //30
	"Protect Templ", //31
	"Activate Light", //32
	"Activate Light", //33
	"Activate Light", //34
	"Activate Light", //35
	"Reserved",     //36
	"Greet everyone", //37
	"Reserved",     //38
	"Reserved",     //39
	"Reserved",     //40
	"Street Light Templ.", //41
	"Group",        //42
	"Attack all but Group", //43
	"Attack all but Group", //44
	"Attack all but Group", //45
	"Attack all but Group", //46
	"Donate Trash to pl #", //47
	"Prob. of dying msg", //48
	"Wants item #", //49
	"Teaches Skill #", //50
	"Gives # EXPs", //51
	"Shout help/code #", //52
	"Come help/code #", //53
	"Reserved",     //54
	"Reserved",     //55
	"Reserved",     //56
	"Reserved",     //57
	"Reserved",     //58
	"Protect Group", //59
	"Random Walk Time", //60
	"Reserved",     //61
	"Create Light", //62
	"Reserved",     //63
	"Self Destruct", //64
	"Reserved",     //65
	"Gives Item #", //66
	"Reserved",     //67
	"Level of Knowledge", //68
	"Accepts Money", //69
	"Reserved",     //70
	"Talkativity",  //71
	"Area of Knowledge", //72
	"Random Walk Max Dist", //73
	"Reserved",     //74
	"Reserved",     //75
	"Reserved",     //76
	"Reserved",     //77
	"Reserved",     //78
	"Rest between patrol", //79
	"Reserved",     //80
	"Reserved",     //81
	"Reserved",     //82
	"Reserved",     //83
	"Reserved",     //84
	"Reserved",     //85
	"Reserved",     //86
	"Reserved",     //87
	"Reserved",     //88
	"Reserved",     //89
	"Reserved",     //90
	"Reserved",     //91
	"Reserved",     //92
	"Attack distance", //93
	"Reserved",     //94
	"Keyword Action", //95
	"Reserved",     //96
	"Unused",       //97
	"Unused",       //98
	"Reserved"      //99
};

static char *obj_drdata[] = {
	"Drdata 0",
	"Drdata 1",
	"Drdata 2",
	"Drdata 3",
	"Drdata 4",
	"Drdata 5",
	"Drdata 6",
	"Drdata 7",
	"Drdata 8",
	"Drdata 9"
};

#define bad_name_count 56
static char *bad_names[bad_name_count] = {
	"Guard",
	"Janitor",
	"Peacekeeper",
	"Merchant",
	"Barkeeper",
	"Cityguard",
	"Librarian",
	"Amazon",
	"Swordsman",
	"Thief",
	"Adventurer",
	"Tower Guard",
	"Tower Ratling",
	"Tower Greenling",
	"Tower Skeleton",
	"Tower Seyan'Du",
	"Tower Warrior",
	"Tower Sorceress",
	"Golem",
	"Ghost",
	"Gatekeeper",
	"Skeleton",
	"Outlaw",
	"Manager",
	"Lizard Youngster",
	"Lizard Youth",
	"Lizard Worker",
	"Lizard Fighter",
	"Lizard Warrior",
	"Lizard Mage",
	"Bartender",
	"Spellcaster",
	"Spellcaster Guard",
	"Keeper",
	"Knight",
	"Undead",
	"Spider",
	"Robber",
	"Undead Lord",
	"Lord",
	"Flame",
	"Lizard Knight",
	"Lizard Archmage",
	"Undead King",
	"Spellcaster Lord",
	"Tower Arch-Templar",
	"Tower Arch-Harakim",
	"Lizard Defender",
	"Elite Lizard Guard",
	"Mage",
	"Archmage",
	"Swindler",
	"Sea Golem",
	"Barbaric Fighter",
	"Lizard Attacker",
	"Lizard Necromancer"
};

int extend(int handle, long sizereq, size_t sizeone, void*templ)
{
	long length;
	void *buffer;

	length = lseek(handle, 0L, SEEK_END);
	if (length < sizereq)
	{
		fprintf(stderr, "Current size = %ldK, extending to %ldK", length / 1024, sizereq / 1024);
		buffer = templ;
		if (buffer == NULL)
		{
			buffer = calloc(1, sizeone);             // automatically clears memory
		}
		if (buffer == NULL)
		{
			return( 0);           // calloc() failure
		}
		for (; length < sizereq; length += sizeone)
		{
			int success = write(handle, buffer, sizeone);
		}
		if (templ == NULL)
		{
			free(buffer);
		}
	}
	return(1); // success
}


static int web_load(void)
{
	int handle = 0;

	/** CHAR **/
	fprintf(stderr, "Loading CHAR: Item size=%d, file size=%dK",
	        sizeof(struct character), CHARSIZE >> 10);

	handle = open(DATDIR "/char.dat", O_RDWR);
	if (handle==-1)
	{
		fprintf(stderr, "Unable to load CHAR");
		return( -1);
	}
	if (!extend(handle, CHARSIZE, sizeof(struct character), NULL))
	{
		fprintf(stderr, "Unable to extend CHAR");
		return( -1);
	}

	ch = mmap(NULL, CHARSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (ch==(void*)-1)
	{
		fprintf(stderr, "Unable to mmap CHAR");
		return( -1);
	}
	close(handle);

	/** TCHAR **/
	fprintf(stderr, "Loading TCHAR: Item size=%d, file size=%dK",
	        sizeof(struct character), TCHARSIZE >> 10);

	handle = open(DATDIR "/tchar.dat", O_RDWR);
	if (handle==-1)
	{
		fprintf(stderr, "Unable to load TCHAR");
		return( -1);
	}
	if (!extend(handle, TCHARSIZE, sizeof(struct character), NULL))
	{
		fprintf(stderr, "Unable to extend TCHAR");
		return( -1);
	}

	ch_temp = mmap(NULL, TCHARSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (ch_temp==(void*)-1)
	{
		fprintf(stderr, "Unable to mmap TCHAR");
		return( -1);
	}
	close(handle);

	/** ITEM **/
	fprintf(stderr, "Loading ITEM: Item size=%d, file size=%dK",
	        sizeof(struct item), ITEMSIZE >> 10);

	handle = open(DATDIR "/item.dat", O_RDWR);
	if (handle==-1)
	{
		fprintf(stderr, "Unable to load ITEM");
		return( -1);
	}
	if (!extend(handle, ITEMSIZE, sizeof(struct item), NULL))
	{
		fprintf(stderr, "Unable to extend ITEM");
		return( -1);
	}


	it = mmap(NULL, ITEMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (it==(void*)-1)
	{
		fprintf(stderr, "Unable to mmap ITEM");
		return( -1);
	}
	close(handle);

	/** TITEM **/
	fprintf(stderr, "Loading TITEM: Item size=%d, file size=%dK",
	        sizeof(struct item), TITEMSIZE >> 10);

	handle = open(DATDIR "/titem.dat", O_RDWR);
	if (handle==-1)
	{
		fprintf(stderr, "Unable to load TITEM");
		return( -1);
	}
	if (!extend(handle, TITEMSIZE, sizeof(struct item), NULL))
	{
		fprintf(stderr, "Unable to extend TITEM");
		return( -1);
	}

	it_temp = mmap(NULL, TITEMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (it_temp==(void*)-1)
	{
		fprintf(stderr, "Unable to mmap TITEM");
		return( -1);
	}
	close(handle);

	/** GLOBS **/
	fprintf(stderr, "Loading GLOBS: Item size=%d, file size=%db",
	        sizeof(struct global), sizeof(struct global));

	handle = open(DATDIR "/global.dat", O_RDWR);
	if (handle==-1)
	{
		fprintf(stderr, "Unable to load GLOBS");
		return( -1);
	}
	if (!extend(handle, GLOBSIZE, sizeof(struct global), NULL))
	{
		fprintf(stderr, "Unable to load GLOBS");
		return( -1);
	}

	globs = mmap(NULL, sizeof(struct global), PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (globs==(void*)-1)
	{
		fprintf(stderr, "Unable to mmap GLOBS");
		return( -1);
	}
	close(handle);

	return(0);
}

static void web_unload(void)
{
	fprintf(stderr, "Unloading data files");

	if (munmap(ch, CHARSIZE))
	{
		fprintf(stderr, "ERROR: munmap(ch) %s", strerror(errno));
	}
	if (munmap(it, ITEMSIZE))
	{
		fprintf(stderr, "ERROR: munmap(it_temp) %s", strerror(errno));
	}
	if (munmap(ch_temp, TCHARSIZE))
	{
		fprintf(stderr, "ERROR: munmap(ch_temp) %s", strerror(errno));
	}
	if (munmap(it_temp, TITEMSIZE))
	{
		fprintf(stderr, "ERROR: munmap(it_temp) %s", strerror(errno));
	}
	if (munmap(globs, sizeof(struct global)))
	{
		fprintf(stderr, "ERROR: munmap(globs) %s", strerror(errno));
	}
}




void list_all_player_characters()
{
	int n;
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<table>\n");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=20&cn=%d>%s</a></td></tr>", n, n, ch[n].name);
	}
	printf("</table>\n");
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
}

void copy_character_template(LIST *head)
{
	int cn, n;
	char *tmp;

	tmp = find_val(head, "cn");
	if (tmp)
	{
		cn = atoi(tmp);
	}
	else
	{
		printf("No number specified.\n");
		return;
	}

	if (cn<0 || cn>=MAXTCHARS)
	{
		printf("Number out of bounds.\n");
		return;
	}

	for (n = 1; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			break;
		}
	}
	if (n==MAXTCHARS)
	{
		printf("MAXTCHARS reached!\n");
		return;
	}
	ch_temp[n] = ch_temp[cn];

	printf("Done. Here is your new, copied monster\n");
	printf("<table>\n"); // show copied monster for direct access.


//                if (ch_temp[n].used==USE_EMPTY) continue;
	printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
	       "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
	       n, n,
	       (ch_temp[n].flags & CF_RESPAWN) ? "<b>" : "",
	       ch_temp[n].name,
	       (ch_temp[n].flags & CF_RESPAWN) ? "</b>" : "",
	       ch_temp[n].x, ch_temp[n].y, ch_temp[n].points_tot >> 10, ch_temp[n].alignment, n, n);

	printf("</table>\n");


}

void view_character_template(LIST *head)
{
	int cn, n;
	char *tmp;

	tmp = find_val(head, "cn");
	if (tmp)
	{
		cn = atoi(tmp);
	}
	else
	{
		printf("No number specified.\n");
		return;
	}

	if (cn<0 || cn>=MAXTCHARS)
	{
		printf("Number out of bounds.\n");
		return;
	}
	printf("<center><a href=/cgi-imp/acct.cgi?step=11>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<form method=post action=/cgi-imp/acct.cgi>\n");
	printf("<table>\n");
	printf("<tr><td>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>\n",
	       ch_temp[cn].name);
	printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>\n",
	       ch_temp[cn].reference);
	printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>\n",
	       ch_temp[cn].description);

	printf("<tr><td valign=top>Kindred:</td><td>\n");
	printf("<input type=checkbox name=kindred value=%d %s>Mercenary<br>\n",
	       KIN_MERCENARY, (ch_temp[cn].kindred & KIN_MERCENARY) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Sorcerer-Mercenary<br>\n\n",
	       KIN_SORCERER, (ch_temp[cn].kindred & KIN_SORCERER) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Warrior-Mercenary<br>\n",
	       KIN_WARRIOR, (ch_temp[cn].kindred & KIN_WARRIOR) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Templar<br>\n",
	       KIN_TEMPLAR, (ch_temp[cn].kindred & KIN_TEMPLAR) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Arch-Templar<br>\n",
	       KIN_ARCHTEMPLAR, (ch_temp[cn].kindred & KIN_ARCHTEMPLAR) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Harakim<br>\n",
	       KIN_HARAKIM, (ch_temp[cn].kindred & KIN_HARAKIM) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Arch-Harakim<br>\n",
	       KIN_ARCHHARAKIM, (ch_temp[cn].kindred & KIN_ARCHHARAKIM) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Seyan'Du<br>\n",
	       KIN_SEYAN_DU, (ch_temp[cn].kindred & KIN_SEYAN_DU) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Purple<br>\n",
	       KIN_PURPLE, (ch_temp[cn].kindred & KIN_PURPLE) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Monster<br>\n",
	       KIN_MONSTER, (ch_temp[cn].kindred & KIN_MONSTER) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Male<br>\n",
	       KIN_MALE, (ch_temp[cn].kindred & KIN_MALE) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Female<br>\n",
	       KIN_FEMALE, (ch_temp[cn].kindred & KIN_FEMALE) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Sprite base:</td><td><input type=text name=sprite value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].sprite);

	printf("<tr><td valign=top>Sound base:</td><td><input type=text name=sound value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].sound);

	printf("<tr><td valign=top>Class:</td><td><input type=text name=class value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].class);

	printf("<tr><td valign=top>Flags:</td><td>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Infrared<br>\n",
	       CF_INFRARED, (ch_temp[cn].flags & CF_INFRARED) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Undead<br>\n",
	       CF_UNDEAD, (ch_temp[cn].flags & CF_UNDEAD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Respawn<br>\n",
	       CF_RESPAWN, (ch_temp[cn].flags & CF_RESPAWN) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Sleep<br>\n",
	       CF_NOSLEEP, (ch_temp[cn].flags & CF_NOSLEEP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Merchant<br>\n",
	       CF_MERCHANT, (ch_temp[cn].flags & CF_MERCHANT) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Simple Animation<br>\n",
	       CF_SIMPLE, (ch_temp[cn].flags & CF_SIMPLE) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Alignment</td><td><input type=text name=alignment value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].alignment);

	printf("<tr><td colspan=2><table>\n");

	printf("<tr><td>Name</td><td>Start Value</td><td>Race Mod</td><td>Race Max</td><td>Difficulty</td></tr>\n");

	for (n = 0; n<5; n++)
	{
		printf("<tr><td>%s</td>\n", at_name[n]);
		printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, ch_temp[cn].attrib[n][0]);
		printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, ch_temp[cn].attrib[n][1]);
		printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, ch_temp[cn].attrib[n][2]);
		printf("<td><input type=text name=attrib%d_3 value=\"%d\" size=10 maxlength=10></td>\n", n, ch_temp[cn].attrib[n][3]);
		printf("</tr>\n");
	}

	printf("<tr><td>Hitpoints</td>\n");
	printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].hp[0]);
	printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].hp[1]);
	printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].hp[2]);
	printf("<td><input type=text name=hp_3 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].hp[3]);
	printf("</tr>\n");

	printf("<tr><td>Endurance</td>\n");
	printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].end[0]);
	printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].end[1]);
	printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].end[2]);
	printf("<td><input type=text name=end_3 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].end[3]);
	printf("</tr>\n");

	printf("<tr><td>Mana</td>\n");
	printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].mana[0]);
	printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].mana[1]);
	printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].mana[2]);
	printf("<td><input type=text name=mana_3 value=\"%d\" size=10 maxlength=10></td>\n", ch_temp[cn].mana[3]);
	printf("</tr>\n");

	for (n = 0; n<50; n++)
	{
		if (skilltab[n].name[0]==0)
		{
			continue;
		}
		printf("<tr><td>%s</td>\n", skilltab[n].name);
		printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, ch_temp[cn].skill[n][0]);
		printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, ch_temp[cn].skill[n][1]);
		printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, ch_temp[cn].skill[n][2]);
		printf("<td><input type=text name=skill%d_3 value=\"%d\" size=10 maxlength=10></td>\n", n, ch_temp[cn].skill[n][3]);
		printf("</tr>\n");
	}

	printf("</table></td></tr>\n");

	printf("<tr><td valign=top>Speed Mod</td><td><input type=text name=speed_mod value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].speed_mod);
	printf("<tr><td valign=top>Weapon Bonus</td><td><input type=text name=weapon_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].weapon_bonus);
	printf("<tr><td valign=top>Armor Bonus</td><td><input type=text name=armor_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].armor_bonus);
	printf("<tr><td valign=top>Gethit Bonus</td><td><input type=text name=gethit_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].gethit_bonus);
	printf("<tr><td valign=top>Light Bonus</td><td><input type=text name=light_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].light_bonus);

	printf("<tr><td>EXP left:</td><td><input type=text name=points value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].points);
	printf("<tr><td>EXP total:</td><td><input type=text name=points_tot value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].points_tot);

	printf("<tr><td>Position X:</td><td><input type=text name=x value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].x);
	printf("<tr><td>Position Y:</td><td><input type=text name=y value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].y);
	printf("<tr><td>Direction:</td><td><input type=text name=dir value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].dir);
	printf("<tr><td>Gold:</td><td><input type=text name=gold value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch_temp[cn].gold);

	if (ch_temp[cn].citem<0 || ch_temp[cn].citem>=MAXTITEM)
	{
		ch_temp[cn].citem = 0;
	}

	printf("<tr><td>Current Item</td><td><input type=text name=citem value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
	       ch_temp[cn].citem, ch_temp[cn].citem ? it_temp[ch_temp[cn].citem].name : "none");

	for (n = 0; n<12; n++)
	{
		if (ch_temp[cn].worn[n]>=MAXTITEM)
		{
			ch_temp[cn].worn[n] = 0;
		}
		printf("<tr><td>%s</td><td><input type=text name=worn%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
		       weartext[n], n, ch_temp[cn].worn[n], ch_temp[cn].worn[n] ? it_temp[ch_temp[cn].worn[n]].name : "none");
	}

	for (n = 0; n<40; n++)
	{
		if (ch_temp[cn].item[n]<0 || ch_temp[cn].item[n]>=MAXTITEM)
		{
			ch_temp[cn].item[n] = 0;
		}
		printf("<tr><td>Item %d</td><td><input type=text name=item%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
		       n, n, ch_temp[cn].item[n], ch_temp[cn].item[n] ? it_temp[ch_temp[cn].item[n]].name : "none");
	}

	printf("<tr><td>Driver Data:</td></tr>\n");
	for (n = 0; n<100; n++)
	{
		if (strcmp(data_name[n], "Unused")==0 || strcmp(data_name[n], "Reserved")==0)
		{
			continue;
		}
		printf("<tr><td>[%3d] %s</td><td><input type=text name=drdata%d value=\"%d\" size=35 maxlength=75></td></tr>\n",
		       n, data_name[n], n, ch_temp[cn].data[n]);
	}

	printf("<tr><td>Driver Texts:</td></tr>\n");
	for (n = 0; n<10; n++)
	{
		if (strcmp(text_name[n], "Unused")==0 || strcmp(text_name[n], "Reserved")==0)
		{
			continue;
		}
		printf("<tr><td>%s</td><td><input type=text name=text_%d value=\"%s\" size=35 maxlength=158></td></tr>\n",
		       text_name[n], n, ch_temp[cn].text[n]);
	}

	printf("<tr><td><input type=submit value=Update></td><td> </td></tr>\n");
	printf("</table>\n");
	printf("<input type=hidden name=step value=14>\n");
	printf("<input type=hidden name=cn value=%d>\n", cn);
	printf("</form>\n");
	printf("<center><a href=/cgi-imp/acct.cgi?step=11>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
}

void view_character_player(LIST *head)
{
	int cn, n;
	char *tmp;

	tmp = find_val(head, "cn");
	if (tmp)
	{
		cn = atoi(tmp);
	}
	else
	{
		printf("No number specified.\n");
		return;
	}

	if (cn<0 || cn>=MAXTCHARS)
	{
		printf("Number out of bounds.\n");
		return;
	}

	printf("<center><a href=/cgi-imp/acct.cgi?step=19>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<form method=post action=/cgi-imp/acct.cgi>\n");
	printf("<table>\n");
	printf("<tr><td>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>\n",
	       ch[cn].name);
	printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>\n",
	       ch[cn].reference);
	printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>\n",
	       ch[cn].description);
	printf("<tr><td>Player:</td><td><input type=text name=player value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].player);
	printf("<tr><td>Pass1:</td><td><input type=text name=pass1 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].pass1);
	printf("<tr><td>Pass2:</td><td><input type=text name=pass2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].pass2);

	printf("<tr><td valign=top>Kindred:</td><td>\n");
	printf("<input type=checkbox name=kindred value=%d %s>Mercenary<br>\n",
	       KIN_MERCENARY, (ch[cn].kindred & KIN_MERCENARY) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Sorcerer-Mercenary<br>\n\n",
	       KIN_SORCERER, (ch[cn].kindred & KIN_SORCERER) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Warrior-Mercenary<br>\n",
	       KIN_WARRIOR, (ch[cn].kindred & KIN_WARRIOR) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Templar<br>\n",
	       KIN_TEMPLAR, (ch[cn].kindred & KIN_TEMPLAR) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Arch-Templar<br>\n",
	       KIN_ARCHTEMPLAR, (ch[cn].kindred & KIN_ARCHTEMPLAR) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Harakim<br>\n",
	       KIN_HARAKIM, (ch[cn].kindred & KIN_HARAKIM) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Arch-Harakim<br>\n",
	       KIN_ARCHHARAKIM, (ch[cn].kindred & KIN_ARCHHARAKIM) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Seyan'Du<br>\n",
	       KIN_SEYAN_DU, (ch[cn].kindred & KIN_SEYAN_DU) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Purple<br>\n",
	       KIN_PURPLE, (ch[cn].kindred & KIN_PURPLE) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Monster<br>\n",
	       KIN_MONSTER, (ch[cn].kindred & KIN_MONSTER) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Male<br>\n",
	       KIN_MALE, (ch[cn].kindred & KIN_MALE) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Female<br>\n",
	       KIN_FEMALE, (ch[cn].kindred & KIN_FEMALE) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Sprite base:</td><td><input type=text name=sprite value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].sprite);

	printf("<tr><td valign=top>Sound base:</td><td><input type=text name=sound value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].sound);

	printf("<tr><td valign=top>Class:</td><td><input type=text name=class value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].class);

	printf("<tr><td valign=top>Flags:</td><td>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Player<br>\n", CF_PLAYER, (ch[cn].flags & CF_PLAYER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Staff<br>\n", CF_STAFF, (ch[cn].flags & CF_STAFF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Imp<br>\n", CF_IMP, (ch[cn].flags & CF_IMP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Creator<br>\n", CF_CREATOR, (ch[cn].flags & CF_CREATOR) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Build Mode<br>\n", CF_BUILDMODE, (ch[cn].flags & CF_BUILDMODE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>God<br>\n", CF_GOD, (ch[cn].flags & CF_GOD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Greater God<br>\n", CF_GREATERGOD, (ch[cn].flags & CF_GREATERGOD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Infrared<br>\n", CF_INFRARED, (ch[cn].flags & CF_INFRARED) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Immortal<br>\n", CF_IMMORTAL, (ch[cn].flags & CF_IMMORTAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Invisible<br>\n", CF_INVISIBLE, (ch[cn].flags & CF_INVISIBLE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Greater Invisibility<br>\n", CF_GREATERINV, (ch[cn].flags & CF_GREATERINV) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Safety Measures for Gods<br>\n", CF_SAFE, (ch[cn].flags & CF_SAFE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>New User<br>\n", CF_NEWUSER, (ch[cn].flags & CF_NEWUSER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Tell<br>\n", CF_NOTELL, (ch[cn].flags & CF_NOTELL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Shout<br>\n", CF_NOSHOUT, (ch[cn].flags & CF_NOSHOUT) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No HP Regen<br>\n", CF_NOHPREG, (ch[cn].flags & CF_NOHPREG) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No END Regen<br>\n", CF_NOENDREG, (ch[cn].flags & CF_NOENDREG) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No MANA Regen<br>\n", CF_NOMANAREG, (ch[cn].flags & CF_NOMANAREG) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Dead Body<br>\n", CF_BODY, (ch[cn].flags & CF_BODY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Stay Awake<br>\n", CF_NOSLEEP, (ch[cn].flags & CF_NOSLEEP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Magic<br>\n", CF_NOMAGIC, (ch[cn].flags & CF_NOMAGIC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Stoned<br>\n", CF_STONED, (ch[cn].flags & CF_STONED) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Usurped<br>\n", CF_USURP, (ch[cn].flags & CF_USURP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Shutup<br>\n", CF_SHUTUP, (ch[cn].flags & CF_SHUTUP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Description<br>\n", CF_NODESC, (ch[cn].flags & CF_NODESC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Profiler<br>\n", CF_PROF, (ch[cn].flags & CF_PROF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Simple Animation<br>\n", CF_SIMPLE, (ch[cn].flags & CF_SIMPLE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Kicked<br>\n", CF_KICKED, (ch[cn].flags & CF_KICKED) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Do Not List<br>\n", CF_NOLIST, (ch[cn].flags & CF_NOLIST) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Do Not List on Who<br>\n", CF_NOWHO, (ch[cn].flags & CF_NOWHO) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Spell Ignore<br>\n", CF_SPELLIGNORE, (ch[cn].flags & CF_SPELLIGNORE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Staff Tells<br>\n", CF_NOSTAFF, (ch[cn].flags & CF_NOSTAFF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Is POH<br>\n", CF_POH, (ch[cn].flags & CF_POH) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Is POH Leader<br>\n", CF_POH_LEADER, (ch[cn].flags & CF_POH_LEADER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Is Looting<br>\n", CF_ISLOOTING, (ch[cn].flags & CF_ISLOOTING) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Gold List<br>\n", CF_GOLDEN, (ch[cn].flags & CF_GOLDEN) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Black List<br>\n", CF_BLACK, (ch[cn].flags & CF_BLACK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Has Password<br>\n", CF_PASSWD, (ch[cn].flags & CF_PASSWD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Client Side Update Needed<br>\n", CF_UPDATE, (ch[cn].flags & CF_UPDATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Save Player To Disk<br>\n", CF_SAVEME, (ch[cn].flags & CF_SAVEME) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Alignment</td><td><input type=text name=alignment value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].alignment);

	printf("<tr><td colspan=2><table>\n");

	printf("<tr><td>Name</td><td>Start Value</td><td>Race Mod</td><td>Race Max</td><td>Difficulty</td></tr>\n");

	for (n = 0; n<5; n++)
	{
		printf("<tr><td>%s</td>\n", at_name[n]);
		printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].attrib[n][0]);
		printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].attrib[n][1]);
		printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].attrib[n][2]);
		printf("<td><input type=text name=attrib%d_3 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].attrib[n][3]);
		printf("</tr>\n");
	}

	printf("<tr><td>Hitpoints</td>\n");
	printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].hp[0]);
	printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].hp[1]);
	printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].hp[2]);
	printf("<td><input type=text name=hp_3 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].hp[3]);
	printf("</tr>\n");

	printf("<tr><td>Endurance</td>\n");
	printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].end[0]);
	printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].end[1]);
	printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].end[2]);
	printf("<td><input type=text name=end_3 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].end[3]);
	printf("</tr>\n");

	printf("<tr><td>Mana</td>\n");
	printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].mana[0]);
	printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].mana[1]);
	printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].mana[2]);
	printf("<td><input type=text name=mana_3 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].mana[3]);
	printf("</tr>\n");

	for (n = 0; n<50; n++)
	{
		if (skilltab[n].name[0]==0)
		{
			continue;
		}
		printf("<tr><td>%s</td>\n", skilltab[n].name);
		printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].skill[n][0]);
		printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].skill[n][1]);
		printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].skill[n][2]);
		printf("<td><input type=text name=skill%d_3 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].skill[n][3]);
		printf("</tr>\n");
	}

	printf("</table></td></tr>\n");

	printf("<tr><td valign=top>Speed Mod</td><td><input type=text name=speed_mod value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].speed_mod);
	printf("<tr><td valign=top>Weapon Bonus</td><td><input type=text name=weapon_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].weapon_bonus);
	printf("<tr><td valign=top>Armor Bonus</td><td><input type=text name=armor_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].armor_bonus);
	printf("<tr><td valign=top>Gethit Bonus</td><td><input type=text name=gethit_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].gethit_bonus);
	printf("<tr><td valign=top>Light Bonus</td><td><input type=text name=light_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].light_bonus);

	printf("<tr><td>EXP left:</td><td><input type=text name=points value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].points);
	printf("<tr><td>EXP total:</td><td><input type=text name=points_tot value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].points_tot);

	printf("<tr><td>Position X:</td><td><input type=text name=x value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].x);
	printf("<tr><td>Position Y:</td><td><input type=text name=y value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].y);
	printf("<tr><td>Direction:</td><td><input type=text name=dir value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].dir);
	printf("<tr><td>Temple X:</td><td><input type=text name=temple_x value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].temple_x);
	printf("<tr><td>Temple Y:</td><td><input type=text name=temple_y value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].temple_y);
	printf("<tr><td>Tavern X:</td><td><input type=text name=tavern_x value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].tavern_x);
	printf("<tr><td>Tavern Y:</td><td><input type=text name=tavern_y value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].tavern_y);
	printf("<tr><td>Temp:</td><td><input type=text name=temp value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].temp);

	printf("<tr><td>To X:</td><td><input type=text name=tox value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].tox);
	printf("<tr><td>To Y:</td><td><input type=text name=toy value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].toy);
	printf("<tr><td>From X:</td><td><input type=text name=frx value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].frx);
	printf("<tr><td>From Y:</td><td><input type=text name=fry value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].fry);
	printf("<tr><td>Status:</td><td><input type=text name=status value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].status);
	printf("<tr><td>Status2:</td><td><input type=text name=status2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].status2);


	printf("<tr><td>Gold:</td><td><input type=text name=gold value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       ch[cn].gold);

	if (ch[cn].citem<0 || ch[cn].citem>=MAXITEM)
	{
		ch[cn].citem = 0;
	}

	printf("<tr><td>Current Item</td><td><input type=text name=citem value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
	       ch[cn].citem, ch[cn].citem ? it[ch[cn].citem].name : "none");

	for (n = 0; n<12; n++)
	{
		if (ch[cn].worn[n]>=MAXITEM)
		{
			ch[cn].worn[n] = 0;
		}
		printf("<tr><td>%s</td><td><input type=text name=worn%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
		       weartext[n], n, ch[cn].worn[n], ch[cn].worn[n] ? it[ch[cn].worn[n]].name : "none");
	}

	for (n = 0; n<40; n++)
	{
		if (ch[cn].item[n]<0 || ch[cn].item[n]>=MAXITEM)
		{
			ch[cn].item[n] = 0;
		}
		printf("<tr><td>Item %d</td><td><input type=text name=item%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
		       n, n, ch[cn].item[n], ch[cn].item[n] ? it[ch[cn].item[n]].name : "none");
	}

	printf("<tr><td>Driver Data:</td></tr>\n");
	for (n = 0; n<100; n++)
	{
		if (strcmp(player_data_name[n], "Unused")==0 || strcmp(player_data_name[n], "Reserved")==0)
		{
			continue;
		}
		printf("<tr><td>[%3d] %s</td><td><input type=text name=drdata%d value=\"%d\" size=35 maxlength=75></td></tr>\n",
		       n, player_data_name[n], n, ch[cn].data[n]);
	}

	printf("<tr><td>Driver Texts:</td></tr>\n");
	for (n = 0; n<10; n++)
	{
		if (strcmp(player_text_name[n], "Unused")==0 || strcmp(player_text_name[n], "Reserved")==0)
		{
			continue;
		}
		printf("<tr><td>%s</td><td><input type=text name=text_%d value=\"%s\" size=35 maxlength=158></td></tr>\n",
		       player_text_name[n], n, ch[cn].text[n]);
	}

	printf("<tr><td><input type=submit value=Update></td><td> </td></tr>\n");
	printf("</table>\n");
	printf("<input type=hidden name=step value=26>\n");
	printf("<input type=hidden name=cn value=%d>\n", cn);
	printf("</form>\n");
	printf("<center><a href=/cgi-imp/acct.cgi?step=19>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
}



void copy_object(LIST *head)
{
	int in, n;
	char *tmp;

	tmp = find_val(head, "in");
	if (tmp)
	{
		in = atoi(tmp);
	}
	else
	{
		printf("No number specified.\n");
		return;
	}

	if (in<1 || in>=MAXTCHARS)
	{
		printf("Number out of bounds.\n");
		return;
	}

	for (n = 1; n<MAXTITEM; n++)
	{
		if (it_temp[n].used==USE_EMPTY)
		{
			break;
		}
	}
	if (n==MAXTITEM)
	{
		printf("MAXTITEM reached!\n");
		return;
	}
	it_temp[n] = it_temp[in];

	printf("Done.\n");
}

void view_object(LIST *head)
{
	int in, n;
	char *tmp;

	tmp = find_val(head, "in");
	if (tmp)
	{
		in = atoi(tmp);
	}
	else
	{
		printf("No number specified.\n");
		return;
	}

	if (in<1 || in>=MAXTITEM)
	{
		printf("Number out of bounds.\n");
		return;
	}
	printf("<center><a href=/cgi-imp/acct.cgi?step=21>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<form method=post action=/cgi-imp/acct.cgi>\n");
	printf("<table>\n");
	printf("<tr><td valign=top>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>\n",
	       it_temp[in].name);
	printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>\n",
	       it_temp[in].reference);
	printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>\n",
	       it_temp[in].description);

	printf("<tr><td valign=top>Flags:</td><td>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Moveblock<br>\n",
	       IF_MOVEBLOCK, (it_temp[in].flags & IF_MOVEBLOCK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Sightblock<br>\n",
	       IF_SIGHTBLOCK, (it_temp[in].flags & IF_SIGHTBLOCK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Take-Able<br>\n",
	       IF_TAKE, (it_temp[in].flags & IF_TAKE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Look-Able<br>\n",
	       IF_LOOK, (it_temp[in].flags & IF_LOOK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Look-Special<br>\n",
	       IF_LOOKSPECIAL, (it_temp[in].flags & IF_LOOKSPECIAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use<br>\n",
	       IF_USE, (it_temp[in].flags & IF_USE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Special<br>\n",
	       IF_USESPECIAL, (it_temp[in].flags & IF_USESPECIAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Destroy<br>\n",
	       IF_USEDESTROY, (it_temp[in].flags & IF_USEDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Activate<br>\n",
	       IF_USEACTIVATE, (it_temp[in].flags & IF_USEACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Deactivate<br>\n",
	       IF_USEDEACTIVATE, (it_temp[in].flags & IF_USEDEACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Re-Activate<br>\n",
	       IF_REACTIVATE, (it_temp[in].flags & IF_REACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Repair<br>\n",
	       IF_NOREPAIR, (it_temp[in].flags & IF_NOREPAIR) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Hidden (data[9])<br>\n",
	       IF_HIDDEN, (it_temp[in].flags & IF_HIDDEN) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Step-Action<br>\n",
	       IF_STEPACTION, (it_temp[in].flags & IF_STEPACTION) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Expire-Proc<br>\n",
	       IF_EXPIREPROC, (it_temp[in].flags & IF_EXPIREPROC) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Fast Light Age<br>\n",
	       IF_LIGHTAGE, (it_temp[in].flags & IF_LIGHTAGE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Unique<br>\n",
	       IF_UNIQUE, (it_temp[in].flags & IF_UNIQUE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Spell<br>\n",
	       IF_SPELL, (it_temp[in].flags & IF_SPELL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Shop-Destroy (quest items)<br>\n",
	       IF_SHOPDESTROY, (it_temp[in].flags & IF_SHOPDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Laby-Destroy (quest items)<br>\n",
	       IF_LABYDESTROY, (it_temp[in].flags & IF_LABYDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Depot (quest items)<br>\n",
	       IF_NODEPOT, (it_temp[in].flags & IF_NODEPOT) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Market (no price change)<br>\n",
	       IF_NOMARKET, (it_temp[in].flags & IF_NOMARKET) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Donate (cheap items)<br>\n",
	       IF_DONATE, (it_temp[in].flags & IF_DONATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Single-Age<br>\n",
	       IF_SINGLEAGE, (it_temp[in].flags & IF_SINGLEAGE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Always expire when inactive<br>\n",
	       IF_ALWAYSEXP1, (it_temp[in].flags & IF_ALWAYSEXP1) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Always expire when active<br>\n",
	       IF_ALWAYSEXP2, (it_temp[in].flags & IF_ALWAYSEXP2) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Armor<br>\n",
	       IF_ARMOR, (it_temp[in].flags & IF_ARMOR) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Magic<br>\n",
	       IF_MAGIC, (it_temp[in].flags & IF_MAGIC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Misc<br>\n",
	       IF_MISC, (it_temp[in].flags & IF_MISC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Sword<br>\n",
	       IF_WP_SWORD, (it_temp[in].flags & IF_WP_SWORD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Dagger<br>\n",
	       IF_WP_DAGGER, (it_temp[in].flags & IF_WP_DAGGER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Axe<br>\n",
	       IF_WP_AXE, (it_temp[in].flags & IF_WP_AXE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Club<br>\n",
	       IF_WP_STAFF, (it_temp[in].flags & IF_WP_STAFF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Two-Handed<br>\n",
	       IF_WP_TWOHAND, (it_temp[in].flags & IF_WP_TWOHAND) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td>Value:</td><td><input type=text name=value value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].value);

	printf("<tr><td valign=top>Placement:</td><td>\n");
	printf("<input type=checkbox name=placement value=%d %s>Head<br>\n",
	       PL_HEAD, (it_temp[in].placement & PL_HEAD) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Neck<br>\n",
	       PL_NECK, (it_temp[in].placement & PL_NECK) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Body<br>\n",
	       PL_BODY, (it_temp[in].placement & PL_BODY) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Arms<br>\n",
	       PL_ARMS, (it_temp[in].placement & PL_ARMS) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Belt<br>\n",
	       PL_BELT, (it_temp[in].placement & PL_BELT) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Legs<br>\n",
	       PL_LEGS, (it_temp[in].placement & PL_LEGS) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Feet<br>\n",
	       PL_FEET, (it_temp[in].placement & PL_FEET) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Left Hand<br>\n",
	       PL_SHIELD, (it_temp[in].placement & PL_SHIELD) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Right Hand<br>\n",
	       PL_WEAPON, (it_temp[in].placement & PL_WEAPON) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Cloak<br>\n",
	       PL_CLOAK, (it_temp[in].placement & PL_CLOAK) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Two-Handed<br>\n",
	       PL_TWOHAND, (it_temp[in].placement & PL_TWOHAND) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Ring<br>\n",
	       PL_RING, (it_temp[in].placement & PL_RING) ? "checked" : "");

	printf("</td></tr>\n");


	printf("<tr><td colspan=2><table>\n");

	printf("<tr><td>Name</td><td>Inactive Mod</td><td>Active Mod</td><td>Min to wear</td><td>\n");

	for (n = 0; n<5; n++)
	{
		printf("<tr><td>%s</td>\n", at_name[n]);
		printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].attrib[n][0]);
		printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].attrib[n][1]);
		printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].attrib[n][2]);
		printf("</tr>\n");
	}

	printf("<tr><td>Hitpoints</td>\n");
	printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].hp[0]);
	printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].hp[1]);
	printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].hp[2]);
	printf("</tr>\n");

	printf("<tr><td>Endurance</td>\n");
	printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].end[0]);
	printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].end[1]);
	printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].end[2]);
	printf("</tr>\n");

	printf("<tr><td>Mana</td>\n");
	printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].mana[0]);
	printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].mana[1]);
	printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].mana[2]);
	printf("</tr>\n");

	for (n = 0; n<50; n++)
	{
		if (skilltab[n].name[0]==0)
		{
			continue;
		}
		printf("<tr><td>%s</td>\n", skilltab[n].name);
		printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].skill[n][0]);
		printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].skill[n][1]);
		printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].skill[n][2]);
		printf("</tr>\n");
	}

	printf("<tr><td>Armor:</td><td><input type=text name=armor_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=armor_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].armor[0], it_temp[in].armor[1]);
	printf("<tr><td>Weapon:</td><td><input type=text name=weapon_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=weapon_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].weapon[0], it_temp[in].weapon[1]);

	printf("<tr><td>Gethit Damage:</td><td><input type=text name=gethit_dam_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=gethit_dam_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].gethit_dam[0], it_temp[in].gethit_dam[1]);

	printf("<tr><td>Max Age:</td><td><input type=text name=max_age_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=max_age_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].max_age[0], it_temp[in].max_age[1]);

	printf("<tr><td>Light</td><td><input type=text name=light_1 value=\"%d\" size=10 maxlength=10></td>\n",
	       it_temp[in].light[0]);
	printf("<td><input type=text name=light_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].light[1]);

	printf("<tr><td valign=top>Sprite:</td><td><input type=text name=sprite_1 value=\"%d\" size=10 maxlength=10></td>\n",
	       it_temp[in].sprite[0]);
	printf("<td><input type=text name=sprite_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].sprite[1]);

	printf("<tr><td>Animation-Status:</td><td><input type=text name=status_1 value=\"%d\" size=10 maxlength=10></td>\n",
	       it_temp[in].status[0]);
	printf("<td><input type=text name=status_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].status[1]);

	printf("</table></td></tr>\n");

	printf("<tr><td>Max Damage:</td><td><input type=text name=max_damage value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].max_damage);

	printf("<tr><td>Duration:</td><td><input type=text name=duration value=\"%d\" size=10 maxlength=10> (in 1/18 of a sec)</td></tr>\n",
	       it_temp[in].duration);
	printf("<tr><td>Cost:</td><td><input type=text name=cost value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].cost);
	printf("<tr><td>Power:</td><td><input type=text name=power value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].power);
	printf("<tr><td>Sprite Overr.:</td><td><input type=text name=spr_ovr value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].sprite_override);

	printf("<tr><td>Min. Rank:</td><td><input type=text name=min_rank value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it_temp[in].min_rank);

	printf("<tr><td valign=top>Driver:</td><td>\n");
	printf("<input type=text name=driver value=%d>", it_temp[in].driver);
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Driver Data:</td><td>\n");
	printf("The way the data is used depends on the driver type!<br>\n");

	for (n = 0; n<10; n++)
	{
		printf("<tr><td>%s:</td><td><input type=text name=drdata%d value=\"%d\" size=10 maxlength=10></td></tr>\n",
		       obj_drdata[n], n, it_temp[in].data[n]);
	}

	printf("<tr><td><input type=submit value=Update></td><td> </td></tr>\n");
	printf("</table>\n");
	printf("<input type=hidden name=step value=24>\n");
	printf("<input type=hidden name=in value=%d>", in);
	printf("</form>\n");
	printf("<center><a href=/cgi-imp/acct.cgi?step=21>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");

}

void view_item(LIST *head)
{
	int in, n;
	char *tmp;

	tmp = find_val(head, "in");
	if (tmp)
	{
		in = atoi(tmp);
	}
	else
	{
		printf("No number specified.\n");
		return;
	}

	if (in<1 || in>=MAXITEM)
	{
		printf("Number out of bounds.\n");
		return;
	}
	printf("<center><a href=/cgi-imp/acct.cgi?step=27>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<form method=post action=/cgi-imp/acct.cgi>\n");
	printf("<table>\n");
	printf("<tr><td valign=top>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>\n",
	       it[in].name);
	printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>\n",
	       it[in].reference);
	printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>\n",
	       it[in].description);
	printf("<tr><td>Carried By:</td><td><input type=text name=carried value=\"%d\" size=10 maxlength=10>(%s)</td></tr>\n",
	       it[in].carried, ch[it[in].carried].name);
	printf("<tr><td>X:</td><td><input type=text name=x value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].x);
	printf("<tr><td>Y:</td><td><input type=text name=y value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].y);

	printf("<tr><td valign=top>Flags:</td><td>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Moveblock<br>\n",
	       IF_MOVEBLOCK, (it[in].flags & IF_MOVEBLOCK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Sightblock<br>\n",
	       IF_SIGHTBLOCK, (it[in].flags & IF_SIGHTBLOCK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Take-Able<br>\n",
	       IF_TAKE, (it[in].flags & IF_TAKE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Look-Able<br>\n",
	       IF_LOOK, (it[in].flags & IF_LOOK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Look-Special<br>\n",
	       IF_LOOKSPECIAL, (it[in].flags & IF_LOOKSPECIAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use<br>\n",
	       IF_USE, (it[in].flags & IF_USE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Special<br>\n",
	       IF_USESPECIAL, (it[in].flags & IF_USESPECIAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Destroy<br>\n",
	       IF_USEDESTROY, (it[in].flags & IF_USEDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Activate<br>\n",
	       IF_USEACTIVATE, (it[in].flags & IF_USEACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Deactivate<br>\n",
	       IF_USEDEACTIVATE, (it[in].flags & IF_USEDEACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Re-Activate<br>\n",
	       IF_REACTIVATE, (it[in].flags & IF_REACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Repair<br>\n",
	       IF_NOREPAIR, (it[in].flags & IF_NOREPAIR) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Hidden (data[9])<br>\n",
	       IF_HIDDEN, (it[in].flags & IF_HIDDEN) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Step-Action<br>\n",
	       IF_STEPACTION, (it[in].flags & IF_STEPACTION) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Expire-Proc<br>\n",
	       IF_EXPIREPROC, (it[in].flags & IF_EXPIREPROC) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Fast Light Age<br>\n",
	       IF_LIGHTAGE, (it[in].flags & IF_LIGHTAGE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Unique<br>\n",
	       IF_UNIQUE, (it[in].flags & IF_UNIQUE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Spell<br>\n",
	       IF_SPELL, (it[in].flags & IF_SPELL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Shop-Destroy (quest items)<br>\n",
	       IF_SHOPDESTROY, (it[in].flags & IF_SHOPDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Laby-Destroy (quest items)<br>\n",
	       IF_LABYDESTROY, (it[in].flags & IF_LABYDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Depot (quest items)<br>\n",
	       IF_NODEPOT, (it[in].flags & IF_NODEPOT) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Market (no price change)<br>\n",
	       IF_NOMARKET, (it[in].flags & IF_NOMARKET) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Donate (cheap items)<br>\n",
	       IF_DONATE, (it[in].flags & IF_DONATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Single-Age<br>\n",
	       IF_SINGLEAGE, (it[in].flags & IF_SINGLEAGE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Always expire when inactive<br>\n",
	       IF_ALWAYSEXP1, (it[in].flags & IF_ALWAYSEXP1) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Always expire when active<br>\n",
	       IF_ALWAYSEXP2, (it[in].flags & IF_ALWAYSEXP2) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Armor<br>\n",
	       IF_ARMOR, (it[in].flags & IF_ARMOR) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Magic<br>\n",
	       IF_MAGIC, (it[in].flags & IF_MAGIC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Misc<br>\n",
	       IF_MISC, (it[in].flags & IF_MISC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Sword<br>\n",
	       IF_WP_SWORD, (it[in].flags & IF_WP_SWORD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Dagger<br>\n",
	       IF_WP_DAGGER, (it[in].flags & IF_WP_DAGGER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Axe<br>\n",
	       IF_WP_AXE, (it[in].flags & IF_WP_AXE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Club<br>\n",
	       IF_WP_STAFF, (it[in].flags & IF_WP_STAFF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Two-Handed<br>\n",
	       IF_WP_TWOHAND, (it[in].flags & IF_WP_TWOHAND) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td>Value:</td><td><input type=text name=value value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].value);

	printf("<tr><td valign=top>Placement:</td><td>\n");
	printf("<input type=checkbox name=placement value=%d %s>Head<br>\n",
	       PL_HEAD, (it[in].placement & PL_HEAD) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Neck<br>\n",
	       PL_NECK, (it[in].placement & PL_NECK) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Body<br>\n",
	       PL_BODY, (it[in].placement & PL_BODY) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Arms<br>\n",
	       PL_ARMS, (it[in].placement & PL_ARMS) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Belt<br>\n",
	       PL_BELT, (it[in].placement & PL_BELT) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Legs<br>\n",
	       PL_LEGS, (it[in].placement & PL_LEGS) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Feet<br>\n",
	       PL_FEET, (it[in].placement & PL_FEET) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Left Hand<br>\n",
	       PL_SHIELD, (it[in].placement & PL_SHIELD) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Right Hand<br>\n",
	       PL_WEAPON, (it[in].placement & PL_WEAPON) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Cloak<br>\n",
	       PL_CLOAK, (it[in].placement & PL_CLOAK) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Two-Handed<br>\n",
	       PL_TWOHAND, (it[in].placement & PL_TWOHAND) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Ring<br>\n",
	       PL_RING, (it[in].placement & PL_RING) ? "checked" : "");

	printf("</td></tr>\n");


	printf("<tr><td colspan=2><table>\n");

	printf("<tr><td>Name</td><td>Inactive Mod</td><td>Active Mod</td><td>Min to wear</td><td>\n");

	for (n = 0; n<5; n++)
	{
		printf("<tr><td>%s</td>\n", at_name[n]);
		printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].attrib[n][0]);
		printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].attrib[n][1]);
		printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].attrib[n][2]);
		printf("</tr>\n");
	}

	printf("<tr><td>Hitpoints</td>\n");
	printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>\n", it[in].hp[0]);
	printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>\n", it[in].hp[1]);
	printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>\n", it[in].hp[2]);
	printf("</tr>\n");

	printf("<tr><td>Endurance</td>\n");
	printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>\n", it[in].end[0]);
	printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>\n", it[in].end[1]);
	printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>\n", it[in].end[2]);
	printf("</tr>\n");

	printf("<tr><td>Mana</td>\n");
	printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>\n", it[in].mana[0]);
	printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>\n", it[in].mana[1]);
	printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>\n", it[in].mana[2]);
	printf("</tr>\n");

	for (n = 0; n<50; n++)
	{
		if (skilltab[n].name[0]==0)
		{
			continue;
		}
		printf("<tr><td>%s</td>\n", skilltab[n].name);
		printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].skill[n][0]);
		printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].skill[n][1]);
		printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].skill[n][2]);
		printf("</tr>\n");
	}

	printf("<tr><td>Armor:</td><td><input type=text name=armor_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=armor_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].armor[0], it[in].armor[1]);
	printf("<tr><td>Weapon:</td><td><input type=text name=weapon_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=weapon_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].weapon[0], it[in].weapon[1]);

	printf("<tr><td>Gethit Damage:</td><td><input type=text name=gethit_dam_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=gethit_dam_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].gethit_dam[0], it[in].gethit_dam[1]);

	printf("<tr><td>Max Age:</td><td><input type=text name=max_age_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=max_age_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].max_age[0], it[in].max_age[1]);

	printf("<tr><td>Light</td><td><input type=text name=light_1 value=\"%d\" size=10 maxlength=10></td>\n",
	       it[in].light[0]);
	printf("<td><input type=text name=light_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].light[1]);

	printf("<tr><td valign=top>Sprite:</td><td><input type=text name=sprite_1 value=\"%d\" size=10 maxlength=10></td>\n",
	       it[in].sprite[0]);
	printf("<td><input type=text name=sprite_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].sprite[1]);

	printf("<tr><td>Animation-Status:</td><td><input type=text name=status_1 value=\"%d\" size=10 maxlength=10></td>\n",
	       it[in].status[0]);
	printf("<td><input type=text name=status_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].status[1]);

	printf("</table></td></tr>\n");

	printf("<tr><td>Max Damage:</td><td><input type=text name=max_damage value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].max_damage);

	printf("<tr><td>Duration:</td><td><input type=text name=duration value=\"%d\" size=10 maxlength=10> (in 1/18 of a sec)</td></tr>\n",
	       it[in].duration);
	printf("<tr><td>Cost:</td><td><input type=text name=cost value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].cost);
	printf("<tr><td>Power:</td><td><input type=text name=power value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].power);
	printf("<tr><td>Sprite Overr.:</td><td><input type=text name=spr_ovr value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].sprite_override);

	printf("<tr><td>Min. Rank:</td><td><input type=text name=min_rank value=\"%d\" size=10 maxlength=10></td></tr>\n",
	       it[in].min_rank);

	printf("<tr><td valign=top>Driver:</td><td>\n");
	printf("<input type=text name=driver value=%d>", it[in].driver);
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Driver Data:</td><td>\n");
	printf("The way the data is used depends on the driver type!<br>\n");

	for (n = 0; n<10; n++)
	{
		printf("<tr><td>%s:</td><td><input type=text name=drdata%d value=\"%d\" size=10 maxlength=10></td></tr>\n",
		       obj_drdata[n], n, it[in].data[n]);
	}

	printf("<tr><td><input type=submit value=Update></td><td> </td></tr>\n");
	printf("</table>\n");
	printf("<input type=hidden name=step value=24>\n");
	printf("<input type=hidden name=in value=%d>", in);
	printf("</form>\n");
	printf("<center><a href=/cgi-imp/acct.cgi?step=27>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");

}

void delete_character_template(LIST *head)
{
	int cn;
	char *tmp;

	tmp = find_val(head, "cn");
	if (tmp)
	{
		cn = atoi(tmp);
	}
	else
	{
		printf("CN not specified.\n");
		return;
	}

	ch_temp[cn].used = USE_EMPTY;

	if (ch_temp[cn].flags & CF_RESPAWN)
	{
		globs->reset_char = cn;
	}

	printf("Done.\n");
}

void delete_object(LIST *head)
{
	int in;
	char *tmp;

	tmp = find_val(head, "in");
	if (tmp)
	{
		in = atoi(tmp);
	}
	else
	{
		printf("IN not specified.\n");
		return;
	}

	it_temp[in].used = USE_EMPTY;

	printf("Done.\n");
}

void update_character_player(LIST *head)
{
	int cn, cnt, val, n;
	unsigned long long lval;
	char *tmp, **tmps, buf[80];

	tmp = find_val(head, "cn");
	if (tmp)
	{
		cn = atoi(tmp);
	}
	else
	{
		printf("CN not specified.\n");
		return;
	}

	bzero(&ch[cn], sizeof(struct character));
	ch[cn].used = USE_ACTIVE;

	tmp = find_val(head, "name");
	if (tmp)
	{
		strncpy(ch[cn].name, tmp, 35);
		ch[cn].name[35] = 0;
	}
	else
	{
		printf("NAME not specified.\n");
		return;
	}

	tmp = find_val(head, "reference");
	if (tmp)
	{
		strncpy(ch[cn].reference, tmp, 35);
		ch[cn].reference[35] = 0;
	}
	else
	{
		printf("REFERENCE not specified.\n");
		return;
	}

	tmp = find_val(head, "description");
	if (tmp)
	{
		strncpy(ch[cn].description, tmp, 195);
		ch[cn].description[195] = 0;
	}
	else
	{
		printf("DESCRIPTION not specified.\n");
		return;
	}

	tmp = find_val(head, "player");
	if (tmp)
	{
		ch[cn].player = atoi(tmp);
	}
	else
	{
		printf("PLAYER not specified.\n");
		return;
	}


	tmp = find_val(head, "pass1");
	if (tmp)
	{
		ch[cn].pass1 = atoi(tmp);
	}
	else
	{
		printf("PASS1 not specified.\n");
		return;
	}

	tmp = find_val(head, "pass2");
	if (tmp)
	{
		ch[cn].pass2 = atoi(tmp);
	}
	else
	{
		printf("PASS2 not specified.\n");
		return;
	}

	cnt = find_val_multi(head, "kindred", &tmps);
	if (cnt)
	{
		for (n = val = 0; n<cnt; n++)
		{
			val |= atoi(tmps[n]);
		}
		ch[cn].kindred = val;
	}

	tmp = find_val(head, "sprite");
	if (tmp)
	{
		ch[cn].sprite = atoi(tmp);
	}
	else
	{
		printf("SPRITE not specified.\n");
		return;
	}

	tmp = find_val(head, "sound");
	if (tmp)
	{
		ch[cn].sound = atoi(tmp);
	}
	else
	{
		printf("SOUND not specified.\n");
		return;
	}

	tmp = find_val(head, "class");
	if (tmp)
	{
		ch[cn].class = atoi(tmp);
	}
	else
	{
		printf("CLASS not specified.\n");
		return;
	}

	cnt  = find_val_multi(head, "flags", &tmps);
	lval = 0;
	if (cnt)
	{
		for (n = 0; n<cnt; n++)
		{
			lval |= atoll(tmps[n]);
		}
	}
	ch[cn].flags = lval;

	tmp = find_val(head, "alignment");
	if (tmp)
	{
		ch[cn].alignment = atoi(tmp);
	}
	else
	{
		printf("ALIGNMENT not specified.\n");
		return;
	}

	for (n = 0; n<100; n++)
	{
		sprintf(buf, "drdata%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].data[n] = atoi(tmp);
		}
		else
		{
			ch[cn].data[n] = 0;
		}
	}

	for (n = 0; n<10; n++)
	{
		sprintf(buf, "text_%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			strncpy(ch[cn].text[n], tmp, 158);
		}
		else
		{
			ch[cn].text[n][0] = 0;
		}
		ch[cn].text[n][159] = 0;
	}

	for (n = 0; n<5; n++)
	{
		sprintf(buf, "attrib%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].attrib[n][0] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_0 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].attrib[n][1] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_1 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].attrib[n][2] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_2 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_3", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].attrib[n][3] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_3 not specified.", n);
			return;
		}
	}

	tmp = find_val(head, "hp_0");
	if (tmp)
	{
		ch[cn].hp[0] = atoi(tmp);
	}
	else
	{
		printf("HP_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_1");
	if (tmp)
	{
		ch[cn].hp[1] = atoi(tmp);
	}
	else
	{
		printf("HP_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_2");
	if (tmp)
	{
		ch[cn].hp[2] = atoi(tmp);
	}
	else
	{
		printf("HP_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_3");
	if (tmp)
	{
		ch[cn].hp[3] = atoi(tmp);
	}
	else
	{
		printf("HP_3 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_0");
	if (tmp)
	{
		ch[cn].end[0] = atoi(tmp);
	}
	else
	{
		printf("END_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_1");
	if (tmp)
	{
		ch[cn].end[1] = atoi(tmp);
	}
	else
	{
		printf("END_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_2");
	if (tmp)
	{
		ch[cn].end[2] = atoi(tmp);
	}
	else
	{
		printf("END_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_3");
	if (tmp)
	{
		ch[cn].end[3] = atoi(tmp);
	}
	else
	{
		printf("END_3 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_0");
	if (tmp)
	{
		ch[cn].mana[0] = atoi(tmp);
	}
	else
	{
		printf("MANA_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_1");
	if (tmp)
	{
		ch[cn].mana[1] = atoi(tmp);
	}
	else
	{
		printf("MANA_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_2");
	if (tmp)
	{
		ch[cn].mana[2] = atoi(tmp);
	}
	else
	{
		printf("MANA_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_3");
	if (tmp)
	{
		ch[cn].mana[3] = atoi(tmp);
	}
	else
	{
		printf("MANA_3 not specified.\n");
		return;
	}


	for (n = 0; n<50; n++)
	{
		sprintf(buf, "skill%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].skill[n][0] = atoi(tmp);
		}
		else
		{
			ch[cn].skill[n][0] = 0;
		}

		sprintf(buf, "skill%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].skill[n][1] = atoi(tmp);
		}
		else
		{
			ch[cn].skill[n][1] = 0;
		}

		sprintf(buf, "skill%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].skill[n][2] = atoi(tmp);
		}
		else
		{
			ch[cn].skill[n][2] = 0;
		}

		sprintf(buf, "skill%d_3", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].skill[n][3] = atoi(tmp);
		}
		else
		{
			ch[cn].skill[n][3] = 0;
		}
	}

	tmp = find_val(head, "speed_mod");
	if (tmp)
	{
		ch[cn].speed_mod = atoi(tmp);
	}
	else
	{
		printf("SPEED_MOD not specified.\n");
		return;
	}

	tmp = find_val(head, "weapon_bonus");
	if (tmp)
	{
		ch[cn].weapon_bonus = atoi(tmp);
	}
	else
	{
		printf("WEAPON_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "light_bonus");
	if (tmp)
	{
		ch[cn].light_bonus = atoi(tmp);
	}
	else
	{
		printf("LIGHT_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "armor_bonus");
	if (tmp)
	{
		ch[cn].armor_bonus = atoi(tmp);
	}
	else
	{
		printf("ARMOR_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "points");
	if (tmp)
	{
		ch[cn].points = atoi(tmp);
	}
	else
	{
		printf("POINTS not specified.\n");
		return;
	}

	tmp = find_val(head, "points_tot");
	if (tmp)
	{
		ch[cn].points_tot = atoi(tmp);
	}
	else
	{
		printf("POINTS_TOT not specified.\n");
		return;
	}

	tmp = find_val(head, "x");
	if (tmp)
	{
		ch[cn].x = atoi(tmp);
	}
	else
	{
		printf("X not specified.\n");
		return;
	}

	tmp = find_val(head, "y");
	if (tmp)
	{
		ch[cn].y = atoi(tmp);
	}
	else
	{
		printf("Y not specified.\n");
		return;
	}

	tmp = find_val(head, "dir");
	if (tmp)
	{
		ch[cn].dir = atoi(tmp);
	}
	else
	{
		printf("DIR not specified.\n");
		return;
	}


	tmp = find_val(head, "temple_x");
	if (tmp)
	{
		ch[cn].temple_x = atoi(tmp);
	}
	else
	{
		printf("temple_x not specified.\n");
		return;
	}

	tmp = find_val(head, "temple_y");
	if (tmp)
	{
		ch[cn].temple_y = atoi(tmp);
	}
	else
	{
		printf("temple_y not specified.\n");
		return;
	}

	tmp = find_val(head, "tavern_x");
	if (tmp)
	{
		ch[cn].tavern_x = atoi(tmp);
	}
	else
	{
		printf("tavern_x not specified.\n");
		return;
	}

	tmp = find_val(head, "tavern_y");
	if (tmp)
	{
		ch[cn].tavern_y = atoi(tmp);
	}
	else
	{
		printf("tavern_y not specified.\n");
		return;
	}

	tmp = find_val(head, "temp");
	if (tmp)
	{
		ch[cn].temp = atoi(tmp);
	}
	else
	{
		printf("temp not specified.\n");
		return;
	}


	tmp = find_val(head, "tox");
	if (tmp)
	{
		ch[cn].tox = atoi(tmp);
	}
	else
	{
		printf("tox not specified.\n");
		return;
	}
	tmp = find_val(head, "toy");
	if (tmp)
	{
		ch[cn].toy = atoi(tmp);
	}
	else
	{
		printf("toy not specified.\n");
		return;
	}
	tmp = find_val(head, "frx");
	if (tmp)
	{
		ch[cn].frx = atoi(tmp);
	}
	else
	{
		printf("frx not specified.\n");
		return;
	}
	tmp = find_val(head, "fry");
	if (tmp)
	{
		ch[cn].fry = atoi(tmp);
	}
	else
	{
		printf("fry not specified.\n");
		return;
	}
	tmp = find_val(head, "status");
	if (tmp)
	{
		ch[cn].status = atoi(tmp);
	}
	else
	{
		printf("status not specified.\n");
		return;
	}
	tmp = find_val(head, "status2");
	if (tmp)
	{
		ch[cn].status2 = atoi(tmp);
	}
	else
	{
		printf("status2 not specified.\n");
		return;
	}


	tmp = find_val(head, "gold");
	if (tmp)
	{
		ch[cn].gold = atoi(tmp);
	}
	else
	{
		printf("GOLD not specified.\n");
		return;
	}

	for (n = 0; n<20; n++)
	{
		sprintf(buf, "worn%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].worn[n] = atoi(tmp);
		}
	}

	for (n = 0; n<40; n++)
	{
		sprintf(buf, "item%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].item[n] = atoi(tmp);
		}
	}

	tmp = find_val(head, "citem");
	if (tmp)
	{
		ch[cn].citem = atoi(tmp);
	}
	else
	{
		printf("CITEM not specified.\n");
		return;
	}

	printf("Done.\n");
}

void update_character_template(LIST *head)
{
	int cn, cnt, val, n;
	unsigned long long lval;
	char *tmp, **tmps, buf[80];

	tmp = find_val(head, "cn");
	if (tmp)
	{
		cn = atoi(tmp);
	}
	else
	{
		printf("CN not specified.\n");
		return;
	}

	bzero(&ch_temp[cn], sizeof(struct character));
	ch_temp[cn].used = USE_ACTIVE;

	tmp = find_val(head, "name");
	if (tmp)
	{
		strncpy(ch_temp[cn].name, tmp, 35);
		ch_temp[cn].name[35] = 0;
	}
	else
	{
		printf("NAME not specified.\n");
		return;
	}

	tmp = find_val(head, "reference");
	if (tmp)
	{
		strncpy(ch_temp[cn].reference, tmp, 35);
		ch_temp[cn].reference[35] = 0;
	}
	else
	{
		printf("REFERENCE not specified.\n");
		return;
	}

	tmp = find_val(head, "description");
	if (tmp)
	{
		strncpy(ch_temp[cn].description, tmp, 195);
		ch_temp[cn].description[195] = 0;
	}
	else
	{
		printf("DESCRIPTION not specified.\n");
		return;
	}

	cnt = find_val_multi(head, "kindred", &tmps);
	if (cnt)
	{
		for (n = val = 0; n<cnt; n++)
		{
			val |= atoi(tmps[n]);
		}
		ch_temp[cn].kindred = val;
	}

	tmp = find_val(head, "sprite");
	if (tmp)
	{
		ch_temp[cn].sprite = atoi(tmp);
	}
	else
	{
		printf("SPRITE not specified.\n");
		return;
	}

	tmp = find_val(head, "sound");
	if (tmp)
	{
		ch_temp[cn].sound = atoi(tmp);
	}
	else
	{
		printf("SOUND not specified.\n");
		return;
	}

	tmp = find_val(head, "class");
	if (tmp)
	{
		ch_temp[cn].class = atoi(tmp);
	}
	else
	{
		printf("CLASS not specified.\n");
		return;
	}

	cnt  = find_val_multi(head, "flags", &tmps);
	lval = 0;
	if (cnt)
	{
		for (n = 0; n<cnt; n++)
		{
			lval |= atoll(tmps[n]);
		}
	}
	ch_temp[cn].flags = lval;

	tmp = find_val(head, "alignment");
	if (tmp)
	{
		ch_temp[cn].alignment = atoi(tmp);
	}
	else
	{
		printf("ALIGNMENT not specified.\n");
		return;
	}

	for (n = 0; n<100; n++)
	{
		sprintf(buf, "drdata%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].data[n] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].data[n] = 0;
		}
	}

	for (n = 0; n<10; n++)
	{
		sprintf(buf, "text_%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			strncpy(ch_temp[cn].text[n], tmp, 158);
		}
		else
		{
			ch_temp[cn].text[n][0] = 0;
		}
		ch_temp[cn].text[n][159] = 0;
	}

	for (n = 0; n<5; n++)
	{
		sprintf(buf, "attrib%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].attrib[n][0] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_0 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].attrib[n][1] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_1 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].attrib[n][2] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_2 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_3", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].attrib[n][3] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_3 not specified.", n);
			return;
		}
	}

	tmp = find_val(head, "hp_0");
	if (tmp)
	{
		ch_temp[cn].hp[0] = atoi(tmp);
	}
	else
	{
		printf("HP_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_1");
	if (tmp)
	{
		ch_temp[cn].hp[1] = atoi(tmp);
	}
	else
	{
		printf("HP_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_2");
	if (tmp)
	{
		ch_temp[cn].hp[2] = atoi(tmp);
	}
	else
	{
		printf("HP_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_3");
	if (tmp)
	{
		ch_temp[cn].hp[3] = atoi(tmp);
	}
	else
	{
		printf("HP_3 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_0");
	if (tmp)
	{
		ch_temp[cn].end[0] = atoi(tmp);
	}
	else
	{
		printf("END_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_1");
	if (tmp)
	{
		ch_temp[cn].end[1] = atoi(tmp);
	}
	else
	{
		printf("END_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_2");
	if (tmp)
	{
		ch_temp[cn].end[2] = atoi(tmp);
	}
	else
	{
		printf("END_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_3");
	if (tmp)
	{
		ch_temp[cn].end[3] = atoi(tmp);
	}
	else
	{
		printf("END_3 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_0");
	if (tmp)
	{
		ch_temp[cn].mana[0] = atoi(tmp);
	}
	else
	{
		printf("MANA_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_1");
	if (tmp)
	{
		ch_temp[cn].mana[1] = atoi(tmp);
	}
	else
	{
		printf("MANA_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_2");
	if (tmp)
	{
		ch_temp[cn].mana[2] = atoi(tmp);
	}
	else
	{
		printf("MANA_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_3");
	if (tmp)
	{
		ch_temp[cn].mana[3] = atoi(tmp);
	}
	else
	{
		printf("MANA_3 not specified.\n");
		return;
	}


	for (n = 0; n<50; n++)
	{
		sprintf(buf, "skill%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].skill[n][0] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].skill[n][0] = 0;
		}

		sprintf(buf, "skill%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].skill[n][1] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].skill[n][1] = 0;
		}

		sprintf(buf, "skill%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].skill[n][2] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].skill[n][2] = 0;
		}

		sprintf(buf, "skill%d_3", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].skill[n][3] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].skill[n][3] = 0;
		}
	}

	tmp = find_val(head, "speed_mod");
	if (tmp)
	{
		ch_temp[cn].speed_mod = atoi(tmp);
	}
	else
	{
		printf("SPEED_MOD not specified.\n");
		return;
	}

	tmp = find_val(head, "weapon_bonus");
	if (tmp)
	{
		ch_temp[cn].weapon_bonus = atoi(tmp);
	}
	else
	{
		printf("WEAPON_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "light_bonus");
	if (tmp)
	{
		ch_temp[cn].light_bonus = atoi(tmp);
	}
	else
	{
		printf("LIGHT_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "armor_bonus");
	if (tmp)
	{
		ch_temp[cn].armor_bonus = atoi(tmp);
	}
	else
	{
		printf("ARMOR_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "points");
	if (tmp)
	{
		ch_temp[cn].points = atoi(tmp);
	}
	else
	{
		printf("POINTS not specified.\n");
		return;
	}

	tmp = find_val(head, "points_tot");
	if (tmp)
	{
		ch_temp[cn].points_tot = atoi(tmp);
	}
	else
	{
		printf("POINTS_TOT not specified.\n");
		return;
	}

	tmp = find_val(head, "x");
	if (tmp)
	{
		ch_temp[cn].x = atoi(tmp);
	}
	else
	{
		printf("X not specified.\n");
		return;
	}

	tmp = find_val(head, "y");
	if (tmp)
	{
		ch_temp[cn].y = atoi(tmp);
	}
	else
	{
		printf("Y not specified.\n");
		return;
	}

	tmp = find_val(head, "dir");
	if (tmp)
	{
		ch_temp[cn].dir = atoi(tmp);
	}
	else
	{
		printf("DIR not specified.\n");
		return;
	}

	tmp = find_val(head, "gold");
	if (tmp)
	{
		ch_temp[cn].gold = atoi(tmp);
	}
	else
	{
		printf("GOLD not specified.\n");
		return;
	}

	for (n = 0; n<20; n++)
	{
		sprintf(buf, "worn%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].worn[n] = atoi(tmp);
		}
	}

	for (n = 0; n<40; n++)
	{
		sprintf(buf, "item%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].item[n] = atoi(tmp);
		}
	}

	tmp = find_val(head, "citem");
	if (tmp)
	{
		ch_temp[cn].citem = atoi(tmp);
	}
	else
	{
		printf("CITEM not specified.\n");
		return;
	}

	if (ch_temp[cn].flags & CF_RESPAWN)
	{
		globs->reset_char = cn;
	}

	if (!ch_temp[cn].data[29] && (ch_temp[cn].flags & CF_RESPAWN))
	{
		ch_temp[cn].data[29] = ch_temp[cn].x + ch_temp[cn].y * MAPX;
	}

	printf("Done.\n");
}

void update_object(LIST *head)
{
	int in, cnt, val, n;
	unsigned long long lval;
	char *tmp, **tmps, buf[80];

	tmp = find_val(head, "in");
	if (tmp)
	{
		in = atoi(tmp);
	}
	else
	{
		printf("IN not specified.\n");
		return;
	}

	bzero(&it_temp[in], sizeof(struct item));
	it_temp[in].used = USE_ACTIVE;

	tmp = find_val(head, "name");
	if (tmp)
	{
		strncpy(it_temp[in].name, tmp, 35);
		it_temp[in].name[35] = 0;
	}
	else
	{
		printf("NAME not specified.\n");
		return;
	}

	tmp = find_val(head, "reference");
	if (tmp)
	{
		strncpy(it_temp[in].reference, tmp, 35);
		it_temp[in].reference[35] = 0;
	}
	else
	{
		printf("REFERENCE not specified.\n");
		return;
	}

	tmp = find_val(head, "description");
	if (tmp)
	{
		strncpy(it_temp[in].description, tmp, 195);
		it_temp[in].description[195] = 0;
	}
	else
	{
		printf("DESCRIPTION not specified.\n");
		return;
	}

	cnt  = find_val_multi(head, "flags", &tmps);
	lval = 0;
	if (cnt)
	{
		for (n = 0; n<cnt; n++)
		{
			lval |= atoll(tmps[n]);
		}
	}
	it_temp[in].flags = lval;

	tmp = find_val(head, "driver");
	if (tmp)
	{
		it_temp[in].driver = atoi(tmp);
	}
	else
	{
		printf("DRIVER not specified.\n");
		return;
	}

	for (n = 0; n<100; n++)
	{
		sprintf(buf, "drdata%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].data[n] = atoi(tmp);
		}
	}

	tmp = find_val(head, "light_1");
	if (tmp)
	{
		it_temp[in].light[0] = atoi(tmp);
	}
	else
	{
		printf("LIGHT_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "light_2");
	if (tmp)
	{
		it_temp[in].light[1] = atoi(tmp);
	}
	else
	{
		printf("LIGHT_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "value");
	if (tmp)
	{
		it_temp[in].value = atoi(tmp);
	}
	else
	{
		printf("VALUE not specified.\n");
		return;
	}

	cnt = find_val_multi(head, "placement", &tmps);
	if (cnt)
	{
		for (n = val = 0; n<cnt; n++)
		{
			val |= atoi(tmps[n]);
		}
		it_temp[in].placement = val;
	}

	for (n = 0; n<5; n++)
	{
		sprintf(buf, "attrib%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].attrib[n][0] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_0 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].attrib[n][1] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_1 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].attrib[n][2] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_2 not specified.", n);
			return;
		}
	}

	tmp = find_val(head, "hp_0");
	if (tmp)
	{
		it_temp[in].hp[0] = atoi(tmp);
	}
	else
	{
		printf("HP_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_1");
	if (tmp)
	{
		it_temp[in].hp[1] = atoi(tmp);
	}
	else
	{
		printf("HP_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_2");
	if (tmp)
	{
		it_temp[in].hp[2] = atoi(tmp);
	}
	else
	{
		printf("HP_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_0");
	if (tmp)
	{
		it_temp[in].end[0] = atoi(tmp);
	}
	else
	{
		printf("END_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_1");
	if (tmp)
	{
		it_temp[in].end[1] = atoi(tmp);
	}
	else
	{
		printf("END_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_2");
	if (tmp)
	{
		it_temp[in].end[2] = atoi(tmp);
	}
	else
	{
		printf("END_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_0");
	if (tmp)
	{
		it_temp[in].mana[0] = atoi(tmp);
	}
	else
	{
		printf("MANA_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_1");
	if (tmp)
	{
		it_temp[in].mana[1] = atoi(tmp);
	}
	else
	{
		printf("MANA_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_2");
	if (tmp)
	{
		it_temp[in].mana[2] = atoi(tmp);
	}
	else
	{
		printf("MANA_2 not specified.\n");
		return;
	}

	for (n = 0; n<50; n++)
	{
		sprintf(buf, "skill%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].skill[n][0] = atoi(tmp);
		}
		else
		{
			it_temp[in].skill[n][0] = 0;
		}

		sprintf(buf, "skill%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].skill[n][1] = atoi(tmp);
		}
		else
		{
			it_temp[in].skill[n][1] = 0;
		}

		sprintf(buf, "skill%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].skill[n][2] = atoi(tmp);
		}
		else
		{
			it_temp[in].skill[n][2] = 0;
		}
	}

	tmp = find_val(head, "armor_1");
	if (tmp)
	{
		it_temp[in].armor[0] = atoi(tmp);
	}
	else
	{
		printf("ARMOR_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "armor_2");
	if (tmp)
	{
		it_temp[in].armor[1] = atoi(tmp);
	}
	else
	{
		printf("ARMOR_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "weapon_1");
	if (tmp)
	{
		it_temp[in].weapon[0] = atoi(tmp);
	}
	else
	{
		printf("WEAPON_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "weapon_2");
	if (tmp)
	{
		it_temp[in].weapon[1] = atoi(tmp);
	}
	else
	{
		printf("WEAPON_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "gethit_dam_1");
	if (tmp)
	{
		it_temp[in].gethit_dam[0] = atoi(tmp);
	}
	else
	{
		printf("GETHIT_DAM_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "gethit_dam_2");
	if (tmp)
	{
		it_temp[in].gethit_dam[1] = atoi(tmp);
	}
	else
	{
		printf("GETHIT_DAM_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "max_age_1");
	if (tmp)
	{
		it_temp[in].max_age[0] = atoi(tmp);
	}
	else
	{
		printf("MAX_AGE_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "max_age_2");
	if (tmp)
	{
		it_temp[in].max_age[1] = atoi(tmp);
	}
	else
	{
		printf("MAX_AGE_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "max_damage");
	if (tmp)
	{
		it_temp[in].max_damage = atoi(tmp);
	}
	else
	{
		printf("MAX_DAMAGE not specified.\n");
		return;
	}

	tmp = find_val(head, "duration");
	if (tmp)
	{
		it_temp[in].duration = atoi(tmp);
	}
	else
	{
		printf("DURATION not specified.\n");
		return;
	}

	tmp = find_val(head, "cost");
	if (tmp)
	{
		it_temp[in].cost = atoi(tmp);
	}
	else
	{
		printf("COST not specified.\n");
		return;
	}

	tmp = find_val(head, "power");
	if (tmp)
	{
		it_temp[in].power = atoi(tmp);
	}
	else
	{
		printf("POWER not specified.\n");
		return;
	}

	tmp = find_val(head, "spr_ovr");
	if (tmp)
	{
		it_temp[in].sprite_override = atoi(tmp);
	}
	else
	{
		printf("SPR_OVR not specified.\n");
		return;
	}

	tmp = find_val(head, "min_rank");
	if (tmp)
	{
		it_temp[in].min_rank = atoi(tmp);
	}
	else
	{
		printf("MIN_RANK not specified.\n");
		return;
	}

	tmp = find_val(head, "sprite_1");
	if (tmp)
	{
		it_temp[in].sprite[0] = atoi(tmp);
	}
	else
	{
		printf("SPRITE_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "sprite_2");
	if (tmp)
	{
		it_temp[in].sprite[1] = atoi(tmp);
	}
	else
	{
		printf("SPRITE_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "status_1");
	if (tmp)
	{
		it_temp[in].status[0] = atoi(tmp);
	}
	else
	{
		printf("STATUS_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "status_2");
	if (tmp)
	{
		it_temp[in].status[1] = atoi(tmp);
	}
	else
	{
		printf("STATUS_2 not specified.\n");
		return;
	}

	globs->reset_item = in;

	printf("Done.\n");
}


void list_characters_template(LIST *head)    // excludes grolms, gargs, icegargs. decides by sprite-nr
{
	int n;
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<table>\n");

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (ch_temp[n].sprite==12240 || ch_temp[n].sprite==18384 || ch_temp[n].sprite==21456)
		{
			continue;
		}
		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
		       "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
		       n, n,
		       (ch_temp[n].flags & CF_RESPAWN) ? "<b>" : "",
		       ch_temp[n].name,
		       (ch_temp[n].flags & CF_RESPAWN) ? "</b>" : "",
		       ch_temp[n].x, ch_temp[n].y, ch_temp[n].points_tot >> 10, ch_temp[n].alignment, n, n);
	}

	printf("</table>\n");
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
}

void list_characters2_template(LIST *head)       // listing grolms,gargs,icegargs only, desides by sprite-nr
{
	int n;
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<table>\n");

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch_temp[n].sprite==12240 || ch_temp[n].sprite==18384 || ch_temp[n].sprite==21456))
		{
			continue;
		}
		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
		       "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
		       n, n,
		       (ch_temp[n].flags & CF_RESPAWN) ? "<b>" : "",
		       ch_temp[n].name,
		       (ch_temp[n].flags & CF_RESPAWN) ? "</b>" : "",
		       ch_temp[n].x, ch_temp[n].y, ch_temp[n].points_tot >> 10, ch_temp[n].alignment, n, n);
	}

	printf("</table>\n");
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
}

void list_characters_template_good(LIST *head)    // excludes grolms, gargs, icegargs. decides by sprite-nr
{
	int n;
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<table>\n");

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (ch_temp[n].sprite==12240 || ch_temp[n].sprite==18384 || ch_temp[n].sprite==21456)
		{
			continue;
		}
		if(ch_temp[n].alignment < 0)
		{
			continue;
		}
		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
		       "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
		       n, n,
		       (ch_temp[n].flags & CF_RESPAWN) ? "<b>" : "",
		       ch_temp[n].name,
		       (ch_temp[n].flags & CF_RESPAWN) ? "</b>" : "",
		       ch_temp[n].x, ch_temp[n].y, ch_temp[n].points_tot >> 10, ch_temp[n].alignment, n, n);
	}

	printf("</table>\n");
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
}

void list_characters_template_non_monster(LIST *head)    // excludes grolms, gargs, icegargs. decides by sprite-nr
{
	int n;
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
	printf("<table>\n");

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if(ch_temp[n].kindred & KIN_MONSTER)
		{
			continue;
		}
		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
		       "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
		       n, n,
		       (ch_temp[n].flags & CF_RESPAWN) ? "<b>" : "",
		       ch_temp[n].name,
		       (ch_temp[n].flags & CF_RESPAWN) ? "</b>" : "",
		       ch_temp[n].x, ch_temp[n].y, ch_temp[n].points_tot >> 10, ch_temp[n].alignment, n, n);
	}

	printf("</table>\n");
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");
}



void list_named_characters_template(LIST *head)    // excludes grolms, gargs, icegargs. decides by sprite-nr
{
	int n;
	int i;
	printf("<table>\n");

	for (n = 0; n<MAXTCHARS; n++)
	{
		int found = 0;
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (ch_temp[n].sprite==12240 ||
		    ch_temp[n].sprite==18384 ||
		    ch_temp[n].sprite==21456 ||
		    ch_temp[n].sprite==14288 ||
		    ch_temp[n].sprite==31696)
		{
			continue;
		}
		for(i = 0; i < bad_name_count; i++)
		{
			if(strcmp(ch_temp[n].name, bad_names[i]) == 0)
			{
				found = 1;
			}
		}
		if(found == 1)
		{
			continue;
		}
		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
		       "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
		       n, n,
		       (ch_temp[n].flags & CF_RESPAWN) ? "<b>" : "",
		       ch_temp[n].name,
		       (ch_temp[n].flags & CF_RESPAWN) ? "</b>" : "",
		       ch_temp[n].x, ch_temp[n].y, ch_temp[n].points_tot >> 10, ch_temp[n].alignment, n, n);
	}

	printf("</table>\n");
}

void list_new_characters_template(LIST *head)        // listing characters with high IDs
{
	int n;

	printf("<table>\n");

	for (n = 1012; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
		       "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
		       n, n,
		       (ch_temp[n].flags & CF_RESPAWN) ? "<b>" : "",
		       ch_temp[n].name,
		       (ch_temp[n].flags & CF_RESPAWN) ? "</b>" : "",
		       ch_temp[n].x, ch_temp[n].y, ch_temp[n].points_tot >> 10, ch_temp[n].alignment, n, n);
	}

	printf("</table>\n");
}

void list_objects(LIST *head)
{
	int n;
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");

	printf("<table>\n");

	for (n = 1; n<MAXTITEM; n++)
	{
		if (it_temp[n].used==USE_EMPTY)
		{
			continue;
		}
//                if (it_temp[n].driver!=23) continue;
//		if (!(it_temp[n].flags&IF_TAKE)) continue;

		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=23&in=%d>%30.30s</a></td>\n"
		       "<td>price: %dG, %dS</td><td>data4: %d</td>\n"
		       "<td><a href=/cgi-imp/acct.cgi?step=25&in=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=22&in=%d>Delete</a></td></tr>\n",
		       n, n, it_temp[n].name,
		       it_temp[n].value / 100, it_temp[n].value % 100, it_temp[n].data[4],
		       n, n);
	}

	printf("</table>\n");
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");

}

void list_items(LIST *head)
{
	int n;
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");

	printf("<table>\n");

	for (n = 1; n<MAXITEM; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			continue;
		}
		if(it[n].x==0 && it[n].y==0 && it[n].carried==0)
		{
			continue;
		}

		if((it[n].flags & IF_SPELL))
		{
			continue;
		}

//                if (it_temp[n].driver!=23) continue;
//		if (!(it_temp[n].flags&IF_TAKE)) continue;

		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=28&in=%d>%30.30s</a></td>\n"
		       "<td>price: %dG, %dS</td><td>data4: %d</td>\n"
		       "<td><a href=/cgi-imp/acct.cgi?step=25&in=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=22&in=%d>Delete</a></td></tr>\n",
		       n, n, it[n].name,
		       it[n].value / 100, it[n].value % 100, it[n].data[4],
		       n, n);
	}

	printf("</table>\n");
	printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n");

}

void list_object_drivers(LIST *head)
{
	int n, nd;
	int ndrivers = 0;
	int found;

	// Find out how many drivers there are
	for (n = 1; n<MAXTITEM; n++)
	{
		if (it_temp[n].driver > ndrivers)
		{
			ndrivers = it_temp[n].driver;
		}
	}
	printf("<ul compact>\n");
	for (nd = 1; nd<=ndrivers; nd++)
	{
		found = 0;
		for (n = 1; n<MAXTITEM; n++)
		{
			if (it_temp[n].used==USE_EMPTY)
			{
				continue;
			}
			if (it_temp[n].driver == nd)
			{
				if (!found)
				{
					printf("<li>Driver #%d:"
					       "<dir compact>", nd);
					found = 1;
				}
				printf("<li>"
				       "<a href=/cgi-imp/acct.cgi?step=23&in=%d>%d:</a>"
				       "&nbsp;&nbsp;&nbsp;%s"
				       "\n</li>\n", n, n, it_temp[n].name);
			}
		}
		if (found)
		{
			printf("</dir></li>\n");
		}
	}
	printf("\n</ul>\n");
}

int main(int argc, char *args[])
{
	int step = 0;
	int n __attribute__ ((unused));
	char *tmp;
	LIST *head;
	head = is_form_empty() ? NULL : cgi_input_parse();

	int result = chdir("/home/merc");

	if(result != 0)
	{
		printf("Unable to find /home/merc");
		exit(1);
	}

	printf("Content-Type: text/html\n\n");
	printf("<html><head><title>World Builder</title><META HTTP-EQUIV=\"PRAGMA\" CONTENT=\"NO-CACHE\"></head>\n");
	#if defined(SUSE)
	printf("<BODY TEXT=#D7D700 BGCOLOR=#264A9F LINK=#FFFFBB VLINK=#CCCC00 ALINK=#FFFF9D\n");
	#else
	printf("<BODY TEXT=#D7D700 BGCOLOR=#264A9F LINK=#FFFFBB VLINK=#CCCC00 ALINK=#FFFF9D background=/gfx/back4.gif>\n");
	#endif
	printf("<center>\n");
	printf("<table width=\"100%%\"><tr>\n");
	printf("<td align=center>\n");
	printf("<a href=\"https://github.com/dylanyaga/openMerc\"><img src=\"/gfx/logo.gif\" width=100 height=60 border=0></a>\n");
	printf("</td><td align=center>\n");
	printf("<h2>Server Info</h2>\n");
	printf("</td><td align=center>\n");
	printf("<a href=\"https://github.com/dylanyaga/openMerc\"><img src=\"/gfx/logo.gif\" width=100 height=60 border=0></a>");
	printf("</td></tr></table>\n");
	printf("<hr width=80%% color=\"#808000\"><br>\n");
	printf("<table width=\"80%%\"><tr><td>\n");

	web_load();

	#ifdef RESET_CHAR_KLUDGE
	n = atoi(args[1]);
	globs->reset_char = n;
	web_unload();
	exit(0);
	#endif

	if (head)
	{
		tmp = find_val(head, "step");
		if (tmp)
		{
			step = atoi(tmp);
		}
	}

	ch_temp[1].used = USE_ACTIVE;
	strcpy(ch_temp[1].name, "Blank Template");

	it_temp[1].used = USE_ACTIVE;
	strcpy(it_temp[1].name, "Blank Template");

	/* for (n=1; n<MAXTCHARS; n++) {
	        if (ch_temp[n].used==USE_EMPTY) continue;
	   } */

/*      for (n=1; n<MAXTITEM; n++) {
                if (it_temp[n].used==USE_EMPTY) continue;
                it_temp[n].value/=5;
        } */

	switch(step)
	{
	case    11:
		list_characters_template(head);
		break;
	case    12:
		delete_character_template(head);
		break;
	case    13:
		view_character_template(head);
		break;
	case    14:
		update_character_template(head);
		break;
	case    15:
		copy_character_template(head);
		break;
	case    16:
		list_characters_template_good(head);
		break;
	case 17:
		list_named_characters_template(head);
		break;
	case 18:
		list_characters_template_non_monster(head);
		break;
	case 19:
		list_all_player_characters();
		break;
	case 20:
		view_character_player(head);
		break;
	case    21:
		list_objects(head);
		break;
	case    22:
		delete_object(head);
		break;
	case    23:
		view_object(head);
		break;
	case    24:
		update_object(head);
		break;
	case    25:
		copy_object(head);
		break;
	case 26:
		update_character_player(head);
		break;
	case 27:
		list_items(head);
		break;
	case 28:
		view_item(head);
		break;
	case    31:
		list_object_drivers(head);
		break;
	case    41:
		list_characters2_template(head);
		break;
	case    51:
		list_new_characters_template(head);
		break;


	default:
		printf("Player Web Editing (Experimental)<br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=19>All Player Characters </a><br><br>\n");
		printf("Together those lists include all character-templates<br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=11>Characters (without Grolms, Gargoyles, Icegargs)</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=16>Characters (only with Positive Alignment) </a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=17>Characters (only Named) </a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=18>Characters (Non Monster) </a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=41>Characters (only Grolms, Gargoyles, Icegargs) </a><br><br>\n");
		printf("This list includes only characters with high IDs for fast access<br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=51>New characters (only if they got a high ID)</a><br><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=21>Object Templates</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=31>Object Driver List</a><br><br>\n");
		printf("Show All Items<br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=27>Item List</a><br>\n");
		break;
	}

	web_unload();

	printf("</td></tr></table>\n");

	printf("<hr width=80%% color=\"#808000\"><br>\n");
	printf("<a href=/cgi-imp/acct.cgi>Back to main page</a>\n");
	printf("<hr width=80%% color=\"#808000\"><br>\n");
	printf("<font size=-1>All material on this server is based on the Mercenaries of Astonia engine by Daniel Brockhaus.</font></center>\n");
	printf("</body></html>\n");


	return(0);
}
