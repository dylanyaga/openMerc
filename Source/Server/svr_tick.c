/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef ONLINE
#include <crypt.h>
#endif

#define TMIDDLEX (TILEX/2)
#define TMIDDLEY (TILEY/2)

#include "server.h"

int ctick=0;

static char intro_msg1[]={"Welcome to Astonia 2, my friend!\n"};
static char intro_msg2[]={"May your visit here be... interesting.\n"};

static char intro_msg3[]={"\n"};
static char intro_msg4[]={"Use #help (or /help) to get a listing of the text commands.\n"};
//static char intro_msg4[]={"WARNING: Lag scrolls will only work if used not later than four minutes after lagging out!\n"};

static inline unsigned int _mcmp(unsigned char *a,unsigned char *b,unsigned int len)
{
	// align a
	while (len>0 && ((int)(a)&3)) {
		if (*a!=*b) return 1;
		a++; b++; len--;
	}
	// compare as much as possible with 32 bit cmds
	while (len>3) {
		if (*(unsigned long *)a!=*(unsigned long *)b) return 1;
		a+=4; b+=4; len-=4;
	}
	// compare remaining 0-3 bytes
	while (len>0) {
		if (*a!=*b) return 1;
		a++; b++; len--;
	}
	return 0;
}

// some magic to avoid a lot of casts
static inline unsigned int mcmp(void *a,void *b,unsigned int len)
{
	return _mcmp(a,b,len);
}

static inline void *_fdiff(unsigned char *a,unsigned char *b,unsigned int len)
{
	// align a
	while (len>0 && ((int)(a)&3)) {
		if (*a!=*b) return a;
		a++; b++; len--;
	}
	// compare as much as possible with 32 bit cmds
	while (len>3) {
		if (*(unsigned long *)a!=*(unsigned long *)b) {
			while (*a==*b) { a++; b++; }
			return a;
		}
		a+=4; b+=4; len-=4;
	}
	// compare remaining 0-3 bytes
	while (len>0) {
		if (*a!=*b) return a;
		a++; b++; len--;
	}
	return 0;
}

// some magic to avoid a lot of casts
static inline void *fdiff(void *a,void *b,unsigned int len)
{
	return _fdiff(a,b,len);
}

static inline unsigned int _mcpy(unsigned char *a,unsigned char *b,unsigned int len)
{
	// align a
	while (len>0 && ((int)(a)&3)) {
		*a=*b;
		a++; b++; len--;
	}
	// compare as much as possible with 32 bit cmds
	while (len>3) {
		*(unsigned long *)a=*(unsigned long *)b;
		a+=4; b+=4; len-=4;
	}
	// compare remaining 0-3 bytes
	while (len>0) {
		*a=*b;
		a++; b++; len--;
	}
	return 0;
}

// some magic to avoid a lot of casts
static inline unsigned int mcpy(void *a,void *b,unsigned int len)
{
	return _mcpy(a,b,len);
}


static char secret[256]={"\
Ifhjf64hH8sa,-#39ddj843tvxcv0434dvsdc40G#34Trefc349534Y5#34trecerr943\
5#erZt#eA534#5erFtw#Trwec,9345mwrxm gerte-534lMIZDN(/dn8sfn8&DBDB/D&s\
8efnsd897)DDzD'D'D''Dofs,t0943-rg-gdfg-gdf.t,e95.34u.5retfrh.wretv.56\
9v4#asf.59m(D)/ND/DDLD;gd+dsa,fw9r,x  OD(98snfsfa"};

unsigned int xcrypt(unsigned int val)
{
        unsigned int res=0;

        res+=(unsigned int)(secret[ val     &255]);
        res+=(unsigned int)(secret[(val>>8 )&255])<<8;
        res+=(unsigned int)(secret[(val>>16)&255])<<16;
        res+=(unsigned int)(secret[(val>>24)&255])<<24;

        res^=0x5a7ce52e;

        return res;
}

void send_mod(int nr)
{
	int n;
	unsigned char buf[16];
	extern char mod[];

	for (n=0; n<8; n++) {
		buf[0]=SV_MOD1+n;
		memcpy(buf+1,mod+n*15,15);
		csend(nr,buf,16);
	}
}

void plr_challenge_newlogin(int nr)
{
        unsigned char buf[16];
        int tmp;

        tmp=RANDOM(0x3fffffff);
        if (tmp==0) tmp=42;

        player[nr].challenge=tmp;
        player[nr].state=ST_NEW_CHALLENGE;
        player[nr].lasttick=globs->ticker;

        buf[0]=SV_CHALLENGE;
        *(unsigned long *)(buf+1)=tmp;

        csend(nr,buf,16);

	send_mod(nr);
}

void plr_challenge_login(int nr)
{
        unsigned char buf[16];
        int tmp,cn;

	plog(nr,"challenge_login");

        tmp=RANDOM(0x3fffffff);
        if (tmp==0) tmp=42;

        player[nr].challenge=tmp;
        player[nr].state=ST_LOGIN_CHALLENGE;
        player[nr].lasttick=globs->ticker;

        buf[0]=SV_CHALLENGE;
        *(unsigned long *)(buf+1)=tmp;

        csend(nr,buf,16);

        cn=*(unsigned long*)(player[nr].inbuf+1);
        if (cn<1 || cn>=MAXCHARS){
		plog(nr,"sent wrong cn %d in challenge login",cn);
                plr_logout(0,nr,LO_CHALLENGE);
                return;
        }
        player[nr].usnr=cn;
        player[nr].pass1=*(unsigned long*)(player[nr].inbuf+5);
        player[nr].pass2=*(unsigned long*)(player[nr].inbuf+9);

	send_mod(nr);
}

void plr_challenge(int nr)
{
        unsigned int tmp;
	
        tmp=*(unsigned long*)(player[nr].inbuf+1);
        player[nr].version=*(unsigned long*)(player[nr].inbuf+5);
        player[nr].race=*(unsigned long*)(player[nr].inbuf+9);

        if (tmp!=xcrypt(player[nr].challenge)) {
                plog(nr,"Challenge failed");
                plr_logout(player[nr].usnr,nr,LO_CHALLENGE);
                return;
        }
        switch(player[nr].state) {
                case ST_NEW_CHALLENGE:          player[nr].state=ST_NEWLOGIN; player[nr].lasttick=globs->ticker; break;
                case ST_LOGIN_CHALLENGE:        player[nr].state=ST_LOGIN; player[nr].lasttick=globs->ticker; break;
                case ST_CHALLENGE:              player[nr].state=ST_NORMAL; player[nr].lasttick=globs->ticker; player[nr].ltick=0; break;
                default:                        plog(nr,"Challenge reply at unexpected state");
        }

        plog(nr,"Challenge ok");
}

void plr_perf_report(int nr)
{
        int ticksize,_idle,skip; //,cn;
//      float kbps;

        ticksize=*(unsigned short*)(player[nr].inbuf+1);
        skip=*(unsigned short*)(player[nr].inbuf+3);
        _idle=*(unsigned short*)(player[nr].inbuf+5);
//      kbps=*(float *)(player[nr].inbuf+7);

        player[nr].lasttick=globs->ticker;      // update timeout

//      plog(nr,"ticksize=%3d, %2d%% skip, %2d%% idle, %2.2fkBps",
//              ticksize,skip,_idle,kbps);

/*      cn=player[nr].usnr;
        if (cn) {
                chlog(cn,"HP=%d/%d End=%d/%d, Mana=%d/%d, Exp=%d/%d, TS=%d, SK=%d%% ID=%d%%",
                        ch[cn].a_hp/1000,ch[cn].hp[5],
                        ch[cn].a_end/1000,ch[cn].end[5],
                        ch[cn].a_mana/1000,ch[cn].mana[5],
                        ch[cn].points,ch[cn].points_tot,
                        ticksize,skip,_idle);
        }*/
}

void plr_unique(int nr)
{
    char buf[16];

    player[nr].unique=*(unsigned long long*)(player[nr].inbuf+1);
    plog(nr,"received unique %llX",player[nr].unique);

    if (!player[nr].unique) {
	globs->unique++;
	player[nr].unique=globs->unique;

	buf[0]=SV_UNIQUE;
	*(unsigned long long*)(buf+1)=player[nr].unique;
	xsend(nr,buf,9);

	plog(nr,"sent unique %llX",player[nr].unique);
    }
}

void plr_passwd(int nr)
{
	int n,hash;

	memcpy(player[nr].passwd,player[nr].inbuf+1,15); player[nr].passwd[15]=0;

	for (n=hash=0; n<15 && player[nr].passwd[n]; n++) {
		hash^=(player[nr].passwd[n]<<(n*2));
	}

        plog(nr,"Received passwd hash %u",hash);
}

void plr_cmd_move(int nr)
{
        int x,y;
        int cn;

        x=*(unsigned short*)(player[nr].inbuf+1);
        y=*(unsigned short*)(player[nr].inbuf+3);

        cn=player[nr].usnr;

        ch[cn].attack_cn=0;
        ch[cn].goto_x=x;
        ch[cn].goto_y=y;
        ch[cn].misc_action=0;
        ch[cn].cerrno=0;
        ch[cn].data[12]=globs->ticker;
}

void plr_cmd_reset(int nr)
{
        int cn;

        cn=player[nr].usnr;

        ch[cn].use_nr=0;
        ch[cn].skill_nr=0;
        ch[cn].attack_cn=0;
        ch[cn].goto_x=0;
        ch[cn].goto_y=0;
        ch[cn].misc_action=0;
        ch[cn].cerrno=0;
        ch[cn].data[12]=globs->ticker;
}

void plr_cmd_turn(int nr)
{
        int x,y;
        int cn;

        x=*(unsigned short*)(player[nr].inbuf+1);
        y=*(unsigned short*)(player[nr].inbuf+3);

        cn=player[nr].usnr;

        if (IS_BUILDING(cn)) {
                do_char_log(cn,3,"x=%d, y=%d, m=%d,light=%d, indoors=%lld, dlight=%d.\n",
                        x,y,x+y*MAPX,map[x+y*MAPX].light,map[x+y*MAPX].flags&MF_INDOORS,map[x+y*MAPX].dlight);

                do_char_log(cn,3,"ch=%d, to_ch=%d, it=%d.\n",
                        map[x+y*MAPX].ch,map[x+y*MAPX].to_ch,map[x+y*MAPX].it);
                do_char_log(cn,3,"flags=%04X %04X\n",
                            (unsigned int) (map[x+y*MAPX].flags>>32),
                            (unsigned int) (map[x+y*MAPX].flags&0xFFFF));

               	do_char_log(cn,3,"sprite=%d, fsprite=%d\n",
               		map[x+y*MAPX].sprite,map[x+y*MAPX].fsprite);
        }


        ch[cn].attack_cn=0;
        ch[cn].goto_x=0;
        ch[cn].misc_action=DR_TURN;
        ch[cn].misc_target1=x;
        ch[cn].misc_target2=y;
        ch[cn].cerrno=0;
        ch[cn].data[12]=globs->ticker;
}

// inventory manipulation... moves citem from/to item or worn - will be done at once
void plr_cmd_inv(int nr)
{
        int what,n,tmp,cn,in,co;

        cn=player[nr].usnr;

        what=*(unsigned long*)(player[nr].inbuf+1);
        n=*(unsigned long*)(player[nr].inbuf+5);
        co=*(unsigned long*)(player[nr].inbuf+9);
        if (co<1 || co>=MAXCHARS) co=0;

        if (what==0) { // normal inventory
                if (n<0 || n>39) return;        // sanity check
                if (ch[cn].stunned) return;

                tmp=ch[cn].item[n];

		if (IS_SANEITEM(tmp) && it[tmp].temp == IT_LAGSCROLL) return;

		do_update_char(cn);
                if (ch[cn].citem&0x80000000) {
                        tmp=ch[cn].citem&0x7fffffff;
                        if (tmp>0) ch[cn].gold+=tmp;
                        ch[cn].citem=0;
                        return;
                } else {
                        if (!IS_BUILDING(cn))
                                ch[cn].item[n]=ch[cn].citem;
                        else {
                                ch[cn].misc_action=DR_SINGLEBUILD;
                                do_char_log(cn,3,"Single mode\n");
                        }
                }
                ch[cn].citem=tmp;
                return;
        }
        if (what==1) { // big inventory
                if (ch[cn].stunned) return;

                do_swap_item(cn,n);
                return;
        }
        if (what==2) {
                if (ch[cn].stunned) return;

                if (ch[cn].citem) return;
                if (n>ch[cn].gold) return;
                if (n<=0) return;
                ch[cn].citem=0x80000000|n;
                ch[cn].gold-=n;

		do_update_char(cn);
                return;
        }
        if (what==5) {
                if (n<0 || n>19 || IS_BUILDING(cn)) return; // sanity check
                ch[cn].use_nr=n;
                ch[cn].skill_target1=co;
                return;
        }
        if (what==6) {
                if (n<0 || n>39 || IS_BUILDING(cn)) return; // sanity check
                ch[cn].use_nr=n+20;
                ch[cn].skill_target1=co;
                return;
        }
        if (what==7) {
                if (n<0 || n>19 || IS_BUILDING(cn)) return; // sanity check
                if ((in=ch[cn].worn[n])!=0) do_look_item(cn,in);
                return;
        }
        if (what==8) {
                if (n<0 || n>39 || IS_BUILDING(cn)) return; // sanity check
                if ((in=ch[cn].item[n])!=0) do_look_item(cn,in);
                return;
        }
        plog(nr,"Unknown CMD-INV-what %d",what);
}

void plr_cmd_inv_look(int nr)
{
        int n,cn,in;

        cn=player[nr].usnr;

        n=*(unsigned short*)(player[nr].inbuf+1);

        if (n<0 || n>39) return;        // sanity check
        if (IS_BUILDING(cn)) {
                ch[cn].citem=ch[cn].item[n];
                do_char_log(cn,3,"Area mode\n");
                ch[cn].misc_action=DR_AREABUILD1;
                return;
        }
        if ((in=ch[cn].item[n])!=0) do_look_item(cn,in);
}

void plr_cmd_mode(int nr)       // speed change, done at once
{
        int cn,mode;
        static char *speedname[3]={"Slow","Normal","Fast"};

        cn=player[nr].usnr;

        mode=*(unsigned short*)(player[nr].inbuf+1);
        if (mode<0 || mode>2) return;

        ch[cn].mode=(unsigned char)mode;

        do_update_char(cn);

        plog(nr,"Speed mode: %d (%s)",mode,speedname[mode]);
}

void plr_cmd_drop(int nr)
{
        int x,y,xs,ys,xe,ye;
        int cn;

        x=*(unsigned short*)(player[nr].inbuf+1);
        y=*(unsigned short*)(player[nr].inbuf+3);

        cn=player[nr].usnr;

        if (IS_BUILDING(cn)) {
                if (ch[cn].misc_action==DR_AREABUILD2) {

                        do_char_log(cn,3,"Areaend: %d,%d\n",x,y);
                        xs=x;
                        ys=y;
                        xe=ch[cn].misc_target1;
                        ye=ch[cn].misc_target2;
                        if (xs>xe) { x=xe; xe=xs; xs=x; }
                        if (ys>ye) { y=ye; ye=ys; ys=y; }
                        do_char_log(cn,3,"Area: %d,%d - %d,%d\n",
                                xs,ys,xe,ye);

                        for (x=xs; x<=xe; x++)
                                for (y=ys; y<=ye; y++)
                                        build_drop(x,y,ch[cn].citem);

                        ch[cn].misc_action=DR_AREABUILD1;
                } else if (ch[cn].misc_action==DR_AREABUILD1) {
                        ch[cn].misc_action=DR_AREABUILD2;
                        ch[cn].misc_target1=x;
                        ch[cn].misc_target2=y;
                        do_char_log(cn,3,"Areastart: %d,%d\n",x,y);
                } else if (ch[cn].misc_action==DR_SINGLEBUILD) {
                        build_drop(x,y,ch[cn].citem);
                }
                return;
        }

        ch[cn].attack_cn=0;
        ch[cn].goto_x=0;
        ch[cn].misc_action=DR_DROP;
        ch[cn].misc_target1=x;
        ch[cn].misc_target2=y;
        ch[cn].cerrno=0;
        ch[cn].data[12]=globs->ticker;
}

void plr_cmd_give(int nr)
{
        int cn,co;

        co=*(unsigned int*)(player[nr].inbuf+1);

        if (co<0 || co>=MAXCHARS) return;

        cn=player[nr].usnr;

        ch[cn].attack_cn=0;
        ch[cn].goto_x=0;
        ch[cn].misc_action=DR_GIVE;
        ch[cn].misc_target1=co;
        ch[cn].cerrno=0;
        ch[cn].data[12]=globs->ticker;
}

void plr_cmd_pickup(int nr)
{
        int x,y;
        int cn;

        x=*(unsigned short*)(player[nr].inbuf+1);
        y=*(unsigned short*)(player[nr].inbuf+3);

        cn=player[nr].usnr;

        if (IS_BUILDING(cn)) {
                build_remove(x,y);
                return;
        }

        ch[cn].attack_cn=0;
        ch[cn].goto_x=0;
        ch[cn].misc_action=DR_PICKUP;
        ch[cn].misc_target1=x;
        ch[cn].misc_target2=y;
        ch[cn].cerrno=0;
        ch[cn].data[12]=globs->ticker;
}

void plr_cmd_use(int nr)
{
        int x,y;
        int cn;

        cn=player[nr].usnr;

        x=*(unsigned short*)(player[nr].inbuf+1);
        y=*(unsigned short*)(player[nr].inbuf+3);

        ch[cn].attack_cn=0;
        ch[cn].goto_x=0;
        ch[cn].misc_action=DR_USE;
        ch[cn].misc_target1=x;
        ch[cn].misc_target2=y;
        ch[cn].cerrno=0;
        ch[cn].data[12]=globs->ticker;
}

void plr_cmd_attack(int nr)
{
        int cn,co;

        co=*(unsigned int*)(player[nr].inbuf+1);
        if (co<0 || co>=MAXCHARS) return;

        cn=player[nr].usnr;

        ch[cn].attack_cn=co;
        ch[cn].goto_x=0;
        ch[cn].misc_action=0;
        ch[cn].cerrno=0;
        ch[cn].data[12]=globs->ticker;

        plog(nr,"Trying to attack %s (%d)",ch[co].name,co);
        remember_pvp(cn,co);
}

void plr_cmd_look(int nr,int autoflag)
{
        int cn,co;

        co=*(unsigned short*)(player[nr].inbuf+1);

        cn=player[nr].usnr;

        if (co&0x8000) do_look_depot(cn,co&0x7fff);
        else do_look_char(cn,co,0,autoflag,0);
}

void plr_cmd_shop(int nr)
{
        int cn,co,n;

        co=*(unsigned short*)(player[nr].inbuf+1);

        n=*(unsigned short*)(player[nr].inbuf+3);

        cn=player[nr].usnr;

        if (co&0x8000) do_depot_char(cn,co&0x7fff,n);
        else do_shop_char(cn,co,n);
}

void plr_cmd_look_item(int nr)
{
        int cn,in,x,y;

        x=*(unsigned short*)(player[nr].inbuf+1);
        y=*(unsigned short*)(player[nr].inbuf+3);

        if (x<0 || x>=MAPX || y<0 || y>=MAPY) return;

        in=map[x+y*MAPX].it;

        cn=player[nr].usnr;

        do_look_item(cn,in);
}

void plr_cmd_stat(int nr)
{
        int n,v,cn;

        cn=player[nr].usnr;

        n=*(unsigned short*)(player[nr].inbuf+1);
        v=*(unsigned short*)(player[nr].inbuf+3);

        if (n<0 || n>107) return;
        if (v<0 || v>99) return;        // sanity checks

        if (n<5) while (v--) do_raise_attrib(cn,n);
        else if (n==5) while (v--) do_raise_hp(cn);
        else if (n==6) while (v--) do_raise_end(cn);
        else if (n==7) while (v--) do_raise_mana(cn);
        else while (v--) do_raise_skill(cn,n-8);

        do_update_char(cn);
}

void plr_cmd_skill(int nr)
{
        int n,cn,co;

        cn=player[nr].usnr;

        n=*(unsigned long*)(player[nr].inbuf+1);
        co=*(unsigned long*)(player[nr].inbuf+5);

        if (n<0 || n>99) return;        // sanity checks
        if (co<0 || co>=MAXCHARS) return;
        if (!ch[cn].skill[n][0]) return;

        ch[cn].skill_nr=n;
        ch[cn].skill_target1=co;
}

void plr_cmd_input1(int nr)
{
        int n;

        for (n=0; n<15; n++)
                player[nr].input[n]=player[nr].inbuf[n+1];
}

void plr_cmd_input2(int nr)
{
        int n;

        for (n=0; n<15; n++)
                player[nr].input[n+15]=player[nr].inbuf[n+1];
}

void plr_cmd_input3(int nr)
{
        int n;

        for (n=0; n<15; n++)
                player[nr].input[n+30]=player[nr].inbuf[n+1];
}

void plr_cmd_input4(int nr)
{
        int n;

        for (n=0; n<15; n++)
                player[nr].input[n+45]=player[nr].inbuf[n+1];
}

void plr_cmd_input5(int nr)
{
        int n;

        for (n=0; n<15; n++)
                player[nr].input[n+60]=player[nr].inbuf[n+1];
}

void plr_cmd_input6(int nr)
{
        int n;

        for (n=0; n<15; n++)
                player[nr].input[n+75]=player[nr].inbuf[n+1];
}

void plr_cmd_input7(int nr)
{
        int n;

        for (n=0; n<15; n++)
                player[nr].input[n+90]=player[nr].inbuf[n+1];
}

void plr_cmd_input8(int nr)
{
        int n;

        for (n=0; n<15; n++)
                player[nr].input[n+105]=player[nr].inbuf[n+1];

        player[nr].input[105+14]=0;

        do_say(player[nr].usnr,player[nr].input);
}

void plr_cmd_setuser(int nr)
{
        int n,cn,pos,flag;
        char *reason = NULL;
		char *race_name;

        cn=player[nr].usnr;

        pos=player[nr].inbuf[2];
        if (pos<0 || pos>65) return;

        switch(player[nr].inbuf[1]) {
                case    0:
                        for (n=0; n<13; n++)
                                ch[cn].text[0][n+pos]=player[nr].inbuf[n+3];
                        break;
                case    1:
                        for (n=0; n<13; n++)
                                ch[cn].text[1][n+pos]=player[nr].inbuf[n+3];
                        break;
                case    2:
                        for (n=0; n<13; n++)
                                ch[cn].text[2][n+pos]=player[nr].inbuf[n+3];
                        if (pos!=65) break;
                        if (strlen(ch[cn].text[0])>3 &&
                            strlen(ch[cn].text[0])<38 &&
                                (ch[cn].flags&CF_NEWUSER)) {

                                flag=0;

                                for (n=0; ch[cn].text[0][n]; n++) {
                                        if (!isalpha(ch[cn].text[0][n])) { flag=1; break; }
                                        ch[cn].text[0][n]=tolower(ch[cn].text[0][n]);
                                }
                                if (flag==1) {
                                        reason = "contains non-letters. Please choose a more normal-looking name.";
                                }

                                // check for bad names here when the string is all lowercase
                                if (god_is_badname(ch[cn].text[0])) flag=3;

                                ch[cn].text[0][0]=toupper(ch[cn].text[0][0]);

                                /* CS, 991030: Reserve "Self" for self-reference. */
                                if (!strcmp(ch[cn].text[0],"Self")) flag = 2;
                                for (n=1; !flag && n<MAXCHARS; n++)
                                        if (ch[n].used!=USE_EMPTY && strcmp(ch[cn].text[0],ch[n].name)==0) { flag=2; break; }
                                /* CS, 000301: Check for names of mobs in templates */
                                for (n=1; !flag && n<MAXTCHARS; n++)
                                        if (!strcmp(ch[cn].text[0],ch_temp[n].name)) { flag=2; break; }
                                if (flag==2) {
                                        reason = "is already in use. Please try to choose another name.";
                                }
                                if (flag==3) {
                                        reason = "is deemed inapropriate. Please try to choose another name.";
                                }

                                if (flag) {
                                        do_char_log(cn,0,"The name \"%s\" you have chosen for your character %s\n",
                                                    ch[cn].text[0], reason);
                                } else {
                                        strcpy(ch[cn].name,ch[cn].text[0]);
                                        strcpy(ch[cn].reference,ch[cn].text[0]);
                                        ch[cn].flags&=~CF_NEWUSER;
                                }
                            }
                          strcpy(ch[cn].description,ch[cn].text[1]);
                          if (strlen(ch[cn].description)>77) strcat(ch[cn].description,ch[cn].text[2]);
                          reason = NULL;
                          if (strlen(ch[cn].description)<10) {
                                  reason = "is too short";
                          } else if (strstr(ch[cn].description,ch[cn].name)==NULL) {
                                  reason = "does not contain your name";
                          } else if (strchr(ch[cn].description,'\"')) {
                                  reason = "contains a double quote";
                          } else if (ch[cn].flags&CF_NODESC) {
                                  reason = "was blocked because you have been known to enter inappropriate descriptions";
                          }
                          if (reason != NULL) {
				if (ch[cn].kindred&KIN_TEMPLAR) race_name = "a Templar";
				else if (ch[cn].kindred&KIN_HARAKIM) race_name = "a Harakim";
				else if (ch[cn].kindred&KIN_MERCENARY) race_name = "a Mercenary";
				else if (ch[cn].kindred&KIN_SEYAN_DU) race_name = "a Seyan'Du";
				else if (ch[cn].kindred&KIN_ARCHHARAKIM) race_name = "an Arch Harakim";
				else if (ch[cn].kindred&KIN_ARCHTEMPLAR) race_name = "an Arch Templar";
				else if (ch[cn].kindred&KIN_WARRIOR) race_name = "a Warrior";
				else if (ch[cn].kindred&KIN_SORCERER) race_name = "a Sorcerer";
				else race_name = "a strange figure";
                                do_char_log(cn,0,"The description you entered for your character %s, so it has been rejected.\n", reason);
				sprintf(ch[cn].description,"%s is %s. %s looks somewhat nondescript.", ch[cn].name, race_name, HE_SHE_CAPITAL(cn));
                          }

                          do_char_log(cn,1,"Account data received.\n");
                          plog(nr,"Account data received");
			  do_update_char(cn);
                        break;
                default:
                        plog(nr,"Unknown setuser subtype %d",player[nr].inbuf[1]);
                        break;
        }
}

void plr_idle(int nr)
{
	if (globs->ticker-player[nr].lasttick>TICKS*60) {
                plog(nr,"Idle too long (protocol level)");
                plr_logout(player[nr].usnr,nr,LO_IDLE);
        }
        if (player[nr].state==ST_EXIT) return;

        if (globs->ticker-player[nr].lasttick2>TICKS*60*15) {
                plog(nr,"Idle too long (player level)");
                plr_logout(player[nr].usnr,nr,LO_IDLE);
        }
}

void plr_cmd_exit(int nr)
{
        plog(nr,"Pressed F12");
        plr_logout(player[nr].usnr,nr,LO_EXIT);
}

void plr_cmd_ctick(int nr)
{
        player[nr].rtick=*(unsigned long*)(player[nr].inbuf+1);
	player[nr].lasttick=globs->ticker;
}

static unsigned int clcmd[255];

void cl_list(void)
{
	int n,m=0,tot=0;

	for (n=0; n<256; n++) {
		tot+=clcmd[n];
		if (clcmd[n]>m) m=clcmd[n];
	}

	for (n=0; n<256; n++) {		
		if (clcmd[n]>m/16)
			xlog("cl type %2d: %5d (%.2f%%)",n,clcmd[n],100.0/tot*clcmd[n]);
	}
}

// dispatch command.
void plr_cmd(int nr)
{
        int cn;

	clcmd[player[nr].inbuf[0]]++;

        switch(player[nr].inbuf[0]) {
            case CL_NEWLOGIN:       	plr_challenge_newlogin(nr); break;
            case CL_CHALLENGE:      	plr_challenge(nr); break;
	    case CL_LOGIN:          	plr_challenge_login(nr); break;
	    case CL_CMD_UNIQUE:		plr_unique(nr); return;
	    case CL_PASSWD:		plr_passwd(nr); break;
        }
        if (player[nr].state!=ST_NORMAL) return;

        if (player[nr].inbuf[0]!=CL_CMD_AUTOLOOK &&
            player[nr].inbuf[0]!=CL_PERF_REPORT &&
            player[nr].inbuf[0]!=CL_CMD_CTICK)
                player[nr].lasttick2=globs->ticker;

        switch(player[nr].inbuf[0]) {
                case CL_PERF_REPORT:    plr_perf_report(nr); return;
                case CL_CMD_LOOK:       plr_cmd_look(nr,0); return;
                case CL_CMD_AUTOLOOK:   plr_cmd_look(nr,1); return;
                case CL_CMD_SETUSER:    plr_cmd_setuser(nr); return;
                case CL_CMD_STAT:       plr_cmd_stat(nr); return;
                case CL_CMD_INPUT1:     plr_cmd_input1(nr); return;
                case CL_CMD_INPUT2:     plr_cmd_input2(nr); return;
                case CL_CMD_INPUT3:     plr_cmd_input3(nr); return;
                case CL_CMD_INPUT4:     plr_cmd_input4(nr); return;
                case CL_CMD_INPUT5:     plr_cmd_input5(nr); return;
                case CL_CMD_INPUT6:     plr_cmd_input6(nr); return;
                case CL_CMD_INPUT7:     plr_cmd_input7(nr); return;
                case CL_CMD_INPUT8:     plr_cmd_input8(nr); return;
                case CL_CMD_CTICK:      plr_cmd_ctick(nr); return;
        }

        cn=player[nr].usnr;
        if (ch[cn].stunned) {
                do_char_log(cn,2,"You have been stunned. You cannot move.\n");
        }

        switch(player[nr].inbuf[0]) {
                case CL_CMD_LOOK_ITEM:  plr_cmd_look_item(nr); return;
                case CL_CMD_GIVE:       plr_cmd_give(nr); return;
                case CL_CMD_TURN:       plr_cmd_turn(nr); return;
                case CL_CMD_DROP:       plr_cmd_drop(nr); return;
                case CL_CMD_PICKUP:     plr_cmd_pickup(nr); return;
                case CL_CMD_ATTACK:     plr_cmd_attack(nr); return;
                case CL_CMD_MODE:       plr_cmd_mode(nr); return;
                case CL_CMD_MOVE:       plr_cmd_move(nr); return;
                case CL_CMD_RESET:      plr_cmd_reset(nr); return;
                case CL_CMD_SKILL:      plr_cmd_skill(nr); return;
                case CL_CMD_INV_LOOK:   plr_cmd_inv_look(nr); return;
                case CL_CMD_USE:        plr_cmd_use(nr); return;
                case CL_CMD_INV:        plr_cmd_inv(nr); return;
                case CL_CMD_EXIT:       plr_cmd_exit(nr); return;
        }

        if (ch[cn].stunned) return;

        switch(player[nr].inbuf[0]) {

                case CL_CMD_SHOP:       plr_cmd_shop(nr); break;
                default:                plog(nr,"Unknown CL: %d",player[nr].inbuf[0]); break;
        }
}

void char_add_net(int cn,unsigned int net)
{
    int n,m;

    for (n=80; n<89; n++)
        if ((ch[cn].data[n]&0x00ffffff)==(net&0x00ffffff)) break;	

    for (m=n; m>80; m--)
	ch[cn].data[m]=ch[cn].data[m-1];

    ch[cn].data[80]=net;
}

void plr_newlogin(int nr)
{
        int cn,temp,tmp;
        unsigned char buf[16];

        if (player[nr].version<MINVERSION) {
                plog(nr,"Client too old (%X). Logout demanded",player[nr].version);
                plr_logout(0,nr,LO_VERSION);
                return;
        }
        if (god_is_banned(player[nr].addr)) {
                plog(nr,"Banned, sent away");
                plr_logout(0,nr,LO_KICKED);
                return;
        }
	if ((tmp=cap(0,nr))) {
		plog(nr,"Reached player cap, returned queue place %d, prio=%d",tmp,player[nr].prio);
		
		buf[0]=SV_CAP;
		*(unsigned int*)(buf+1)=tmp;
		*(unsigned int*)(buf+5)=player[nr].prio;
		csend(nr,buf,16);

		player[nr].state=ST_NEWCAP;
		player[nr].lasttick=globs->ticker;
		player[nr].lasttick2=globs->ticker;
		return;
	}
        temp=player[nr].race;
        if (temp!=2 && temp!=3 && temp!=4 && temp!=76 && temp!=77 && temp!=78) temp=2;

        cn=god_create_char(temp,1);
        ch[cn].player=nr;

        ch[cn].temple_x=ch[cn].tavern_x=HOME_MERCENARY_X;
        ch[cn].temple_y=ch[cn].tavern_y=HOME_MERCENARY_Y;

        ch[cn].points=0;
        ch[cn].points_tot=0;
        ch[cn].luck=205;

        globs->players_created++;

        if (!god_drop_char_fuzzy_large(cn,ch[cn].temple_x,ch[cn].temple_y,ch[cn].temple_x,ch[cn].temple_y))
                if (!god_drop_char_fuzzy_large(cn,ch[cn].temple_x+3,ch[cn].temple_y,ch[cn].temple_x,ch[cn].temple_y))
                        if (!god_drop_char_fuzzy_large(cn,ch[cn].temple_x,ch[cn].temple_y+3,ch[cn].temple_x,ch[cn].temple_y)) {
                                plog(nr,"plr_newlogin(): could not drop new character");
                                plr_logout(cn,nr,LO_NOROOM);
                                ch[cn].used=0;
                                return;
                        }

        ch[cn].creation_date=time(NULL);
        ch[cn].login_date=time(NULL);
        ch[cn].flags|=CF_NEWUSER|CF_PLAYER;
        ch[cn].addr=player[nr].addr;
	char_add_net(cn,ch[cn].addr);
        ch[cn].mode=1;
        do_update_char(cn);

        player[nr].usnr=cn;
        player[nr].pass1=ch[cn].pass1;
        player[nr].pass2=ch[cn].pass2;

        buf[0]=SV_NEWPLAYER;
        *(unsigned long*)(buf+1)=cn;
        *(unsigned long*)(buf+5)=ch[cn].pass1;
        *(unsigned long*)(buf+9)=ch[cn].pass2;
        *(unsigned char*)(buf+13)=VERSION&255;
        *(unsigned char*)(buf+14)=(VERSION>>8)&255;
        *(unsigned char*)(buf+15)=VERSION>>16;
        csend(nr,buf,16);

        player[nr].state=ST_NORMAL;
        player[nr].lasttick=globs->ticker;
        player[nr].ltick=0;
        player[nr].ticker_started=1;

        buf[0]=SV_TICK;
        *(unsigned char*)(buf+1)=(unsigned char)ctick;
        xsend(nr,buf,2);

        plog(nr,"Created new character");

        do_char_log(cn,1,intro_msg1);
        do_char_log(cn,1," \n");
        do_char_log(cn,1,intro_msg2);
        do_char_log(cn,1," \n");
        do_char_log(cn,1,intro_msg3);
        do_char_log(cn,1," \n");
        do_char_log(cn,1,intro_msg4);
        do_char_log(cn,1," \n");

	if (player[nr].passwd[0] && !(ch[cn].flags&CF_PASSWD)) {
		god_change_pass(cn,cn,player[nr].passwd);
	}

        // do_staff_log(2,"New player %s entered the game!\n",ch[cn].name);
        do_announce(cn, 0, "A new player has entered the game.\n", ch[cn].name);	
}


void plr_login(int nr)
{
        int cn,tmp,in,n;
        unsigned char buf[16];

        if (player[nr].version<MINVERSION) {
                plog(nr,"Client too old (%X). Logout demanded",player[nr].version);
                plr_logout(0,nr,LO_VERSION);
                return;
        }

        cn=player[nr].usnr;

        if (cn<=0 || cn>=MAXCHARS) {
                plog(nr,"Login as %d denied (illegal cn)",cn);
                plr_logout(0,nr,LO_PARAMS);
                return;
        }

        if (ch[cn].pass1!=player[nr].pass1 || ch[cn].pass2!=ch[cn].pass2) {
                plog(nr,"Login as %s denied (pass1/pass2)",ch[cn].name);
                plr_logout(0,nr,LO_PASSWORD);
                return;
        }
	if ((ch[cn].flags&CF_PASSWD) && strcmp(ch[cn].passwd,player[nr].passwd)) {
		plog(nr,"Login as %s denied (password)",ch[cn].name);
                plr_logout(0,nr,LO_PASSWORD);
                return;
	}

	if (ch[cn].used==USE_EMPTY) {
                plog(nr,"Login as %s denied (deleted)",ch[cn].name);
                plr_logout(0,nr,LO_PASSWORD);
		return;
        }

	if (ch[cn].used!=USE_NONACTIVE && !(ch[cn].flags&CF_CCP)) {
                plog(nr,"Login as %s who is already active",ch[cn].name);
                plr_logout(cn,ch[cn].player,LO_IDLE);
        }

        if (ch[cn].flags&CF_KICKED) {
                plog(nr,"Login as %s denied (kicked)",ch[cn].name);
                plr_logout(0,nr,LO_KICKED);
                return;
        }

	if (!(ch[cn].flags&(CF_GOLDEN|CF_GOD)) && god_is_banned(player[nr].addr)) {
                chlog(cn,"Banned, sent away");
                plr_logout(0,nr,LO_KICKED);
                return;
        }

        if ((tmp=cap(cn,nr))) {
		chlog(cn,"Reached player cap, returned queue place %d, prio=%d",tmp,player[nr].prio);
		
		buf[0]=SV_CAP;
		*(unsigned int*)(buf+1)=tmp;
		*(unsigned int*)(buf+5)=player[nr].prio;
		csend(nr,buf,16);

		player[nr].state=ST_CAP;
		player[nr].lasttick=globs->ticker;
		player[nr].lasttick2=globs->ticker;
		return;
	}

        ch[cn].player=nr;

        if (!(ch[cn].flags&CF_CCP) && (ch[cn].flags&CF_GOD)) ch[cn].flags|=CF_INVISIBLE;

        player[nr].state=ST_NORMAL;
        player[nr].lasttick=globs->ticker;
        player[nr].ltick=0;
        player[nr].ticker_started=1;

        buf[0]=SV_LOGIN_OK;
        *(unsigned long*)(buf+1)=VERSION;
        csend(nr,buf,16);

        buf[0]=SV_TICK;
        *(unsigned char*)(buf+1)=(unsigned char)ctick;
        xsend(nr,buf,2);

	ch[cn].used=USE_ACTIVE;
        ch[cn].login_date=time(NULL);
        ch[cn].addr=player[nr].addr;
	char_add_net(cn,ch[cn].addr);
        ch[cn].current_online_time=0;

        player[nr].cpl.mode = -1; // "impossible" client default will force change

        if (!(ch[cn].flags&CF_CCP)) {
                if (!god_drop_char_fuzzy_large(cn,ch[cn].tavern_x,ch[cn].tavern_y,ch[cn].tavern_x,ch[cn].tavern_y))
                        if (!god_drop_char_fuzzy_large(cn,ch[cn].tavern_x+3,ch[cn].tavern_y,ch[cn].tavern_x,ch[cn].tavern_y))
                                if (!god_drop_char_fuzzy_large(cn,ch[cn].tavern_x,ch[cn].tavern_y+3,ch[cn].tavern_x,ch[cn].tavern_y)) {
                                        plog(nr,"plr_login(): could not drop new character");
                                        plr_logout(cn,nr,LO_NOROOM);
                                        return;
                                }
        }

	for (n=0; n<20; n++) {
		if ((in=ch[cn].spell[n])) {
                        if (it[in].temp==SK_RECALL) {
				it[in].used=0;
				ch[cn].spell[n]=0;
				chlog(cn,"CHEATER: removed active teleport");
			}
		}
	}

        do_update_char(cn);
        ch[cn].tavern_x=ch[cn].temple_x;
        ch[cn].tavern_y=ch[cn].temple_y;

        plog(nr,"Login successful");

        do_char_log(cn,1,intro_msg1);
        do_char_log(cn,1," \n");
        do_char_log(cn,1,intro_msg2);
        do_char_log(cn,1," \n");
        do_char_log(cn,1,intro_msg3);
        do_char_log(cn,1," \n");
        do_char_log(cn,1,intro_msg4);
        do_char_log(cn,1," \n");

	if (player[nr].passwd[0] && !(ch[cn].flags&CF_PASSWD)) {
		god_change_pass(cn,cn,player[nr].passwd);
	}

        if (!(ch[cn].flags&CF_CCP) && (ch[cn].flags&CF_GOD)) do_char_log(cn,0,"Remember, you are invisible!\n");

        do_announce(cn, 0, "%s entered the game.\n", ch[cn].name);
}

void plr_logout(int cn,int nr,int reason)
{
        int n,in,co;
        unsigned char buf[16];

        if (nr<0 || nr>=MAXPLAYER) nr=0;

        if (cn>0 && cn<MAXCHARS && (ch[cn].player==nr || nr==0) && (ch[cn].flags&CF_USURP)) {
                ch[cn].flags&=~(CF_CCP|CF_USURP|CF_STAFF|CF_IMMORTAL|CF_GOD|CF_CREATOR);
                co=ch[cn].data[97];
                plr_logout(co,0,LO_SHUTDOWN);
        }

        if (cn>0 && cn<MAXCHARS && (ch[cn].player==nr || nr==0) && (ch[cn].flags&(CF_PLAYER)) && !(ch[cn].flags&CF_CCP)) {

                if (reason==LO_EXIT) {
                        chlog(cn,"Punished for leaving the game by means of F12");

                        do_char_log(cn,0," \n");
                        do_char_log(cn,0,"You are being punished for leaving the game without entering a tavern:\n");
                        do_char_log(cn,0," \n");
                        do_char_log(cn,0,"You have been hit by a demon. You lost %d HP.\n",ch[cn].hp[5]*8/10);
                        ch[cn].a_hp-=ch[cn].hp[5]*800;
                        if (ch[cn].a_hp<500) {
                                do_char_log(cn,0,"The demon killed you.\n");
                                do_char_log(cn,0," \n");
                                do_char_killed(0,cn);
                        } else {
                                do_char_log(cn,0," \n");
                                if (ch[cn].gold/10>0) do_char_log(cn,0,"A demon grabs your purse and removes %dG, %dS.\n",(ch[cn].gold/10)/100,(ch[cn].gold/10)%100);
                                ch[cn].gold-=ch[cn].gold/10;
                                if ((in=ch[cn].citem) && (in&0x80000000)) {
                                	do_char_log(cn,0,"The demon also takes the money in your hand.\n");
                                	ch[cn].citem=0;
                                }
                        }
                        do_area_log(cn,0,ch[cn].x,ch[cn].y,2,"%s left the game without saying goodbye and was punished by the gods.\n",ch[cn].name);
                }

                if (map[ch[cn].x+ch[cn].y*MAPX].ch==cn) {
                        map[ch[cn].x+ch[cn].y*MAPX].ch=0;
                        if (ch[cn].light) do_add_light(ch[cn].x,ch[cn].y,-ch[cn].light);
                }
                if (map[ch[cn].tox+ch[cn].toy*MAPX].to_ch==cn) map[ch[cn].tox+ch[cn].toy*MAPX].to_ch=0;
                remove_enemy(cn);

                if (reason==LO_IDLE || reason==LO_SHUTDOWN || reason==0) { // give lag scroll to player
                        if (abs(ch[cn].x-ch[cn].temple_x)+abs(ch[cn].y-ch[cn].temple_y)>10 && !(map[ch[cn].x+ch[cn].y*MAPX].flags&MF_NOLAG)) {
                                in=god_create_item(IT_LAGSCROLL);
                                it[in].data[0]=ch[cn].x;
                                it[in].data[1]=ch[cn].y;
				it[in].data[2]=globs->ticker;
                                if (in) god_give_char(in,cn);
                        }
                }

                ch[cn].x=ch[cn].y=ch[cn].tox=ch[cn].toy=ch[cn].frx=ch[cn].fry=0;

                ch[cn].player=0;
                ch[cn].status=0;
                ch[cn].status2=0;
                ch[cn].dir=1;
                ch[cn].escape_timer=0;
                for (n=0; n<4; n++) ch[cn].enemy[n]=0;
                ch[cn].attack_cn=0;
                ch[cn].skill_nr=0;
                ch[cn].goto_x=0;
                ch[cn].use_nr=0;
                ch[cn].misc_action=0;
                ch[cn].stunned=0;
                ch[cn].retry=0;
                for (n=0; n<13; n++) { // reset afk, group and follow
                        if (n==11) continue; // leave fightback set
                        ch[cn].data[n]=0;
                }
                ch[cn].data[96]=0;

                ch[cn].used=USE_NONACTIVE;
                ch[cn].logout_date=time(NULL);

		ch[cn].flags|=CF_SAVEME;

                if (IS_BUILDING(cn)) god_build(cn,0);

                do_announce(cn, 0, "%s left the game.\n", ch[cn].name);
        }

        if (nr && reason && reason!=LO_USURP) {
                buf[0]=SV_EXIT;
                *(unsigned int*)(buf+1)=reason;
                if (player[nr].state==ST_NORMAL) xsend(nr,buf,16);
		else csend(nr,buf,16);

                player_exit(nr);
        }
}

void plr_state(int nr)
{
        if (globs->ticker-player[nr].lasttick>TICKS*15 && player[nr].state==ST_EXIT) {
                close(player[nr].sock);
                plog(nr,"Connection closed (ST_EXIT)");
                player[nr].sock=0;
		deflateEnd(&player[nr].zs);
        }

        if (globs->ticker-player[nr].lasttick>TICKS*60) {
                plog(nr,"Idle timeout");
                plr_logout(0,nr,LO_IDLE);
        }

        switch(player[nr].state) {
                case    ST_NEWLOGIN:            plr_newlogin(nr); break;
		case    ST_LOGIN:               plr_login(nr); break;

		case 	ST_NEWCAP:		if (globs->ticker-player[nr].lasttick>TICKS*10) player[nr].state=ST_NEWLOGIN; break;
		case 	ST_CAP:		        if (globs->ticker-player[nr].lasttick>TICKS*10) player[nr].state=ST_LOGIN; break;

                case    ST_NEW_CHALLENGE:
                case    ST_LOGIN_CHALLENGE:
                case    ST_CONNECT:             break;
                case    ST_EXIT:                break;

                default:                        plog(nr,"UNKNOWN ST: %d",player[nr].state); break;
        }
}

void inline plr_change_stat(int nr,unsigned char *a,unsigned char *b,unsigned char code,unsigned char n)
{
        unsigned char buf[16];

        //if (a[0]!=b[0] || a[1]!=b[1] || a[2]!=b[2] || a[3]!=b[3] || a[4]!=b[4] || a[5]!=b[5]) {
        if (*(unsigned long*)a!=*(unsigned long*)b ||
            *(unsigned short*)(a+4)!=*(unsigned short*)(b+4)) {
                buf[0]=code;
                buf[1]=n;

                buf[2]=b[0];
                buf[3]=b[1];
                buf[4]=b[2];
                buf[5]=b[3];
                buf[6]=b[4];
                buf[7]=b[5];

                xsend(nr,buf,8);
                mcpy(a,b,6);
        }
}

void inline plr_change_power(int nr,unsigned short *a,unsigned short *b,unsigned char code)
{
        unsigned char buf[16];

        if (mcmp(a,b,12)) {
                buf[0]=code;
                *(unsigned short*)(buf+1)=b[0];
                *(unsigned short*)(buf+3)=b[1];
                *(unsigned short*)(buf+5)=b[2];
                *(unsigned short*)(buf+7)=b[3];
                *(unsigned short*)(buf+9)=b[4];
                *(unsigned short*)(buf+11)=b[5];

                xsend(nr,buf,13);
                mcpy(a,b,12);
        }
}

inline int ch_base_status(int n)
{
        if (n<4) return n;

        if (n<16) return n;     // unassigned!!

        if (n<24) return 16;    // walk up
        if (n<32) return 24;    // walk down
        if (n<40) return 32;    // walk left
        if (n<48) return 40;    // walk right

        if (n<60) return 48;    // walk left+up
        if (n<72) return 60;    // walk left+down
        if (n<84) return 72;    // walk right+up
        if (n<96) return 84;    // walk right+down

        if (n<100) return 96;
        if (n<104) return 100;  // turn up to left
        if (n<108) return 104;  // turn up to right
        if (n<112) return 108;
        if (n<116) return 112;
        if (n<120) return 116;  // turn down to left
        if (n<124) return 120;
        if (n<128) return 124;  // turn down to right
        if (n<132) return 128;
        if (n<136) return 132;  // turn left to up
        if (n<140) return 136;
        if (n<144) return 140;  // turn left to down
        if (n<148) return 144;
        if (n<152) return 148;  // turn right to up
        if (n<156) return 152;
        if (n<160) return 160;  // turn right to down
        if (n<164) return 160;

        if (n<168) return 160;  // attack up
        if (n<176) return 168;  // attack down
        if (n<184) return 176;  // attack left
        if (n<192) return 184;  // attack right

        if (n<200) return 192;  // misc up
        if (n<208) return 200;  // misc down
        if (n<216) return 208;  // misc left
        if (n<224) return 216;  // misc right

        // remainder is unassigned

        return n;
}

static inline int it_base_status(int n)
{
        if (n==0) return 0;
        if (n==1) return 1;

        if (n<6) return 2;
        if (n<8) return 6;
        if (n<16) return 8;
        if (n<21) return 16;

        return n;
}

int cl_light_26(int n,int dosend,struct cmap *cmap,struct cmap *smap)
{
	char buf[16],p;
	int m,l=0;

	if (!dosend) {	
		for (m=n; m<n+27 && m<TILEX*TILEY; m++) {
			if (cmap[m].light!=smap[m].light) l++;
		}
		return 50*l/16;
	} else {

		buf[0]=SV_SETMAP3;
                *(unsigned short*)(buf+1)=(unsigned short)(n|((unsigned short)(smap[n].light)<<12));
		cmap[n].light=smap[n].light;

		for (m=n+2,p=3; m<min(n+26+2,TILEX*TILEY); m+=2,p++) {
			*(unsigned char*)(buf+p)=smap[m].light|(smap[m-1].light<<4);
			cmap[m].light=smap[m].light;
			cmap[m-1].light=smap[m-1].light;
		}
		xsend(dosend,buf,16);
		return 1;
	}
}
int cl_light_one(int n,int dosend,struct cmap *cmap,struct cmap *smap)
{
	char buf[16];

	if (!dosend) {	
		return 50*1/3;
	} else {

		buf[0]=SV_SETMAP4;
                *(unsigned short*)(buf+1)=(unsigned short)(n|((unsigned short)(smap[n].light)<<12));
		cmap[n].light=smap[n].light;
		xsend(dosend,buf,3);
		return 1;
	}
}
int cl_light_three(int n,int dosend,struct cmap *cmap,struct cmap *smap)
{
	char buf[16],p;
	int m,l=0;

	if (!dosend) {	
		for (m=n; m<n+3 && m<TILEX*TILEY; m++) {
			if (cmap[m].light!=smap[m].light) l++;
		}
		return 50*l/4;
	} else {

		buf[0]=SV_SETMAP5;
                *(unsigned short*)(buf+1)=(unsigned short)(n|((unsigned short)(smap[n].light)<<12));
		cmap[n].light=smap[n].light;

		for (m=n+2,p=3; m<min(n+2+2,TILEX*TILEY); m+=2,p++) {
			*(unsigned char*)(buf+p)=smap[m].light|(smap[m-1].light<<4);
			cmap[m].light=smap[m].light;
			cmap[m-1].light=smap[m-1].light;
		}
		xsend(dosend,buf,4);
		return 1;
	}
	return 0;
}
int cl_light_seven(int n,int dosend,struct cmap *cmap,struct cmap *smap)
{
	char buf[16],p;
	int m,l=0;

	if (!dosend) {	
		for (m=n; m<n+7 && m<TILEX*TILEY; m++) {
			if (cmap[m].light!=smap[m].light) l++;
		}
		return 50*l/6;
	} else {

		buf[0]=SV_SETMAP6;
                *(unsigned short*)(buf+1)=(unsigned short)(n|((unsigned short)(smap[n].light)<<12));
		cmap[n].light=smap[n].light;

		for (m=n+2,p=3; m<min(n+6+2,TILEX*TILEY); m+=2,p++) {
			*(unsigned char*)(buf+p)=smap[m].light|(smap[m-1].light<<4);
			cmap[m].light=smap[m].light;
			cmap[m-1].light=smap[m-1].light;
		}
		xsend(dosend,buf,6);
		return 1;
	}
}

// check for any changed values in player data and send them to the client
void plr_change(int nr)
{
        int cn,n,in,p,lastn=-1;
        struct cplayer *cpl;
        struct cmap *cmap;
        struct cmap *smap;
        unsigned char buf[256];

        cn=player[nr].usnr;
        cpl=&player[nr].cpl;
        cmap=player[nr].cmap;
        smap=player[nr].smap;

	if ((ch[cn].flags&CF_UPDATE) || (cn&15)==(globs->ticker&15)) {
                // cplayer - stats
		if (strcmp(cpl->name,ch[cn].name)) {
			buf[0]=SV_SETCHAR_NAME1;
			mcpy(buf+1,ch[cn].name,15);
			xsend(nr,buf,16);
			buf[0]=SV_SETCHAR_NAME2;
			mcpy(buf+1,ch[cn].name+15,15);
			xsend(nr,buf,16);
			buf[0]=SV_SETCHAR_NAME3;
			mcpy(buf+1,ch[cn].name+30,10);
			*(unsigned long*)(buf+11)=ch[cn].temp;
			xsend(nr,buf,16);
			mcpy(cpl->name,ch[cn].name,40);
		}
		if (cpl->mode!=ch[cn].mode) {
			buf[0]=SV_SETCHAR_MODE;
			buf[1]=ch[cn].mode;
			xsend(nr,buf,2);
	
			cpl->mode=ch[cn].mode;
		}
		for (n=0; n<5; n++) plr_change_stat(nr,cpl->attrib[n],ch[cn].attrib[n],SV_SETCHAR_ATTRIB,n);

		plr_change_power(nr,cpl->hp,ch[cn].hp,SV_SETCHAR_HP);
		plr_change_power(nr,cpl->end,ch[cn].end,SV_SETCHAR_ENDUR);
		plr_change_power(nr,cpl->mana,ch[cn].mana,SV_SETCHAR_MANA);
	
		for (n=0; n<50; n++) plr_change_stat(nr,cpl->skill[n],ch[cn].skill[n],SV_SETCHAR_SKILL,n);

		for (n=0; n<40; n++) {
                if (cpl->item[n]!=(in=ch[cn].item[n]) || (!IS_BUILDING(cn) && it[in].flags&IF_UPDATE)) {
                        buf[0]=SV_SETCHAR_ITEM;
                        *(unsigned long*)(buf+1)=n;
                        if (in) {
                                if (IS_BUILDING(cn)) {
                                        if (in&0x40000000) {
                                                switch(in&0xfffffff) {
                                                        case    MF_MOVEBLOCK:   *(short int*)(buf+5)=47; break;
                                                        case    MF_SIGHTBLOCK:  *(short int*)(buf+5)=83; break;
                                                        case    MF_INDOORS:     *(short int*)(buf+5)=48; break;
                                                        case    MF_UWATER:      *(short int*)(buf+5)=50; break;
                                                        case    MF_NOMONST:     *(short int*)(buf+5)=51; break;
                                                        case    MF_BANK:        *(short int*)(buf+5)=52; break;
                                                        case    MF_TAVERN:      *(short int*)(buf+5)=53; break;
                                                        case    MF_NOMAGIC:     *(short int*)(buf+5)=54; break;
                                                        case    MF_DEATHTRAP:   *(short int*)(buf+5)=74; break;
                                                        case    MF_ARENA:       *(short int*)(buf+5)=78; break;
                                                        case    MF_NOEXPIRE:    *(short int*)(buf+5)=81; break;
                                                        case    MF_NOLAG:       *(short int*)(buf+5)=49; break;
                                                        default:                *(short int*)(buf+5)=0; break;
                                                }
                                                *(short int*)(buf+7)=0;
                                        } else if (in&0x20000000) {
                                                *(short int*)(buf+5)=in&0xfffffff;
                                                *(short int*)(buf+7)=0;
                                        } else {
                                                *(short int*)(buf+5)=it_temp[in].sprite[0];
                                                *(short int*)(buf+7)=0;
                                        }
                                } else {
                                        if (it[in].active) *(short int*)(buf+5)=it[in].sprite[1];
                                        else  *(short int*)(buf+5)=it[in].sprite[0];
                                        *(short int*)(buf+7)=it[in].placement;

                                        it[in].flags&=~IF_UPDATE;
                                }
                        } else {
                                *(short int*)(buf+5)=0;
                                *(short int*)(buf+7)=0;
                        }
                        xsend(nr,buf,9);

                        cpl->item[n]=in;

			}
		}
	
		for (n=0; n<20; n++) {
			if (cpl->worn[n]!=(in=ch[cn].worn[n]) || (it[in].flags&IF_UPDATE)) {
				buf[0]=SV_SETCHAR_WORN;
				*(unsigned long*)(buf+1)=n;
				if (in) {
					if (it[in].active) *(short int*)(buf+5)=it[in].sprite[1];
					else *(short int*)(buf+5)=it[in].sprite[0];
					*(short int*)(buf+7)=it[in].placement;
				} else {
					*(short int*)(buf+5)=0;
					*(short int*)(buf+7)=0;
				}
				xsend(nr,buf,9);
	
				cpl->worn[n]=in;
				it[in].flags&=~IF_UPDATE;
			}
		}
	
		for (n=0; n<20; n++) {
			if (cpl->spell[n]!=(in=ch[cn].spell[n]) ||
			    (cpl->active[n]!=it[in].active*16/max(1,it[in].duration)) || (it[in].flags&IF_UPDATE)) {
				buf[0]=SV_SETCHAR_SPELL;
				*(unsigned long*)(buf+1)=n;
				if (in) {
					*(short int*)(buf+5)=it[in].sprite[1];
					*(short int*)(buf+7)=it[in].active*16/max(1,it[in].duration);
					cpl->spell[n]=in;
					cpl->active[n]=(it[in].active*16/max(1,it[in].duration));
					it[in].flags&=~IF_UPDATE;
				} else {
					*(short int*)(buf+5)=0;
					*(short int*)(buf+7)=0;
					cpl->spell[n]=0;
					cpl->active[n]=0;
				}
				xsend(nr,buf,9);
			}
		}
	
		if (cpl->citem!=(in=ch[cn].citem) || (!IS_BUILDING(cn) && !(in&0x80000000) && it[in].flags&IF_UPDATE)) {
			buf[0]=SV_SETCHAR_OBJ;
			if (in&0x80000000) {
				if ((in&0x7fffffff)>999999) *(short int*)(buf+1)=121;
				else if ((in&0x7fffffff)>99999) *(short int*)(buf+1)=120;
				else if ((in&0x7fffffff)>9999) *(short int*)(buf+1)=41;
				else if ((in&0x7fffffff)>999) *(short int*)(buf+1)=40;
				else if ((in&0x7fffffff)>99) *(short int*)(buf+1)=39;
				else if ((in&0x7fffffff)>9) *(short int*)(buf+1)=38;
				else *(short int*)(buf+1)=37;
				*(short int*)(buf+3)=0;
			} else if (in) {
				if (IS_BUILDING(cn)) {
					*(short int*)(buf+1)=46;
					*(short int*)(buf+3)=0;
				} else {
					if (it[in].active) *(short int*)(buf+1)=it[in].sprite[1];
					else *(short int*)(buf+1)=it[in].sprite[0];
					*(short int*)(buf+3)=it[in].placement;
	
					it[in].flags&=~IF_UPDATE;
				}
			} else {
				*(short int*)(buf+1)=0;
				*(short int*)(buf+3)=0;
			}
			xsend(nr,buf,5);
	
			cpl->citem=in;
		}
	}

        if (cpl->a_hp!=(ch[cn].a_hp+500)/1000) {
		buf[0]=SV_SETCHAR_AHP;
                *(unsigned short*)(buf+1)=(unsigned short)((ch[cn].a_hp+500)/1000);
                xsend(nr,buf,3);

                cpl->a_hp=(ch[cn].a_hp+500)/1000;
	}

	if (cpl->a_end!=(ch[cn].a_end+500)/1000) {
		buf[0]=SV_SETCHAR_AEND;
                *(unsigned short*)(buf+1)=(unsigned short)((ch[cn].a_end+500)/1000);
                xsend(nr,buf,3);

                cpl->a_end=(ch[cn].a_end+500)/1000;
	}

        if (cpl->a_mana!=(ch[cn].a_mana+500)/1000) {
		buf[0]=SV_SETCHAR_AMANA;
                *(unsigned short*)(buf+1)=(unsigned short)((ch[cn].a_mana+500)/1000);
                xsend(nr,buf,3);

                cpl->a_mana=(ch[cn].a_mana+500)/1000;
	}

        if (cpl->dir!=ch[cn].dir) {
                buf[0]=SV_SETCHAR_DIR;
                *(unsigned char*)(buf+1)=ch[cn].dir;
                xsend(nr,buf,2);

                cpl->dir=ch[cn].dir;
        }

        if (cpl->points!=ch[cn].points ||
            cpl->points_tot!=ch[cn].points_tot ||
            cpl->kindred!=ch[cn].kindred) {
                buf[0]=SV_SETCHAR_PTS;
                *(unsigned long*)(buf+1)=ch[cn].points;
                *(unsigned long*)(buf+5)=ch[cn].points_tot;
                *(unsigned int*)(buf+9)=ch[cn].kindred;
                xsend(nr,buf,13);

                cpl->points=ch[cn].points;
                cpl->points_tot=ch[cn].points_tot;
                cpl->kindred=ch[cn].kindred;
        }

        if (cpl->gold!=ch[cn].gold || cpl->armor!=ch[cn].armor || cpl->weapon!=ch[cn].weapon) {
                buf[0]=SV_SETCHAR_GOLD;
                *(unsigned long*)(buf+1)=ch[cn].gold;
                *(unsigned long*)(buf+5)=ch[cn].armor;
                *(unsigned long*)(buf+9)=ch[cn].weapon;
                xsend(nr,buf,13);

                cpl->gold=ch[cn].gold;
                cpl->weapon=ch[cn].weapon;
                cpl->armor=ch[cn].armor;
        }

        if ((ch[cn].flags&CF_GOD) && (globs->ticker&31)==0) {
                buf[0]=SV_LOAD;
                *(unsigned int*)(buf+1)=globs->load;
                xsend(nr,buf,5);
        }

        // cplayer - map

        if (cpl->x!=ch[cn].x || cpl->y!=ch[cn].y) {
                if (cpl->x==ch[cn].x-1 && cpl->y==ch[cn].y) {
                        buf[0]=SV_SCROLL_RIGHT;
                        xsend(nr,buf,1);

                        memmove(cmap,cmap+1,sizeof(struct cmap)*(TILEX*TILEY-1));
                } else if (cpl->x==ch[cn].x+1 && cpl->y==ch[cn].y) {
                        buf[0]=SV_SCROLL_LEFT;
                        xsend(nr,buf,1);

                        memmove(cmap+1,cmap,sizeof(struct cmap)*(TILEX*TILEY-1));
                } else if (cpl->x==ch[cn].x && cpl->y==ch[cn].y-1) {
                        buf[0]=SV_SCROLL_DOWN;
                        xsend(nr,buf,1);

                        memmove(cmap,cmap+TILEX,sizeof(struct cmap)*(TILEX*TILEY-TILEX));
                } else if (cpl->x==ch[cn].x && cpl->y==ch[cn].y+1) {
                        buf[0]=SV_SCROLL_UP;
                        xsend(nr,buf,1);

                        memmove(cmap+TILEX,cmap,sizeof(struct cmap)*(TILEX*TILEY-TILEX));
                } else if (cpl->x==ch[cn].x+1 && cpl->y==ch[cn].y+1) {
                        buf[0]=SV_SCROLL_LEFTUP;
                        xsend(nr,buf,1);

                        memmove(cmap+TILEX+1,cmap,sizeof(struct cmap)*(TILEX*TILEY-TILEX-1));
                } else if (cpl->x==ch[cn].x+1 && cpl->y==ch[cn].y-1) {
                        buf[0]=SV_SCROLL_LEFTDOWN;
                        xsend(nr,buf,1);

                        memmove(cmap,cmap+TILEX-1,sizeof(struct cmap)*(TILEX*TILEY-TILEX+1));
                } else if (cpl->x==ch[cn].x-1 && cpl->y==ch[cn].y+1) {
                        buf[0]=SV_SCROLL_RIGHTUP;
                        xsend(nr,buf,1);

                        memmove(cmap+TILEX-1,cmap,sizeof(struct cmap)*(TILEX*TILEY-TILEX+1));
                } else if (cpl->x==ch[cn].x-1 && cpl->y==ch[cn].y-1) {
                        buf[0]=SV_SCROLL_RIGHTDOWN;
                        xsend(nr,buf,1);

                        memmove(cmap,cmap+TILEX+1,sizeof(struct cmap)*(TILEX*TILEY-TILEX-1));
                }

                cpl->x=ch[cn].x;
                cpl->y=ch[cn].y;

		buf[0]=SV_SETORIGIN;
		*(short*)(buf+1)=ch[cn].x-TMIDDLEX; //smap[0].x;
		*(short*)(buf+3)=ch[cn].y-TMIDDLEY; //smap[0].y;
		xsend(nr,buf,5);
        }

	// light as special case which sends multiple tiles at once
        for (n=0; n<TILEX*TILEY; n++) {
                if (cmap[n].light!=smap[n].light) {
			int bp=0,l,bl=0,pe;
                        static int (*lfunc[])(int,int,struct cmap*,struct cmap*)=
				{cl_light_one,cl_light_three,cl_light_seven,cl_light_26};

			for (l=0; l<sizeof(lfunc)/sizeof(int*); l++) {
				pe=lfunc[l](n,0,cmap,smap);
				if (pe>=bp) { bp=pe; bl=l; }
			}

			lfunc[bl](n,nr,cmap,smap);
                }
	}

	// everything else
	//for (n=0; n<TILEX*TILEY; n++) {
        // 	if (!fdiff(&cmap[n],&smap[n],sizeof(struct cmap))) continue;

	n=0;
	while (1) {
		unsigned char *tmp;

		tmp=fdiff(cmap+n,smap+n,sizeof(struct cmap)*(TILEX*TILEY-n));	// find address of first difference
		if (!tmp) break;						// no difference found? then we're done

		// calculate index of difference found
		n=(tmp-(unsigned char*)(cmap+n))/sizeof(struct cmap)+n;

	//for (f=0; (n=player[nr].changed_field[f])!=-1; f++) {	
                if (n>lastn && n-lastn<127) {
			buf[0]=SV_SETMAP|(n-lastn);
                        buf[1]=0; p=2;
		} else {
			buf[0]=SV_SETMAP;
			buf[1]=0;
			*(unsigned short*)(buf+2)=(short)n;
			p=4;
		}

                if (cmap[n].ba_sprite!=smap[n].ba_sprite) {
			buf[1]|=1;
			*(unsigned short*)(buf+p)=smap[n].ba_sprite; p+=2;
			
		}
                if (cmap[n].flags!=smap[n].flags) {
			buf[1]|=2;
			*(unsigned int*)(buf+p)=smap[n].flags; p+=4;
		}
                if (cmap[n].flags2!=smap[n].flags2) {
			buf[1]|=4;
			*(unsigned int*)(buf+p)=smap[n].flags2; p+=4;
		}
                if (cmap[n].it_sprite!=smap[n].it_sprite) {
			buf[1]|=8;
                        *(unsigned short*)(buf+p)=smap[n].it_sprite; p+=2;
		}
                if (cmap[n].it_status!=smap[n].it_status && it_base_status(cmap[n].it_status)!=it_base_status(smap[n].it_status)) {
			buf[1]|=16;
                        *(unsigned char*)(buf+p)=smap[n].it_status; p+=1;
                }
                if (cmap[n].ch_sprite!=smap[n].ch_sprite ||
                    (cmap[n].ch_status!=smap[n].ch_status && ch_base_status(cmap[n].ch_status)!=ch_base_status(smap[n].ch_status)) ||
		    cmap[n].ch_status2!=smap[n].ch_status2) {
			buf[1]|=32;
			*(unsigned short*)(buf+p)=smap[n].ch_sprite; p+=2;
			*(unsigned char*)(buf+p)=smap[n].ch_status; p+=1;
			*(unsigned char*)(buf+p)=smap[n].ch_status2; p+=1;
		}
                if (cmap[n].ch_speed!=smap[n].ch_speed ||
                    cmap[n].ch_nr!=smap[n].ch_nr ||
                    cmap[n].ch_id!=smap[n].ch_id) {
			buf[1]|=64;
			*(unsigned short*)(buf+p)=smap[n].ch_nr; p+=2;
                        *(unsigned short*)(buf+p)=smap[n].ch_id; p+=2;
			*(unsigned char*)(buf+p)=smap[n].ch_speed; p+=1;
		}

                if (cmap[n].ch_proz!=smap[n].ch_proz) {
			buf[1]|=128;
                        *(unsigned char*)(buf+p)=smap[n].ch_proz; p+=1;
                }
		
		if (buf[1]) { // we found a change (no change found can happen since it_status & ch_status are not transmitted all the time)
			xsend(nr,buf,p);	
			lastn=n;
		}
                mcpy(&cmap[n],&smap[n],sizeof(struct cmap));

		n++;				// done with this field
		if (n>=TILEX*TILEY) break;	// finished?
        }

        if (ch[cn].attack_cn!=cpl->attack_cn ||
            ch[cn].goto_x!=cpl->goto_x ||
            ch[cn].goto_y!=cpl->goto_y ||
            ch[cn].misc_action!=cpl->misc_action ||
            ch[cn].misc_target1!=cpl->misc_target1 ||
            ch[cn].misc_target2!=cpl->misc_target2) {
                buf[0]=SV_SETTARGET;
                cpl->attack_cn=*(unsigned short*)(buf+1)=ch[cn].attack_cn;
                cpl->goto_x=*(unsigned short*)(buf+3)=ch[cn].goto_x;
                cpl->goto_y=*(unsigned short*)(buf+5)=ch[cn].goto_y;
                cpl->misc_action=*(unsigned short*)(buf+7)=ch[cn].misc_action;
                cpl->misc_target1=*(unsigned short*)(buf+9)=ch[cn].misc_target1;
                cpl->misc_target2=*(unsigned short*)(buf+11)=ch[cn].misc_target2;
                xsend(nr,buf,13);
        }
}

void char_play_sound(int cn,int sound,int vol,int pan)
{
        unsigned char buf[16];
        int nr;

        nr=ch[cn].player;

        if (!nr) return;

        buf[0]=SV_PLAYSOUND;
        *(int*)(buf+1)=sound;
        *(int*)(buf+5)=vol;
        *(int*)(buf+9)=pan;

        xsend(nr,buf,13);
}

inline int do_char_calc_light(int cn,int light)
{
        int val;

        if (light==0 && ch[cn].skill[SK_PERCEPT][5]>150) light=1;

        val=light*min(ch[cn].skill[SK_PERCEPT][5],10)/10;

        if (val>255) val=255;

        if ((ch[cn].flags&CF_INFRARED) && val<5) val=5;

        return val;
}

inline int check_dlight(int x,int y)
{
        int m;

        m=x+y*MAPX;

        if (!(map[m].flags&MF_INDOORS)) return globs->dlight;

        return (globs->dlight*map[m].dlight)/256;
}

inline int check_dlightm(int m)
{
        if (!(map[m].flags&MF_INDOORS)) return globs->dlight;

        return (globs->dlight*map[m].dlight)/256;
}

static void inline empty_field(struct cmap *smap,int n)
{
	smap[n].ba_sprite=SPR_EMPTY;
        smap[n].flags=0;
        smap[n].flags2=0;
        smap[n].light=0;

        smap[n].ch_sprite=0;
        smap[n].ch_status=0;
        smap[n].ch_status2=0;
        smap[n].ch_speed=0;
        smap[n].ch_nr=0;
        smap[n].ch_id=0;
        smap[n].ch_proz=0;

        smap[n].it_sprite=0;
        smap[n].it_status=0;
}

/* static int inline tmp_vis(int x,int y)
{
        int in;

        if (map[x+y*MAPX].flags&MF_SIGHTBLOCK) return 0;
        if ((in=map[x+y*MAPX].it) && (it[in].flags&IF_SIGHTBLOCK)) return 0;
        if (map[x+y*MAPX].ch) return 0;

        return 1;
}

static int tmp_see(int frx,int fry,int tox,int toy)
{
        long long fx,fy,tx,ty,x,y;
        long long dx,dy,panic=0;
        //long long visi=16384;

        x=fx=(((long long)(frx))<<16)+32768; y=fy=(((long long)(fry))<<16)+32768;
        tx=(((long long)(tox))<<16)+32768; ty=(((long long)(toy))<<16)+32768;

        dx=tox-frx;
        dy=toy-fry;

        if (!dx && !dy) return 65536;

        if (abs(dx)>abs(dy)) { dy*=65536; dy/=abs(dx); if (dx>0) dx=65536; else dx=-65536; }
        else { dx*=65536; dx/=abs(dy); if (dy>0) dy=65536; else dy=-65536; }

        while (1) {
                x+=dx; y+=dy;
                if ((x>>16)==(tx>>16) && (y>>16)==(ty>>16)) return 65536; //return visi;
                //if (!tmp_vis(x>>16,y>>16)) visi-=abs(32768-(x&0xffff))+abs(32768-(y&0xffff));
                //if (visi<0) return 0;

                if (!tmp_vis(x>>16,y>>16)) return 0;

                if (panic++>25) {
                        xlog("f=%d,%d t=%d,%d (%d,%d) d=%d,%d",(int)(fx>>16),(int)(fy>>16),(int)(tx>>16),(int)(ty>>16),(int)(x>>16),(int)(y>>16),(int)dx,(int)dy);
                        return 1;
                }
        }
}


static int tmp_see1(int frx,int fry,int tox,int toy)
{
        long long fx,fy,tx,ty,x,y;
        long long dx,dy,panic=0;

        x=fx=(((long long)(frx))<<16)+60000; y=fy=(((long long)(fry))<<16)+60000;
        tx=(((long long)(tox))<<16)+60000; ty=(((long long)(toy))<<16)+60000;

        dx=tox-frx;
        dy=toy-fry;

        if (!dx && !dy) return 1;

        if (abs(dx)>abs(dy)) { dy*=65536; dy/=abs(dx); if (dx>0) dx=65536; else dx=-65536; }
        else { dx*=65536; dx/=abs(dy); if (dy>0) dy=65536; else dy=-65536; }

        while (1) {
                x+=dx; y+=dy;
                if ((x>>16)==(tx>>16) && (y>>16)==(ty>>16)) return 1;
                if (!tmp_vis(x>>16,y>>16)) return 0;

                if (panic++>25) {
                        xlog("f=%d,%d t=%d,%d (%d,%d) d=%d,%d",(int)(fx>>16),(int)(fy>>16),(int)(tx>>16),(int)(ty>>16),(int)(x>>16),(int)(y>>16),(int)dx,(int)dy);
                        return 1;
                }
        }
}

static int tmp_see2(int frx,int fry,int tox,int toy)
{
        long long fx,fy,tx,ty,x,y;
        long long dx,dy,panic=0;

        x=fx=(((long long)(frx))<<16)+5000; y=fy=(((long long)(fry))<<16)+5000;
        tx=(((long long)(tox))<<16)+5000; ty=(((long long)(toy))<<16)+5000;

        dx=tox-frx;
        dy=toy-fry;

        if (!dx && !dy) return 1;

        if (abs(dx)>abs(dy)) { dy*=65536; dy/=abs(dx); if (dx>0) dx=65536; else dx=-65536; }
        else { dx*=65536; dx/=abs(dy); if (dy>0) dy=65536; else dy=-65536; }

        while (1) {
                x+=dx; y+=dy;
                if ((x>>16)==(tx>>16) && (y>>16)==(ty>>16)) return 1;
                if (!tmp_vis(x>>16,y>>16)) return 0;

                if (panic++>25) {
                        xlog("f=%d,%d t=%d,%d (%d,%d) d=%d,%d",(int)(fx>>16),(int)(fy>>16),(int)(tx>>16),(int)(ty>>16),(int)(x>>16),(int)(y>>16),(int)dx,(int)dy);
                        return 1;
                }
        }
}*/

#define YSCUT	3	//3
#define YECUT	1	//1
#define XSCUT	2	//2
#define XECUT	2	//2

void plr_getmap_complete(int nr)
{
        int cn,co,light,tmp,visible,infra=0;
        int x,y,n,m;
        int ys,ye,xs,xe;
        unsigned char do_all=0;
        struct cmap *smap;

        cn=player[nr].usnr;
        smap=player[nr].smap;

        ys=ch[cn].y-(TILEY/2)+YSCUT;
        ye=ch[cn].y+(TILEY/2)-YECUT;
        xs=ch[cn].x-(TILEX/2)+XSCUT;
        xe=ch[cn].x+(TILEX/2)-XECUT;

        // trigger recomputation of visibility map
        can_see(cn,ch[cn].x,ch[cn].y,ch[cn].x+1,ch[cn].y+1,16);

	// check if visibility changed
        if (player[nr].vx!=see[cn].x || player[nr].vy!=see[cn].y || mcmp(player[nr].visi,see[cn].vis,40*40)) {
            	mcpy(player[nr].visi,see[cn].vis,40*40);
            	player[nr].vx=see[cn].x;
            	player[nr].vy=see[cn].y;
            	do_all=1;
        }

	if (IS_BUILDING(cn)) do_all=1;

        for (n=YSCUT*TILEX+XSCUT,m=xs+ys*MAPX,y=ys; y<ye; y++,m+=MAPX-TILEX+XSCUT+XECUT,n+=XSCUT+XECUT) {
                for (x=xs; x<xe; x++,n++,m++) {

                        if (do_all || map[m].it || map[m].ch || mcmp(&player[nr].xmap[n],&map[m],sizeof(struct map))) {	// any change on map data?
                                mcpy(&player[nr].xmap[n],&map[m],sizeof(struct map)); 		// remember changed values
			} else continue;

			//player[nr].changed_field[p++]=n;

                        // field outside of map? then display empty one.
			if (x<0 || y<0 || x>=MAPX || y>=MAPY) { empty_field(smap,n); continue; }

			// calculate light
			tmp=check_dlightm(m);

			light=max(map[m].light,tmp);
			light=do_char_calc_light(cn,light);
			if (light<=5 && (ch[cn].flags&CF_INFRARED)) infra=1; else infra=0;

			// everybody sees himself
			if (light==0 && map[m].ch==cn) light=1;

			// no light, nothing visible
			if (light==0) { empty_field(smap,n); continue; }

			// --- Begin of Flags -----
			smap[n].flags=0;

			if (map[m].flags&(MF_GFX_INJURED|
					  MF_GFX_INJURED1|
					  MF_GFX_INJURED2|
					  MF_GFX_DEATH|
					  MF_GFX_TOMB|
					  MF_GFX_EMAGIC|
					  MF_GFX_GMAGIC|
					  MF_GFX_CMAGIC|
					  MF_UWATER)) {
				if (map[m].flags&MF_GFX_INJURED) smap[n].flags|=INJURED;
				if (map[m].flags&MF_GFX_INJURED1) smap[n].flags|=INJURED1;
				if (map[m].flags&MF_GFX_INJURED2) smap[n].flags|=INJURED2;

				if (map[m].flags&MF_GFX_DEATH)  smap[n].flags|=(map[m].flags&MF_GFX_DEATH)>>23;
				if (map[m].flags&MF_GFX_TOMB)   smap[n].flags|=(map[m].flags&MF_GFX_TOMB)>>23;
				if (map[m].flags&MF_GFX_EMAGIC) smap[n].flags|=(map[m].flags&MF_GFX_EMAGIC)>>23;
				if (map[m].flags&MF_GFX_GMAGIC) smap[n].flags|=(map[m].flags&MF_GFX_GMAGIC)>>23;
				if (map[m].flags&MF_GFX_CMAGIC) smap[n].flags|=(map[m].flags&MF_GFX_CMAGIC)>>23;

				if (map[m].flags&MF_UWATER) smap[n].flags|=UWATER;
			}

			if (infra) smap[n].flags|=INFRARED;

			if (IS_BUILDING(cn)) { smap[n].flags2=(unsigned int)map[m].flags; } else smap[n].flags2=0;

			tmp=(x-ch[cn].x+20)+(y-ch[cn].y+20)*40;

			if (see[cn].vis[tmp+0+0] ||
			    see[cn].vis[tmp+0+40] ||
			    see[cn].vis[tmp+0-40] ||
			    see[cn].vis[tmp+1+0] ||
			    see[cn].vis[tmp+1+40] ||
			    see[cn].vis[tmp+1-40] ||
			    see[cn].vis[tmp-1+0] ||
			    see[cn].vis[tmp-1+40] ||
			    see[cn].vis[tmp-1-40]) visible=1;
			else visible=0;

			if (!visible) smap[n].flags|=INVIS;

			// --- End of Flags ----- - more flags set in character part further down
			
			// --- Begin of Light -----
			if (light>64) smap[n].light=0;
			else if (light>52) smap[n].light=1;
			else if (light>40) smap[n].light=2;
			else if (light>32) smap[n].light=3;
			else if (light>28) smap[n].light=4;
			else if (light>24) smap[n].light=5;
			else if (light>20) smap[n].light=6;
			else if (light>16) smap[n].light=7;
			else if (light>14) smap[n].light=8;
			else if (light>12) smap[n].light=9;
			else if (light>10) smap[n].light=10;
			else if (light>8) smap[n].light=11;
			else if (light>6) smap[n].light=12;
			else if (light>4) smap[n].light=13;
			else if (light>2) smap[n].light=14;
			else smap[n].light=15;
			// --- End of Flags -----

			// --- Begin of ba_sprite  -----
			smap[n].ba_sprite=map[m].sprite;
			// --- End of ba_sprite -----

			// --- Begin of Character ----- - also sets flags
			if (visible && (co=map[m].ch)!=0 && (tmp=do_char_can_see(cn,co))!=0) {
				if (!ch[co].sprite_override) smap[n].ch_sprite=ch[co].sprite;
				else smap[n].ch_sprite=ch[co].sprite_override;
				smap[n].ch_status=ch[co].status;
				smap[n].ch_status2=ch[co].status2;
				smap[n].ch_speed=ch[co].speed;
				smap[n].ch_nr=co;
				smap[n].ch_id=(unsigned short)char_id(co);
				if (tmp<=75 && ch[co].hp[5]>0) smap[n].ch_proz=((ch[co].a_hp+5)/10)/ch[co].hp[5];
				else smap[n].ch_proz=0;
				smap[n].flags|=ISCHAR;
				if (ch[co].stunned) smap[n].flags|=STUNNED;
				if (ch[co].flags&CF_STONED) smap[n].flags|=STUNNED|STONED;
			} else {
				smap[n].ch_sprite=0;
				smap[n].ch_status=0;
				smap[n].ch_status2=0;
				smap[n].ch_speed=0;
				smap[n].ch_nr=0;
				smap[n].ch_id=0;
				smap[n].ch_proz=0;
			}
			// --- End of Character -----

			// --- Begin of Item -----
			if (map[m].fsprite) {
				smap[n].it_sprite=map[m].fsprite;
				smap[n].it_status=0;
			} else if ((co=map[m].it)!=0 && (!(it[co].flags&(IF_TAKE|IF_HIDDEN)) || (visible && do_char_can_see_item(cn,co)))) {
				if (it[co].active) {
					smap[n].it_sprite=it[co].sprite[1];
					smap[n].it_status=it[co].status[1];
				} else {
					smap[n].it_sprite=it[co].sprite[0];
					smap[n].it_status=it[co].status[0];
				}
				if ((it[co].flags&IF_LOOK) || (it[co].flags&IF_LOOKSPECIAL))
					smap[n].flags|=ISITEM;
				if (!(it[co].flags&IF_TAKE) && it[co].flags&(IF_USE|IF_USESPECIAL)) smap[n].flags|=ISUSABLE;
			} else {
				smap[n].it_sprite=0;
				smap[n].it_status=0;
			}
			// --- Begin of Item -----
                }
        }
	//player[nr].changed_field[p++]=-1;

        player[nr].vx=see[cn].x;
        player[nr].vy=see[cn].y;
}

#define YSCUTF	(YSCUT+3)	//3
#define YECUTF	(YECUT)		//1
#define XSCUTF	(XSCUT)		//2
#define XECUTF	(XECUT+3)	//2

void plr_getmap_fast(int nr)
{
        int cn,co,light,tmp,visible,infra=0;
        int x,y,n,m;
        int ys,ye,xs,xe;
        unsigned char do_all=0;
        struct cmap *smap;

        cn=player[nr].usnr;
        smap=player[nr].smap;

        ys=ch[cn].y-(TILEY/2)+YSCUTF;
        ye=ch[cn].y+(TILEY/2)-YECUTF;
        xs=ch[cn].x-(TILEX/2)+XSCUTF;
        xe=ch[cn].x+(TILEX/2)-XECUTF;

        // trigger recomputation of visibility map
        can_see(cn,ch[cn].x,ch[cn].y,ch[cn].x+1,ch[cn].y+1,16);

	// check if visibility changed
        if (player[nr].vx!=see[cn].x || player[nr].vy!=see[cn].y || mcmp(player[nr].visi,see[cn].vis,40*40)) {
            	mcpy(player[nr].visi,see[cn].vis,40*40);
            	player[nr].vx=see[cn].x;
            	player[nr].vy=see[cn].y;
            	do_all=1;
        }

	if (IS_BUILDING(cn)) do_all=1;

        for (n=YSCUTF*TILEX+XSCUTF,m=xs+ys*MAPX,y=ys; y<ye; y++,m+=MAPX-TILEX+XSCUTF+XECUTF,n+=XSCUTF+XECUTF) {
                for (x=xs; x<xe; x++,n++,m++) {

                        if (do_all || map[m].it || map[m].ch || mcmp(&player[nr].xmap[n],&map[m],sizeof(struct map))) {	// any change on map data?
                                mcpy(&player[nr].xmap[n],&map[m],sizeof(struct map)); 		// remember changed values
			} else continue;

                        // field outside of map? then display empty one.
			if (x<0 || y<0 || x>=MAPX || y>=MAPY) { empty_field(smap,n); continue; }

			// calculate light
			tmp=check_dlightm(m);

			light=max(map[m].light,tmp);
			light=do_char_calc_light(cn,light);
			if (light<=5 && (ch[cn].flags&CF_INFRARED)) infra=1; else infra=0;

			// everybody sees himself
			if (light==0 && map[m].ch==cn) light=1;

			// no light, nothing visible
			if (light==0) { empty_field(smap,n); continue; }

			// Flags
			smap[n].flags=0;
			smap[n].flags2=0;

			if (map[m].flags&(MF_GFX_INJURED|
					  MF_GFX_INJURED1|
					  MF_GFX_INJURED2|
					  MF_GFX_DEATH|
					  MF_GFX_TOMB|
					  MF_GFX_EMAGIC|
					  MF_GFX_GMAGIC|
					  MF_GFX_CMAGIC)) {
				if (map[m].flags&MF_GFX_INJURED) smap[n].flags|=INJURED;
				if (map[m].flags&MF_GFX_INJURED1) smap[n].flags|=INJURED1;
				if (map[m].flags&MF_GFX_INJURED2) smap[n].flags|=INJURED2;

				if (map[m].flags&MF_GFX_DEATH)  smap[n].flags|=(map[m].flags&MF_GFX_DEATH)>>23;
				if (map[m].flags&MF_GFX_TOMB)   smap[n].flags|=(map[m].flags&MF_GFX_TOMB)>>23;
				if (map[m].flags&MF_GFX_EMAGIC) smap[n].flags|=(map[m].flags&MF_GFX_EMAGIC)>>23;
				if (map[m].flags&MF_GFX_GMAGIC) smap[n].flags|=(map[m].flags&MF_GFX_GMAGIC)>>23;
				if (map[m].flags&MF_GFX_CMAGIC) smap[n].flags|=(map[m].flags&MF_GFX_CMAGIC)>>23;
			}

			if (map[m].flags&MF_UWATER) smap[n].flags|=UWATER;

			if (infra) smap[n].flags|=INFRARED;

			if (IS_BUILDING(cn)) { smap[n].flags2=(unsigned int)map[m].flags; }

			tmp=(x-ch[cn].x+20)+(y-ch[cn].y+20)*40;

			if (see[cn].vis[tmp+0+0] ||
			    see[cn].vis[tmp+0+40] ||
			    see[cn].vis[tmp+0-40] ||
			    see[cn].vis[tmp+1+0] ||
			    see[cn].vis[tmp+1+40] ||
			    see[cn].vis[tmp+1-40] ||
			    see[cn].vis[tmp-1+0] ||
			    see[cn].vis[tmp-1+40] ||
			    see[cn].vis[tmp-1-40]) visible=1;
			else visible=0;

			if (!visible)smap[n].flags|=INVIS;
			
			if (light>64) smap[n].light=0;
			else if (light>52) smap[n].light=1;
			else if (light>40) smap[n].light=2;
			else if (light>32) smap[n].light=3;
			else if (light>28) smap[n].light=4;
			else if (light>24) smap[n].light=5;
			else if (light>20) smap[n].light=6;
			else if (light>16) smap[n].light=7;
			else if (light>14) smap[n].light=8;
			else if (light>12) smap[n].light=9;
			else if (light>10) smap[n].light=10;
			else if (light>8) smap[n].light=11;
			else if (light>6) smap[n].light=12;
			else if (light>4) smap[n].light=13;
			else if (light>2) smap[n].light=14;
			else smap[n].light=15;

			// background
			smap[n].ba_sprite=map[m].sprite;

			// character
			if (visible && (co=map[m].ch)!=0 && (tmp=do_char_can_see(cn,co))!=0) {
				if (!ch[co].sprite_override) smap[n].ch_sprite=ch[co].sprite;
				else smap[n].ch_sprite=ch[co].sprite_override;
				smap[n].ch_status=ch[co].status;
				smap[n].ch_status2=ch[co].status2;
				smap[n].ch_speed=ch[co].speed;
				smap[n].ch_nr=co;
				smap[n].ch_id=(unsigned short)char_id(co);
				if (tmp<=75 && ch[co].hp[5]>0) smap[n].ch_proz=((ch[co].a_hp+5)/10)/ch[co].hp[5];
				else smap[n].ch_proz=0;
				smap[n].flags|=ISCHAR;
				if (ch[co].stunned) smap[n].flags|=STUNNED;
				if (ch[co].flags&CF_STONED) smap[n].flags|=STUNNED|STONED;
			} else {
				smap[n].ch_sprite=0;
				smap[n].ch_status=0;
				smap[n].ch_status2=0;
				smap[n].ch_speed=0;
				smap[n].ch_nr=0;
				smap[n].ch_id=0;
				smap[n].ch_proz=0;
			}

			// item
			if (map[m].fsprite) {
				smap[n].it_sprite=map[m].fsprite;
				smap[n].it_status=0;
			} else if ((co=map[m].it)!=0 && (!(it[co].flags&(IF_TAKE|IF_HIDDEN)) || (visible && do_char_can_see_item(cn,co)))) {
				if (it[co].active) {
					smap[n].it_sprite=it[co].sprite[1];
					smap[n].it_status=it[co].status[1];
				} else {
					smap[n].it_sprite=it[co].sprite[0];
					smap[n].it_status=it[co].status[0];
				}
				if ((it[co].flags&IF_LOOK) || (it[co].flags&IF_LOOKSPECIAL))
					smap[n].flags|=ISITEM;
				if (!(it[co].flags&IF_TAKE) && it[co].flags&(IF_USE|IF_USESPECIAL)) smap[n].flags|=ISUSABLE;
			} else {
				smap[n].it_sprite=0;
				smap[n].it_status=0;

			}
                }
        }
        player[nr].vx=see[cn].x;
        player[nr].vy=see[cn].y;
}

void plr_clear_map(void)
{
	int n;

	for (n=1; n<MAXPLAYER; n++) {
		bzero(player[n].smap,sizeof(player[n].smap));
		player[n].vx=0;	// force do_all in plr_getmap
	}
}

void plr_getmap(int nr)
{
	static int mode=0;

	if (globs->load_avg>8000 && mode==0 && (globs->flags&GF_SPEEDY)) { mode=1; plr_clear_map(); do_announce(0,0,"Entered speed savings mode. Display will be imperfect.\n"); }
	if ((!(globs->flags&GF_SPEEDY) || globs->load_avg<6500) && mode!=0) { mode=0; do_announce(0,0,"Left speed savings mode.\n"); }

	if (mode==0) plr_getmap_complete(nr);
	else plr_getmap_fast(nr);	
}

void stone_gc(int cn,int mode)
{
	int co;
	
	if (!(ch[cn].flags&CF_PLAYER)) return;
	if (!(co=ch[cn].data[64])) return;
	if (!IS_ACTIVECHAR(co)) return;
	if (ch[co].data[63]!=cn) return;
	if (mode) ch[co].flags|=CF_STONED;
	else ch[co].flags&=~CF_STONED;
}

void plr_tick(int nr)
{
        int cn;

        player[nr].ltick++;

        if (player[nr].state==ST_NORMAL) {
                cn=player[nr].usnr;

                if (cn && ch[cn].data[19] && (ch[cn].flags&CF_PLAYER)) {
                        if (player[nr].ltick>player[nr].rtick+ch[cn].data[19] && !(ch[cn].flags&CF_STONED)) {
                                chlog(cn,"Turned to stone due to lag (%.2fs)",(player[nr].ltick-player[nr].rtick)/18.0);
                                ch[cn].flags|=CF_STONED;
                                stone_gc(cn,1);
                        } else if (player[nr].ltick<player[nr].rtick+ch[cn].data[19]-TICKS && (ch[cn].flags&CF_STONED)) {
                                chlog(cn,"Unstoned, lag is gone");
                                ch[cn].flags&=~CF_STONED;
                                stone_gc(cn,0);
                        }
                }
        }
}

int check_valid(int cn)
{
        int n,in;

        if (ch[cn].x<1 || ch[cn].y<1 || ch[cn].x>MAPX-2 || ch[cn].y>MAPY-2) {
                chlog(cn,"Killed character %s (%d) for invalid data",ch[cn].name,cn);
                do_char_killed(0,cn);
                return 0;
        }

        n=ch[cn].x+ch[cn].y*MAPX;
        if (map[n].ch!=cn) {
                chlog(cn,"Not on map (%d)!",map[n].ch);
                if (map[n].ch) { chlog(cn,"drop=%d",god_drop_char_fuzzy_large(cn,900,900,900,900)); }
                else map[n].ch=cn;
        }

        if (IS_BUILDING(cn)) return 1;

        for (n=0; n<40; n++) {
                if ((in=ch[cn].item[n])!=0) {
                        if (it[in].carried!=cn || it[in].used!=USE_ACTIVE) {
                                xlog("Reset item %d (%s,%d) from char %d (%s)",
                                        in,it[in].name,it[in].used,cn,ch[cn].name);
                                ch[cn].item[n]=0;
                        }
                }
        }

        for (n=0; n<62; n++) {
                if ((in=ch[cn].depot[n])!=0) {
                        if (it[in].carried!=cn || it[in].used!=USE_ACTIVE) {
                                xlog("Reset depot item %d (%s,%d) from char %d (%s)",
                                        in,it[in].name,it[in].used,cn,ch[cn].name);
                                ch[cn].depot[n]=0;
                        }
                }
        }

        for (n=0; n<20; n++) {
                if ((in=ch[cn].worn[n])!=0) {
                        if (it[in].carried!=cn || it[in].used!=USE_ACTIVE) {
                                xlog("Reset worn item %d (%s,%d) from char %d (%s)",
                                        in,it[in].name,it[in].used,cn,ch[cn].name);
                                ch[cn].worn[n]=0;
                        }
                }
                if ((in=ch[cn].spell[n])!=0) {
                        if (it[in].carried!=cn || it[in].used!=USE_ACTIVE) {
                                xlog("Reset spell item %d (%s,%d,%d) from char %d (%s)",
                                        in,it[in].name,it[in].carried,it[in].used,cn,ch[cn].name);
                                ch[cn].spell[n]=0;
                        }
                }
        }

        if ((ch[cn].flags&CF_STONED) && !(ch[cn].flags&CF_PLAYER)) {
        	int co;
        	
        	if (!(co=ch[cn].data[63]) || !IS_ACTIVECHAR(co)) {
       			ch[cn].flags&=~CF_STONED;
       			chlog(cn,"oops, stoned removed");
       		}
        }

        return 1;
}

void check_expire(int cn)
{
	int erase=0,t,week=60*60*24*7,day=60*60*24;

	t=time(NULL);
	
	if (ch[cn].points_tot==0) {
		if (ch[cn].login_date+3*day<t) erase=1;
	} else if (ch[cn].points_tot<10000) {
		if (ch[cn].login_date+1*week<t) erase=1;
	} else if (ch[cn].points_tot<100000) {
		if (ch[cn].login_date+2*week<t) erase=1;
	} else if (ch[cn].points_tot<1000000) {
		if (ch[cn].login_date+4*week<t) erase=1;
	} else {
		if (ch[cn].login_date+6*week<t) erase=1;
	}
	if (erase) {
		xlog("erased player %s, %d exp",ch[cn].name,ch[cn].points_tot);
		god_destroy_items(cn);
		ch[cn].used=USE_EMPTY;
	}
}

inline int group_active(int cn)
{
        if ((ch[cn].flags&(CF_PLAYER|CF_USURP|CF_NOSLEEP)) && ch[cn].used==USE_ACTIVE) return 1;

        if (ch[cn].data[92]) return 1;

        return 0;
}

char *strnchr(char *ptr,char c,int len)
{
        while (len) {
                if (*ptr==c) return ptr;
                ptr++;
                len--;
        }
        return NULL;
}

static long cl=0;
static int cltick=0;
static int wakeup=1;

void tick(void)
{
        int n,cnt,hour,awake,body,online=0,plon;
        unsigned long long prof;
        time_t t;
        struct tm *tm;

        prof_tick();

        t=time(NULL);
        tm=localtime(&t);
        hour=tm->tm_hour;

        globs->ticker++;
        globs->uptime++;

        globs->uptime_per_hour[hour]++;

	if ((globs->ticker&31)==0) pop_save_char(globs->ticker%MAXCHARS);

        // send tick to players
        for (n=1; n<MAXPLAYER; n++) {
                if (!player[n].sock) continue;
                if (player[n].state!=ST_NORMAL && player[n].state!=ST_EXIT) continue;
                plr_tick(n);
                if (player[n].state==ST_NORMAL) online++;
        }

        if (online>globs->max_online) globs->max_online=online;
        if (online>globs->max_online_per_hour[hour]) globs->max_online_per_hour[hour]=online;

        // check for player commands and translate to character commands, also does timeout checking
        for (n=1; n<MAXPLAYER; n++) {
                if (!player[n].sock) continue;
                while (player[n].in_len>=16) {
                        plr_cmd(n);
                        player[n].in_len-=16;
                        memmove(player[n].inbuf,player[n].inbuf+16,240);
                }
                plr_idle(n);
        }

        // do login stuff
        for (n=1; n<MAXPLAYER; n++) {
                if (!player[n].sock) continue;
                if (player[n].state==ST_NORMAL) continue;
                plr_state(n);
        }

        // send changes to players
        for (n=1; n<MAXPLAYER; n++) {
                if (!player[n].sock) continue;
                if (player[n].state!=ST_NORMAL) continue;
//#define SPEEDTEST
#ifdef SPEEDTEST
		{
			int zz;

			for (zz=0; zz<80; zz++) {
				prof=prof_start(); plr_getmap(n); prof_stop(10,prof);
				prof=prof_start(); plr_change(n); prof_stop(11,prof);
			}
		}
#else
		prof=prof_start(); plr_getmap(n); prof_stop(10,prof);
		prof=prof_start(); plr_change(n); prof_stop(11,prof);
#endif
        }


        // let characters act
        cnt=awake=body=plon=0;

        if ((globs->ticker&63)==0) {
                if (wakeup>=MAXCHARS) wakeup=1;
                ch[wakeup++].data[92]=TICKS*60;
        }

        for (n=1; n<MAXCHARS; n++) {
        	if (ch[n].used==USE_EMPTY) continue;
        	cnt++;

		if (ch[n].flags&CF_UPDATE) {
			really_update_char(n);
			ch[n].flags&=~CF_UPDATE;
		}

        	if (ch[n].used==USE_NONACTIVE && (n&1023)==(globs->ticker&1023)) {
                	check_expire(n);
                }
                if (ch[n].flags&CF_BODY) {
                	if (!(ch[n].flags&CF_PLAYER) && ch[n].data[98]++>TICKS*60*30) {
                		chlog(n,"removing lost body");
                		god_destroy_items(n);
                		ch[n].used=USE_EMPTY;
                		continue;
                	}
                	body++;
                	continue;
                }

                if (ch[n].data[92]>0) ch[n].data[92]--; // reduce single awake
                if (ch[n].status<8 && !group_active(n)) continue;

                awake++;

                if (ch[n].used==USE_ACTIVE) {
                        if ((n&1023)==(globs->ticker&1023) && !check_valid(n)) continue;
                        ch[n].current_online_time++;
                        ch[n].total_online_time++;
                        if (ch[n].flags&(CF_PLAYER|CF_USURP)) {
                                globs->total_online_time++;
                                globs->online_per_hour[hour]++;
				if ((ch[n].flags&CF_PLAYER) && ch[n].data[71]>0) ch[n].data[71]--;
				if ((ch[n].flags&CF_PLAYER) && ch[n].data[72]>0) ch[n].data[72]--;
				if ((ch[n].flags&CF_PLAYER) && !(ch[n].flags&CF_INVISIBLE)) plon++;
                        }
                        prof=prof_start(); plr_act(n); prof_stop(12,prof);
                }
                do_regenerate(n);
        }
        globs->character_cnt=cnt;
        globs->awake=awake;
        globs->body=body;
	globs->players_online=plon;

        prof=prof_start(); pop_tick(); prof_stop(13,prof);
        prof=prof_start(); effect_tick(); prof_stop(14,prof);
        prof=prof_start(); item_tick(); prof_stop(15,prof);

        // do global updates like time of day, weather etc.
        prof=prof_start(); global_tick(); prof_stop(26,prof);

        ctick++;
        if (ctick>19) ctick=0;

        cltick++;
        if (cltick>17) {
                cltick=0;
                n=clock()/(CLOCKS_PER_SEC/100);
                if (cl && n>cl) {
			globs->load=n-cl;
			globs->load_avg=(int)((globs->load_avg*0.0099+globs->load*0.01)*100);
		}
                cl=n;
        }
}

void player_exit(int nr)
{
        int cn;

        player[nr].state=ST_EXIT;
        player[nr].lasttick=globs->ticker;

        cn=player[nr].usnr;
        if (cn && cn>0 && cn<MAXCHARS && ch[cn].player==nr) ch[cn].player=0;
}

