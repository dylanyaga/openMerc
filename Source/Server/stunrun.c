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

// data[0] to data[19] are used to remember whom we saw last turn
// data[20] contains the last one who attacked us
// data[21] timer for above
// data[22] current state
// data[23] timer for staying at home (state 0)
// data[24] timer for moving slowly

// data[29] to data[41] are safe to use:

// data[30] to data[34] contain any members of the same group who got attacked last turn
// data[35] to data[39] timer for above
// data[40] number of runs we survived

// data[50] to data[70] are safe to use:
// data[50] to data[69]: timer for seen array


int npc_stunrun_add_seen(int cn,int co)
{
	int n;

	for (n=0; n<20; n++) 
		if (ch[cn].data[n]==co) {
			ch[cn].data[n+50]=globs->ticker;
			return 1;
		}

	for (n=0; n<20; n++)
		if (!ch[cn].data[n]) break;
	if (n<20) {
		ch[cn].data[n]=co;
		ch[cn].data[n+50]=globs->ticker;
	}

	return 1;
}


int npc_stunrun_gotattack(int cn,int co)
{
	npc_stunrun_add_seen(cn,co);

	ch[cn].data[20]=co;

	return 1;
}

void npc_stunrun_add_fight(int cn,int co)
{
	int n;

	for (n=30; n<35; n++) 
		if (ch[cn].data[n]==co) {
			ch[cn].data[n+5]=globs->ticker;
			return;
		}

	for (n=30; n<35; n++)
		if (!ch[cn].data[n]) break;
	if (n<35) {
		ch[cn].data[n]=co;
		ch[cn].data[n+5]=globs->ticker;
	}
}

int npc_stunrun_seeattack(int cn,int cc,int co)
{
	if (do_char_can_see(cn,co)) {
		npc_stunrun_add_seen(cn,co);
		npc_stunrun_add_fight(cn,co);
	}
	if (do_char_can_see(cn,cc)) {
		npc_stunrun_add_seen(cn,cc);
		npc_stunrun_add_fight(cn,cc);
	}

	return 1;
}

int npc_stunrun_see(int cn,int co)
{
	if (!do_char_can_see(cn,co)) return 1;	// processed it: we cannot see him, so ignore him

	npc_stunrun_add_seen(cn,co);

	return 1;
}

int npc_stunrun_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4)
{
	switch(type) {
		case	NT_GOTHIT:	return npc_stunrun_gotattack(cn,dat1);
		case	NT_GOTMISS:	return npc_stunrun_gotattack(cn,dat1);
		case	NT_DIDHIT:	return 0;
		case	NT_DIDMISS:	return 0;
		case	NT_DIDKILL:	return 0;
		case	NT_GOTEXP:	return 0;
		case	NT_SEEKILL:	return 0;
		case	NT_SEEHIT:	return npc_stunrun_seeattack(cn,dat1,dat2); 
		case	NT_SEEMISS:	return npc_stunrun_seeattack(cn,dat1,dat2); 
		case	NT_GIVE:	return 0;
		case	NT_SEE:		return npc_stunrun_see(cn,dat1);
		case	NT_DIED:	return 0;
		case	NT_SHOUT:	return 0;
		case	NT_HITME:	return 0;

		default:		xlog("Unknown NPC message for %d (%s): %d",
						cn,ch[cn].name,type);
					return 0;
	}
}

int npc_check_target(int x,int y) 
{
	int in,m;

	if (x<1 || x>=MAPX) return 0;
	if (y<1 || y>=MAPY) return 0;

	m=x+y*MAPX;

	if (((unsigned long)map[m].flags&(MF_MOVEBLOCK|MF_NOMONST)) || map[m].ch || map[m].to_ch ||
	    ((in=map[m].it) && (it[in].flags&IF_MOVEBLOCK) && it[in].driver!=2)) return 0;

	return 1;
}

int npc_is_stunned(int cn)
{
	int n,in;

	for (n=0; n<20; n++) 
		if ((in=ch[cn].spell[n]) && it[in].temp==SK_STUN) break;

	if (n<20) return 1;

	return 0;
}

int npc_is_blessed(int cn)
{
	int n,in;

	for (n=0; n<20; n++) 
		if ((in=ch[cn].spell[n]) && it[in].temp==SK_BLESS) break;

	if (n<20) return 1;

	return 0;
}

struct seen
{
	int co;
	int dist;
	int friend;
	int stun;
	int help;
};

int npc_stunrun_high(int cn)
{
	int n,co,maxseen=0,m,tmp,done=0,in;
	struct seen seen[30];
	int flee=0;				// should we flee?
	int help=0;				// should we help someone?
	int stun=0;				// should we stun someone?
	int up=0,down=0,left=0,right=0;		// directions to move in

	ch[cn].data[92]=TICKS*60;

	for (n=0; n<20; n++) {
		if ((co=ch[cn].data[n]) && IS_SANECHAR(co)) {
			if (ch[co].data[42]==ch[cn].data[42]) {
				seen[maxseen].co=co;
				seen[maxseen].dist=npc_dist(cn,co);
				seen[maxseen].friend=1;
				seen[maxseen].stun=0;
				seen[maxseen].help=(ch[co].a_hp<ch[co].hp[5]*400);
				help=max(seen[maxseen].help,help);
				maxseen++;
			} else {
				seen[maxseen].co=co;
				seen[maxseen].dist=npc_dist(cn,co);
				seen[maxseen].friend=0;
				if (!npc_is_stunned(co)) seen[maxseen].stun=(ch[cn].skill[SK_STUN][5]*12>ch[co].skill[SK_RESIST][5]*10);
				else seen[maxseen].stun=0;
				stun=max(seen[maxseen].stun,stun);
				seen[maxseen].help=0;
				if (seen[maxseen].dist<6) flee++;	// we dont like infights, try to stay away from enemies
				if (seen[maxseen].dist<4) flee++;
				if (seen[maxseen].dist<2) {
					flee+=2;
					if (seen[maxseen].stun) {
						seen[maxseen].stun+=5;
						stun=max(seen[maxseen].stun,stun);
					}
				}
				maxseen++;
			}
		}
	}

	for (n=30; n<35; n++) {
		if ((co=ch[cn].data[n]) && IS_SANECHAR(co)) {
			for (m=0; m<maxseen; m++) {
				if (seen[m].co==co) {
					if (ch[co].data[42]==ch[cn].data[42]) {
						seen[m].help++;
						help=max(seen[m].help,help);
					} else {
						if (seen[m].stun) seen[m].stun+=2;
						stun=max(seen[m].stun,stun);
					}
					break;
				}
			}
		}
	}

	if ((co=ch[cn].data[20]) && IS_SANECHAR(co)) {
		flee+=5; // we dont like infights, try to flee if attacked
		for (m=0; m<maxseen; m++) {
			if (seen[m].co==co) {
				if (seen[m].stun) seen[m].stun+=5; else flee+=2;
				stun=max(seen[m].stun,stun);
				break;
			}
		}
		if (m==maxseen) {
			seen[maxseen].co=co;
			seen[maxseen].dist=npc_dist(cn,co);
			seen[maxseen].friend=0;
			seen[maxseen].stun=(ch[cn].skill[SK_STUN][5]*12>ch[co].skill[SK_RESIST][5]*10);
			if (seen[maxseen].stun) seen[maxseen].stun+=5; else flee+=2;
			stun=max(seen[m].stun,stun);
			seen[maxseen].help=0;
			maxseen++;
		}
	}

	if (ch[cn].a_mana<ch[cn].mana[5]*125) {
		stun-=3;
		help-=3;
		flee++;
	}

//	do_sayx(cn,"-- flee=%d, help=%d, stun=%d ---------",flee,help,stun);
	/* for (n=0; n<maxseen; n++) {
		do_sayx(cn,"%s: dist=%d, friend=%d, stun=%d, help=%d",
			ch[seen[n].co].reference,
			seen[n].dist,
			seen[n].friend,
			seen[n].stun,
			seen[n].help);
	} */

	// reset former orders
	ch[cn].use_nr=0;
	ch[cn].skill_nr=0;
	ch[cn].attack_cn=0;
	ch[cn].goto_x=0;
	ch[cn].goto_y=0;
	ch[cn].misc_action=0;
	ch[cn].cerrno=0;

	if (ch[cn].a_hp<ch[cn].hp[5]*666) flee+=5;

	if (!done && ch[cn].a_hp<ch[cn].hp[5]*666) done=npc_try_spell(cn,cn,SK_HEAL);

	if (ch[cn].a_end>15000) ch[cn].mode=1;
	else ch[cn].mode=0;

	if (!done && flee>1 && flee>=help && flee>=stun) {
		if (ch[cn].a_end>15000) ch[cn].mode=2;
		else ch[cn].mode=1;

		for (n=0; n<maxseen; n++) {
			if (!seen[n].friend) {
				if (seen[n].dist<6) tmp=-2000;
				else tmp=-1000;
			} else tmp=150;

			co=seen[n].co;
			if (ch[co].x>ch[cn].x) right+=tmp/(ch[co].x-ch[cn].x);
			if (ch[co].x<ch[cn].x) left+=tmp/(ch[cn].x-ch[co].x);
			if (ch[co].y>ch[cn].y) down+=tmp/(ch[co].y-ch[cn].y);
			if (ch[co].y<ch[cn].y) up+=tmp/(ch[cn].y-ch[co].y);
		}

		// check if up is free space
		for (n=1; n<5; n++) {
			if (!npc_check_target(ch[cn].x,ch[cn].y-n)) {
				up-=20;
				if (!npc_check_target(ch[cn].x+1,ch[cn].y-n)) {
					up-=20;
					if (!npc_check_target(ch[cn].x-1,ch[cn].y-n)) {
						up-=10000;
						break;
					}
				}
			}
		}

		// check if down is free space
		for (n=1; n<5; n++) {
			if (!npc_check_target(ch[cn].x,ch[cn].y+n)) {
				down-=20;
				if (!npc_check_target(ch[cn].x+1,ch[cn].y+n)) {
					down-=20;
					if (!npc_check_target(ch[cn].x-1,ch[cn].y+n)) {
						down-=10000;
						break;
					}
				}
			}
		}

		// check if left is free space
		for (n=1; n<5; n++) {
			if (!npc_check_target(ch[cn].x-n,ch[cn].y)) {
				left-=20;
				if (!npc_check_target(ch[cn].x-n,ch[cn].y+1)) {
					left-=20;
					if (!npc_check_target(ch[cn].x-n,ch[cn].y-n)) {
						left-=10000;
						break;
					}
				}
			}
		}

		// check if right is free space
		for (n=1; n<5; n++) {
			if (!npc_check_target(ch[cn].x+n,ch[cn].y)) {
				right-=20;
				if (!npc_check_target(ch[cn].x+n,ch[cn].y+1)) {
					right-=20;
					if (!npc_check_target(ch[cn].x+n,ch[cn].y-n)) {
						right-=10000;
						break;
					}
				}
			}
		}

		if (ch[cn].dir==DX_UP) up+=20;
		if (ch[cn].dir==DX_DOWN) down+=20;
		if (ch[cn].dir==DX_LEFT) left+=20;
		if (ch[cn].dir==DX_RIGHT) right+=20;

		//do_sayx(cn,"up=%d, down=%d, left=%d, right=%d",up,down,left,right);

		if (!done && up>=down && up>=left && up>=right) {
			//do_sayx(cn,"Would flee up.");
			if (npc_check_target(ch[cn].x,ch[cn].y-1)) {
				ch[cn].goto_x=ch[cn].x;
				ch[cn].goto_y=ch[cn].y-1;
				done=1;
			} else if (npc_check_target(ch[cn].x+1,ch[cn].y-1)) {
				ch[cn].goto_x=ch[cn].x+1;
				ch[cn].goto_y=ch[cn].y-1;
				done=1;
			} else if (npc_check_target(ch[cn].x-1,ch[cn].y-1)) {
				ch[cn].goto_x=ch[cn].x-1;
				ch[cn].goto_y=ch[cn].y-1;
				done=1;
			}
		} 

		if (!done && down>=up && down>=left && down>=right) {
			//do_sayx(cn,"Would flee down.");
			if (npc_check_target(ch[cn].x,ch[cn].y+1)) {
				ch[cn].goto_x=ch[cn].x;
				ch[cn].goto_y=ch[cn].y+1;
				done=1;
			} else if (npc_check_target(ch[cn].x+1,ch[cn].y+1)) {
				ch[cn].goto_x=ch[cn].x+1;
				ch[cn].goto_y=ch[cn].y+1;
				done=1;
			} else if (npc_check_target(ch[cn].x-1,ch[cn].y+1)) {
				ch[cn].goto_x=ch[cn].x-1;
				ch[cn].goto_y=ch[cn].y+1;
				done=1;
			}
		} 

		if (!done && left>=up && left>=down && left>=right) {
			//do_sayx(cn,"Would flee left.");
			if (npc_check_target(ch[cn].x-1,ch[cn].y)) {
				ch[cn].goto_x=ch[cn].x-1;
				ch[cn].goto_y=ch[cn].y;
				done=1;
			} else if (npc_check_target(ch[cn].x-1,ch[cn].y+1)) {
				ch[cn].goto_x=ch[cn].x-1;
				ch[cn].goto_y=ch[cn].y+1;
				done=1;
			} else if (npc_check_target(ch[cn].x-1,ch[cn].y-1)) {
				ch[cn].goto_x=ch[cn].x-1;
				ch[cn].goto_y=ch[cn].y-1;
				done=1;
			}
		} 

		if (!done && right>=up && right>=down && right>=left) {
			//do_sayx(cn,"Would flee right.");
			if (npc_check_target(ch[cn].x+1,ch[cn].y)) {
				ch[cn].goto_x=ch[cn].x+1;
				ch[cn].goto_y=ch[cn].y;
				done=1;
			} else if (npc_check_target(ch[cn].x+1,ch[cn].y+1)) {
				ch[cn].goto_x=ch[cn].x+1;
				ch[cn].goto_y=ch[cn].y+1;
				done=1;
			} else if (npc_check_target(ch[cn].x+1,ch[cn].y-1)) {
				ch[cn].goto_x=ch[cn].x+1;
				ch[cn].goto_y=ch[cn].y-1;
				done=1;
			}
		} 

		if (!done) {
			//do_sayx(cn,"Would Panic!");
			if ((co=ch[cn].data[20])) {
				ch[cn].attack_cn=co;
				npc_try_spell(cn,co,SK_STUN);
				done=1;
			}
		}
	}

	if (!done) done=npc_try_spell(cn,cn,SK_BLESS);
	if (!done) done=npc_try_spell(cn,cn,SK_MSHIELD);
	if (!done) done=npc_try_spell(cn,cn,SK_PROTECT);
	if (!done) done=npc_try_spell(cn,cn,SK_ENHANCE);


	if (!done && stun>1 && stun>=help) {
		for (m=n=tmp=0; n<maxseen; n++) {
			if (seen[n].stun>tmp || (seen[n].stun && seen[n].stun==tmp && seen[n].dist<seen[m].dist)) {
				tmp=seen[n].stun;
				m=n;
			}
		}
		if (tmp>0) {
			done=npc_try_spell(cn,seen[m].co,SK_STUN);
			if (!done) done=npc_try_spell(cn,seen[m].co,SK_CURSE);
			ch[cn].data[24]=globs->ticker;
		}
	} 

	if (!done && help>0) {
		for (m=n=tmp=0; n<maxseen; n++) {
			if (seen[n].help>tmp || (seen[n].help && seen[n].help==tmp && seen[n].dist<seen[m].dist)) {
				if (!npc_is_blessed(seen[n].co) || ch[seen[n].co].a_hp<ch[seen[n].co].hp[5]*400) {
					tmp=seen[n].help;
					m=n;
				}
			}
		}
		if (tmp>0) {
			if (ch[seen[m].co].a_hp<ch[seen[m].co].hp[5]*400) done=npc_try_spell(cn,seen[m].co,SK_HEAL);
			if (!done) done=npc_try_spell(cn,seen[m].co,SK_BLESS);
			if (!done) done=npc_try_spell(cn,seen[m].co,SK_PROTECT);
			if (!done) done=npc_try_spell(cn,seen[m].co,SK_ENHANCE);
			ch[cn].data[24]=globs->ticker;
		}
	}



	if (!done) {


		if (ch[cn].data[22]==0) {	// staying at home
			if ((in=ch[cn].citem)) {
				ch[cn].citem=0;
				it[in].used=USE_EMPTY;
			}
			if (ch[cn].data[23]==0) ch[cn].data[23]=globs->ticker;	// init
			if (ch[cn].data[23]+TICKS*60*60<globs->ticker) {
				int x,y;

				for (y=322,tmp=0; y<=332 && !tmp; y++) {
					for (x=212; x<=232 && !tmp; x++) {
						if ((co=map[x+y*MAPX].ch) && ch[co].data[42]!=ch[cn].data[42]) tmp=1;
					}
				}
				if (!tmp) ch[cn].data[22]=1; // set state for moving towards entry
				ch[cn].data[23]=globs->ticker;
			}
		}
		if (ch[cn].data[22]==1 && globs->ticker>ch[cn].data[24]+TICKS*10) {	// moving towards entry
			if (!ch[cn].citem) {
				in=god_create_item(718);
				ch[cn].citem=in;
				it[in].carried=cn;
			}
			if (abs(ch[cn].x-264)+abs(ch[cn].y-317)<20) { ch[cn].data[22]=2; ch[cn].data[23]=globs->ticker; }
			else {
				if (npc_check_target(264,317)) {
					ch[cn].goto_x=264;
					ch[cn].goto_y=317;
				} else if (npc_check_target(265,318)) {
					ch[cn].goto_x=265;
					ch[cn].goto_y=318;
				}
				if (ch[cn].x>232) {
					ch[cn].data[24]=globs->ticker;
				} else ch[cn].data[24]=0;
			}
		}
		if (ch[cn].data[22]==2) {	// moving towards home
			if (abs(ch[cn].x-217)+abs(ch[cn].y-349)<3) {
				ch[cn].data[22]=0;
				ch[cn].data[23]=globs->ticker;
				ch[cn].data[40]++;
			} else {
				ch[cn].goto_x=217;
				ch[cn].goto_y=349;
			}
		}
	}

//	do_sayx(cn,"state=%d",ch[cn].data[22]);

	for (n=0; n<20; n++) 
		if (ch[cn].data[n+50]+TICKS*2<globs->ticker) ch[cn].data[n]=0;	// erase all chars we saw
	for (n=30; n<35; n++) 
		if (ch[cn].data[n+5]+TICKS*2<globs->ticker) ch[cn].data[n]=0; 	// erase all fellows that got hit
	if (ch[cn].data[21]+TICKS*2<globs->ticker) ch[cn].data[20]=0;		// forget who hit us

	return 0;
}

void npc_stunrun_low(int cn)
{
	;
}
