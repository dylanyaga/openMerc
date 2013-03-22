#include <stdio.h>
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include "ddraw.h"
#include <process.h>
#include <signal.h>
#pragma hdrstop
#include "dd.h"
#include "common.h"
#include "inter.h"
//#include "minilzo.h"

extern int RED,GREEN,BLUE,RGBM,MAXXOVER;
extern char *DDERR;
extern int dd_cache_hit,dd_cache_miss,MAXCACHE,invisible,cachex,cachey,MAXXOVER;
extern int pskip,pidle;
extern int maxmem,usedmem,maxvid,usedvid;
extern int noshop;
void dd_invalidate_cache(void);
void conv_init(void);
int init_pnglib(void);

extern int cursor_type;
HCURSOR cursor[10];

void cmd(int cmd,int x,int y);

int quit=0;
char host_addr[84]={MHOST};
int host_port=5555;

extern char path[];
extern int tricky_flag;

HWND desk_hwnd;
HINSTANCE hinst;

int init_sound(HWND hwnd);
void engine(void);

#define MWORD 2048

char input[128];
int in_len=0;
int cur_pos=0;
int hist_nr=0;
int view_pos=0;
int tabmode=0;
int tabstart=0;
int logstart=0;
int logtimer=0;
int do_alpha=2;
int do_shadow=1;

void dd_invalidate_alpha(void);

char history[20][128];
int hist_len[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char words[MWORD][40];

#define xisalpha(a) (((a)=='#') || (isalpha(a)))

void complete_word(void)
{
	int n=0,z,pos;
	char buf[40];

	if (cur_pos<1) return;

	for (z=cur_pos-1; z>=0; z--) if (!xisalpha(input[z])) {
			z++; break;
		}
	if (z<0) z=0;
	while (z<cur_pos && n<39) buf[n++]=input[z++];
	buf[n]=0;

	if (n<1) return;

	for (z=tabstart; z<MWORD; z++) {
		if (!strncmp(buf,words[z],n) && strlen(words[z])>(unsigned)n) {
			pos=cur_pos;
			while (pos<115 && words[z][n]) input[pos++]=words[z][n++];
			if (pos<115) input[pos++]=' ';
			in_len=pos;
			tabmode=1;
			tabstart=z+1;
			return;
		}
	}
	tabmode=0;
	tabstart=0;
	in_len=cur_pos;
}

void add_word(char *buf)
{
	int n;

	for (n=0; n<MWORD-1; n++)
		if (!strcmp(words[n],buf)) break;

	memmove(words[1],words[0],n*40);
	memcpy(words[0],buf,40);
}

void add_words(void)
{
	char buf[40];
	int z1=0,z2;

	while (input[z1]) {
		z2=0;
		while (xisalpha(input[z1]) && z2<39) {
			buf[z2++]=input[z1++];
		}
		buf[z2]=0;
		add_word(buf);
		while (input[z1] && !xisalpha(input[z1])) z1++;
	}
}

// CTL3D
void pascal (*ctl3don)(HANDLE,short int)=NULL;
HBRUSH dlg_back;
int dlg_col,dlg_fcol;

extern int blockcnt,blocktot,blockgc;
int mx=0,my=0;

void say(char *input)
{
	int n;
	char buf[16];
	buf[0]=CL_CMD_INPUT1;
	for (n=0; n<15; n++)
		buf[n+1]=input[n];
	xsend(buf);

	buf[0]=CL_CMD_INPUT2;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+15];
	xsend(buf);

	buf[0]=CL_CMD_INPUT3;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+30];
	xsend(buf);

	buf[0]=CL_CMD_INPUT4;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+45];
	xsend(buf);

	buf[0]=CL_CMD_INPUT5;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+60];
	xsend(buf);

	buf[0]=CL_CMD_INPUT6;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+75];
	xsend(buf);

	buf[0]=CL_CMD_INPUT7;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+90];
	xsend(buf);

	buf[0]=CL_CMD_INPUT8;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+105];
	xsend(buf);
}

int do_ticker=1;
extern int gamma;
extern int usedvidmem;

extern int alphapix,fullpix;

LRESULT FAR PASCAL _export MainWndProc(HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	int keys;
	extern int ser_ver,xmove;

	keys=0;
	if (GetAsyncKeyState(VK_SHIFT)&0x8000) keys|=1;
	if (GetAsyncKeyState(VK_CONTROL)&0x8000) keys|=2;

	switch (message) {
		case    WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			switch ((int)wParam) {
				case    27:         cmd(CL_CMD_RESET,0,0); show_shop=0; noshop=QSIZE*12; xmove=0; break;	//ESC
				case    'p':            if (keys) button_command(16);
					else cmd(CL_CMD_MODE,2,0); break;		//F1
				case    'q':            if (keys) button_command(17);
					else cmd(CL_CMD_MODE,1,0); break;		//F2
				case    'r':            if (keys) button_command(18);
					else cmd(CL_CMD_MODE,0,0); break;		//F3
				case    's':            if (keys) button_command(19);
					else pdata.show_proz=1-pdata.show_proz; break;	//F4
				case    't':            if (keys) button_command(20);			//F5
					else do_alpha++; if (do_alpha==3) do_alpha=0; dd_invalidate_alpha(); break;
				case    'u':            if (keys) button_command(21);
					else pdata.hide=1-pdata.hide; break;		//F6
				case    'v':            if (keys) button_command(22);
					else pdata.show_names=1-pdata.show_names; break;//F7
				case    'w':            if (keys) button_command(23);break;			   //F8
				case    'x':            if (keys) button_command(24);
					else dd_savescreen(); break;						   //F9
				case  'y':        if (keys)	button_command(25);
					else {
						gamma+=250;
						if (gamma>6000)	gamma=5000;
						xlog(2,"Set gamma correction to %1.2f",gamma/5000.0);
						dd_invalidate_cache();
					}
					break;		// F10
				case    'z':        if (keys) button_command(26);
					else {
						xlog(2," ");											//F11
						xlog(2,"Client Version %d.%02d.%02d",VERSION>>16,(VERSION>>8)&255,VERSION&255);
						xlog(2,"Server Version %d.%02d.%02d",ser_ver>>16,(ser_ver>>8)&255,ser_ver&255);
						xlog(2,"MAXX=%d, MAXY=%d, MAXXO=%d",MAXX,MAXY,MAXXOVER);
						xlog(2,"R=%04X, G=%04X, B=%04X",RED,GREEN,BLUE);
						xlog(2,"RGBM=%d",RGBM);
						xlog(2,"MAXCACHE=%d",MAXCACHE);
						xlog(2,"Hit=%d, Miss=%d, Invis=%d",dd_cache_hit,dd_cache_miss,invisible);
						xlog(2,"Ratio=%.2f%%",100.0/(dd_cache_hit+dd_cache_miss)*dd_cache_hit);
						xlog(2,"Skip=%d%% Idle=%d%%",pskip,pidle);
						xlog(2,"MaxMem=%dK, UsedMem=%dK",maxmem>>10,usedmem>>10);
						xlog(2,"MemBlocks=%d (T=%d,GC=%d)",blockcnt,blocktot,blockgc);
						xlog(2,"MaxVid=%dK, UsedVid=%dK",(maxvid*32*32*2)>>10,(usedvid*32*32*2)>>10);
						xlog(2,"cachex=%d, cachey=%d, MAXXOVER=%d",cachex,cachey,MAXXOVER);
						xlog(2,"usedvidmemflag=%d",usedvidmem);
						xlog(2,"alphapix=%d, fullpix=%d, ratio=%.2f",alphapix,fullpix,100.0/(alphapix+fullpix+1)*alphapix);

//                                do_ticker=1-do_ticker;
					}
					break;

				case    '{':            if (keys) button_command(27);
					else cmd_exit();
					break;		   //F12


					// text editor
				case  9:          complete_word();
					break;

				case    8:              if (cur_pos && in_len) { //BACKSPACE
						if (tabmode) {
							in_len=cur_pos; tabmode=0; tabstart=0;
						}
						if (cur_pos>in_len)	cur_pos=in_len;
						memmove(input+cur_pos-1,input+cur_pos,120-cur_pos);
						in_len--;
						cur_pos--;
					}
					break;
				case 46:             if (in_len) { // DEL
						if (tabmode) {
							in_len=cur_pos; tabmode=0; tabstart=0;
						} else {
							memmove(input+cur_pos,input+cur_pos+1,120-cur_pos);
							in_len--;
						}
					}
					break;
				case 33:          if (logstart<22*8) {
						logstart+=11; logtimer=TICKS*30;
					}
					break;
				case 34:          if (logstart>0) {
						logstart-=11; logtimer=TICKS*30;
					}
					break;
				case 36:          cur_pos=0; tabmode=0; tabstart=0; break; // HOME
				case 35:          cur_pos=in_len; tabmode=0; tabstart=0; break;	// END
				case 37:          if (cur_pos) cur_pos--;
					tabmode=0; tabstart=0;
					break;
				case 39:          if (cur_pos<115) cur_pos++;
					tabmode=0; tabstart=0;
					break;
				case 38:          if (hist_nr<19) {
						memcpy(history[hist_nr],input,128);
						hist_len[hist_nr]=in_len;
						hist_nr++;

						memcpy(input,history[hist_nr],128);
						in_len=cur_pos=hist_len[hist_nr];

						tabmode=0; tabstart=0;
					}
					break;
				case 40:          if (hist_nr>0) {
						memcpy(history[hist_nr],input,128);
						hist_len[hist_nr]=in_len;

						hist_nr--;

						memcpy(input,history[hist_nr],128);
						in_len=cur_pos=hist_len[hist_nr];

						tabmode=0; tabstart=0;
					}
					break;
/*           default:          xlog(3,"key=%d",(int)wParam);
							 break;*/

			}
			break;

		case WM_CHAR:
			switch ((int)wParam) {
				case    13:            if (in_len==0) break;

					if (tabmode) {
						tabmode=0; tabstart=0;
						in_len--;
					}

					memmove(history[2],history[1],18*128);
					memmove(&hist_len[2],&hist_len[1],sizeof(int)*18);

					memcpy(history[1],input,128);
					hist_len[1]=in_len;

					input[in_len]=0;
					in_len=0;
					cur_pos=0;
					view_pos=0;
					hist_nr=0;

					add_words();

					say(input);

					break;

				default:                if ((int)wParam>31 && (int)wParam<128 && in_len<115) {
						if (tabmode) {
							if (!isalnum((char)wParam))	in_len--;
							cur_pos=in_len;
							tabmode=0;
							tabstart=0;
						}
						if (cur_pos>in_len)	cur_pos=in_len;
						memmove(input+cur_pos+1,input+cur_pos,120-cur_pos);
						input[cur_pos]=(char)wParam;
						in_len++;
						cur_pos++;
					}
					break;
			}
			break;

		case WM_PAINT:
			BeginPaint(hWnd,&ps);
			EndPaint(hWnd,&ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_MOUSEMOVE:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_MOVE);
			mx=LOWORD(lParam);
			my=HIWORD(lParam);
			break;

		case WM_LBUTTONDOWN:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_LB_DOWN);
			break;

		case WM_LBUTTONUP:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_LB_UP);
			break;

		case WM_RBUTTONDOWN:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_RB_DOWN);
			break;

		case WM_RBUTTONUP:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_RB_UP);
			break;

		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}


HWND InitWindow(HINSTANCE hInstance,int nCmdShow)
{
	WNDCLASS wc;
	HWND hWnd;
	char buf[256];
	int n;

	hinst=hInstance;

	for (n=1; n<10; n++)
		cursor[n]=LoadCursor(hInstance,MAKEINTRESOURCE(n));

	wc.style=CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc=(long (FAR PASCAL*)())MainWndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(1));
	wc.hCursor=NULL;
	wc.hbrBackground=NULL;
	wc.lpszMenuName=NULL;
	wc.lpszClassName="DDCWin";

	RegisterClass(&wc);

	sprintf(buf,MNAME" v%d.%02d.%02d",
			VERSION>>16,(VERSION>>8)&255,VERSION&255);

	desk_hwnd=hWnd=CreateWindowEx(
#ifndef DB_WINDOWED
								 WS_EX_TOPMOST,
#else
								 0,
#endif
								 "DDCWin",
								 buf,
#ifndef DB_WINDOWED
								 WS_VISIBLE|WS_BORDER|WS_POPUP|WS_SYSMENU,
								 10,10,0,0,
#else
								 WS_VISIBLE|WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX,
								 10,10,802,621,
#endif
								 NULL,
								 NULL,
								 hInstance,
								 NULL);

        SetFocus(hWnd);
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

int parse_cmd(char *s)
{
	int n;

	while (isspace(*s))	s++;
	while (*s) {
		if (*s=='-') {
			s++;
			if (tolower(*s)=='t') {
				s++;
				tricky_flag=1;
			} else if (tolower(*s)=='d') {
				s++;
				while (isspace(*s)) s++;
				n=0; while (n<150 && *s && !isspace(*s)) path[n++]=*s++;
				if (path[n]!='\\') path[n++]='\\';
				path[n]=0;
			} else if (tolower(*s)=='p') {
				s++;
				while (isspace(*s)) s++;
				host_port=atoi(s);
				while (*s && !isspace(*s)) s++;
			} else return -1;
		} else return -2;
		while (isspace(*s))	s++;
	}
	return 1;
}

void log_system_data(void)
{
	char buf[256];
	unsigned int langid,lcid,size=80;
	char systemdir[256],windir[256],cdir[256],user[256],computer[256];

	langid=GetSystemDefaultLangID();
	lcid=GetSystemDefaultLCID();

	GetSystemDirectory(systemdir,80);
        GetWindowsDirectory(windir,80);
	GetCurrentDirectory(80,cdir);
	GetUserName((void*)user,&size); size=80;
	GetComputerName((void*)computer,&size);

	sprintf(buf,"|langid=%u, lcid=%u",langid,lcid); say(buf);
	sprintf(buf,"|systemdir=\"%s\"",systemdir); say(buf);
	sprintf(buf,"|windowsdir=\"%s\"",windir); say(buf);
	sprintf(buf,"|currentdir=\"%s\"",cdir); say(buf);
	sprintf(buf,"|username=\"%s\"",user); say(buf);
	sprintf(buf,"|computername=\"%s\"",computer); say(buf);
}

#pragma argsused
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd;
	char buf[2048];
	int tmp;
	HANDLE lib;
	void pascal (*regxx)(HANDLE);
	void pascal (*regxy)(HANDLE);
	HANDLE mutex;

        /* create_pnglib();
	exit(1); */

	parse_cmd(lpCmdLine);

	mutex=CreateMutex(NULL,0,"MOAB");
	if (mutex==NULL || GetLastError()==ERROR_ALREADY_EXISTS && strcmp(host_addr,"192.168.42.1")) {
		MessageBox(0,"Another instance of "MNAME" is already running.","Error",MB_OK|MB_ICONSTOP);
		return 0;
	}

	lib=LoadLibrary("CTL3D32.DLL");
	if (lib) {
		regxx=(void pascal *)GetProcAddress(lib,"Ctl3dRegister");
		if (regxx) regxx(GetCurrentProcess());
		ctl3don=(void pascal *)GetProcAddress(lib,"Ctl3dSubclassDlg");
		regxy=(void pascal *)GetProcAddress(lib,"Ctl3dUnregister");
	} else {
		regxy=NULL;
		ctl3don=NULL;
	}

	dlg_col=GetSysColor(COLOR_BTNFACE);
	dlg_back=CreateSolidBrush(dlg_col);
	dlg_fcol=GetSysColor(COLOR_WINDOWTEXT);

	hwnd=InitWindow(hInstance,nCmdShow);

	load_options();
	init_engine();
	options();
	if (quit) exit(0);

	init_sound(hwnd);

	if ((tmp=dd_init(hwnd,MODEX,MODEY))!=0) {

		sprintf(buf,"|DDERROR=%d",-tmp);
		say(buf);
		Sleep(1000);

		sprintf(buf,
				"DirectX init failed with code %d.\n"
				"DDError=%s\n"
				"Client Version %d.%02d.%02d\n"
				"MAXX=%d, MAXY=%d\n"
				"R=%04X, G=%04X, B=%04X\n"
				"RGBM=%d\n"
				"MAXCACHE=%d\n",
				-tmp,DDERR,VERSION>>16,(VERSION>>8)&255,VERSION&255,MAXX,MAXY,RED,GREEN,BLUE,RGBM,MAXCACHE);
		MessageBox(hwnd,buf,"DirectX init failed.",MB_ICONSTOP|MB_OK);
		exit(1);
	}
        sprintf(buf,"|R=%04X, G=%04X, B=%04X, RGBM=%d",RED,GREEN,BLUE,RGBM);
	say(buf);

        init_xalloc();
	conv_init();
	init_pnglib();
	dd_init_sprites();	

	if (RGBM==-1) {
		sprintf(buf,"|unknown card: R=%04X G=%04X B=%04X",RED,GREEN,BLUE);
		say(buf);
	}

	log_system_data();

	engine();

	dd_deinit();

	if (regxy) regxy(GetCurrentProcess());

	save_options();

	return 0;
}
