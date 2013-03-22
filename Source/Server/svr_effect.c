/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "server.h"

int can_drop(int m)
{
	if (map[m].ch || map[m].to_ch || map[m].it || (map[m].flags&MF_MOVEBLOCK) || (map[m].flags&MF_DEATHTRAP) || map[m].fsprite) return 0;
	return 1;
}

int is_beam(int in)
{
	if (!in) return 0;
	if (it[in].temp!=453) return 0;
	if (!it[in].active) return 0;

	return 1;
}


void effect_tick(void)
{
	int n,cnt=0,in,m,co,fn,cn,in2,flag,z;

	for (n=1; n<MAXEFFECT; n++) {
		if (fx[n].used==USE_EMPTY) continue;
		cnt++;
		if (fx[n].used!=USE_ACTIVE) continue;

		if (fx[n].type==1) {	// remove injury flag from map
			fx[n].duration--;
			if (fx[n].duration==0) {
				fx[n].used=USE_EMPTY;
				map[fx[n].data[0]+fx[n].data[1]*MAPX].flags&=~(MF_GFX_INJURED|MF_GFX_INJURED1|MF_GFX_INJURED2);
			}
		}

		if (fx[n].type==2) {	// timer for character respawn
			if (fx[n].duration) fx[n].duration--;
			if (fx[n].duration==0 && plr_check_target(fx[n].data[0]+fx[n].data[1]*MAPX)) {
				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags|=MF_MOVEBLOCK;
				fx[n].type=8;
			}
		}

		if (fx[n].type==3) {	// death mist
			fx[n].duration++;
			if (fx[n].duration==19) {

				fx[n].used=0;

				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags&=~MF_GFX_DEATH;
			} else {
				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags&=~MF_GFX_DEATH;
				map[m].flags|=((unsigned long long)fx[n].duration)<<40;

				if (fx[n].duration==9) {
					plr_map_remove(fx[n].data[2]);

					if (can_drop(m)) ;
					else if (can_drop(m+1)) m+=1;
					else if (can_drop(m-1)) m+=-1;
					else if (can_drop(m+MAPX)) m+=MAPX;
					else if (can_drop(m-MAPX)) m+=-MAPX;
					else if (can_drop(m+1+MAPX)) m+=1+MAPX;
					else if (can_drop(m+1-MAPX)) m+=1-MAPX;
					else if (can_drop(m-1+MAPX)) m+=-1+MAPX;
					else if (can_drop(m-1-MAPX)) m+=-1-MAPX;
					else if (can_drop(m+2)) m+=2;
					else if (can_drop(m-2)) m+=-2;
					else if (can_drop(m+2*MAPX)) m+=2*MAPX;
					else if (can_drop(m-2*MAPX)) m+=-2*MAPX;
					else if (can_drop(m+2+MAPX)) m+=2+MAPX;
					else if (can_drop(m+2-MAPX)) m+=2-MAPX;
					else if (can_drop(m-2+MAPX)) m+=-2+MAPX;
					else if (can_drop(m-2-MAPX)) m+=-2-MAPX;
					else if (can_drop(m+1+2*MAPX)) m+=1+2*MAPX;
					else if (can_drop(m+1-2*MAPX)) m+=1-2*MAPX;
					else if (can_drop(m-1+2*MAPX)) m+=-1+2*MAPX;
					else if (can_drop(m-1-2*MAPX)) m+=-1-2*MAPX;
					else if (can_drop(m+2+2*MAPX)) m+=2+2*MAPX;
					else if (can_drop(m+2-2*MAPX)) m+=2-2*MAPX;
					else if (can_drop(m-2+2*MAPX)) m+=-2+2*MAPX;
					else if (can_drop(m-2-2*MAPX)) m+=-2-2*MAPX;
					else {
						int temp;

						co=fx[n].data[2];
						temp=ch[co].temp;
						
						chlog(co,"could not drop grave");

						god_destroy_items(co);
						ch[co].used=USE_EMPTY;
						
						if (ch[co].flags&CF_RESPAWN)
							fx_add_effect(2,TICKS*60*5+RANDOM(TICKS*60*10),ch_temp[temp].x,ch_temp[temp].y,temp);
						m=0;
					}

					if (m) {
						co=fx[n].data[2];

						flag=0;
						for (z=0; z<40 && !flag; z++) {
							if (ch[co].item[z]) { flag=1; break; }
						}
						for (z=0; z<20 && !flag; z++) {
							if (ch[co].worn[z]) { flag=1; break; }
						}
						if (ch[co].citem) flag=1;
						if (ch[co].gold) flag=1;
						
                                                if (flag) {
							map[m].flags|=MF_MOVEBLOCK;
							fn=fx_add_effect(4,0,m%MAPX,m/MAPX,fx[n].data[2]);
							fx[fn].data[3]=fx[n].data[3];
						} else {
							int temp;

							temp=ch[co].temp;

							god_destroy_items(co);
							ch[co].used=USE_EMPTY;

							if (temp && (ch[co].flags&CF_RESPAWN)) {
                                                                if (temp==189 || temp==561) {
									fx_add_effect(2,TICKS*60*20+RANDOM(TICKS*60*5),ch_temp[temp].x,ch_temp[temp].y,temp);									
                                                                } else {
									fx_add_effect(2,TICKS*60*4+RANDOM(TICKS*60*1),ch_temp[temp].x,ch_temp[temp].y,temp);
								}
								xlog("respawn %d (%s): YES",co,ch[co].name);
                                                        } else xlog("respawn %d (%s): NO",co,ch[co].name);
						}
					}
				}
			}
		}

		if (fx[n].type==4) {	// tomb stone
			fx[n].duration++;
			if (fx[n].duration==29) {

				fx[n].used=USE_EMPTY;
				
				co=fx[n].data[2];
				
				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags&=~MF_GFX_TOMB;
				map[m].flags&=~MF_MOVEBLOCK;								

				in=god_create_item(170);				
				it[in].data[0]=co;
				if (ch[co].data[99]) it[in].max_age[0]*=4;

				sprintf(it[in].description,"Here rests %s, killed by %s on the %d%s%s%s%s day of the Year %d.",
					ch[co].reference,
					fx[n].data[3] ? ch[fx[n].data[3]].reference : "unknown causes",
					globs->mdday,
					(globs->mdday==1 ? "st" : ""),
					(globs->mdday==2 ? "nd" : ""),
					(globs->mdday==3 ? "rd" : ""),
					(globs->mdday>3 ? "th" : ""),
					globs->mdyear);
				god_drop_item(in,fx[n].data[0],fx[n].data[1]);
				ch[co].x=it[in].x;
				ch[co].y=it[in].y;
				
				chlog(co,"grave done");
			} else {
				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags&=~MF_GFX_TOMB;
				map[m].flags|=((unsigned long long)fx[n].duration)<<35;
			}
		}

		if (fx[n].type==5) {	// evil magic
			fx[n].duration++;
			m=fx[n].data[0]+fx[n].data[1]*MAPX;
			if (fx[n].duration==8) {

				fx[n].used=USE_EMPTY;
				map[m].flags&=~MF_GFX_EMAGIC;
			} else {
				map[m].flags&=~MF_GFX_EMAGIC;
				map[m].flags|=((unsigned long long)fx[n].duration)<<45;
			}
		}

		if (fx[n].type==6) {	// good magic
			fx[n].duration++;
			m=fx[n].data[0]+fx[n].data[1]*MAPX;
			if (fx[n].duration==8) {

				fx[n].used=USE_EMPTY;
				map[m].flags&=~MF_GFX_GMAGIC;
			} else {
				map[m].flags&=~MF_GFX_GMAGIC;
				map[m].flags|=((unsigned long long)fx[n].duration)<<48;
			}
		}

		if (fx[n].type==7) {	// caster magic
			fx[n].duration++;
			m=fx[n].data[0]+fx[n].data[1]*MAPX;
			if (fx[n].duration==8) {
				fx[n].used=USE_EMPTY;
				map[m].flags&=~MF_GFX_CMAGIC;
			} else {
				map[m].flags&=~MF_GFX_CMAGIC;
				map[m].flags|=((unsigned long long)fx[n].duration)<<51;
			}
		}

		if (fx[n].type==8) {	// repawn mist
			fx[n].duration++;
			if (fx[n].duration==19) {
				fx[n].used=0;
				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags&=~MF_GFX_DEATH;
			} else {
				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags&=~MF_GFX_DEATH;
				map[m].flags|=((unsigned long long)fx[n].duration)<<40;
				if (fx[n].duration==9) {
					m=fx[n].data[0]+fx[n].data[1]*MAPX;
					map[m].flags&=~MF_MOVEBLOCK;
					if (!pop_create_char(fx[n].data[2],1) && (ch_temp[fx[n].data[2]].flags&CF_RESPAWN)) {
						fx[n].type=2;
						fx[n].duration=TICKS*60*5;	// try again every 5 minutes
						map[m].flags&=~MF_GFX_DEATH;
					}
				}
			}
		}
		if (fx[n].type==9) {	// controlled item animation with optional monster creation
			fx[n].duration--;
			in=fx[n].data[0];
			if (!(fx[n].duration&1))
				it[in].status[1]++;

			if (fx[n].duration==0) {
				map[it[in].x+it[in].y*MAPX].it=0;
				if (fx[n].data[1]) {
					cn=pop_create_char(fx[n].data[1],0);
					god_drop_char(cn,it[in].x,it[in].y);
					ch[cn].dir=DX_RIGHTUP;
					plr_reset_status(cn);
				}
				fx[n].used=USE_EMPTY;
				it[in].used=USE_EMPTY;
			}
		}
		if (fx[n].type==10) { // respawn object
			if (fx[n].duration) fx[n].duration--;
			else {
				m=fx[n].data[0]+fx[n].data[1]*MAPX;

				// check if object isnt allowed to respawn (supporting beams for mine)
				if (is_beam(map[m].it) ||
				    is_beam(map[m-1].it) || is_beam(map[m+1].it) ||
				    is_beam(map[m-MAPX].it) || is_beam(map[m+MAPX].it) ||
				
				    is_beam(map[m-2].it) || is_beam(map[m+2].it) ||
				    is_beam(map[m-2*MAPX].it) || is_beam(map[m+2*MAPX].it) ||
				
				    is_beam(map[m-1+1*MAPX].it) || is_beam(map[m+1+1*MAPX].it) ||
				    is_beam(map[m-1-1*MAPX].it) || is_beam(map[m+1-1*MAPX].it) ||
				
				    is_beam(map[m-2+1*MAPX].it) || is_beam(map[m+2+1*MAPX].it) ||
				    is_beam(map[m-2-1*MAPX].it) || is_beam(map[m+2-1*MAPX].it) ||
				
				    is_beam(map[m-1+2*MAPX].it) || is_beam(map[m+1+2*MAPX].it) ||
				    is_beam(map[m-1-2*MAPX].it) || is_beam(map[m+1-2*MAPX].it) ||
				
				    is_beam(map[m-2+2*MAPX].it) || is_beam(map[m+2+2*MAPX].it) ||
				    is_beam(map[m-2-2*MAPX].it) || is_beam(map[m+2-2*MAPX].it)) {
					fx[n].duration=TICKS*60*15;
					continue;
				}
				in2=map[m].it;
				map[m].it=0;

				in=god_create_item(fx[n].data[2]);

				if (!god_drop_item(in,fx[n].data[0],fx[n].data[1])) {
					fx[n].duration=TICKS*60;
					it[in].used=USE_EMPTY;
					map[m].it=in2;
				} else {
					fx[n].used=USE_EMPTY;
					if (in2) it[in2].used=USE_EMPTY;
					reset_go(fx[n].data[0],fx[n].data[1]);
				}
			}
		}
		if (fx[n].type==11) {	// remove queued spell flags
			fx[n].duration--;
			if (fx[n].duration<1) {
				fx[n].used=USE_EMPTY;
				ch[fx[n].data[0]].data[96]&=~fx[n].data[1];
			}
		}
                if (fx[n].type==12) {	// death mist
			fx[n].duration++;
			if (fx[n].duration==19) {

				fx[n].used=0;

				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags&=~MF_GFX_DEATH;
			} else {

				m=fx[n].data[0]+fx[n].data[1]*MAPX;
				map[m].flags&=~MF_GFX_DEATH;
				map[m].flags|=((unsigned long long)fx[n].duration)<<40;
                        }

		}
	}
	globs->effect_cnt=cnt;
}

int fx_add_effect(int type,int duration,int d1,int d2,int d3)
{
	int n;

	for (n=1; n<MAXEFFECT; n++)
		if (fx[n].used==USE_EMPTY) break;
	if (n==MAXEFFECT) return 0;

	fx[n].used=USE_ACTIVE;

	fx[n].type=type;
	fx[n].duration=duration;

	fx[n].flags=0;

	fx[n].data[0]=d1;
	fx[n].data[1]=d2;
	fx[n].data[2]=d3;

	return n;
}
