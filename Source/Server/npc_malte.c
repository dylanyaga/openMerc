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

// data[0] = player CN who saved us
// data[1] = state

// data[29] to data[41] are safe to use:

// data[50] to data[70] are safe to use:


int npc_malte_gotattack(int cn,int co)
{
	int cc;

	if (ch[cn].data[42]!=ch[co].data[42]) {
		if (!(cc=ch[cn].attack_cn) || npc_dist(cn,co)<npc_dist(cn,cc)) {
			ch[cn].attack_cn=co;
			ch[cn].goto_x=0;
		}
	}

	return 1;
}

int npc_malte_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4)
{
	switch(type) {
		case	NT_GOTHIT:	return npc_malte_gotattack(cn,dat1);
		case	NT_GOTMISS:	return npc_malte_gotattack(cn,dat1);
		case	NT_DIDHIT:	return 0;
		case	NT_DIDMISS:	return 0;
		case	NT_DIDKILL:	return 0;
		case	NT_GOTEXP:	return 0;
		case	NT_SEEKILL:	return 0;
		case	NT_SEEHIT:	return 0;
		case	NT_SEEMISS:	return 0;
		case	NT_GIVE:	return 0;
		case	NT_SEE:		return 0;
		case	NT_DIED:	return 0;
		case	NT_SHOUT:	return 0;
		case	NT_HITME:	return 0;

		default:		xlog("Unknown NPC message for %d (%s): %d",
						cn,ch[cn].name,type);
					return 0;
	}
}

int npc_malte_high(int cn)
{
	return 0;
}

void npc_malte_low(int cn)
{
	int co,in;

	if (globs->ticker<ch[cn].data[2]) return;

	co=ch[cn].data[0];

	switch(ch[cn].data[1]) {
		case	0:	do_sayx(cn,"Thank you so much for saving me, %s!",ch[co].name);
				ch[cn].data[2]=globs->ticker+TICKS*8;
				ch[cn].data[1]++;
				ch[cn].misc_action=DR_TURN;
				ch[cn].misc_target1=DX_DOWN;
				break;
		case	1:	do_sayx(cn,"Before the monsters caught me, I discovered that you need a coin to open certain doors down here.");
				ch[cn].data[2]=globs->ticker+TICKS*8;
				ch[cn].data[1]++;
				break;
		case	2:	do_sayx(cn,"I found this part of the coin, and I heard that Damor in Aston has another one. Ask him for the 'Black Stronghold Coin'.");
				ch[cn].data[2]=globs->ticker+TICKS*5;
				ch[cn].data[1]++;
				in=ch[cn].citem=god_create_item(763);
				it[in].carried=cn;
				ch[cn].misc_action=DR_GIVE;
				ch[cn].misc_target1=co;
				break;
		case	3:	do_sayx(cn,"Shiva, the mage who creates all the monsters, has the third part of it.");
				ch[cn].data[2]=globs->ticker+TICKS*8;
				ch[cn].data[1]++;
				break;
		case	4:	do_sayx(cn,"I have no idea where the other parts are.");
				ch[cn].data[2]=globs->ticker+TICKS*8;
				ch[cn].data[1]++;
				break;
		case	5:	do_sayx(cn,"I will recall now. I have enough of this prison!");
				ch[cn].data[2]=globs->ticker+TICKS*6;
				ch[cn].data[1]++;
				fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);
				break;
		case	6:	do_sayx(cn,"Good luck my friend. And thank you for freeing me!");
				plr_map_remove(cn);
				god_destroy_items(cn);
				ch[cn].used=USE_EMPTY;
				break;
	}
}
