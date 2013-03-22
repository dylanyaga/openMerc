/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "driver.h"

void plr_map_remove(int cn)     // remove character from map
{
        int m,in;

        m=ch[cn].x+ch[cn].y*MAPX;

        map[m].ch=0;
        map[ch[cn].tox+ch[cn].toy*MAPX].to_ch=0;

        if (ch[cn].light) {
                do_add_light(ch[cn].x,ch[cn].y,-ch[cn].light);
        }

        if (!(ch[cn].flags&CF_BODY)) {
                if ((in=map[m].it)!=0 && (it[in].flags&IF_STEPACTION)) {
                        step_driver_remove(cn,in);
                }
        }
}

int char_wears_item(int cn,int tmp)
{
        int in,n;

        for (n=0; n<20; n++)
                if ((in=ch[cn].worn[n])!=0 && it[in].temp==tmp) return 1;

        return 0;
}

void plr_map_set(int cn)        // set character to map and remove target character
{
        int m,in,ret,x,y;

        m=ch[cn].x+ch[cn].y*MAPX;

        if (!(ch[cn].flags&CF_BODY)) {

                if ((in=map[m].it)!=0 && (it[in].flags&IF_STEPACTION)) {
                        ret=step_driver(cn,in);
                        if (ret==1) {
                                map[m].to_ch=0;

                                x=ch[cn].x+(ch[cn].x-ch[cn].frx);
                                y=ch[cn].y+(ch[cn].y-ch[cn].fry);
				
				if (!map[x+y*MAPX].ch) {
					ch[cn].x=x;
					ch[cn].y=y;

					map[ch[cn].x+ch[cn].y*MAPX].ch=cn;
	
					ch[cn].use_nr=0;
					ch[cn].skill_nr=0;
					ch[cn].attack_cn=0;
					ch[cn].goto_x=0;
					ch[cn].goto_y=0;
					ch[cn].misc_action=0;
	
					if (ch[cn].light) {
						do_add_light(ch[cn].x,ch[cn].y,ch[cn].light);
					}
					return;
				} else ret=-1;
                        }
			if (ret==-1) {
                                map[m].to_ch=0;

                                ch[cn].x=ch[cn].frx;
                                ch[cn].y=ch[cn].fry;
                                map[ch[cn].x+ch[cn].y*MAPX].ch=cn;

                                ch[cn].use_nr=0;
                                ch[cn].skill_nr=0;
                                ch[cn].attack_cn=0;
                                ch[cn].goto_x=0;
                                ch[cn].goto_y=0;
                                ch[cn].misc_action=0;

                                if (ch[cn].light) {
                                        do_add_light(ch[cn].x,ch[cn].y,ch[cn].light);
                                }
                                return;
                        }
                        /* CS, 991127: Support for step_teleport() */
                        if (ret==2) { // TELEPORT_SUCCESS
                                if (ch[cn].light) {
                                        do_add_light(ch[cn].x,ch[cn].y,ch[cn].light);
                                }
                                return;
                        }
                }

                if ((map[m].flags&MF_TAVERN) && (ch[cn].flags&(CF_PLAYER))) {
                        if (IS_BUILDING(cn)) {
                                god_build(cn,0);
                        }
                        ch[cn].tavern_x=ch[cn].x;
                        ch[cn].tavern_y=ch[cn].y;
                        chlog(cn,"Entered tavern");
                        plr_logout(cn,ch[cn].player,LO_TAVERN);
                        return;
                }

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
        }

        map[m].ch=cn;
        map[m].to_ch=0;

        if (!(ch[cn].flags&CF_BODY)) {

                if (ch[cn].light) {
                        do_add_light(ch[cn].x,ch[cn].y,ch[cn].light);
                }

                if (map[m].flags&MF_DEATHTRAP) {
                        do_char_log(cn,0,"You entered a Deathtrap. You are dead!\n");
                        chlog(cn,"entered a Deathtrap");
                        do_char_killed(0,cn);
                        return;
                }


        }
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
}

void plr_move_up(int cn)
{
        plr_map_remove(cn);
        ch[cn].frx=ch[cn].x;
        ch[cn].fry=ch[cn].y;
        ch[cn].y--;
        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y;
        plr_map_set(cn);
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_move_down(int cn)
{
        plr_map_remove(cn);
        ch[cn].frx=ch[cn].x;
        ch[cn].fry=ch[cn].y;
        ch[cn].y++;
        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y;
        plr_map_set(cn);
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_move_left(int cn)
{
        plr_map_remove(cn);
        ch[cn].frx=ch[cn].x;
        ch[cn].fry=ch[cn].y;
        ch[cn].x--;
        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y;
        plr_map_set(cn);
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_move_right(int cn)
{
        plr_map_remove(cn);
        ch[cn].frx=ch[cn].x;
        ch[cn].fry=ch[cn].y;
        ch[cn].x++;
        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y;
        plr_map_set(cn);
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_move_leftup(int cn)
{
        plr_map_remove(cn);
        ch[cn].frx=ch[cn].x;
        ch[cn].fry=ch[cn].y;
        ch[cn].x--;
        ch[cn].y--;
        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y;
        plr_map_set(cn);
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_move_leftdown(int cn)
{
        plr_map_remove(cn);
        ch[cn].frx=ch[cn].x;
        ch[cn].fry=ch[cn].y;
        ch[cn].x--;
        ch[cn].y++;
        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y;
        plr_map_set(cn);
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_move_rightup(int cn)
{
        plr_map_remove(cn);
        ch[cn].frx=ch[cn].x;
        ch[cn].fry=ch[cn].y;
        ch[cn].x++;
        ch[cn].y--;
        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y;
        plr_map_set(cn);
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_move_rightdown(int cn)
{
        plr_map_remove(cn);
        ch[cn].frx=ch[cn].x;
        ch[cn].fry=ch[cn].y;
        ch[cn].x++;
        ch[cn].y++;
        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y;
        plr_map_set(cn);
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_turn_up(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
        ch[cn].dir=DX_UP;
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_turn_leftup(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
        ch[cn].dir=DX_LEFTUP;
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_turn_leftdown(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
        ch[cn].dir=DX_LEFTDOWN;
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_turn_down(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
        ch[cn].dir=DX_DOWN;
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_turn_rightdown(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
        ch[cn].dir=DX_RIGHTDOWN;
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_turn_rightup(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
        ch[cn].dir=DX_RIGHTUP;
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_turn_left(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
        ch[cn].dir=DX_LEFT;
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_turn_right(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
        ch[cn].dir=DX_RIGHT;
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_attack(int cn,int surround)
{
        int co,x,y;

        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        x=ch[cn].x;
        y=ch[cn].y;

        switch(ch[cn].dir) {
                case DX_UP:     y--; break;
                case DX_DOWN:   y++; break;
                case DX_LEFT:   x--; break;
                case DX_RIGHT:  x++; break;
                default:        xlog("plr_attack (svr_act.c): unknown dir %d for char %d",ch[cn].dir,cn);
                                ch[cn].cerrno=ERR_FAILED;
                                return;
        }

        co=map[x+y*MAPX].ch;
        if (!co) co=map[x+y*MAPX].to_ch;
        if (!co) {
                co=ch[cn].attack_cn;
                if (ch[co].frx!=x || ch[co].fry!=y) co=0;
        }
        if (!co) {
                do_char_log(cn,2,"Your target moved away!\n");
                return;
        }

        if (ch[cn].attack_cn==co) do_attack(cn,co,surround);
}

void plr_give(int cn)
{
        int co,x,y;

        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        x=ch[cn].x;
        y=ch[cn].y;

        switch(ch[cn].dir) {
                case DX_UP:     y--; break;
                case DX_DOWN:   y++; break;
                case DX_LEFT:   x--; break;
                case DX_RIGHT:  x++; break;
                default:        xlog("plr_give (svr_act.c): Unknown dir %d for char %d",ch[cn].dir,cn);
                                ch[cn].cerrno=ERR_FAILED;
                                return;
        }

        co=map[x+y*MAPX].ch;
        if (!co) co=map[x+y*MAPX].to_ch;
        if (!co) {
                do_char_log(cn,2,"Your target moved away!\n");
                return;
        }

        do_give(cn,co);
}

void plr_pickup(int cn)
{
        int m,in,x,y,n;

        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        if (ch[cn].citem) { ch[cn].cerrno=ERR_FAILED; return; }

        if (ch[cn].dir==DX_UP && ch[cn].y>0) { m=ch[cn].x+ch[cn].y*MAPX-MAPX; x=ch[cn].x; y=ch[cn].y-1; }
        else if (ch[cn].dir==DX_DOWN && ch[cn].y<MAPY-1) { m=ch[cn].x+ch[cn].y*MAPX+MAPX; x=ch[cn].x; y=ch[cn].y+1; }
        else if (ch[cn].dir==DX_LEFT && ch[cn].x>0) { m=ch[cn].x+ch[cn].y*MAPX-1; x=ch[cn].x-1; y=ch[cn].y; }
        else if (ch[cn].dir==DX_RIGHT && ch[cn].x<MAPX-1) { m=ch[cn].x+ch[cn].y*MAPX+1; x=ch[cn].x+1; y=ch[cn].y; }
        else { ch[cn].cerrno=ERR_FAILED; return; }

        in=map[m].it;
        if (!in) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!(it[in].flags&IF_TAKE)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].cerrno=ERR_SUCCESS;
	do_update_char(cn);

        // support for money:
        if (it[in].flags&IF_MONEY) {
                ch[cn].gold+=it[in].value;

                do_char_log(cn,2,"You got %dG %dS\n",it[in].value/100,it[in].value%100);
                chlog(cn,"Took %dG %dS",it[in].value/100,it[in].value%100);

                map[m].it=0;

                it[in].used=USE_EMPTY;
                it[in].x=0;
                it[in].y=0;

                if (it[in].active) {
                        if (it[in].light[1])
                                do_add_light(x,y,-it[in].light[1]);
                } else {
                        if (it[in].light[0])
                                do_add_light(x,y,-it[in].light[0]);
                }
                return;
        }

        map[m].it=0;

        if (ch[cn].flags&(CF_PLAYER)) {
                for (n=0; n<40; n++) {
                        if (!ch[cn].item[n]) break;
                }
                if (n<40) ch[cn].item[n]=in;
                else ch[cn].citem=in;
                chlog(cn,"Took %s",it[in].name);
        } else ch[cn].citem=in;

        it[in].x=0;
        it[in].y=0;
        it[in].carried=cn;

        if (it[in].active) {
                if (it[in].light[1])
                        do_add_light(x,y,-it[in].light[1]);
        } else {
                if (it[in].light[0])
                        do_add_light(x,y,-it[in].light[0]);
        }
}

void plr_bow(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        do_char_log(cn,2,"You bow deeply.\n");
        do_area_log(cn,0,ch[cn].x,ch[cn].y,1,"%s bows deeply.\n",ch[cn].reference);
        chlog(cn,"Bows");
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_wave(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        do_char_log(cn,2,"You wave happily.\n");
        do_area_log(cn,0,ch[cn].x,ch[cn].y,1,"%s waves happily.\n",ch[cn].reference);
        chlog(cn,"Waves");
        ch[cn].cerrno=ERR_SUCCESS;
}

void plr_use(int cn)
{
        int m,in,x,y;

        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        if (ch[cn].dir==DX_UP && ch[cn].y>0) { m=ch[cn].x+ch[cn].y*MAPX-MAPX; x=ch[cn].x; y=ch[cn].y-1; }
        else if (ch[cn].dir==DX_DOWN && ch[cn].y<MAPY-1) { m=ch[cn].x+ch[cn].y*MAPX+MAPX; x=ch[cn].x; y=ch[cn].y+1; }
        else if (ch[cn].dir==DX_LEFT && ch[cn].x>0) { m=ch[cn].x+ch[cn].y*MAPX-1; x=ch[cn].x-1; y=ch[cn].y; }
        else if (ch[cn].dir==DX_RIGHT && ch[cn].x<MAPX-1) { m=ch[cn].x+ch[cn].y*MAPX+1; x=ch[cn].x+1; y=ch[cn].y; }
        else { ch[cn].cerrno=ERR_FAILED; return; }

        in=map[m].it;
        if (!in) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!(it[in].flags&(IF_USE|IF_USESPECIAL))) { ch[cn].cerrno=ERR_FAILED; return; }

        use_driver(cn,in,0);    // use_driver sets errno
}

void plr_skill(int cn)
{
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        skill_driver(cn,ch[cn].skill_target2);  // skill_driver sets errno
}

void plr_drop(int cn)
{
        int m,in,x,y,tmp,money=0,in2;

        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        in=ch[cn].citem;
        if (!in) return;

        if (ch[cn].dir==DX_UP && ch[cn].y>0) { m=ch[cn].x+ch[cn].y*MAPX-MAPX; x=ch[cn].x; y=ch[cn].y-1; }
        else if (ch[cn].dir==DX_DOWN && ch[cn].y<MAPY-1) { m=ch[cn].x+ch[cn].y*MAPX+MAPX; x=ch[cn].x; y=ch[cn].y+1; }
        else if (ch[cn].dir==DX_LEFT && ch[cn].x>0) { m=ch[cn].x+ch[cn].y*MAPX-1; x=ch[cn].x-1; y=ch[cn].y; }
        else if (ch[cn].dir==DX_RIGHT && ch[cn].x<MAPX-1) { m=ch[cn].x+ch[cn].y*MAPX+1; x=ch[cn].x+1; y=ch[cn].y; }
        else { ch[cn].cerrno=ERR_FAILED; return; }

        if ((in2=map[m].it)!=0 && (it[in2].flags&IF_STEPACTION)) {
                step_driver(cn,in2);
                ch[cn].cerrno=ERR_FAILED;
                return;
        }

        if (map[m].ch || map[m].to_ch || map[m].it || (map[m].flags&MF_MOVEBLOCK)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].citem=0;

        ch[cn].cerrno=ERR_SUCCESS;
	do_update_char(cn);

        if (in&0x80000000) {
                tmp=in&0x7FFFFFFF;
                in=god_create_item(1); // blank template
                if (!in) { ch[cn].cerrno=ERR_FAILED; return; }
                it[in].flags|=IF_TAKE|IF_LOOK|IF_MONEY;
                it[in].value=tmp;
                strcpy(it[in].reference, "some money");
                if (tmp>999999) {
                        strcpy(it[in].description, "A huge pile of gold coins");
                        it[in].sprite[0]=121;
                } else if (tmp>99999) {
                        strcpy(it[in].description, "A very large pile of gold coins");
                        it[in].sprite[0]=120;
                } else if (tmp>9999) {
                        strcpy(it[in].description, "A large pile of gold coins");
                        it[in].sprite[0]=41;
                } else if (tmp>999) {
                        strcpy(it[in].description, "A small pile of gold coins");
                        it[in].sprite[0]=40;
                } else if (tmp>99) {
                        strcpy(it[in].description, "Some gold coins");
                        it[in].sprite[0]=39;
                } else if (tmp>9) {
                        strcpy(it[in].description, "A pile of silver coins");
                        it[in].sprite[0]=38;
                } else if (tmp>2) {
                        strcpy(it[in].description, "A few silver coins");
                        it[in].sprite[0]=37;
                } else if (tmp==2) {
                        strcpy(it[in].description, "A couple of silver coins");
                        it[in].sprite[0]=37;
                } else if (tmp==1) {
                        strcpy(it[in].description, "A lonely silver coin");
                        it[in].sprite[0]=37;
                }
                money=1;

                chlog(cn,"Dropped %dG %dS",tmp/100,tmp%100);
        } else {
                if (!do_maygive(cn,0,in)) {
                        do_char_log(cn,0,"You are not allowed to do that!\n");
                        ch[cn].citem=in;
                        ch[cn].cerrno=ERR_FAILED;
                        return;
                }
                chlog(cn,"Dropped %s",it[in].name);
        }

        map[m].it=in;

        it[in].x=(short)x;
        it[in].y=(short)y;
        it[in].carried=0;

        if (it[in].active) {
                if (it[in].light[1])
                        do_add_light(x,y,it[in].light[1]);
        } else {
                if (it[in].light[0])
                        do_add_light(x,y,it[in].light[0]);
        }
}

void plr_misc(int cn)
{
        switch(ch[cn].status2) {
                case    0:      plr_attack(cn,0); break;
                case    1:      plr_pickup(cn); break;
                case    2:      plr_drop(cn); break;
                case    3:      plr_give(cn); break;
                case    4:      plr_use(cn); break;
                case    5:      plr_attack(cn,1); break;
                case    6:      plr_attack(cn,0); break;
                case    7:      plr_bow(cn); break;
                case    8:      plr_wave(cn); break;
                case    9:      plr_skill(cn); break;
                default:        xlog("plr_misc (svr_act.c): unknown status2 %d for char %d",ch[cn].status2,cn);
                                ch[cn].cerrno=ERR_FAILED;
                                break;
        }
}

int plr_check_target(int m)
{
        if (map[m].ch || map[m].to_ch) return 0;

        if (map[m].flags&MF_MOVEBLOCK) return 0;

        if (map[m].it && (it[map[m].it].flags&IF_MOVEBLOCK)) return 0;

        return 1;
}

int plr_set_target(int m,int cn)
{
        if (!plr_check_target(m)) return 0;

        map[m].to_ch=cn;

        return 1;
}


// -------------------
// interface functions
// -------------------

// Only these may be called from the outside!

void act_move_up(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].y<1) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].dir!=DX_UP) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!do_char_can_flee(cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!plr_set_target(ch[cn].x+ch[cn].y*MAPX-MAPX,cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].status=16;

        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y-1;
}

void act_move_down(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].y>=MAPY-2) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].dir!=DX_DOWN) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!do_char_can_flee(cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!plr_set_target(ch[cn].x+ch[cn].y*MAPX+MAPX,cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].status=24;

        ch[cn].tox=ch[cn].x;
        ch[cn].toy=ch[cn].y+1;
}

void act_move_left(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].x<1) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].dir!=DX_LEFT) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!do_char_can_flee(cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!plr_set_target(ch[cn].x+ch[cn].y*MAPX-1,cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].status=32;

        ch[cn].tox=ch[cn].x-1;
        ch[cn].toy=ch[cn].y;
}

void act_move_right(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].x>=MAPX-2) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].dir!=DX_RIGHT) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!do_char_can_flee(cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!plr_set_target(ch[cn].x+ch[cn].y*MAPX+1,cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].status=40;

        ch[cn].tox=ch[cn].x+1;
        ch[cn].toy=ch[cn].y;
}

void act_move_leftup(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].x<1) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].y<1) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].dir!=DX_LEFTUP) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!do_char_can_flee(cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!plr_check_target(ch[cn].x+ch[cn].y*MAPX-MAPX)) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!plr_check_target(ch[cn].x+ch[cn].y*MAPX-1)) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!plr_set_target(ch[cn].x+ch[cn].y*MAPX-MAPX-1,cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].status=48;

        ch[cn].tox=ch[cn].x-1;
        ch[cn].toy=ch[cn].y-1;
}

void act_move_leftdown(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].x<1) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].y>=MAPY-2) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].dir!=DX_LEFTDOWN) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!do_char_can_flee(cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!plr_check_target(ch[cn].x+ch[cn].y*MAPX+MAPX)) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!plr_check_target(ch[cn].x+ch[cn].y*MAPX-1)) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!plr_set_target(ch[cn].x+ch[cn].y*MAPX+MAPX-1,cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].status=60;

        ch[cn].tox=ch[cn].x-1;
        ch[cn].toy=ch[cn].y+1;
}

void act_move_rightup(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].x>=MAPX-2){ ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].y<1){ ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].dir!=DX_RIGHTUP) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!do_char_can_flee(cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!plr_check_target(ch[cn].x+ch[cn].y*MAPX-MAPX)) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!plr_check_target(ch[cn].x+ch[cn].y*MAPX+1)) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!plr_set_target(ch[cn].x+ch[cn].y*MAPX-MAPX+1,cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].status=72;

        ch[cn].tox=ch[cn].x+1;
        ch[cn].toy=ch[cn].y-1;
}

void act_move_rightdown(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].x>=MAPX-2) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].y>=MAPY-2) { ch[cn].cerrno=ERR_FAILED; return; }
        if (ch[cn].dir!=DX_RIGHTDOWN) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!do_char_can_flee(cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        if (!plr_check_target(ch[cn].x+ch[cn].y*MAPX+MAPX)) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!plr_check_target(ch[cn].x+ch[cn].y*MAPX+1)) { ch[cn].cerrno=ERR_FAILED; return; }
        if (!plr_set_target(ch[cn].x+ch[cn].y*MAPX+MAPX+1,cn)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].status=84;

        ch[cn].tox=ch[cn].x+1;
        ch[cn].toy=ch[cn].y+1;
}

void act_turn_up(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==DX_DOWN) act_turn_rightdown(cn);
        else if (ch[cn].dir==DX_LEFTDOWN) act_turn_left(cn);
        else if (ch[cn].dir==DX_RIGHTDOWN) act_turn_right(cn);
        else if (ch[cn].dir==DX_LEFT) act_turn_leftup(cn);
        else if (ch[cn].dir==DX_RIGHT) act_turn_rightup(cn);
        else if (ch[cn].dir==DX_LEFTUP) ch[cn].status=132;
        else ch[cn].status=148; // dir=DX_RIGHT and any possible insane value...
}

void act_turn_down(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==DX_UP) act_turn_leftup(cn);
        else if (ch[cn].dir==DX_LEFTUP) act_turn_left(cn);
        else if (ch[cn].dir==DX_RIGHTUP) act_turn_right(cn);
        else if (ch[cn].dir==DX_LEFT) act_turn_leftdown(cn);
        else if (ch[cn].dir==DX_RIGHT) act_turn_rightdown(cn);
        else if (ch[cn].dir==DX_LEFTDOWN) ch[cn].status=140;
        else ch[cn].status=156; // dir=DX_RIGHT and any possible insane value...
}

void act_turn_left(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==DX_RIGHT) act_turn_rightup(cn);
        else if (ch[cn].dir==DX_RIGHTUP) act_turn_up(cn);
        else if (ch[cn].dir==DX_RIGHTDOWN) act_turn_down(cn);
        else if (ch[cn].dir==DX_UP) act_turn_leftup(cn);
        else if (ch[cn].dir==DX_DOWN) act_turn_leftdown(cn);
        else if (ch[cn].dir==DX_LEFTUP) ch[cn].status=100;
        else ch[cn].status=116; // dir=DX_DOWN and any possible insane value...
}

void act_turn_right(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==DX_LEFT) act_turn_leftdown(cn);
        else if (ch[cn].dir==DX_LEFTUP) act_turn_up(cn);
        else if (ch[cn].dir==DX_LEFTDOWN) act_turn_down(cn);
        else if (ch[cn].dir==DX_UP) act_turn_rightup(cn);
        else if (ch[cn].dir==DX_DOWN) act_turn_rightdown(cn);
        else if (ch[cn].dir==DX_RIGHTUP) ch[cn].status=108;
        else ch[cn].status=124; // dir=DX_DOWN and any possible insane value...
}

void act_turn_leftup(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==DX_RIGHTDOWN) act_turn_down(cn);
        else if (ch[cn].dir==DX_DOWN) act_turn_leftdown(cn);
        else if (ch[cn].dir==DX_RIGHT) act_turn_rightup(cn);
        else if (ch[cn].dir==DX_RIGHTUP) act_turn_up(cn);
        else if (ch[cn].dir==DX_LEFTDOWN) act_turn_left(cn);
        else if (ch[cn].dir==DX_UP) ch[cn].status=96;
        else ch[cn].status=128; // dir=DX_LEFT and any possible insane value...
}

void act_turn_leftdown(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==DX_RIGHTUP) act_turn_up(cn);
        else if (ch[cn].dir==DX_UP) act_turn_leftup(cn);
        else if (ch[cn].dir==DX_RIGHT) act_turn_rightdown(cn);
        else if (ch[cn].dir==DX_RIGHTDOWN) act_turn_down(cn);
        else if (ch[cn].dir==DX_LEFTUP) act_turn_left(cn);
        else if (ch[cn].dir==DX_DOWN) ch[cn].status=112;
        else ch[cn].status=136; // dir=DX_LEFT and any possible insane value...
}

void act_turn_rightup(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==DX_LEFTDOWN) act_turn_down(cn);
        else if (ch[cn].dir==DX_DOWN) act_turn_rightdown(cn);
        else if (ch[cn].dir==DX_LEFT) act_turn_leftup(cn);
        else if (ch[cn].dir==DX_LEFTUP) act_turn_up(cn);
        else if (ch[cn].dir==DX_RIGHTDOWN) act_turn_right(cn);
        else if (ch[cn].dir==DX_UP) ch[cn].status=104;
        else ch[cn].status=144; // dir=DX_RIGHT and any possible insane value...
}

void act_turn_rightdown(int cn)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==DX_LEFTUP) act_turn_up(cn);
        else if (ch[cn].dir==DX_UP) act_turn_rightup(cn);
        else if (ch[cn].dir==DX_LEFT) act_turn_leftdown(cn);
        else if (ch[cn].dir==DX_LEFTDOWN) act_turn_down(cn);
        else if (ch[cn].dir==DX_RIGHTUP) act_turn_right(cn);
        else if (ch[cn].dir==DX_DOWN) ch[cn].status=120;
        else ch[cn].status=152; // dir=DX_LEFT and any possible insane value...
}

void act_turn(int cn,int dir)
{
        ch[cn].cerrno=ERR_NONE;

        if (ch[cn].dir==dir) { ch[cn].cerrno=ERR_SUCCESS; return; }

        switch(dir) {
                case DX_UP:             act_turn_up(cn); break;
                case DX_DOWN:           act_turn_down(cn); break;
                case DX_RIGHT:          act_turn_right(cn); break;
                case DX_LEFT:           act_turn_left(cn); break;
                case DX_LEFTUP:         act_turn_leftup(cn); break;
                case DX_LEFTDOWN:       act_turn_leftdown(cn); break;
                case DX_RIGHTUP:        act_turn_rightup(cn); break;
                case DX_RIGHTDOWN:      act_turn_rightdown(cn); break;
                default:        xlog("act_turn (svr_act.c): unknown dir %d for char %d",dir,cn);
                                ch[cn].cerrno=ERR_FAILED;
                                break;
        }
}

void act_attack(int cn)         // attack character in front of the character
{
        int v;

        ch[cn].cerrno=ERR_NONE;

        if (!(ch[cn].flags&CF_SIMPLE)) {
        	do {
                	v=RANDOM(3);
                } while (ch[cn].lastattack==v);
                ch[cn].lastattack=v;
                if (v) v+=4;
        } else v=0;

        switch(ch[cn].dir) {
                case    DX_UP:          if (ch[cn].y>0) { ch[cn].status=160; ch[cn].status2=v; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_DOWN:        if (ch[cn].y<MAPY-1) { ch[cn].status=168; ch[cn].status2=v; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_LEFT:        if (ch[cn].x>0) { ch[cn].status=176; ch[cn].status2=v; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_RIGHT:       if (ch[cn].y<MAPX-1) { ch[cn].status=184; ch[cn].status2=v; } else ch[cn].cerrno=ERR_FAILED; break;
                default:                ch[cn].cerrno=ERR_FAILED; break;
        }
}

void act_give(int cn)   // give current object to character in front of the character
{
        if ((ch[cn].flags&CF_SIMPLE)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].cerrno=ERR_NONE;

        switch(ch[cn].dir) {
                case    DX_UP:          if (ch[cn].y>0) { ch[cn].status=160; ch[cn].status2=3; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_DOWN:        if (ch[cn].y<MAPY-1) { ch[cn].status=168; ch[cn].status2=3; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_LEFT:        if (ch[cn].x>0) { ch[cn].status=176; ch[cn].status2=3; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_RIGHT:       if (ch[cn].y<MAPX-1) { ch[cn].status=184; ch[cn].status2=3; } else ch[cn].cerrno=ERR_FAILED; break;
                default:                ch[cn].cerrno=ERR_FAILED; break;
        }
}

void act_bow(int cn)
{
        if ((ch[cn].flags&CF_SIMPLE)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].cerrno=ERR_NONE;

        switch(ch[cn].dir) {
                case    DX_UP:          if (ch[cn].y>0) { ch[cn].status=160; ch[cn].status2=7; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_DOWN:        if (ch[cn].y<MAPY-1) { ch[cn].status=168; ch[cn].status2=7; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_LEFT:        if (ch[cn].x>0) { ch[cn].status=176; ch[cn].status2=7; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_RIGHT:       if (ch[cn].y<MAPX-1) { ch[cn].status=184; ch[cn].status2=7; } else ch[cn].cerrno=ERR_FAILED; break;
                default:                ch[cn].cerrno=ERR_FAILED; break;
        }
}

void act_wave(int cn)
{
        if ((ch[cn].flags&CF_SIMPLE)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].cerrno=ERR_NONE;

        switch(ch[cn].dir) {
                case    DX_UP:          if (ch[cn].y>0) { ch[cn].status=160; ch[cn].status2=8; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_DOWN:        if (ch[cn].y<MAPY-1) { ch[cn].status=168; ch[cn].status2=8; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_LEFT:        if (ch[cn].x>0) { ch[cn].status=176; ch[cn].status2=8; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_RIGHT:       if (ch[cn].y<MAPX-1) { ch[cn].status=184; ch[cn].status2=8; } else ch[cn].cerrno=ERR_FAILED; break;
                default:                ch[cn].cerrno=ERR_FAILED; break;
        }
}

void act_skill(int cn)
{
        if ((ch[cn].flags&CF_SIMPLE)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].cerrno=ERR_NONE;

        switch(ch[cn].dir) {
                case    DX_UP:          ch[cn].status=160; ch[cn].status2=9; return;
                case    DX_DOWN:        ch[cn].status=168; ch[cn].status2=9; return;
                case    DX_LEFT:        ch[cn].status=176; ch[cn].status2=9; return;
                case    DX_RIGHT:       ch[cn].status=184; ch[cn].status2=9; return;
        }
        ch[cn].cerrno=ERR_FAILED;
}

void act_pickup(int cn)         // get the object in front of the character
{
        if ((ch[cn].flags&CF_SIMPLE)) { ch[cn].cerrno=ERR_FAILED; return; }

        ch[cn].cerrno=ERR_NONE;

        if (!do_char_can_flee(cn)  || (ch[cn].flags&CF_SIMPLE)) { ch[cn].cerrno=ERR_FAILED; return; }

        switch(ch[cn].dir) {
                case    DX_UP:          if (ch[cn].y>0) { ch[cn].status=160; ch[cn].status2=1; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_DOWN:        if (ch[cn].y<MAPY-1) { ch[cn].status=168; ch[cn].status2=1; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_LEFT:        if (ch[cn].x>0) { ch[cn].status=176; ch[cn].status2=1; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_RIGHT:       if (ch[cn].y<MAPX-1) { ch[cn].status=184; ch[cn].status2=1; } else ch[cn].cerrno=ERR_FAILED; break;
                default:                ch[cn].cerrno=ERR_FAILED; break;
        }
}

void act_use(int cn)            // get the object in front of the character
{
        ch[cn].cerrno=ERR_NONE;

        if (!do_char_can_flee(cn) || (ch[cn].flags&CF_SIMPLE)) { ch[cn].cerrno=ERR_FAILED; return; }

        switch(ch[cn].dir) {
                case    DX_UP:          if (ch[cn].y>0) { ch[cn].status=160; ch[cn].status2=4; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_DOWN:        if (ch[cn].y<MAPY-1) { ch[cn].status=168; ch[cn].status2=4; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_LEFT:        if (ch[cn].x>0) { ch[cn].status=176; ch[cn].status2=4; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_RIGHT:       if (ch[cn].y<MAPX-1) { ch[cn].status=184; ch[cn].status2=4; } else ch[cn].cerrno=ERR_FAILED; break;
                default:                ch[cn].cerrno=ERR_FAILED; break;
        }
}

void act_drop(int cn)           // drops the current object in front of the character
{
        ch[cn].cerrno=ERR_NONE;

        if (!do_char_can_flee(cn) || (ch[cn].flags&CF_SIMPLE)) { ch[cn].cerrno=ERR_FAILED; return; }

        switch(ch[cn].dir) {
                case    DX_UP:          if (ch[cn].y>0) { ch[cn].status=160; ch[cn].status2=2; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_DOWN:        if (ch[cn].y<MAPY-1) { ch[cn].status=168; ch[cn].status2=2; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_LEFT:        if (ch[cn].x>0) { ch[cn].status=176; ch[cn].status2=2; } else ch[cn].cerrno=ERR_FAILED; break;
                case    DX_RIGHT:       if (ch[cn].y<MAPX-1) { ch[cn].status=184; ch[cn].status2=2; } else ch[cn].cerrno=ERR_FAILED; break;
                default:                ch[cn].cerrno=ERR_FAILED; break;
        }
}

void act_idle(int cn)
{
        if ((globs->ticker&15)==(cn&15)) do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);
}

// ------------------
// end of act library
// ------------------

void plr_reset_status(int cn)
{
        switch(ch[cn].dir) {
                case    DX_UP:          ch[cn].status=0; break;
                case    DX_DOWN:        ch[cn].status=1; break;
                case    DX_LEFT:        ch[cn].status=2; break;
                case    DX_RIGHT:       ch[cn].status=3; break;
                case    DX_LEFTUP:      ch[cn].status=4; break;
                case    DX_LEFTDOWN:    ch[cn].status=5; break;
                case    DX_RIGHTUP:     ch[cn].status=6; break;
                case    DX_RIGHTDOWN:   ch[cn].status=7; break;
                default:                xlog("plr_doact (svr_act.c): illegal value for dir: %d for char %d",ch[cn].dir,cn);
                                        ch[cn].dir=DX_UP;
                                        ch[cn].status=0;
                                        break;
        }
}


void plr_doact(int cn)
{
	unsigned long long prof;
	
        // put idle value in status in case driver does nothing...
        plr_reset_status(cn);

        if (group_active(cn)) {
        	prof=prof_start(); driver(cn); prof_stop(24,prof);
        }
}

unsigned char speedtab[20][20]=
{
//  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},      //20
        {1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1},      //19
        {1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1},      //18
        {1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1},      //17
        {1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1},      //16
        {1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},      //15
        {1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1},      //14
        {1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0},      //13
        {0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,0,1},      //12
        {0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1},      //11
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0},      //10
        {1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0},      //9
        {1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,1,0},      //8
        {0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},      //7
        {0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0},      //6
        {0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0},      //5
        {0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0},      //4
        {0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0},      //3
        {0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0},      //2
        {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0}       //1
};

static inline int speedo(int n)
{
        return speedtab[ch[n].speed][ctick];
}

void plr_act(int cn)
{
        if (ch[cn].stunned) { act_idle(cn); return; }
        if (ch[cn].flags&CF_STONED) { act_idle(cn); return; }

        switch(ch[cn].status) {
                // idle up
                case    0:              act_idle(cn); plr_doact(cn); return;
                // idle down
                case    1:              act_idle(cn); plr_doact(cn); return;
                // idle left
                case    2:              act_idle(cn); plr_doact(cn); return;
                // idle right
                case    3:              act_idle(cn); plr_doact(cn); return;
                // idle left-up
                case 4:                 act_idle(cn); plr_doact(cn); return;
                // idle left-down
                case 5:                 act_idle(cn); plr_doact(cn); return;
                // idle right-up
                case 6:                 act_idle(cn); plr_doact(cn); return;
                // idle right-down
                case 7:                 act_idle(cn); plr_doact(cn); return;

                // walk up
                case    16:
                case    17:
                case    18:
                case    19:
                case    20:
                case    21:
                case    22:     if (speedo(cn)) ch[cn].status++; return;
                case    23:     if (speedo(cn)) { ch[cn].status=16; plr_move_up(cn); plr_doact(cn); } return;

                // walk down
                case    24:
                case    25:
                case    26:
                case    27:
                case    28:
                case    29:
                case    30:     if (speedo(cn)) ch[cn].status++; return;
                case    31:     if (speedo(cn)) { ch[cn].status=24; plr_move_down(cn); plr_doact(cn); } return;

                // walk left
                case    32:
                case    33:
                case    34:
                case    35:
                case    36:
                case    37:
                case    38:     if (speedo(cn)) ch[cn].status++; return;
                case    39:     if (speedo(cn)) { ch[cn].status=32; plr_move_left(cn); plr_doact(cn); } return;

                // walk right
                case    40:
                case    41:
                case    42:
                case    43:
                case    44:
                case    45:
                case    46:     if (speedo(cn)) ch[cn].status++; return;
                case    47:     if (speedo(cn)) { ch[cn].status=40; plr_move_right(cn); plr_doact(cn); } return;

                // left+up:
                case    48:
                case    49:
                case    50:
                case    51:
                case    52:
                case    53:
                case    54:
                case    55:
                case    56:
                case    57:
                case    58:     if (speedo(cn)) ch[cn].status++; return;
                case    59:     if (speedo(cn)) { ch[cn].status=48; plr_move_leftup(cn); plr_doact(cn); } return;

                // left+down
                case    60:
                case    61:
                case    62:
                case    63:
                case    64:
                case    65:
                case    66:
                case    67:
                case    68:
                case    69:
                case    70:     if (speedo(cn)) ch[cn].status++; return;
                case    71:     if (speedo(cn)) { ch[cn].status=60; plr_move_leftdown(cn); plr_doact(cn); } return;

                // right+up
                case    72:
                case    73:
                case    74:
                case    75:
                case    76:
                case    77:
                case    78:
                case    79:
                case    80:
                case    81:
                case    82:     if (speedo(cn)) ch[cn].status++; return;
                case    83:     if (speedo(cn)) { ch[cn].status=72; plr_move_rightup(cn); plr_doact(cn); } return;

                // right+down
                case    84:
                case    85:
                case    86:
                case    87:
                case    88:
                case    89:
                case    90:
                case    91:
                case    92:
                case    93:
                case    94:     if (speedo(cn)) ch[cn].status++; return;
                case    95:     if (speedo(cn)) { ch[cn].status=84; plr_move_rightdown(cn); plr_doact(cn); } return;

                // turn up to left-up
                case    96:
                case    97:
                case    98:     if (speedo(cn)) ch[cn].status++; return;
                case    99:     if (speedo(cn)) { ch[cn].status=96; plr_turn_leftup(cn); plr_doact(cn); } return;

                // turn left-up to left
                case    100:
                case    101:
                case    102:    if (speedo(cn)) ch[cn].status++; return;
                case    103:    if (speedo(cn)) { ch[cn].status=96; plr_turn_left(cn); plr_doact(cn); } return;

                // turn up to right-up
                case    104:
                case    105:
                case    106:    if (speedo(cn)) ch[cn].status++; return;
                case    107:    if (speedo(cn)) { ch[cn].status=104; plr_turn_rightup(cn); plr_doact(cn); } return;

                // turn right-up to up
                case    108:
                case    109:
                case    110:    if (speedo(cn)) ch[cn].status++; return;
                case    111:    if (speedo(cn)) { ch[cn].status=108; plr_turn_right(cn); plr_doact(cn); } return;

                // turn down to left-down
                case    112:
                case    113:
                case    114:    if (speedo(cn)) ch[cn].status++; return;
                case    115:    if (speedo(cn)) { ch[cn].status=112; plr_turn_leftdown(cn); plr_doact(cn); } return;

                // turn left-down to down
                case    116:
                case    117:
                case    118:    if (speedo(cn)) ch[cn].status++; return;
                case    119:    if (speedo(cn)) { ch[cn].status=116; plr_turn_left(cn); plr_doact(cn); } return;

                // turn down to right-down
                case    120:
                case    121:
                case    122:    if (speedo(cn)) ch[cn].status++; return;
                case    123:    if (speedo(cn)) { ch[cn].status=120; plr_turn_rightdown(cn); plr_doact(cn); } return;

                // turn right-down to right
                case    124:
                case    125:
                case    126:    if (speedo(cn)) ch[cn].status++; return;
                case    127:    if (speedo(cn)) { ch[cn].status=124; plr_turn_right(cn); plr_doact(cn); } return;

                // turn left to left-up
                case    128:
                case    129:
                case    130:    if (speedo(cn)) ch[cn].status++; return;
                case    131:    if (speedo(cn)) { ch[cn].status=128; plr_turn_leftup(cn); plr_doact(cn); } return;

                // turn left-up to up
                case    132:
                case    133:
                case    134:    if (speedo(cn)) ch[cn].status++; return;
                case    135:    if (speedo(cn)) { ch[cn].status=132; plr_turn_up(cn); plr_doact(cn); } return;

                // turn left to left-down
                case    136:
                case    137:
                case    138:    if (speedo(cn)) ch[cn].status++; return;
                case    139:    if (speedo(cn)) { ch[cn].status=136; plr_turn_leftdown(cn); plr_doact(cn); } return;

                // turn left-down to down
                case    140:
                case    141:
                case    142:    if (speedo(cn)) ch[cn].status++; return;
                case    143:    if (speedo(cn)) { ch[cn].status=140; plr_turn_down(cn); plr_doact(cn); } return;

                // turn right to right-up
                case    144:
                case    145:
                case    146:    if (speedo(cn)) ch[cn].status++; return;
                case    147:    if (speedo(cn)) { ch[cn].status=144; plr_turn_rightup(cn); plr_doact(cn); } return;

                // turn right-up to right
                case    148:
                case    149:
                case    150:    if (speedo(cn)) ch[cn].status++; return;
                case    151:    if (speedo(cn)) { ch[cn].status=148; plr_turn_up(cn); plr_doact(cn); } return;

                // turn right to right-down
                case    152:
                case    153:
                case    154:    if (speedo(cn)) ch[cn].status++; return;
                case    155:    if (speedo(cn)) { ch[cn].status=152; plr_turn_rightdown(cn); plr_doact(cn); } return;

                // turn right-down to down
                case    156:
                case    157:
                case    158:    if (speedo(cn)) ch[cn].status++; return;
                case    159:    if (speedo(cn)) { ch[cn].status=156; plr_turn_down(cn); plr_doact(cn); } return;

                // misc up
                case    160:
                case    161:
                case    162:
                case    163:
                case    164:
                case    165:
                case    166:    if (speedo(cn)) ch[cn].status++; return;
                case    167:    if (speedo(cn)) { ch[cn].status=160; plr_misc(cn); plr_doact(cn); } return;

                // misc down
                case    168:
                case    169:
                case    170:
                case    171:
                case    172:
                case    173:
                case    174:    if (speedo(cn)) ch[cn].status++; return;
                case    175:    if (speedo(cn)) { ch[cn].status=168; plr_misc(cn); plr_doact(cn); } return;

                // misc left
                case    176:
                case    177:
                case    178:
                case    179:
                case    180:
                case    181:
                case    182:    if (speedo(cn)) ch[cn].status++; return;
                case    183:    if (speedo(cn)) { ch[cn].status=176; plr_misc(cn); plr_doact(cn); } return;

                // misc right
                case    184:
                case    185:
                case    186:
                case    187:
                case    188:
                case    189:
                case    190:    if (speedo(cn)) ch[cn].status++; return;
                case    191:    if (speedo(cn)) { ch[cn].status=184; plr_misc(cn); plr_doact(cn); } return;

                default:        xlog("plr_act (svr_act.c): Unknown character status %d for char %d",ch[cn].status,cn);
                                ch[cn].status=0;
                                return;

        }
        /* not reached */
}
