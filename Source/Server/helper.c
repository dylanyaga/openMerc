/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <malloc.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

//static int ox=0,oy=0;
//static char visi[40*40];

static char *visi;
static char _visi[40*40];
static int ox=0,oy=0;
static char ismonster=0;

static inline void add_vis(int x,int y,int v)
{
        if (!visi[(x-ox+20)+(y-oy+20)*40]) visi[(x-ox+20)+(y-oy+20)*40]=v;
}

static inline int check_map_see(int x,int y)
{
        int m;

        if (x<=0 || x>=MAPX || y<=0 || y>=MAPY) return 0;

        m=x+y*MAPX;

        if (ismonster) { if (map[m].flags&(MF_SIGHTBLOCK|MF_NOMONST)) return 0; }
        else { if (map[m].flags&MF_SIGHTBLOCK) return 0; }
        if (map[m].it && (it[map[m].it].flags&IF_SIGHTBLOCK)) return 0;

        return 1;
}

static inline int check_map_go(int x,int y)
{
        int m;

        if (x<=0 || x>=MAPX || y<=0 || y>=MAPY) return 0;

        m=x+y*MAPX;

        if (map[m].flags&MF_MOVEBLOCK) return 0;

        if (map[m].it && (it[map[m].it].flags&IF_MOVEBLOCK)) return 0;

        return 1;
}

static inline int close_vis_see(int x,int y,int v)
{
        if (!check_map_see(x,y)) return 0;

        x=x-ox+20;
        y=y-oy+20;

        if (visi[(x+1)+(y)*40]==v) return 1;
        if (visi[(x-1)+(y)*40]==v) return 1;
        if (visi[(x)+(y+1)*40]==v) return 1;
        if (visi[(x)+(y-1)*40]==v) return 1;
        if (visi[(x+1)+(y+1)*40]==v) return 1;
        if (visi[(x+1)+(y-1)*40]==v) return 1;
        if (visi[(x-1)+(y+1)*40]==v) return 1;
        if (visi[(x-1)+(y-1)*40]==v) return 1;

        return 0;
}

static inline int close_vis_go(int x,int y,int v)
{
        if (!check_map_go(x,y)) return 0;

        x=x-ox+20;
        y=y-oy+20;

        if (visi[(x+1)+(y)*40]==v) return 1;
        if (visi[(x-1)+(y)*40]==v) return 1;
        if (visi[(x)+(y+1)*40]==v) return 1;
        if (visi[(x)+(y-1)*40]==v) return 1;
        if (visi[(x+1)+(y+1)*40]==v) return 1;
        if (visi[(x+1)+(y-1)*40]==v) return 1;
        if (visi[(x-1)+(y+1)*40]==v) return 1;
        if (visi[(x-1)+(y-1)*40]==v) return 1;

        return 0;
}

static inline int check_vis(int x,int y)
{
        int best=99;

        x=x-ox+20;
        y=y-oy+20;

        if (visi[(x+1)+(y+0)*40] && visi[(x+1)+(y+0)*40]<best) best=visi[(x+1)+(y+0)*40];
        if (visi[(x-1)+(y+0)*40] && visi[(x-1)+(y+0)*40]<best) best=visi[(x-1)+(y+0)*40];
        if (visi[(x+0)+(y+1)*40] && visi[(x+0)+(y+1)*40]<best) best=visi[(x+0)+(y+1)*40];
        if (visi[(x+0)+(y-1)*40] && visi[(x+0)+(y-1)*40]<best) best=visi[(x+0)+(y-1)*40];
        if (visi[(x+1)+(y+1)*40] && visi[(x+1)+(y+1)*40]<best) best=visi[(x+1)+(y+1)*40];
        if (visi[(x+1)+(y-1)*40] && visi[(x+1)+(y-1)*40]<best) best=visi[(x+1)+(y-1)*40];
        if (visi[(x-1)+(y+1)*40] && visi[(x-1)+(y+1)*40]<best) best=visi[(x-1)+(y+1)*40];
        if (visi[(x-1)+(y-1)*40] && visi[(x-1)+(y-1)*40]<best) best=visi[(x-1)+(y-1)*40];

        if (best==99) return 0;
        else return best;
}

static void can_map_see(int _fx,int _fy,int maxdist)
{
        int xc,yc,x,y,dist;

        bzero(visi,sizeof(char)*40*40);

        ox=_fx; oy=_fy;
        xc=_fx; yc=_fy;

        add_vis(_fx,_fy,1);

        for (dist=1; dist<maxdist+1; dist++) {
                for (x=xc-dist; x<=xc+dist; x++) {
                        if (close_vis_see(x,yc-dist,dist)) add_vis(x,yc-dist,dist+1);
                        if (close_vis_see(x,yc+dist,dist)) add_vis(x,yc+dist,dist+1);
                }
                for (y=yc-dist+1; y<=yc+dist-1; y++) {
                        if (close_vis_see(xc-dist,y,dist)) add_vis(xc-dist,y,dist+1);
                        if (close_vis_see(xc+dist,y,dist)) add_vis(xc+dist,y,dist+1);
                }
        }
}

static void can_map_go(int _fx,int _fy,int maxdist)
{
        int xc,yc,x,y,dist;

        bzero(visi,sizeof(char)*40*40);

        ox=_fx; oy=_fy;
        xc=_fx; yc=_fy;

        add_vis(_fx,_fy,1);

        for (dist=1; dist<maxdist+1; dist++) {
                for (x=xc-dist; x<=xc+dist; x++) {
                        if (close_vis_go(x,yc-dist,dist)) add_vis(x,yc-dist,dist+1);
                        if (close_vis_go(x,yc+dist,dist)) add_vis(x,yc+dist,dist+1);
                }
                for (y=yc-dist+1; y<=yc+dist-1; y++) {
                        if (close_vis_go(xc-dist,y,dist)) add_vis(xc-dist,y,dist+1);
                        if (close_vis_go(xc+dist,y,dist)) add_vis(xc+dist,y,dist+1);
                }
        }
}

void reset_go(int xc,int yc)
{
        int x,y,cn;

        for (y=max(0,yc-18); y<min(MAPY-1,yc+18); y++)
                for (x=max(0,xc-18); x<min(MAPX-1,xc+18); x++)
                        if ((cn=map[x+y*MAPX].ch)!=0) see[cn].x=see[cn].y=0;

        ox=oy=0;
}

int can_see(int cn,int _fx,int _fy,int tx,int ty,int maxdist)
{
        int tmp;
        extern int see_hit,see_miss;
        unsigned long long prof;

        prof=prof_start();

        if (cn) {
                visi=see[cn].vis;
                if (_fx!=see[cn].x || _fy!=see[cn].y) {
                        if ((ch[cn].kindred&KIN_MONSTER) && !(ch[cn].flags&(CF_USURP|CF_THRALL))) ismonster=1;
                        else ismonster=0;
                        can_map_see(_fx,_fy,maxdist);
                        see[cn].x=_fx;
                        see[cn].y=_fy;
                        see_miss++;
                } else {
                        see_hit++;
                        ox=_fx;
                        oy=_fy;
                }
        } else {
                if (visi!=_visi) { visi=_visi; ox=oy=0; }
                if (ox!=_fx || oy!=_fy) {
                        ismonster=0;
                        can_map_see(_fx,_fy,maxdist);
                }
        }

        tmp=check_vis(tx,ty);

	prof_stop(16,prof);

        return tmp;
}

int can_go(int _fx,int _fy,int tx,int ty)
{
        int tmp;
	unsigned long long prof;
	
	prof=prof_start();

        if (visi!=_visi) { visi=_visi; ox=oy=0; }
        if (ox!=_fx || oy!=_fy) can_map_go(_fx,_fy,15);
        tmp=check_vis(tx,ty);
	
	prof_stop(17,prof);
	
        return tmp;
}

/* moved to svr_tick for speed reasons
int check_dlight(int x,int y)
{
        int m;

        m=x+y*MAPX;

        if (!(map[m].flags&MF_INDOORS)) return globs->dlight;

        return (globs->dlight*map[m].dlight)/256;
} */

void compute_dlight(int xc,int yc)
{
        int xs,ys,xe,ye,x,y,v,d,best=0,m;
        unsigned long long prof;

        prof=prof_start();

        xs=max(0,xc-LIGHTDIST);
        ys=max(0,yc-LIGHTDIST);
        xe=min(MAPX-1,xc+1+LIGHTDIST);
        ye=min(MAPY-1,yc+1+LIGHTDIST);

        for (y=ys; y<ye; y++) {
                m=y*MAPX+xs;
                for (x=xs; x<xe; x++,m++) {
                        if ((xc-x)*(xc-x)+(yc-y)*(yc-y)>(LIGHTDIST*LIGHTDIST+1)) continue;
                        if (!(map[m].flags&MF_INDOORS)) {
                                if ((v=can_see(0,xc,yc,x,y,LIGHTDIST))==0) continue;
                                d=256/(v*(abs(xc-x)+abs(yc-y)));
                                if (d>best) best=d;
                        }
                }
        }
        if (best>256) best=256;
        map[xc+yc*MAPX].dlight=best;

        prof_stop(18,prof);
}

void remove_lights(int x,int y)
{
        int xs,ys,xe,ye,in,cn,v,m;
        unsigned long long prof;

        prof=prof_start();

        xs=max(1,x-LIGHTDIST);
        ys=max(1,y-LIGHTDIST);
        xe=min(MAPX-2,x+1+LIGHTDIST);
        ye=min(MAPY-2,y+1+LIGHTDIST);

        for (y=ys; y<ye; y++) {
                m=y*MAPX+xs;
                for (x=xs; x<xe; x++,m++) {
                        if ((in=map[m].it)!=0) {
                                if (it[in].active) {
                                        if ((v=it[in].light[1])!=0)
                                                do_add_light(x,y,-v);
                                } else {
                                        if ((v=it[in].light[0])!=0)
                                                do_add_light(x,y,-v);
                                }
                        }

                        if ((cn=map[m].ch)!=0)
                                if ((v=ch[cn].light)!=0)
                                        do_add_light(x,y,-v);
                        map[m].dlight=0;
                }
        }

        prof_stop(19,prof);
}

void add_lights(int x,int y)
{
        int xs,ys,xe,ye,in,cn,v,m;
        unsigned long long prof;

        prof=prof_start();

        xs=max(1,x-LIGHTDIST);
        ys=max(1,y-LIGHTDIST);
        xe=min(MAPX-2,x+1+LIGHTDIST);
        ye=min(MAPY-2,y+1+LIGHTDIST);

        for (y=ys; y<ye; y++) {
                m=y*MAPX+xs;
                for (x=xs; x<xe; x++,m++) {
                        if ((in=map[m].it)!=0) {
                                if (it[in].active) {
                                        if ((v=it[in].light[1])!=0)
                                                do_add_light(x,y,v);
                                } else {
                                        if ((v=it[in].light[0])!=0)
                                                do_add_light(x,y,v);
                                }
                        }
                        if ((cn=map[m].ch)!=0)
                                if ((v=ch[cn].light)!=0)
                                        do_add_light(x,y,v);
                        if (map[m].flags&MF_INDOORS)
                                compute_dlight(x,y);
                }
        }

        prof_stop(20,prof);
}

int char_id(int cn)
{
        int id=0,n;

        for (n=0; n<40; n+=sizeof(int))
                id^=*(unsigned int*)(&ch[cn].name[n]);

        id^=ch[cn].pass1;
        id^=ch[cn].pass2;

        return id;
}

int points2rank(int v)
{
        if (v<      50) return 0;
        if (v<     850) return 1;
        if (v<    4900) return 2;
        if (v<   17700) return 3;
        if (v<   48950) return 4;
        if (v<  113750) return 5;
        if (v<  233800) return 6;
        if (v<  438600) return 7;
        if (v<  766650) return 8;
        if (v< 1266650) return 9;
        if (v< 1998700) return 10;
        if (v< 3035500) return 11;
        if (v< 4463550) return 12;
        if (v< 6384350) return 13;
        if (v< 8915600) return 14;
        if (v<12192400) return 15;
        if (v<16368450) return 16;
        if (v<21617250) return 17;
        if (v<28133300) return 18;
        if (v<36133300) return 19;

        if (v<49014500) return 20;
        if (v<63000600) return 21;
        if (v<80977100) return 22;

        return 23;
}

/* Calculates experience to next level from current experience and the
   points2rank() function. As no inverse function is supplied we use a
   binary search to determine the experience for the next level.
   If the given number of points corresponds to the highest level,
   return 0. */
int points_tolevel(int curr_exp)
{
        int curr_level, next_level, p0, p5, p9, r, j;

        curr_level = points2rank(curr_exp);
        if (curr_level == 23) return 0;
        next_level = curr_level + 1;

        p0 = 1;
        p5 = 1;
        p9 = 20 * curr_exp;
        for (j=0; p0<p9 && j<100; j++) {
                p5 = (p0 + p9) / 2;
                r = points2rank(curr_exp + p5);
                if (r < next_level) {
                        p0 = p5 + 1;
                } else {
                        p9 = p5 - 1;
                }
        }
        if (p0 > (20*curr_exp)) return 0;       // Can't do it
        p5++;
        return p5;
}

int rankdiff(int cn,int co)
{
        return points2rank(ch[co].points_tot)-points2rank(ch[cn].points_tot);
}

int absrankdiff(int cn, int co)
{
        int rd = rankdiff(cn, co);
        return (rd < 0 ? -rd : rd);
}

int in_attackrange(int cn, int co)
{
        return (absrankdiff(cn, co) <= ATTACK_RANGE);
}

int in_grouprange(int cn, int co)
{
        return (absrankdiff(cn, co) <= GROUP_RANGE);
}

int scale_exps2(int cn,int co_rank,int exp)
{
        static float scale_tab[49]={
        //       -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,   0,
                 0.01,0.01,0.01,0.01,0.01,0.01,0.01,0.01,0.01,0.02,0.03,0.04,0.05,0.06,0.07,0.10,0.15,0.20,0.25,0.33,0.50,0.70,0.80,0.90,1.00,

        //         1,   2,   3,   4,   5,   6,   7,   8,   9 , 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24
                1.02,1.04,1.08,1.16,1.32,1.50,1.75,2.00,2.25,2.50,2.75,3.00,3.25,3.50,3.75,4.00,4.00,4.00,4.00,4.00,4.00,4.00,4.00,4.00};
        int diff;

        diff=co_rank-points2rank(ch[cn].points_tot);

        diff+=24;
        if (diff<0) diff=0;
        if (diff>48) diff=48;

//      xlog("scale %d to %d: diff=%d, scale=%f",points2rank(ch[cn].points_tot),co_rank,diff,scale_tab[diff]);

        return (int)(exp*scale_tab[diff]);

}

int scale_exps(int cn,int co,int exp)
{
        return scale_exps2(cn,points2rank(ch[co].points_tot),exp);
}

/* CS, 991128: Ranks rearranged for clarity */
char *rank_name[RANKS] = {
"Private",              "Private First Class",  "Lance Corporal",	// 0 1 2
"Corporal",             "Sergeant",             "Staff Sergeant",	// 3 4 5
"Master Sergeant",      "First Sergeant",       "Sergeant Major",	// 6 7 8
"Second Lieutenant",    "First Lieutenant",     "Captain",		// 9 10 11

"Major",                "Lieutenant Colonel",   "Colonel",		// 12 13 14
"Brigadier General",    "Major General",        "Lieutenant General",	// 15 16 17
"General",              "Field Marshal",        "Knight of Astonia",	// 18 19 20
"Baron of Astonia",     "Earl of Astonia",      "Warlord of Astonia"	// 21 22 23
};

char *who_rank_name[RANKS] = {
" Pvt ",                " PFC ",                " LCp ",
" Cpl ",                " Sgt ",                " SSg ",
" MSg ",                " 1Sg ",                " SgM ",
"2Lieu",                "1Lieu",                "Captn",

"Major",                "LtCol",                "Colnl",
"BrGen",                "MaGen",                "LtGen",
"Genrl",                "FDMAR",                "KNIGT",
//"BARON",                "EARLA",                "WARLD",	// SH 04.04.2000
"BARON",                " EARL",                "WARLD",	// SH 04.04.2000
};

int create_special_item(int temp)
{
        int in,mul=1,spr;
        char *pref,*suffix,name[60];

        in=god_create_item(temp);
        if (!in) return 0;

        it[in].temp=0;

        switch(RANDOM(8)) {
                case    0:      pref="Shining "; it[in].light[0]+=10; break;
                case    1:      pref="Godly "; mul=2; break;
                default:        pref=""; break;
        }
	switch(RANDOM(8)) {	// SH 04.04.2000
//        switch(RANDOM(7)) {
                case    0:      suffix=" of the Lion"; it[in].attrib[AT_BRAVE][0]+=4*mul; break;
                case    1:      suffix=" of the Snake"; it[in].attrib[AT_WILL][0]+=4*mul; break;
                case    2:      suffix=" of the Owl"; it[in].attrib[AT_INT][0]+=4*mul; break;
                case    3:      suffix=" of the Weasel"; it[in].attrib[AT_AGIL][0]+=4*mul; break;
                case    4:      suffix=" of the Bear"; it[in].attrib[AT_STREN][0]+=4*mul; break;
                case    5:      suffix=" of Magic"; it[in].mana[0]+=10*mul; break;
                case    6:      suffix=" of Life"; it[in].hp[0]+=10*mul; break;
/* Added by SoulHunter 04.04.2000 */
		case	7:	suffix=" of Defence"; it[in].armor[0]+=2*mul; break;
/* --end */
		default:        suffix=""; break; // not reached!
        }

        switch(temp) {
                case    57:     spr=840; break;
                case    59:     spr=845; break;
                case    63:     spr=830; break;
                case    65:     spr=835; break;
                case    69:     spr=870; break;
                case    71:     spr=875; break;
                case    75:     spr=850; break;
                case    76:     spr=855; break;
                case    94:     spr=860; break;
                case    95:     spr=865; break;
                case    981:    spr=16775; break;
                case    982:    spr=16780; break;
                default:        spr=it[in].sprite[0]; break;
        }

        it[in].sprite[0]=spr;
        it[in].max_damage=0;

        strcpy(name,it[in].name);
        sprintf(it[in].name,"%s%s%s",pref,name,suffix);
        sprintf(it[in].reference,"%s%s%s",pref,name,suffix);
        sprintf(it[in].description,"A %s%s%s.",pref,name,suffix);
        it[in].name[0]=toupper(it[in].name[0]);

        return in;
}


struct npc_class
{
        char *name;
};

struct npc_class npc_class[]={
        {""},                           // 0
        {"Weak Thief"},                 // 1
        {"Thief"},                      // 2
        {"Ghost"},                      // 3
        {"Weak Skeleton"},              // 4
        {"Strong Skeleton"},            // 5
        {"Skeleton"},                   // 6
        {"Outlaw"},                     // 7
        {"Grolm Fighter"},              // 8
        {"Grolm Warrior"},              // 9
        {"Grolm Knight"},               // 10
        {"Lizard Youngster"},           // 11
        {"Lizard Youth"},               // 12
        {"Lizard Worker"},              // 13
        {"Lizard Fighter"},             // 14
        {"Lizard Warrior"},             // 15
        {"Lizard Mage"},                // 16
        {"Ratling"},                    // 17
        {"Ratling Fighter"},            // 18
        {"Ratling Warrior"},            // 19
        {"Ratling Knight"},             // 20
        {"Ratling Baron"},              // 21
        {"Ratling Count"},              // 22
        {"Ratling Duke"},               // 23
        {"Ratling Prince"},             // 24
        {"Ratling King"},               // 25
        {"Spellcaster"},                // 26
        {"Knight"},                     // 27
        {"Weak Golem"},                 // 28
        {"Captain Gargoyle"},           // 29
        {"Undead"},                     // 30
        {"Very Strong Ice Gargoyle"},   // 31
        {"Strong Outlaw"},              // 32
        {"Private Grolm"},              // 33
        {"PFC Grolm"},                  // 34
        {"Lance Corp Grolm"},           // 35
        {"Corporal Grolm"},             // 36
        {"Sergeant Grolm"},             // 37
        {"Staff Sergeant Grolm"},       // 38
        {"Master Sergeant Grolm"},      // 39
        {"First Sergeant Grolm"},       // 40
        {"Sergeant Major Grolm"},       // 41
        {"2nd Lieutenant Grolm"},       // 42
        {"1st Lieutenant Grolm"},       // 43
        {"Major Gargoyle"},             // 44
        {"Lt. Colonel Gargoyle"},      	// 45
        {"Colonel Gargoyle"},           // 46
        {"Brig. General Gargoyle"},     // 47
        {"Major General Gargoyle"},     // 48
        {"Lieutenant Gargoyle"},        // 49
        {"Weak Spider"},                // 50
        {"Spider"},                     // 51
        {"Strong Spider"},              // 52
        {"Very Strong Outlaw"},         // 53
        {"Lizard Knight"},              // 54
        {"Lizard Archmage"},            // 55
        {"Undead Lord"},                // 56
        {"Undead King"},                // 57
        {"Very Weak Ice Gargoyle"},     // 58
        {"Strong Golem"},               // 59
        {"Strong Ghost"},               // 60
        {"Shiva"},                      // 61
        {"Flame"},                      // 62
        {"Weak Ice Gargoyle"},          // 63
        {"Ice Gargoyle"},               // 64
        {"Strong Ice Gargoyle"},        // 65
        {"Greenling"},                  // 66
        {"Greenling Fighter"},          // 67
        {"Greenling Warrior"},          // 68
        {"Greenling Knight"},           // 69
        {"Greenling Baron"},            // 70
        {"Greenling Count"},            // 71
        {"Greenling Duke"},             // 72
        {"Greenling Prince"},           // 73
        {"Greenling King"},             // 74
        {"Strong Thief"},               // 75
        {"Major Grolm"}};               // 76

int killed_class(int cn,int val)
{
        int bit,tmp;

        if (val<32) {
                bit=1<<(val);
                tmp=ch[cn].data[60]&bit;
                ch[cn].data[60]|=bit;
                return tmp;
        } else if (val<64) {
                bit=1<<(val-32);
                tmp=ch[cn].data[61]&bit;
                ch[cn].data[61]|=bit;
                return tmp;
        } else if (val<96) {
                bit=1<<(val-64);
                tmp=ch[cn].data[62]&bit;
                ch[cn].data[62]|=bit;
                return tmp;
        } else {
                bit=1<<(val-96);
                tmp=ch[cn].data[63]&bit;
                ch[cn].data[63]|=bit;
                return tmp;
        }
}

char *get_class_name(int nr)
{
        /* CS, 991128: Check for out-of-bounds on class number */
        if (nr < 0) {
                xlog("error: get_class_name(%d)", nr);
                return "err... nothing";
        } else if (nr >= ARRAYSIZE(npc_class)) {
                xlog("error: get_class_name(%d)", nr);
                return "umm... whatzit";
        }
        return npc_class[nr].name;
}


/* Convert case in str as for a proper name: 1st is capital, others lower.
   BEWARE, this function changes its argument! */
void titlecase_str(char *str)
{
        if (*str) {
                *str = toupper(*str);
                for (str++; *str; str++)
                        *str = tolower(*str);
        }
}

/* Returns 1 if the <abbr> is an abbreviation of <name> */
int prefix(char *abbr, char *name)
{
        if (!*abbr) return 0;
        for ( ; *abbr; abbr++,name++) if (*abbr != *name) return 0;
        return 1;
}


/* Convert a time difference in ticks to a number of months, days, hours, minutes */
/* NOTE: The time string returned is a shared static, so it will not keep fresh
   if someone else calls this function. */
char *ago_string(int dt)
{
        int minutes, hours, days, months;
        static char when_string[100];

        minutes = dt / (60 * TICKS);
        if (minutes <= 0) {
                strcpy(when_string, "just now");
        } else if (minutes < 60) {
                sprintf(when_string, "%d minutes ago", minutes);
        } else {
                hours = minutes / 60;
                if (hours <= 36) {
                        sprintf(when_string, "%d hours ago", hours);
                } else {
                        days = hours / 24;
                        if (days <= 61) {
                                sprintf(when_string, "%d days ago", days);
                        } else {
                                months = days / 30; // This is sloppy, I know.
                                sprintf(when_string, "%d months ago", months);
                        }
                }
        }
        return when_string;
}

int use_labtransfer(int cn,int nr,int exp)
{
        int x,y,co;

        for (y=159; y<179; y++) {
                for (x=164; x<=184; x++) {
                        if ((co=map[x+y*MAPX].ch) && (ch[co].flags&(CF_PLAYER|CF_LABKEEPER))) {
                                do_char_log(cn,0,"Sorry, the area is still busy. %s is there.\n",ch[co].name);
				chlog(cn,"Sorry, the area is still busy. %s is there",ch[co].name);
                                return 0;
                        }
                }
        }

        switch(nr) {
                case 1: co=pop_create_char(137,0); break;	// grolms
                case 2: co=pop_create_char(156,0); break;	// lizard
                case 3: co=pop_create_char(278,0); break;	// spellcaster
                case 4: co=pop_create_char(315,0); break;	// knight
                case 5: co=pop_create_char(328,0); break;	// undead
                case 6: co=pop_create_char(458,0); break;	// light&dark
                case 7: co=pop_create_char(462,0); break;	// underwater
                case 8: co=pop_create_char(845,0); break;	// forest / golem
                case 9: co=pop_create_char(919,0); break;	// riddle
		default:        do_char_log(cn,0,"Sorry, could not determine which enemy to send you.\n");
				chlog(cn,"Sorry, could not determine which enemy to send you");
				return 0;
        }


        if (!co) {
                do_char_log(cn,0,"Sorry, could not create your enemy.\n");
		chlog(cn,"Sorry, could not create your enemy");
                return 0;
        }

        if (!god_drop_char(co,174,172)) {
                do_char_log(cn,0,"Sorry, could not place your enemy.\n");
		chlog(cn,"Sorry, could not place your enemy");
                god_destroy_items(co);
                ch[co].used=USE_EMPTY;
                return 0;
        }

        ch[co].data[64]=globs->ticker+5*60*TICKS; // die in two minutes if not otherwise
        ch[co].data[24]=0;    // do not interfere in fights
        ch[co].data[36]=0;    // no walking around
        ch[co].data[43]=0;    // don't attack anyone
        ch[co].data[80]=0;    // no enemies

        ch[co].data[0]=cn;      // person to make solve
        ch[co].data[1]=nr;      // labnr
        ch[co].data[2]=exp;     // exp plr is supposed to get
        ch[co].flags|=CF_LABKEEPER|CF_NOSLEEP;
	ch[co].flags&=~CF_RESPAWN;

        npc_add_enemy(co,cn,1); // make him attack the solver

        if (!god_transfer_char(cn,174,166)) {
                do_char_log(cn,0,"Sorry, could not transfer you to your enemy.\n");
		chlog(cn,"Sorry, could not transfer you to your enemy");
                god_destroy_items(co);
                ch[co].used=USE_EMPTY;
                return 0;
        }
	chlog(cn,"Entered Labkeeper room");

        return 1;
}

void use_labtransfer2(int cn,int co)
{
	int cc;

	if (IS_COMPANION(cn) && (cc=ch[cn].data[63])==ch[co].data[0]) {
		do_char_log(cc,0,"Your Companion killed your enemy.\n");
		finish_laby_teleport(cc,ch[co].data[1],ch[co].data[2]);
		god_transfer_char(cn,512,512);
		chlog(cc,"Labkeeper room solved by GC");
		return;
	}
        if (ch[co].data[0]!=cn) {
                do_char_log(cn,0,"Sorry, this killing does not count, as you're not the designated killer.\n");
		chlog(cn,"Sorry, this killing does not count, as you're not the designated killer");
                return;
        }
        finish_laby_teleport(cn,ch[co].data[1],ch[co].data[2]);
	chlog(cn,"Solved Labkeeper Room");

	if ((cc=ch[cn].data[64]) && IS_SANENPC(cc) && IS_COMPANION(cc)) 	// transfer GC as well
		god_transfer_char(cc,512,512);	
}

//----------------------

static char **badword=NULL;
static int cursize=0,maxsize=0;

void init_badwords(void)
{
        char buf[80],*dst,*src;
        FILE *fp;

	if (badword) {
		free(badword); badword=NULL;
		cursize=maxsize=0;
	}

        fp=fopen("badwords.txt","r");
        if (!fp) return;

        while (fgets(buf,79,fp)) {
                buf[79]=0; // fgets is silly.

                dst=src=buf;

                while (*src) {
                        if (isspace(*src)) { src++; continue; }
                        if (*src=='\r' || *src=='\n') break;
                        *dst++=*src++;
                }
                *dst=0;

                if (strlen(buf)<3) continue;

                if (cursize>=maxsize) {
                        maxsize+=256;
                        badword=realloc(badword,maxsize*sizeof(char**));
                }
                badword[cursize++]=strdup(buf);
        }

        fclose(fp);
}

int is_badword(char *sentence)
{
        int n;
	char temp[255];

	if (strlen(sentence)>250) return 0;
	strcpy(temp,sentence);
	strlower(temp);

        for (n=0; n<cursize; n++) {
                if (strstr(temp,badword[n])) return 1;
        }
        return 0;
}

// checks for "bad" words in player talking and mutes them
void player_analyser(int cn,char *text)
{
	if (ch[cn].flags&CF_SHUTUP) return;

        if (is_badword(text)) {
		ch[cn].data[72]+=TICKS*50;
		if (ch[cn].data[72]>TICKS*120) {
			do_char_log(cn,0,"Don't say I didn't warn you. Now I'll shut your mouth for you!\n");
			ch[cn].flags|=CF_SHUTUP;
			chlog(cn,"Auto-Shutup for \"%s\" (%d)",text,ch[cn].data[72]/TICKS);
		} else if (ch[cn].data[72]>TICKS*80) {
			do_char_log(cn,0,"My, what a filthy mouth you have. You'd better keep it closed for a while!\n");
			chlog(cn,"Bad-Mouth warning for \"%s\" (%d)",text,ch[cn].data[72]/TICKS);
		}
	}
}

void show_time(int cn)
{
	int hour,minute,day,month,year;

	hour=globs->mdtime/(60*60);
	minute=(globs->mdtime/60)%60;
	day=globs->mdday%28+1;
	month=globs->mdday/28+1;
	year=globs->mdyear;	

	do_char_log(cn,1,"It's %d:%02d on the %d%s%s%s%s%s%s%s of the %d%s%s%s%s month of the year %d.\n",
		hour,minute,
		day,
		day==1  ? "st" : "",
		day==2  ? "nd" : "",
		day==3  ? "rd" : "",
		day==21 ? "st" : "",
		day==22 ? "nd" : "",
		day==23 ? "rd" : "",
		(day>3 && day<21) || day>23 ? "th" : "",
		month,
		month==1 ? "st" : "",
		month==2 ? "nd" : "",
		month==3 ? "rd" : "",
		month>3  ? "th" : "",
		year);
}

void scprintf(char *dst,char *format,...)
{
        va_list va;

        while (*dst) dst++;

        va_start(va,format);
        vsprintf(dst,format,va);
        va_end(va);
}

void effectlist(int cn)
{
	int n;
	char buf[256];

	for (n=1; n<MAXEFFECT; n++) {
		if (!fx[n].used) continue;

		sprintf(buf,"%3d: ",n);

		switch(fx[n].type) {
			case 1:		scprintf(buf,"REMINJ ");
					break;
			case 2:		scprintf(buf,"RSPAWN (%d,%d t=%d)",fx[n].data[0],fx[n].data[1],fx[n].data[2]);
					break;
			case 3:		scprintf(buf,"DTMIST ");
					break;
			case 4:		scprintf(buf,"TSTONE ");
					break;
			case 5:		scprintf(buf,"IMAGIC ");
					break;
			case 6:		scprintf(buf,"GMAGIC ");
					break;
			case 7:		scprintf(buf,"CMAGIC ");
					break;
			case 8:		scprintf(buf,"RSMIST ");
					break;
			case 9:		scprintf(buf,"MCREAT ");
					break;
			case 10:	scprintf(buf,"RSOBJT ");
					break;
			case 11:	scprintf(buf,"RQUEUE ");
					break;
			case 12:	scprintf(buf,"TRMIST ");
					break;
			default:	scprintf(buf,"?????? ");
					break;
		}
		scprintf(buf,"\n");
		do_char_log(cn,0,buf);
	}	
}

#define	CAP		(globs->cap)

void set_cap(int cn,int nr)
{
	if (!nr) {
		globs->flags&=~GF_CAP;
		do_char_log(cn,2,"Removed cap, old setting was at %d\n",CAP);
	} else {
		globs->flags|=GF_CAP;
		globs->cap=nr;
		do_char_log(cn,2,"Enabled cap, setting to %d\n",CAP);
	}
}

int cap(int cn,int nr)
{
	int n,place,qsize=0;

	if (!(globs->flags&GF_CAP)) return 0;				// no cap if feature is turned off

	if (cn && (ch[cn].flags&(CF_GOD))) return 0;    		// always allow gods to come in all the time

	if (cn) {	
		for (n=1; n<MAXCHARS; n++) {				// find body for possible grave retrieval
			if (!ch[n].used) continue;
			if (!(ch[n].flags&CF_BODY)) continue;
	
			if (ch[n].data[CHD_CORPSEOWNER]==cn) return 0;	// always allow corpse retrieval
		}
	}

        player[nr].prio+=10;

	if (cn && (ch[cn].flags&CF_IMP)) player[nr].prio+=25;;		// imps are preferred a lot

        if (cn && (ch[cn].flags&CF_STAFF)) player[nr].prio+=10;;	// staff is preferred

        if (cn && (ch[cn].flags&CF_GOLDEN)) player[nr].prio+=10;	// golden list players are preferred

        if (cn && (ch[cn].flags&CF_BLACK)) player[nr].prio-=5;		// not blacklisted player get disadvantage

	for (n=place=1; n<MAXPLAYER; n++) {
		if (!player[n].sock) continue;
		if (n==nr) continue;
		if (player[n].state==ST_NORMAL || player[n].state==ST_EXIT) continue;
		if (player[n].prio>=player[nr].prio) place++;
		qsize++;
	}

	globs->queuesize=qsize+globs->players_online-CAP;

        if (globs->players_online+place<=CAP) return 0;

        return place;
}

int soultransform(int cn,int in,int in2,int temp)
{
	god_take_from_char(in,cn);
	god_take_from_char(in2,cn);
	
	it[in].used=USE_EMPTY;
	it[in2].used=USE_EMPTY;
	
	in=god_create_item(temp);
	god_give_char(in,cn);
	
	return in;
}

int soulrepair(int cn,int in,int in2)
{
	god_take_from_char(in,cn);
	it[in].used=USE_EMPTY;

	it[in2]=it_temp[it[in2].temp];
	it[in2].carried=cn;
	it[in2].flags|=IF_UPDATE;
	it[in2].temp=0;

	return in2;
}

void souldestroy(int cn,int in)
{
	god_take_from_char(in,cn);
	it[in].used=USE_EMPTY;
}

#define over_add(a,b)	a=(((int)(a)+(int)(b)>120) ? (120) : ((a)+(b)))

void soultrans_equipment(int cn,int in,int in2)
{
	int stren,rank,ran;
	
	rank=it[in].data[0];
	
	while (rank) {
		stren=RANDOM(rank+1);
		rank-=stren;
		
		if (it[in].flags&IF_WEAPON) ran=RANDOM(27);
		else ran=RANDOM(26);
		
		switch(ran) {
			case 0:		it[in2].hp[2]+=stren*25; it[in2].hp[0]+=stren*5; break;
			case 1:		it[in2].mana[2]+=stren*25; it[in2].mana[0]+=stren*5; break;

			case 2:
			case 3:
			case 4:
			case 5:
			case 6:		over_add(it[in2].attrib[ran-2][2],stren*3); it[in2].attrib[ran-2][0]+=stren/2; break;
			
			case 7:		over_add(it[in2].skill[SK_DAGGER][2],stren*5); it[in2].skill[SK_DAGGER][0]+=stren; break;
			case 8:		over_add(it[in2].skill[SK_SWORD][2],stren*5); it[in2].skill[SK_SWORD][0]+=stren; break;
			case 9:		over_add(it[in2].skill[SK_TWOHAND][2],stren*5); it[in2].skill[SK_TWOHAND][0]+=stren; break;
			case 10:	over_add(it[in2].skill[SK_STEALTH][2],stren*5); it[in2].skill[SK_STEALTH][0]+=stren; break;
			case 11:	over_add(it[in2].skill[SK_MSHIELD][2],stren*5); it[in2].skill[SK_MSHIELD][0]+=stren; break;
			case 12:	over_add(it[in2].skill[SK_PROTECT][2],stren*5); it[in2].skill[SK_PROTECT][0]+=stren; break;
			case 13:	over_add(it[in2].skill[SK_ENHANCE][2],stren*5); it[in2].skill[SK_ENHANCE][0]+=stren; break;
			case 14:	over_add(it[in2].skill[SK_STUN][2],stren*5); it[in2].skill[SK_STUN][0]+=stren; break;
			case 15:	over_add(it[in2].skill[SK_CURSE][2],stren*5); it[in2].skill[SK_CURSE][0]+=stren; break;
			case 16:	over_add(it[in2].skill[SK_BLESS][2],stren*5); it[in2].skill[SK_BLESS][0]+=stren; break;
			case 17:	over_add(it[in2].skill[SK_RESIST][2],stren*5); it[in2].skill[SK_RESIST][0]+=stren; break;
			case 18:	over_add(it[in2].skill[SK_BLAST][2],stren*5); it[in2].skill[SK_BLAST][0]+=stren; break;
			case 19:	over_add(it[in2].skill[SK_HEAL][2],stren*5); it[in2].skill[SK_HEAL][0]+=stren; break;
			case 20:	over_add(it[in2].skill[SK_GHOST][2],stren*5); it[in2].skill[SK_GHOST][0]+=stren; break;
			case 21:	over_add(it[in2].skill[SK_IMMUN][2],stren*5); it[in2].skill[SK_IMMUN][0]+=stren; break;
			case 22:	over_add(it[in2].skill[SK_SURROUND][2],stren*5); it[in2].skill[SK_SURROUND][0]+=stren; break;
			case 23:	over_add(it[in2].skill[SK_CONCEN][2],stren*5); it[in2].skill[SK_CONCEN][0]+=stren; break;
			case 24:	over_add(it[in2].skill[SK_WARCRY][2],stren*5); it[in2].skill[SK_WARCRY][0]+=stren; break;
			case 25:	it[in2].armor[0]+=stren/2; break;
			case 26:	it[in2].weapon[0]+=stren/2; break;
			default:	xlog("should never happen in soultrans_equipment()"); break;
		}
	}
	
	it[in2].temp=0;
	it[in2].flags|=IF_UPDATE|IF_IDENTIFIED|IF_NOREPAIR|IF_SOULSTONE;
	
	it[in2].min_rank=max(it[in].data[0],it[in2].min_rank);

	if (!it[in2].max_damage) it[in2].max_damage=60000;
	
	souldestroy(cn,in);
	
	sprintf(it[in2].description,"A %s enhanced by a rank %d soulstone.",it[in2].name,it[in].data[0]);
}

int use_soulstone(int cn,int in)
{
	int in2,rank;
	
	if (!IS_SANECHAR(cn)) return 0;
	if (!IS_SANEITEM(in)) return 0;
	
	if (!(in2=ch[cn].citem)) {
		do_char_log(cn,1,"Try using something with the soulstone. That is, click on the stone with an item under your cursor.\n");
		return 0;
	}
	
	if (!IS_SANEITEM(in2)) return 0;
	
	if (it[in2].driver==68) {
		it[in].data[1]+=RANDOM(it[in2].data[1]+1)	;
		rank=points2rank(it[in].data[1]);
		it[in].data[0]=rank;
		sprintf(it[in].description,"Level %d soulstone, holding %d exp.",rank,it[in].data[1]);
		
		if (rank>20) do_char_log(cn,1,"That's as high as they go.\n");
		
		souldestroy(cn,in2);
		
		return 1;
	}
	
	switch(it[in2].temp) {
		case 18:	in=soultransform(cn,in,in2,101); it[in].hp[0]+=10; return 1;	// red flower
		case 46:	in=soultransform(cn,in,in2,102); it[in].mana[0]+=10; return 1;	// purple flower
		case 91:	in=soulrepair(cn,in,in2); it[in].max_age[1]*=4; return 1;	// torch
		case 100:	in=soultransform(cn,in,in2,102); return 1;			// flask
		
		case 101:	souldestroy(cn,in); it[in].hp[0]+=10; return 1;			// healing potion
		case 102:	souldestroy(cn,in); it[in].mana[0]+=10; return 1;		// mana potion
				
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 51:
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
		case 62:
		case 63:
		case 64:
		case 65:
		case 66:
		case 67:
		case 68:
		case 69:
		case 70:
		case 71:
		case 72:
		case 73:
		case 74:
		case 75:
		case 76:
		case 77:
		case 78:
		case 79:
		case 80:
		case 94:
		case 95:
		case 96:
		case 97:
		case 98:
		case 99:
		case 116:
		case 125:
		case 158:
		case 501:
		case 502:
		case 503:
		case 523:
		case 524:
		case 813:
		case 981:
		case 982:
		case 983:
		case 984:
		case 985:
		case 986:	soultrans_equipment(cn,in,in2); return 1;
		default:	do_char_log(cn,1,"Nothing happened.\n"); return 0;
	}
}

int load_mod(void)
{
	int handle,len;
	extern char mod[];

	handle=open("mod.txt",O_RDONLY);
	if (handle!=-1) {
		len=read(handle,mod,130);
		mod[len]=0;
		close(handle);
	} else strcpy(mod,"Live long and prosper!");

	return 1;
}

/*
#define SIZE 50

void test_filesend(int nr,int size)
{
	static char buf[16384];
	int n,trans;
	static int sent=0;

        for (n=0; n<sizeof(buf); n++) buf[n]=RANDOM(256);

	sent+=size/3;

	trans=(SIZE*player[nr].rtick)-sent;

	if (trans<5) return;

	trans=min(1024,trans);

	plog(nr,"trans=%d, allow=%d, sent=%d",trans,(SIZE*player[nr].rtick),sent);
	
	buf[0]=SV_IGNORE;
	*(unsigned int*)(buf+1)=trans;
	xsend(nr,buf,trans);
	sent+=trans;
	
}*/
