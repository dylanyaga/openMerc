/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#define _XOPEN_SOURCE
#define __USE_XOPEN
#include <unistd.h>
#include <math.h>

#include "server.h"

#define KILLERONLY

#define AREASIZE        12
#define SPIRALSIZE      ((2*AREASIZE+1)*(2*AREASIZE+1))

int do_is_ignore(int cn,int co,int flag);
void do_look_player_depot(int cn,char *cv);
void do_look_player_inventory(int cn,char *cv);
void do_look_player_equipment(int cn,char *cv);
void do_steal_player(int cn, char *cv, char *ci);

/* CS, 991113: Support for outwardly spiralling area with a single loop */
int areaspiral[SPIRALSIZE] = {0, 0};

/* This routine initializes areaspiral[] with a set of offsets from a given location
   that form a spiral starting from a central point, where the offset is 0. */
void initspiral() {
        int j, dist;
        int point = 0; // offset in array

        areaspiral[point] = 0;
        for (dist=1; dist<=AREASIZE; dist++) {
                                         areaspiral[++point] = -MAPX; // N
                for (j=2*dist-1; j; j--) areaspiral[++point] = -1;    // W
                for (j=2*dist; j; j--) areaspiral[++point]   = MAPX;  // S
                for (j=2*dist; j; j--) areaspiral[++point]   = 1;     // E
                for (j=2*dist; j; j--) areaspiral[++point]   = -MAPX; // N
        }
}


void do_area_log(int cn,int co,int xs,int ys,int font,char *format,...) // cn,co are the only ones NOT to get the message
{
        int x,y,cc,m;
        va_list args;
        char buf[1024];
        unsigned long long prof;

        prof=prof_start();

        va_start(args,format);
        vsprintf(buf,format,args);
        va_end(args);

        for (y=max(0,ys-12); y<min(MAPY,ys+13); y++) {
                m=y*MAPX;
                for (x=max(0,xs-12); x<min(MAPX,xs+13); x++) {
                        if ((cc=map[x+m].ch)!=0) {
                                if (cc!=cn && cc!=co) {
                                        if (!ch[cc].player && ch[cc].temp!=15) continue;
                                        do_log(cc,font,buf);
                                }
                        }
                }
        }
        prof_stop(2,prof);
}

/* CS, 991113: Respect invisibility in 'say'. */
void do_area_say1(int cn, int xs, int ys, char *msg)
{
        char msg_named[500], msg_invis[500];
        int invis = 0;
        int npcs[20], cnt = 0;
        int j, m, cc;
        unsigned long long prof;

        prof=prof_start();

        sprintf(msg_named, "%.30s: \"%.300s\"\n", ch[cn].name, msg);
        if (IS_INVISIBLE(cn)) {
                invis = 1;
                sprintf(msg_invis, "Somebody says: \"%.300s\"\n", msg);
        }
        if (areaspiral[1] == 0) initspiral();
        m = XY2M(xs,ys); // starting point
        for (j=0; j<SPIRALSIZE; j++) {
                m += areaspiral[j];
                if (m < 0 || m >= (MAPX*MAPY)) continue;
                cc = map[m].ch;
                if (!IS_SANECHAR(cc)) continue;
                if ((ch[cc].flags & (CF_PLAYER|CF_USURP))) {    // listener is a player
                        if (!invis || invis_level(cn) <= invis_level(cc)) {
                                do_log(cc, 3, msg_named); // talker visible to listener
                        } else {
                                do_log(cc, 3, msg_invis); // talker invis
                        }
                } else { // listener is NPC: Store in list for second pass
                        // DB: note: this should be changed for staff/god NPCs
                        if (!invis && cnt<ARRAYSIZE(npcs)) { // NPCs pretend not to hear invis people
                                if (j < 169) { // don't address mobs outside radius 6
                                        npcs[cnt++] = cc;
                                }
                        }
                }
        }
        for (j=0; j<cnt; j++)
                if (do_char_can_see(npcs[j],cn)) npc_hear(npcs[j],cn,msg);
               prof_stop(3,prof);
}

void do_area_sound(int cn,int co,int xs,int ys,int nr)
{
        int x,y,cc,s,m;
        int xvol,xpan;
        unsigned long long prof;

        prof=prof_start();
        for (y=max(0,ys-8); y<min(MAPY,ys+9); y++) {
                m=y*MAPX;
                for (x=max(0,xs-8); x<min(MAPX,xs+9); x++) {
                        if ((cc=map[x+m].ch)!=0) {
                                if (cc!=cn && cc!=co) {
                                        if (!ch[cc].player) continue;
                                        s=ys-y+xs-x;
                                        if (s<0) xpan=-500;
                                        else if (s>0) xpan=500;
                                        else xpan=0;

                                        s=((ys-y)*(ys-y)+(xs-x)*(xs-x))*30;

                                        xvol=-150-s;
                                        if (xvol<-5000) xvol=-5000;
                                        char_play_sound(cc,nr,xvol,xpan);
                                }
                        }
                }
        }
        prof_stop(4,prof);
}

void do_notify_char(int cn,int type,int dat1,int dat2,int dat3,int dat4)
{
        driver_msg(cn,type,dat1,dat2,dat3,dat4);
}

void do_area_notify(int cn,int co,int xs,int ys,int type,int dat1,int dat2,int dat3,int dat4)
{
        int x,y,cc,m;
        unsigned long long prof;

        prof=prof_start();

        for (y=max(0,ys-AREASIZE); y<min(MAPY,ys+AREASIZE+1); y++) {
                m=y*MAPX;
                for (x=max(0,xs-AREASIZE); x<min(MAPX,xs+AREASIZE+1); x++) {
                        if ((cc=map[x+m].ch)!=0) {
                                if (cc!=cn && cc!=co) {
                                        do_notify_char(cc,type,dat1,dat2,dat3,dat4);
                                }
                        }
                }
        }
        prof_stop(5,prof);
}

// use this one sparingly! It uses quite a bit of computation time!
/* This routine finds the 3 closest NPCs to the one doing the shouting,
   so that they can come to the shouter's rescue or something. */
void do_npc_shout(int cn,int type,int dat1,int dat2,int dat3,int dat4)
{
        int co,dist;
        int best[3]={99,99,99},bestn[3]={0,0,0};
        unsigned long long prof;

        prof=prof_start();
        if (ch[cn].data[52]==3) {

                for (co=1; co<MAXCHARS; co++) {
                        if (co!=cn && ch[co].used==USE_ACTIVE && !(ch[co].flags&CF_BODY)) {
                                if (ch[co].flags&(CF_PLAYER|CF_USURP)) continue;
                                if (ch[co].data[53]!=ch[cn].data[52]) continue;
                                dist=abs(ch[cn].x-ch[co].x)+abs(ch[cn].y-ch[co].y);
                                if (dist<best[0]) {
                                        best[2]=best[1]; best[1]=best[0];
                                        bestn[2]=bestn[1]; bestn[1]=bestn[0];
                                        best[0]=dist;
                                        bestn[0]=co;
                                } else if (dist<best[1]) {
                                        best[2]=best[1]; bestn[2]=bestn[1];
                                        best[1]=dist; bestn[1]=co;
                                } else if (dist<best[3]) {
                                        best[3]=dist; bestn[3]=co;
                                }
                        }
                }

                if (bestn[0]) do_notify_char(bestn[0],type,dat1,dat2,dat3,dat4);
                if (bestn[1]) do_notify_char(bestn[1],type,dat1,dat2,dat3,dat4);
                if (bestn[2]) do_notify_char(bestn[2],type,dat1,dat2,dat3,dat4);
        } else {
                for (co=1; co<MAXCHARS; co++) {
                        if (co!=cn && ch[co].used==USE_ACTIVE && !(ch[co].flags&CF_BODY)) {
                                if (ch[co].flags&(CF_PLAYER|CF_USURP)) continue;
                                if (ch[co].data[53]!=ch[cn].data[52]) continue;
                                do_notify_char(co,type,dat1,dat2,dat3,dat4);
                        }
                }
        }
        prof_stop(6,prof);
}

void do_log(int cn,int font,char *text)
{
        int n=0,len,nr;
        unsigned char buf[16];

        nr=ch[cn].player;
        if (nr<1 || nr>=MAXPLAYER) return;

        if (player[nr].usnr!=cn) {
                ch[cn].player=0;
                return;
        }

        len=strlen(text)-1;

        while (n<=len) {
                buf[0]=SV_LOG+font;
                memcpy(buf+1,text+n,15); // possible bug: n+15>textend !!!
                xsend(ch[cn].player,buf,16);

                n+=15;
        }
}

void do_char_log(int cn,int font,char *format,...)
{
        va_list args;
        char buf[1024];

        if (!ch[cn].player && ch[cn].temp!=15) return;

        va_start(args,format);
        vsprintf(buf,format,args);
        do_log(cn,font,buf);
        va_end(args);
}

void do_staff_log(int font,char *format,...)
{
        va_list args;
        char buf[1024];
        int n;

        va_start(args,format);
        vsprintf(buf,format,args);

        for (n=1; n<MAXCHARS; n++) {
                if (ch[n].player && (ch[n].flags&(CF_STAFF|CF_IMP|CF_USURP)) && !(ch[n].flags&CF_NOSTAFF))
                        do_log(n,font,buf);
        }
        va_end(args);
}

/* CS, 991113: #NOWHO and visibility levels */
/* A level conscious variant of do_staff_log() */
void do_admin_log(int source, char *format,...)
{
        va_list args;
        char buf[1024];
        int n;

        va_start(args,format);
        vsprintf(buf,format,args);

        for (n=1; n<MAXCHARS; n++) {
                /* various tests to exclude listeners (n) */
                if (!ch[n].player) continue; // not a player
                if (!(ch[n].flags & (CF_STAFF|CF_IMP|CF_USURP))) continue; // only to staffers and QMs
                if ((ch[source].flags & (CF_INVISIBLE|CF_NOWHO)) && // privacy wanted
                    invis_level(source) > invis_level(n)) continue; // and source outranks listener

                do_log(n, 2, buf);
        }
        va_end(args);
}

/* CS, 991205: Players see new arrivals */
/* Announcement to all players */
void do_announce(int source, int author, char *format,...)
{
        va_list args;
        char buf_anon[1024];
        char buf_named[1024];
        int n;

        if (!*format) return;

        va_start(args,format);
        vsprintf(buf_anon,format,args);
        if (author) {
                sprintf(buf_named, "[%s] %s", ch[author].name, buf_anon);
        } else {
                strcpy(buf_named, buf_anon);
        }

        for (n=1; n<MAXCHARS; n++) {
                /* various tests to exclude listeners (n) */
                if (!ch[n].player && ch[n].temp!=15) continue; // not a player
                if ((ch[source].flags & (CF_INVISIBLE|CF_NOWHO)) && // privacy wanted
                    invis_level(source) > invis_level(n)) continue; // and source outranks listener
                if ((source != 0) && (invis_level(source) <= invis_level(n))) {
                        do_log(n, 2, buf_named);
                } else {
                        do_log(n, 2, buf_anon);
                }
        }
        va_end(args);
}

void do_caution(int source, int author, char *format,...)
{
        va_list args;
        char buf_anon[1024];
        char buf_named[1024];
        int n;

        if (!*format) return;

        va_start(args,format);
        vsprintf(buf_anon,format,args);
        if (author) {
                sprintf(buf_named, "[%s] %s", ch[author].name, buf_anon);
        } else {
                strcpy(buf_named, buf_anon);
        }

        for (n=1; n<MAXCHARS; n++) {
                /* various tests to exclude listeners (n) */
                if (!ch[n].player && ch[n].temp!=15) continue; // not a player
                if ((ch[source].flags & (CF_INVISIBLE|CF_NOWHO)) && // privacy wanted
                    invis_level(source) > invis_level(n)) continue; // and source outranks listener
                if ((source != 0) && (invis_level(source) <= invis_level(n))) {
			do_log(n, 0, buf_named);
		} else {
			do_log(n, 0, buf_anon);
                }
        }
        va_end(args);
}

void do_imp_log(int font,char *format,...)
{
        va_list args;
        char buf[1024];
        int n;

        va_start(args,format);
        vsprintf(buf,format,args);

        for (n=1; n<MAXCHARS; n++) {
                if (ch[n].player && (ch[n].flags&(CF_IMP|CF_USURP)))
			do_log(n,font,buf);
        }
        va_end(args);
}

/* CS, 991204: Match on partial names */
int do_lookup_char(char *name)
{
        int n;
        char matchname[100];
        int len;
        int bestmatch = 0;
        int quality = 0; // 1 = npc 2 = inactive plr 3 = active plr

        len = strlen(name);
        if (len < 2) return 0;
        sprintf(matchname, "%-.90s", name);

        for (n=1; n<MAXCHARS; n++) {
                if (ch[n].used!=USE_ACTIVE && ch[n].used!=USE_NONACTIVE) continue;
                if (ch[n].flags&CF_BODY) continue;
                if (strncasecmp(ch[n].name, matchname, len)) continue;
                if (strlen(ch[n].name) == len) { // perfect match
                        bestmatch = n;
                        break;
                }
                if (ch[n].flags & (CF_PLAYER|CF_USURP)) {
                        if (ch[n].x != 0) { // active plr
                                if (quality < 3) {
                                        bestmatch = n;
                                        quality = 3;
                                }
                        } else { // inactive plr
                                if (quality < 2) {
                                        bestmatch = n;
                                        quality = 2;
                                }
                        }
                } else { // NPC
                        if (quality < 1) {
                                bestmatch = n;
                                quality = 1;
                        }
                }
        }
        return bestmatch;
}

/* look up a character by name.
   special case "self" returns the looker. */
int do_lookup_char_self(char *name, int cn)
{
        if (!strcasecmp(name, "self")) return cn;
        return do_lookup_char(name);
}

int do_is_ignore(int cn,int co,int flag)
{
        int n;

        if (!flag)
                for (n=30; n<39; n++)
                        if (ch[co].data[n]==cn) return 1;


        for (n=50; n<59; n++)
                if (ch[co].data[n]==cn) return 1;

        return 0;
}

void do_tell(int cn,char *con,char *text)
{
        int co;
        char buf[256];

        if (ch[cn].flags&CF_SHUTUP) {
                do_char_log(cn,0,"You try to speak, but you only produce a croaking sound.\n");
                return;
        }

        co=do_lookup_char(con);
        if (!co) {
                do_char_log(cn,0,"Unknown name: %s\n",con);
                return;
        }
        if (!(ch[co].flags&(CF_PLAYER)) || ch[co].used!=USE_ACTIVE || ((ch[co].flags&CF_INVISIBLE) && invis_level(cn)<invis_level(co)) ||
            (!(ch[cn].flags&CF_GOD) && ((ch[co].flags&CF_NOTELL) || do_is_ignore(cn,co,0)))) {
                do_char_log(cn,0,"%s is not listening\n",ch[co].name);
                return;
        }

        /* CS, 991127: Support for AFK <message> */
        if (ch[co].data[0]) {
                if (ch[co].text[0][0]) {
                        do_char_log(cn,0,"%s is away from keyboard; Message:\n",ch[co].name);
                        do_char_log(cn,3,"  \"%s\"\n", ch[co].text[0]);
                } else {
                        do_char_log(cn,0,"%s is away from keyboard.\n",ch[co].name);
                }
        }

        if (!text) {
                do_char_log(cn,0,"I understand that you want to tell %s something. But what?\n",ch[co].name);
                return;
        }

        if ((ch[cn].flags&CF_INVISIBLE) && invis_level(cn)>invis_level(co)) sprintf(buf,"Somebody tells you: \"%.200s\"\n",text);
        else sprintf(buf,"%s tells you: \"%.200s\"\n",ch[cn].name,text);
        do_char_log(co,3,"%s",buf);

	if (ch[co].flags&CF_CCP) ccp_tell(co,cn,text);

        do_char_log(cn,1,"Told %s: \"%.200s\"\n",ch[co].name,text);

        if (cn==co) do_char_log(cn,1,"Do you like talking to yourself?\n");

        if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Told %s: \"%s\"",ch[co].name,text);
}

void do_notell(int cn)
{
        ch[cn].flags^=CF_NOTELL;

        if (ch[cn].flags&CF_NOTELL) do_char_log(cn,1,"You will no longer hear people #tell you something.\n");
        else do_char_log(cn,1,"You will hear if people #tell you something.\n");

        if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Set notell to %s",(ch[cn].flags&CF_NOTELL) ? "on" : "off");
}

void do_noshout(int cn)
{
        ch[cn].flags^=CF_NOSHOUT;

        if (ch[cn].flags&CF_NOSHOUT) do_char_log(cn,1,"You will no longer hear people #shout.\n");
        else do_char_log(cn,1,"You will hear people #shout.\n");

        if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Set noshout to %s",(ch[cn].flags&CF_NOSHOUT) ? "on" : "off");
}

void do_shout(int cn,char *text)
{
        char buf[256];
        int n;

        if (!text) {
                do_char_log(cn,0,"Shout. Yes. Shout it will be. But what do you want to shout?\n");
                return;
        }

        if (ch[cn].a_end<50000) {
                do_char_log(cn,0,"You're too exhausted to shout!\n");
                return;
        }

        if (ch[cn].flags&CF_SHUTUP) {
                do_char_log(cn,0,"You try to shout, but you only produce a croaking sound.\n");
                return;
        }

        ch[cn].a_end-=50000;

        if (ch[cn].flags&CF_INVISIBLE) sprintf(buf,"Somebody shouts: \"%.200s\"\n",text);
        else sprintf(buf,"%.30s shouts: \"%.200s\"\n",ch[cn].name,text);

        for (n=1; n<MAXCHARS; n++) {
                if (((ch[n].flags&(CF_PLAYER|CF_USURP)) || ch[n].temp==15) && ch[n].used==USE_ACTIVE && ((!(ch[n].flags&CF_NOSHOUT) && !do_is_ignore(cn,n,0)) || (ch[cn].flags&CF_GOD))) {
                        do_char_log(n,3,"%s",buf);
                        if (ch[n].flags&CF_CCP) ccp_shout(n,cn,text);
                }
        }

        if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Shouts \"%s\"",text);

}

void do_itell(int cn,char *text)
{
        int co;

        if (!text) {
                do_char_log(cn,0,"Imp-Tell. Yes. imp-tell it will be. But what do you want to tell the other imps?\n");
                return;
        }

        if (ch[cn].flags&CF_SHUTUP) {
                do_char_log(cn,0,"You try to imp-tell, but you only produce a croaking sound.\n");
                return;
        }

        if ((ch[cn].flags&CF_USURP) && IS_SANECHAR(co=ch[cn].data[97])) do_imp_log(2,"%.30s (%.30s) imp-tells: \"%.170s\"\n",ch[cn].name,ch[co].name,text);
        else do_imp_log(2,"%.30s imp-tells: \"%.200s\"\n",ch[cn].name,text);

        if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"imp-tells \"%s\"",text);

}

void do_stell(int cn,char *text)
{
        if (!text) {
                do_char_log(cn,0,"Staff-Tell. Yes. staff-tell it will be. But what do you want to tell the other staff members?\n");
                return;
        }

        if (ch[cn].flags&CF_SHUTUP) {
                do_char_log(cn,0,"You try to staff-tell, but you only produce a croaking sound.\n");
                return;
        }

        do_staff_log(2,"%.30s staff-tells: \"%.200s\"\n",ch[cn].name,text);

        if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"staff-tells \"%s\"",text);

}

void do_nostaff(int cn)
{
	    ch[cn].flags^=CF_NOSTAFF;

	    if (ch[cn].flags&CF_NOSTAFF) do_char_log(cn,1,"You will no longer hear people using #stell.\n");
	    else do_char_log(cn,1,"You will hear people using #stell.\n");

	    if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Set nostaff to %s",(ch[cn].flags&CF_NOSTAFF) ? "on" : "off");
}

/* Group tell */
void do_gtell(int cn, char *text)
{
        int n, co, found=0;

        if (!text) {
                do_char_log(cn,0,"Group-Tell. Yes. group-tell it will be. But what do you want to tell the other group members?\n");
                return;
        }

        if (ch[cn].flags&CF_SHUTUP) {
                do_char_log(cn,0,"You try to group-tell, but you only produce a croaking sound.\n");
                return;
        }

        for (n=CHD_MINGROUP; n<=CHD_MAXGROUP; n++) {
                if ((co=ch[cn].data[n])) {
                        if (!isgroup(co, cn)) {
                                ch[cn].data[n] = 0;     // throw out defunct group member
                        } else {
                                do_char_log(co, 2, "%s group-tells: \"%s\"\n", ch[cn].name, text);
                                found = 1;
                        }
                }
        }
        if (found) {
                do_char_log(cn, 2, "Told the group: \"%s\"\n", text);
                if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"group-tells \"%s\"",text);
        } else {
                do_char_log(cn, 0, "You don't have a group to talk to!\n");
        }
}

void do_help(int cn,char *topic)
{
	//             "!        .         .         .         .        !"
	do_char_log(cn,1,"The following commands are available:\n");
	do_char_log(cn,1," \n");
	do_char_log(cn,1,"#afk <message>         away from keyboard.\n");
	do_char_log(cn,1,"#allow <player>        to access your grave.\n");
	do_char_log(cn,1,"#bow                   you'll bow.\n");
	do_char_log(cn,1,"#fightback             toggle auto-figtback.\n");
	do_char_log(cn,1,"#follow <player>|self  you'll follow player.\n");
	do_char_log(cn,1,"#gold <amount>         get X gold coins.\n");
	do_char_log(cn,1,"#group <player>        group with player.\n");
	do_char_log(cn,1,"#gtell <message        tell to your group.\n");
	do_char_log(cn,1,"#ignore <player>       ignore that player.\n");
	do_char_log(cn,1,"#iignore <player>      ignore normal talk too.\n");
	do_char_log(cn,1,"#lag <seconds>         lag control.\n");
	do_char_log(cn,1,"#noshout               you won't hear shouts.\n");
	do_char_log(cn,1,"#notell                you won't hear tells.\n");
	do_char_log(cn,1,"#seen <player>         when last seen here?.\n");
	do_char_log(cn,1,"#shout <text>          to all players.\n");
	do_char_log(cn,1,"#skua                  leave purple.\n");
	do_char_log(cn,1,"#sort <order>          sort inventory.\n");
	do_char_log(cn,1,"#spellignore           don't attack if spelled.\n");
	do_char_log(cn,1,"#tell <player> <text>  tells player text.\n");
	do_char_log(cn,1,"#wave                  you'll wave.\n");
	do_char_log(cn,1,"#who                   see who's online.\n");
	do_char_log(cn,1," \n");

	if (ch[cn].flags&(CF_POH_LEADER)) {
		do_char_log(cn,1,"#poh <player>          add player to POH.\n");
		do_char_log(cn,1,"#pol <player>          make plr POH leader.\n");
	}

	do_char_log(cn,1,"Most of the commands are toggles. That is, use the same command again to turn off the effect. You can replace the '#' with '/'.\n");
	do_char_log(cn,1," \n");

	if (ch[cn].flags&(CF_STAFF|CF_IMP|CF_USURP)) {
		do_char_log(cn,2,"Staff Commands:\n");
		do_char_log(cn,2," \n");
		do_char_log(cn,2,"#announce <message>    broadcast IMPORTANT msg.\n");
		do_char_log(cn,2,"#caution <text>        warn the population.\n");
		do_char_log(cn,2,"#info <player>         identify player.\n");
		do_char_log(cn,2,"#look <player>         look at player.\n");
		do_char_log(cn,2,"#stell <text>          tell all staff members.\n");
		do_char_log(cn,2," \n");
	}

	if (ch[cn].flags&(CF_IMP|CF_USURP)) {
		do_char_log(cn,3,"Imp Commands:\n");
		do_char_log(cn,3," \n");
		do_char_log(cn,3,"#addban <player>       add plr to ban list.\n");
		do_char_log(cn,3,"#delban <lineno>       del plr from ban list.\n");
		do_char_log(cn,3,"#enemy <NPC><char>     make NPC fight char.\n");
		do_char_log(cn,3,"#enter                 fake enter the game.\n");
		do_char_log(cn,3,"#exit                  return from #USURP.\n");
		do_char_log(cn,3,"#force <char><text>    make him act.\n");
		//do_char_log(cn,3,"#gargoyle              turn self into a garg.\n");
		do_char_log(cn,3,"#goto <char>           go to char.\n");
		do_char_log(cn,3,"#goto <x> <y>          goto x,y.\n");
		do_char_log(cn,3,"#goto n|e|s|w <nnn>    goto <nnn> in dir.\n");
		//do_char_log(cn,3,"#grolm                 turn self into a grolm.\n");
		do_char_log(cn,3,"#itell <text>          tell all imps.\n");
		do_char_log(cn,3,"#kick <player>         kick player out.\n");
		do_char_log(cn,3,"#leave                 fake leave the game.\n");
		do_char_log(cn,3,"#listban               show ban list.\n");
		do_char_log(cn,3,"#look <player>         look at player.\n");
		do_char_log(cn,3,"#luck <player> <val>   set players luck.\n");
		do_char_log(cn,3,"#mark <player> <text>  mark a player with notes.\n");
		do_char_log(cn,3,"#name <name> <N.Name>  change chars(npcs) names.\n");
		do_char_log(cn,3,"#nodesc <player>       remove description.\n");
		do_char_log(cn,3,"#nolist <player>       exempt from top 10.\n");
		do_char_log(cn,3,"#nostaff               you won't hear #stell.\n");
		do_char_log(cn,3,"#nowho <player>        not listed in who.\n");
		do_char_log(cn,3,"#npclist <search>      display list of NPCs.\n");
		do_char_log(cn,3,"#raise <player> <exp>  give player exps.\n");
		do_char_log(cn,3,"#respawn <temp-id>     make npcs id respawn.\n");
		do_char_log(cn,3,"#shutup <player>       make unable to talk.\n");
		do_char_log(cn,3,"#slap <player>         slap in face.\n");
		do_char_log(cn,3,"#sprite <player>       change a player's sprite.\n");
		//do_char_log(cn,3,"#summon <name> [<rank> [<which>]]\n");
		do_char_log(cn,3,"#thrall <name> [<rank>] clone slave.\n");
		do_char_log(cn,3,"#usurp <ID>            turn self into ID.\n");
		do_char_log(cn,3,"#write <text>          make scrolls with text.\n");
		do_char_log(cn,3," \n");
	}
	if (ch[cn].flags&(CF_GOD)) {
		do_char_log(cn,3,"God Commands:\n");
		do_char_log(cn,3," \n");
		//do_char_log(cn,3,"#build <template>       build mode.\n");
		do_char_log(cn,3,"#create <item template> creating items.\n");
		//do_char_log(cn,3,"#creator <player>       make player a Creator.\n");
		do_char_log(cn,3,"#ggold <amount>         give money to a player.\n");
		do_char_log(cn,3,"#god <player>           make player a God.\n");
		do_char_log(cn,3,"#imp <player> <amount>  make player an Imp.\n");
		do_char_log(cn,3,"#mailpass <player>      send passwd to admin.\n");
		do_char_log(cn,3,"#password <name>        change a plr's passwd.\n");
		do_char_log(cn,3,"#perase <player>        total player erase.\n");
		do_char_log(cn,3,"#pol <player>           make player POH leader.\n");
		//do_char_log(cn,3,"#race <player> <temp>   new race for a player(avoid).\n");
		do_char_log(cn,3,"#send <player> <target> teleport player to target.\n");
		do_char_log(cn,3,"#staff <player>         make a player staffer.\n");
		do_char_log(cn,3,"#summon <name> [<rank> [<which>]]\n");
		do_char_log(cn,3,"#tavern                 log off quickly.\n");
		do_char_log(cn,3," \n");
	}
	if (ch[cn].flags&(CF_GREATERGOD)) {
		do_char_log(cn,3,"Greater God Commands:\n");
		do_char_log(cn,3," \n");
		do_char_log(cn,3,"#build <template>       build mode.\n");
                do_char_log(cn,3,"#creator <player>       make player a Creator.\n");
                do_char_log(cn,3,"#greatergod <player>    make player a G-God.\n");
                do_char_log(cn,3,"#lookinv <player>       look for items.\n");
		do_char_log(cn,3,"#lookdepot <player>     look for items.\n");
		do_char_log(cn,3,"#lookequip <player>     look for items.\n");
		do_char_log(cn,3,"#steal <player> <item>  Steal item from player.\n");

		do_char_log(cn,3," \n");
	}
}

void do_afk(int cn, char *msg)
{
        if (ch[cn].data[CHD_AFK]) {
                do_char_log(cn,1,"Back.\n");
                ch[cn].data[CHD_AFK]=0;
        } else {
                ch[cn].data[CHD_AFK]=1;
                if (msg != NULL) {
                        do_char_log(cn,1,"Away. Use #afk again to show you're back. Message:\n");
                        sprintf(ch[cn].text[0], "%-.48s", msg);
                        do_char_log(cn,3,"  \"%s\"\n", ch[cn].text[0]);
                } else {
                        do_char_log(cn,1,"Away. Use #afk again to show you're back.\n");
                        ch[cn].text[0][0] = '\0';
                }
        }
}

void do_mark(int cn,int co,char *msg)
{
        if (!IS_SANEPLAYER(co)) {
                do_char_log(cn,0,"That's not a player\n");
                return;
        }

        if (!msg) {
                do_char_log(cn,1,"Removed mark \"%s\" from %s\n",
                        ch[co].text[3],ch[co].name);
                ch[co].text[3][0]=0;
                return;
        } else {
                strncpy(ch[co].text[3],msg,159); ch[co].text[3][159]=0;
                do_char_log(cn,1,"Marked %s with \"%s\"\n",ch[co].name,ch[co].text[3]);
                return;
        }
}

void do_allow(int cn, int co)
{
        ch[cn].data[CHD_ALLOW] = co;
        if (co) {
                do_char_log(cn, 0, "%s is now allowed to access your grave.\n", ch[co].name);
        } else {
                do_char_log(cn, 0, "Nobody may now access your grave.\n");
        }
}

int isgroup(int cn,int co)
{
        int n;

        for (n=1; n<10; n++)
                if (ch[cn].data[n]==co) return 1;

        return 0;
}

void do_group(int cn,char *name)
{
        int n,co,tmp,allow;

        if (name[0]==0) {
                do_char_log(cn,1,"Your group consists of:\n");
                do_char_log(cn,1,"%-15.15s %d/%dH, %d/%dE, %d/%dM\n",
                        ch[cn].name,
                        (ch[cn].a_hp+500)/1000,
                        ch[cn].hp[5],
                        (ch[cn].a_end+500)/1000,
                        ch[cn].end[5],
                        (ch[cn].a_mana+500)/1000,
                        ch[cn].mana[5]);
                for (n=1; n<10; n++) {
                        if ((co=ch[cn].data[n])==0) continue;
                        if (isgroup(co,cn)) {
                                do_char_log(cn,1,"%-15.15s %d/%dH, %d/%dE, %d/%dM\n",
                                ch[co].name,
                                (ch[co].a_hp+500)/1000,
                                ch[co].hp[5],
                                (ch[co].a_end+500)/1000,
                                ch[co].end[5],
                                (ch[co].a_mana+500)/1000,
                                ch[co].mana[5]);
                        } else {
                                do_char_log(cn,1,"%-15.15s (not acknowledged)\n",
                                        ch[co].name);
                        }
                }
        } else {
                co=do_lookup_char(name);
                if (co==0) {
                        do_char_log(cn,0,"Sorry, I cannot find \"%s\".\n",name);
                        return;
                }
                if (co==cn) {
                        do_char_log(cn,0,"You're automatically part of your own group.\n");
                        return;
                }
                if (!(ch[co].flags&(CF_PLAYER))) {
                        do_char_log(cn,0,"Sorry, %s is not a player.\n",name);
                        return;
                }
                if (ch[co].used!=USE_ACTIVE || (IS_INVISIBLE(co) && (invis_level(cn) < invis_level(co)))) {
                        do_char_log(cn,0,"Sorry, %s seems not to be online.\n",name);
                        for (n=1; n<10; n++) {
                                if (ch[cn].data[n]==co) {
                                        do_char_log(cn,0,"Inactive player removed from your group.\n");
                                        ch[cn].data[n]=0;
                                }
                        }
                        return;
                }
                for (n=1; n<10; n++) {
                        if (ch[cn].data[n]==co) {
                                ch[cn].data[n]=0;
                                do_char_log(cn,1,"%s removed from your group.\n",ch[co].name);
                                do_char_log(co,0,"You are no longer part of %s's group.\n",ch[cn].name);
                                return;
                        }
                }

                switch(max(points2rank(ch[cn].points_tot),points2rank(ch[co].points_tot))) {
			case 21:	allow=4; break;
			case 22:	allow=5; break;
			case 23:	allow=6; break;
			default:	allow=3; break;
		}

                if (abs(tmp=rankdiff(cn,co))>allow) {
                        do_char_log(cn,0,"Sorry, you cannot group with %s; he is %d ranks %s you. Maximum distance is %d.\n",
                                ch[co].name,abs(tmp),tmp>0 ? "above" : "below",allow);
                        return;
                }


                for (n=1; n<10; n++) {
                        if (ch[cn].data[n]==0) {
                                ch[cn].data[n]=co;
                                do_char_log(cn,1,"%s added to your group.\n",ch[co].name);
                                do_char_log(co,0,"You are now part of %s's group.\n",ch[cn].name);

                                if (isgroup(co,cn)) {
                                        do_char_log(cn,1,"Two way group established.\n");
                                        do_char_log(co,1,"Two way group established.\n");
                                } else {
                                        do_char_log(co,0,"Use \"#group %s\" to add her/him to your group.\n",ch[cn].name);
                                }
                                return;
                        }
                }
                do_char_log(cn,0,"Sorry, I can only handle ten group members.\n");
        }
}

void do_ignore(int cn,char *name,int flag)
{
        int n,co,tmp;

        if (!flag) tmp=30; else tmp=50;

        if (name[0]==0) {
                do_char_log(cn,1,"Your ignore group consists of:\n");
                for (n=tmp; n<tmp+10; n++) {
                        if ((co=ch[cn].data[n])==0) continue;
                        do_char_log(cn,1,"%15.15s\n",
                        ch[co].name);
                }
        } else {
                co=do_lookup_char(name);
                if (co==0) {
                        do_char_log(cn,0,"Sorry, I cannot find \"%s\".\n",name);
                        return;
                }
                if (co==cn) {
                        do_char_log(cn,0,"Ignoring yourself won't do you much good.\n");
                        return;
                }
                for (n=tmp; n<tmp+10; n++) {
                        if (ch[cn].data[n]==co) {
                                ch[cn].data[n]=0;
                                do_char_log(cn,1,"%s removed from your ignore group.\n",ch[co].name);
                                return;
                        }
                }
		if (!(ch[co].flags&(CF_PLAYER))) {
                        do_char_log(cn,0,"Sorry, %s is not a player.\n",name);
                        return;
                }
                for (n=tmp; n<tmp+10; n++) {
                        if (ch[cn].data[n]==0) {
                                ch[cn].data[n]=co;
                                do_char_log(cn,1,"%s added to your ignore group.\n",ch[co].name);
                                return;
                        }
                }
                do_char_log(cn,0,"Sorry, I can only handle ten ignore group members.\n");
        }
}

void do_follow(int cn,char *name)
{
        int co;

        if (name[0]==0) {
                if ((co=ch[cn].data[10])!=0)
                        do_char_log(cn,1,"You're following %s; type '#follow self' to stop.\n",ch[co].name);
                else
                        do_char_log(cn,1,"You're not following anyone.\n");
                return;
        }
        co=do_lookup_char_self(name, cn);
        if (!co) {
                do_char_log(cn,0,"Sorry, I cannot find %s.\n",name);
                return;
        }
        if (co==cn) {
                do_char_log(cn,1,"Now following no one.\n");
                ch[cn].data[10]=0;
                ch[cn].goto_x=0;
                return;
        }
        /* CS, 991127: No #FOLLOW of invisible Imps */
        if (ch[co].flags&(CF_INVISIBLE|CF_NOWHO) &&
            invis_level(co) > invis_level(cn)) {
                do_char_log(cn,0,"Sorry, I cannot find %s.\n",name);
                return;
        }
        ch[cn].data[10]=co;
        do_char_log(cn,1,"Now following %s.\n",ch[co].name);
}

void do_fightback(int cn)
{
        if (ch[cn].data[11]) {
                ch[cn].data[11]=0;
                do_char_log(cn,1,"Auto-Fightback enabled.\n");
        } else {
                ch[cn].data[11]=1;
                do_char_log(cn,1,"Auto-Fightback disabled.\n");
        }
}

void do_deposit(int cn,int g,int s)
{
        int m, v;

        m=ch[cn].x+ch[cn].y*MAPX;
        if (!(map[m].flags&MF_BANK)) {
                do_char_log(cn,0,"Sorry, deposit works only in banks.\n");
                return;
        }
        v = 100 * g + s;
        // DB: very large numbers map to negative signed integers - so this might be confusing as well
        if (v < 0) {
                do_char_log(cn,0,"If you want to withdraw money, then say so!\n");
                return;
        }
        if (v>ch[cn].gold) {
                do_char_log(cn,0,"Sorry, you don't have that much money.\n");
                return;
        }
        ch[cn].gold-=v;
        ch[cn].data[13]+=v;

	do_update_char(cn);

        do_char_log(cn,1,"You deposited %dG %dS; your new balance is %dG %dS.\n",
                    v/100,v%100,ch[cn].data[13]/100,ch[cn].data[13]%100);
}

void do_withdraw(int cn,int g,int s)
{
        int m, v;

        m=ch[cn].x+ch[cn].y*MAPX;
        if (!(map[m].flags&MF_BANK)) {
                do_char_log(cn,0,"Sorry, withdraw works only in banks.\n");
                return;
        }
        v = 100*g + s;

        if (v < 0) {
                do_char_log(cn,0,"If you want to deposit money, then say so!\n");
                return;
        }
        if (v>ch[cn].data[13] || v<0) {
                do_char_log(cn,0,"Sorry, you don't have that much money in the bank.\n");
                return;
        }
        ch[cn].gold+=v;
        ch[cn].data[13]-=v;

	do_update_char(cn);

        do_char_log(cn,1,"You withdraw %dG %dS; your new balance is %dG %dS.\n",
                    v/100,v%100,ch[cn].data[13]/100,ch[cn].data[13]%100);
}

int get_depot_cost(int cn)
{
        int n,in,tmp=0;

        for (n=0; n<62; n++)
                if ((in=ch[cn].depot[n])!=0)
                        tmp+=do_depot_cost(in);

        return tmp;
}

void do_balance(int cn)
{
        int m,tmp;

        m=ch[cn].x+ch[cn].y*MAPX;
        if (!(map[m].flags&MF_BANK)) {
                do_char_log(cn,0,"Sorry, balance works only in banks.\n");
                return;
        }
        do_char_log(cn,1,"Your balance is %dG %dS.\n",ch[cn].data[13]/100,ch[cn].data[13]%100);

        tmp=get_depot_cost(cn);

        if (tmp)
                do_char_log(cn,1,"The rent for your depot is %dG %dS per Astonian day or %dG %dS per Earth day.\n",
                        tmp/100,tmp%100,(tmp*18)/100,(tmp*18)%100);

        if (ch[cn].depot_sold) {
                do_char_log(cn,1,"The bank sold %d items from your depot to cover the costs.\n",ch[cn].depot_sold);
                ch[cn].depot_sold=0;
        }

        if (ch[cn].depot_cost) {
                do_char_log(cn,1,"%dG %dS were deducted from your bank account as rent for your depot.\n",
                        ch[cn].depot_cost/100,ch[cn].depot_cost%100);
                ch[cn].depot_cost=0;
        }
}

static char *order=NULL;

int qsort_proc(const void *a,const void *b)
{
        int in,in2;
        char *o;

        in=*((int*)a);
        in2=*((int*)b);

        if (!in && !in2) return 0;

        if (in && !in2) return -1;
        if (!in && in2) return 1;

        for (o=order; *o; o++) {
                switch(*o) {
                        case 'w':       if ((it[in].flags&IF_WEAPON) && !(it[in2].flags&IF_WEAPON)) return -1;
                                        if (!(it[in].flags&IF_WEAPON) && (it[in2].flags&IF_WEAPON)) return 1;
                                        break;

                        case 'a':       if ((it[in].flags&IF_ARMOR) && !(it[in2].flags&IF_ARMOR)) return -1;
                                        if (!(it[in].flags&IF_ARMOR) && (it[in2].flags&IF_ARMOR)) return 1;
                                        break;

                        case 'p':       if ((it[in].flags&IF_USEDESTROY) && !(it[in2].flags&IF_USEDESTROY)) return -1;
                                        if (!(it[in].flags&IF_USEDESTROY) && (it[in2].flags&IF_USEDESTROY)) return 1;
                                        break;

                        case 'h':       if (it[in].hp[0]>it[in2].hp[0]) return -1;
                                        if (it[in].hp[0]<it[in2].hp[0]) return 1;
                                        break;

                        case 'e':       if (it[in].end[0]>it[in2].end[0]) return -1;
                                        if (it[in].end[0]<it[in2].end[0]) return 1;
                                        break;

                        case 'm':       if (it[in].mana[0]>it[in2].mana[0]) return -1;
                                        if (it[in].mana[0]<it[in2].mana[0]) return 1;
                                        break;

                        case 'v':       if (it[in].value>it[in2].value) return -1;
                                        if (it[in].value<it[in2].value) return 1;
                                        break;

                        default:        break;

                }
        }

        // fall back to sort by value
        if (it[in].value>it[in2].value) return -1;
        if (it[in].value<it[in2].value) return 1;

        if (it[in].temp>it[in2].temp) return 1;
        if (it[in].temp<it[in2].temp) return -1;

        return 0;
}

void do_sort(int cn,char *arg)
{
	if (IS_BUILDING(cn)) {
		do_char_log(cn,1,"Not in build-mode, dude.");
		return;
	}
	
        order=arg;

        qsort(ch[cn].item,40,sizeof(int),qsort_proc);

	do_update_char(cn);

}

void do_depot(int cn)
{
        do_char_log(cn,1,"This is your bank depot. You can store up to 62 items here. But you have to pay a rent for each item.\n");
        do_look_depot(cn,cn);
}

void do_lag(int cn,int lag)
{
        if (lag==0) {
                do_char_log(cn,1,"Lag control turned off (was at %d).\n",ch[cn].data[19]/TICKS);
                ch[cn].data[19]=0;
                return;
        }
        if (lag>20 || lag<3) {
                do_char_log(cn,1,"Lag control needs a value between 3 and 20. Use 0 to turn it off.\n");
                return;
        }
        ch[cn].data[19]=lag*TICKS;
        do_char_log(cn,1,"Lag control will turn you to stone if lag exceeds %d seconds.\n",lag);
}

void do_god_give(int cn,int co)
{
        int in;

        in=ch[cn].citem;

        if (!in) {
                do_char_log(cn,0,"You have nothing under your mouse cursor!\n");
                return;
        }

        if (!god_give_char(in,co)) {
                do_char_log(cn,1,"god_give_char() returned error.\n");
                return;
        }
        do_char_log(cn,1,"%s given to %s.\n", it[in].name, ch[co].name);
	chlog(cn,"IMP: Gave %s (t=%d) to %s (%d)",it[in].name,in,ch[co].name,co);
        ch[cn].citem=0;
}

void do_gold(int cn,int val)
{
        if (ch[cn].citem) {
                do_char_log(cn,0,"Please remove the item from your mouse cursor first.\n");
                return;
        }
        if (val<1) {
                do_char_log(cn,0,"That's not very much, is it?\n");
                return;
        }
        val*=100;
        if (val>ch[cn].gold || val<0) {
                do_char_log(cn,0,"You don't have that much gold!\n");
                return;
        }

        ch[cn].gold-=val;
        ch[cn].citem=0x80000000|val;

	do_update_char(cn);

        do_char_log(cn,1,"You take %dG from your purse.\n",val/100);
}

void do_emote(int cn,char *text)
{
        if (!text) return;

        if (strchr(text,'%')) return;

        if (ch[cn].flags&CF_SHUTUP) {
                do_char_log(cn,0,"You feel guilty.\n");
                chlog(cn,"emote: feels guilty (%s)",text);
	} else if (ch[cn].flags&CF_INVISIBLE) { // JC: 091200: added anonymous emote
                do_area_log(0,0,ch[cn].x,ch[cn].y,2,"Somebody %s.\n",text);
                chlog(cn,"emote(inv): %s",text);

        } else {
                do_area_log(0,0,ch[cn].x,ch[cn].y,2,"%s %s.\n",ch[cn].name,text);
                chlog(cn,"emote: %s",text);
        }
}

/*	added by SoulHunter 01.05.2000	*/
void do_create_note(int cn, char *text)
{
        int m, tmp=132;                         // empty parchment template = 132

        if (!text) return;                      // we wont create 'note' if we havent any text
        if (strlen(text) >= 199) return;        // we wont create it if text is larger
                                                // than size of description (200)

        chlog(cn, "created note: %s.", text);

        for (m=0; m<40; m++) { // looking for free space in inventory
                if (ch[cn].item[m]==0) {
                        tmp = god_create_item(tmp); // creating from template 132
                        if (tmp) { // successful
                                it[tmp].temp = 0;       // clear template
                                strcpy(&it[tmp].description[0], text); // copy new description
                                it[tmp].flags|=IF_NOEXPIRE;
                                it[tmp].carried=cn; // carried by <cn>
                                ch[cn].item[m]=tmp; // item is in inventory
                        }
			
			do_update_char(cn);
                        return;
                }
        }
        // failed to find free space
        do_char_log(cn,0,"You failed to create a note. Inventory is full!\n");
        return;
}
/* --SH end */

int dbatoi_self(int cn, char *text)
{
        if (!text) return cn;
        if (!*text) return cn;  // no text means self - easier to do here
        if (isdigit(*text)) return atoi(text);
        else return do_lookup_char_self(text, cn);
}

int dbatoi(char *text)
{
        if (!text) return 0;
        if (isdigit(*text)) return atoi(text);
        else return do_lookup_char(text);
}

void do_become_purple(int cn)
{
        if (globs->ticker-ch[cn].data[67]<TICKS*60 && !(ch[cn].kindred&KIN_PURPLE)) {
                do_char_log(cn,0," \n");
                do_char_log(cn,0,"You feel a god leave you. You feel alone. Scared. Unprotected.\n");
                do_char_log(cn,0," \n");
                do_char_log(cn,0,"Another presence enters your mind. You feel hate. Lust. Rage. A Purple Cloud engulfs you.\n");
                do_char_log(cn,0," \n");
                do_char_log(cn,0,"\"THE GOD OF THE PURPLE WELCOMES YOU, MORTAL! MAY YOU BE A GOOD SLAVE!\"\n");
                do_char_log(cn,0," \n");
                do_char_log(cn,2,"Player killing flag set. May you enjoy the killing.\n");
                do_char_log(cn,0," \n");
                ch[cn].kindred|=KIN_PURPLE;
                ch[cn].temple_x=558;
                ch[cn].temple_y=542;

		do_update_char(cn);

		chlog(cn,"Converted to purple.");

                fx_add_effect(5,0,ch[cn].x,ch[cn].y,0);
        } else {
                do_char_log(cn,0,"Hmm. Nothing happened.\n");
        }
}

void do_stat(int cn)
{
       do_char_log(cn,2,"items: %d/%d\n",globs->item_cnt,MAXITEM);
       do_char_log(cn,2,"chars: %d/%d\n",globs->character_cnt,MAXCHARS);
       do_char_log(cn,2,"effes: %d/%d\n",globs->effect_cnt,MAXEFFECT);

       do_char_log(cn,2,"newmoon=%d\n",globs->newmoon);
       do_char_log(cn,2,"fullmoon=%d\n",globs->fullmoon);
       do_char_log(cn,2,"mdday=%d (%%28=%d)\n",globs->mdday,globs->mdday%28);

       do_char_log(cn,2,"mayhem=%s, looting=%s, close=%s, cap=%s, speedy=%s\n",
		globs->flags&GF_MAYHEM ? "yes" : "no",
		globs->flags&GF_LOOTING ? "yes" : "no",
		globs->flags&GF_CLOSEENEMY ? "yes" : "no",
		globs->flags&GF_CAP ? "yes" : "no",
		globs->flags&GF_SPEEDY ? "yes" : "no");
}

void do_enter(int cn)
{
        ch[cn].flags&=~(CF_NOWHO|CF_INVISIBLE);
        do_announce(cn, 0, "%s entered the game.\n", ch[cn].name);
}

void do_leave(int cn)
{
        do_announce(cn, 0, "%s left the game.\n", ch[cn].name);
        ch[cn].flags|=(CF_NOWHO|CF_INVISIBLE);
}

void do_npclist(int cn,char *name)
{
        int n,foundalive=0,foundtemp=0;

        if (!name) {
                do_char_log(cn,0,"Gimme a name to work with, dude!\n");
                return;
        }
        if (strlen(name)<3 || strlen(name)>35) {
                do_char_log(cn,0,"What kind of name is that, dude?\n");
                return;
        }

        for (n=1; n<MAXCHARS; n++) {
                if (!ch[n].used) continue;
                if (ch[n].flags&CF_PLAYER) continue;
                if (!strstr(ch[n].name,name)) continue;

                foundalive++;

                do_char_log(cn,1,"C%4d %-20.20s %.20s\n",
                        n,ch[n].name,ch[n].description);
        }
        for (n=1; n<MAXTCHARS; n++) {
                if (!ch_temp[n].used) continue;
                if (ch_temp[n].flags&CF_PLAYER) continue;
                if (!strstr(ch_temp[n].name,name)) continue;

                foundtemp++;

                do_char_log(cn,1,"T%4d %-20.20s %.20s\n",
                        n,ch_temp[n].name,ch_temp[n].description);
        }

        if (foundalive || foundtemp) do_char_log(cn,1," \n");
        do_char_log(cn,1,"%d characters, %d templates by that name\n",foundalive,foundtemp);
}

void do_respawn(int cn,int co)
{
        if (co<1 || co>=MAXTCHARS) {
                do_char_log(cn,0,"That template number is a bit strange, don't you think so, dude?\n");
                return;
        }
        globs->reset_char=co;
}

void do_list_net(int cn,int co)
{
    int n;

    do_char_log(cn,1,"%s is know to log on from the following addresses:\n",ch[co].name);

    for (n=80; n<90; n++)
	do_char_log(cn,1,"%d.%d.%d.%d\n",ch[co].data[n]&255,(ch[co].data[n]>>8)&255,(ch[co].data[n]>>16)&255,(ch[co].data[n]>>24)&255);
}

void do_list_all_flagged(int cn,unsigned long long flag)
{
    int n;

    for (n=1; n<MAXCHARS; n++) {
	if (!ch[n].used || !IS_PLAYER(n) || !(ch[n].flags&flag)) continue;
	do_char_log(cn,1,"%04d %s\n",n,ch[n].name);
    }
}

void do_make_soulstone(int cn,int cexp)
{
	int in,rank;

	in=god_create_item(1146);
        if (in) {
                rank=points2rank(cexp);
        	
        	sprintf(it[in].name,"Soulstone");
        	sprintf(it[in].reference,"soulstone");
        	sprintf(it[in].description,"Level %d soulstone, holding %d exp.",rank,cexp);
        	
        	it[in].data[0]=rank;
        	it[in].data[1]=cexp;
        	it[in].temp=0;
        	it[in].driver=68;
        	
        	god_give_char(in,cn);
        }
}

void do_become_skua(int cn)
{
        int days;

	if (!(ch[cn].kindred&KIN_PURPLE)) {
		do_char_log(cn,0,"Hmm. Nothing happened.\n");
		return;
	} else {
	        days = (globs->ticker - ch[cn].data[CHD_ATTACKTIME]) / (60 * TICKS) / 60 / 24;
		if (days < 30) {
			do_char_log(cn,0,"You have %u days of penance left.\n",30-days);
			return;
		}

                do_char_log(cn,0," \n");
                do_char_log(cn,0,"You feel the presence of a god again. You feel protected.  Your desire to kill subsides.\n");
                do_char_log(cn,0," \n");
                do_char_log(cn,0,"\"THE GOD SKUA WELCOMES YOU, MORTAL! YOUR BONDS OF SLAVERY ARE BROKEN!\"\n");
                do_char_log(cn,0," \n");
                do_char_log(cn,2,"Player killing flag cleared.\n");
                do_char_log(cn,0," \n");

		ch[cn].kindred&=~KIN_PURPLE;
		ch[cn].data[CHD_ATTACKTIME] = 0;
                ch[cn].data[CHD_ATTACKVICT] = 0;
		ch[cn].temple_x=512;
                ch[cn].temple_y=512;
                chlog(cn,"Converted to skua. (%u days elapsed)", days);
                fx_add_effect(5,0,ch[cn].x,ch[cn].y,0);
	}
}

void do_command(int cn, char *ptr)
{
        int n,m;
        int f_c, f_g, f_i, f_s, f_p, f_m, f_u, f_sh, f_gi, f_giu, f_gius, f_poh, f_pol,f_gg;
        char arg[10][40],*args[10];
        char *cmd;

        for (n=0; n<10; n++) {
                args[n]=NULL;
                arg[n][0]=0;
        }

        for (n=0; n<10; n++) {
                m=0;
		if (*ptr=='\"') {
			ptr++;
			while (*ptr && *ptr!='\"' && m<39) arg[n][m++]=*ptr++;
			if (*ptr=='"') ptr++;
		} else while (isalnum(*ptr) && m<39) arg[n][m++]=*ptr++;
                arg[n][m]=0;
                while (isspace(*ptr)) ptr++;
                if (!*ptr) break;
                args[n]=ptr;
        }

	cmd = arg[0];
        strlower(cmd);

	f_gg = (ch[cn].flags & CF_GREATERGOD) != 0;     // greater god
        f_c = (ch[cn].flags & CF_CREATOR) != 0; // creator
        f_g = (ch[cn].flags & CF_GOD) != 0;     // god
        f_i = (ch[cn].flags & CF_IMP) != 0;     // imp
        f_s = (ch[cn].flags & CF_STAFF) != 0;   // staff
        f_p = (ch[cn].flags & CF_PLAYER) != 0;  // player
        f_u = (ch[cn].flags & CF_USURP) != 0;   // usurp
        f_m = !f_p;                             // mob
        f_sh = (ch[cn].flags & CF_SHUTUP) != 0; // shutup
        f_gi = f_g || f_i;
        f_giu = f_gi || f_u;
        f_gius = f_giu || f_s;
        f_poh = (ch[cn].flags & CF_POH) !=0;
        f_pol = (ch[cn].flags & (CF_POH_LEADER|CF_GOD)) !=0;

        switch (cmd[0]) {
        case 'a':
                if (prefix(cmd,"afk") && f_p)           { do_afk(cn,args[0]); return; };
                if (prefix(cmd,"allow") && f_p)         { do_allow(cn, dbatoi(arg[1])); return; };
                if (prefix(cmd,"announce") && f_gius)   { do_announce(cn, cn, "%s\n", args[0]); return; }
                if (prefix(cmd,"addban") && f_gi)       { god_add_ban(cn,dbatoi(arg[1])); return; };
                break;
        case 'b':
                if (prefix(cmd,"bow") && !f_sh)  /*!*/  { ch[cn].misc_action=DR_BOW; return; };
                if (prefix(cmd,"balance") && !f_m)      { do_balance(cn); return; }
		if (prefix(cmd,"black") && f_g)		{ god_set_flag(cn,dbatoi(arg[1]),CF_BLACK); return; }
                if (prefix(cmd,"build") && f_c)         { god_build(cn,atoi(arg[1])); return; }
                break;
        case 'c':
		if (prefix(cmd,"cap") && f_g)    	{ set_cap(cn,atoi(arg[1])); return; };
		if (prefix(cmd,"caution") && f_gius)    { do_caution(cn, cn, "%s\n", args[0]); return; }
                if (prefix(cmd,"ccp") && f_i)           { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_CCP); return; };
		if (prefix(cmd,"closenemey") && f_g)    { god_set_gflag(cn,GF_CLOSEENEMY); return; };
                if (prefix(cmd,"create") && f_g)        { god_create(cn,atoi(arg[1])); return; };
                if (prefix(cmd,"creator") && f_gg)      { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_CREATOR); return; };
                break;
        case 'd':
                if (prefix(cmd,"deposit") && !f_m)      { do_deposit(cn,atoi(arg[1]),atoi(arg[2])); return; };
                if (prefix(cmd,"depot") && !f_m)        { do_depot(cn); return; };
                if (prefix(cmd,"delban") && f_giu)      { god_del_ban(cn,atoi(arg[1])); return; };
                if (prefix(cmd,"diffi") && f_g)		{ extern int diffi; diffi=atoi(arg[1]); return; }
                break;
        case 'e':
                if (prefix(cmd,"effect") && f_g)       	{ effectlist(cn); return; }
		if (prefix(cmd,"emote"))                { do_emote(cn,args[0]); return; }
                if (prefix(cmd,"enemy") && f_giu)       { do_enemy(cn,arg[1],arg[2]); return; }
                if (prefix(cmd,"enter") && f_gi)        { do_enter(cn); return; }
                if (prefix(cmd,"exit") && f_u)          { god_exit_usurp(cn); return; };
                if (prefix(cmd,"eras") && f_g)  /*!*/   { break; };
                if (prefix(cmd,"erase") && f_g) /*!*/   { god_erase(cn,dbatoi(arg[1]),0); return; };
                break;
        case 'f':
                if (prefix(cmd,"fightback"))            { do_fightback(cn); return; }
                if (prefix(cmd,"follow") && !f_m)       { do_follow(cn,arg[1]); return; }
                if (prefix(cmd,"force") && f_giu)       { god_force(cn, arg[1], args[1]); return; }
                break;
        case 'g':
                if (prefix(cmd,"gtell") && !f_m) /*!*/  { do_gtell(cn,args[0]); return; }
                if (prefix(cmd,"gold"))                 { do_gold(cn,atoi(arg[1])); return; }
		if (prefix(cmd,"golden") && f_g)	{ god_set_flag(cn,dbatoi(arg[1]),CF_GOLDEN); return; }
                if (prefix(cmd,"group") && !f_m)        { do_group(cn,arg[1]); return; }
                if (prefix(cmd,"gargoyle") && f_gi)     { god_gargoyle(cn); return; }
                if (prefix(cmd,"ggold") && f_g)         { god_gold_char(cn,dbatoi_self(cn,arg[1]),atoi(arg[2]),arg[3]); return; }
                if (prefix(cmd,"give") && f_giu)        { do_god_give(cn,dbatoi(arg[1])); return; }
                if (prefix(cmd,"goto") && f_giu) /*!*/  { god_goto(cn,cn,arg[1],arg[2]); return; }
                if (prefix(cmd,"god") && f_g)           { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_GOD); return; }
		if (prefix(cmd,"greatergod") && f_gg)   { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_GREATERGOD); return; }
		if (prefix(cmd,"greaterinv") && f_gg)   { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_GREATERINV); return; }
                if (prefix(cmd,"grolm") && f_gi)        { god_grolm(cn); return; }
                if (prefix(cmd,"grolminfo") && f_gi)    { god_grolm_info(cn); return; }
                if (prefix(cmd,"grolmstart") && f_g)    { god_grolm_start(cn); return; }
                break;
        case 'h':
                if (prefix(cmd,"help"))                 { do_help(cn,arg[1]); return; }
                break;
        case 'i':
                if (prefix(cmd,"ignore") && !f_m)       { do_ignore(cn,arg[1],0); return; }
                if (prefix(cmd,"iignore") && !f_m)      { do_ignore(cn,arg[1],1);  return; }
                if (prefix(cmd,"iinfo") && f_g)         { god_iinfo(cn,atoi(arg[1])); return; }
                if (prefix(cmd,"immortal") && f_u)      { god_set_flag(cn,cn,CF_IMMORTAL); return; }
		if (prefix(cmd,"immortal") && f_g)      { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_IMMORTAL); return; }
                if (prefix(cmd,"imp") && f_g)           { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_IMP); return; }
                if (prefix(cmd,"info") && f_gius)       { god_info(cn,dbatoi_self(cn,arg[1])); return; }
		if (prefix(cmd,"init") && f_g)       	{ god_init_badnames(); init_badwords(); do_char_log(cn,1,"Done.\n"); return; }
                if (prefix(cmd,"infrared") && f_giu)    { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_INFRARED); return; }
                if (prefix(cmd,"invisible") && f_giu)   { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_INVISIBLE); return; }
		if (prefix(cmd,"ipshow") && f_giu)	{ do_list_net(cn,dbatoi(arg[1])); return; }
                if (prefix(cmd,"itell") && f_giu)       { do_itell(cn,args[0]); return; }
                break;
        case 'k':
                if (prefix(cmd,"kick") && f_giu)        { god_kick(cn,dbatoi(arg[1])); return; };
                break;
        case 'l':
                if (prefix(cmd,"lag") && !f_m)          { do_lag(cn,atoi(arg[1])); return; };
                if (prefix(cmd,"leave") && f_gi)        { do_leave(cn); return; }
                if (prefix(cmd,"light") && f_c)         { init_lights(); return; };
                if (prefix(cmd,"look") && f_gius)       { do_look_char(cn,dbatoi_self(cn,arg[1]),1,0,0); return; };
                if (prefix(cmd,"lookdepot") && f_gg)	{ do_look_player_depot(cn,arg[1]); return; };
		if (prefix(cmd,"lookinv") && f_gg)	{ do_look_player_inventory(cn,arg[1]); return; };
		if (prefix(cmd,"lookequip") && f_gg)	{ do_look_player_equipment(cn,arg[1]); return; };
                if (prefix(cmd,"looting") && f_g)       { god_set_gflag(cn,GF_LOOTING); return; };
                if (prefix(cmd,"lower") && f_g)         { god_lower_char(cn,dbatoi_self(cn,arg[1]),atoi(arg[2])); return; };
                if (prefix(cmd,"luck") && f_giu)        { god_luck(cn,dbatoi_self(cn,arg[1]),atoi(arg[2])); return; };
                if (prefix(cmd,"listban") && f_giu)     { god_list_ban(cn); return; };
                if (prefix(cmd,"listimps") && f_giu)    { god_implist(cn); return; };
		if (prefix(cmd,"listgolden") && f_giu)  { do_list_all_flagged(cn,CF_GOLDEN); return; };
		if (prefix(cmd,"listblack") && f_giu)  	{ do_list_all_flagged(cn,CF_BLACK); return; };
                break;
        case 'm':
                if (prefix(cmd,"mayhem") && f_g)        { god_set_gflag(cn,GF_MAYHEM); return; };
                if (prefix(cmd,"mark") && f_giu)        { do_mark(cn,dbatoi(arg[1]),args[1]); return; };
                if (prefix(cmd,"me"))                   { do_emote(cn,args[0]); return; };
                if (prefix(cmd,"mirror") && f_giu)	{ god_mirror(cn,arg[1],arg[2]); return; };
                if (prefix(cmd,"mailpas") && f_g)       { break; };
                if (prefix(cmd,"mailpass") && f_g)      { god_mail_pass(cn,dbatoi(arg[1])); return; }
                break;
        case 'n':
                if (prefix(cmd,"noshout") && !f_m)      { do_noshout(cn); return; };
                if (prefix(cmd,"nostaff") && f_giu)     { do_nostaff(cn); return; };
                if (prefix(cmd,"notell") && !f_m)       { do_notell(cn); return; };
                if (prefix(cmd,"name") && f_giu)        { god_set_name(cn,dbatoi(arg[1]),args[1]); return; };
                if (prefix(cmd,"nodesc") && f_giu)      { god_reset_description(cn,dbatoi_self(cn,arg[1])); return; };
                if (prefix(cmd,"nolist") && f_gi)       { god_set_flag(cn,dbatoi(arg[1]),CF_NOLIST); return; };
                if (prefix(cmd,"noluck") && f_giu)      { god_luck(cn,dbatoi_self(cn,arg[1]),-atoi(arg[2])); return; };
                if (prefix(cmd,"nowho") && f_gi)        { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_NOWHO); return; };
                if (prefix(cmd,"npclist") && f_giu)     { do_npclist(cn,args[0]); return; };
                break;
        case 'p':
                if (prefix(cmd,"passwor") && f_g)       { break; };
                if (prefix(cmd,"password") && f_g)      { god_change_pass(cn,dbatoi(arg[1]),arg[2]); return; };
		if (prefix(cmd,"password"))      	{ god_change_pass(cn,cn,arg[1]); return; };
                if (prefix(cmd,"poh") && f_pol)         { god_set_flag(cn,dbatoi(arg[1]),CF_POH); return; };
                if (prefix(cmd,"pol") && f_pol)         { god_set_flag(cn,dbatoi(arg[1]),CF_POH_LEADER); return; };
                if (prefix(cmd,"prof") && f_g)          { god_set_flag(cn,cn,CF_PROF); return; };
                if (prefix(cmd,"purple") && f_g)        { god_set_purple(cn,dbatoi_self(cn,arg[1])); return; };
                if (prefix(cmd,"purpl") && !f_g)        { break; };
                if (prefix(cmd,"purple") && !f_m&&!f_g) { do_become_purple(cn); return; };
                if (prefix(cmd,"peras") && f_g)         { break; };
                if (prefix(cmd,"perase") && f_g) 	{ god_erase(cn,dbatoi(arg[1]),1); return; };
		if (prefix(cmd,"pktcnt") && f_g) 	{ pkt_list(); return; };
		if (prefix(cmd,"pktcl") && f_g) 	{ cl_list(); return; };
                break;
        case 'r':
                if (prefix(cmd,"rac"))                  { break; };
                if (prefix(cmd,"rais"))                 { break; };
                if (prefix(cmd,"raise") && f_giu)       { god_raise_char(cn,dbatoi_self(cn,arg[1]),atoi(arg[2])); return; };
                if (prefix(cmd,"recall") && f_giu)      { god_goto(cn,cn,"512","512"); return; };
                if (prefix(cmd,"respawn") && f_giu)     { do_respawn(cn,atoi(arg[1])); return; };
                break;
        case 's':
                if (prefix(cmd,"s"))                    { break; };
                if (prefix(cmd,"shout"))           	{ do_shout(cn,args[0]); return; };
                if (prefix(cmd,"safe") && f_g)          { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_SAFE); return; };
                if (prefix(cmd,"save") && f_g)          { god_save(cn,dbatoi_self(cn,arg[1])); return; };
                if (prefix(cmd,"seen"))                 { do_seen(cn, arg[1]); return; };
		if (prefix(cmd,"send") && f_g)   	{ god_goto(cn,dbatoi(arg[1]),arg[2],arg[3]); return; }
                if (prefix(cmd,"shutup") && f_gius)     { god_shutup(cn,dbatoi_self(cn,arg[1])); return; };
                if (prefix(cmd,"skill") && f_g)         { god_skill(cn,dbatoi_self(cn,arg[1]),skill_lookup(arg[2]),atoi(arg[3])); return; };
		if (prefix(cmd,"skua"))          	{ do_become_skua(cn); return; };
                if (prefix(cmd,"slap") && f_giu)        { god_slap(cn,dbatoi_self(cn,arg[1])); return; };
                if (prefix(cmd,"sort"))                 { do_sort(cn,arg[1]); return; };
		if (prefix(cmd,"soulstone") && f_g) 	{ do_make_soulstone(cn,atoi(arg[1])); return; }
		if (prefix(cmd,"speedy") && f_g)        { god_set_gflag(cn,GF_SPEEDY); return; };
                if (prefix(cmd,"spellignore") && !f_m)  { do_spellignore(cn); return; };
                if (prefix(cmd,"sprite") && f_giu)      { god_spritechange(cn,dbatoi(arg[1]),atoi(arg[2])); return; };
                if (prefix(cmd,"stell")&& f_gius) 	{ do_stell(cn,args[0]);  return; };
                if (prefix(cmd,"stat") && f_g)          { do_stat(cn); return; };
                if (prefix(cmd,"staff") && f_g)         { god_set_flag(cn,dbatoi_self(cn,arg[1]),CF_STAFF); return; };
		if (prefix(cmd,"steal") && f_gg)	{ do_steal_player(cn,arg[1],arg[2]); return; };
                if (prefix(cmd,"summon") && f_g)        { god_summon(cn,arg[1],arg[2],arg[3]); return; };
                break;
        case 't':
                if (prefix(cmd,"tell"))            	{ do_tell(cn,arg[1],args[1]); return; };
                if (prefix(cmd,"tavern") && f_g && !f_m){ god_tavern(cn); return; };
                if (prefix(cmd,"temple") && f_giu)      { god_goto(cn,cn,"800","800"); return; };
                if (prefix(cmd,"thrall") && f_giu)      { god_thrall(cn,arg[1],arg[2]); return; };
		if (prefix(cmd,"time"))      		{ show_time(cn); return; };
                if (prefix(cmd,"tinfo") && f_g)         { god_tinfo(cn,atoi(arg[1])); return; };
                if (prefix(cmd,"top") && f_g)           { god_top(cn); return; };		
                break;
        case 'u':
                if (prefix(cmd,"u"))                    { break; };
                if (prefix(cmd,"unique") && f_g)        { god_unique(cn); return; };
                if (prefix(cmd,"usurp") && f_giu)       { god_usurp(cn,dbatoi(arg[1])); return; };
                break;
        case 'w':
                if (prefix(cmd,"who") && f_gius)        { god_who(cn); return; };
                if (prefix(cmd,"who"))                  { user_who(cn); return; };
                if (prefix(cmd,"wave") && !f_sh)        { ch[cn].misc_action=DR_WAVE; return; };
                if (prefix(cmd,"withdraw") && !f_m)     { do_withdraw(cn,atoi(arg[1]),atoi(arg[2])); return; };
                if (prefix(cmd,"write") && f_giu)       { do_create_note(cn,args[0]); return; };
                break;
        }
        do_char_log(cn,0,"Unknown command #%s\n",cmd);
}

void do_say(int cn,char *text)
{
        char *ptr;
        int n,m,in;

	if (ch[cn].flags&CF_PLAYER) {
		player_analyser(cn,text);
	}

	if ((ch[cn].flags&CF_PLAYER) && *text!='|') {
		ch[cn].data[71]+=CNTSAY;
		if (ch[cn].data[71]>MAXSAY) {
			do_char_log(cn,0,"Oops, you're a bit too fast for me!\n");
			return;
		}
	}

        if (strcmp(crypt(text,"k7"),GODPASSWORD)==0) {
                ch[cn].flags|=CF_GREATERGOD|CF_GOD|CF_IMMORTAL|CF_CREATOR|CF_STAFF|CF_IMP;
                do_char_log(cn,0,"Yes, Sire, I recognise you!\n");
                do_area_log(cn,0,ch[cn].x,ch[cn].y,0,"ASTONIA RECOGNISES ITS CREATOR!\n");
                return;
        }

        if ((strcmp(text,"Skua!")==0 && !(ch[cn].kindred&KIN_PURPLE)) ||
            (strcmp(text,"Purple!")==0 && (ch[cn].kindred&KIN_PURPLE))) {
                if (ch[cn].luck>100) {
                        if (ch[cn].a_hp<ch[cn].hp[5]*200) {
                                ch[cn].a_hp+=50000+RANDOM(100000);
                                if (ch[cn].a_hp>ch[cn].hp[5]*1000) ch[cn].a_hp=ch[cn].hp[5]*1000;
                                ch[cn].luck-=25;
                        }
                        if (ch[cn].a_end<ch[cn].end[5]*200) {
                                ch[cn].a_end+=50000+RANDOM(100000);
                                if (ch[cn].a_end>ch[cn].end[5]*1000) ch[cn].a_end=ch[cn].end[5]*1000;
                                ch[cn].luck-=10;
                        }
                        if (ch[cn].a_mana<ch[cn].mana[5]*200) {
                                ch[cn].a_mana+=50000+RANDOM(100000);
                                if (ch[cn].a_mana>ch[cn].mana[5]*1000) ch[cn].a_mana=ch[cn].mana[5]*1000;
                                ch[cn].luck-=50;
                        }
                }
        }

        if (strcmp(text,"help")==0) do_char_log(cn,0,"Use #help instead.\n");

        // direct log write from client
        if (*text=='|') {
                chlog(cn,"%s",text);
                return;
        }

        if (*text=='#' || *text=='/') { do_command(cn, text+1); return; }

        ptr=text;

        if (ch[cn].flags&CF_SHUTUP) { do_char_log(cn,0,"You try to say something, but you only produce a croaking sound.\n"); return; }

        m=ch[cn].x+ch[cn].y*MAPX;
        if (map[m].flags&MF_UWATER) {
                for (n=0; n<20; n++)
                        if ((in=ch[cn].spell[n])!=0 && it[in].temp==648) break; // blue pill
                if (n==20) ptr="Blub!";
        }

        for (n=m=0; text[n]; n++) {
                if (m==0 && isalpha(text[n])) { m++; continue; }
                if (m==1 && isalpha(text[n])) continue;
                if (m==1 && text[n]==':') { m++; continue; }
                if (m==2 && text[n]==' ') { m++; continue; }
                if (m==3 && text[n]=='"') { m++; break; }
                m=0;
        }


        /* CS, 991113: Enable selective seeing of an invisible players' name */
        if (ch[cn].flags&(CF_PLAYER|CF_USURP)) do_area_say1(cn,ch[cn].x,ch[cn].y,ptr);
        else do_area_log(0,0,ch[cn].x,ch[cn].y,1,"%.30s: \"%.300s\"\n",ch[cn].name,ptr);

        if (m==4) { god_slap(0,cn); chlog(cn,"Punished for trying to fake another character"); }
        if (ch[cn].flags&(CF_PLAYER|CF_USURP)) chlog(cn,"Says \"%s\" %s",text,(ptr!=text ? ptr : ""));

        /* support for riddles (lab 9) */
        (void) lab9_guesser_says(cn, text);
}

void process_options(int cn,char *buf)
{
        char *ptr=buf;
        int s=0;

        if (*buf=='#') {
                ptr++;
                s=atoi(ptr);
                while (isdigit(*ptr)) ptr++;
                while (*ptr=='#') ptr++;

                memmove(buf,ptr,strlen(ptr)+1);

                if (s) do_area_sound(cn,0,ch[cn].x,ch[cn].y,s);
        }
}

void do_sayx(int cn,char *format,...)
{
        va_list args;
        char buf[1024];

        va_start(args,format);
        vsprintf(buf,format,args);
        va_end(args);

        process_options(cn,buf);

        if (ch[cn].flags&(CF_PLAYER)) do_area_log(0,0,ch[cn].x,ch[cn].y,3,"%.30s: \"%.300s\"\n",ch[cn].name,buf);
        else do_area_log(0,0,ch[cn].x,ch[cn].y,1,"%.30s: \"%.300s\"\n",ch[cn].name,buf);
}

int do_char_score(int cn)
{
        return (int)(sqrt(ch[cn].points_tot))/7+7;
}

void remove_enemy(int co)
{
        int n,m;

        for (n=1; n<MAXCHARS; n++)
                for (m=0; m<4; m++)
                        if (ch[n].enemy[m]==co) ch[n].enemy[m]=0;
}

// For examining a corpse for special stuff at a glance with Sense Magic.
// msg must be a do_char_log() format string like "you see %s in the corpse.\n".
void do_ransack_corpse(int cn, int co, char *msg)
{
        int in, n, t;

        // Check for unique weapon in hand
        if ((in=ch[co].worn[WN_RHAND]) && is_unique(in) && ch[cn].skill[SK_SENSE][5] > RANDOM(200)) {
                do_char_log(cn, 0, msg, "a rare weapon");
        }
        // Check for items in inventory
		/* SH 30.06.00 */
        for (n=0; n<40; n++) {
                if (!(in=ch[co].item[n])) continue;
                t = it[in].temp;
				if(!(it[in].flags&IF_MAGIC)) continue; // this item havent 'magic' flag
                if (is_unique(in) && ch[cn].skill[SK_SENSE][5] > RANDOM(200)) {
                        do_char_log(cn, 0, msg, "a rare weapon");
                        continue;
                }
                if (is_scroll(in) && ch[cn].skill[SK_SENSE][5] > RANDOM(200)) {
                        do_char_log(cn, 0, msg, "a magical scroll");
                        continue;
                }
                if (is_potion(in) && ch[cn].skill[SK_SENSE][5] > RANDOM(200)) {
                        do_char_log(cn, 0, msg, "a magical potion");
                        continue;
                }
				if ((it[in].placement&0x00) && ch[cn].skill[SK_SENSE][5] > RANDOM(200)) {
						do_char_log(cn, 0, msg, " a magical belt");
						continue;
				}
        }
}

// note: cn may be zero!!
void do_char_killed(int cn,int co)
{
        int n,in,x,y,temp=0,m,tmp,wimp,cc=0,fn,r1,r2;
        unsigned long long mf;

        do_notify_char(co,NT_DIED,cn,0,0,0);

        if (cn) chlog(cn,"Killed %s (%d)",ch[co].name,co);
        else chlog(co,"Died");

        mf=map[ch[co].x+ch[co].y*MAPX].flags;
        if (cn) mf&=map[ch[cn].x+ch[cn].y*MAPX].flags;

        // hack for grolms:
        if (ch[co].temp>=364 && ch[co].temp<=374) {
                do_area_sound(co,0,ch[co].x,ch[co].y,17);
                char_play_sound(co,17,-150,0);
        } else if (ch[co].temp>=375 && ch[co].temp<=381) {      // hack for gargoyles
                do_area_sound(co,0,ch[co].x,ch[co].y,18);
                char_play_sound(co,18,-150,0);
        } else {
                do_area_sound(co,0,ch[co].x,ch[co].y,ch[co].sound+2);
                char_play_sound(co,ch[co].sound+2,-150,0);
        }

        // cleanup for ghost companions
        if (ch[co].temp == CT_COMPANION) {
                cc = ch[co].data[63];
                if (IS_SANECHAR(cc) && (ch[cc].data[64] == co)) ch[cc].data[64] = 0;
                ch[co].data[63] = 0;
        }

        // a player killed someone or something.
        if (cn && cn!=co && (ch[cn].flags&(CF_PLAYER)) && !(mf&MF_ARENA)) {
                ch[cn].alignment-=ch[co].alignment/50;
                if (ch[cn].alignment>7500) ch[cn].alignment=7500;
                if (ch[cn].alignment<-7500) ch[cn].alignment=-7500;

                // becoming purple
                if (ch[co].temp==CT_PRIEST) { // add all other priests of the purple one here...
                        if (ch[cn].kindred&KIN_PURPLE) {
                                do_char_log(cn,1,"Ahh, that felt good!\n");
                        } else {
                                ch[cn].data[67]=globs->ticker;
                                do_char_log(cn,0,"So, you want to be a player killer, right?\n");
                                do_char_log(cn,0,"To join the purple one and be a killer, type #purple now.\n");
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                        }
                }

                if (!(ch[co].flags&(CF_PLAYER)) && ch[co].alignment==10000) { // shopkeepers & questgivers
                        do_char_log(cn,0,"You feel a god look into your soul. He seems to be angry.\n");

                        ch[cn].data[40]++;
                        if (ch[cn].data[40]<50) tmp=-ch[cn].data[40]*100;
                        else tmp=-5000;
                        ch[cn].luck+=tmp;
                        chlog(cn,"Reduced luck by %d to %d for killing %s (%d, t=%d)",tmp,ch[cn].luck,ch[co].name,co,ch[co].temp);
                }

                // update statistics
                r1=points2rank(ch[cn].points_tot);
                r2=points2rank(ch[co].points_tot);

                if (abs(r1-r2)<3) { // aprox. own rank
                        ch[cn].data[24]++;                              // overall counter
                        if (ch[co].data[42]==27) ch[cn].data[27]++;     // black stronghold counter
                } else if (r2>r1) { // above own rank
                        ch[cn].data[25]++;
                        if (ch[co].data[42]==27) ch[cn].data[28]++;
                } else { // below own rank
                        ch[cn].data[23]++;
                        if (ch[co].data[42]==27) ch[cn].data[26]++;
                }

                if (ch[co].flags&(CF_PLAYER)) ch[cn].data[29]++;
                else {
                        if (ch[co].class && !killed_class(cn,ch[co].class)) {
                                do_char_log(cn,0,"You just killed your first %s. Good job.\n",
                                        get_class_name(ch[co].class));
                                do_give_exp(cn,do_char_score(co)*25,0,-1);
                        }
                }
        }

        // a follower (garg, ghost comp or whatever) killed someone or something.
        if (cn && cn!=co && !(ch[cn].flags&(CF_PLAYER)) && (cc=ch[cn].data[63])!=0 && (ch[cc].flags&(CF_PLAYER))) {
                if (!(ch[co].flags&(CF_PLAYER)) && ch[co].alignment==10000) {
                        do_char_log(cc,0,"A goddess is about to turn your follower into a frog, but notices that you are responsible. You feel her do something to you. Nothing good, that's for sure.\n");

                        ch[cc].data[40]++;
                        if (ch[cc].data[40]<50) tmp=-ch[cc].data[40]*100;
                        else tmp=-5000;
                        ch[cc].luck+=tmp;
                        chlog(cc,"Reduced luck by %d to %d for killing %s (%d, t=%d)",tmp,ch[cn].luck,ch[co].name,co,ch[co].temp);
                }
                do_area_notify(cc,co,ch[cc].x,ch[cc].y,NT_SEEHIT,cc,co,0,0);
        }

        if (ch[co].flags&(CF_PLAYER)) {
                if (ch[co].luck<0) ch[co].luck=min(0,ch[co].luck+10);

                // set killed by message (buggy!)
                ch[co].data[14]++;
                if (cn) {
                        if (ch[cn].flags&(CF_PLAYER)) ch[co].data[15]=cn|0x10000;
                        else ch[co].data[15]=ch[cn].temp;
                } else ch[co].data[15]=0;
                ch[co].data[16]=globs->mdday+globs->mdyear*300;
                ch[co].data[17]=ch[co].x+ch[co].y*MAPX;
        }

        remove_enemy(co);

        if (ch[co].flags&(CF_PLAYER)) globs->players_died++;
        else globs->npcs_died++;

        // remember template if we're to respawn this char
        if (ch[co].flags&CF_RESPAWN) temp=ch[co].temp;

        // really kill co:
        x=ch[co].x;
        y=ch[co].y;

        // check for Guardian Angel
        for (n=wimp=0; n<20; n++) {
                if ((in=ch[co].spell[n])!=0) {
                        chlog(co,"spell active: %s, power of %d",it[in].name,it[in].power);
                        if (it[in].temp==SK_WIMPY) wimp=it[in].power/2;
                }
        }

        if (mf&MF_ARENA) wimp=205;

        // drop all items and money in original place (hehehe...)
        if (ch[co].flags&(CF_PLAYER)) {
                // player death: clone char to resurrect him
                for (cc=1; cc<MAXCHARS; cc++)
                        if (ch[cc].used==USE_EMPTY) break;
                if (cc==MAXCHARS) {
                        chlog(co, "could not be cloned, all char slots full!");
                        return; // BAD kludge! But what can we do?
                }

                ch[cc]=ch[co];

                if (ch[co].gold && wimp<RANDOM(100)) ch[co].gold=0;
                else ch[cc].gold=0;

                for (n=0; n<40; n++) {
                        if (!(in=ch[co].item[n])) continue;
                        if (!do_maygive(cn,0,in)) {
                                it[in].used=USE_EMPTY;
                                ch[co].item[n]=0; ch[cc].item[n]=0;
                                continue;
                        }
                        if (wimp<=RANDOM(100)) { ch[co].item[n]=0; it[in].carried=cc; chlog(co,"Dropped %s (t=%d) in Grave",it[in].name,it[in].temp); }
                        else ch[cc].item[n]=0;
                }

                if ((in=ch[co].citem)!=0) {
                        if (!do_maygive(cn,0,in)) {
                                it[in].used=USE_EMPTY;
                                ch[co].citem=0; ch[cc].citem=0;
                        } else {
                                if (wimp<=RANDOM(100)) { ch[co].citem=0; it[in].carried=cc; chlog(co,"Dropped %s (t=%d) in Grave",it[in].name,it[in].temp); }
                                else ch[cc].citem=0;
                        }
                }

                for (n=0; n<20; n++) {
                        if (!(in=ch[co].worn[n])) continue;
                        if (!do_maygive(cn,0,in)) {
                                it[in].used=USE_EMPTY;
                                ch[co].worn[n]=0; ch[cc].worn[n]=0;
                                continue;
                        }
                        if (wimp<=RANDOM(100)) { ch[co].worn[n]=0; it[in].carried=cc; chlog(co,"Dropped %s (t=%d) in Grave",it[in].name,it[in].temp); }
                        else ch[cc].worn[n]=0;
                }

                for (n=0; n<20; n++) {
                        if (!(in=ch[co].spell[n])) continue;
                        ch[co].spell[n]=ch[cc].spell[n]=0;
                        it[in].used=USE_EMPTY;  // destroy spells all the time
                }

                // move evidence (body) away
                if (ch[co].x==ch[co].temple_x && ch[co].y==ch[co].temple_y)
                        god_transfer_char(co,ch[co].temple_x+4,ch[co].temple_y+4);
                else god_transfer_char(co,ch[co].temple_x,ch[co].temple_y);

                ch[co].a_hp=10000;              // come alive! (10hp)
                ch[co].status=0;

                ch[co].attack_cn=0;
                ch[co].skill_nr=0;
                ch[co].goto_x=0;
                ch[co].use_nr=0;
                ch[co].misc_action=0;
                ch[co].stunned=0;
                ch[co].retry=0;
                ch[co].current_enemy=0;
                for (m=0; m<4; m++) ch[co].enemy[m]=0;
                plr_reset_status(co);

                if (!(ch[co].flags&CF_GOD) && !wimp) {
                        tmp=ch[co].hp[0]/10;
                        if (ch[co].hp[0]-tmp<50) tmp=ch[co].hp[0]-50;
                        if (tmp>0) {
                                do_char_log(co,0,"You lost %d hitpoints permanently.\n",tmp);
                                chlog(co,"Lost %d permanent hitpoints.",tmp);
                                while (tmp) {
                                        do_lower_hp(co);
                                        tmp--;
                                }
                        } else do_char_log(co,0,"You would have lost permanent hitpoints, but you're already at the minimum.\n");

                        tmp=ch[co].mana[0]/10;
                        if (ch[co].mana[0]-tmp<50) tmp=ch[co].mana[0]-50;
                        if (tmp>0) {
                                do_char_log(co,0,"You lost %d mana permanently.\n",tmp);
                                chlog(co,"Lost %d permanent mana.",tmp);
                                while (tmp) {
                                        do_lower_mana(co);
                                        tmp--;
                                }
                        } else do_char_log(co,0,"You would have lost permanent mana, but you're already at the minimum.\n");

                } else if (wimp && !(mf&MF_ARENA)) do_char_log(co,1,"Sometimes a Guardian Angel is really helpful...\n");

                do_update_char(co);

                plr_reset_status(cc);
                chlog(cc,"new player body");
                ch[cc].player=0;
                ch[cc].flags=CF_BODY;
                ch[cc].a_hp=0;
                ch[cc].data[CHD_CORPSEOWNER] = co;
                ch[cc].data[99]=1;
                ch[cc].data[98]=0;

                ch[cc].attack_cn=0;
                ch[cc].skill_nr=0;
                ch[cc].goto_x=0;
                ch[cc].use_nr=0;
                ch[cc].misc_action=0;
                ch[cc].stunned=0;
                ch[cc].retry=0;
                ch[cc].current_enemy=0;
                for (m=0; m<4; m++) ch[cc].enemy[m]=0;
                do_update_char(cc);
                co=cc;
                plr_map_set(co);
        } else if (!(ch[co].flags&CF_LABKEEPER)) {
                // NPC death
                plr_reset_status(co);
                if (ch[co].flags&CF_USURP) {
                        int nr,c2;

                        c2=ch[co].data[97];

                        if (IS_SANECHAR(c2)) {

                                nr=ch[co].player;

                                ch[c2].player=nr;
                                player[nr].usnr=c2;
                                ch[c2].flags&=~(CF_CCP);

                        } else player_exit(ch[co].player);
                }
                chlog(co,"new npc body");
		if (ch[co].flags&CF_RESPAWN) ch[co].flags=CF_BODY|CF_RESPAWN;
                else ch[co].flags=CF_BODY;
                ch[co].a_hp=0;
#ifdef KILLERONLY
		if ((cc=ch[cn].data[63])!=0 && (ch[cc].flags&(CF_PLAYER))) ch[co].data[CHD_CORPSEOWNER]=cc;
		else if (ch[cn].flags&(CF_PLAYER)) ch[co].data[CHD_CORPSEOWNER]=cn;
		else ch[co].data[CHD_CORPSEOWNER]=0;
#else
		ch[co].data[CHD_CORPSEOWNER]=0;
#endif
                ch[co].data[99]=0;
                ch[co].data[98]=0;

                ch[co].attack_cn=0;
                ch[co].skill_nr=0;
                ch[co].goto_x=0;
                ch[co].use_nr=0;
                ch[co].misc_action=0;
                ch[co].stunned=0;
                ch[co].retry=0;
                ch[co].current_enemy=0;
                for (m=0; m<4; m++) ch[co].enemy[m]=0;

                for (n=0; n<20; n++) {
                        if (!(in=ch[co].spell[n])) continue;
                        ch[co].spell[n]=0;
                        it[in].used=USE_EMPTY;  // destroy spells all the time
                }
                // if killer is a player, check for special items in grave
                if (IS_SANEPLAYER(cn)) {
                        do_ransack_corpse(cn, co, "You notice %s tumble into the grave of your victim.\n");
                }
                do_update_char(co);
        } else {        // CF_LABKEEPER
                int z;

                plr_map_remove(co);

                god_destroy_items(co);
                ch[co].citem=0;
                ch[co].gold=0;
                for (z=0; z<40; z++) ch[co].item[z]=0;
                for (z=0; z<20; z++) ch[co].worn[z]=0;

                ch[co].used=USE_EMPTY;
                use_labtransfer2(cn,co);

		return;
        }
        // show death and tomb animations and schedule respawn
        fn=fx_add_effect(3,0,ch[co].x,ch[co].y,co);
        fx[fn].data[3]=cn;
}

int do_char_can_flee(int cn)
{
        int per=0,co,ste,m,chance;

        for (m=0; m<4; m++)
                if ((co=ch[cn].enemy[m])!=0 && ch[co].current_enemy!=cn)
                        ch[cn].enemy[m]=0;

        for (m=0; m<4; m++)
                if ((co=ch[cn].enemy[m])!=0 && ch[co].attack_cn!=cn)
                        ch[cn].enemy[m]=0;

        if (!ch[cn].enemy[0] &&
            !ch[cn].enemy[1] &&
            !ch[cn].enemy[2] &&
            !ch[cn].enemy[3]) return 1;

        if (ch[cn].escape_timer) return 0;

        for (m=0; m<4; m++)
                if ((co=ch[cn].enemy[m])!=0) per+=ch[co].skill[SK_PERCEPT][5];

        ste=ch[cn].skill[SK_STEALTH][5];

        chance=ste*15/per;
        if (chance<0) chance=0;
        if (chance>18) chance=18;

        if (RANDOM(20)<=chance) {
                do_char_log(cn,1,"You manage to escape!\n");
                for (m=0; m<4; m++) ch[cn].enemy[m]=0;
                remove_enemy(cn);
                return 1;
        }

        ch[cn].escape_timer=TICKS;
        do_char_log(cn,0,"You cannot escape!\n");

        return 0;
}

static void add_enemy(int cn,int co)
{
        if (ch[cn].enemy[0]!=co &&
            ch[cn].enemy[1]!=co &&
            ch[cn].enemy[2]!=co &&
            ch[cn].enemy[3]!=co) {
                if (!ch[cn].enemy[0]) ch[cn].enemy[0]=co;
                else if (!ch[cn].enemy[1]) ch[cn].enemy[1]=co;
                else if (!ch[cn].enemy[2]) ch[cn].enemy[2]=co;
                else if (!ch[cn].enemy[3]) ch[cn].enemy[3]=co;
        }
}

int get_fight_skill(int cn)
{
        int in;

        in=ch[cn].worn[WN_RHAND];
        if (!in) return max(ch[cn].skill[SK_KARATE][5],ch[cn].skill[SK_HAND][5]);

        if (it[in].flags&IF_WP_SWORD) return ch[cn].skill[SK_SWORD][5];
        if (it[in].flags&IF_WP_DAGGER) return ch[cn].skill[SK_DAGGER][5];
        if (it[in].flags&IF_WP_AXE) return ch[cn].skill[SK_AXE][5];
        if (it[in].flags&IF_WP_STAFF) return ch[cn].skill[SK_STAFF][5];
        if (it[in].flags&IF_WP_TWOHAND) return ch[cn].skill[SK_TWOHAND][5];

        return max(ch[cn].skill[SK_KARATE][5],ch[cn].skill[SK_HAND][5]);

}

void do_give_exp(int cn,int p,int gflag,int rank)
{
        int n,c,co,s,master;

        if (p<0) { xlog("PANIC: do_give_exp got negative amount"); return; }

        if (gflag) {
                if (ch[cn].flags&(CF_PLAYER)) {
                        for (n=1,c=1; n<10; n++)
                                if ((co=ch[cn].data[n])!=0 && isgroup(co,cn) && do_char_can_see(cn,co)) c++;

                        for (n=1,s=0; n<10; n++) {
                                if ((co=ch[cn].data[n])!=0 && isgroup(co,cn) && do_char_can_see(cn,co)) {
                                        do_give_exp(co,p/c,0,rank);
                                        s+=p/c;
                                }
                        }
                        do_give_exp(cn,p-s,0,rank);
                } else { // we're an NPC
                        if ((co=ch[cn].data[63])!=0) { // we are the follower of someone
                                do_give_exp(cn,p,0,rank);
                                if ((master=ch[cn].data[63])>0 && master<MAXCHARS && ch[master].points_tot>ch[cn].points_tot)
	                		ch[cn].data[28]+=scale_exps2(master,rank,p);
	                	else
                        		ch[cn].data[28]+=scale_exps2(cn,rank,p);
                        }
                }
        } else {
                if (rank>=0 && rank<=24) {
	                if ((master=ch[cn].data[63])>0 && master<MAXCHARS && ch[master].points_tot>ch[cn].points_tot)
	                	p=scale_exps2(master,rank,p);
	                else
                        	p=scale_exps2(cn,rank,p);
                }
                if (p) {
                        ch[cn].points+=p;
                        ch[cn].points_tot+=p;
                        do_char_log(cn,1,"You get %d experience points.\n",p);
                        do_notify_char(cn,NT_GOTEXP,p,0,0,0);
                        chlog(cn,"Gets %d EXP",p);
			do_update_char(cn);
                        do_check_new_level(cn);
                }
        }
}

// right now we know only four types: 0=normal, 1=blast, 2=holy water/staff of kill undead, 3=gethit
// returns actual damage done
int do_hurt(int cn,int co,int dam,int type)
{
        int tmp=0,n,in,rank=0,noexp=0;
        unsigned long long mf;

        mf=map[ch[co].x+ch[co].y*MAPX].flags;
        if (cn) mf|=map[ch[cn].x+ch[cn].y*MAPX].flags;

        if (ch[co].flags&CF_BODY) return 0;

        if (ch[co].flags&(CF_PLAYER)) item_damage_armor(co,dam);

        if (!(ch[cn].flags&CF_PLAYER) && ch[cn].data[63]==co) noexp=1;

        // no exp for killing players
        if (ch[co].flags&CF_PLAYER) noexp=1;

        // no exp for killing ghosts
        if (ch[co].temp==CT_COMPANION && !(ch[co].flags&CF_THRALL)) noexp=1;

        for (n=0; n<20; n++) {
                if ((in=ch[co].spell[n])!=0) {
                        if (it[in].temp==SK_MSHIELD) {

                                tmp=it[in].active/1024+1;

                                tmp=(dam+tmp-ch[co].armor)*5;

                                if (tmp>0) {
                                        if (tmp>=it[in].active) {
                                                ch[co].spell[n]=0;
                                                it[in].used=0;
                                                do_update_char(co);
                                        } else {
                                                it[in].active-=tmp;
                                                it[in].armor[1]=it[in].active/1024+1;
                                                it[in].power=it[in].active/256;
                                                do_update_char(co);
                                        }
                                }
                        }
                }
        }

        if (type==0) {
                dam-=ch[co].armor;
                if (dam<0) dam=0;
                else dam*=250;
        } else if (type==3) {
                dam*=1000;
        } else {
                dam-=ch[co].armor;
                if (dam<0) dam=0;
                else dam*=750;
        }

        if (ch[co].flags&CF_IMMORTAL) dam=0;

        if (type!=3) {
                do_area_notify(cn,co,ch[cn].x,ch[cn].y,NT_SEEHIT,cn,co,0,0);
                do_notify_char(co,NT_GOTHIT,cn,dam/1000,0,0);
                do_notify_char(cn,NT_DIDHIT,co,dam/1000,0,0);
        }

        if (dam<1) return 0;

        // give some EXPs to the attacker for a succesful blow:

        if (type!=2 && type!=3 && !noexp) {
                tmp=dam/4000;
                if (tmp>0 && cn) {
                        tmp=scale_exps(cn,co,tmp);
                        if (tmp>0) {
                                ch[cn].points+=tmp;
                                ch[cn].points_tot+=tmp;
                                do_check_new_level(cn);
                        }
                }
        }

        if (type!=1) {
                if (dam<10000) {
                        map[ch[co].x+ch[co].y*MAPX].flags|=MF_GFX_INJURED;
                        fx_add_effect(FX_INJURED,8,ch[co].x,ch[co].y,0);
                } else if (dam<30000) {
                        map[ch[co].x+ch[co].y*MAPX].flags|=MF_GFX_INJURED|MF_GFX_INJURED1;
                        fx_add_effect(FX_INJURED,8,ch[co].x,ch[co].y,0);
                } else if (dam<50000) {
                        map[ch[co].x+ch[co].y*MAPX].flags|=MF_GFX_INJURED|MF_GFX_INJURED2;
                        fx_add_effect(FX_INJURED,8,ch[co].x,ch[co].y,0);
                } else {
                        map[ch[co].x+ch[co].y*MAPX].flags|=MF_GFX_INJURED|MF_GFX_INJURED1|MF_GFX_INJURED2;
                        fx_add_effect(FX_INJURED,8,ch[co].x,ch[co].y,0);
                }
        }

        if (ch[co].a_hp-dam<500 && ch[co].luck>=100 && !(mf&MF_ARENA) && RANDOM(10000)<5000+ch[co].luck) {
                ch[co].a_hp=ch[co].hp[5]*500;
                ch[co].luck/=2;
                do_char_log(co,0,"A god reached down and saved you from the killing blow. You must have done the gods a favor sometime in the past!\n");
                do_area_log(co,0,ch[co].x,ch[co].y,0,"A god reached down and saved %s from the killing blow.\n",ch[co].reference);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
                god_transfer_char(co,ch[co].temple_x,ch[co].temple_y);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);

                chlog(co,"Saved by the Gods (new luck=%d)",ch[co].luck);
                ch[co].data[44]++;

                do_notify_char(cn,NT_DIDKILL,co,0,0,0);
                do_area_notify(cn,co,ch[cn].x,ch[cn].y,NT_SEEKILL,cn,co,0,0);

        } else ch[co].a_hp-=dam;

        if (ch[co].a_hp<8000 && ch[co].a_hp>=500)
                do_char_log(co,0,"You're almost dead... Give running a try!\n");

        if (ch[co].a_hp<500) {
                do_area_log(cn,co,ch[cn].x,ch[cn].y,0,"%s is dead!\n",ch[co].reference);
                do_char_log(cn,0,"You killed %s.\n",ch[co].reference);
                if (ch[cn].flags&CF_INVISIBLE) do_char_log(co,0,"Oh dear, that blow was fatal. Somebody killed you...\n");
                else do_char_log(co,0,"Oh dear, that blow was fatal. %s killed you...\n",ch[cn].name);
                do_notify_char(cn,NT_DIDKILL,co,0,0,0);
                do_area_notify(cn,co,ch[cn].x,ch[cn].y,NT_SEEKILL,cn,co,0,0);

                if (type!=2 && cn && !(mf&MF_ARENA) && !noexp) {
                        tmp=do_char_score(co);
                        rank=points2rank(ch[co].points_tot);

                        if (!ch[co].skill[SK_MEDIT][0]) {
                                for (n=0; n<20; n++) {
                                        if ((in=ch[co].spell[n])) {
                                                if (it[in].temp==SK_PROTECT || it[in].temp==SK_ENHANCE || it[in].temp==SK_BLESS)
                                                        tmp+=tmp/5;
                                        }
                                }
                        }

                }
                do_char_killed(cn,co);


                if (type!=2 && cn && cn!=co && !(mf&MF_ARENA) && !noexp) do_give_exp(cn,tmp,1,rank);

                ch[cn].cerrno=ERR_SUCCESS;
        } else {
                if (type==0 && ch[co].gethit_dam>0) do_hurt(co,cn,RANDOM(ch[co].gethit_dam)+1,3);
        }

        return dam/1000;
}

void do_attack(int cn,int co,int surround)
{
        int hit,dam=0,die,m,odam=0;
        int chance,s1,s2,bonus=0,diff;

        if (!may_attack_msg(cn,co,1)) {
                chlog(cn,"Prevented from attacking %s (%d)",ch[co].name,co);
                ch[cn].attack_cn=0;
                ch[cn].cerrno=ERR_FAILED;
                return;
        }

        if (ch[co].flags&CF_STONED) {
                ch[cn].attack_cn=0;
                ch[cn].cerrno=ERR_FAILED;
                return;
        }

        if (ch[cn].current_enemy!=co) {
                ch[cn].current_enemy=co; // reset current_enemy whenever char does something different !!!

                chlog(cn,"Attacks %s (%d)",ch[co].name,co);
        }

        add_enemy(co,cn);

        s1=get_fight_skill(cn);
        s2=get_fight_skill(co);

        if (globs->flags&GF_MAYHEM) {
                if (!(ch[cn].flags&CF_PLAYER)) s1+=10;
                if (!(ch[co].flags&CF_PLAYER)) s2+=10;
        }

        if (ch[cn].flags&(CF_PLAYER)) if (ch[cn].luck<0) s1+=ch[cn].luck/250-1;

        if (ch[co].flags&(CF_PLAYER)) if (ch[co].luck<0) s2+=ch[co].luck/250-1;

        if (!is_facing(co,cn)) s2-=10;
        if (is_back(co,cn)) s2-=10;

        if (ch[co].stunned || !ch[co].attack_cn) s2-=10;

        diff=s1-s2;

        if      (diff<-40) { chance=1; bonus=-16; }
        else if (diff<-36) { chance=2; bonus=-8; }
        else if (diff<-32) { chance=3; bonus=-4; }
        else if (diff<-28) { chance=4; bonus=-2; }
        else if (diff<-24) { chance=5; bonus=-1; }
        else if (diff<-20) chance=6;
        else if (diff<-16) chance=7;
        else if (diff<-12) chance=8;
        else if (diff< -8) chance=9;
        else if (diff< -4) chance=10;
        else if (diff<  0) chance=11;
        else if (diff== 0) chance=12;
        else if (diff<  4) chance=13;
        else if (diff<  8) chance=14;
        else if (diff< 12) chance=15;
        else if (diff< 16) { chance=16; bonus=1; }
        else if (diff< 20) { chance=17; bonus=2; }
        else if (diff< 24) { chance=18; bonus=3; }
        else if (diff< 28) { chance=19; bonus=4; }
        else if (diff< 32) { chance=19; bonus=5; }
        else if (diff< 36) { chance=19; bonus=10; }
        else if (diff< 40) { chance=19; bonus=15; }
        else { chance=19; bonus=20; }

        /* chance=get_fight_skill(cn)*12/get_fight_skill(co);

        if (chance<1) chance=1;
        if (chance>19) chance=19; */

        die=RANDOM(20)+1;
        if (die<=chance) hit=1;
        else hit=0;

        if (hit) {
                dam=ch[cn].weapon+RANDOM(6)+1;
                if (ch[cn].attrib[AT_STREN][5]>3) dam+=RANDOM(ch[cn].attrib[AT_STREN][5]/2);
                if (die==2) dam+=RANDOM(6)+1;
                if (die==1) dam+=RANDOM(6)+RANDOM(6)+2;
                odam=dam;
                dam+=bonus;

                if (ch[cn].flags&(CF_PLAYER)) item_damage_weapon(cn,dam);

                dam=do_hurt(cn,co,dam,0);

                if (dam<1) {
                        do_area_sound(co,0,ch[co].x,ch[co].y,ch[cn].sound+3);
                        char_play_sound(co,ch[cn].sound+3,-150,0);
                } else {
                        do_area_sound(co,0,ch[co].x,ch[co].y,ch[cn].sound+4);
                        char_play_sound(co,ch[cn].sound+4,-150,0);
                }
                if (surround && ch[cn].skill[SK_SURROUND][0]) {
                        m=ch[cn].x+ch[cn].y*MAPX;
                        if ((co=map[m+1].ch)!=0 && ch[co].attack_cn==cn) {
                                if (ch[cn].skill[SK_SURROUND][5]+RANDOM(20)>get_fight_skill(co)) do_hurt(cn,co,odam-odam/4,0);
                        }
                        if ((co=map[m-1].ch)!=0 && ch[co].attack_cn==cn) {
                                if (ch[cn].skill[SK_SURROUND][5]+RANDOM(20)>get_fight_skill(co)) do_hurt(cn,co,odam-odam/4,0);
                        }
                        if ((co=map[m+MAPX].ch)!=0 && ch[co].attack_cn==cn) {
                                if (ch[cn].skill[SK_SURROUND][5]+RANDOM(20)>get_fight_skill(co)) do_hurt(cn,co,odam-odam/4,0);
                        }
                        if ((co=map[m-MAPX].ch)!=0 && ch[co].attack_cn==cn) {
                                if (ch[cn].skill[SK_SURROUND][5]+RANDOM(20)>get_fight_skill(co)) do_hurt(cn,co,odam-odam/4,0);
                        }
                }
        } else {
                do_area_sound(co,0,ch[co].x,ch[co].y,ch[cn].sound+5);
                char_play_sound(co,ch[cn].sound+5,-150,0);

                do_area_notify(cn,co,ch[cn].x,ch[cn].y,NT_SEEMISS,cn,co,0,0);
                do_notify_char(co,NT_GOTMISS,cn,0,0,0);
                do_notify_char(cn,NT_DIDMISS,co,0,0,0);
        }
}

int do_maygive(int cn,int co,int in)
{
	if (in<1 || in>=MAXITEM) return 1;
	
        if (it[in].temp==IT_LAGSCROLL) return 0; // lag scroll

        return 1;
}

void do_give(int cn,int co)
{
        int tmp,in;

        if (!ch[cn].citem) { ch[cn].cerrno=ERR_FAILED; return; }
        in=ch[cn].citem;

        ch[cn].cerrno=ERR_SUCCESS;

	do_update_char(cn);
	do_update_char(co);

        if (in&0x80000000) {
                tmp=in&0x7FFFFFFF;
                ch[co].gold+=tmp;
                do_char_log(cn,1,"You give the gold to %s.\n",ch[co].name);
                do_char_log(co,0,"You got %dG %dS from %s.\n",
                        tmp/100,tmp%100,ch[cn].name);
                if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Gives %s (%d) %dG %dS",ch[co].name,co,tmp/100,tmp%100);

                do_notify_char(co,NT_GIVE,cn,0,tmp,0);

                ch[cn].citem=0;

		do_update_char(cn);

                return;
        }

        if (!do_maygive(cn,co,in)) {
                do_char_log(cn,0,"You're not allowed to do that!\n");
                ch[cn].misc_action=DR_IDLE;
                return;
        }

        chlog(cn,"Gives %s (%d) to %s (%d)",it[in].name,in,ch[co].name,co);

        if (it[in].driver==31 && (ch[co].flags&CF_UNDEAD)) {
                if (ch[cn].flags&CF_NOMAGIC) {
                        do_char_log(cn,0,"It doesn't work! An evil aura is present.\n");
                        ch[cn].misc_action=DR_IDLE;
                        return;
                }
                do_hurt(cn,co,it[in].data[0],2);
                it[in].used=USE_EMPTY;
                ch[cn].citem=0;
                return;
        }

        if ((ch[co].flags&(CF_PLAYER)) && (it[in].flags&IF_SHOPDESTROY)) {
                do_char_log(cn,0,"Beware! The gods see what you're doing.\n");
        }

        if (ch[co].citem) {
                tmp=god_give_char(in,co);

                if (tmp) {
                        ch[cn].citem=0;
                        do_char_log(cn,1,"You give %s to %s.\n",
                                it[in].name,ch[co].name);
                } else ch[cn].misc_action=DR_IDLE;
        } else {
                ch[cn].citem=0;
                ch[co].citem=in;
                it[in].carried=co;

		do_update_char(cn);

                do_char_log(cn,1,"You give %s to %s.\n",it[in].name,ch[co].name);
        }
        do_notify_char(co,NT_GIVE,cn,in,0,0);
}

int invis_level(int cn)
{
	if (ch[cn].flags&CF_GREATERINV) return 15;
        if (ch[cn].flags&CF_GOD) return 10;
        if (ch[cn].flags&(CF_IMP|CF_USURP)) return 5;
        if (ch[cn].flags&CF_STAFF) return 2;
        return 1;
}

int do_char_can_see(int cn,int co)
{
        int d,d1,d2,light,rd;
        unsigned long long prof;

        if (cn==co) return 1;

        if (ch[co].used!=USE_ACTIVE) return 0;
        if ((ch[co].flags&CF_INVISIBLE) && invis_level(cn)<invis_level(co)) return 0;
        if (ch[co].flags&CF_BODY) return 0;

        prof=prof_start();

        // raw distance:
        d1=abs(ch[cn].x-ch[co].x);

        d2=abs(ch[cn].y-ch[co].y);

        rd=d=d1*d1+d2*d2;

        if (d>1000) { prof_stop(21,prof); return 0; } // save some time...

        // modify by perception and stealth:
        if (ch[co].mode==0) d=(d*(ch[co].skill[SK_STEALTH][5]+20))/20;
        else if (ch[co].mode==1) d=(d*(ch[co].skill[SK_STEALTH][5]+50))/50;
        else d=(d*(ch[co].skill[SK_STEALTH][5]+100))/100;

        d-=ch[cn].skill[SK_PERCEPT][5]*2;

        // modify by light:
        if (!(ch[cn].flags&CF_INFRARED)) {
                light=max(map[ch[co].x+ch[co].y*MAPX].light,check_dlight(ch[co].x,ch[co].y));
                light=do_char_calc_light(cn,light);

                if (light==0) { prof_stop(21,prof); return 0; }
                if (light>64) light=64;
                d+=(64-light)*2;
        }

        if (rd<3 && d>70) d=70;
        if (d>200) { prof_stop(21,prof); return 0; }

        if (!can_see(cn,ch[cn].x,ch[cn].y,ch[co].x,ch[co].y,15)) { prof_stop(21,prof); return 0; }

        prof_stop(21,prof);

        if (d<1) return 1;

        return d;
}

int do_char_can_see_item(int cn,int in)
{
        int d,d1,d2,light,rd;
        unsigned long long prof;

        if (it[in].used!=USE_ACTIVE) return 0;

        // raw distance:
        d1=abs(ch[cn].x-it[in].x);

        d2=abs(ch[cn].y-it[in].y);

        rd=d=d1*d1+d2*d2;

        if (d>1000) return 0; // save some time...

        prof=prof_start();

        // modify by perception
        d+=50-ch[cn].skill[SK_PERCEPT][5]*2;

        // modify by light:
        if (!(ch[cn].flags&CF_INFRARED)) {
                light=max(map[it[in].x+it[in].y*MAPX].light,check_dlight(it[in].x,it[in].y));
                light=do_char_calc_light(cn,light);

                if (light==0) { prof_stop(22,prof); return 0; }
                if (light>64) light=64;
                d+=(64-light)*3;
        }

        if (it[in].flags&IF_HIDDEN) {
                d+=it[in].data[9];
        } else if (rd<3 && d>200) d=200;

        if (d>200) { prof_stop(22,prof); return 0; }

        if (!can_see(cn,ch[cn].x,ch[cn].y,it[in].x,it[in].y,15)) { prof_stop(22,prof); return 0; }

        prof_stop(22,prof);

        if (d<1) return 1;

        return d;
}


void do_update_char(int cn)
{
	ch[cn].flags|=(CF_UPDATE|CF_SAVEME);
}

void really_update_char(int cn)
{
        int n,m,oldlight,z,sublight=0;
        int hp=0,end=0,mana=0,weapon=0,armor=0,light=0,gethit=0,infra=0;
        int attrib[5];
        int skill[50];
        unsigned long long prof;
	
        prof=prof_start();

        ch[cn].flags&=~(CF_NOHPREG|CF_NOENDREG|CF_NOMANAREG);
        ch[cn].sprite_override=0;

        m=ch[cn].x+ch[cn].y*MAPX;

        if ((map[m].flags&MF_NOMAGIC) && !char_wears_item(cn,466) && !char_wears_item(cn,481)) {
                if (!(ch[cn].flags&CF_NOMAGIC)) {
                        ch[cn].flags|=CF_NOMAGIC;
                        remove_spells(cn);
                        do_char_log(cn,0,"You feel your magic fail.\n");
                }
        } else {
                if (ch[cn].flags&CF_NOMAGIC) {
                        ch[cn].flags&=~CF_NOMAGIC;
                        do_update_char(cn);
                        do_char_log(cn,0,"You feel your magic return.\n");
                }
        }

        oldlight=ch[cn].light;

        for (n=0; n<5; n++) {
                ch[cn].attrib[n][4]=0;
                attrib[n]=0;
        }

        ch[cn].hp[4]=0; hp=0;
        ch[cn].end[4]=0; end=0;
        ch[cn].mana[4]=0; mana=0;

        for (n=0; n<50; n++) {
                ch[cn].skill[n][4]=0;
                skill[n]=0;
        }

        ch[cn].armor=0; armor=0;
        ch[cn].weapon=0; weapon=0;
        ch[cn].gethit_dam=0; gethit=0;
        ch[cn].stunned=0;
        ch[cn].light=0; light=0;

        for (n=0; n<20; n++) {
                if (!ch[cn].worn[n]) continue;
                m=ch[cn].worn[n];

                if (!(ch[cn].flags&CF_NOMAGIC)) {

                        for (z=0; z<5; z++) {
                                if (it[m].active) attrib[z]+=it[m].attrib[z][1];
                                else attrib[z]+=it[m].attrib[z][0];
                        }

                        if (it[m].active) hp+=it[m].hp[1];
                        else hp+=it[m].hp[0];

                        if (it[m].active) end+=it[m].end[1];
                        else end+=it[m].end[0];

                        if (it[m].active) mana+=it[m].mana[1];
                        else mana+=it[m].mana[0];

                        for (z=0; z<50; z++) {
                                if (it[m].active) skill[z]+=it[m].skill[z][1];
                                else skill[z]+=it[m].skill[z][0];
                        }
                }

                if (it[m].active) {
                        armor+=it[m].armor[1];
                        gethit+=it[m].gethit_dam[1];
                        if (it[m].weapon[1]>weapon) weapon=it[m].weapon[1];
                        if (it[m].light[1]>light) light=it[m].light[1];
                        else if (it[m].light[1]<0) sublight-=it[m].light[1];
                } else {
                        armor+=it[m].armor[0];
                        gethit+=it[m].gethit_dam[0];
                        if (it[m].weapon[0]>weapon) weapon=it[m].weapon[0];
                        if (it[m].light[0]>light) light=it[m].light[0];
                        else if (it[m].light[0]<0) sublight-=it[m].light[0];
                }
        }

        armor+=ch[cn].armor_bonus;
        weapon+=ch[cn].weapon_bonus;
        gethit+=ch[cn].gethit_bonus;
        light+=ch[cn].light_bonus;

        if (!(ch[cn].flags&CF_NOMAGIC)) {
                for (n=0; n<20; n++) {
                        if (!ch[cn].spell[n]) continue;
                        m=ch[cn].spell[n];

                        for (z=0; z<5; z++) attrib[z]+=it[m].attrib[z][1];

                        hp+=it[m].hp[1];
                        end+=it[m].end[1];
                        mana+=it[m].mana[1];

                        for (z=0; z<50; z++) skill[z]+=it[m].skill[z][1];

                        armor+=it[m].armor[1];
                        weapon+=it[m].weapon[1];
                        if (it[m].light[1]>light) light=it[m].light[1];
                        else if (it[m].light[1]<0) sublight-=it[m].light[1];

                        if (it[m].temp==SK_STUN || it[m].temp==SK_WARCRY2) ch[cn].stunned=1;

                        if (it[m].hp[0]<0) ch[cn].flags|=CF_NOHPREG;
                        if (it[m].end[0]<0) ch[cn].flags|=CF_NOENDREG;
                        if (it[m].mana[0]<0) ch[cn].flags|=CF_NOMANAREG;

                        if (it[m].sprite_override) ch[cn].sprite_override=it[m].sprite_override;

                        if (it[m].temp==635) infra|=1;
                        if (it[m].temp==637) infra|=2;
                        if (it[m].temp==639) infra|=4;
                        if (it[m].temp==641) infra|=8;
                }
        }

        for (z=0; z<5; z++) {
                attrib[z]=(int)ch[cn].attrib[z][0]+(int)ch[cn].attrib[z][1]+attrib[z];
                if (attrib[z]<1) attrib[z]=1;
                if (attrib[z]>250) attrib[z]=250;
                ch[cn].attrib[z][5]=attrib[z];
        }

        hp=(int)ch[cn].hp[0]+(int)ch[cn].hp[1]+hp;
        if (hp<10) hp=10;
        if (hp>999) hp=999;
        ch[cn].hp[5]=hp;

        end=(int)ch[cn].end[0]+(int)ch[cn].end[1]+end;
        if (end<10) end=10;
        if (end>999) end=999;
        ch[cn].end[5]=end;

        mana=(int)ch[cn].mana[0]+(int)ch[cn].mana[1]+mana;
        if (mana<10) mana=10;
        if (mana>999) mana=999;
        ch[cn].mana[5]=mana;

        if (ch[cn].flags&(CF_PLAYER)) {
                if (infra==15 && !(ch[cn].flags&CF_INFRARED)) {
                        ch[cn].flags|=CF_INFRARED;
                        do_char_log(cn,0,"You can see in the dark!\n");
                }
                if (infra!=15 && (ch[cn].flags&CF_INFRARED) && !(ch[cn].flags&CF_GOD)) {
                        ch[cn].flags&=~CF_INFRARED;
                        do_char_log(cn,0,"You can no longer see in the dark!\n");
                }
        }

        for (z=0; z<50; z++) {
                skill[z]=(int)ch[cn].skill[z][0]+(int)ch[cn].skill[z][1]+skill[z];

                skill[z]+=      ((int)ch[cn].attrib[skilltab[z].attrib[0]][5]+
                                 (int)ch[cn].attrib[skilltab[z].attrib[1]][5]+
                                 (int)ch[cn].attrib[skilltab[z].attrib[2]][5])/5;

                if (skill[z]<1) skill[z]=1;
                if (skill[z]>250) skill[z]=250;
                ch[cn].skill[z][5]=skill[z];
        }

        if (armor<0) armor=0;
        if (armor>250) armor=250;
        ch[cn].armor=armor;

        if (weapon<0) weapon=0;
        if (weapon>250) weapon=250;
        ch[cn].weapon=weapon;

        if (gethit<0) gethit=0;
        if (gethit>250) gethit=250;
        ch[cn].gethit_dam=gethit;

        light-=sublight;
        if (light<0) light=0;
        if (light>250) light=250;
        ch[cn].light=light;

        n=10;
        if (ch[cn].mode==0) {
                n=(ch[cn].attrib[AT_AGIL][5]+ch[cn].attrib[AT_STREN][5])/50+ch[cn].speed_mod+12;
        }
        if (ch[cn].mode==1) { // normal
                n=(ch[cn].attrib[AT_AGIL][5]+ch[cn].attrib[AT_STREN][5])/50+ch[cn].speed_mod+14;
        }
        if (ch[cn].mode==2) { // fast
                n=(ch[cn].attrib[AT_AGIL][5]+ch[cn].attrib[AT_STREN][5])/50+ch[cn].speed_mod+16;
        }

        ch[cn].speed=20-n;

        if (ch[cn].speed<0) ch[cn].speed=0;
        if (ch[cn].speed>19) ch[cn].speed=19;

        if (ch[cn].a_hp>ch[cn].hp[5]*1000) ch[cn].a_hp=ch[cn].hp[5]*1000;
        if (ch[cn].a_end>ch[cn].end[5]*1000) ch[cn].a_end=ch[cn].end[5]*1000;
        if (ch[cn].a_mana>ch[cn].mana[5]*1000) ch[cn].a_mana=ch[cn].mana[5]*1000;

        if (oldlight!=ch[cn].light && ch[cn].used==USE_ACTIVE &&
            ch[cn].x>0 && ch[cn].x<MAPX && ch[cn].y>0 && ch[cn].y<MAPY &&
            map[ch[cn].x+ch[cn].y*MAPX].ch==cn)
                do_add_light(ch[cn].x,ch[cn].y,ch[cn].light-oldlight);

       prof_stop(7,prof);
}

// note: this calculates ALL normal endurance/hp changes.
//       further, it is called ONLY from tick()
void do_regenerate(int cn)
{
        int n,in,nohp=0,noend=0,nomana=0,old,hp=0,end=0,mana=0,uwater=0,gothp=0;
        int moonmult=20;
        unsigned long long prof;

// !!!!!        if ((ch[cn].flags&CF_STONED) && !(ch[cn].flags&CF_PLAYER)) ch[cn].flags&=~CF_STONED;

        if (ch[cn].flags&CF_STONED) return;

        prof=prof_start();

        if (((globs->flags&GF_MAYHEM) || globs->newmoon) && (ch[cn].flags&(CF_PLAYER))) moonmult=10;
        if (globs->fullmoon && (ch[cn].flags&(CF_PLAYER))) moonmult=40;

        if (ch[cn].flags&CF_NOHPREG) nohp=1;
        if (ch[cn].flags&CF_NOENDREG) noend=1;
        if (ch[cn].flags&CF_NOMANAREG) nomana=1;

        if (map[ch[cn].x+ch[cn].y*MAPX].flags&MF_UWATER) uwater=1;

        if (!ch[cn].stunned) {

                switch(ch_base_status(ch[cn].status)) {
                        case    0:
                        case    1:
                        case    2:
                        case    3:
                        case    4:
                        case    5:
                        case    6:
                        case    7:      if (!noend) {
                                                end=1;

                                                ch[cn].a_end+=moonmult*4;

                                                if (!noend && ch[cn].skill[SK_REST][0]) {
                                                        ch[cn].a_end+=ch[cn].skill[SK_REST][5]*moonmult/30;
                                                }
                                        }

                                        if (!nohp) {
                                                hp=1;

                                                ch[cn].a_hp+=moonmult*2;
                                                gothp+=moonmult;

                                                if (ch[cn].skill[SK_REGEN][0]) {
                                                        ch[cn].a_hp+=ch[cn].skill[SK_REGEN][5]*moonmult/30;
                                                        gothp+=ch[cn].skill[SK_REGEN][5]*moonmult/30;
                                                }
                                        }

                                        if (!nomana && ch[cn].skill[SK_MEDIT][0]) {
                                                mana=1;
                                                ch[cn].a_mana+=moonmult;
                                                ch[cn].a_mana+=ch[cn].skill[SK_MEDIT][5]*moonmult/30;
                                        }
                                        break;

                        case    16:
                        case    24:
                        case    32:
                        case    40:
                        case    48:
                        case    60:
                        case    72:
                        case    84:
                        case    96:
                        case    100:
                        case    104:
                        case    108:
                        case    112:
                        case    116:
                        case    120:
                        case    124:
                        case    128:
                        case    132:
                        case    136:
                        case    140:
                        case    144:
                        case    148:
                        case    152:    if (ch[cn].mode==2) {
                                                ch[cn].a_end-=25;
                                        } else if (ch[cn].mode==0) {
                                                if (!noend) { ch[cn].a_end+=25; end=1; }
                                        }
                                        break;

                        case    160:
                        case    168:
                        case    176:
                        case    184:    if (ch[cn].status2==0 || ch[cn].status2==5 || ch[cn].status2==6) { // attack
                                                if (ch[cn].mode==1) {
                                                        ch[cn].a_end-=12;
                                                } else if (ch[cn].mode==2) {
                                                        ch[cn].a_end-=50;
                                                }
                                        } else { // misc
                                                if (ch[cn].mode==2) ch[cn].a_end-=25;
                                                else if (ch[cn].mode==0) {
                                                        if (!noend) { ch[cn].a_end+=25; end=1; }
                                                }
                                        }
                                        break;
                        default:        fprintf(stderr,"do_regenerate(): unknown ch_base_status %d.\n",ch_base_status(ch[cn].status));
                                        break;
                }
        }

        if (ch[cn].flags&CF_UNDEAD) { ch[cn].a_hp+=650; hp=1; gothp+=650; }

        if ((in=ch[cn].worn[WN_NECK]) && it[in].temp==768) { // amulet of ankh
                if (ch[cn].skill[SK_REGEN][0]) ch[cn].a_hp+=ch[cn].skill[SK_REGEN][5]*moonmult/60;
                if (ch[cn].skill[SK_REST][0]) ch[cn].a_end+=ch[cn].skill[SK_REST][5]*moonmult/60;
                if (ch[cn].skill[SK_MEDIT][0]) ch[cn].a_mana+=ch[cn].skill[SK_MEDIT][5]*moonmult/60;
        }

        // force to sane values
        if (ch[cn].a_hp>ch[cn].hp[5]*1000) ch[cn].a_hp=ch[cn].hp[5]*1000;
        if (ch[cn].a_end>ch[cn].end[5]*1000) ch[cn].a_end=ch[cn].end[5]*1000;
        if (ch[cn].a_mana>ch[cn].mana[5]*1000) ch[cn].a_mana=ch[cn].mana[5]*1000;

        if (hp && ch[cn].a_hp<ch[cn].hp[5]*900) ch[cn].data[92]=TICKS*60;
        if (mana && ch[cn].a_mana<ch[cn].mana[5]*900) ch[cn].data[92]=TICKS*60;

        if (ch[cn].a_end<1500 && ch[cn].mode!=0) {
                ch[cn].mode=0;
                do_update_char(cn);
                do_char_log(cn,0,"You're exhausted.\n");
        }

        if (ch[cn].escape_timer>0) ch[cn].escape_timer--;

        // spell effects
        for (n=0; n<20; n++) {
                if ((in=ch[cn].spell[n])!=0) {
//                      ch[cn].data[92]=TICKS*60;
                        if (it[in].flags&IF_PERMSPELL) {
                                if (it[in].hp[0]!=-1) ch[cn].a_hp+=it[in].hp[0];
                                if (it[in].end[0]!=-1) ch[cn].a_end+=it[in].end[0];
                                if (it[in].mana[0]!=-1) ch[cn].a_mana+=it[in].mana[0];
                                if (ch[cn].a_hp<500) {
                                        chlog(cn,"killed by spell: %s",it[in].name);
                                        do_char_log(cn,0,"The %s killed you!\n",it[in].name);
                                        do_area_log(cn,0,ch[cn].x,ch[cn].y,0,"The %s killed %s.\n",it[in].name,ch[cn].reference);
                                        do_char_killed(0,cn);
                                        return;
                                }
                                if (ch[cn].a_end<500) {
                                        ch[cn].a_end=500;
                                        it[in].active=0;
                                        chlog(cn,"%s ran out due to lack of endurance",it[in].name);
                                }
                                if (ch[cn].a_mana<500) {
                                        ch[cn].a_mana=500;
                                        it[in].active=0;
                                        chlog(cn,"%s ran out due to lack of mana",it[in].name);
                                }
                        } else {
                                it[in].active--;
                                if (it[in].active==TICKS*30) {
                                        if (ch[cn].flags&(CF_PLAYER|CF_USURP)) {
                                                do_char_log(cn,0,"%s is about to run out.\n",it[in].name);
                                        } else {
                                                int co;

                                                if (ch[cn].temp==CT_COMPANION && (co=ch[cn].data[63]) &&
                                                    IS_SANEPLAYER(co) && (it[in].temp==SK_BLESS ||
                                                     it[in].temp==SK_PROTECT || it[in].temp==SK_ENHANCE)) {
                                                        do_sayx(cn,"My spell %s is running out, %s.",
                                                                it[in].name,ch[co].name);
                                                }
                                        }
                                }
                        }

                        if (it[in].temp==649) uwater=0;

                        if (it[in].temp==SK_MSHIELD) {
                                old=it[in].armor[1];
                                it[in].armor[1]=it[in].active/1024+1;
                                it[in].power=it[in].active/256;
                                if (old!=it[in].armor[1]) do_update_char(cn);
                        }

                        if (!it[in].active) {
                                if (it[in].temp==SK_RECALL && ch[cn].used==USE_ACTIVE) {
                                        int xo,yo;

                                        xo=ch[cn].x; yo=ch[cn].y;

                                        if (god_transfer_char(cn,it[in].data[0],it[in].data[1])) {
                                                if (!(ch[cn].flags&CF_INVISIBLE)) {
                                                        fx_add_effect(12,0,xo,yo,0);
                                                        fx_add_effect(12,0,ch[cn].x,ch[cn].y,0);
                                                }
                                        }
                                        ch[cn].status=0; ch[cn].attack_cn=0; ch[cn].skill_nr=0; ch[cn].goto_x=0;
                                        ch[cn].use_nr=0; ch[cn].misc_action=0; ch[cn].dir=DX_DOWN;

                                } else do_char_log(cn,0,"%s ran out.\n",it[in].name);
                                it[in].used=USE_EMPTY;
                                ch[cn].spell[n]=0;
                                do_update_char(cn);
                        }
                }
        }

        if (uwater && (ch[cn].flags&(CF_PLAYER))) {
                ch[cn].a_hp-=250+gothp;
                if (ch[cn].a_hp<500) do_char_killed(0,cn);
        }

        // item tear and wear
        if (ch[cn].used==USE_ACTIVE && (ch[cn].flags&(CF_PLAYER))) char_item_expire(cn);

        prof_stop(8,prof);
}

int attrib_needed(int v,int diff)
{
        return v*v*v*diff/20;
}

int hp_needed(int v,int diff)
{
        return v*diff;
}

int end_needed(int v,int diff)
{
        return v*diff/2;
}

int mana_needed(int v,int diff)
{
        return v*diff;
}

int skill_needed(int v,int diff)
{
        return max(v,v*v*v*diff/40);
}

int do_raise_attrib(int cn,int nr)
{
        int p,v;

        v=ch[cn].attrib[nr][0];

        if (!v || v>=ch[cn].attrib[nr][2]) return 0;

        p=attrib_needed(v,ch[cn].attrib[nr][3]);

        if (p>ch[cn].points) return 0;

        ch[cn].points-=p;
        ch[cn].attrib[nr][0]++;

	do_update_char(cn);
        return 1;
}

int do_raise_hp(int cn)
{
        int p,v;

        v=ch[cn].hp[0];

        if (!v || v>=ch[cn].hp[2]) return 0;

        p=hp_needed(v,ch[cn].hp[3]);
        if (p>ch[cn].points) return 0;

        ch[cn].points-=p;
        ch[cn].hp[0]++;

	do_update_char(cn);
        return 1;
}

int do_lower_hp(int cn)
{
        int p,v;

        if (ch[cn].hp[0]<11) return 0;

        ch[cn].hp[0]--;

        v=ch[cn].hp[0];

        p=hp_needed(v,ch[cn].hp[3]);

        ch[cn].points_tot-=p;

	do_update_char(cn);
        return 1;
}

int do_lower_mana(int cn)
{
        int p,v;

        if (ch[cn].mana[0]<11) return 0;

        ch[cn].mana[0]--;

        v=ch[cn].mana[0];

        p=mana_needed(v,ch[cn].mana[3]);

        ch[cn].points_tot-=p;

	do_update_char(cn);
        return 1;
}

int do_raise_end(int cn)
{
        int p,v;

        v=ch[cn].end[0];

        if (!v || v>=ch[cn].end[2]) return 0;

        p=end_needed(v,ch[cn].end[3]);
        if (p>ch[cn].points) return 0;

        ch[cn].points-=p;
        ch[cn].end[0]++;

	do_update_char(cn);
        return 1;
}

int do_raise_mana(int cn)
{
        int p,v;

        v=ch[cn].mana[0];

        if (!v || v>=ch[cn].mana[2]) return 0;

        p=mana_needed(v,ch[cn].mana[3]);
        if (p>ch[cn].points) return 0;

        ch[cn].points-=p;
        ch[cn].mana[0]++;

	do_update_char(cn);
        return 1;
}

int do_raise_skill(int cn,int nr)
{
        int p,v;

        v=ch[cn].skill[nr][0];

        if (!v || v>=ch[cn].skill[nr][2]) return 0;

        p=skill_needed(v,ch[cn].skill[nr][3]);

        if (p>ch[cn].points) return 0;

        ch[cn].points-=p;
        ch[cn].skill[nr][0]++;

	do_update_char(cn);
        return 1;
}

int do_item_value(int in)
{
        if (in<1 || in>=MAXITEM) return 0;
        return it[in].value;
}


void do_look_item(int cn,int in)
{
        int n,in2,flag=0,act;

        if (it[in].active) act=1;
        else act=0;

        for (n=0; n<40; n++)
                if (ch[cn].item[n]==in) { flag=1; break; }
        for (n=0; n<20 && !flag; n++)
                if (ch[cn].worn[n]==in) { flag=1; break; }
        if (!flag && !do_char_can_see_item(cn,in)) return;

        if (it[in].flags&IF_LOOKSPECIAL) look_driver(cn,in);
        else {
                do_char_log(cn,1,"%s\n",it[in].description);
                if (it[in].max_age[act] || it[in].max_damage) {
                        if (it[in].damage_state==0) do_char_log(cn,1,"It's in perfect condition.\n");
                        else if (it[in].damage_state==1) do_char_log(cn,1,"It's showing signs of age.\n");
                        else if (it[in].damage_state==2) do_char_log(cn,1,"It's fairly old.\n");
                        else if (it[in].damage_state==3) do_char_log(cn,1,"It is old.\n");
                        else if (it[in].damage_state==4) do_char_log(cn,0,"It is very old and battered.\n");
                }
                if (IS_BUILDING(cn)) {
                        do_char_log(cn,1,"Temp: %d, Sprite: %d,%d.\n",it[in].temp,it[in].sprite[0],it[in].sprite[1]);
                        do_char_log(cn,1,"In-Active Age %d of %d.\n",it[in].current_age[0],it[in].max_age[0]);
                        do_char_log(cn,1,"Active Age %d of %d.\n",it[in].current_age[1],it[in].max_age[1]);
                        do_char_log(cn,1,"Damage %d of %d.\n",it[in].current_damage,it[in].max_damage);
                        do_char_log(cn,1,"Active %d of %d.\n",it[in].active,it[in].duration);
                        do_char_log(cn,1,"Driver=%d [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d].\n",
                                it[in].driver,it[in].data[0],it[in].data[1],it[in].data[2],it[in].data[3],it[in].data[4],
                                        it[in].data[5],it[in].data[6],it[in].data[7],it[in].data[8],it[in].data[9]);
                }

                if (ch[cn].flags&CF_GOD) {
                        do_char_log(cn,2,"ID=%d, Temp=%d, Value: %dG %dS.\n",in,it[in].temp,it[in].value/100,it[in].value%100);
                        do_char_log(cn,2,"active=%d, sprite=%d/%d\n",it[in].active,it[in].sprite[0],it[in].sprite[1]);
                        do_char_log(cn,2,"max_age=%d/%d, current_age=%d/%d\n",it[in].max_age[0],it[in].max_age[1],it[in].current_age[0],it[in].current_age[1]);
                        do_char_log(cn,2,"max_damage=%d, current_damage=%d\n",it[in].max_damage,it[in].current_damage);			
                }

                in2=ch[cn].citem;
                /* CS, 000208: Check for sane item */
                if (IS_SANEITEM(in2)) {
                        do_char_log(cn,1," \n");
                        do_char_log(cn,1,"You compare it with a %s:\n",it[in2].name);
                        if (it[in].weapon[0]>it[in2].weapon[0])
                                do_char_log(cn,1,"A %s is the better weapon.\n",it[in].name);
                        else if (it[in].weapon[0]<it[in2].weapon[0])
                                do_char_log(cn,1,"A %s is the better weapon.\n",it[in2].name);
                        else do_char_log(cn,1,"No difference as a weapon.\n");

                        if (it[in].armor[0]>it[in2].armor[0])
                                do_char_log(cn,1,"A %s is the better armor.\n",it[in].name);
                        else if (it[in].armor[0]<it[in2].armor[0])
                                do_char_log(cn,1,"A %s is the better armor.\n",it[in2].name);
                        else do_char_log(cn,1,"No difference as armor.\n");
                } else {
                        if (it[in].flags & IF_IDENTIFIED)
                                item_info(cn, in, 1);
                }
                // Remote scan of tombstones
                if (it[in].temp == IT_TOMBSTONE && it[in].data[0]) {

                        do_ransack_corpse(cn, it[in].data[0], "In the tombstone you notice %s!\n");
                }
		if (it[in].driver==57) {
			int percent;

			percent=min(100,100*(ch[cn].points_tot/10)/(it[in].data[4]+1));

			if (percent<50) do_char_log(cn,2,"You sense that it's too early in your career to touch this pole.\n");
			else if (percent<70) do_char_log(cn,2,"You sense that it's a bit early in your career to touch this pole.\n");			
		}
        }
}

int barter(int cn,int opr,int flag) // flag=1 merchant is selling, flag=0 merchant is buying
{
        int pr;

        if (flag) {
                pr=opr*4-(opr*ch[cn].skill[SK_BARTER][5])/50;
                if (pr<opr) pr=opr;
        } else {
                pr=opr/4+(opr*ch[cn].skill[SK_BARTER][5])/200;
                if (pr>opr) pr=opr;
        }

        return pr;
}

void do_shop_char(int cn,int co,int nr)
{
        int in,pr,in2,flag=0,tmp;

        if (co<=0 || co>=MAXCHARS || nr<0 || nr>=124) return;
        if (!(ch[co].flags&CF_MERCHANT) && !(ch[co].flags&CF_BODY)) return;
        if (!(ch[co].flags&CF_BODY) && !do_char_can_see(cn,co)) return;
        if ((ch[co].flags&CF_BODY) && abs(ch[cn].x-ch[co].x)+abs(ch[cn].y-ch[co].y)>1) return;

        if ((in=ch[cn].citem)!=0 && (ch[co].flags&CF_MERCHANT)) {
                if (in&0x80000000) {
                        do_char_log(cn,0,"You want to sell money? Weird!\n");
                        return;
                }

                in2=ch[co].data[0];
                if ((it[in].flags&IF_ARMOR) && (it_temp[in2].flags&IF_ARMOR)) flag=1;
                if ((it[in].flags&IF_WEAPON) && (it_temp[in2].flags&IF_WEAPON)) flag=1;
                if ((it[in].flags&IF_MAGIC) && (it_temp[in2].flags&IF_MAGIC)) flag=1;
                if ((it[in].flags&IF_MISC) && (it_temp[in2].flags&IF_MISC)) flag=1;
                if (!flag) {
                        do_char_log(cn,0,"%s doesn't buy those.\n",ch[co].name);
                        return;
                }
                pr=barter(cn,do_item_value(in),0);

                if (ch[co].gold<pr) {
                        do_char_log(cn,0,"%s cannot afford that.\n",ch[co].reference);
                        return;
                }
                ch[cn].citem=0;

                ch[cn].gold+=pr;

                god_give_char(in,co);
                chlog(cn,"Sold %s",it[in].name);
                do_char_log(cn,1,"You sold a %s for %dG %dS.\n",it[in].reference,pr/100,pr%100);

                tmp=it[in].temp;
                if (tmp>0 && tmp<MAXTITEM) it_temp[tmp].t_sold++;
        } else {
                if (nr<62) {
                        if (nr<40) {
                                if ((in=ch[co].item[nr])!=0) {
                                        if (ch[co].flags&CF_MERCHANT) {
                                                pr=barter(cn,do_item_value(in),1);
                                                if (ch[cn].gold<pr) {
                                                        do_char_log(cn,0,"You cannot afford that.\n");
                                                        return;
                                                }
                                        } else pr=0;
                                        god_take_from_char(in,co);
                                        if (god_give_char(in,cn)) {
                                                if (ch[co].flags&CF_MERCHANT) {

                                                        ch[cn].gold-=pr;

                                                        chlog(cn,"Bought %s",it[in].name);
                                                        do_char_log(cn,1,"You bought a %s for %dG %dS.\n",it[in].reference,pr/100,pr%100);

                                                        tmp=it[in].temp;
                                                        if (tmp>0 && tmp<MAXTITEM) it_temp[tmp].t_bought++;
                                                } else {
                                                        chlog(cn,"Took %s",it[in].name);
                                                        do_char_log(cn,1,"You took a %s.\n",it[in].reference);
                                                }
                                        } else {
                                                god_give_char(in,co);
                                                if (ch[co].flags&CF_MERCHANT)
                                                        do_char_log(cn,0,"You cannot buy the %s because your inventory is full.\n",it[in].reference);
                                                else
                                                        do_char_log(cn,0,"You cannot take the %s because your inventory is full.\n",it[in].reference);
                                        }
                                }
                        } else if (nr<60) {
                                if ((ch[co].flags&CF_BODY) && (in=ch[co].worn[nr-40])!=0) {
                                        god_take_from_char(in,co);
                                        if (god_give_char(in,cn)) {
                                                chlog(cn,"Took %s",it[in].name);
                                                do_char_log(cn,1,"You took a %s.\n",it[in].reference);
                                        } else {
                                                god_give_char(in,co);
                                                do_char_log(cn,0,"You cannot take the %s because your inventory is full.\n",it[in].reference);
                                        }
                                }
                        } else if (nr==60) {
                                if ((ch[co].flags&CF_BODY) && (in=ch[co].citem)!=0) {
                                        god_take_from_char(in,co);
                                        if (god_give_char(in,cn)) {
                                                chlog(cn,"Took %s",it[in].name);
                                                do_char_log(cn,1,"You took a %s.\n",it[in].reference);
                                        } else {
                                                god_give_char(in,co);
                                                do_char_log(cn,0,"You cannot take the %s because your inventory is full.\n",it[in].reference);
                                        }
                                }
                        } else {
                                if ((ch[co].flags&CF_BODY) && ch[co].gold) {
                                        ch[cn].gold+=ch[co].gold;
                                        chlog(cn,"Took %dG %dS",ch[co].gold/100,ch[co].gold%100);
                                        do_char_log(cn,1,"You took %dG %dS.\n",ch[co].gold/100,ch[co].gold%100);
                                        ch[co].gold=0;
                                }
                        }
                } else {
                        nr-=62;

                        if (nr<40) {
                                if ((in=ch[co].item[nr])!=0) {
                                        do_char_log(cn,1,"%s:\n",it[in].name);
                                        do_char_log(cn,1,"%s\n",it[in].description);
                                }
                        } else if (nr<61) {
                                if ((ch[co].flags&CF_BODY) && (in=ch[co].worn[nr-40])!=0) {
                                        do_char_log(cn,1,"%s:\n",it[in].name);
                                        do_char_log(cn,1,"%s\n",it[in].description);
                                }
                        } else {
                                if ((ch[co].flags&CF_BODY) && (in=ch[co].citem)!=0) {
                                        do_char_log(cn,1,"%s:\n",it[in].name);
                                        do_char_log(cn,1,"%s\n",it[in].description);
                                }
                        }
                }
        }
        if (ch[co].flags&CF_MERCHANT) update_shop(co);
        do_look_char(cn,co,0,0,1);
}

int do_depot_cost(int in)
{
        int cost;

        cost=1;
        cost+=it[in].value/1600;
        cost+=(it[in].power*it[in].power*it[in].power)/16000;

        if (it[in].flags&IF_LABYDESTROY) cost+=20000;

        return cost;
}

int do_add_depot(int cn,int in)
{
        int n;

        for (n=0; n<62; n++)
                if (!ch[cn].depot[n]) break;

        if (n==62) return 0;

        ch[cn].depot[n]=in;
	do_update_char(cn);

        return 1;
}

void do_pay_depot(int cn)
{
        int c,in,n,lv,ln,tmp;

        while (1) {
                c=get_depot_cost(cn);

                if (c>ch[cn].data[13]) {
                        lv=99999999; ln=-1;
                        for (n=0; n<62; n++) {
                                if ((in=ch[cn].depot[n])!=0) {
                                        tmp=do_item_value(in);
                                        if (tmp<lv) { lv=tmp; ln=n; }
                                }
                        }
                        if (ln==-1) { chlog(cn,"PANIC: depot forced sale failed!"); return; }
                        ch[cn].data[13]+=lv/2;
                        in=ch[cn].depot[ln];
                        it[in].used=USE_EMPTY;
                        ch[cn].depot[ln]=0;
                        ch[cn].depot_sold++;
                        chlog(cn,"Bank sold %s for %dG %dS to pay for depot (slot %d)",
                                it[in].name,(lv/2)/100,(lv/2)%100,ln);
                } else {
                        ch[cn].data[13]-=c;
                        ch[cn].depot_cost+=c;
                        break;
                }
        }
}

void do_depot_char(int cn,int co,int nr)
{
        int in,pr=0;

        if (co<=0 || co>=MAXCHARS || nr<0 || nr>=124) return;
        if (cn!=co) return;

        if (!(map[ch[cn].x+ch[cn].y*MAPX].flags&MF_BANK) && !(ch[cn].flags&CF_GOD)) {
                do_char_log(cn,0,"You cannot access your depot outside a bank.\n");
                return;
        }

        if ((in=ch[cn].citem)!=0) {

                if (in&0x80000000) {
                        do_char_log(cn,0,"Use #deposit to put money in the bank!\n");
                        return;
                }

                if (!do_maygive(cn,0,in) || (it[in].flags&IF_NODEPOT)) {
                        do_char_log(cn,0,"You are not allowed to do that!\n");
                        return;
                }

                pr=do_depot_cost(in);

                if (do_add_depot(co,in)) {
                        ch[cn].citem=0;
                        do_char_log(cn,1,"You deposited %s. The rent is %dG %dS per Astonian day or %dG %dS per Earth day.\n",
                                it[in].reference,
                                pr/100, pr%100, (pr*18)/100, (pr*18)%100);
                        chlog(cn,"Deposited %s",it[in].name);
                }
        } else {
                if (nr<62) {
                        if ((in=ch[co].depot[nr])!=0) {
                                if (god_give_char(in,cn)) {
                                        ch[co].depot[nr]=0;
                                        do_char_log(cn,1,"You took the %s from your depot.\n",it[in].reference);
                                        chlog(cn,"Took %s from depot",it[in].name);
                                } else do_char_log(cn,0,"You cannot take the %s because your inventory is full.\n",it[in].reference);
                        }

                } else {
                        nr-=62;

                        if ((in=ch[co].depot[nr])!=0) {
                                do_char_log(cn,1,"%s:\n",it[in].name);
                                do_char_log(cn,1,"%s\n",it[in].description);
                        }
                }
        }
}

void do_look_char(int cn,int co,int godflag,int autoflag,int lootflag)
{
        int p,n,nr,hp_diff=0,end_diff=0,mana_diff=0,in,pr,spr,m;
        char buf[16],*killer;

        if (co<=0 || co>=MAXCHARS) return;

        if ((ch[co].flags&CF_BODY) && abs(ch[cn].x-ch[co].x)+abs(ch[cn].y-ch[co].y)>1) return;
	if ((ch[co].flags&CF_BODY) && !lootflag) return;

        if (godflag || (ch[co].flags&CF_BODY)) p=1;
        else p=do_char_can_see(cn,co);
        if (!p) return;

        if (!autoflag && !(ch[co].flags&CF_MERCHANT) && !(ch[co].flags&CF_BODY)) {
		if ((ch[cn].flags&CF_PLAYER) && !autoflag) {
			ch[cn].data[71]+=CNTSAY;
			if (ch[cn].data[71]>MAXSAY) {
				do_char_log(cn,0,"Oops, you're a bit too fast for me!\n");
				return;
			}
		}

                if (ch[co].description[0]) do_char_log(cn,1,"%s\n",ch[co].description);
                else do_char_log(cn,1,"You see %s.\n",ch[co].reference);

                if (IS_PLAYER(co) && ch[co].data[0]) {
                        if (ch[co].text[0][0]) {
                                do_char_log(cn,0,"%s is away from keyboard; Message:\n",ch[co].name);
                                do_char_log(cn,3,"  \"%s\"\n", ch[co].text[0]);
                        } else {
                                do_char_log(cn,0,"%s is away from keyboard.\n",ch[co].name);
                        }
                }

                if ((ch[co].flags&(CF_PLAYER)) && (ch[co].kindred&KIN_PURPLE))
                        do_char_log(cn,0,"%s is a follower of the Purple One.\n",ch[co].reference);

                if (!godflag && cn!=co && (ch[cn].flags&(CF_PLAYER)) && !(ch[cn].flags&CF_INVISIBLE)&& !(ch[cn].flags&CF_SHUTUP))
                	do_char_log(co,1,"%s looks at you.\n",ch[cn].name);

                if ((ch[co].flags&(CF_PLAYER)) && ch[co].data[14] && !(ch[co].flags&CF_GOD)) {
                        if (!ch[co].data[15]) killer="unknown causes";
                        else if (ch[co].data[15]>=MAXCHARS) killer=ch[ch[co].data[15]&0xffff].reference;
                        else killer=ch_temp[ch[co].data[15]].reference;

                        do_char_log(cn,1,"%s died %d times, the last time on the day %d of the year %d, killed by %s %s.\n",
                                ch[co].reference,
                                ch[co].data[14],
                                ch[co].data[16]%300,ch[co].data[16]/300,
                                killer,
                                get_area_m(ch[co].data[17]%MAPX,ch[co].data[17]/MAPX,1));
                }

                if ((ch[co].flags&(CF_PLAYER)) && ch[co].data[44] && !(ch[co].flags&CF_GOD)) {
                        do_char_log(cn,1,"%s was saved from death %d times.\n",
                                ch[co].reference,
                                ch[co].data[44]);
                }

                if ((ch[co].flags&(CF_PLAYER)) && (ch[co].flags&(CF_POH))) {
                        if (ch[co].flags&CF_POH_LEADER) {
                                do_char_log(cn,0,"%s is a Leader among the Purples of Honor.\n",ch[co].reference);
                        } else {
                                do_char_log(cn,0,"%s is a Purple of Honor.\n",ch[co].reference);
                        }
                }

                if (ch[co].text[3][0] && (ch[co].flags&CF_PLAYER)) {
                        do_char_log(cn,0,"%s\n",ch[co].text[3]);
                }
        }

        nr=ch[cn].player;

        buf[0]=SV_LOOK1;
        if (p<=75) {
                *(unsigned short*)(buf+1)=(ch[co].worn[0] ? it[ch[co].worn[0]].sprite[0] : 0);
                *(unsigned short*)(buf+3)=(ch[co].worn[2] ? it[ch[co].worn[2]].sprite[0] : 0);
                *(unsigned short*)(buf+5)=(ch[co].worn[3] ? it[ch[co].worn[3]].sprite[0] : 0);
                *(unsigned short*)(buf+7)=(ch[co].worn[5] ? it[ch[co].worn[5]].sprite[0] : 0);
                *(unsigned short*)(buf+9)=(ch[co].worn[6] ? it[ch[co].worn[6]].sprite[0] : 0);
                *(unsigned short*)(buf+11)=(ch[co].worn[7] ? it[ch[co].worn[7]].sprite[0] : 0);
                *(unsigned short*)(buf+13)=(ch[co].worn[8] ? it[ch[co].worn[8]].sprite[0] : 0);
                *(unsigned char*)(buf+15)=autoflag;
        } else {
                *(unsigned short*)(buf+1)=35;
                *(unsigned short*)(buf+3)=35;
                *(unsigned short*)(buf+5)=35;
                *(unsigned short*)(buf+7)=35;
                *(unsigned short*)(buf+9)=35;
                *(unsigned short*)(buf+11)=35;
                *(unsigned short*)(buf+13)=35;
                *(unsigned char*)(buf+15)=autoflag;
        }
        xsend(nr,buf,16);

        buf[0]=SV_LOOK2;

        if (p<=75) {
                *(unsigned short*)(buf+1)=(ch[co].worn[9] ? it[ch[co].worn[9]].sprite[0] : 0);
                *(unsigned short*)(buf+13)=(ch[co].worn[10] ? it[ch[co].worn[10]].sprite[0] : 0);
        } else {
                *(unsigned short*)(buf+1)=35;
                *(unsigned short*)(buf+13)=35;
        }

        *(unsigned short*)(buf+3)=ch[co].sprite;
        *(unsigned int*)(buf+5)=ch[co].points_tot;
        if (p>75) {
                hp_diff=ch[co].hp[5]/2-RANDOM(ch[co].hp[5]+1);
                end_diff=ch[co].end[5]/2-RANDOM(ch[co].end[5]+1);
                mana_diff=ch[co].mana[5]/2-RANDOM(ch[co].mana[5]+1);
        }
        *(unsigned int*)(buf+9)=ch[co].hp[5]+hp_diff;
        xsend(nr,buf,16);

        buf[0]=SV_LOOK3;
        *(unsigned short*)(buf+1)=ch[co].end[5]+end_diff;
        *(unsigned short*)(buf+3)=(ch[co].a_hp+500)/1000+hp_diff;
        *(unsigned short*)(buf+5)=(ch[co].a_end+500)/1000+end_diff;
        *(unsigned short*)(buf+7)=co;
        *(unsigned short*)(buf+9)=(unsigned short)char_id(co);
        *(unsigned short*)(buf+11)=ch[co].mana[5]+mana_diff;
        *(unsigned short*)(buf+13)=(ch[co].a_mana+500)/1000+mana_diff;

        xsend(nr,buf,16);

        buf[0]=SV_LOOK4;
        if (p<=75) {
                *(unsigned short*)(buf+1)=(ch[co].worn[1] ? it[ch[co].worn[1]].sprite[0] : 0);
                *(unsigned short*)(buf+3)=(ch[co].worn[4] ? it[ch[co].worn[4]].sprite[0] : 0);
                *(unsigned short*)(buf+10)=(ch[co].worn[11] ? it[ch[co].worn[11]].sprite[0] : 0);
                *(unsigned short*)(buf+12)=(ch[co].worn[12] ? it[ch[co].worn[12]].sprite[0] : 0);
                *(unsigned short*)(buf+14)=(ch[co].worn[13] ? it[ch[co].worn[13]].sprite[0] : 0);
        } else {
                *(unsigned short*)(buf+1)=35;
                *(unsigned short*)(buf+3)=35;
                *(unsigned short*)(buf+10)=35;
                *(unsigned short*)(buf+12)=35;
                *(unsigned short*)(buf+14)=35;
        }
        if ((ch[co].flags&(CF_MERCHANT|CF_BODY)) && !autoflag) {
                *(unsigned char*)(buf+5)=1;
                if ((in=ch[cn].citem)!=0) {
                        if (ch[co].flags&CF_MERCHANT) {
                                pr=barter(cn,do_item_value(in),0);
                        } else pr=0;
                } else pr=0;
                *(unsigned int*)(buf+6)=pr;
        } else {
                *(unsigned char*)(buf+5)=0;
        }

        xsend(nr,buf,16);

        buf[0]=SV_LOOK5;
        for (n=0; n<15; n++) buf[n+1]=ch[co].name[n];
        xsend(nr,buf,16);

        if ((ch[co].flags&(CF_MERCHANT|CF_BODY)) && !autoflag) {
                for (n=0; n<40; n+=2) {
                        buf[0]=SV_LOOK6;
                        buf[1]=n;
                        for (m=n; m<min(40,n+2); m++) {
                                if ((in=ch[co].item[m])!=0) {
                                        spr=it[in].sprite[0];
                                        if (ch[co].flags&CF_MERCHANT) {
                                                pr=barter(cn,do_item_value(in),1);
                                        } else pr=0;
                                } else { spr=pr=0; }
                                *(unsigned short*)(buf+2+(m-n)*6)=spr;
                                *(unsigned int*)(buf+4+(m-n)*6)=pr;
                        }
                        xsend(nr,buf,16);
                }

                for (n=0; n<20; n+=2) {
                        buf[0]=SV_LOOK6;
                        buf[1]=n+40;
                        for (m=n; m<min(20,n+2); m++) {
                                if ((in=ch[co].worn[m])!=0 && (ch[co].flags&CF_BODY)) {
                                        spr=it[in].sprite[0];
                                        pr=0;
                                } else { spr=pr=0; }
                                *(unsigned short*)(buf+2+(m-n)*6)=spr;
                                *(unsigned int*)(buf+4+(m-n)*6)=pr;
                        }
                        xsend(nr,buf,16);
                }

                buf[0]=SV_LOOK6;
                buf[1]=60;
                if ((in=ch[co].citem)!=0 && (ch[co].flags&CF_BODY)) {
                        spr=it[in].sprite[0];
                        pr=0;
                } else { spr=pr=0; }
                *(unsigned short*)(buf+2+0*6)=spr;
                *(unsigned int*)(buf+4+0*6)=pr;

                if (ch[co].gold && (ch[co].flags&CF_BODY)) {
                        if (ch[co].gold>999999) spr=121;
                        else if (ch[co].gold>99999) spr=120;
                        else if (ch[co].gold>9999) spr=41;
                        else if (ch[co].gold>999) spr=40;
                        else if (ch[co].gold>99) spr=39;
                        else if (ch[co].gold>9) spr=38;
                        else spr=37;
                        pr=0;
                } else { spr=pr=0; }
                *(unsigned short*)(buf+2+1*6)=spr;
                *(unsigned int*)(buf+4+1*6)=pr;
                xsend(nr,buf,16);
        }

        if ((ch[cn].flags&(CF_GOD|CF_IMP|CF_USURP)) && !autoflag && !(ch[co].flags&CF_MERCHANT) && !(ch[co].flags&CF_BODY) && !(ch[co].flags&CF_GOD)) {
                do_char_log(cn,3,"This is char %d, created from template %d, pos %d,%d\n",co,ch[co].temp,ch[co].x,ch[co].y);
		if (ch[co].flags&CF_GOLDEN) do_char_log(cn,3,"Golden List.\n");
		if (ch[co].flags&CF_BLACK) do_char_log(cn,3,"Black List.\n");
        }
}

void do_look_depot(int cn,int co)
{
        int n,nr,in,pr,spr,m;
        char buf[16];

        if (co<=0 || co>=MAXCHARS) return;
        if (cn!=co) return;

        if (!(map[ch[cn].x+ch[cn].y*MAPX].flags&MF_BANK) && !(ch[cn].flags&CF_GOD)) {
                do_char_log(cn,0,"You cannot access your depot outside a bank.\n");
                return;
        }

        nr=ch[cn].player;

        buf[0]=SV_LOOK1;
        *(unsigned short*)(buf+1)=35;
        *(unsigned short*)(buf+3)=35;
        *(unsigned short*)(buf+5)=35;
        *(unsigned short*)(buf+7)=35;
        *(unsigned short*)(buf+9)=35;
        *(unsigned short*)(buf+11)=35;
        *(unsigned short*)(buf+13)=35;
        *(unsigned char*)(buf+15)=0;
        xsend(nr,buf,16);

        buf[0]=SV_LOOK2;

        *(unsigned short*)(buf+1)=35;
        *(unsigned short*)(buf+13)=35;

        *(unsigned short*)(buf+3)=ch[co].sprite;
        *(unsigned int*)(buf+5)=ch[co].points_tot;

        *(unsigned int*)(buf+9)=ch[co].hp[5];
        xsend(nr,buf,16);

        buf[0]=SV_LOOK3;
        *(unsigned short*)(buf+1)=ch[co].end[5];
        *(unsigned short*)(buf+3)=(ch[co].a_hp+500)/1000;
        *(unsigned short*)(buf+5)=(ch[co].a_end+500)/1000;
        *(unsigned short*)(buf+7)=co|0x8000;
        *(unsigned short*)(buf+9)=(unsigned short)char_id(co);
        *(unsigned short*)(buf+11)=ch[co].mana[5];
        *(unsigned short*)(buf+13)=(ch[co].a_mana+500)/1000;

        xsend(nr,buf,16);

        buf[0]=SV_LOOK4;
        *(unsigned short*)(buf+1)=35;
        *(unsigned short*)(buf+3)=35;
        *(unsigned short*)(buf+10)=35;
        *(unsigned short*)(buf+12)=35;
        *(unsigned short*)(buf+14)=35;
        *(unsigned char*)(buf+5)=1;

        // CS, 000205: Check for sane item (not money)
        if (IS_SANEITEM(in=ch[cn].citem)) pr=TICKS*do_depot_cost(in);
        else pr=0;

        *(unsigned int*)(buf+6)=pr;
        xsend(nr,buf,16);

        buf[0]=SV_LOOK5;
        for (n=0; n<15; n++) buf[n+1]=ch[co].name[n];
        xsend(nr,buf,16);

        for (n=0; n<62; n+=2) {
                buf[0]=SV_LOOK6;
                buf[1]=n;
                for (m=n; m<min(62,n+2); m++) {
                        if ((in=ch[co].depot[m])!=0) {
                                spr=it[in].sprite[0];
                                pr=TICKS*do_depot_cost(in);
                        } else { spr=pr=0; }
                        *(unsigned short*)(buf+2+(m-n)*6)=spr;
                        *(unsigned int*)(buf+4+(m-n)*6)=pr;
                }
                xsend(nr,buf,16);
        }
}

// DEBUG ADDED: JC 07/11/2001
void do_look_player_depot(int cn,char *cv)
{
        int in,m;
	int count = 0;

        int co = dbatoi(cv);
	if (!IS_SANECHAR(co)) {
		do_char_log(cn,0,"Bad character: %s!\n", cv);
		return;
	}
	do_char_log(cn,1,"Depot contents for : %s\n", ch[co].name);
	do_char_log(cn,1,"-----------------------------------\n");

	for(m=0; m<62; m++) {
		if ((in=ch[co].depot[m])!=0) {
			do_char_log(cn,1,"%6d: %s\n",in,it[in].name);
			count++;
		}
	}

	do_char_log(cn,1," \n");
        do_char_log(cn,1,"Total : %d items.\n",count);
}

void do_look_player_inventory(int cn,char *cv)
{
        int n,in;
        int count = 0;

        int co = dbatoi(cv);
        if (!IS_SANECHAR(co)) {
                do_char_log(cn,0,"Bad character: %s!\n", cv);
                return;
        }
        do_char_log(cn,1,"Inventory contents for : %s\n", ch[co].name);
        do_char_log(cn,1,"-----------------------------------\n");

        for (n=0; n<40; n++) {
               if ((in=ch[co].item[n])!=0) {
                        do_char_log(cn,1,"%6d: %s\n",in,it[in].name);
                        count++;
		}
	}

        do_char_log(cn,1," \n");
        do_char_log(cn,1,"Total : %d items.\n",count);
}

void do_look_player_equipment(int cn,char *cv)
{
        int n,in;
        int count = 0;

        int co = dbatoi(cv);
        if (!IS_SANECHAR(co)) {
                do_char_log(cn,0,"Bad character: %s!\n", cv);
                return;
        }
        do_char_log(cn,1,"Equipment for : %s\n", ch[co].name);
        do_char_log(cn,1,"-----------------------------------\n");

        for (n=0; n<20; n++) {
               if ((in=ch[co].worn[n])!=0) {
                        do_char_log(cn,1,"%6d: %s\n",in,it[in].name);
                        count++;
		}
	}

        do_char_log(cn,1," \n");
        do_char_log(cn,1,"Total : %d items.\n",count);
}


void do_steal_player(int cn, char *cv, char *ci)
{
	int n;
	int i_index = 0;
	char found_i = 0;
	char found_d = 0;
	char found_e = 0;

	int co = dbatoi(cv);
	int in = atoi(ci);

	if (!IS_SANECHAR(co)) {
		do_char_log(cn,0,"Bad character: %s!\n", cv);
		return;
	} else if (in == 0)
		return;

	//look through depot and inventory for this item
	for (n=0; n<40; n++) {
		if (in==ch[co].item[n]) {
			i_index = n;
			found_i = !(0);
			break;
		}
	}

	if (!found_i) {
		for (n=0; n<62; n++) {
			if (in==ch[co].depot[n]) {
				i_index = n;
				found_d = !(0);
				break;
			}
		}
	}

	if (!found_i && !found_d) {
		for (n=0; n<20; n++) {
			if (in==ch[co].worn[n]) {
				i_index = n;
				found_e = !(0);
				break;
			}
		}
	}

	if (found_i | found_d | found_e) {
		if (god_give_char(in,cn)) {
			if (found_i)
				ch[co].item[i_index]=0;
			else if (found_d)
				ch[co].depot[i_index]=0;
			else if (found_e)
				ch[co].worn[i_index]=0;
			
			do_char_log(cn,1,"You stole %s from %s.",it[in].reference, ch[co].name);
		} else do_char_log(cn,0,"You cannot take the %s because your inventory is full.\n",it[in].reference);
	} else
		do_char_log(cn,0,"Item not found.\n");
}

static inline void map_add_light(int x,int y,int v)
{
        register unsigned int m;
        // if (x<0 || x>=MAPX || y<0 || y>=MAPY || v==0) return;

        m=x+y*MAPX;

        map[m].light+=v;

        if (map[m].light<0) {
//              xlog("Error in light computations at %d,%d (+%d=%d).",x,y,v,map[m].light);
                map[m].light=0;
        }
}

void do_add_light(int xc,int yc,int stren)
{
        int x,y,xs,ys,xe,ye,v,d,flag;
        unsigned long long prof;

        prof=prof_start();

        map_add_light(xc,yc,stren);

        if (stren<0) { flag=1; stren=-stren; }
        else flag=0;

        xs=max(0,xc-LIGHTDIST);
        ys=max(0,yc-LIGHTDIST);
        xe=min(MAPX-1,xc+1+LIGHTDIST);
        ye=min(MAPY-1,yc+1+LIGHTDIST);

        for (y=ys; y<ye; y++) {
                for (x=xs; x<xe; x++) {
                        if (x==xc && y==yc) continue;
                        if ((xc-x)*(xc-x)+(yc-y)*(yc-y)>(LIGHTDIST*LIGHTDIST+1)) continue;
                        if ((v=can_see(0,xc,yc,x,y,LIGHTDIST))!=0) {
                                d=stren/(v*(abs(xc-x)+abs(yc-y)));
                                if (flag) map_add_light(x,y,-d);
                                else map_add_light(x,y,d);
                        }
                }
        }
        prof_stop(9,prof);
}

// will put citem into item[X], X being the first free slot.
// returns the slot number on success, -1 otherwise.
int do_store_item(int cn)
{
        int n;

        if (ch[cn].citem&0x80000000) return -1;

        for (n=0; n<40; n++)
                if (!ch[cn].item[n]) break;

        if (n==40) return -1;

        ch[cn].item[n]=ch[cn].citem;
        ch[cn].citem=0;

	do_update_char(cn);

        return n;

}

int do_swap_item(int cn,int n)
{
        int tmp,in,m;
        static char *at_text[5]={"not brave enough","not determined enough","not intuitive enough","not agile enough","not strong enough"};

        if (ch[cn].citem&0x80000000) return -1;

        if (n<0 || n>19) return -1; // sanity check

        tmp=ch[cn].citem;

        // check prerequisites:
        if (tmp) {
                if (it[tmp].driver==52 && it[tmp].data[0]!=cn) {
                        if (it[tmp].data[0]==0) {
                                char buf[300];

                                it[tmp].data[0]=cn;

                                sprintf(buf,"%s Engraved in it are the letters \"%s\".",
                                        it[tmp].description,ch[cn].name);
                                if (strlen(buf)<200) strcpy(it[tmp].description,buf);
                        } else {
                                do_char_log(cn,0,"The gods frown at your attempt to wear another ones %s.\n",it[tmp].reference);
                                return -1;
                        }
                }
                for (m=0; m<5; m++) {
                        if (it[tmp].attrib[m][2]>ch[cn].attrib[m][0]) {
                                do_char_log(cn,0,"You're %s to use that.\n",at_text[m]);
                                return -1;
                        }
                }
                for (m=0; m<50; m++) {
                        if (it[tmp].skill[m][2]>ch[cn].skill[m][0]) {
                                do_char_log(cn,0,"You don't know how to use that.\n");
                                return -1;
                        }
                        if (it[tmp].skill[m][2] && !ch[cn].skill[m][0]) {
                                do_char_log(cn,0,"You don't know how to use that.\n");
                                return -1;
                        }
                }
                if (it[tmp].hp[2]>ch[cn].hp[0]) {
                        do_char_log(cn,0,"You don't have enough life force to use that.\n");
                        return -1;
                }
                if (it[tmp].end[2]>ch[cn].end[0]) {
                        do_char_log(cn,0,"You don't have enough endurance to use that.\n");
                        return -1;
                }
                if (it[tmp].mana[2]>ch[cn].mana[0]) {
                        do_char_log(cn,0,"You don't have enough mana to use that.\n");
                        return -1;
                }

                if ((it[tmp].driver==18 && (ch[cn].kindred&KIN_PURPLE)) ||
                    (it[tmp].driver==39 && !(ch[cn].kindred&KIN_PURPLE)) ||
                    (it[tmp].driver==40 && !(ch[cn].kindred&KIN_SEYAN_DU))) {
                        do_char_log(cn,0,"Ouch. That hurt.\n");
                        return -1;
                }

                if (it[tmp].min_rank>points2rank(ch[cn].points_tot)) {
                	do_char_log(cn,0,"You're not experienced enough to use that.\n");
                	return -1;
                }


                // check for correct placement:
                switch(n) {
                        case    WN_HEAD:        if (!(it[tmp].placement&PL_HEAD)) return -1; else break;
                        case    WN_NECK:        if (!(it[tmp].placement&PL_NECK)) return -1; else break;
                        case    WN_BODY:        if (!(it[tmp].placement&PL_BODY)) return -1; else break;
                        case    WN_ARMS:        if (!(it[tmp].placement&PL_ARMS)) return -1; else break;
                        case    WN_BELT:        if (!(it[tmp].placement&PL_BELT)) return -1; else break;
                        case    WN_LEGS:        if (!(it[tmp].placement&PL_LEGS)) return -1; else break;
                        case    WN_FEET:        if (!(it[tmp].placement&PL_FEET)) return -1; else break;
                        case    WN_LHAND:       if (!(it[tmp].placement&PL_SHIELD)) return -1;
                                                if ((in=ch[cn].worn[WN_RHAND])!=0 && (it[in].placement&PL_TWOHAND)) return -1;
                                                break;
                        case    WN_RHAND:       if (!(it[tmp].placement&PL_WEAPON)) return -1;
                                                if ((it[tmp].placement&PL_TWOHAND) && ch[cn].worn[WN_LHAND]) return -1;
                                                break;
                        case    WN_CLOAK:       if (!(it[tmp].placement&PL_CLOAK)) return -1; else break;
                        case    WN_RRING:
                        case    WN_LRING:       if (!(it[tmp].placement&PL_RING)) return -1; else break;
                        default:                return -1;
                }
        }

        ch[cn].citem=ch[cn].worn[n];
        ch[cn].worn[n]=tmp;

        do_update_char(cn);

        return n;
}

/* replaced by may_attack_message()
int do_mayattack(int cn,int co)
{
        int m1,m2,f;

        if (!(ch[cn].flags&CF_PLAYER)) return 1;
        if (!(ch[co].flags&CF_PLAYER)) return 1;

        m1=ch[cn].x+ch[cn].y*MAPX;
        m2=ch[co].x+ch[co].y*MAPX;
        f=map[m1].flags&map[m2].flags;  // make sure we take only flags present on both player's location

        if (f&MF_ARENA) return 1;

        if ((ch[cn].kindred&KIN_PURPLE) && (ch[co].kindred&KIN_PURPLE)) return 1;

        return 0;
}
*/

/* Check if cn may attack co. if (msg), tell cn if not. */
int may_attack_msg(int cn, int co, int msg)
{
        int m1,m2;

        if (!IS_SANECHAR(cn) || !IS_SANECHAR(co)) return 1;

	// unsafe gods may attack anyone
        if ((ch[cn].flags&CF_GOD && !(ch[cn].flags&CF_SAFE))) {
                return 1;
        }

        // unsafe gods may be attacked by anyone!
        if ((ch[co].flags&CF_GOD && !(ch[co].flags&CF_SAFE))) {
                return 1;
        }

	// player GC? act as if he would try to attack the master of the GC instead
        if (IS_COMPANION(cn) && ch[cn].data[64]==0) {
                cn=ch[cn].data[63];
                if (!IS_SANECHAR(cn)) return 1; // um, lets him try to kill this GC - it's got bad values anway
        }

        // NPCs may attack anyone, anywhere
        if (!IS_PLAYER(cn)) return 1;

        // Check for NOFIGHT
        m1=XY2M(ch[cn].x,ch[cn].y);
        m2=XY2M(ch[co].x,ch[co].y);
        if ((map[m1].flags|map[m2].flags)&MF_NOFIGHT) {
                if (msg) do_char_log(cn, 0, "You can't attack anyone here!\n");
                return 0;
        }

        // player GC? act as if he would try to attack the master of the GC instead
        if (IS_COMPANION(co) && ch[co].data[64]==0) {
                co=ch[co].data[63];
                if (!IS_SANECHAR(co)) return 1; // um, lets him try to kill this GC - it's got bad values anway
        }

        // Check for plr-npc (OK)
        if (!IS_PLAYER(cn) || !IS_PLAYER(co)) return 1;

        // Both are players. Check for Arena (OK)
        if (map[m1].flags & map[m2].flags & MF_ARENA) return 1;

        // Check if aggressor is purple
        if (!IS_PURPLE(cn)) {
                if (msg) do_char_log(cn, 0, "You can't attack other players! You're not a follower of the Purple One.\n");
                return 0;
        }

        // Check if victim is purple
        if (!IS_PURPLE(co)) {
                if (msg) do_char_log(cn, 0, "You can't attack %s! %s's not a follower of the Purple One.\n",
                                     ch[co].name,
                                     IS_FEMALE(co) ? "She" : "He");
                return 0;
        }

        // See if the ranks match:
        /*if (points2rank(ch[cn].points_tot)<9 && points2rank(ch[co].points_tot)>=9) {
                if (msg) do_char_log(cn,0,"You're not allowed to attack %s. %s is an officer, but you are not.\n",
                        ch[co].name,
                        (ch[co].kindred&KIN_MALE) ? "He" : "She");
                return 0;
        }
        if (points2rank(ch[cn].points_tot)>=9 && points2rank(ch[co].points_tot)<9) {
                if (msg) do_char_log(cn,0,"You're not allowed to attack %s. You are an officer, but %s is not.\n",
                        ch[co].name,
                        (ch[co].kindred&KIN_MALE) ? "he" : "she");
                return 0;
        }
        if (points2rank(ch[cn].points_tot)<20 && points2rank(ch[co].points_tot)>=20) {
                if (msg) do_char_log(cn,0,"You're not allowed to attack %s. %s is of noble birth, but you are not.\n",
                        ch[co].name,
                        (ch[co].kindred&KIN_MALE) ? "He" : "She");
                return 0;
        }
        if (points2rank(ch[cn].points_tot)>=20 && points2rank(ch[co].points_tot)<20) {
                if (msg) do_char_log(cn,0,"You're not allowed to attack %s. You are of noble birth, but %s is not.\n",
                        ch[co].name,
                        (ch[co].kindred&KIN_MALE) ? "he" : "she");
                return 0;
        }
        if (points2rank(ch[cn].points_tot)<9 && abs(points2rank(ch[cn].points_tot)-points2rank(ch[co].points_tot))>3) {
                if (msg) do_char_log(cn,0,"You're not allowed to attack %s. The rank difference is too large.\n",ch[co].name);
                return 0;
        }
        if (points2rank(ch[cn].points_tot)<20 && abs(points2rank(ch[cn].points_tot)-points2rank(ch[co].points_tot))>5) {
                if (msg) do_char_log(cn,0,"You're not allowed to attack %s. The rank difference is too large.\n",ch[co].name);
                return 0;
        }*/

	if (abs(points2rank(ch[cn].points_tot)-points2rank(ch[co].points_tot))>3) {
                if (msg) do_char_log(cn,0,"You're not allowed to attack %s. The rank difference is too large.\n",ch[co].name);
                return 0;
        }

        return 1;
}

void do_check_new_level(int cn)
{
        int hp=0,end=0,mana=0,diff,rank,temp,n;

        if (!IS_PLAYER(cn)) return;

        rank=points2rank(ch[cn].points_tot);

        if (ch[cn].data[45]<rank) {
                if (ch[cn].kindred&(KIN_TEMPLAR|KIN_ARCHTEMPLAR)) {
                        hp=15; end=10; mana=5;
                }
                if (ch[cn].kindred&(KIN_MERCENARY|KIN_SORCERER|KIN_WARRIOR|KIN_SEYAN_DU)) {
                        hp=10; end=10; mana=10;
                }
                if (ch[cn].kindred&(KIN_HARAKIM|KIN_ARCHHARAKIM)) {
                        hp=5; end=10; mana=15;
                }
                if (hp==0) return;

                diff=rank-ch[cn].data[45];
                ch[cn].data[45]=rank;

                if (diff==1) do_char_log(cn,0,"You rose a level! Congratulations! You received %d hitpoints, %d endurance and %d mana.\n",
                                hp*diff,end*diff,mana*diff);
                else do_char_log(cn,0,"You rose %d levels! Congratulations! You received %d hitpoints, %d endurance and %d mana.\n",
                                diff,hp*diff,end*diff,mana*diff);
                /* CS, 991203: Announce the player's new rank */
                temp = (ch[cn].kindred & KIN_PURPLE) ? CT_PRIEST : CT_LGUARD;
                // Find a character with appropriate template
                for (n=1; n<MAXCHARS; n++) {
                        if (ch[n].used!=USE_ACTIVE) continue;
                        if (ch[n].flags&CF_BODY) continue;
                        if (ch[n].temp == temp) break;
                }
                // Have him yell it out
                if (n<MAXCHARS) {
                        char message[100];
                        sprintf(message, "Hear ye, hear ye! %s has attained the rank of %s!",
                                ch[cn].name, rank_name[rank]);
                        do_shout(n, message);
                }

                ch[cn].hp[1]=hp*rank;
                ch[cn].end[1]=end*rank;
                ch[cn].mana[1]=mana*rank;

                do_update_char(cn);
        }
}

/* CS, 991103: Tell when a certain player last logged on. */
void do_seen(int cn, char *cco)
{
        int co;
        time_t last_date, current_date;
        int days;
        char *when;
        char interval[50];

        if (!*cco) {
                do_char_log(cn, 0, "When was WHO last seen?\n");
                return;
        }

        // numeric only for deities
        if (isdigit(*cco) && ((ch[cn].flags & (CF_IMP|CF_GOD|CF_USURP)) == 0)) {
                co = 0;
        } else {
                co = dbatoi_self(cn, cco);
        }

        if (!co) {
                do_char_log(cn, 0, "I've never heard of %s.\n", cco);
                return;
        }

        if (!IS_PLAYER(co)) {
                do_char_log(cn, 0, "%s is not a player.\n", ch[co].name);
                return;
        }

        if (!(ch[cn].flags&CF_GOD) && (ch[co].flags&CF_GOD)) {
                do_char_log(cn,0,"No one knows when the gods where last seen.\n");
                return;
        }

        if (ch[cn].flags&(CF_IMP|CF_GOD)) {
                time_t last,now;
                struct tm tlast,tnow,*tmp;

                last=max(ch[co].login_date,ch[co].logout_date);
                now=time(NULL);

                tmp=localtime(&last); tlast=*tmp;
                tmp=localtime(&now); tnow=*tmp;

                do_char_log(cn,2,"%s was last seen on %04d-%02d-%02d %02d:%02d:%02d (time now: %04d-%02d-%02d %02d:%02d:%02d)\n",
                        ch[co].name,
                        tlast.tm_year+1900,
                        tlast.tm_mon+1,
                        tlast.tm_mday,
                        tlast.tm_hour,
                        tlast.tm_min,
                        tlast.tm_sec,
                        tnow.tm_year+1900,
                        tnow.tm_mon+1,
                        tnow.tm_mday,
                        tnow.tm_hour,
                        tnow.tm_min,
                        tnow.tm_sec);

                if (ch[co].used==USE_ACTIVE && !(ch[co].flags&CF_INVISIBLE)) do_char_log(cn,2,"PS: %s is online right now!\n",ch[co].name);


        } else {
                last_date = max(ch[co].login_date, ch[co].logout_date) / (24*3600);
                current_date = time(NULL) / (24*3600);
                days = current_date - last_date;
                switch (days) {
                        case 0: when = "earlier today"; break;
                        case 1: when = "yesterday"; break;
                        case 2: when = "the day before yesterday"; break;
                        default: sprintf(interval, "%d days ago", days);
                                when = interval; break;
                }
                do_char_log(cn, 1, "%s was last seen %s.\n", ch[co].name, when);
        }
}

/* CS, 991204: Do not fight back if spelled */
void do_spellignore(int cn)
{
        ch[cn].flags ^= CF_SPELLIGNORE;
        if (ch[cn].flags & CF_SPELLIGNORE) {
                do_char_log(cn, 1, "Now ignoring spell attacks.\n");
        } else {
                do_char_log(cn, 1, "Now reacting to spell attacks.\n");
        }
}


/* CS, 000209: Remember PvP attacks */
void remember_pvp(int cn, int co)
{
        int mf;
        mf = map[XY2M(ch[cn].x,ch[cn].y)].flags;
        if (mf & MF_ARENA) return; // Arena attacks don't count

        /* Substitute masters for companions, some sanity checks */
        if (!IS_SANEUSEDCHAR(cn)) return;
        if (IS_COMPANION(cn)) cn = ch[cn].data[CHD_MASTER];
        if (!IS_SANEPLAYER(cn)) return;
        if (!IS_PURPLE(cn)) return;

        if (!IS_SANEUSEDCHAR(co)) return;
        if (IS_COMPANION(co)) co = ch[co].data[CHD_MASTER];
        if (!IS_SANEPLAYER(co)) return;

        if (cn == co) return;

        ch[cn].data[CHD_ATTACKTIME] = globs->ticker;
        ch[cn].data[CHD_ATTACKVICT] = co;
}

