/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "server.h"
#include "driver.h"
#include "npc.h"

/*
DATA 0-9 is for exclusive drivers as MERCHANT

DATA usage by all NPC drivers

10-18:  patrol-stops, using m=x+y*MAPX
19:     next stop
20-23:  doors to close
24:     prevent fight mode, -1=defend evil, 0=no interference, 1=defend good
25:     special driver
26:     special sub-driver
27:     last time we got stop command (password logic)
28:     exp earned since creation (GHOST COMPANION et al)
29:     resting position (x+y*MAPX)
30:     resting dir (what a waste of space)
31:     protect character created from template X
32-35:  lights to keep burning
36:     frust timer. if above 100, char will use NPC-magic (teleport etc.) to reach objective
37-40:  last character we talked to. set 37 to !=0 to activate talking
41:     turn on/off lights from this template if daylight on/off
42:     group
43-46:  if 43 is set attack everyone NOT member of one of the groups 43-46
47:     get and destroy/donate garbage
48:     say text on death with this probability (X%)
49:     wants item X
50:     teaches skill X in exchange for item (raises it to 1)
51:     raises EXP by X in exchange for item
52:     shout for help if attacked, using code X
53:     come to help if called by code X
54:     place of shout
55:     timeout for shout, if 54 is zero and 55 is set, WE shouted for help
56:     timeout2 for greet
57:     rest time between patrol (used by driver)
58:     importance of current job (for better endurance management) (0=low, 1=medium, 2=high)
59:     help all members of group X
60:     random walk, time between walks
61:     time elapsed
62:     create light: 0=never, 1=when dark, 2=when dark and not resting, 3=when dark and fighting
63:     obey and protect character X (CHD_MASTER)
64:     self destruct in X ticks (CHD_COMPANION??)
65:     friend is under attack - help by magic etc.
66:     gives item X in exchange for item from 49
67:     timeout for greeting
68:     value of knowledge (CHD_ATTACKTIME??)
69:     follow character X (CHD_ATTACKVICT??)
70:     last time we called our god for help
71:     talkativity (CHD_TALKATIVE)
72:     area of knowledge
73:     random walk: max distance to origin
74:     last time we created a ghost
75:     last time we stunned someone
76:     last known position of an enemy
77:     timeout for 76
78:     attacked by invisible
79:     rest time between patrol (value from admin interface)
80-91:  characters on kill list
92:     no-sleep bonus
93:     attack distance (gets warning first)
94:     last warning time
95:     keyword action, 1=wait for password and attack if not given, 2=dont attack anyone further away then [93] from resting position
96:     queued spells
97:     being usurped by
98:     GC no-see-master timeout
99:     used by populate

TEXT usage by all NPC drivers
0:      killed enemy %1
1:      attacking new enemy %1
2:      greeting %1
3:      killed by %1
4:      shouting for help (against %1)
5:      coming to help %1
6:      keyword                 -|
7:      reaction to keyword      |
8:      warning message         -|
9:      abused as memory of already searched graves. i know, i know, bad style and all...
*/

// ***************************************
// *         Helper Routines             *
// ***************************************

int get_frust_x_off(int f)
{
        switch(f%5) {
                case    0:      return 0;
                case    1:      return 1;
                case    2:      return -1;
                case    3:      return 2;
                case    4:      return -2;
                default:        return 0;
        }
}

int get_frust_y_off(int f)
{
        switch((f/5)%5) {
                case    0:      return 0;
                case    1:      return 1;
                case    2:      return -1;
                case    3:      return 2;
                case    4:      return -2;
                default:        return 0;
        }
}

int npc_dist(int cn,int co)
{
        return max(abs(ch[cn].x-ch[co].x),abs(ch[cn].y-ch[co].y));
}

int npc_add_enemy(int cn,int co,int always)
{
        int n,idx,d1,d2,cc;

        // don't attack anyone of the same group. Never, never, never.
        if (ch[cn].data[42]==ch[co].data[42]) return 0;
        // Group 1 mobs shall not attack ghost companions.
        // Ishtar said 65536 to 65536+4096, I hope this is OK.
        if (!always && (ch[cn].data[42] == 1) && (ch[co].data[42] & 0x10000)) return 0;

        if (!always && (ch[cn].points_tot+500)*25<ch[co].points_tot) return 0;

        ch[cn].data[76]=ch[co].x+ch[co].y*MAPX;
        ch[cn].data[77]=globs->ticker;

        cc=ch[cn].attack_cn;

        d1=npc_dist(cn,cc);
        d2=npc_dist(cn,co);

        if (!ch[cn].attack_cn ||
            (d1>d2 && (globs->flags&GF_CLOSEENEMY)) ||
            (d1==d2 && (!cc || ch[cc].attack_cn!=cn) && ch[co].attack_cn==cn) ) {
                ch[cn].attack_cn=co;
                ch[cn].goto_x=0;        // cancel goto (patrol) as well
                ch[cn].data[58]=2;
        }

        idx=co|(char_id(co)<<16);

        for (n=80; n<92; n++) if (ch[cn].data[n]==idx) return 0;

        for (n=91; n>80; n--) ch[cn].data[n]=ch[cn].data[n-1];

        ch[cn].data[80]=idx;

        return 1;
}

int npc_is_enemy(int cn,int co)
{
        int n,idx;

        idx=co|(char_id(co)<<16);

        for (n=80; n<92; n++) if (ch[cn].data[n]==idx) return 1;

        return 0;
}

int npc_list_enemies(int npc, int cn)
{
        int none = 1;
        int n;
        int cv;

        do_char_log(cn, 2, "Enemies of %s:\n", ch[npc].name);
        for (n=80; n<92; n++) {
                if ((cv=ch[npc].data[n] & 0xFFFF)) {
                        do_char_log(cn, 2, "  %s\n", ch[cv].name);
                        none = 0;
                }
        }
        if (none) {
                do_char_log(cn, 2, "-none-\n");
                return 0;
        }
        return 1;
}

int npc_remove_enemy(int npc, int enemy)
{
        int found = 0;
        int n;

        for (n=CHD_ENEMY1ST; n<=CHD_ENEMYZZZ; n++) {
                if ((ch[npc].data[n] & 0xFFFF) == enemy) found = 1;
                if (found) {
                        if (n < CHD_ENEMYZZZ) {
                                ch[npc].data[n] = ch[npc].data[n+1];
                        } else {
                                ch[npc].data[n] = 0;
                        }
                }
        }
        return found;
}

/* say one of the NPC's canned response messages, with optional name. */
void npc_saytext_n(int npc, int n, char *name)
{
        if (ch[npc].flags&CF_SHUTUP) return;

        if (ch[npc].text[n][0]) {
                if (ch[npc].temp == CT_COMPANION) {
                        if (ch[npc].data[CHD_TALKATIVE] == -10)
                                do_sayx(npc, ch[npc].text[n], name);
                } else {
                                do_sayx(npc, ch[npc].text[n], name);
                }
        }
}

// ***************************************
// *         Message Handlers            *
// ***************************************

int npc_see(int cn,int co);

int npc_gotattack(int cn,int co,int dam)
{
        int cc;

        ch[cn].data[92]=TICKS*60;

        if (co && (ch[co].flags&CF_PLAYER) && ch[cn].alignment==10000 && (strcmp(ch[cn].name,"Peacekeeper") || ch[cn].a_hp<ch[cn].hp[5]*500) && ch[cn].data[70]<globs->ticker) {
                do_sayx(cn,"Skua! Protect the innocent! Send me a Peacekeeper!");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                ch[cn].data[70]=globs->ticker+TICKS*60*1;

                cc=god_create_char(80,1);
                ch[cc].temp = CT_COMPANION;
                ch[cc].data[42]=65536+cn;                       // set group
                ch[cc].data[59]=65536+cn;                       // protect all other members of this group

                /* make thrall harmless */
                ch[cc].data[24] = 0;    // do not interfere in fights
                ch[cc].data[36] = 0;    // no walking around
                ch[cc].data[43] = 0;    // don't attack anyone
                ch[cc].data[80] = co|(char_id(co)<<16);   // attacker is enemy
                ch[cc].data[63] = cn;   // obey and protect enthraller
                ch[cc].data[64]=globs->ticker+120*TICKS;        // make her vanish after 2 minutes
                ch[cc].data[70]=globs->ticker+TICKS*60*1;       // and make sure thralled peacies don't summon more than one more

                strcpy(ch[cc].name,"Shadow of Peace");
                strcpy(ch[cc].reference,"Shadow of Peace");
                strcpy(ch[cc].description,"You see a Shadow of Peace.");

                if (!god_drop_char_fuzzy(cc,ch[co].x,ch[co].y)) {
                        god_destroy_items(cc);
                        ch[cc].used=0;
                }
        }

        if (ch[cn].alignment>1000 && ch[cn].data[70]<globs->ticker && ch[cn].a_mana<ch[cn].mana[5]*333) {
                do_sayx(cn,"Skua! Help me!");
                ch[cn].data[70]=globs->ticker+TICKS*60*2;
                ch[cn].a_mana=ch[cn].mana[5]*1000;
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        if (ch[cn].data[52] && ch[cn].a_hp<ch[cn].hp[5]*666) {  // we're supposed to shout for help if attacked and HP below 2/3rd
                if (ch[cn].data[55]+TICKS*60<globs->ticker) { // shout every 60 seconds
                        ch[cn].data[54]=0;
                        ch[cn].data[55]=globs->ticker;
                        npc_saytext_n(cn, 4, ch[co].name);
                        do_npc_shout(cn,NT_SHOUT,cn,ch[cn].data[52],ch[cn].x,ch[cn].y);
                }
        }

        if (!do_char_can_see(cn,co)) {  // we have been attacked but cannot see the attacker
                ch[cn].data[78]=globs->ticker+TICKS*30;
                return 1;
        }

        // fight back when attacked - all NPCs do this:
        if (npc_add_enemy(cn,co,1)) {
                npc_saytext_n(cn, 1, ch[co].name);
                chlog(cn,"Added %s to kill list for attacking me",ch[co].name);
        }

        return 1;
}

int npc_gothit(int cn,int co,int dam)
{
        if (npc_gotattack(cn,co,dam)) return 1;

        return 0;
}

int npc_gotmiss(int cn,int co)
{
        if (npc_gotattack(cn,co,0)) return 1;
        return 0;
}

int npc_didhit(int cn,int co,int dam)
{
        return 0;
}

int npc_didmiss(int cn,int co)
{
        return 0;
}

int npc_killed(int cn,int cc,int co)
{
        int n,idx;

        if (ch[cn].attack_cn==co) ch[cn].attack_cn=0;
        ch[cn].data[76]=ch[cn].data[77]=ch[cn].data[78]=0;

        idx=co|(char_id(co)<<16);

        for (n=80; n<92; n++) { // remove enemy
                if (ch[cn].data[n]==idx) {
                        // add expansion checks and a handler for texts !!!
                        if (cn==cc) {
                                npc_saytext_n(cn, 0, ch[co].name);
                        }
                        ch[cn].data[n]=0;
                        return 1;
                }
        }

        return 0;
}

int npc_didkill(int cn,int co)
{
        if (npc_killed(cn,cn,co)) return 1;

        return 0;
}

int npc_gotexp(int cn,int amount)
{
        return 0;
}

int npc_seekill(int cn,int cc,int co)
{
        if (npc_killed(cn,cc,co)) return 1;

        return 0;
}

int npc_seeattack(int cn,int cc,int co)
{
        int diff,ret,c2,c3;

        ch[cn].data[92]=TICKS*60;

        if (!do_char_can_see(cn,co)) return 1;  // processed it: we cannot see the defender, so ignore it
        if (!do_char_can_see(cn,cc)) return 1;  // processed it: we cannot see the attacker, so ignore it

        if (ch[cn].data[24]) { // prevent fight mode
                diff=(ch[cc].alignment-50)-ch[co].alignment;
                if (diff<=0) {
                        if (ch[cn].data[24]>0) { ret=npc_add_enemy(cn,cc,1); c2=cc; c3=co; }
                        else { ret=npc_add_enemy(cn,co,1); c2=co; c3=cc; }
                } else {
                        if (ch[cn].data[24]>0) { ret=npc_add_enemy(cn,co,1); c2=co; c3=cc; }
                        else { ret=npc_add_enemy(cn,cc,1); c2=cc; c3=co; }
                }
                if (ret) {
                        npc_saytext_n(cn, 1, ch[c2].name);
                        chlog(cn,"Added %s to kill list for attacking %s (prevent fight)",ch[c2].name,ch[c3].name);
                }
                return 1;
        }

        if (ch[cn].data[31]) { // protect char (by temp)
                if (ch[co].temp==ch[cn].data[31]) {
                        ret=npc_add_enemy(cn,cc,1);
                        if (ret) {
                                npc_saytext_n(cn, 1, ch[cc].name);
                                chlog(cn,"Added %s to kill list for attacking %s (protect char)",ch[cc].name,ch[co].name);
                        }
                        if (!ch[cn].data[65]) ch[cn].data[65]=co;
                }
                if (ch[cc].temp==ch[cn].data[31]) {
                        ret=npc_add_enemy(cn,co,1);
                        if (ret) {
                                npc_saytext_n(cn, 1, ch[co].name);
                                chlog(cn,"Added %s to kill list for being attacked by %s (protect char)",ch[co].name,ch[cc].name);
                        }
                        if (!ch[cn].data[65]) ch[cn].data[65]=cc;
                }
        }

        if (ch[cn].data[63]) { // protect char (by nr)
                if (co==ch[cn].data[63]) {
                        ret=npc_add_enemy(cn,cc,1);
                        if (ret) {
                                npc_saytext_n(cn, 1, ch[cc].name);
                                chlog(cn,"Added %s to kill list for attacking %s (protect char)",ch[cc].name,ch[co].name);
                        }
                        if (!ch[cn].data[65]) ch[cn].data[65]=co;
                }
                if (cc==ch[cn].data[63]) {
                        ret=npc_add_enemy(cn,co,1);
                        if (ret) {
                                npc_saytext_n(cn, 1, ch[co].name);
                                chlog(cn,"Added %s to kill list for being attacked by %s (protect char)",ch[co].name,ch[cc].name);
                        }
                        if (!ch[cn].data[65]) ch[cn].data[65]=cc;
                }
        }

        if (ch[cn].data[59]) { // protect by group
                if (ch[cn].data[59]==ch[co].data[42]) {
                        ret=npc_add_enemy(cn,cc,1);
                        if (ret) {
                                npc_saytext_n(cn, 1, ch[cc].name);
                                chlog(cn,"Added %s to kill list for attacking %s (protect group)",ch[cc].name,ch[co].name);
                        }
                        if (!ch[cn].data[65]) ch[cn].data[65]=co;
                }
                if (ch[cn].data[59]==ch[cc].data[42]) {
                        ret=npc_add_enemy(cn,co,1);
                        if (ret) {
                                npc_saytext_n(cn, 1, ch[co].name);
                                chlog(cn,"Added %s to kill list for being attacked by %s (protect group)",ch[co].name,ch[cc].name);
                        }
                        if (!ch[cn].data[65]) ch[cn].data[65]=cc;
                }
        }

        if (ch[co].temp==CT_COMPANION && ch[co].data[63]==cn) { // MY ghost companion
                if (!ch[cn].data[65]) {
                        ch[cn].data[65]=co;
                }
        }

        if (ch[cc].temp==CT_COMPANION && ch[cc].data[63]==cn) { // MY ghost companion
                if (!ch[cn].data[65]) {
                        ch[cn].data[65]=cc;
                }
        }

        return 0;
}

int npc_seehit(int cn,int cc,int co)
{
        if (npc_seeattack(cn,cc,co)) return 1;
        if (npc_see(cn,cc)) return 1;
        if (npc_see(cn,co)) return 1;

        return 0;
}

int npc_seemiss(int cn,int cc,int co)
{
        if (npc_seeattack(cn,cc,co)) return 1;
        if (npc_see(cn,cc)) return 1;
        if (npc_see(cn,co)) return 1;

        return 0;
}

int npc_give(int cn,int co,int in,int money)
{
        int nr, ar;

        if (ch[co].flags&(CF_PLAYER|CF_USURP)) ch[cn].data[92]=TICKS*60;
        else if (!group_active(cn)) return 0;

        if (in && ch[cn].data[49]==it[in].temp) {
                if (ch[cn].data[49]==740 && ch[cn].temp==518) { // hack for black candle

                        ch[co].data[43]++;

                        god_take_from_char(in,cn);
                        it[in].used=USE_EMPTY;  // silently destroy the item.

                        do_sayx(cn,"Ah, a black candle! Great work, %s! Now we will have peace for a while...",ch[co].name);
                        do_area_log(cn,0,ch[cn].x,ch[cn].y,1,"The Cityguard is impressed by %s's deed.\n",ch[co].name);

                } else do_sayx(cn,"Thank you %s. That's the %s I wanted.",ch[co].name,it[in].reference);

                /* quest-requested items */
                if ((nr=ch[cn].data[50])!=0) {
                        // hack for stun=bash and curse=surround:
                        if (nr==19 && (ch[co].kindred&(KIN_TEMPLAR|KIN_ARCHTEMPLAR))) nr=32;
                        if (nr==20 && (ch[co].kindred&(KIN_TEMPLAR|KIN_ARCHTEMPLAR))) nr=33;
                        if (nr==19 && (ch[co].kindred&KIN_SEYAN_DU) && ch[co].skill[19][0]) nr=32;
                        if (nr==20 && (ch[co].kindred&KIN_SEYAN_DU) && ch[co].skill[20][0]) nr=33;

                        if (nr==19 && (ch[co].kindred&KIN_SEYAN_DU)) {
                                do_sayx(cn,"Bring me the item again to learn Immunity, %s!",ch[co].name);
                        }
                        if (nr==20 && (ch[co].kindred&KIN_SEYAN_DU)) {
                                do_sayx(cn,"Bring me the item again to learn Surround Hit, %s!",ch[co].name);
                        }
                        // end hack
                        do_sayx(cn,"Now I'll teach you %s.",skilltab[nr].name);
                        if (ch[co].skill[nr][0]) {
                                do_sayx(cn,"But you already know %s, %s!",skilltab[nr].name,ch[co].name);
                                god_take_from_char(in,cn);
                                god_give_char(in,co);
                                do_char_log(co,1,"%s did not accept the %s.\n",ch[cn].reference,it[in].name);
                        } else {
                                ch[co].skill[nr][0]=1;
                                do_char_log(co,0,"You learned %s!\n",skilltab[nr].name);
                                do_update_char(co);
                                if ((nr=ch[cn].data[51])!=0) {
                                        do_sayx(cn,"Now I'll teach you a bit about life, the world and everything, %s.",ch[co].name);
                                        do_give_exp(co,nr,0,-1);
                                }
                                god_take_from_char(in,cn);
                                it[in].used=USE_EMPTY;  // silently destroy the item.
                        }
                }

                /* items with a return gift */
                if ((nr=ch[cn].data[66])!=0) {
                        do_sayx(cn,"Here is your %s in exchange.",it_temp[nr].reference);
                        god_take_from_char(in,cn);
                        it[in].used=USE_EMPTY;
                        in=god_create_item(nr);
                        god_give_char(in,co);
                }

                /* special for riddle givers */
                ar = ch[cn].data[72];
                if (IS_PLAYER(co) && (ar >= RIDDLE_MIN_AREA) && (ar <= RIDDLE_MAX_AREA)) {
                        int idx;

                        /* determine which of the 5 riddlers are active */
                        idx = ar - RIDDLE_MIN_AREA;

                        if ((guesser[idx] != 0) && (guesser[idx] != co)) {
                                do_sayx(cn, "I'm still riddling %s; please come back later!\n", ch[guesser[idx]].name);
                                god_take_from_char(in,cn);
                                god_give_char(in,co);
                                return 0;
                        }

                        /* ok to destroy the gift now */
                        god_take_from_char(in,co);
                        it[in].used=USE_EMPTY;
                        /* Select and ask a riddle */
                        lab9_pose_riddle(cn, co);
                }

        } else if (!in && money) {
                do_sayx(cn,"I don't take money from you!");
                ch[co].gold+=money;
                ch[cn].gold-=money;
        } else {
                god_take_from_char(in,cn);
                god_give_char(in,co);
                do_char_log(co,1,"%s did not accept the %s.\n",ch[cn].reference,it[in].name);
        }

        return 0;
}

int npc_cityguard_see(int cn,int co,int flag)
{
        int n;

        if (ch[co].data[42]==27) { // shout if enemy in sight (!)
                if (ch[cn].data[55]+TICKS*180<globs->ticker) { // shout every 180 seconds
                        ch[cn].data[54]=0;
                        ch[cn].data[55]=globs->ticker;
                        npc_saytext_n(cn, 4, ch[co].name);
                        do_npc_shout(cn,NT_SHOUT,cn,ch[cn].data[52],ch[cn].x,ch[cn].y);

                        // shout for players too
                        for (n=1; n<MAXCHARS; n++) {
                                if ((ch[n].flags&(CF_PLAYER|CF_USURP)) && ch[n].used==USE_ACTIVE && !(ch[n].flags&CF_NOSHOUT)) {
                                        if (flag) do_char_log(n,3,"Cityguard: \"The monsters are approaching the city! Alert!\"\n");
                                        else do_char_log(n,3,"Cityguard: \"The monsters are approaching the outpost! Alert!\"\n");
                                }
                        }
                }
        }

        return 0;
}

int is_potion(int in)
{
        static int potions[] = {101,102,127,131,135,148,224,273,274,449};
        int tn, n;

        tn = it[in].temp;
        for (n=0; n<ARRAYSIZE(potions); n++)
                if (tn == potions[n]) return 1;
        return 0;
}

int is_scroll(int in)
{
        int tn;

        tn = it[in].temp;
        return (((tn >= 699) && (tn <= 716)) ||
                ((tn >= 175) && (tn <= 178)) ||
                ((tn >= 181) && (tn <= 189)));
}

int is_unique(int in)
{
        static int unique[60]={
                280,281,282,283,284,285,286,287,288,289,290,291,292,525,526,
                527,528,529,530,531,532,533,534,535,536,537,538,539,540,541,
                542,543,544,545,546,547,548,549,550,551,552,553,554,555,556,
                572,573,574,575,576,577,578,579,580,581,582,583,584,585,586};
        int n;

        for (n=0; n<60; n++)
                if (it[in].temp==unique[n]) return 1;

        return 0;
}

int count_uniques(int cn)
{
        int n,in,cnt=0;

        if (IS_BUILDING(cn)) {
                do_char_log(cn, 0, "Not in build mode.\n");
                return 0;
        }

        if ((in=ch[cn].citem) && !(in&0x80000000) && is_unique(in)) cnt++;
                for (n=0; n<40; n++) {
                        if ((in=ch[cn].item[n]) && is_unique(in)) cnt++;
                }
                for (n=0; n<20; n++) {
                        if ((in=ch[cn].worn[n]) && is_unique(in)) cnt++;
                }
                for (n=0; n<62; n++) {
                        if ((in=ch[cn].depot[n]) && is_unique(in)) cnt++;
                }
        return cnt;
}

int npc_see(int cn,int co)
{
        int n,idx,indoor1,indoor2;
        int x1,x2,y1,y2,dist,ret,cnt;

        if (ch[co].flags&(CF_PLAYER|CF_USURP)) ch[cn].data[92]=TICKS*60;
        else if (!group_active(cn)) return 0;

        if (!do_char_can_see(cn,co)) return 1;  // processed it: we cannot see him, so ignore him

        /* CS, 000209: Check for Ghost Companion seeing his master */
        if (ch[cn].temp == CT_COMPANION && co == ch[cn].data[CHD_MASTER]) {
                // happy to see master, timeout reset
                ch[cn].data[98] = globs->ticker + COMPANION_TIMEOUT;
        }

        // special sub driver
        if (ch[cn].data[26]) {
                switch(ch[cn].data[26]) {
                        case    1:      ret=npc_cityguard_see(cn,co,0); break;
                        case    3:      ret=npc_cityguard_see(cn,co,1); break;
                        default:        ret=0;
                }
                if (ret) return 1;
        }

        if (map[ch[cn].x+ch[cn].y*MAPX].flags&MF_INDOORS) indoor1=1;
        else indoor1=0;
        if (map[ch[co].x+ch[co].y*MAPX].flags&MF_INDOORS) indoor2=1;
        else indoor2=0;

        // check if this is an enemy we added to our list earlier
        if (!ch[cn].attack_cn) { // only attack him if we aren't fighting already
                idx=co|(char_id(co)<<16);

                for (n=80; n<92; n++) {
                        if (ch[cn].data[n]==idx) {
                                ch[cn].attack_cn=co;
                                ch[cn].goto_x=0; // cancel goto (patrol)
                                ch[cn].data[58]=2;
                                return 1;
                        }
                }
        }

        // check if we need to attack him (by group)
        if (ch[cn].data[43]) {
                for (n=43; n<47; n++) {
                        if (ch[cn].data[n] && ch[co].data[42]==ch[cn].data[n]) break;
                        if (ch[cn].data[n]==65536 && (IS_PLAYER(co) || (ch[co].temp==CT_COMPANION))) break;
                }
                if (n==47) {
                        if (ch[cn].data[95]==2 && ch[cn].data[93]) { // attack distance
                                dist=max(abs((ch[cn].data[29]%MAPX)-ch[co].x),abs((ch[cn].data[29]/MAPX)-ch[co].y));
                                if (dist>ch[cn].data[93]) {
                                        co=0;
                                }
                        }
                        if (co && npc_add_enemy(cn,co,0)) {
                                npc_saytext_n(cn, 1, ch[co].name);
                                chlog(cn,"Added %s to kill list because he's not in my group",ch[co].name);
                                return 1;
                        }
                }
        }

        // attack with warning
        if (ch[cn].data[95]==1 && (ch[co].flags&(CF_PLAYER)) && globs->ticker>ch[cn].data[27]+TICKS*120) {
                x1=ch[co].x;
                x2=ch[cn].data[29]%MAPX;
                y1=ch[co].y;
                y2=ch[cn].data[29]/MAPX;
                dist=abs(x1-x2)+abs(y1-y2);
                if (dist<=ch[cn].data[93]) {
                        if (npc_add_enemy(cn,co,0)) {
                                npc_saytext_n(cn, 1, ch[co].name);
                                chlog(cn,"Added %s to kill list because he didn't say the password.",ch[co].name);
                                return 1;
                        }
                } else if (dist<=ch[cn].data[93]*2 && ch[cn].data[94]+TICKS*15<globs->ticker) {
                        npc_saytext_n(cn, 8, NULL);
                        ch[cn].data[94]=globs->ticker;
                        return 1;
                }
        }

        // check if we need to talk to him
        if (!ch[cn].attack_cn && (ch[co].flags&(CF_PLAYER)) && ch[cn].data[37] && indoor1==indoor2 && ch[cn].data[56]<globs->ticker) {
                for (n=37; n<41; n++)
                        if (ch[cn].data[n]==co) break;
                if (n==41) {
                        if (strcmp(ch[cn].text[2],"#stunspec")==0) {
                                if ((ch[co].kindred&KIN_TEMPLAR) || (ch[co].kindred&KIN_ARCHTEMPLAR) || ((ch[co].kindred&KIN_SEYAN_DU) && ch[co].skill[19][0]))
                                        do_sayx(cn,"Hello, %s. I'll teach you Immunity, if you bring me the potion from the Skeleton Lord.",ch[co].name);
                                else
                                        do_sayx(cn,"Hello, %s. I'll teach you Stun, if you bring me the potion from the Skeleton Lord.",ch[co].name);
                        } else if (strcmp(ch[cn].text[2],"#cursespec")==0) {
                                if ((ch[co].kindred&KIN_TEMPLAR) || (ch[co].kindred&KIN_ARCHTEMPLAR) || ((ch[co].kindred&KIN_SEYAN_DU) && ch[co].skill[19][0]))
                                        do_sayx(cn,"Hi, %s. Bring me a Potion of Life and I'll teach you Surround Hit.",ch[co].name);
                                else
                                        do_sayx(cn,"Hi, %s. Bring me a Potion of Life and I'll teach you Curse.",ch[co].name);
                        } else {
				// here is message filter, if talking NPC is priest and player char is PURPLE we greet him								
				if((ch[cn].temp == 180) && (ch[co].kindred&KIN_PURPLE)) { // priest template is 180
				    do_sayx(cn,"Greetings, %s!",ch[co].name);
				} else { // if it is not priest or player is not purple we do nothing, NPC will talk as usual
				    npc_saytext_n(cn, 2, ch[co].name);
				}
			    }
			
                        ch[cn].data[40]=ch[cn].data[39];
                        ch[cn].data[39]=ch[cn].data[38];
                        ch[cn].data[38]=ch[cn].data[37];
                        ch[cn].data[37]=co;
                        ch[cn].data[56]=globs->ticker+TICKS*30;

                        if (ch[cn].data[26]==5) { // special proc for unique warning

                                cnt=count_uniques(co);

                                if (cnt==1) {
                                        do_sayx(cn,"I see you have a sword dedicated to the gods. Make good use of it, %s.\n",ch[co].name);
                                } else if (cnt>1) {
                                        do_sayx(cn,"I see you have several swords dedicated to the gods. They will get angry if you keep more than one, %s.\n",ch[co].name);
                                }
                        }
                }
        }
        return 0;
}

int npc_died(int cn,int co)
{
        if (ch[cn].data[48] && co) {
                if (RANDOM(100)<ch[cn].data[48]) {
                        npc_saytext_n(cn, 3, ch[co].name);
                }
                return 1;
        }
        return 0;
}

int npc_shout(int cn,int co,int code,int x,int y)
{
        if (ch[cn].data[53] && ch[cn].data[53]==code) { // someone called help. If it's our code, we come to the rescue
                ch[cn].data[92]=TICKS*60;
                ch[cn].data[54]=x+y*MAPX;
                ch[cn].data[55]=globs->ticker;
                npc_saytext_n(cn, 5, ch[co].name);

                // cancel current long-term actions:
                ch[cn].goto_x=0; ch[cn].misc_action=0;

                return 1;
        }

        return 0;
}

int npc_hitme(int cn,int co)
{
        if (!do_char_can_see(cn,co)) return 1;

        if (ch[cn].data[26]==4 || ch[cn].data[26]==2) { // generic or shiva
                if (npc_add_enemy(cn,co,1)) {
                        npc_saytext_n(cn, 1, ch[co].name);
                        chlog(cn,"Added %s to kill list for stepping on trap",ch[co].name);
                        return 1;
                }
        }
        return 0;
}

int npc_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4)
{
        if (ch[cn].data[25]) {
                switch(ch[cn].data[25]) {
                        case    1:      return npc_stunrun_msg(cn,type,dat1,dat2,dat3,dat4);
                        case    2:      return npc_cityattack_msg(cn,type,dat1,dat2,dat3,dat4);
                        case    3:      return npc_malte_msg(cn,type,dat1,dat2,dat3,dat4);
                        default:        chlog(cn,"unknown special driver %d",ch[cn].data[25]); break;
                }
                return 0;
        }
        switch(type) {
                case    NT_GOTHIT:      return npc_gothit(cn,dat1,dat2);
                case    NT_GOTMISS:     return npc_gotmiss(cn,dat1);
                case    NT_DIDHIT:      return npc_didhit(cn,dat1,dat2);
                case    NT_DIDMISS:     return npc_didmiss(cn,dat1);
                case    NT_DIDKILL:     return npc_didkill(cn,dat1);
                case    NT_GOTEXP:      return npc_gotexp(cn,dat1);
                case    NT_SEEKILL:     return npc_seekill(cn,dat1,dat2);
                case    NT_SEEHIT:      return npc_seehit(cn,dat1,dat2);
                case    NT_SEEMISS:     return npc_seemiss(cn,dat1,dat2);
                case    NT_GIVE:        return npc_give(cn,dat1,dat2,dat3);
                case    NT_SEE:         return npc_see(cn,dat1);
                case    NT_DIED:        return npc_died(cn,dat1);
                case    NT_SHOUT:       return npc_shout(cn,dat1,dat2,dat3,dat4);
                case    NT_HITME:       return npc_hitme(cn,dat1);
                default:                xlog("Unknown NPC message for %d (%s): %d",cn,ch[cn].name,type);
                                        return 0;
        }
}

// ********************************
// *       High Prio Driver       *
// ********************************

int get_spellcost(int cn,int spell)
{
        switch(spell) {
                case    SK_BLAST:       return ch[cn].skill[SK_BLAST][5]/5;
                case    SK_IDENT:       return 50;
                case    SK_CURSE:       return 35;
                case    SK_BLESS:       return 35;
                case    SK_ENHANCE:     return 15;
                case    SK_PROTECT:     return 15;
                case    SK_LIGHT:       return 5;
                case    SK_STUN:        return 20;
                case    SK_HEAL:        return 25;
                case    SK_GHOST:       return 45;
		case    SK_MSHIELD:     return 25;
		case	SK_RECALL:	return 15;
                default:                return 9999;
        }
}

int spellflag(int spell)
{
        switch(spell) {
                case    SK_LIGHT:       return SP_LIGHT;
                case    SK_PROTECT:     return SP_PROTECT;
                case    SK_ENHANCE:     return SP_ENHANCE;
                case    SK_BLESS:       return SP_BLESS;
                case    SK_HEAL:        return SP_HEAL;
                case    SK_CURSE:       return SP_CURSE;
                case    SK_STUN:        return SP_STUN;
                case    SK_DISPEL:      return SP_DISPEL;
                default:                return 0;
        }
}

int npc_try_spell(int cn,int co,int spell)
{
        int mana,n,in,tmp;

        if (ch[cn].flags&CF_NOMAGIC) return 0;
        if (ch[co].used!=USE_ACTIVE) return 0;
        if (ch[co].flags&CF_BODY) return 0;

	if (!ch[cn].skill[spell][0]) // we don't know this spell
                return 0;

	if (ch[co].flags&CF_STONED) return 0;   // he's stoned, dont spell him

	// dont blast if enemies armor is too strong
        if (spell==SK_BLAST && ch[cn].skill[SK_BLAST][5]-ch[co].armor<10) return 0;

        // dont curse if chances of success are bad
        if (spell==SK_CURSE && 10*ch[cn].skill[SK_CURSE][5]/max(1,ch[co].skill[SK_RESIST][5])<7) {
	    return 0;
	}

        // dont stun if chances of success are bad
        if (spell==SK_STUN && 10*ch[cn].skill[SK_STUN][5]/max(1,ch[co].skill[SK_RESIST][5])<5) {
	    return 0;
	}

	for (n=0; n<20; n++)
                if ((in=ch[cn].spell[n]) && it[in].temp==SK_BLAST) break;
        if (n<20) return 0;

	mana=ch[cn].a_mana/1000;

        for (n=0; n<20; n++) {
                if ((in=ch[co].spell[n])!=0) {
                        if (it[in].temp==spell && it[in].power+10>=spell_immunity(ch[cn].skill[spell][5],ch[co].skill[SK_IMMUN][5]) && it[in].active>it[in].duration/2) break;
                }
        }
	
        if (n==20) {
                tmp=spellflag(spell);
                if (mana>=get_spellcost(cn,spell) && !(ch[co].data[96]&tmp)) {
                        ch[cn].skill_nr=spell;
                        ch[cn].skill_target1=co;
                        ch[co].data[96]|=tmp;
                        fx_add_effect(11,8,co,tmp,0);
                        return 1;
                }
        }

        return 0;
}

int npc_can_spell(int cn,int co,int spell)
{
        if (ch[cn].a_mana/1000<get_spellcost(cn,spell)) return 0;
        if (ch[cn].skill[spell][0]==0) return 0;
        if (ch[co].skill[spell][5]>ch[cn].skill[spell][5]) return 0;

        return 1;
}

int npc_quaff_potion(int cn,int itemp,int stemp)
{
        int n,in;

        for (n=0; n<20; n++)
                if ((in=ch[cn].spell[n]) && it[in].temp==stemp) return 0;   // potion already active

        for (n=0; n<40; n++) if ((in=ch[cn].item[n]) && it[in].temp==itemp) break;       // find potion
        if (n==40) return 0;                                                            // no potion :(

        do_area_log(cn,0,ch[cn].x,ch[cn].y,1,"The %s uses a %s.\n",ch[cn].name,it[in].name);

        use_driver(cn,in,1);

        return 1;
}

void die_companion(int cn)
{
        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);
        god_destroy_items(cn);
	ch[cn].gold=0;
        do_char_killed(0,cn);
}

int npc_driver_high(int cn)
{
        int x,y,in,co,indoor1,indoor2,cc,in2;

        if (ch[cn].data[25]) {
                switch(ch[cn].data[25]) {
                        case    1:      return npc_stunrun_high(cn);
                        case    2:      return npc_cityattack_high(cn);
                        case    3:      return npc_malte_high(cn);
                        default:        chlog(cn,"unknown special driver %d",ch[cn].data[25]); break;
                }
                return 0;
        }

        // reset panic mode if expired
        if (ch[cn].data[78]<globs->ticker) ch[cn].data[78]=0;

        // self destruct
        if (ch[cn].data[64]) {
                if (ch[cn].data[64]<TICKS*60*15) // values less than 15 minutes mean it's a value set from the administration
                                                 // interface and shall wait that long...
                        ch[cn].data[64]+=globs->ticker;

                if (ch[cn].data[64]<globs->ticker) {
                        do_sayx(cn,"Free!");
                        god_destroy_items(cn);
                        plr_map_remove(cn);
                        ch[cn].used=USE_EMPTY;
                        remove_enemy(cn);
                        return 1;
                }
        }

        // Count down master-no-see timer for player ghost companions
        if (ch[cn].temp==CT_COMPANION && ch[cn].data[64]==0) {
                co=ch[cn].data[CHD_MASTER]; // master
                if (!IS_SANECHAR(co) || ch[co].data[64] != cn) { // mismatch
                        chlog(cn, "killed for bad master(%d)", co);
                        die_companion(cn);
                        return 1;
                }
                if (globs->ticker > ch[cn].data[98]) {
                        ch[co].luck--;
                        chlog(cn, "Self-destructed because of neglect by %s",ch[co].name);
                        die_companion(cn);
                        return 1;
                }
        }

        // Count down riddle timeout for riddle givers
        if ((ch[cn].data[72] >= RIDDLE_MIN_AREA) && (ch[cn].data[72] <= RIDDLE_MAX_AREA)) {
                int idx;

                idx = ch[cn].data[72] - RIDDLE_MIN_AREA;
                if (riddletimeout[idx] > 0) {
                        if (--riddletimeout[idx] <= 0) {
                                // Guesser forgets riddler
                                if (IS_SANECHAR(guesser[idx]) && IS_PLAYER(guesser[idx])) {
                                        ch[guesser[idx]].data[CHD_RIDDLER] = 0;
                                }
                                guesser[idx] = 0;
                                do_char_log(cn,1,"%s tells you: Too late! Too late! Try again some time.\n",ch[cn].name);
                        }
                }
        }

        // heal us if we're hurt
        if (ch[cn].a_hp<ch[cn].hp[5]*600) {
                if (npc_try_spell(cn,cn,SK_HEAL)) return 1;
        }

        // donate/destroy citem if that's our job
        if ((in=ch[cn].citem)!=0 && ch[cn].data[47]) {
                if (it[in].damage_state || (it[in].flags&IF_SHOPDESTROY) || !(it[in].flags&IF_DONATE)) {
                        it[in].used=USE_EMPTY;
                        ch[cn].citem=0;
                } else {
                        it[in].current_age[0]=0;
                        it[in].current_age[1]=0;
                        it[in].current_damage=0;
                        god_donate_item(in,ch[cn].data[47]);
                        ch[cn].citem=0;
                }
        }

        if ((in=ch[cn].item[39])!=0 && ch[cn].data[47]) {
                if (it[in].damage_state || (it[in].flags&IF_SHOPDESTROY) || !(it[in].flags&IF_DONATE)) {
                        it[in].used=USE_EMPTY;
                        ch[cn].citem=0;
                } else {
                        it[in].current_age[0]=0;
                        it[in].current_age[1]=0;
                        it[in].current_damage=0;
                        god_donate_item(in,ch[cn].data[47]);
                        ch[cn].item[39]=0;
                }
        }

        // generic spell management
        if (ch[cn].a_mana>ch[cn].mana[5]*850 && ch[cn].skill[SK_MEDIT][0]) {
                if (ch[cn].a_mana>75000 && npc_try_spell(cn,cn,SK_BLESS)) return 1;
                if (npc_try_spell(cn,cn,SK_PROTECT)) return 1;
                if (npc_try_spell(cn,cn,SK_MSHIELD)) return 1;
                if (npc_try_spell(cn,cn,SK_ENHANCE)) return 1;
                if (npc_try_spell(cn,cn,SK_BLESS)) return 1;
        }

        // generic endurance management
        if (ch[cn].data[58]>1 && ch[cn].a_end>10000) {
                if (ch[cn].mode!=2) {
                        ch[cn].mode=2;
                        do_update_char(cn);
                }
        } else if (ch[cn].data[58]==1 && ch[cn].a_end>10000) {
                if (ch[cn].mode!=1) {
                        ch[cn].mode=1;
                        do_update_char(cn);
                }
        } else if (ch[cn].mode!=0) {
                ch[cn].mode=0;
                do_update_char(cn);
        }

        // create light
        if (ch[cn].data[62]>ch[cn].data[58] && check_dlight(ch[cn].x,ch[cn].y)<20 && map[ch[cn].x+ch[cn].y*MAPX].light<20) {
                if (npc_try_spell(cn,cn,SK_LIGHT)) return 1;
        }

        // make sure protected character survives
        if ((co=ch[cn].data[63])!=0) {
                if (ch[co].a_hp<ch[co].hp[5]*600) { // he's hurt
                        if (npc_try_spell(cn,co,SK_HEAL)) return 1;
                }
        }

        // help friend
        if ((co=ch[cn].data[65])!=0) {
                cc=ch[co].attack_cn;

                // bless us first if we have enough mana - makes spells more powerful
                if (ch[cn].a_mana>get_spellcost(cn,SK_BLESS)*2+get_spellcost(cn,SK_PROTECT)+get_spellcost(cn,SK_ENHANCE)) {
                        if (npc_try_spell(cn,cn,SK_BLESS)) return 1;
                }

                if (ch[co].a_hp<ch[co].hp[5]*600) { // he's hurt
                        if (npc_try_spell(cn,co,SK_HEAL)) return 1;
                }

                if (!npc_can_spell(co,cn,SK_PROTECT) && npc_try_spell(cn,co,SK_PROTECT)) return 1;
                if (!npc_can_spell(co,cn,SK_ENHANCE) && npc_try_spell(cn,co,SK_ENHANCE)) return 1;
                if (!npc_can_spell(co,cn,SK_BLESS) && npc_try_spell(cn,co,SK_BLESS)) return 1;

                // blast his enemy if our friend is losing and his enemy is our enemy as well
                if (cc && ch[co].a_hp<ch[co].hp[5]*650 && npc_is_enemy(cn,cc)) {
                        if (npc_try_spell(cn,cc,SK_BLAST)) return 1;
                }
                ch[cn].data[65]=0;
        }

        // generic fight-magic managemant
        if ((co=ch[cn].attack_cn)!=0 || ch[cn].data[78]) { // we're fighting

                if (npc_quaff_potion(cn,833,254)) return 1;         // use greenling pot if available
                if (npc_quaff_potion(cn,267,254)) return 1;         // use ratling pot if available

                if (co && (ch[cn].a_hp<ch[cn].hp[5]*600 || !RANDOM(10))) { // we're losing
                        if (npc_try_spell(cn,co,SK_BLAST)) return 1;
                }

                if (co && globs->ticker>ch[cn].data[75] && npc_try_spell(cn,co,SK_STUN)) {
                        ch[cn].data[75]=globs->ticker+ch[cn].skill[SK_STUN][5]+18*8;
                        return 1;
                }
                if (ch[cn].a_mana>75000 && npc_try_spell(cn,cn,SK_BLESS)) return 1;
                if (npc_try_spell(cn,cn,SK_PROTECT)) return 1;
                if (npc_try_spell(cn,cn,SK_MSHIELD)) return 1;
                if (npc_try_spell(cn,cn,SK_ENHANCE)) return 1;
                if (npc_try_spell(cn,cn,SK_BLESS)) return 1;
                if (co && npc_try_spell(cn,co,SK_CURSE)) return 1;
                if (co && globs->ticker>ch[cn].data[74]+TICKS*10 && npc_try_spell(cn,co,SK_GHOST)) {
                        ch[cn].data[74]=globs->ticker;
                        return 1;
                }

                if (co && ch[co].armor+5>ch[cn].weapon) { // blast always if we cannot hurt him otherwise
                        if (npc_try_spell(cn,co,SK_BLAST)) return 1;
                }
        }

        // did we panic?
        if (ch[cn].data[78] && !ch[cn].attack_cn && !ch[cn].goto_x) {
                ch[cn].goto_x=ch[cn].x+5-RANDOM(10);
                ch[cn].goto_y=ch[cn].y+5-RANDOM(10);
                return 1;
        }

        // are we on protect and want to follow our master?
        if (!ch[cn].attack_cn && (co=ch[cn].data[69])!=0 && follow_driver(cn,ch[cn].data[69])) {
                if (abs(ch[cn].x-ch[co].y)+abs(ch[cn].y-ch[co].y)>6) ch[cn].data[58]=2;
                else ch[cn].data[58]=1;
                return 1;
        }

        if (!ch[cn].data[41] && !ch[cn].data[47]) return 0;     // don't scan if we don't use the information anyway

        // save some work. you need to check here if no other work needs to be done!
        if (ch[cn].data[41] && ch[cn].misc_action==DR_USE) return 0;
        if (ch[cn].data[47] && ch[cn].misc_action==DR_PICKUP) return 0;
        if (ch[cn].data[47] && ch[cn].misc_action==DR_USE) return 0;

        if (map[ch[cn].x+ch[cn].y*MAPX].flags&MF_INDOORS) indoor1=1;
        else indoor1=0;

        for (y=max(ch[cn].y-8,1); y<min(ch[cn].y+8,MAPY-1); y++) {
                for (x=max(ch[cn].x-8,1); x<min(ch[cn].x+8,MAPX-1); x++) {
                        if ((in=map[x+y*MAPX].it)!=0) {
                                if (map[x+y*MAPX].flags&MF_INDOORS) indoor2=1;
                                else indoor2=0;

                                if (it[in].temp==ch[cn].data[41]) {
                                        if (!it[in].active && (globs->dlight<200 || indoor2)) {
                                                ch[cn].misc_action=DR_USE;
                                                ch[cn].misc_target1=x;
                                                ch[cn].misc_target2=y;
                                                ch[cn].goto_x=0;        // cancel goto, which stems probably from patrol
                                                ch[cn].data[58]=1;
                                                return 1;
                                        }

                                if (it[in].active && globs->dlight>200 && !indoor2) {
                                                ch[cn].misc_action=DR_USE;
                                                ch[cn].misc_target1=x;
                                                ch[cn].misc_target2=y;
                                                ch[cn].goto_x=0;        // cancel goto, which stems probably from patrol
                                                ch[cn].data[58]=1;
                                                return 1;
                                        }
                                }
                                if (ch[cn].data[47] && indoor1==indoor2 && (it[in].flags&IF_TAKE) &&
                                    can_go(ch[cn].x,ch[cn].y,it[in].x,it[in].y) &&
                                    do_char_can_see_item(cn,in) && it[in].temp!=18) {
                                        ch[cn].misc_action=DR_PICKUP;
                                        ch[cn].misc_target1=x;
                                        ch[cn].misc_target2=y;
                                        ch[cn].goto_x=0;
                                        ch[cn].data[58]=1;
                                        return 1;
                                }
                                if (ch[cn].data[47] && indoor1==indoor2 && (it[in].driver==7) &&
                                    can_go(ch[cn].x,ch[cn].y,it[in].x,it[in].y) &&
                                    do_char_can_see_item(cn,in)) {
                                        if (plr_check_target(x+y*MAPX+1) && !map[x+y*MAPX+1].it) {
                                                in2=god_create_item(18);
                                                it[in2].carried=cn;
                                                ch[cn].citem=in2;

                                                ch[cn].misc_action=DR_DROP;
                                                ch[cn].misc_target1=x+1;
                                                ch[cn].misc_target2=y;
                                                ch[cn].goto_x=0;
                                                ch[cn].data[58]=1;
                                                return 1;
                                        }
                                }
                        }
                }
        }

        return 0;
}


// ********************************
// *        Low Prio Driver       *
// ********************************

int shiva_activate_candle(int cn,int in)
{
        if (globs->mdtime>2000) return 0;
        if (ch[cn].data[0]>=globs->mdday) return 0;

        chlog(cn,"Created new candle, time=%d, day=%d, last day=%d",globs->mdtime,globs->mdday,ch[cn].data[0]);

        ch[cn].data[0]=globs->mdday+9;

        it[in].active=0;
        if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
                do_add_light(it[in].x,it[in].y,it[in].light[0]-it[in].light[1]);

        fx_add_effect(6,0,it[in].x,it[in].y,0);
        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        do_sayx(cn,"Shirak ishagur gorweran dulak!");

        ch[cn].a_mana-=800*1000;

        return 1;
}

// grave looting and friends

// does not check correctly for two-handed weapons
int npc_check_placement(int in,int n)
{
        switch(n) {
                case    WN_HEAD:        if (!(it[in].placement&PL_HEAD)) return 0; else break;
                case    WN_NECK:        if (!(it[in].placement&PL_NECK)) return 0; else break;
                case    WN_BODY:        if (!(it[in].placement&PL_BODY)) return 0; else break;
                case    WN_ARMS:        if (!(it[in].placement&PL_ARMS)) return 0; else break;
                case    WN_BELT:        if (!(it[in].placement&PL_BELT)) return 0; else break;
                case    WN_LEGS:        if (!(it[in].placement&PL_LEGS)) return 0; else break;
                case    WN_FEET:        if (!(it[in].placement&PL_FEET)) return 0; else break;
                case    WN_LHAND:       if (!(it[in].placement&PL_SHIELD)) return 0; else break;
                case    WN_RHAND:       if (!(it[in].placement&PL_WEAPON)) return 0; else break;
                case    WN_CLOAK:       if (!(it[in].placement&PL_CLOAK)) return 0; else break;
                case    WN_RRING:
                case    WN_LRING:       if (!(it[in].placement&PL_RING)) return 0; else break;
                default:                return 0;
        }

        return 1;
}

int npc_can_wear_item(int cn,int in)
{
        int m;

        if (in&0x80000000) return 0;

        for (m=0; m<5; m++)
                if (it[in].attrib[m][2]>ch[cn].attrib[m][0]) return 0;

        for (m=0; m<50; m++)
                if (it[in].skill[m][2]>ch[cn].skill[m][0])return 0;

        if (it[in].hp[2]>ch[cn].hp[0]) return 0;

        if (it[in].end[2]>ch[cn].end[0]) return 0;

        if (it[in].mana[2]>ch[cn].mana[0]) return 0;

        return 1;
}

int npc_item_value(int in)
{
        int score=0,n;

        for (n=0; n<50; n++) score+=it[in].attrib[n][0]*5;

        score+=it[in].value/10;

        score+=it[in].weapon[0]*50;
        score+=it[in].armor[0]*50;

        score-=it[in].damage_state;

        return score;
}

int npc_want_item(int cn,int in)
{
        if (ch[cn].item[38]) return 0;  // hack: dont take more stuff if inventory is almost full

        if (ch[cn].citem) {
                chlog(cn,"have %s in citem",it[in].name);
                if (do_store_item(cn)==-1) {
                        it[ch[cn].citem].used=USE_EMPTY;
                        ch[cn].citem=0;
                }
        }

        if (it[in].temp==833 || it[in].temp==267) {
                ch[cn].citem=in;
                it[in].carried=cn;
                do_store_item(cn);
                return 1;
        }

        return 0;
}

int npc_equip_item(int cn,int in)
{
        int n;

        if (ch[cn].citem) {
                chlog(cn,"have %s in citem",it[in].name);
                if (do_store_item(cn)==-1) {
                        it[ch[cn].citem].used=USE_EMPTY;
                        ch[cn].citem=0;
                }
        }

        for (n=0; n<20; n++) {
                if (!ch[cn].worn[n] || npc_item_value(in)>npc_item_value(ch[cn].worn[n])) {
                        if (npc_check_placement(in,n)) {
                                if (npc_can_wear_item(cn,in)) {
                                        chlog(cn,"now wearing %s",it[in].name);

                                        // remove old item if any
                                        if (ch[cn].worn[n]) {
                                                chlog(cn,"storing item");
                                                ch[cn].citem=ch[cn].worn[n];
                                                if (do_store_item(cn)==-1) return 0;    // stop looting if our backpack is full
                                        }

                                        ch[cn].worn[n]=in;
                                        it[in].carried=cn;

					do_update_char(cn);
                                        return 1;
                                }
                        }
                }
        }

        return 0;
}

int npc_loot_grave(int cn,int in)
{
        int co,n;

        if (abs(ch[cn].x-it[in].x)+abs(ch[cn].y-it[in].y)>1 || drv_dcoor2dir(it[in].x-ch[cn].x,it[in].y-ch[cn].y)!=ch[cn].dir) {
                ch[cn].misc_action=DR_USE;
                ch[cn].misc_target1=it[in].x;
                ch[cn].misc_target2=it[in].y;
                return 1;
        }

        co=it[in].data[0];

        for (n=0; n<20; n++) {
                if ((in=ch[co].worn[n])) {
                        if (npc_equip_item(cn,in)) {
                                chlog(cn,"got %s from %s's grave",it[in].name,ch[co].name);
                                ch[co].worn[n]=0;
                                return 1;
                        }
                }
        }
        for (n=0; n<40; n++) {
                if ((in=ch[co].item[n])) {
                        if (npc_equip_item(cn,in)) {
                                chlog(cn,"got %s from %s's grave",it[in].name,ch[co].name);
                                ch[co].item[n]=0;
                                return 1;
                        }
                        if (npc_want_item(cn,in)) {
                                chlog(cn,"got %s from %s's grave",it[in].name,ch[co].name);
                                ch[co].item[n]=0;
                                return 1;
                        }
                }
        }

        if (ch[co].gold) {
                chlog(cn,"got %.2fG from %s's grave",ch[co].gold/100.0,ch[co].name);
                ch[cn].gold+=ch[co].gold;
                ch[co].gold=0;
                return 1;
        }

        return 0;
}

int npc_already_searched_grave(int cn,int in)
{
        int n;

        for (n=0; n<160; n+=sizeof(int))
                if (*(int*)(ch[cn].text[9]+n)==in) return 1;

        return 0;
}

void npc_add_searched_grave(int cn,int in)
{
        memmove(ch[cn].text[9]+sizeof(int),ch[cn].text[9],sizeof(ch[cn].text[9])-sizeof(int));

        *(int*)(ch[cn].text[9])=in;
}

int npc_grave_logic(int cn)
{
        int x,y,in;

        for (y=max(ch[cn].y-8,1); y<min(ch[cn].y+8,MAPY-1); y++) {
                for (x=max(ch[cn].x-8,1); x<min(ch[cn].x+8,MAPX-1); x++) {
                        if ((in=map[x+y*MAPX].it)!=0) {
                                if (it[in].temp==170 &&
                                    can_go(ch[cn].x,ch[cn].y,it[in].x,it[in].y) &&
                                    do_char_can_see_item(cn,in) &&
                                    !npc_already_searched_grave(cn,in)) {
                                        if (!npc_loot_grave(cn,in)) {
                                                npc_add_searched_grave(cn,in);
						ch[cn].flags&=~CF_ISLOOTING;
                                        } else ch[cn].flags|=CF_ISLOOTING;
                                        return 1;
                                }
                        }
                }
        }
        return 0;
}

void npc_driver_low(int cn)
{
        int n,x,y,m,in,panic,co;

        if (ch[cn].data[25]) {	// check for special driver routine
                switch(ch[cn].data[25]) {
                        case    1:      npc_stunrun_low(cn); return;
                        case    2:      npc_cityattack_low(cn); return;
                        case    3:      npc_malte_low(cn); return;
                        default:        chlog(cn,"unknown special driver %d",ch[cn].data[25]); break;
                }
                return;
        }

        if (ch[cn].last_action==ERR_SUCCESS) ch[cn].data[36]=0;         // reset frust with successful action
        else if (ch[cn].last_action==ERR_FAILED) ch[cn].data[36]++;     // increase frust with failed action

	// are we supposed to loot graves?
        if (ch[cn].alignment<0 &&
	    (globs->flags&GF_LOOTING) &&
	    ((cn&15)==(globs->ticker&15) || (ch[cn].flags&CF_ISLOOTING)) &&
	    ch[cn].temp!=CT_COMPANION) {
                if (npc_grave_logic(cn)) return;
        }

        // did someone call help? - high prio
        if (ch[cn].data[55] && ch[cn].data[55]+TICKS*120>globs->ticker && ch[cn].data[54]) {
                m=ch[cn].data[54];
                ch[cn].goto_x=m%MAPX+get_frust_x_off(globs->ticker);
                ch[cn].goto_y=m/MAPX+get_frust_y_off(globs->ticker);
                ch[cn].data[58]=2;
                return;
        }

        // go to last known enemy position and stay there for up to 30 seconds
        if (ch[cn].data[77] && ch[cn].data[77]+TICKS*30>globs->ticker) {
                m=ch[cn].data[76];
                ch[cn].goto_x=m%MAPX+get_frust_x_off(ch[cn].data[36]);
                ch[cn].goto_y=m/MAPX+get_frust_y_off(ch[cn].data[36]);
                return;
        }

        if (ch[cn].a_hp<ch[cn].hp[5]*750) // we're hurt: rest
                return;

        for (n=20; n<24; n++) { // close door, medium prio
                if ((m=ch[cn].data[n])!=0) {
                        // check if the door is free:
                        if (!map[m].ch && !map[m].to_ch &&
                            !map[m+1].ch && !map[m+1].to_ch &&
                            !map[m-1].ch && !map[m-1].to_ch &&
                            !map[m+MAPX].ch && !map[m+MAPX].to_ch &&
                            !map[m-MAPX].ch && !map[m-MAPX].to_ch) {
                                in=map[m].it;
                                if (in && it[in].active) {
                                        ch[cn].misc_action=DR_USE;
                                        ch[cn].misc_target1=m%MAPX;
                                        ch[cn].misc_target2=m/MAPX;
                                        ch[cn].data[58]=1;
                                        return;
                                }
                        }
                }
        }

        for (n=32; n<36; n++) { // activate light, medium prio
                if ((m=ch[cn].data[n])!=0 && m<MAPX*MAPY) {
                        in=map[m].it;
                        if (in && !it[in].active) {
                                ch[cn].misc_action=DR_USE;
                                ch[cn].misc_target1=m%MAPX;
                                ch[cn].misc_target2=m/MAPX;
                                ch[cn].data[58]=1;
                                return;
                        }
                }
        }

        if (ch[cn].data[10]) {  // patrol, low
                n=ch[cn].data[19];
                if (n<10 || n>18) { ch[cn].data[19]=n=10; }

                if (ch[cn].data[57]>globs->ticker) return;

                m=ch[cn].data[n];
                x=m%MAPX+get_frust_x_off(ch[cn].data[36]);
                y=m/MAPX+get_frust_y_off(ch[cn].data[36]);
                if (ch[cn].data[36]>20 || abs(ch[cn].x-x)+abs(ch[cn].y-y)<4) {
                        if (ch[cn].data[36]<=20) {
                                if (ch[cn].data[79]) ch[cn].data[57]=globs->ticker+ch[cn].data[79];
                        }

                        n++;
                        if (n>18) n=10;
                        if (ch[cn].data[n]==0) n=10;

                        ch[cn].data[19]=n;
                        ch[cn].data[36]=0;


                        return;
                }
                ch[cn].goto_x=x;
                ch[cn].goto_y=y;
                ch[cn].data[58]=0;
                return;
        }
        if (ch[cn].data[60]) { //random walk, low
                ch[cn].data[58]=0;
                if (ch[cn].data[61]<1) {
                        ch[cn].data[61]=ch[cn].data[60];

                        for (panic=0; panic<5; panic++) {
                                x=ch[cn].x-5+RANDOM(11);
                                y=ch[cn].y-5+RANDOM(11);

                                if (x<1 || x>=MAPX || y<1 || y>MAPX) continue;

                                if (ch[cn].data[73]) { // too far away from origin?
                                        int xo,yo;

                                        xo=ch[cn].data[29]%MAPX;
                                        yo=ch[cn].data[29]/MAPX;

                                        if (abs(x-xo)+abs(y-yo)>ch[cn].data[73]) {
                                                if (plr_check_target(xo+yo*MAPX)) {
                                                        ch[cn].goto_x=xo;
                                                        ch[cn].goto_y=yo;
                                                        return;
                                                } else if (plr_check_target(xo+1+yo*MAPX)) {
                                                        ch[cn].goto_x=xo+1;
                                                        ch[cn].goto_y=yo;
                                                        return;
                                                } else if (plr_check_target(xo-1+yo*MAPX)) {
                                                        ch[cn].goto_x=xo-1;
                                                        ch[cn].goto_y=yo;
                                                        return;
                                                } else if (plr_check_target(xo+yo*MAPX+MAPX)) {
                                                        ch[cn].goto_x=xo;
                                                        ch[cn].goto_y=yo+1;
                                                        return;
                                                } else if (plr_check_target(xo+yo*MAPX-MAPX)) {
                                                        ch[cn].goto_x=xo;
                                                        ch[cn].goto_y=yo-1;
                                                        return;
                                                } else  continue;
                                        }
                                }

                                if (!plr_check_target(x+y*MAPX)) continue;
                                if (!can_go(ch[cn].x,ch[cn].y,x,y)) continue;
                                break;
                        }

                        if (panic==5) return;

                        ch[cn].goto_x=x;
                        ch[cn].goto_y=y;
                        return;

                } else ch[cn].data[61]--;

                return;
        }

        if (ch[cn].data[29]) {  // resting position, lowest prio
                m=ch[cn].data[29];
		x=m%MAPX+get_frust_x_off(ch[cn].data[36]);
                y=m/MAPX+get_frust_y_off(ch[cn].data[36]);

                ch[cn].data[58]=0;
                if (ch[cn].x!=x || ch[cn].y!=y) {
			ch[cn].goto_x=x;
			ch[cn].goto_y=y;
                        return;
                }

                if (ch[cn].dir!=ch[cn].data[30]) {
                        ch[cn].misc_action=DR_TURN;

                        switch(ch[cn].data[30]) {
                                case DX_UP:             ch[cn].misc_target1=x; ch[cn].misc_target2=y-1; break;
                                case DX_DOWN:           ch[cn].misc_target1=x; ch[cn].misc_target2=y+1; break;
                                case DX_LEFT:           ch[cn].misc_target1=x-1; ch[cn].misc_target2=y; break;
                                case DX_RIGHT:          ch[cn].misc_target1=x+1; ch[cn].misc_target2=y; break;
                                case DX_LEFTUP:         ch[cn].misc_target1=x-1; ch[cn].misc_target2=y-1; break;
                                case DX_LEFTDOWN:       ch[cn].misc_target1=x-1; ch[cn].misc_target2=y+1; break;
                                case DX_RIGHTUP:        ch[cn].misc_target1=x+1; ch[cn].misc_target2=y-1; break;
                                case DX_RIGHTDOWN:      ch[cn].misc_target1=x+1; ch[cn].misc_target2=y+1; break;
                                default:                ch[cn].misc_action=DR_IDLE; break;
                        }
                        return;
                }
        }

        // reset talked-to list
        if (ch[cn].data[67]+(TICKS*60*5)<globs->ticker) {
                if (ch[cn].data[37]) for (n=37; n<41; n++) ch[cn].data[37]=1;   // hope we never have a character nr 1!
                ch[cn].data[67]=globs->ticker;
        }

        // special sub-proc for Shiva (black stronghold mage)
        if (ch[cn].data[26]==2 && ch[cn].a_mana>ch[cn].mana[5]*900) {
                for (n=1,m=0; n<MAXCHARS; n++) {
                        if (ch[n].used!=USE_ACTIVE) continue;
                        if (ch[n].flags&(CF_BODY|CF_RESPAWN)) continue;
                        if (ch[n].data[42]==27) m++;
                }

                if (m<15) {
                        n=0;

                        in=map[446+347*MAPX].it;
                        if (in) {
                                if (!it[in].active) n++;
                                else { if (shiva_activate_candle(cn,in)) return; }
                        }
                        in=map[450+351*MAPX].it;
                        if (in) {
                                if (!it[in].active) n++;
                                else { if (shiva_activate_candle(cn,in)) return; }
                        }
                        in=map[457+348*MAPX].it;
                        if (in) {
                                if (!it[in].active) n++;
                                else { if (shiva_activate_candle(cn,in)) return; }
                        }
                        in=map[457+340*MAPX].it;
                        if (in) {
                                if (!it[in].active) n++;
                                else { if (shiva_activate_candle(cn,in)) return; }
                        }
                        in=map[449+340*MAPX].it;
                        if (in) {
                                if (!it[in].active) n++;
                                else { if (shiva_activate_candle(cn,in)) return; }
                        }

                        if (n) {

                                for (m=0; m<n; m++) {
                                        co=pop_create_char(503+m,0);
                                        if (!co) { do_sayx(cn,"create char (%d)",m); break; }
                                        if (!god_drop_char_fuzzy(co,452,345)) {
                                                do_sayx(cn,"drop char (%d)",m);
                                                god_destroy_items(co);
                                                ch[co].used=USE_EMPTY;
                                                break;
                                        }
                                        fx_add_effect(6,0,ch[co].x,ch[co].y,0);
                                }
                                fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);
                                do_sayx(cn,"Khuzak gurawin duskar!");
                                ch[cn].a_mana-=n*100*1000;
                                chlog(cn,"created %d new monsters",n);
                        }
                }
                ch[cn].a_mana-=100*1000;
        }
}

void update_shop(int cn)
{
        int n,m,x,z,in,temp;
        int sale[10];

        for (n=0; n<10; n++) sale[n]=ch[cn].data[n];

        // check if we have free space (at least 10)
        do_sort(cn,"v");
        for (n=m=x=0; n<40; n++) if (!(in=ch[cn].item[n])) m++; else {
                for (z=0; z<10; z++) if (it[in].temp==sale[z]) { sale[z]=0; break; }
                if (z==10) x=n;
        }
        if (m<2 && ch[cn].item[x]) {
		in=ch[cn].item[x];

                if (RANDOM(2) && (it[in].flags&IF_DONATE)) {
                        god_donate_item(in,0);
                } else {
                        it[in].used=USE_EMPTY;
                }
                ch[cn].item[x]=0;
        }

        // check if our store is complete
        for (n=0; n<10; n++) {
                temp=sale[n];
                if (!temp) continue;
                in=god_create_item(temp);
                if (in) {
                        if (!god_give_char(in,cn)) it[in].used=USE_EMPTY;
                }
        }

        // small-repair all items (set current max_damage and age to zero)
        // junk all items needing serious repair
        for (n=0; n<40; n++) {
                if ((in=ch[cn].item[n])!=0) {
                        if (it[in].damage_state || (it[in].flags&IF_SHOPDESTROY)) {
                                it[in].used=USE_EMPTY;
                                ch[cn].item[n]=0;
                        } else {
                                it[in].current_damage=0;
                                it[in].current_age[0]=0;
                                it[in].current_age[1]=0;
                        }
                }
        }
        do_sort(cn,"v");
}
