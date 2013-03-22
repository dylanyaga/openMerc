#include <stdio.h>
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock.h>
#include <zlib.h>

struct z_stream_s zs;

#pragma hdrstop
#include "common.h"
#include "inter.h"
#include "merc.rh"
#include "dd.h"
//#include "minilzo.h"

char passwd[15]={0};

#define DEBUG(a) 
//printf("%d: %s\n",__cnt++,a); fflush(stdout)
//static int __cnt=0;
#define DEBUG2(a) 
//xlog(1,a)

static void load_unique(void);
static void save_unique(void);

static int unique1=0,unique2=0;

static struct look tmplook;
struct look shop;

extern int show_look,look_timer;

static int sock=-1;

int t_size=0;	// ticks in queue

int ser_ver=0;

int ticker=0;

extern char host_addr[];
extern int host_port;

extern struct key okey;
extern int do_exit;

int sv_cmd(unsigned char *buf);
void sv_newplayer(unsigned char *buf);
unsigned int xcrypt(unsigned int val);

int so_status=0;

char *logout_reason[]={
"unknown",                                          //0
"Client failed challenge.",                         //1
"Client was idle too long.",                        //2
"No room to drop character.",                       //3
"Invalid parameters.",                              //4
"Character already active or no player character.", //5
"Invalid password.",                                //6
"Client too slow.",                                 //7
"Receive failure.",                                 //8
"Server is being shutdown.",                        //9
"You entered a Tavern.",                            //10
"Client version too old. Update needed.",           //11
"Aborting on user request.",                        //12
"this should never show up",                        //13
"You have been banned for an hour. Enhance your social behaviour before you come back."                 //14
};

extern HWND desk_hwnd;

int xrecv(int sock,char *buf,int len,int flags)
{
	int ret,size=0;

	while (size<len) {
		ret=recv(sock,buf+size,len-size,flags);
                if (ret<1) return size;
		size+=ret;
	}

        return size;
}

void so_error(char *err)
{
	char buf[250];

	dd_deinit();

	save_options();

	if (!do_exit) {
		sprintf(buf,"Error: %s (%d)",err,WSAGetLastError());
		MessageBox(desk_hwnd,buf,"Irregular Exit",MB_ICONSTOP|MB_OK);
	}

	exit(0);
}

int so_login(unsigned char *buf,HWND hwnd)
{
	unsigned int tmp,prio;
	unsigned char obuf[16];
	char xbuf[128];
	extern int race;
	static int capcnt;
	static char mod[256];

	if (buf[0]==SV_CHALLENGE) {	// answer challenges at once
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Login Part I");

		tmp=*(unsigned long*)(buf+1);
		tmp=xcrypt(tmp);

		obuf[0]=CL_CHALLENGE;
		*(unsigned long*)(obuf+1)=tmp;
		*(unsigned long*)(obuf+5)=VERSION;
		*(unsigned long*)(obuf+9)=race;
		send(sock,(char*)obuf,16,0);

		load_unique();

		obuf[0]=CL_CMD_UNIQUE;
		*(unsigned long*)(obuf+1)=unique1;
		*(unsigned long*)(obuf+5)=unique2;
		send(sock,(char*)obuf,16,0);

		capcnt=0;

		return 0;
	}
	if (buf[0]==SV_NEWPLAYER) {	// answer newplayer at once
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Login as New Player OK");
		okey.usnr=*(unsigned long*)(buf+1);
		okey.pass1=*(unsigned long*)(buf+5);
		okey.pass2=*(unsigned long*)(buf+9);
		ser_ver=*(unsigned char*)(buf+13);
		ser_ver+=(int)((*(unsigned char*)(buf+14)))<<8;
		ser_ver+=(int)((*(unsigned char*)(buf+15)))<<16;
		save_options();
		return 1;
	}
	if (buf[0]==SV_LOGIN_OK) {
		ser_ver=*(unsigned long*)(buf+1);
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Login OK");
		return 1;
	}

	if (buf[0]==SV_EXIT) {
		tmp=*(unsigned int *)(buf+1);
		if (tmp<1 || tmp>14) sprintf(xbuf,"STATUS: Server demands exit:\nunknown reason");
		sprintf(xbuf,"STATUS: Server demands exit:\n%s",logout_reason[tmp]);
		SetDlgItemText(hwnd,IDC_STATUS,xbuf);
		return -1;
	}

	if (buf[0]==SV_CAP) {
		tmp=*(unsigned int*)(buf+1);
		prio=*(unsigned int*)(buf+5);
		capcnt++;
		sprintf(xbuf,"STATUS: Player limit reached. Your place in queue: %d. Priority: %d. Try: %d",tmp,prio,capcnt);
		SetDlgItemText(hwnd,IDC_STATUS,xbuf);
		return 0;
	}

	if (buf[0]==SV_MOD1) { memcpy(mod,buf+1,15); return 0; }
	if (buf[0]==SV_MOD2) { memcpy(mod+15,buf+1,15); return 0; }
	if (buf[0]==SV_MOD3) { memcpy(mod+30,buf+1,15); return 0; }
	if (buf[0]==SV_MOD4) { memcpy(mod+45,buf+1,15); return 0; }
	if (buf[0]==SV_MOD5) { memcpy(mod+60,buf+1,15); return 0; }
	if (buf[0]==SV_MOD6) { memcpy(mod+75,buf+1,15); return 0; }
	if (buf[0]==SV_MOD7) { memcpy(mod+90,buf+1,15); return 0; }
	if (buf[0]==SV_MOD8) { 
		memcpy(mod+105,buf+1,15); 
		SetDlgItemText(hwnd,IDC_MOD,mod);
		return 0; 
	}

	return 0;
}

void convert(HWND hwnd);

void so_connect(HWND hwnd)
{
	WSADATA dummy;
	struct sockaddr_in addr;
	struct hostent *he;
	unsigned char buf[16];
	static int flag=0;
	unsigned long haddr;
	int one=1,tmp;

	if (so_status) return;
	so_status=1;

	//convert(hwnd); return;

	if (!flag) {
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Windows Sockets");
		if (WSAStartup((1<<8)+1,&dummy)) {
			SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Could not init Windows Sockets");
			so_status=0;
		}
		flag=1;
	}

	SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Initialising socket");
	sock=socket(PF_INET,SOCK_STREAM,0);
	if (sock==-1) {
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Could not init socket");
		so_status=0;
		return;
	}

	if (isdigit(host_addr[0])) {
		haddr=inet_addr(host_addr);
		if (haddr==INADDR_NONE) {
			SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Illegal IP Address");
			so_status=0;
			return;
		}
	} else {
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Getting server address");
		he=gethostbyname(host_addr);
		if (!he) {
			SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Server unknown");
			so_status=0;
			return;
		}
		haddr=*(unsigned long *)(&he->h_addr_list[0][0]);
	}

	addr.sin_family=AF_INET;
	addr.sin_port=htons(host_port);
	addr.sin_addr.s_addr=haddr;

	SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Connecting to server");
	if (connect(sock,(struct sockaddr *)&addr,sizeof(addr))) {
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Could not establish connection");
		so_status=0;
		return;
	}

	if (passwd[0]) {
		buf[0]=CL_PASSWD;
		memcpy(buf+1,passwd,15);
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Sending Password");
		if (send(sock,(char*)buf,16,0)<16) {
			SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Server closed connection (1).");
			so_status=0;
			return;
		}
	}

	if (okey.usnr) {
		buf[0]=CL_LOGIN;
		*(unsigned long*)(buf+1)=okey.usnr;
		*(unsigned long*)(buf+5)=okey.pass1;
		*(unsigned long*)(buf+9)=okey.pass2;
	} else {
		buf[0]=CL_NEWLOGIN;
	}

	SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Sending Login Info");
	if (send(sock,(char*)buf,16,0)<16) {
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Server closed connection (1).");
		so_status=0;
		return;
	}
	SetDlgItemText(hwnd,IDC_STATUS,"STATUS: Waiting for reply");

	do {
		if (xrecv(sock,buf,16,0)<16) {
			SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Server closed connection (2).");
			so_status=0;
			return;
		}
		tmp=so_login(buf,hwnd);
		if (tmp==-1) { so_status=0; close(sock); return; }
	} while (!tmp);

	Sleep(500);

	ioctlsocket(sock,FIONBIO,(u_long*)&one);

	zs.zalloc=Z_NULL;
	zs.zfree=Z_NULL;
	zs.opaque=Z_NULL;
	if (inflateInit(&zs)) {
		SetDlgItemText(hwnd,IDC_STATUS,"STATUS: ERROR: Compressor failure.");
		so_status=0;
		return;
	}


        EndDialog(hwnd,0);	

	return;
}

// ---------------------------------

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

void sv_newplayer(unsigned char *buf)
{
	DEBUG("SV NEWPLAYER");

	okey.usnr=*(unsigned long*)(buf+1);
	okey.pass1=*(unsigned long*)(buf+5);
	okey.pass2=*(unsigned long*)(buf+9);

	save_options();
}

void sv_setchar_name1(unsigned char *buf)
{
	DEBUG("SV SETCHAR NAME1");
	memcpy(pl.name,buf+1,15);
}

void sv_setchar_name2(unsigned char *buf)
{
	DEBUG("SV SETCHAR NAME2");
	memcpy(pl.name+15,buf+1,15);
}

void sv_setchar_name3(unsigned char *buf)
{
	DEBUG("SV SETCHAR NAME3");
	memcpy(pl.name+30,buf+1,10);
	strcpy(okey.name,pl.name);
	okey.race=*(unsigned long *)(buf+11);
	save_options();
}

void sv_setchar_mode(unsigned char *buf)
{
	DEBUG("SV SETCHAR MODE");
	pl.mode=buf[1];
}

void sv_setchar_hp(unsigned char *buf)
{
	DEBUG("SV SETCHAR HP");
	pl.hp[0]=*(unsigned short *)(buf+1);
	pl.hp[1]=*(unsigned short *)(buf+3);
	pl.hp[2]=*(unsigned short *)(buf+5);
	pl.hp[3]=*(unsigned short *)(buf+7);
	pl.hp[4]=*(unsigned short *)(buf+9);
	pl.hp[5]=*(unsigned short *)(buf+11);
}

void sv_setchar_endur(unsigned char *buf)
{
	DEBUG("SV SETCHAR ENDUR");
	pl.end[0]=*(short int*)(buf+1);
	pl.end[1]=*(short int*)(buf+3);
	pl.end[2]=*(short int*)(buf+5);
	pl.end[3]=*(short int*)(buf+7);
	pl.end[4]=*(short int*)(buf+9);
	pl.end[5]=*(short int*)(buf+11);
}

void sv_setchar_mana(unsigned char *buf)
{
	DEBUG("SV SETCHAR MANA");
	pl.mana[0]=*(short int*)(buf+1);
	pl.mana[1]=*(short int*)(buf+3);
	pl.mana[2]=*(short int*)(buf+5);
	pl.mana[3]=*(short int*)(buf+7);
	pl.mana[4]=*(short int*)(buf+9);
	pl.mana[5]=*(short int*)(buf+11);
}

void sv_setchar_attrib(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR ATTRIB");

	n=buf[1];
	if (n<0 || n>4) return;

	pl.attrib[n][0]=buf[2];
	pl.attrib[n][1]=buf[3];
	pl.attrib[n][2]=buf[4];
	pl.attrib[n][3]=buf[5];
	pl.attrib[n][4]=buf[6];
	pl.attrib[n][5]=buf[7];
}

void sv_setchar_skill(unsigned char *buf)
{
	int n;
	extern int skill_cmp(const void *a,const void *b);

	DEBUG("SV SETCHAR SKILL");

	n=buf[1];
	if (n<0 || n>49) return;

	pl.skill[n][0]=buf[2];
	pl.skill[n][1]=buf[3];
	pl.skill[n][2]=buf[4];
	pl.skill[n][3]=buf[5];
	pl.skill[n][4]=buf[6];
	pl.skill[n][5]=buf[7];

	qsort(skilltab,50,sizeof(struct skilltab),skill_cmp);
}

void sv_setchar_ahp(unsigned char *buf)
{
	DEBUG("SV SETCHAR AHP");
	pl.a_hp=*(unsigned short*)(buf+1);
}

void sv_setchar_aend(unsigned char *buf)
{
	DEBUG("SV SETCHAR AEND");
	pl.a_end=*(unsigned short*)(buf+1);
}

void sv_setchar_amana(unsigned char *buf)
{
	DEBUG("SV SETCHAR AMAANA");
	pl.a_mana=*(unsigned short*)(buf+1);
}

void sv_setchar_dir(unsigned char *buf)
{
	DEBUG("SV SETCHAR DIR");
	pl.dir=*(unsigned char*)(buf+1);
}

void sv_setchar_pts(unsigned char *buf)
{
	DEBUG("SV SETCHAR PTS");
	pl.points=*(unsigned long*)(buf+1);
	pl.points_tot=*(unsigned long*)(buf+5);
	pl.kindred=*(unsigned long*)(buf+9);
}

void sv_setchar_gold(unsigned char *buf)
{
	DEBUG("SV SETCHAR GOLD");
	pl.gold=*(unsigned long*)(buf+1);
	pl.armor=*(unsigned long*)(buf+5);
	pl.weapon=*(unsigned long*)(buf+9);
}

void sv_setchar_item(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR ITEM");

	n=*(unsigned long*)(buf+1);
	if (n<0 || n>39) xlog(0,"Invalid setchar item");
	pl.item[n]=*(short int*)(buf+5);
	pl.item_p[n]=*(short int*)(buf+7);

//	xlog("SV SETCHAR ITEM (%d,%d,%d)",*(unsigned long*)(buf+1),*(short int*)(buf+5),*(short int*)(buf+7));
}

void sv_setchar_worn(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR WORN");

	n=*(unsigned long*)(buf+1);
	if (n<0 || n>19) xlog(0,"Invalid setchar worn");
	pl.worn[n]=*(short int*)(buf+5);
	pl.worn_p[n]=*(short int*)(buf+7);
}

void sv_setchar_spell(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR SPELL");

	n=*(unsigned long*)(buf+1);
	if (n<0 || n>19) xlog(0,"Invalid setchar spell");
	pl.spell[n]=*(short int*)(buf+5);
	pl.active[n]=*(short int*)(buf+7);
}

void sv_setchar_obj(unsigned char *buf)
{
	DEBUG("SV SETCHAR OBJ");

	pl.citem=*(short int*)(buf+1);
	pl.citem_p=*(short int*)(buf+3);

//	xlog("SV SETCHAR OBJ (%d,%d)",*(short int*)(buf+1),*(short int*)(buf+3));
}

static int lastn=0;

int sv_setmap(unsigned char *buf,int off)
{
	int n,p;
	static int cnt[8]={0,0,0,0,0,0,0,0};

	DEBUG("SV SETMAP");

	if (off) {
		n=lastn+off;
		p=2;
	} else {
		n=*(unsigned short*)(buf+2);
		p=4;
	}
	
	if (n<0 || n>=TILEX*TILEY) { xlog(0,"corrupt setmap!"); return -1; }

	lastn=n;
	if (!buf[1]) { DEBUG("Warning: no flags in setmap!"); return -1; }

	if (buf[1]&1) {
		map[n].ba_sprite=*(unsigned short*)(buf+p); p+=2;
		cnt[0]++;
	}
	if (buf[1]&2) {
		map[n].flags=*(unsigned int*)(buf+p); p+=4;
		cnt[1]++;
	}
	if (buf[1]&4) {
		map[n].flags2=*(unsigned int*)(buf+p); p+=4;
		cnt[2]++;
	}
	if (buf[1]&8) {
		map[n].it_sprite=*(unsigned short*)(buf+p); p+=2;
		cnt[3]++;
	}
	if (buf[1]&16) {
		map[n].it_status=*(unsigned char*)(buf+p); p+=1;
		cnt[4]++;
	}
	if (buf[1]&32) {
		map[n].ch_sprite=*(unsigned short*)(buf+p); p+=2;
		map[n].ch_status=*(unsigned char*)(buf+p); p+=1;
		map[n].ch_stat_off=*(unsigned char*)(buf+p); p+=1;
		cnt[5]++;
	}
	if (buf[1]&64) {
		map[n].ch_nr=*(unsigned short*)(buf+p); p+=2;
		map[n].ch_id=*(unsigned short*)(buf+p); p+=2;
		map[n].ch_speed=*(unsigned char*)(buf+p); p+=1;
		cnt[6]++;
	}
	if (buf[1]&128) {
		map[n].ch_proz=*(unsigned char*)(buf+p); p+=1;
		cnt[7]++;
	}	
	return p;
}

int sv_setmap3(unsigned char *buf,int cnt)
{
	int n,m,p;
	unsigned char tmp;

	//printf("cnt=%d, ",cnt); 
	DEBUG("SV SETMAP3"); 
        
	n=(*(unsigned short*)(buf+1))&2047;
	tmp=(*(unsigned short*)(buf+1))>>12;
	if (n<0 || n>=TILEX*TILEY) { xlog(0,"corrupt setmap3!"); return -1; }

	map[n].light=tmp;	

	for (m=n+2,p=3; m<n+cnt+2; m+=2,p++) {
		if (m<TILEX*TILEY) {
			tmp=*(unsigned char*)(buf+p);
		
			map[m].light=(unsigned char)(tmp&15);
			map[m-1].light=(unsigned char)(tmp>>4);
		}		
	}

	return p;
}

void sv_setorigin(unsigned char *buf)
{
	int x,y,xp,yp,n;

	DEBUG("SV SETORIGIN");

	xp=*(short*)(buf+1);
	yp=*(short*)(buf+3);

	for (y=n=0; y<TILEY; y++) {
		for (x=0; x<TILEX; x++,n++) {
			map[n].x=(unsigned short)(x+xp);
			map[n].y=(unsigned short)(y+yp);
		}
	}	
}

void sv_tick(unsigned char *buf)
{
	DEBUG2("SV TICK");
	ctick=*(unsigned char*)(buf+1);
}

void sv_log(unsigned char *buf,int font)
{
	static char text[512];
	static int cnt=0;
	int n;

	DEBUG("SV LOG");

	memcpy(text+cnt,buf+1,15);

	for (n=cnt; n<cnt+15; n++)
		if (text[n]==10) {
			text[n]=0;
			tlog(text,font);
			cnt=0;
			return;
		}
	cnt+=15;

	if (cnt>500) {
		xlog(0,"sv_log(): cnt too big!");

		text[cnt]=0;
		xlog(1,text);
		cnt=0;
	}
}

#pragma argsused
void sv_scroll_right(unsigned char *buf)
{
	DEBUG("SV SCROLL_RIGHT");

	memmove(map,map+1,sizeof(struct cmap)*(TILEX*TILEY-1));
}

#pragma argsused
void sv_scroll_left(unsigned char *buf)
{
	DEBUG("SV SCROLL_LEFT");

	memmove(map+1,map,sizeof(struct cmap)*(TILEX*TILEY-1));
}

#pragma argsused
void sv_scroll_down(unsigned char *buf)
{
	DEBUG("SV SCROLL_DOWN");

	memmove(map,map+TILEX,sizeof(struct cmap)*(TILEX*TILEY-TILEX));
}

#pragma argsused
void sv_scroll_up(unsigned char *buf)
{
	DEBUG("SV SCROLL_UP");

	memmove(map+TILEX,map,sizeof(struct cmap)*(TILEX*TILEY-TILEX));
}

#pragma argsused
void sv_scroll_leftup(unsigned char *buf)
{
	DEBUG("SV SCROLL_LEFTUP");

	memmove(map+TILEX+1,map,sizeof(struct cmap)*(TILEX*TILEY-TILEX-1));
}

#pragma argsused
void sv_scroll_leftdown(unsigned char *buf)
{
	DEBUG("SV SCROLL_LEFTDOWN");

	memmove(map,map+TILEX-1,sizeof(struct cmap)*(TILEX*TILEY-TILEX+1));
}

#pragma argsused
void sv_scroll_rightup(unsigned char *buf)
{
	DEBUG("SV SCROLL_RIGHTUP");

	memmove(map+TILEX-1,map,sizeof(struct cmap)*(TILEX*TILEY-TILEX+1));
}

#pragma argsused
void sv_scroll_rightdown(unsigned char *buf)
{
	DEBUG("SV SCROLL_RIGHTDOWN");

	memmove(map,map+TILEX+1,sizeof(struct cmap)*(TILEX*TILEY-TILEX-1));
}

void sv_look1(unsigned char *buf)
{
	DEBUG("SV LOOK1");

	tmplook.worn[0]=*(unsigned short*)(buf+1);
	tmplook.worn[2]=*(unsigned short*)(buf+3);
	tmplook.worn[3]=*(unsigned short*)(buf+5);
	tmplook.worn[5]=*(unsigned short*)(buf+7);
	tmplook.worn[6]=*(unsigned short*)(buf+9);
	tmplook.worn[7]=*(unsigned short*)(buf+11);
	tmplook.worn[8]=*(unsigned short*)(buf+13);
	tmplook.autoflag=*(unsigned char*)(buf+15);
}

void sv_look2(unsigned char *buf)
{
	DEBUG("SV LOOK2");

	tmplook.worn[9]=*(unsigned short*)(buf+1); // 1 2
	tmplook.sprite=*(unsigned short*)(buf+3); // 3 4
	tmplook.points=*(unsigned int*)(buf+5); // 5 6 7 8
	tmplook.hp=*(unsigned int*)(buf+9); //9 10 11 12
	tmplook.worn[10]=*(unsigned short*)(buf+13); // 13 14
}

void sv_look3(unsigned char *buf)
{
	DEBUG("SV LOOK3");

	tmplook.end=*(unsigned short*)(buf+1);
	tmplook.a_hp=*(unsigned short*)(buf+3);
	tmplook.a_end=*(unsigned short*)(buf+5);
	tmplook.nr=*(unsigned short*)(buf+7);
	tmplook.id=*(unsigned short*)(buf+9);
	tmplook.mana=*(unsigned short*)(buf+11);
	tmplook.a_mana=*(unsigned short*)(buf+13);
}

void sv_look4(unsigned char *buf)
{
	DEBUG("SV LOOK4");

	tmplook.worn[1]=*(unsigned short*)(buf+1);
	tmplook.worn[4]=*(unsigned short*)(buf+3);
	tmplook.extended=buf[5];
	tmplook.pl_price=*(unsigned int*)(buf+6);
	tmplook.worn[11]=*(unsigned short*)(buf+10);
	tmplook.worn[12]=*(unsigned short*)(buf+12);
	tmplook.worn[13]=*(unsigned short*)(buf+14);
}

void sv_look5(unsigned char *buf)
{
	int n;

	DEBUG("SV LOOK5");

	for (n=0; n<15; n++) tmplook.name[n]=buf[n+1];
	tmplook.name[15]=0;

	if (!tmplook.extended) {
		if (!tmplook.autoflag) {
			show_look=1;
			look=tmplook;
			look_timer=10*TICKS;
		}
		add_look(tmplook.nr,tmplook.name,tmplook.id);
	}
}

void sv_look6(unsigned char *buf)
{
	int n,s;

	DEBUG("SV LOOK6");

	s=buf[1];

	for (n=s; n<min(62,s+2); n++) {
		tmplook.item[n]=*(unsigned short *)(buf+2+(n-s)*6);
		tmplook.price[n]=*(unsigned int *)(buf+4+(n-s)*6);
	}
	if (n==62) {
		show_shop=1;
		shop=tmplook;
	}
}

void sv_settarget(unsigned char *buf)
{
	DEBUG("SV SETTARGET");

	pl.attack_cn=*(unsigned short*)(buf+1);
	pl.goto_x=*(unsigned short*)(buf+3);
	pl.goto_y=*(unsigned short*)(buf+5);
	pl.misc_action=*(unsigned short*)(buf+7);
	pl.misc_target1=*(unsigned short*)(buf+9);
	pl.misc_target2=*(unsigned short*)(buf+11);
}

void sv_playsound(unsigned char *buf)
{
	int nr,vol,pan;
	char name[80];

	DEBUG("SV PLAYSOUND");

	nr=*(unsigned int*)(buf+1);
	vol=*(int*)(buf+5);
	pan=*(int*)(buf+9);

//  xlog(1,"sample=%d, pan=%d, vol=%d",nr,pan,vol);

	sprintf(name,"sfx\\%d.wav",nr);
	play_sound(name,vol,-pan);		// add flag to reverse channels!!
}

void sv_exit(unsigned char *buf)
{
	int reason;

	DEBUG("SV EXIT");

	reason=*(unsigned int*)(buf+1);

	xlog(1," ");
	if (reason<1 || reason>12) xlog(1,"EXIT: Reason unknown.");
	else xlog(1,"EXIT: %s",logout_reason[reason]);

	do_exit=1;
}

void sv_load(unsigned char *buf)
{
	extern int load;

	DEBUG("SV LOAD");

	load=*(unsigned int*)(buf+1);
}

void sv_unique(unsigned char *buf)
{
	extern int load;

	DEBUG("SV UNIQUE");

	unique1=*(unsigned int*)(buf+1);
	unique2=*(unsigned int*)(buf+5);
	save_unique();
}

int sv_ignore(unsigned char *buf)
{
	int size,d;
	static int cnt=0,got=0,start=0;

	size=*(unsigned int*)(buf+1);
	got+=size;

	if (!start) start=time(NULL);	

	if (cnt++>16) {
		cnt=0;
		d=time(NULL)-start;
		if (d==0) d=1;
		
                xlog(3,"ignore=%d, got=%d, tps=%.2fK/s",size,got,(double)got/d/1024.0);
	}
	
	return size;
}

int sv_cmd(unsigned char *buf)
{

	if (buf[0]&SV_SETMAP) return sv_setmap(buf,buf[0]&~SV_SETMAP);

	switch(buf[0]) {
		case	SV_SETCHAR_NAME1:	sv_setchar_name1(buf); break;
		case	SV_SETCHAR_NAME2:	sv_setchar_name2(buf); break;
		case	SV_SETCHAR_NAME3:	sv_setchar_name3(buf); break;
		case	SV_SETCHAR_MODE:	sv_setchar_mode(buf); return 2;
		case	SV_SETCHAR_ATTRIB:	sv_setchar_attrib(buf); return 8;
		case	SV_SETCHAR_SKILL:	sv_setchar_skill(buf); return 8;
		case	SV_SETCHAR_HP:		sv_setchar_hp(buf); return 13;
		case	SV_SETCHAR_ENDUR:	sv_setchar_endur(buf); return 13;
		case	SV_SETCHAR_MANA:	sv_setchar_mana(buf); return 13;
		case	SV_SETCHAR_AHP:		sv_setchar_ahp(buf); return 3;
		case	SV_SETCHAR_AEND:       	sv_setchar_aend(buf); return 3;
		case	SV_SETCHAR_AMANA:	sv_setchar_amana(buf); return 3;
		case	SV_SETCHAR_DIR:		sv_setchar_dir(buf); return 2;

		case	SV_SETCHAR_PTS:		sv_setchar_pts(buf); return 13;
		case	SV_SETCHAR_GOLD:	sv_setchar_gold(buf); return 13;
		case	SV_SETCHAR_ITEM:	sv_setchar_item(buf); return 9;
		case	SV_SETCHAR_WORN:	sv_setchar_worn(buf); return 9;
		case	SV_SETCHAR_SPELL:	sv_setchar_spell(buf); return 9;
		case	SV_SETCHAR_OBJ:		sv_setchar_obj(buf); return 5;

		case	SV_SETMAP3:		return sv_setmap3(buf,26);
		case	SV_SETMAP4:		return sv_setmap3(buf,0);
		case	SV_SETMAP5:		return sv_setmap3(buf,2);
		case	SV_SETMAP6:		return sv_setmap3(buf,6);
		case	SV_SETORIGIN:		sv_setorigin(buf); return 5;

		case	SV_TICK:		sv_tick(buf); return 2;

		case	SV_LOG0:		sv_log(buf,0); break;
		case	SV_LOG1:		sv_log(buf,1); break;
		case	SV_LOG2:		sv_log(buf,2); break;
		case	SV_LOG3:		sv_log(buf,3); break;

		case	SV_SCROLL_RIGHT:	sv_scroll_right(buf); return 1;
		case	SV_SCROLL_LEFT:		sv_scroll_left(buf); return 1;
		case	SV_SCROLL_DOWN:		sv_scroll_down(buf); return 1;
		case	SV_SCROLL_UP:		sv_scroll_up(buf); return 1;

		case	SV_SCROLL_RIGHTDOWN:	sv_scroll_rightdown(buf); return 1;
		case	SV_SCROLL_RIGHTUP:		sv_scroll_rightup(buf); return 1;
		case	SV_SCROLL_LEFTDOWN:		sv_scroll_leftdown(buf); return 1;
		case	SV_SCROLL_LEFTUP:		sv_scroll_leftup(buf); return 1;

		case	SV_LOOK1:				sv_look1(buf); break;
		case	SV_LOOK2:				sv_look2(buf); break;
		case	SV_LOOK3:				sv_look3(buf); break;
		case	SV_LOOK4:				sv_look4(buf); break;
		case	SV_LOOK5:				sv_look5(buf); break;
		case	SV_LOOK6:				sv_look6(buf); break;

		case	SV_SETTARGET:			sv_settarget(buf); return 13;

		case	SV_PLAYSOUND:			sv_playsound(buf); return 13;

		case	SV_EXIT:				sv_exit(buf); break;

		case  	SV_LOAD:             	sv_load(buf); return 5;

		case  	SV_UNIQUE:             	sv_unique(buf); return 9;
		case 	SV_IGNORE:		return sv_ignore(buf);

		default: 			xlog(0,"Unknown SV: %d",buf[0]); return -1;
	}

	return 16;
}

#pragma argused
void so_perf_report(int ticksize,int skip,int idle)
{
	unsigned char buf[16];

	buf[0]=CL_PERF_REPORT;
	*(unsigned short*)(buf+1)=(unsigned short)0;
	*(unsigned short*)(buf+3)=(unsigned short)0;
	*(unsigned short*)(buf+5)=(unsigned short)0;
	*(float*)(buf+7)=0;
	xsend(buf);
}

int non_critical_socket_error(void)
{
    int err;

    err=WSAGetLastError();
    WSASetLastError(0);

    switch(err) {
        case 0:
        case WSAEWOULDBLOCK:
        case WSAEINTR:
        case WSAEINPROGRESS:        return 1;

        case WSAEFAULT:
        case WSAENETDOWN:
        case WSAENOTCONN:
        case WSAENETRESET:
        case WSAENOTSOCK:
        case WSAEOPNOTSUPP:
        case WSAESHUTDOWN:
        case WSAEMSGSIZE:
        case WSAEINVAL:
        case WSAECONNABORTED:
        case WSAETIMEDOUT:
        case WSAECONNRESET:
        case WSANOTINITIALISED:    return 0;
    }

    return 0;
}

void xsend(unsigned char *buf)
{
	int len=0,ret;

	while (len<16) {
		ret=send(sock,buf+len,16-len,0);
        if (ret<0) {
			 if (!non_critical_socket_error()) so_error("transmit buffer overflow");
			 do_msg();
			 continue;
		}
		if (ret!=16) do_msg();
		len+=ret;
	}
}

#define TSIZE	(8192*16)

unsigned char tickbuf[TSIZE];
int ticksize=0;		// amount of data in tickbuf
int tickstart=0;	// start index to scan buffer for next tick

//#define TRANS 1024

#ifdef TRANS
int tmptime=0,tmpsize=0;
#endif

int game_loop(void)
{
	int ret,tmp;

#ifdef TRANS
	if (tmptime!=(tmp=time(NULL))) {
		xlog(2,"tmpsize=%d",tmpsize);
		tmptime=tmp;
		tmpsize=0;
	}
#endif
        
	while (1) {
#ifdef TRANS
		ret=recv(sock,tickbuf+ticksize,min(TRANS-tmpsize,TSIZE-ticksize),0);
#else
		ret=recv(sock,tickbuf+ticksize,TSIZE-ticksize,0);
#endif
		if (ret<0) {
			 if (!non_critical_socket_error()) so_error("receive error");
			 ret=0;
		}
#ifdef TRANS
		tmpsize+=ret;
#endif
                ticksize+=ret;

                if (ticksize>=tickstart+2) {
			tmp=*(unsigned short*)(tickbuf+tickstart);
			tmp&=0x7fff;
			if (tmp<2) so_error("transmission corrupt");
			tickstart+=tmp;
			t_size++;
		} else break;	        
		do_msg();
	}	

	return 0;	// no more work
}

int tick_do(void)
{
	int len,idx=0,ret,csize,comp;
	static char buf[65536];
	static int ctot=1,utot=1,t=0,td;

	if (!t) t=time(NULL);
        
        len=*(unsigned short*)(tickbuf);
	comp=len&0x8000;
	len&=0x7fff;
	ctot+=len;
        if (len>ticksize) return 0;

        if (comp) {		
		zs.next_in=tickbuf+2;
		zs.avail_in=len-2;
	
		zs.next_out=buf;
		zs.avail_out=65536;

                ret=inflate(&zs,Z_SYNC_FLUSH);
		if (ret!=Z_OK) { xlog(0,"uncompress error %d!",ret); }
		
		if (zs.avail_in) { xlog(0,"uncompress: avail is %d!!\n",zs.avail_in); }

                csize=65536-zs.avail_out;				
	} else {
		csize=len-2;
		if (csize) memcpy(buf,tickbuf+2,csize);                                
	}

	utot+=csize;

	td=time(NULL)-t;
	if (!td) td=1;
        
	lastn=-1;	// reset sv_setmap
	ctick++; if (ctick>19) ctick=0;
	
        while (idx<csize) {
		ret=sv_cmd(buf+idx);
		if (ret==-1) { xlog(1,"Warning: syntax error in server data"); DEBUG("Warning: syntax error in server data"); exit(1); }
		idx+=ret;
	}

	ticksize-=len;
	tickstart-=len;
	t_size--;
        if (ticksize) memmove(tickbuf,tickbuf+len,ticksize);

        engine_tick();
        
	return 1;
}

static void save_unique(void)
{
	HKEY hk;

	if (RegCreateKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Notepad",&hk)!=ERROR_SUCCESS) return;

	RegSetValueEx(hk,"fStyle1",0,REG_DWORD,(void*)&unique1,4);
	RegSetValueEx(hk,"fStyle2",0,REG_DWORD,(void*)&unique2,4);
}

static void load_unique(void)
{
	HKEY hk;
	int size=4,type;

	if (RegCreateKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Notepad",&hk)!=ERROR_SUCCESS) return;

	RegQueryValueEx(hk,"fStyle1",0,(void*)&type,(void*)&unique1,(void*)&size);
	RegQueryValueEx(hk,"fStyle2",0,(void*)&type,(void*)&unique2,(void*)&size);
}
