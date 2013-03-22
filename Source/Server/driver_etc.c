/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "server.h"
#include "driver.h"

int pathfinder(int cn,int tx,int ty,int flag,int x2,int y2);

int char_moveto(int cn,int x,int y,int flag,int x2,int y2)
{
	int dir,in;
	unsigned long long prof;

	if (ch[cn].x==x && ch[cn].y==y && flag!=1 && flag!=3) return 1;

	if (ch[cn].cerrno==ERR_FAILED) { ch[cn].cerrno=ERR_NONE; return -1; }

	if (ch[cn].unreach>globs->ticker && ch[cn].unreachx==x && ch[cn].unreachy==y) {
		return -1;
	} 

	prof=prof_start();
	dir=pathfinder(cn,x,y,flag,x2,y2); 
	prof_stop(1,prof);
	
	if (dir==-1) { 
		ch[cn].unreach=globs->ticker+TICKS;
		ch[cn].unreachx=x;
		ch[cn].unreachy=y;
		return -1; 
	}
	if (dir==0) return 0;

	switch(dir) {
		case DX_RIGHTDOWN:
			if (ch[cn].dir!=DX_RIGHTDOWN) { act_turn_rightdown(cn); return 0; }
			act_move_rightdown(cn);
			return 0;
		case DX_RIGHTUP:
			if (ch[cn].dir!=DX_RIGHTUP) { act_turn_rightup(cn); return 0; }
			act_move_rightup(cn);
			return 0;
		case DX_LEFTDOWN:
			if (ch[cn].dir!=DX_LEFTDOWN) { act_turn_leftdown(cn); return 0; }
			act_move_leftdown(cn);
			return 0;
		case DX_LEFTUP:
			if (ch[cn].dir!=DX_LEFTUP) { act_turn_leftup(cn); return 0; }
			act_move_leftup(cn);
			return 0;
		case DX_RIGHT:
			if (ch[cn].dir!=DX_RIGHT) { act_turn_right(cn); return 0; }
			if ((in=map[(ch[cn].x+1)+(ch[cn].y)*MAPX].it)!=0 && !it[in].active && it[in].driver==2)
				{ act_use(cn); return 0; }
			act_move_right(cn);
			return 0;
		case DX_LEFT:
			if (ch[cn].dir!=DX_LEFT) { act_turn_left(cn); return 0; }
			if ((in=map[(ch[cn].x-1)+(ch[cn].y)*MAPX].it)!=0 && !it[in].active && it[in].driver==2)
				{ act_use(cn); return 0; }
			act_move_left(cn);
			return 0;
		case DX_DOWN:
			if (ch[cn].dir!=DX_DOWN) { act_turn_down(cn); return 0; }
			if ((in=map[(ch[cn].x)+(ch[cn].y+1)*MAPX].it)!=0 && !it[in].active && it[in].driver==2)
				{ act_use(cn); return 0; }
			act_move_down(cn);
			return 0;
		case DX_UP:
			if (ch[cn].dir!=DX_UP) { act_turn_up(cn); return 0; }
			if ((in=map[(ch[cn].x)+(ch[cn].y-1)*MAPX].it)!=0 && !it[in].active && it[in].driver==2)
				{ act_use(cn); return 0; }
			act_move_up(cn);
			return 0;
	}
	return -1;
}

int char_dropto(int cn,int x,int y)
{
	int m;

	if (ch[cn].cerrno==ERR_FAILED) { ch[cn].cerrno=ERR_NONE; return -1; }

	// nothing to drop?
	if (!ch[cn].citem) return -1;

	m=x+y*MAPX;

	// drop if possible
	if (ch[cn].x==x-1 && ch[cn].y==y) {
		if (ch[cn].dir!=DX_RIGHT) 
			{ act_turn_right(cn); return 0; }
//		if (map[m].ch || map[m].to_ch || map[m].it || (map[m].flags&MF_MOVEBLOCK)) return-1;
		act_drop(cn);
		return 1;
	}
	if (ch[cn].x==x+1 && ch[cn].y==y) {
		if (ch[cn].dir!=DX_LEFT) 
			{ act_turn_left(cn); return 0; }
//		if (map[m].ch || map[m].to_ch || map[m].it || (map[m].flags&MF_MOVEBLOCK)) return-1;
		act_drop(cn);
		return 1;
	}
	if (ch[cn].x==x && ch[cn].y==y-1) {
		if (ch[cn].dir!=DX_DOWN) 
			{ act_turn_down(cn); return 0; }
//		if (map[m].ch || map[m].to_ch || map[m].it || (map[m].flags&MF_MOVEBLOCK)) return-1;
		act_drop(cn);
		return 1;
	} 
	if (ch[cn].x==x && ch[cn].y==y+1) {
		if (ch[cn].dir!=DX_UP) 
			{ act_turn_up(cn); return 0; }
//		if (map[m].ch || map[m].to_ch || map[m].it || (map[m].flags&MF_MOVEBLOCK)) return-1;
		act_drop(cn);
		return 1;
	}

	// we're too far away... go there:
	if (char_moveto(cn,x,y,1,0,0)==-1) return -1;

	return 0;
}

int char_pickup(int cn,int x,int y)
{
	if (ch[cn].cerrno==ERR_FAILED) { ch[cn].cerrno=ERR_NONE; return -1; }

	// pickup if possible
	if (ch[cn].x==x-1 && ch[cn].y==y) {
		if (ch[cn].dir!=DX_RIGHT) 
			{ act_turn_right(cn); return 0; }
		act_pickup(cn);
		return 1;
	}
	if (ch[cn].x==x+1 && ch[cn].y==y) {
		if (ch[cn].dir!=DX_LEFT) 
			{ act_turn_left(cn); return 0; }
		act_pickup(cn);
		return 1;
	}
	if (ch[cn].x==x && ch[cn].y==y-1) {
		if (ch[cn].dir!=DX_DOWN) 
			{ act_turn_down(cn); return 0; }
		act_pickup(cn);
		return 1;
	} 
	if (ch[cn].x==x && ch[cn].y==y+1) {
		if (ch[cn].dir!=DX_UP) 
			{ act_turn_up(cn); return 0; }
		act_pickup(cn);
		return 1;
	}

	return -1;
}

int char_use(int cn,int x,int y)
{
	if (ch[cn].cerrno==ERR_FAILED) { ch[cn].cerrno=ERR_NONE; return -1; }

	// use if possible
	if (ch[cn].x==x-1 && ch[cn].y==y) {
		if (ch[cn].dir!=DX_RIGHT) 
			{ act_turn_right(cn); return 0; }
		act_use(cn);
		return 1;
	}
	if (ch[cn].x==x+1 && ch[cn].y==y) {
		if (ch[cn].dir!=DX_LEFT) 
			{ act_turn_left(cn); return 0; }
		act_use(cn);
		return 1;
	}
	if (ch[cn].x==x && ch[cn].y==y-1) {
		if (ch[cn].dir!=DX_DOWN) 
			{ act_turn_down(cn); return 0; }
		act_use(cn);
		return 1;
	} 
	if (ch[cn].x==x && ch[cn].y==y+1) {
		if (ch[cn].dir!=DX_UP) 
			{ act_turn_up(cn); return 0; }
		act_use(cn);
		return 1;
	}

	return -1;
}

int char_pickupto(int cn,int x,int y)
{
	int ret;

	if (ch[cn].cerrno==ERR_FAILED) { ch[cn].cerrno=ERR_NONE; return -1; }

	// already an item in hand?
	if (ch[cn].citem) return -1;

	ret=char_pickup(cn,x,y);
	if (ret==-1) {
		if (char_moveto(cn,x,y,1,0,0)==-1) return -1;
		else return 0;
	}
	if (ret==1) return 1;
	return 0;
}

int char_useto(int cn,int x,int y)
{
	int ret;

	if (ch[cn].cerrno==ERR_FAILED) { ch[cn].cerrno=ERR_NONE; return -1; }

	ret=char_use(cn,x,y);
	if (ret==-1) {
		if (char_moveto(cn,x,y,1,0,0)==-1) return -1;
		else return 0;
	}
	if (ret==1) return 1;
	return 0;
}


int char_attack_char(int cn,int co)
{
	int ax,ay,x,y,err,tox,toy,dist1,dist2,diff;

	if (ch[cn].cerrno==ERR_FAILED) { ch[cn].cerrno=ERR_NONE; return -1; }

	if (ch[co].used!=USE_ACTIVE || do_char_can_see(cn,co)==0 || cn==co || (ch[co].flags&CF_BODY) || (ch[co].flags&CF_STONED)) return -1;

	x=ch[co].x; tox=ch[co].tox;
	y=ch[co].y; toy=ch[co].toy;
	ax=ch[cn].x; ay=ch[cn].y;

	if ((x==ax+1 && (y==ay+1 || y==ay-1)) || (x==ax-1 && (y==ay+1 || y==ay-1))) {                
                err=char_moveto(cn,x,y,2,tox,toy);
		if (err==-1) return -1;
		else return 0;
	}

	// attack if possible
	if ((ax==x-1 && ay==y) || (ax==tox-1 && ay==toy)) {
		if (ch[cn].dir!=DX_RIGHT) 
			{ act_turn_right(cn); return 0; }
		act_attack(cn);
		return 1;
	}
	if ((ax==x+1 && ay==y) || (ax==tox+1 && ay==toy)) {
		if (ch[cn].dir!=DX_LEFT) 
			{ act_turn_left(cn); return 0; }
		act_attack(cn);
		return 1;
	}
	if ((ax==x && ay==y-1) || (ax==tox && ay==toy-1)) {
		if (ch[cn].dir!=DX_DOWN) 
			{ act_turn_down(cn); return 0; }
		act_attack(cn);
		return 1;
	} 
	if ((ax==x && ay==y+1) || (ax==tox && ay==toy+1)) {
		if (ch[cn].dir!=DX_UP) 
			{ act_turn_up(cn); return 0; }
		act_attack(cn);
		return 1;
	}

        dist1=abs(ax-x)+abs(ay-y);
        dist2=abs(ax-tox)+abs(ay-toy);
        
        diff=dist1-dist2;

        if (dist1>20 && diff<5) {
                x=tox=tox+(tox-x)*8;
                y=toy=toy+(toy-y)*8;
        } else if (dist1>10 && diff<4) {
                x=tox=tox+(tox-x)*5;
                y=toy=toy+(toy-y)*5;
        } else if (dist1>5 && diff<3) {
                x=tox=tox+(tox-x)*3;
                y=toy=toy+(toy-y)*3;
        } else if (dist1>3 && diff<2) {
                x=tox=tox+(tox-x)*2;
                y=toy=toy+(toy-y)*2;
        } else if (dist1>2 && diff<1) {
                x=tox=tox+(tox-x);
                y=toy=toy+(toy-y);
        }

	err=char_moveto(cn,x,y,2,tox,toy);
	if (err==-1) return -1;
	else return 0;
}

int char_give_char(int cn,int co)
{
	int ax,ay,x,y,err,tox,toy;

	if (ch[cn].cerrno==ERR_FAILED) { ch[cn].cerrno=ERR_NONE; return -1; }

	if (ch[co].used!=USE_ACTIVE || do_char_can_see(cn,co)==0 || cn==co) 
		return -1;

	if (!ch[cn].citem) return 1;

	x=ch[co].x; tox=ch[co].tox;
	y=ch[co].y; toy=ch[co].toy;
	ax=ch[cn].x; ay=ch[cn].y;

	if ((x==ax+1 && (y==ay+1 || y==ay-1)) || (x==ax-1 && (y==ay+1 || y==ay-1))) {
		err=char_moveto(cn,x,y,2,tox,toy);
		if (err==-1) return -1;
		else return 0;
	}

	// give if possible
	if ((ax==x-1 && ay==y) || (ax==tox-1 && ay==toy)) {
		if (ch[cn].dir!=DX_RIGHT) 
			{ act_turn_right(cn); return 0; }
		act_give(cn);
		return 0;
	}
	if ((ax==x+1 && ay==y) || (ax==tox+1 && ay==toy)) {
		if (ch[cn].dir!=DX_LEFT) 
			{ act_turn_left(cn); return 0; }
		act_give(cn);
		return 0;
	}
	if ((ax==x && ay==y-1) || (ax==tox && ay==toy-1)) {
		if (ch[cn].dir!=DX_DOWN) 
			{ act_turn_down(cn); return 0; }
		act_give(cn);
		return 0;
	} 
	if ((ax==x && ay==y+1) || (ax==tox && ay==toy+1)) {
		if (ch[cn].dir!=DX_UP) 
			{ act_turn_up(cn); return 0; }
		act_give(cn);
		return 0;
	}

	err=char_moveto(cn,x,y,2,tox,toy);
	if (err==-1) return -1;
	else return 0;
}

int drv_dcoor2dir(int dx,int dy)
{
	if (dx>0 && dy>0) return DX_RIGHTDOWN;
	if (dx>0 && dy==0) return DX_RIGHT;
	if (dx>0 && dy<0) return DX_RIGHTUP;
	if (dx==0 && dy>0) return DX_DOWN;
	if (dx==0 && dy<0) return DX_UP;
	if (dx<0 && dy>0) return DX_LEFTDOWN;
	if (dx<0 && dy==0) return DX_LEFT;
	if (dx<0 && dy<0) return DX_LEFTUP;

	return -1;
}

int drv_turncount(int dir1,int dir2)
{
	if (dir1==dir2) return 0;
	if (dir1==DX_UP) {
		switch(dir2) {
			case	DX_DOWN: 	return 4;
			case	DX_RIGHTUP:
			case	DX_LEFTUP:	return 1;
			case	DX_RIGHT:
			case	DX_LEFT:	return 2;
			default:		return 3;
		}
	}
	if (dir1==DX_DOWN) {
		switch(dir2) {
			case	DX_UP: 		return 4;
			case	DX_RIGHTDOWN:
			case	DX_LEFTDOWN:	return 1;
			case	DX_RIGHT:
			case	DX_LEFT:	return 2;
			default:		return 3;
		}
	}
	if (dir1==DX_LEFT) {
		switch(dir2) {
			case	DX_RIGHT: 	return 4;
			case	DX_LEFTUP:
			case	DX_LEFTDOWN:	return 1;
			case	DX_UP:
			case	DX_DOWN:	return 2;
			default:		return 3;
		}
	}
	if (dir1==DX_RIGHT) {
		switch(dir2) {
			case	DX_LEFT: 	return 4;
			case	DX_RIGHTUP:
			case	DX_RIGHTDOWN:	return 1;
			case	DX_UP:
			case	DX_DOWN:	return 2;
			default:		return 3;
		}
	}
	if (dir1==DX_LEFTUP) {
		switch(dir2) {
			case	DX_RIGHTDOWN: 	return 4;
			case	DX_UP:
			case	DX_LEFT:	return 1;
			case	DX_RIGHTUP:
			case	DX_LEFTDOWN:	return 2;
			default:		return 3;
		}
	}
	if (dir1==DX_LEFTDOWN) {
		switch(dir2) {
			case	DX_RIGHTUP: 	return 4;
			case	DX_DOWN:
			case	DX_LEFT:	return 1;
			case	DX_RIGHTDOWN:
			case	DX_LEFTUP:	return 2;
			default:		return 3;
		}
	}
	if (dir1==DX_RIGHTUP) {
		switch(dir2) {
			case	DX_LEFTDOWN: 	return 4;
			case	DX_UP:
			case	DX_RIGHT:	return 1;
			case	DX_RIGHTDOWN:
			case	DX_LEFTUP:	return 2;
			default:		return 3;
		}
	}
	if (dir1==DX_RIGHTDOWN) {
		switch(dir2) {
			case	DX_LEFTUP: 	return 4;
			case	DX_DOWN:
			case	DX_RIGHT:	return 1;
			case	DX_RIGHTUP:
			case	DX_LEFTDOWN:	return 2;
			default:		return 3;
		}
	}
	return 99;
}
