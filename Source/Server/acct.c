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
#include "cgi-lib.h" /* include the cgi-lib.h header file */
#include "html-lib.h" /* include the html-lib.h header file */
#include "gendefs.h"
#include "data.h"

/* CS, 991113: TCHARSIZE and TITEMSIZE now in data.h */

struct character *ch;
struct item *it;
struct global *globs;

#if 0
unsigned long long atoll(char *string)
{
        unsigned long long val=0;

        while (isspace(*string)) string++;

        while (isdigit(*string)) {
                val*=10;
                val+=*string-'0';
                string++;
        }
        return val;
}
#endif

char *at_name[5]={
"Braveness",
"Willpower",
"Intuition",
"Agility",
"Strength"};

static int load(void)
{
        int handle;
        char cwd[128];

        handle=open(DATDIR"/tchar.dat",O_RDWR);
        if (handle==-1) {
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                        fprintf(stderr, "cwd: %s\n", cwd);
                perror(DATDIR"/tchar.dat");
                return -1;
        }

        ch=mmap(NULL,TCHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (ch==(void*)-1) {
                fprintf(stderr,"cannot mmap tchar.dat.\n");
                return -1;
        }
        close(handle);

        handle=open(DATDIR"/titem.dat",O_RDWR);
        if (handle==-1) {
                fprintf(stderr,"titem.dat does not exist.\n");
                return -1;
        }

        it=mmap(NULL,TITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (it==(void*)-1) {
                fprintf(stderr,"cannot mmap titem.dat.\n");
                return -1;
        }
        close(handle);

        handle=open(DATDIR"/global.dat",O_RDWR);
        if (handle==-1) return -1;

        globs=mmap(NULL,sizeof(struct global),PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (globs==(void*)-1) return -1;
        close(handle);

        return 0;
}

static void unload(void)
{
        if (munmap(ch,TCHARSIZE)) perror("munmap(ch)");
        if (munmap(it,TITEMSIZE)) perror("munmap(it)");
        if (munmap(globs,sizeof(struct global))) perror("munmap(globs)");
}

static char *weartext[20]={"Head","Neck","Body","Arms","Belt","Legs","Feet","Left Hand","Right Hand","Cloak",
                           "Left Ring","Right Ring"};

static char *text_name[10]={
"Killed Enemy $1",      //0
"Will attack $1",       //1
"Greeting $1",          //2
"Killed by $1",         //3
"Shout Help against $1",//4
"Going to Help $1",     //5
"Keyword",              //6
"Rection to keyword",   //7
"Warning about attack", //8
"Unused",       //9
};

static char *data_name[100]={
"Generic 1",            //0
"Generic 2",            //1
"Generic 3",            //2
"Generic 4",            //3
"Generic 5",            //4
"Generic 6",            //5
"Generic 7",            //6
"Generic 8",            //7
"Generic 9",            //8
"Generic 10",           //9
"Patrol",               //10
"Patrol",               //11
"Patrol",               //12
"Patrol",               //13
"Patrol",               //14
"Patrol",               //15
"Patrol",               //16
"Patrol",               //17
"Patrol",               //18
"Reserved",             //19
"Close Door",           //20
"Close Door",           //21
"Close Door",           //22
"Close Door",           //23
"Prevent Fights",       //24
"Special Driver",       //25
"Special Sub-Driver",   //26
"Reserved",             //27
"Unused",               //28
"Resting Position",     //29
"Resting Dir",          //30
"Protect Templ",        //31
"Activate Light",       //32
"Activate Light",       //33
"Activate Light",       //34
"Activate Light",       //35
"Reserved",             //36
"Greet everyone",       //37
"Reserved",             //38
"Reserved",             //39
"Reserved",             //40
"Street Light Templ.",  //41
"Group",                //42
"Attack all but Group", //43
"Attack all but Group", //44
"Attack all but Group", //45
"Attack all but Group", //46
"Donate Trash to pl #", //47
"Prob. of dying msg",   //48
"Wants item #",         //49
"Teaches Skill #",      //50
"Gives # EXPs",         //51
"Shout help/code #",    //52
"Come help/code #",     //53
"Reserved",             //54
"Reserved",             //55
"Reserved",             //56
"Reserved",             //57
"Reserved",             //58
"Protect Group",        //59
"Random Walk Time",     //60
"Reserved",             //61
"Create Light",         //62
"Reserved",             //63
"Self Destruct",        //64
"Reserved",             //65
"Gives Item #",         //66
"Reserved",             //67
"Level of Knowledge",   //68
"Accepts Money",        //69
"Reserved",             //70
"Talkativity",          //71
"Area of Knowledge",    //72
"Random Walk Max Dist", //73
"Reserved",             //74
"Reserved",             //75
"Reserved",             //76
"Reserved",             //77
"Reserved",             //78
"Rest between patrol",  //79
"Reserved",             //80
"Reserved",             //81
"Reserved",             //82
"Reserved",             //83
"Reserved",             //84
"Reserved",             //85
"Reserved",             //86
"Reserved",             //87
"Reserved",             //88
"Reserved",             //89
"Reserved",             //90
"Reserved",             //91
"Reserved",             //92
"Attack distance",      //93
"Reserved",             //94
"Keyword Action",       //95
"Reserved",             //96
"Unused",               //97
"Unused",               //98
"Reserved"              //99
};

void copy_character(LIST *head)
{
        int cn,n;
        char *tmp;

        tmp=find_val(head,"cn");
        if (tmp) cn=atoi(tmp);
        else { printf("No number specified."); return; }

        if (cn<0 || cn>=MAXTCHARS) { printf("Number out of bounds."); return; }

        for (n=1; n<MAXTCHARS; n++)
                if (ch[n].used==USE_EMPTY) break;
        if (n==MAXTCHARS) { printf("MAXTCHARS reached!"); return; }
        ch[n]=ch[cn];

        printf("Done. Here is your new, copied monster");
	printf("<table>");	  // show copied monster for direct access.

        
//                if (ch[n].used==USE_EMPTY) continue;
                printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
                        "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
                        n,n,
                        (ch[n].flags&CF_RESPAWN) ? "<b>" : "",
                        ch[n].name,
                        (ch[n].flags&CF_RESPAWN) ? "</b>" : "",
                        ch[n].x,ch[n].y,ch[n].points_tot>>10,ch[n].alignment,n,n);

        printf("</table>");
	
	
}

void view_character(LIST *head)
{
        int cn,n;
        char *tmp;

        tmp=find_val(head,"cn");
        if (tmp) cn=atoi(tmp);
        else { printf("No number specified."); return; }

        if (cn<0 || cn>=MAXTCHARS) { printf("Number out of bounds."); return; }

        printf("<form method=post action=/cgi-imp/acct.cgi>");
        printf("<table>");
        printf("<tr><td>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>",
                ch[cn].name);
        printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>",
                ch[cn].reference);
        printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>",
                ch[cn].description);

        printf("<tr><td valign=top>Kindred:</td><td>");
        printf("Mercenary <input type=checkbox name=kindred value=%d %s><br>",
                KIN_MERCENARY,(ch[cn].kindred&KIN_MERCENARY) ? "checked" : "");
        printf("Sorcerer-Mercenary <input type=checkbox name=kindred value=%d %s><br>",
                KIN_SORCERER,(ch[cn].kindred&KIN_SORCERER) ? "checked" : "");
        printf("Warrior-Mercenary <input type=checkbox name=kindred value=%d %s><br>",
                KIN_WARRIOR,(ch[cn].kindred&KIN_WARRIOR) ? "checked" : "");
        printf("Templar <input type=checkbox name=kindred value=%d %s><br>",
                KIN_TEMPLAR,(ch[cn].kindred&KIN_TEMPLAR) ? "checked" : "");
        printf("Arch-Templar <input type=checkbox name=kindred value=%d %s><br>",
                KIN_ARCHTEMPLAR,(ch[cn].kindred&KIN_ARCHTEMPLAR) ? "checked" : "");
        printf("Harakim <input type=checkbox name=kindred value=%d %s><br>",
                KIN_HARAKIM,(ch[cn].kindred&KIN_HARAKIM) ? "checked" : "");
        printf("Arch-Harakim <input type=checkbox name=kindred value=%d %s><br>",
                KIN_ARCHHARAKIM,(ch[cn].kindred&KIN_ARCHHARAKIM) ? "checked" : "");
        printf("Seyan'Du <input type=checkbox name=kindred value=%d %s><br>",
                KIN_SEYAN_DU,(ch[cn].kindred&KIN_SEYAN_DU) ? "checked" : "");
        printf("Purple <input type=checkbox name=kindred value=%d %s><br>",
                KIN_PURPLE,(ch[cn].kindred&KIN_PURPLE) ? "checked" : "");
        printf("Monster <input type=checkbox name=kindred value=%d %s><br>",
                KIN_MONSTER,(ch[cn].kindred&KIN_MONSTER) ? "checked" : "");
        printf("Male <input type=checkbox name=kindred value=%d %s><br>",
                KIN_MALE,(ch[cn].kindred&KIN_MALE) ? "checked" : "");
        printf("Female <input type=checkbox name=kindred value=%d %s><br>",
                KIN_FEMALE,(ch[cn].kindred&KIN_FEMALE) ? "checked" : "");
        printf("</td></tr>");

        printf("<tr><td valign=top>Sprite base:</td><td><input type=text name=sprite value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].sprite);

        printf("<tr><td valign=top>Sound base:</td><td><input type=text name=sound value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].sound);

        printf("<tr><td valign=top>Class:</td><td><input type=text name=class value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].class);

        printf("<tr><td valign=top>Flags:</td><td>");
        printf("Infrared <input type=checkbox name=flags value=%Lu %s><br>",
                CF_INFRARED,(ch[cn].flags&CF_INFRARED) ? "checked" : "");
        printf("Undead <input type=checkbox name=flags value=%Lu %s><br>",
                CF_UNDEAD,(ch[cn].flags&CF_UNDEAD) ? "checked" : "");
        printf("Respawn <input type=checkbox name=flags value=%Lu %s><br>",
                CF_RESPAWN,(ch[cn].flags&CF_RESPAWN) ? "checked" : "");
        printf("No-Sleep <input type=checkbox name=flags value=%Lu %s><br>",
                CF_NOSLEEP,(ch[cn].flags&CF_NOSLEEP) ? "checked" : "");
        printf("Merchant <input type=checkbox name=flags value=%Lu %s><br>",
                CF_MERCHANT,(ch[cn].flags&CF_MERCHANT) ? "checked" : "");
        printf("Simple Animation <input type=checkbox name=flags value=%Lu %s><br>",
                CF_SIMPLE,(ch[cn].flags&CF_SIMPLE) ? "checked" : "");
        printf("</td></tr>");

        printf("<tr><td valign=top>Alignment</td><td><input type=text name=alignment value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].alignment);

        printf("<tr><td colspan=2><table>");

        printf("<tr><td>Name</td><td>Start Value</td><td>Race Mod</td><td>Race Max</td><td>Difficulty</td></tr>");

        for (n=0; n<5; n++) {
                printf("<tr><td>%s</td>",at_name[n]);
                printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>",n,ch[cn].attrib[n][0]);
                printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>",n,ch[cn].attrib[n][1]);
                printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>",n,ch[cn].attrib[n][2]);
                printf("<td><input type=text name=attrib%d_3 value=\"%d\" size=10 maxlength=10></td>",n,ch[cn].attrib[n][3]);
                printf("</tr>");
        }

        printf("<tr><td>Hitpoints</td>");
        printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>",ch[cn].hp[0]);
        printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>",ch[cn].hp[1]);
        printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>",ch[cn].hp[2]);
        printf("<td><input type=text name=hp_3 value=\"%d\" size=10 maxlength=10></td>",ch[cn].hp[3]);
        printf("</tr>");

        printf("<tr><td>Endurance</td>");
        printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>",ch[cn].end[0]);
        printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>",ch[cn].end[1]);
        printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>",ch[cn].end[2]);
        printf("<td><input type=text name=end_3 value=\"%d\" size=10 maxlength=10></td>",ch[cn].end[3]);
        printf("</tr>");

        printf("<tr><td>Mana</td>");
        printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>",ch[cn].mana[0]);
        printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>",ch[cn].mana[1]);
        printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>",ch[cn].mana[2]);
        printf("<td><input type=text name=mana_3 value=\"%d\" size=10 maxlength=10></td>",ch[cn].mana[3]);
        printf("</tr>");

        for (n=0; n<50; n++) {
                if (skilltab[n].name[0]==0) continue;
                printf("<tr><td>%s</td>",skilltab[n].name);
                printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>",n,ch[cn].skill[n][0]);
                printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>",n,ch[cn].skill[n][1]);
                printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>",n,ch[cn].skill[n][2]);
                printf("<td><input type=text name=skill%d_3 value=\"%d\" size=10 maxlength=10></td>",n,ch[cn].skill[n][3]);
                printf("</tr>");
        }

        printf("</table></td></tr>");

        printf("<tr><td valign=top>Speed Mod</td><td><input type=text name=speed_mod value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].speed_mod);
        printf("<tr><td valign=top>Weapon Bonus</td><td><input type=text name=weapon_bonus value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].weapon_bonus);
        printf("<tr><td valign=top>Armor Bonus</td><td><input type=text name=armor_bonus value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].armor_bonus);
        printf("<tr><td valign=top>Gethit Bonus</td><td><input type=text name=gethit_bonus value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].gethit_bonus);
        printf("<tr><td valign=top>Light Bonus</td><td><input type=text name=light_bonus value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].light_bonus);

        printf("<tr><td>EXP left:</td><td><input type=text name=points value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].points);
        printf("<tr><td>EXP total:</td><td><input type=text name=points_tot value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].points_tot);

        printf("<tr><td>Position X:</td><td><input type=text name=x value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].x);
        printf("<tr><td>Position Y:</td><td><input type=text name=y value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].y);
        printf("<tr><td>Direction:</td><td><input type=text name=dir value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].dir);
        printf("<tr><td>Gold:</td><td><input type=text name=gold value=\"%d\" size=10 maxlength=10></td></tr>",
                ch[cn].gold);

        if (ch[cn].citem<0 || ch[cn].citem>=MAXTITEM) ch[cn].citem=0;

        printf("<tr><td>Current Item</td><td><input type=text name=citem value=\"%d\" size=10 maxlength=10> (%s)</td></tr>",
                ch[cn].citem,ch[cn].citem ? it[ch[cn].citem].name : "none");

        for (n=0; n<12; n++) {
                if (ch[cn].worn[n]>=MAXTITEM) ch[cn].worn[n]=0;
                printf("<tr><td>%s</td><td><input type=text name=worn%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>",
                weartext[n],n,ch[cn].worn[n],ch[cn].worn[n] ? it[ch[cn].worn[n]].name : "none");
        }

        for (n=0; n<40; n++) {
                if (ch[cn].item[n]<0 || ch[cn].item[n]>=MAXTITEM) ch[cn].item[n]=0;
                printf("<tr><td>Item %d</td><td><input type=text name=item%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>",
                n,n,ch[cn].item[n],ch[cn].item[n] ? it[ch[cn].item[n]].name : "none");
        }

        printf("<tr><td>Driver Data:</td></tr>");
        for (n=0; n<100; n++) {
                if (strcmp(data_name[n],"Unused")==0 || strcmp(data_name[n],"Reserved")==0) continue;
                printf("<tr><td>[%3d] %s</td><td><input type=text name=drdata%d value=\"%d\" size=35 maxlength=75></td></tr>",
                        n, data_name[n],n,ch[cn].data[n]);
        }

        printf("<tr><td>Driver Texts:</td></tr>");
        for (n=0; n<10; n++) {
                if (strcmp(text_name[n],"Unused")==0 || strcmp(text_name[n],"Reserved")==0) continue;
                printf("<tr><td>%s</td><td><input type=text name=text_%d value=\"%s\" size=35 maxlength=158></td></tr>",
                        text_name[n],n,ch[cn].text[n]);
        }

        printf("<tr><td><input type=submit value=Update></td><td> </td></tr>");
        printf("</table>");
        printf("<input type=hidden name=step value=14>");
        printf("<input type=hidden name=cn value=%d>",cn);
        printf("</form>");
        printf("<a href=/cgi-imp/acct.cgi?step=10>Back</a><br>");
}

static char *obj_drdata[]={
"Drdata 0",
"Drdata 1",
"Drdata 2",
"Drdata 3",
"Drdata 4",
"Drdata 5",
"Drdata 6",
"Drdata 7",
"Drdata 8",
"Drdata 9"};

void copy_object(LIST *head)
{
        int in,n;
        char *tmp;

        tmp=find_val(head,"in");
        if (tmp) in=atoi(tmp);
        else { printf("No number specified."); return; }

        if (in<1 || in>=MAXTCHARS) { printf("Number out of bounds."); return; }

        for (n=1; n<MAXTITEM; n++)
                if (it[n].used==USE_EMPTY) break;
        if (n==MAXTITEM) { printf("MAXTITEM reached!"); return; }
        it[n]=it[in];

        printf("Done.");
}

void view_object(LIST *head)
{
        int in,n;
        char *tmp;

        tmp=find_val(head,"in");
        if (tmp) in=atoi(tmp);
        else { printf("No number specified."); return; }

        if (in<1 || in>=MAXTCHARS) { printf("Number out of bounds."); return; }

        printf("<form method=post action=/cgi-imp/acct.cgi>");
        printf("<table>");
        printf("<tr><td valign=top>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>",
                it[in].name);
        printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>",
                it[in].reference);
        printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>",
                it[in].description);

        printf("<tr><td valign=top>Flags:</td><td>");
        printf("Moveblock <input type=checkbox name=flags value=%Lu %s><br>",
                IF_MOVEBLOCK,(it[in].flags&IF_MOVEBLOCK) ? "checked" : "");
        printf("Sightblock <input type=checkbox name=flags value=%Lu %s><br>",
                IF_SIGHTBLOCK,(it[in].flags&IF_SIGHTBLOCK) ? "checked" : "");
        printf("Take-Able <input type=checkbox name=flags value=%Lu %s><br>",
                IF_TAKE,(it[in].flags&IF_TAKE) ? "checked" : "");
        printf("Look-Able <input type=checkbox name=flags value=%Lu %s><br>",
                IF_LOOK,(it[in].flags&IF_LOOK) ? "checked" : "");
        printf("Look-Special <input type=checkbox name=flags value=%Lu %s><br>",
                IF_LOOKSPECIAL,(it[in].flags&IF_LOOKSPECIAL) ? "checked" : "");
        printf("Use <input type=checkbox name=flags value=%Lu %s><br>",
                IF_USE,(it[in].flags&IF_USE) ? "checked" : "");
        printf("Use-Special <input type=checkbox name=flags value=%Lu %s><br>",
                IF_USESPECIAL,(it[in].flags&IF_USESPECIAL) ? "checked" : "");
        printf("Use-Destroy <input type=checkbox name=flags value=%Lu %s><br>",
                IF_USEDESTROY,(it[in].flags&IF_USEDESTROY) ? "checked" : "");
        printf("Use-Activate <input type=checkbox name=flags value=%Lu %s><br>",
                IF_USEACTIVATE,(it[in].flags&IF_USEACTIVATE) ? "checked" : "");
        printf("Use-Deactivate <input type=checkbox name=flags value=%Lu %s><br>",
                IF_USEDEACTIVATE,(it[in].flags&IF_USEDEACTIVATE) ? "checked" : "");
        printf("Re-Activate <input type=checkbox name=flags value=%Lu %s><br>",
                IF_REACTIVATE,(it[in].flags&IF_REACTIVATE) ? "checked" : "");
        printf("No-Repair <input type=checkbox name=flags value=%Lu %s><br>",
                IF_NOREPAIR,(it[in].flags&IF_NOREPAIR) ? "checked" : "");

        printf("Hidden (data[9])<input type=checkbox name=flags value=%Lu %s><br>",
                IF_HIDDEN,(it[in].flags&IF_HIDDEN) ? "checked" : "");
        printf("Step-Action <input type=checkbox name=flags value=%Lu %s><br>",
                IF_STEPACTION,(it[in].flags&IF_STEPACTION) ? "checked" : "");
        printf("Expire-Proc <input type=checkbox name=flags value=%Lu %s><br>",
                IF_EXPIREPROC,(it[in].flags&IF_EXPIREPROC) ? "checked" : "");

        printf("Fast Light Age <input type=checkbox name=flags value=%Lu %s><br>",
                IF_LIGHTAGE,(it[in].flags&IF_LIGHTAGE) ? "checked" : "");

        printf("Unique <input type=checkbox name=flags value=%Lu %s><br>",
                IF_UNIQUE,(it[in].flags&IF_UNIQUE) ? "checked" : "");

        printf("Spell <input type=checkbox name=flags value=%Lu %s><br>",
                IF_SPELL,(it[in].flags&IF_SPELL) ? "checked" : "");
        printf("Shop-Destroy (quest items)<input type=checkbox name=flags value=%Lu %s><br>",
                IF_SHOPDESTROY,(it[in].flags&IF_SHOPDESTROY) ? "checked" : "");
        printf("Laby-Destroy (quest items)<input type=checkbox name=flags value=%Lu %s><br>",
                IF_LABYDESTROY,(it[in].flags&IF_LABYDESTROY) ? "checked" : "");
        printf("No-Depot (quest items)<input type=checkbox name=flags value=%Lu %s><br>",
                IF_NODEPOT,(it[in].flags&IF_NODEPOT) ? "checked" : "");
        printf("No Market (no price change)<input type=checkbox name=flags value=%Lu %s><br>",
                IF_NOMARKET,(it[in].flags&IF_NOMARKET) ? "checked" : "");
        printf("Donate (cheap items)<input type=checkbox name=flags value=%Lu %s><br>",
                IF_DONATE,(it[in].flags&IF_DONATE) ? "checked" : "");
        printf("Single-Age <input type=checkbox name=flags value=%Lu %s><br>",
                IF_SINGLEAGE,(it[in].flags&IF_SINGLEAGE) ? "checked" : "");
        printf("Always expire when inactive <input type=checkbox name=flags value=%Lu %s><br>",
                IF_ALWAYSEXP1,(it[in].flags&IF_ALWAYSEXP1) ? "checked" : "");
        printf("Always expire when active <input type=checkbox name=flags value=%Lu %s><br>",
                IF_ALWAYSEXP2,(it[in].flags&IF_ALWAYSEXP2) ? "checked" : "");
        printf("Armor <input type=checkbox name=flags value=%Lu %s><br>",
                IF_ARMOR,(it[in].flags&IF_ARMOR) ? "checked" : "");
        printf("Magic <input type=checkbox name=flags value=%Lu %s><br>",
                IF_MAGIC,(it[in].flags&IF_MAGIC) ? "checked" : "");
        printf("Misc <input type=checkbox name=flags value=%Lu %s><br>",
                IF_MISC,(it[in].flags&IF_MISC) ? "checked" : "");
        printf("Weapon: Sword <input type=checkbox name=flags value=%Lu %s><br>",
                IF_WP_SWORD,(it[in].flags&IF_WP_SWORD) ? "checked" : "");
        printf("Weapon: Dagger <input type=checkbox name=flags value=%Lu %s><br>",
                IF_WP_DAGGER,(it[in].flags&IF_WP_DAGGER) ? "checked" : "");
        printf("Weapon: Axe <input type=checkbox name=flags value=%Lu %s><br>",
                IF_WP_AXE,(it[in].flags&IF_WP_AXE) ? "checked" : "");
        printf("Weapon: Club <input type=checkbox name=flags value=%Lu %s><br>",
                IF_WP_STAFF,(it[in].flags&IF_WP_STAFF) ? "checked" : "");
        printf("Weapon: Two-Handed <input type=checkbox name=flags value=%Lu %s><br>",
                IF_WP_TWOHAND,(it[in].flags&IF_WP_TWOHAND) ? "checked" : "");
        printf("</td></tr>");

        printf("<tr><td>Value:</td><td><input type=text name=value value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].value);

        printf("<tr><td valign=top>Placement:</td><td>");
        printf("Head <input type=checkbox name=placement value=%d %s><br>",
                PL_HEAD,(it[in].placement&PL_HEAD) ? "checked" : "");
        printf("Neck <input type=checkbox name=placement value=%d %s><br>",
                PL_NECK,(it[in].placement&PL_NECK) ? "checked" : "");
        printf("Body <input type=checkbox name=placement value=%d %s><br>",
                PL_BODY,(it[in].placement&PL_BODY) ? "checked" : "");
        printf("Arms <input type=checkbox name=placement value=%d %s><br>",
                PL_ARMS,(it[in].placement&PL_ARMS) ? "checked" : "");
        printf("Belt <input type=checkbox name=placement value=%d %s><br>",
                PL_BELT,(it[in].placement&PL_BELT) ? "checked" : "");
        printf("Legs <input type=checkbox name=placement value=%d %s><br>",
                PL_LEGS,(it[in].placement&PL_LEGS) ? "checked" : "");
        printf("Feet <input type=checkbox name=placement value=%d %s><br>",
                PL_FEET,(it[in].placement&PL_FEET) ? "checked" : "");
        printf("Left Hand <input type=checkbox name=placement value=%d %s><br>",
                PL_SHIELD,(it[in].placement&PL_SHIELD) ? "checked" : "");
        printf("Right Hand <input type=checkbox name=placement value=%d %s><br>",
                PL_WEAPON,(it[in].placement&PL_WEAPON) ? "checked" : "");
        printf("Cloak <input type=checkbox name=placement value=%d %s><br>",
                PL_CLOAK,(it[in].placement&PL_CLOAK) ? "checked" : "");
        printf("Two-Handed <input type=checkbox name=placement value=%d %s><br>",
                PL_TWOHAND,(it[in].placement&PL_TWOHAND) ? "checked" : "");
        printf("Ring <input type=checkbox name=placement value=%d %s><br>",
                PL_RING,(it[in].placement&PL_RING) ? "checked" : "");

        printf("</td></tr>");


        printf("<tr><td colspan=2><table>");

        printf("<tr><td>Name</td><td>Inactive Mod</td><td>Active Mod</td><td>Min to wear</td><td>");

        for (n=0; n<5; n++) {
                printf("<tr><td>%s</td>",at_name[n]);
                printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>",n,it[in].attrib[n][0]);
                printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>",n,it[in].attrib[n][1]);
                printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>",n,it[in].attrib[n][2]);
                printf("</tr>");
        }

        printf("<tr><td>Hitpoints</td>");
        printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>",it[in].hp[0]);
        printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>",it[in].hp[1]);
        printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>",it[in].hp[2]);
        printf("</tr>");

        printf("<tr><td>Endurance</td>");
        printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>",it[in].end[0]);
        printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>",it[in].end[1]);
        printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>",it[in].end[2]);
        printf("</tr>");

        printf("<tr><td>Mana</td>");
        printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>",it[in].mana[0]);
        printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>",it[in].mana[1]);
        printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>",it[in].mana[2]);
        printf("</tr>");

        for (n=0; n<50; n++) {
                if (skilltab[n].name[0]==0) continue;
                printf("<tr><td>%s</td>",skilltab[n].name);
                printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>",n,it[in].skill[n][0]);
                printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>",n,it[in].skill[n][1]);
                printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>",n,it[in].skill[n][2]);
                printf("</tr>");
        }

        printf("<tr><td>Armor:</td><td><input type=text name=armor_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=armor_2 value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].armor[0],it[in].armor[1]);
        printf("<tr><td>Weapon:</td><td><input type=text name=weapon_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=weapon_2 value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].weapon[0],it[in].weapon[1]);

        printf("<tr><td>Gethit Damage:</td><td><input type=text name=gethit_dam_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=gethit_dam_2 value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].gethit_dam[0],it[in].gethit_dam[1]);

        printf("<tr><td>Max Age:</td><td><input type=text name=max_age_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=max_age_2 value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].max_age[0],it[in].max_age[1]);

        printf("<tr><td>Light</td><td><input type=text name=light_1 value=\"%d\" size=10 maxlength=10></td>",
                it[in].light[0]);
        printf("<td><input type=text name=light_2 value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].light[1]);

        printf("<tr><td valign=top>Sprite:</td><td><input type=text name=sprite_1 value=\"%d\" size=10 maxlength=10></td>",
                it[in].sprite[0]);
        printf("<td><input type=text name=sprite_2 value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].sprite[1]);

        printf("<tr><td>Animation-Status:</td><td><input type=text name=status_1 value=\"%d\" size=10 maxlength=10></td>",
                it[in].status[0]);
        printf("<td><input type=text name=status_2 value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].status[1]);

        printf("</table></td></tr>");

        printf("<tr><td>Max Damage:</td><td><input type=text name=max_damage value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].max_damage);

        printf("<tr><td>Duration:</td><td><input type=text name=duration value=\"%d\" size=10 maxlength=10> (in 1/18 of a sec)</td></tr>",
                it[in].duration);
        printf("<tr><td>Cost:</td><td><input type=text name=cost value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].cost);
        printf("<tr><td>Power:</td><td><input type=text name=power value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].power);
        printf("<tr><td>Sprite Overr.:</td><td><input type=text name=spr_ovr value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].sprite_override);
                
       	printf("<tr><td>Min. Rank:</td><td><input type=text name=min_rank value=\"%d\" size=10 maxlength=10></td></tr>",
                it[in].min_rank);

        printf("<tr><td valign=top>Driver:</td><td>");
        printf("<input type=text name=driver value=%d>",it[in].driver);
        printf("</td></tr>");

        printf("<tr><td valign=top>Driver Data:</td><td>");
        printf("The way the data is used depends on the driver type!<br>");

        for (n=0; n<10; n++) {
                printf("<tr><td>%s:</td><td><input type=text name=drdata%d value=\"%d\" size=10 maxlength=10></td></tr>",
                obj_drdata[n],n,it[in].data[n]);
        }

        printf("<tr><td><input type=submit value=Update></td><td> </td></tr>");
        printf("</table>");
        printf("<input type=hidden name=step value=24>");
        printf("<input type=hidden name=in value=%d>",in);
        printf("</form>");
        printf("<a href=/cgi-imp/acct.cgi?step=20>Back</a><br>");
}

void delete_character(LIST *head)
{
        int cn;
        char *tmp;

        tmp=find_val(head,"cn");
        if (tmp) cn=atoi(tmp);
        else { printf("CN not specified."); return; }

        ch[cn].used=USE_EMPTY;

        if (ch[cn].flags&CF_RESPAWN) globs->reset_char=cn;

        printf("Done.");
}

void delete_object(LIST *head)
{
        int in;
        char *tmp;

        tmp=find_val(head,"in");
        if (tmp) in=atoi(tmp);
        else { printf("IN not specified."); return; }

        it[in].used=USE_EMPTY;

        printf("Done.");
}

void update_character(LIST *head)
{
        int cn,cnt,val,n;
        unsigned long long lval;
        char *tmp,**tmps,buf[80];

        tmp=find_val(head,"cn");
        if (tmp) cn=atoi(tmp);
        else { printf("CN not specified."); return; }

        bzero(&ch[cn],sizeof(struct character));
        ch[cn].used=USE_ACTIVE;

        tmp=find_val(head,"name");
        if (tmp) { strncpy(ch[cn].name,tmp,35); ch[cn].name[35]=0; }
        else { printf("NAME not specified."); return; }

        tmp=find_val(head,"reference");
        if (tmp) { strncpy(ch[cn].reference,tmp,35); ch[cn].reference[35]=0; }
        else { printf("REFERENCE not specified."); return; }

        tmp=find_val(head,"description");
        if (tmp) { strncpy(ch[cn].description,tmp,195); ch[cn].description[195]=0; }
        else { printf("DESCRIPTION not specified."); return; }

        cnt=find_val_multi(head,"kindred",&tmps);
        if (cnt) {
                for (n=val=0; n<cnt; n++)
                        val|=atoi(tmps[n]);
                ch[cn].kindred=val;
        }

        tmp=find_val(head,"sprite");
        if (tmp) ch[cn].sprite=atoi(tmp);
        else { printf("SPRITE not specified."); return; }

        tmp=find_val(head,"sound");
        if (tmp) ch[cn].sound=atoi(tmp);
        else { printf("SOUND not specified."); return; }

        tmp=find_val(head,"class");
        if (tmp) ch[cn].class=atoi(tmp);
        else { printf("CLASS not specified."); return; }

        cnt=find_val_multi(head,"flags",&tmps);
        lval=0;
        if (cnt)
                for (n=0; n<cnt; n++)
                        lval|=atoll(tmps[n]);
        ch[cn].flags=lval;

        tmp=find_val(head,"alignment");
        if (tmp) ch[cn].alignment=atoi(tmp);
        else { printf("ALIGNMENT not specified."); return; }

        for (n=0; n<100; n++) {
                sprintf(buf,"drdata%d",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].data[n]=atoi(tmp);
                else ch[cn].data[n]=0;
        }

        for (n=0; n<10; n++) {
                sprintf(buf,"text_%d",n);
                tmp=find_val(head,buf);
                if (tmp) strncpy(ch[cn].text[n],tmp,158);
                else ch[cn].text[n][0]=0;
                ch[cn].text[n][159]=0;
        }

        for (n=0; n<5; n++) {
                sprintf(buf,"attrib%d_0",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].attrib[n][0]=atoi(tmp);
                else { printf("ATTRIB%d_0 not specified.",n); return; }

                sprintf(buf,"attrib%d_1",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].attrib[n][1]=atoi(tmp);
                else { printf("ATTRIB%d_1 not specified.",n); return; }

                sprintf(buf,"attrib%d_2",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].attrib[n][2]=atoi(tmp);
                else { printf("ATTRIB%d_2 not specified.",n); return; }

                sprintf(buf,"attrib%d_3",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].attrib[n][3]=atoi(tmp);
                else { printf("ATTRIB%d_3 not specified.",n); return; }
        }

        tmp=find_val(head,"hp_0");
        if (tmp) ch[cn].hp[0]=atoi(tmp);
        else { printf("HP_0 not specified."); return; }

        tmp=find_val(head,"hp_1");
        if (tmp) ch[cn].hp[1]=atoi(tmp);
        else { printf("HP_1 not specified."); return; }

        tmp=find_val(head,"hp_2");
        if (tmp) ch[cn].hp[2]=atoi(tmp);
        else { printf("HP_2 not specified."); return; }

        tmp=find_val(head,"hp_3");
        if (tmp) ch[cn].hp[3]=atoi(tmp);
        else { printf("HP_3 not specified."); return; }

        tmp=find_val(head,"end_0");
        if (tmp) ch[cn].end[0]=atoi(tmp);
        else { printf("END_0 not specified."); return; }

        tmp=find_val(head,"end_1");
        if (tmp) ch[cn].end[1]=atoi(tmp);
        else { printf("END_1 not specified."); return; }

        tmp=find_val(head,"end_2");
        if (tmp) ch[cn].end[2]=atoi(tmp);
        else { printf("END_2 not specified."); return; }

        tmp=find_val(head,"end_3");
        if (tmp) ch[cn].end[3]=atoi(tmp);
        else { printf("END_3 not specified."); return; }

        tmp=find_val(head,"mana_0");
        if (tmp) ch[cn].mana[0]=atoi(tmp);
        else { printf("MANA_0 not specified."); return; }

        tmp=find_val(head,"mana_1");
        if (tmp) ch[cn].mana[1]=atoi(tmp);
        else { printf("MANA_1 not specified."); return; }

        tmp=find_val(head,"mana_2");
        if (tmp) ch[cn].mana[2]=atoi(tmp);
        else { printf("MANA_2 not specified."); return; }

        tmp=find_val(head,"mana_3");
        if (tmp) ch[cn].mana[3]=atoi(tmp);
        else { printf("MANA_3 not specified."); return; }


        for (n=0; n<50; n++) {
                sprintf(buf,"skill%d_0",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].skill[n][0]=atoi(tmp);
                else ch[cn].skill[n][0]=0;

                sprintf(buf,"skill%d_1",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].skill[n][1]=atoi(tmp);
                else ch[cn].skill[n][1]=0;

                sprintf(buf,"skill%d_2",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].skill[n][2]=atoi(tmp);
                else ch[cn].skill[n][2]=0;

                sprintf(buf,"skill%d_3",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].skill[n][3]=atoi(tmp);
                else ch[cn].skill[n][3]=0;
        }

        tmp=find_val(head,"speed_mod");
        if (tmp) ch[cn].speed_mod=atoi(tmp);
        else { printf("SPEED_MOD not specified."); return; }

        tmp=find_val(head,"weapon_bonus");
        if (tmp) ch[cn].weapon_bonus=atoi(tmp);
        else { printf("WEAPON_BONUS not specified."); return; }

        tmp=find_val(head,"light_bonus");
        if (tmp) ch[cn].light_bonus=atoi(tmp);
        else { printf("LIGHT_BONUS not specified."); return; }

        tmp=find_val(head,"armor_bonus");
        if (tmp) ch[cn].armor_bonus=atoi(tmp);
        else { printf("ARMOR_BONUS not specified."); return; }

        tmp=find_val(head,"points");
        if (tmp) ch[cn].points=atoi(tmp);
        else { printf("POINTS not specified."); return; }

        tmp=find_val(head,"points_tot");
        if (tmp) ch[cn].points_tot=atoi(tmp);
        else { printf("POINTS_TOT not specified."); return; }

        tmp=find_val(head,"x");
        if (tmp) ch[cn].x=atoi(tmp);
        else { printf("X not specified."); return; }

        tmp=find_val(head,"y");
        if (tmp) ch[cn].y=atoi(tmp);
        else { printf("Y not specified."); return; }

        tmp=find_val(head,"dir");
        if (tmp) ch[cn].dir=atoi(tmp);
        else { printf("DIR not specified."); return; }

        tmp=find_val(head,"gold");
        if (tmp) ch[cn].gold=atoi(tmp);
        else { printf("GOLD not specified."); return; }

        for (n=0; n<20; n++) {
                sprintf(buf,"worn%d",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].worn[n]=atoi(tmp);
        }

        for (n=0; n<40; n++) {
                sprintf(buf,"item%d",n);
                tmp=find_val(head,buf);
                if (tmp) ch[cn].item[n]=atoi(tmp);
        }

        tmp=find_val(head,"citem");
        if (tmp) ch[cn].citem=atoi(tmp);
        else { printf("CITEM not specified."); return; }

        if (ch[cn].flags&CF_RESPAWN) globs->reset_char=cn;

        if (!ch[cn].data[29] && (ch[cn].flags&CF_RESPAWN)) ch[cn].data[29]=ch[cn].x+ch[cn].y*MAPX;

        printf("Done.");
}

void update_object(LIST *head)
{
        int in,cnt,val,n;
        unsigned long long lval;
        char *tmp,**tmps,buf[80];

        tmp=find_val(head,"in");
        if (tmp) in=atoi(tmp);
        else { printf("IN not specified."); return; }

        bzero(&it[in],sizeof(struct item));
        it[in].used=USE_ACTIVE;

        tmp=find_val(head,"name");
        if (tmp) { strncpy(it[in].name,tmp,35); it[in].name[35]=0; }
        else { printf("NAME not specified."); return; }

        tmp=find_val(head,"reference");
        if (tmp) { strncpy(it[in].reference,tmp,35); it[in].reference[35]=0; }
        else { printf("REFERENCE not specified."); return; }

        tmp=find_val(head,"description");
        if (tmp) { strncpy(it[in].description,tmp,195); it[in].description[195]=0; }
        else { printf("DESCRIPTION not specified."); return; }

        cnt=find_val_multi(head,"flags",&tmps);
        lval=0;
        if (cnt)
                for (n=0; n<cnt; n++)
                        lval|=atoll(tmps[n]);
        it[in].flags=lval;

        tmp=find_val(head,"driver");
        if (tmp) it[in].driver=atoi(tmp);
        else { printf("DRIVER not specified."); return; }

        for (n=0; n<100; n++) {
                sprintf(buf,"drdata%d",n);
                tmp=find_val(head,buf);
                if (tmp) it[in].data[n]=atoi(tmp);
        }

        tmp=find_val(head,"light_1");
        if (tmp) it[in].light[0]=atoi(tmp);
        else { printf("LIGHT_1 not specified."); return; }

        tmp=find_val(head,"light_2");
        if (tmp) it[in].light[1]=atoi(tmp);
        else { printf("LIGHT_2 not specified."); return; }

        tmp=find_val(head,"value");
        if (tmp) it[in].value=atoi(tmp);
        else { printf("VALUE not specified."); return; }

        cnt=find_val_multi(head,"placement",&tmps);
        if (cnt) {
                for (n=val=0; n<cnt; n++)
                        val|=atoi(tmps[n]);
                it[in].placement=val;
        }

        for (n=0; n<5; n++) {
                sprintf(buf,"attrib%d_0",n);
                tmp=find_val(head,buf);
                if (tmp) it[in].attrib[n][0]=atoi(tmp);
                else { printf("ATTRIB%d_0 not specified.",n); return; }

                sprintf(buf,"attrib%d_1",n);
                tmp=find_val(head,buf);
                if (tmp) it[in].attrib[n][1]=atoi(tmp);
                else { printf("ATTRIB%d_1 not specified.",n); return; }

                sprintf(buf,"attrib%d_2",n);
                tmp=find_val(head,buf);
                if (tmp) it[in].attrib[n][2]=atoi(tmp);
                else { printf("ATTRIB%d_2 not specified.",n); return; }
        }

        tmp=find_val(head,"hp_0");
        if (tmp) it[in].hp[0]=atoi(tmp);
        else { printf("HP_0 not specified."); return; }

        tmp=find_val(head,"hp_1");
        if (tmp) it[in].hp[1]=atoi(tmp);
        else { printf("HP_1 not specified."); return; }

        tmp=find_val(head,"hp_2");
        if (tmp) it[in].hp[2]=atoi(tmp);
        else { printf("HP_2 not specified."); return; }

        tmp=find_val(head,"end_0");
        if (tmp) it[in].end[0]=atoi(tmp);
        else { printf("END_0 not specified."); return; }

        tmp=find_val(head,"end_1");
        if (tmp) it[in].end[1]=atoi(tmp);
        else { printf("END_1 not specified."); return; }

        tmp=find_val(head,"end_2");
        if (tmp) it[in].end[2]=atoi(tmp);
        else { printf("END_2 not specified."); return; }

        tmp=find_val(head,"mana_0");
        if (tmp) it[in].mana[0]=atoi(tmp);
        else { printf("MANA_0 not specified."); return; }

        tmp=find_val(head,"mana_1");
        if (tmp) it[in].mana[1]=atoi(tmp);
        else { printf("MANA_1 not specified."); return; }

        tmp=find_val(head,"mana_2");
        if (tmp) it[in].mana[2]=atoi(tmp);
        else { printf("MANA_2 not specified."); return; }

        for (n=0; n<50; n++) {
                sprintf(buf,"skill%d_0",n);
                tmp=find_val(head,buf);
                if (tmp) it[in].skill[n][0]=atoi(tmp);
                else it[in].skill[n][0]=0;

                sprintf(buf,"skill%d_1",n);
                tmp=find_val(head,buf);
                if (tmp) it[in].skill[n][1]=atoi(tmp);
                else it[in].skill[n][1]=0;

                sprintf(buf,"skill%d_2",n);
                tmp=find_val(head,buf);
                if (tmp) it[in].skill[n][2]=atoi(tmp);
                else it[in].skill[n][2]=0;
        }

        tmp=find_val(head,"armor_1");
        if (tmp) it[in].armor[0]=atoi(tmp);
        else { printf("ARMOR_1 not specified."); return; }

        tmp=find_val(head,"armor_2");
        if (tmp) it[in].armor[1]=atoi(tmp);
        else { printf("ARMOR_2 not specified."); return; }

        tmp=find_val(head,"weapon_1");
        if (tmp) it[in].weapon[0]=atoi(tmp);
        else { printf("WEAPON_1 not specified."); return; }

        tmp=find_val(head,"weapon_2");
        if (tmp) it[in].weapon[1]=atoi(tmp);
        else { printf("WEAPON_2 not specified."); return; }

        tmp=find_val(head,"gethit_dam_1");
        if (tmp) it[in].gethit_dam[0]=atoi(tmp);
        else { printf("GETHIT_DAM_1 not specified."); return; }

        tmp=find_val(head,"gethit_dam_2");
        if (tmp) it[in].gethit_dam[1]=atoi(tmp);
        else { printf("GETHIT_DAM_2 not specified."); return; }

        tmp=find_val(head,"max_age_1");
        if (tmp) it[in].max_age[0]=atoi(tmp);
        else { printf("MAX_AGE_1 not specified."); return; }

        tmp=find_val(head,"max_age_2");
        if (tmp) it[in].max_age[1]=atoi(tmp);
        else { printf("MAX_AGE_2 not specified."); return; }

        tmp=find_val(head,"max_damage");
        if (tmp) it[in].max_damage=atoi(tmp);
        else { printf("MAX_DAMAGE not specified."); return; }

        tmp=find_val(head,"duration");
        if (tmp) it[in].duration=atoi(tmp);
        else { printf("DURATION not specified."); return; }

        tmp=find_val(head,"cost");
        if (tmp) it[in].cost=atoi(tmp);
        else { printf("COST not specified."); return; }

        tmp=find_val(head,"power");
        if (tmp) it[in].power=atoi(tmp);
        else { printf("POWER not specified."); return; }

        tmp=find_val(head,"spr_ovr");
        if (tmp) it[in].sprite_override=atoi(tmp);
        else { printf("SPR_OVR not specified."); return; }
        
        tmp=find_val(head,"min_rank");
        if (tmp) it[in].min_rank=atoi(tmp);
        else { printf("MIN_RANK not specified."); return; }

        tmp=find_val(head,"sprite_1");
        if (tmp) it[in].sprite[0]=atoi(tmp);
        else { printf("SPRITE_1 not specified."); return; }

        tmp=find_val(head,"sprite_2");
        if (tmp) it[in].sprite[1]=atoi(tmp);
        else { printf("SPRITE_2 not specified."); return; }

        tmp=find_val(head,"status_1");
        if (tmp) it[in].status[0]=atoi(tmp);
        else { printf("STATUS_1 not specified."); return; }

        tmp=find_val(head,"status_2");
        if (tmp) it[in].status[1]=atoi(tmp);
        else { printf("STATUS_2 not specified."); return; }

        globs->reset_item=in;

        printf("Done.");
}


void list_characters(LIST *head)	// excludes grolms, gargs, icegargs. decides by sprite-nr
{
        int n;

        printf("<table>");

        for (n=0; n<MAXTCHARS; n++) {
                if (ch[n].used==USE_EMPTY) continue;
	    if (ch[n].sprite==12240 || ch[n].sprite==18384 || ch[n].sprite==21456) continue; 
                printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
                        "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
                        n,n,
                        (ch[n].flags&CF_RESPAWN) ? "<b>" : "",
                        ch[n].name,
                        (ch[n].flags&CF_RESPAWN) ? "</b>" : "",
                        ch[n].x,ch[n].y,ch[n].points_tot>>10,ch[n].alignment,n,n);
        }

        printf("</table>");
}

void list_characters2(LIST *head)		// listing grolms,gargs,icegargs only, desides by sprite-nr
{
        int n;

        printf("<table>");

        for (n=0; n<MAXTCHARS; n++) {
                if (ch[n].used==USE_EMPTY) continue;
	    if (!(ch[n].sprite==12240 || ch[n].sprite==18384 || ch[n].sprite==21456)) continue;
                printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
                        "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
                        n,n,
                        (ch[n].flags&CF_RESPAWN) ? "<b>" : "",
                        ch[n].name,
                        (ch[n].flags&CF_RESPAWN) ? "</b>" : "",
                        ch[n].x,ch[n].y,ch[n].points_tot>>10,ch[n].alignment,n,n);
        }

        printf("</table>");
}

void list_new_characters(LIST *head)		// listing characters with high IDs
{
        int n;

        printf("<table>");

        for (n=1012; n<MAXTCHARS; n++) {
                if (ch[n].used==USE_EMPTY) continue;
                printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
                        "%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %dK</td><td>Alignment: %d</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
                        n,n,
                        (ch[n].flags&CF_RESPAWN) ? "<b>" : "",
                        ch[n].name,
                        (ch[n].flags&CF_RESPAWN) ? "</b>" : "",
                        ch[n].x,ch[n].y,ch[n].points_tot>>10,ch[n].alignment,n,n);
        }

        printf("</table>");
}

void list_objects(LIST *head)
{
        int n;

        printf("<table>");

        for (n=1; n<MAXTITEM; n++) {
                if (it[n].used==USE_EMPTY) continue;
//                if (it[n].driver!=23) continue;
//		if (!(it[n].flags&IF_TAKE)) continue;

                printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=23&in=%d>%30.30s</a></td>"
                        "<td>price: %dG, %dS</td><td>data4: %d</td>"
                        "<td><a href=/cgi-imp/acct.cgi?step=25&in=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=22&in=%d>Delete</a></td></tr>\n",
                        n,n,it[n].name,
                        it[n].value/100,it[n].value%100,it[n].data[4],
                        n,n);
        }

        printf("</table>");
}

void list_object_drivers(LIST *head)
{
        int n, nd;
        int ndrivers = 0;
        int found;

        // Find out how many drivers there are
        for (n=1; n<MAXTITEM; n++) {
                if (it[n].driver > ndrivers) ndrivers = it[n].driver;
        }
        printf("<ul compact>\n");
        for (nd=1; nd<=ndrivers; nd++) {
                found = 0;
                for (n=1; n<MAXTITEM; n++) {
                        if (it[n].driver == nd) {
                                if (!found) {
                                        printf("<li>Driver #%d:"
                                               "<dir compact>", nd);
                                        found = 1;
                                }
                                printf("<li>"
                                       "<a href=/cgi-imp/acct.cgi?step=23&in=%d>%d:</a>"
                                       "&nbsp;&nbsp;&nbsp;%s"
                                       "\n</li>\n", n, n, it[n].name);
                        }
                }
                if (found) printf("</dir></li>");
        }
        printf("\n</ul>\n");
}

int main(int argc, char *args[])
{
        int step=0;
        int n __attribute__ ((unused));
        char *tmp;
        LIST *head;
        head = is_form_empty() ? NULL : cgi_input_parse();

        chdir("/home/merc");

        printf("Content-Type: text/html\n\n");
        printf("<html><head><title>World Builder</title><META HTTP-EQUIV=\"PRAGMA\" CONTENT=\"NO-CACHE\"></head>\n");
        #if defined(SUSE)
        printf("<BODY TEXT=#D7D700 BGCOLOR=#264A9F LINK=#FFFFBB VLINK=#CCCC00 ALINK=#FFFF9D");
        #else
        printf("<BODY TEXT=#D7D700 BGCOLOR=#264A9F LINK=#FFFFBB VLINK=#CCCC00 ALINK=#FFFF9D background=/gfx/back4.gif>");
        #endif
        printf("<center>");
        printf("<table width=\"100%%\"><tr>");
        printf("<td align=center>");
        printf("<a href=\"http://www.astonia.com/\"><img src=\"/gfx/logo.gif\" width=100 height=60 border=0></a>");
        printf("</td><td align=center>");
        printf("<h2>Server Info</h2>\n");
        printf("</td><td align=center>");
        printf("<a href=\"http://www.astonia.com/\"><img src=\"/gfx/logo.gif\" width=100 height=60 border=0></a>");
        printf("</td></tr></table>");
        printf("<img src=/gfx/barsmall.gif border=0 align=left alt=---- width=100%% height=5><br>");
        printf("<table width=100%%><tr><td>\n");

        load();
        
        #ifdef RESET_CHAR_KLUDGE	
        n=atoi(args[1]);
        globs->reset_char=n;
	unload();
                exit(0);
        #endif

       	if (head) {
                tmp=find_val(head,"step");
                if (tmp) step=atoi(tmp);
        }

        ch[1].used=USE_ACTIVE;
        strcpy(ch[1].name,"Blank Template");

        it[1].used=USE_ACTIVE;
        strcpy(it[1].name,"Blank Template");

        /* for (n=1; n<MAXTCHARS; n++) {
                if (ch[n].used==USE_EMPTY) continue;
        } */

/*      for (n=1; n<MAXTITEM; n++) {
                if (it[n].used==USE_EMPTY) continue;
                it[n].value/=5;
        } */

        switch(step) {
                case    11:     list_characters(head); break;
                case    12:     delete_character(head); break;
                case    13:     view_character(head); break;
                case    14:     update_character(head); break;
                case    15:     copy_character(head); break;

                case    21:     list_objects(head); break;
                case    22:     delete_object(head); break;
                case    23:     view_object(head); break;
                case    24:     update_object(head); break;
                case    25:     copy_object(head); break;

                case    31:     list_object_drivers(head); break;
                case    41:     list_characters2(head); break;
			    case    51:     list_new_characters(head); break;


                default:
						printf("Together those lists include all character-templates<br>");
				        printf("<a href=/cgi-imp/acct.cgi?step=11>Characters (without Grolms, Gargoyles, Icegargs)</a><br>");
						printf("<a href=/cgi-imp/acct.cgi?step=41>Characters (only    Grolms, Gargoyles, Icegargs) </a><br><br>");
						printf("This list includes only characters with high IDs for fast access<br>");
						printf("<a href=/cgi-imp/acct.cgi?step=51>New characters (only if they got a high ID)</a><br><br>");
				        printf("<a href=/cgi-imp/acct.cgi?step=21>Object Templates</a><br>");
				        printf("<a href=/cgi-imp/acct.cgi?step=31>Object Driver List</a><br>");
                        break;
        }

        unload();

        printf("</td></tr></table>");

        printf("<img src=/gfx/barsmall.gif border=0 align=left alt=---- width=100%% height=5><br>");
        printf("<table width=\"100%%\"><tr>");
        if (step>11 && step<20) printf("<td align=center><a href=/cgi-imp/acct.cgi?step=11>Back to main page</a></td>");
        else if (step>21 && step<30) printf("<td align=center><a href=/cgi-imp/acct.cgi?step=21>Back to main page</a></td>");
        else printf("<td align=center><a href=/cgi-imp/acct.cgi>Back to main page</a></td>");
        printf("<td align=right><table><tr><td>");

        printf("</td></tr></table></td></tr></table>");
        printf("<img src=/gfx/barsmall.gif border=0 align=left alt=---- width=100%% height=5><br>");
        printf("<font size=-1>All material on this server is copyright <a href=mailto:joker@astonia.com>D.Brockhaus</a></font></center>");
        printf("</body></html>");


        return 0;
}
