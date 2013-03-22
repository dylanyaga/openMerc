/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

int pathfinder(int cn,int tx,int ty,int flag,int x2,int y2);
int char_moveto(int cn,int x,int y,int flag,int x2,int y2);
int char_dropto(int cn,int x,int y);
int char_pickup(int cn,int x,int y);
int char_use(int cn,int x,int y);
int char_pickupto(int cn,int x,int y);
int char_useto(int cn,int x,int y);
int char_attack_char(int cn,int co);
int char_give_char(int cn,int co);
int driver_fightback(int cn);
int drv_turncount(int dir1,int dir2);
int drv_gather(int cn,int *ignore,int isize,int ignore_nr);
int drv_find_prey(int cn,int *ignore,int isize,int looksize);
int drv_find_long_way(int cn,int x,int y);
int drv_raise(int cn);
int drv_shop(int cn,int state);
int drv_drop(int cn,int state);
int drv_is_enemy(int cn,int co);
int drv_is_friend(int cn,int co);
int drv_dcoor2dir(int dx,int dy);
int drv_attack(int cn,int co);

int follow_driver(int cn,int co);

int spell_value(int in);

void do_sayx(int cn,char *format,...) __attribute__ ((format(printf,2,3)));

#define ERR_NONE	0
#define ERR_SUCCESS	1	// operation finished, successfully
#define ERR_FAILED	2	// failed and will never succedd
