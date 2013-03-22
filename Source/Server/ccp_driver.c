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

#ifdef REAL_CCP

struct ccp_mem
{
    int lastshout;
    int fighting;
    int enemy_strength;

	int level;

	int sector[MAPX/10][MAPY/10];
};

static struct ccp_mem *ccp_mem[MAXCHARS];

struct ccp_mem *get_ccp_mem(int cn)
{
    struct ccp_mem *mem;

    mem=ccp_mem[cn];
    if (!mem) {
	mem=calloc(sizeof(struct ccp_mem),1);
	ccp_mem[cn]=mem;
    }
    return mem;
}

void ccp_sector_score(int cn,int val)
{
	struct ccp_mem *mem=get_ccp_mem(cn);
	int x,y;

	x=ch[cn].x/10;
	y=ch[cn].y/10;

	mem->sector[x][y]+=val;	
}
#endif

void ccp_tell(int cn,int co,char *text)
{
#ifdef REAL_CCP
    if (ch[co].flags&CF_CCP) return;
    do_tell(cn,ch[co].name,"Sorry, I'm just a robot, I cannot really talk back.");
#endif
}

void ccp_shout(int cn,int co,char *text)
{
#ifdef REAL_CCP
    struct ccp_mem *mem=get_ccp_mem(cn);

    if (mem->lastshout>globs->ticker+TICKS*10) return;

    if (strstr(text,ch[cn].name)) {
	do_shout(cn,"I'm just a robot, stop shouting my name!");
	mem->lastshout=globs->ticker;
    }
#endif
}

#ifdef REAL_CCP
int ccp_set_enemy(int cn,int co)
{
    struct ccp_mem *mem=get_ccp_mem(cn);

    mem->fighting=co;

	do_char_log(cn,1,"co pts=%d, cn pts=%d\n",
		ch[co].points_tot,
		ch[cn].points_tot);

    if (ch[co].points_tot>ch[cn].points_tot*4) mem->enemy_strength=7;
    else if (ch[co].points_tot>ch[cn].points_tot*2) mem->enemy_strength=6;
    else if (ch[co].points_tot>(ch[cn].points_tot*3)/2) mem->enemy_strength=5;
    else if (ch[co].points_tot>ch[cn].points_tot) mem->enemy_strength=4;
    else if (ch[co].points_tot>(ch[cn].points_tot*2)/3) mem->enemy_strength=3;
    else if (ch[co].points_tot>ch[cn].points_tot/2) mem->enemy_strength=2;
    else if (ch[co].points_tot>ch[cn].points_tot/4) mem->enemy_strength=1;
	else mem->enemy_strength=0;

	do_char_log(cn,1,"set enemy %d (%s), strength=%d\n",co,ch[co].name,mem->enemy_strength);

	return 1;
}


void ccp_gotattack(int cn,int co)
{
	struct ccp_mem *mem=get_ccp_mem(cn);

	if (co==mem->fighting) return;

	if (!mem->fighting) {
		ch[cn].attack_cn=co;
		ccp_set_enemy(cn,co);
		return;
	}
}

void ccp_seen(int cn,int co)
{
	struct ccp_mem *mem=get_ccp_mem(cn);

	if (ch[co].attack_cn==cn) {	// he's attacking us
		if (ch[co].points_tot>ch[cn].points_tot) ccp_sector_score(cn,-10000);
	}

	if (co==mem->fighting) return;

	if (ch[cn].a_hp<ch[cn].hp[5]*800) return;
	if (ch[cn].a_mana<ch[cn].hp[5]*800) return;
	
	if (ch[co].flags&CF_PLAYER) return;
	if (ch[co].alignment>0) return;
	if (IS_COMPANION(co)) return;

	if (!do_char_can_see(cn,co)) return;
	if (!can_go(ch[cn].x,ch[cn].y,ch[co].x,ch[co].y)) return;

	if (ch[co].points_tot>(ch[cn].points_tot*3)/2) return;	// too strong

	if (ch[co].points_tot<ch[cn].points_tot/3) {			// too weak
		if (ch[co].points_tot>ch[cn].points_tot) ccp_sector_score(cn,-3000);
		return;
	}

	ccp_set_enemy(cn,co);
	
	ccp_sector_score(cn,1000);
}

int ccp_raise(int cn)
{
	struct ccp_mem *mem=get_ccp_mem(cn);
	int n;
	static int skill[]={SK_DAGGER,SK_MEDIT,SK_BLAST,SK_CURSE,SK_STUN,SK_ENHANCE,SK_PROTECT,SK_BLESS,SK_MSHIELD};

    for (n=0; n<5; n++)
		if (ch[cn].attrib[n][0]<mem->level) return do_raise_attrib(cn,n);

	for (n=0; n<sizeof(skill)/sizeof(int); n++)
		if (ch[cn].skill[skill[n]][0] && ch[cn].skill[skill[n]][0]<mem->level*2) return do_raise_skill(cn,skill[n]);

	mem->level++;

	do_char_log(cn,1,"raise new level=%d\n",mem->level);

	return 1;
}

void ccp_gotexp(int cn)
{
	int dontpanic=10;

	while (ccp_raise(cn) && dontpanic) dontpanic--;
}
#endif

void ccp_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4)
{
#ifdef REAL_CCP
	switch (type) {
		case NT_GOTMISS:
		case NT_GOTHIT:		ccp_gotattack(cn,dat1); break;
		case NT_SEE:		ccp_seen(cn,dat1); break;
		case NT_GOTEXP:		ccp_gotexp(cn); break;
	}
#endif
}

#ifdef REAL_CCP
int ccp_at_recall_point(int cn)
{
    if (abs(ch[cn].x-ch[cn].temple_x)+abs(ch[cn].y-ch[cn].temple_y)<5) return 1;
    return 0;
}

void ccp_goto_sector(int cn)
{
	int x,y,best=-999999999,bx,by;
	struct ccp_mem *mem=get_ccp_mem(cn);

	x=ch[cn].x/10;
	y=ch[cn].y/10;

    best=mem->sector[x][y]; bx=x; by=y;

	if (x>0 && mem->sector[x-1][y]>best)         { best=mem->sector[x-1][y]; bx=x-1; by=y; }
	if (x<MAPX/10-1 && mem->sector[x+1][y]>best) { best=mem->sector[x+1][y]; bx=x+1; by=y; }
	if (y>0 && mem->sector[x][y-1]>best)         { best=mem->sector[x][y-1]; bx=x; by=y-1; }
	if (y<MAPY/10-1 && mem->sector[x][y+1]>best) { best=mem->sector[x][y+1]; bx=x; by=y+1; }

	do_char_log(cn,1,"x=%d, y=%d, best=%d, bx=%d, by=%d\n",x,y,best,bx,by);

	x=bx*10+RANDOM(7)+2;
	y=by*10+RANDOM(7)+2;

    if (x==bx && y==by) return;

	ch[cn].goto_x=x;
	ch[cn].goto_y=y;

	mem->sector[bx][by]-=5;
}
#endif

void ccp_driver(int cn)
{
#ifdef REAL_CCP
    struct ccp_mem *mem=get_ccp_mem(cn);
    int co;

    if (ch[cn].a_hp<ch[cn].hp[5]*500 && !ccp_at_recall_point(cn) && npc_try_spell(cn,cn,SK_RECALL)) return;
	
	if (ch[cn].a_mana>1000*30) { // always keep 15 mana for recall
		if (ch[cn].a_hp<ch[cn].hp[5]*750 && npc_try_spell(cn,cn,SK_HEAL)) return;
        if (npc_try_spell(cn,cn,SK_PROTECT)) return;
		if (npc_try_spell(cn,cn,SK_ENHANCE)) return;
		if (npc_try_spell(cn,cn,SK_BLESS)) return;
		if (npc_try_spell(cn,cn,SK_MSHIELD)) return;		
	}

	// dont fight enemy if no longer visible
    if (mem->fighting && !do_char_can_see(cn,mem->fighting)) mem->fighting=0;

    if ((co=mem->fighting)) {
		ch[cn].attack_cn=co;
		ch[cn].goto_x=ch[cn].goto_y=0;
		if (ch[cn].a_mana>1000*30) { // always keep 15 mana for recall
			if (mem->enemy_strength>1) {			
				if (npc_try_spell(cn,co,SK_CURSE)) return;	// keep him cursed
				if (npc_try_spell(cn,co,SK_STUN)) return;	// and stunned all the time
				if (mem->enemy_strength>2) {			
					if (npc_try_spell(cn,co,SK_BLAST)) return;	// then blast
				}
			}
		}
	}
	if (ch[cn].attack_cn || ch[cn].goto_x || ch[cn].misc_action) return; // we're busy

	if (ch[cn].skill[SK_MEDIT][0] && ch[cn].a_mana<ch[cn].mana[5]*900) return;	// wait for mana regeneration
	if (ch[cn].a_hp<ch[cn].hp[5]*900) return;	// wait for hp regeneration

	ccp_sector_score(cn,-1);	// we get bored easily...

    ccp_goto_sector(cn);
#endif
}
