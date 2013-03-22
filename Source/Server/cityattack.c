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

// NOTE: Even though we don't use the data[] array, we may only use it as it is used in driver.c, since a lot
//	 of other functions rely on the data in there being either absent or correct.

// data[0] to data[24] are safe to use:

// data[0] = state
// data[1] = failed movetos

// data[29] to data[41] are safe to use:

// data[50] to data[70] are safe to use:


int npc_cityattack_gotattack(int cn,int co)
{
	return 1;
}

int npc_cityattack_seeattack(int cn,int cc,int co)
{
	if (do_char_can_see(cn,co)) {
		;
	}
	if (do_char_can_see(cn,cc)) {
		;
	}

	return 1;
}

int npc_cityattack_see(int cn,int co)
{
	int cc;

	if (!do_char_can_see(cn,co)) return 1;	// processed it: we cannot see him, so ignore him

	if (ch[cn].data[42]!=ch[co].data[42]) {
		if (!(cc=ch[cn].attack_cn) || npc_dist(cn,co)<npc_dist(cn,cc)) {
			ch[cn].attack_cn=co;
			ch[cn].goto_x=0;
		}
	}

	return 1;
}

int npc_cityattack_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4)
{
	switch(type) {
		case	NT_GOTHIT:	return npc_cityattack_gotattack(cn,dat1);
		case	NT_GOTMISS:	return npc_cityattack_gotattack(cn,dat1);
		case	NT_DIDHIT:	return 0;
		case	NT_DIDMISS:	return 0;
		case	NT_DIDKILL:	return 0;
		case	NT_GOTEXP:	return 0;
		case	NT_SEEKILL:	return 0;
		case	NT_SEEHIT:	return npc_cityattack_seeattack(cn,dat1,dat2); 
		case	NT_SEEMISS:	return npc_cityattack_seeattack(cn,dat1,dat2); 
		case	NT_GIVE:	return 0;
		case	NT_SEE:		return npc_cityattack_see(cn,dat1);
		case	NT_DIED:	return 0;
		case	NT_SHOUT:	return 0;
		case	NT_HITME:	return 0;

		default:		xlog("Unknown NPC message for %d (%s): %d",
						cn,ch[cn].name,type);
					return 0;
	}
}

int npc_cityattack_high(int cn)
{
	int co;

	// heal if hurt
	if (ch[cn].a_hp<ch[cn].hp[5]*600) { 
		if (npc_try_spell(cn,cn,SK_HEAL)) return 1;
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
	if (ch[cn].attack_cn && ch[cn].a_end>10000) {
		if (ch[cn].mode!=2) {
			ch[cn].mode=2;
			do_update_char(cn);
		}
	} else if (ch[cn].a_end>10000) {
		if (ch[cn].mode!=1) {
			ch[cn].mode=1;
			do_update_char(cn);
		}
	} else if (ch[cn].mode!=0) {
		ch[cn].mode=0;
		do_update_char(cn);
	}

	// fight management
	if ((co=ch[cn].attack_cn)) { // we're fighting
		if (co && ch[cn].a_hp<ch[cn].hp[5]*600) { // we're losing
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

	return 0;
}

int npc_moveto(int cn,int x,int y)
{
	int dx,dy,try;

	if (abs(ch[cn].x-x)<3 && abs(ch[cn].y-y)<3) {
		ch[cn].data[1]=0;
		return 1;
	}

	if (ch[cn].data[1]==0 && npc_check_target(x,y)) {
		ch[cn].data[1]++;
		ch[cn].goto_x=x;
		ch[cn].goto_y=y;
		return 0;
	}

	for (dx=0,try=1; dx<3; dx++) {
		for (dy=0; dy<3; dy++,try++) {
			if (ch[cn].data[1]<try && npc_check_target(x+dx,y+dy)) {
				ch[cn].data[1]++;
				ch[cn].goto_x=x+dx;
				ch[cn].goto_y=y+dy;
				return 0;
			}
			if (ch[cn].data[1]<try && npc_check_target(x-dx,y+dy)) {
				ch[cn].data[1]++;
				ch[cn].goto_x=x-dx;
				ch[cn].goto_y=y+dy;
				return 0;
			}
			if (ch[cn].data[1]<try && npc_check_target(x+dx,y-dy)) {
				ch[cn].data[1]++;
				ch[cn].goto_x=x+dx;
				ch[cn].goto_y=y-dy;
				return 0;
			}
			if (ch[cn].data[1]<try && npc_check_target(x-dx,y-dy)) {
				ch[cn].data[1]++;
				ch[cn].goto_x=x-dx;
				ch[cn].goto_y=y-dy;
				return 0;
			}

		}
	}

	ch[cn].data[1]=0;

	return 0;
}

int npc_cityattack_wait(cn)
{
	if ((globs->mdtime%28800)<20) return 1;

	return 0;
}

void npc_cityattack_low(int cn)
{
	int ret=0;

	switch(ch[cn].data[0]) {
		case	0:	ret=npc_moveto(cn,456,356); break;
		case	1:	ret=npc_moveto(cn,447,356); break;
		case	2:	ret=npc_moveto(cn,447,362); break;
		case	3:	ret=npc_moveto(cn,474,362); break;
		case	4:	ret=npc_cityattack_wait(cn); break;
		case	5:	ret=npc_moveto(cn,486,362); break;
		case	6:	ret=npc_moveto(cn,509,362); break;
		case	7:	ret=npc_moveto(cn,526,362); break;
		case	8:	ret=npc_moveto(cn,531,386); break;
		case	9:	ret=npc_moveto(cn,534,403); break;
	}
	if (ret) ch[cn].data[0]++;
}
