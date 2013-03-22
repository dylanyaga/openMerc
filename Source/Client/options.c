
#include <stdio.h>
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include "ddraw.h"
#include <process.h>
#include <time.h>
#include <dir.h>
#pragma hdrstop
#include "dd.h"
#include "common.h"
#include "inter.h"
#include "merc.rh"

/*
 * Borland and Microsoft disagree on the size of the OPENFILENAME structure.
 * The following header is excerpted from "win.h" as included with
 * Jacob Navia's * lcc32 compiler.
 */

typedef struct tagOFN {
	DWORD lStructSize;
	HWND hwndOwner;
	HINSTANCE hInstance;
	LPCTSTR lpstrFilter;
	LPTSTR lpstrCustomFilter;
	DWORD nMaxCustFilter;
	DWORD nFilterIndex;
	LPTSTR lpstrFile;
	DWORD nMaxFile;
	LPTSTR lpstrFileTitle;
	DWORD nMaxFileTitle;
	LPCTSTR lpstrInitialDir;
	LPCTSTR lpstrTitle;
	DWORD Flags;
	WORD nFileOffset;
	WORD nFileExtension;
	LPCTSTR lpstrDefExt;
	DWORD lCustData;
	LPOFNHOOKPROC lpfnHook;
	LPCTSTR lpTemplateName;
} LCC_OPENFILENAME,*LCC_LPOPENFILENAME;

extern char history[20][128];
extern int hist_len[20];
extern char words[2048][40];
extern char passwd[15];

extern void pascal (*ctl3don)(HANDLE,short int);
extern HBRUSH dlg_back;
extern int dlg_col,dlg_fcol;

extern int quit;

extern int do_alpha;
extern int do_shadow;

extern HINSTANCE hinst;
extern HWND desk_hwnd;
extern int so_status;

struct key okey;
struct pdata pdata={"","","",0};

//--------------
// option flags
//--------------

extern int domusic,dosound,smode;
extern char host_addr[];
static int opmusic,opsound,opshadow;
int race=0,sex=0;

static char *new_msg1={
	"Do you really want to create a new account?\n\n"
	"Your old account will no longer be accessible, unless you remembered to save it. You did save it, didn't you?\n"};

static char *new_msg2={
	"Have you read and understood the previous message?"};

static char *load_msg={
	"Do you really want to load an account?\n\n"
	"Your old account will no longer be accessible if you didn't save it yet. You did save it, didn't you?\n"
};

int dd_change(int x,int y);

void translate_okey2race(int *race_ptr,int *sex_ptr)
{
	switch(okey.race) {
		case 2: race=2; sex=1; break;	// mercenary M
		case 3: race=1; sex=1; break;	// templar M
		case 4: race=3; sex=1; break;	// harakim M
	
		case 13: race=4; sex=1; break;	// seyan M
	
		case 76: race=2; sex=2; break;	// mercenary F
		case 77: race=2; sex=2; break;	// templar F
		case 78: race=2; sex=2; break;	// harakim F
		
		case 79: race=2; sex=2; break;	// seyan F
	
		case 543: race=0; sex=1; break;	// god M
	
		case 544: race=5; sex=1; break;	// arch templar M
		case 545: race=6; sex=1; break;	// arch harakim M
		
		case 546: race=7; sex=1; break;	// sorcerer M
		case 547: race=8; sex=1; break;	// warrior M
	
		case 548: race=0; sex=2; break;	// god F
		
		case 549: race=5; sex=2; break;	// arch templar F
		case 550: race=6; sex=2; break;	// arch harakim F
		
		case 551: race=7; sex=2; break;	// sorcerer F
		case 552: race=8; sex=2; break;	// warrior F
	
		default: race=0; sex=1; break;
	}

	*race_ptr=race;
	*sex_ptr=sex;
}

void load_options(void)
{
	int n,handle,flag=0;

	handle=open("merc.dat",O_RDONLY|O_BINARY);
	if (handle!=-1) {
		if (read(handle,history,sizeof(history))!=sizeof(history)) flag=1;
		if (read(handle,hist_len,sizeof(hist_len))!=sizeof(hist_len)) flag=1;
		if (read(handle,words,sizeof(words))!=sizeof(words)) flag=1;
		if (read(handle,&domusic,sizeof(domusic))!=sizeof(domusic)) flag=1;
		if (read(handle,&dosound,sizeof(dosound))!=sizeof(dosound)) flag=1;
		if (read(handle,&pdata,sizeof(pdata))!=sizeof(pdata)) flag=1;
		if (read(handle,&okey,sizeof(okey))!=sizeof(okey)) flag=1;
		if (read(handle,&do_alpha,sizeof(do_alpha))!=sizeof(do_alpha)) do_alpha=2;
		if (read(handle,&do_shadow,sizeof(do_shadow))!=sizeof(do_shadow)) do_shadow=1;
		close(handle);
	} else flag=1;

	if (flag) {
		/* flag=1; */
		memset(history,0,sizeof(history));
		memset(hist_len,0,sizeof(hist_len));
		memset(words,0,sizeof(words));
		domusic=0; dosound=1; do_alpha=2; do_shadow=1;
		memset(&pdata,0,sizeof(pdata));
		pdata.show_names=1;
		for (n=0; n<12; n++) {
			pdata.xbutton[n].skill_nr=-1;
			strcpy(pdata.xbutton[n].name,"-");
		}
		memset(&okey,0,sizeof(okey));
		strcpy(okey.name,"New Account");
	}
}

void save_options(void)
{
	int handle;

	handle=open("merc.dat",O_WRONLY|O_BINARY|O_CREAT|O_TRUNC,0666);
	if (handle!=-1) {
		write(handle,history,sizeof(history));
		write(handle,hist_len,sizeof(hist_len));
		write(handle,words,sizeof(words));
		write(handle,&domusic,sizeof(domusic));
		write(handle,&dosound,sizeof(dosound));
		write(handle,&pdata,sizeof(pdata));
		write(handle,&okey,sizeof(okey));
		write(handle,&do_alpha,sizeof(do_alpha));
		write(handle,&do_shadow,sizeof(do_shadow));
		close(handle);
	}
}

void load_char(HWND hwnd,char *name)
{
	int handle,n,flag=0;
	char buf[256];

	if (MessageBox(hwnd,load_msg,"Are you sure?",MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL)!=IDYES)
		return;

	handle=open(name,O_RDONLY|O_BINARY);
	if (handle==-1) {
		sprintf(buf,"Could not open file \"%s\".",name);
		MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
		return;
	}

	if (lseek(handle,0,SEEK_END)>(long) (sizeof(struct pdata)+sizeof(struct key))) flag=1;
	lseek(handle,0,SEEK_SET);

	read(handle,&okey,sizeof(struct key));
	if (read(handle,&pdata,sizeof(struct pdata))!=sizeof(struct pdata) || flag) {
		pdata.hide=0;
		pdata.show_names=1;
		pdata.show_proz=1;
		pdata.cname[0]=0;
		pdata.ref[0]=0;
		pdata.desc[0]=0;
		pdata.changed=0;

		for (n=0; n<12; n++) {
			pdata.xbutton[n].skill_nr=-1;
			strcpy(pdata.xbutton[n].name,"-");
		}
	}

	close(handle);

	pdata.changed=1;
}

void save_char(HWND hwnd,char *name)
{
	int handle;
	char buf[256];

	handle=open(name,O_WRONLY|O_BINARY|O_CREAT|O_TRUNC,0600);
	if (handle==-1) {
		sprintf(buf,"Could not open file \"%s\".",name);
		MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
		return;
	}

	write(handle,&okey,sizeof(struct key));
	write(handle,&pdata,sizeof(struct pdata));

	close(handle);

	sprintf(buf,"Saved as \"%s\".",name);
	MessageBox(hwnd,buf,"Done",MB_OK);
}

int isNT(void)
{
	OSVERSIONINFO o;
	o.dwOSVersionInfoSize=sizeof(o);

	GetVersionEx(&o);

	if (o.dwPlatformId==VER_PLATFORM_WIN32_NT) return 1;
	else return 0;
}

struct mtp {
	DWORD        lStructSize;
	HWND         hwndOwner;
	HINSTANCE    hInstance;
	LPCSTR       lpstrFilter;
	LPSTR        lpstrCustomFilter;
	DWORD        nMaxCustFilter;
	DWORD        nFilterIndex;
	LPSTR        lpstrFile;
	DWORD        nMaxFile;
	LPSTR        lpstrFileTitle;
	DWORD        nMaxFileTitle;
	LPCSTR       lpstrInitialDir;
	LPCSTR       lpstrTitle;
	DWORD        Flags;
	WORD         nFileOffset;
	WORD         nFileExtension;
	LPCSTR       lpstrDefExt;
	LPARAM       lCustData;
	LPOFNHOOKPROC lpfnHook;
	LPCSTR       lpTemplateName;
};

int load_dialog(HWND hwnd,char *name)
{
	LCC_OPENFILENAME ofn;
        char filter[]={MNAME" Character Save\0*.moa\0\0"};
	char buf[256]={"\0"};
	int disk,err;
	char dir[256],dir2[300];

	disk=getdisk();
	getcurdir(0,dir);

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hwnd;
	ofn.hInstance=hinst;
	ofn.lpstrFilter=filter;
	ofn.lpstrCustomFilter=NULL;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=buf;
	ofn.nMaxFile=256;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle="Load Character";
	ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	ofn.nFileOffset=0;
	ofn.nFileExtension=0;
	ofn.lpstrDefExt="moa";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;

	if (!GetOpenFileName((OPENFILENAME *) &ofn)) {
		setdisk(disk);
		chdir(dir);
		return 0;
	}

	strcpy(name,buf);

	setdisk(disk);
	sprintf(dir2,"\\%s",dir);
	chdir(dir2);

	return 1;
}

int save_dialog(HWND hwnd,char *name)
{
	LCC_OPENFILENAME ofn;
        char filter[]={MNAME" Character Save\0*.moa\0\0"};
	char buf[256];
	int disk;
	char dir[256],dir2[300];

	disk=getdisk();
	getcurdir(0,dir);

	strcpy(buf,okey.name);
	strcat(buf,".moa");

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hwnd;
	ofn.hInstance=hinst;
	ofn.lpstrFilter=filter;
	ofn.lpstrCustomFilter=NULL;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=buf;
	ofn.nMaxFile=256;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle="Save Character as";
	ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
	ofn.nFileOffset=0;
	ofn.nFileExtension=0;
	ofn.lpstrDefExt="moa";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;

	if (!GetSaveFileName((OPENFILENAME *) &ofn)) {
		setdisk(disk);
		chdir(dir);
		return 0;
	}

	strcpy(name,buf);

	setdisk(disk);
	sprintf(dir2,"\\%s",dir);
	chdir(dir2);

	return 1;
}

void update_race(HWND hwnd)
{
	if (race==1) CheckRadioButton(hwnd,IDC_TEMP,IDC_SEY,IDC_TEMP);
	if (race==2) CheckRadioButton(hwnd,IDC_TEMP,IDC_SEY,IDC_MERC);
	if (race==3) CheckRadioButton(hwnd,IDC_TEMP,IDC_SEY,IDC_HARA);
	if (race==4) CheckRadioButton(hwnd,IDC_TEMP,IDC_SEY,IDC_SEY);
	if (race==5) CheckRadioButton(hwnd,IDC_TEMP,IDC_SEY,IDC_ARCHTEMPLAR);
	if (race==6) CheckRadioButton(hwnd,IDC_TEMP,IDC_SEY,IDC_ARCHHARAKIM);
	if (race==7) CheckRadioButton(hwnd,IDC_TEMP,IDC_SEY,IDC_WARRIOR);
	if (race==8) CheckRadioButton(hwnd,IDC_TEMP,IDC_SEY,IDC_SORCERER);

	if (sex==1) CheckRadioButton(hwnd,IDC_MALE,IDC_FEMALE,IDC_MALE);
	if (sex==2) CheckRadioButton(hwnd,IDC_MALE,IDC_FEMALE,IDC_FEMALE);
}

void update_alpha(HWND hwnd)
{
	if (do_alpha==2) CheckRadioButton(hwnd,IDC_FULLALPHA,IDC_NOALPHA,IDC_FULLALPHA);
	if (do_alpha==1) CheckRadioButton(hwnd,IDC_FULLALPHA,IDC_NOALPHA,IDC_PARTIALALPHA);
	if (do_alpha==0) CheckRadioButton(hwnd,IDC_FULLALPHA,IDC_NOALPHA,IDC_NOALPHA);
}

#pragma argsused
APIENTRY OptionsProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	int n;
	char buf[256];
	static int done=0;
	CREATESTRUCT *cs;

	switch (message) {
		case WM_CTLCOLOR:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORSCROLLBAR:
			SetTextColor((HDC) wParam,dlg_fcol);
			SetBkColor((HDC) wParam,dlg_col);
			return(int) dlg_back;

		case WM_CLOSE:			
			//if (IsDlgButtonChecked(hwnd,IDC_DOMUSIC)) domusic=1;
			//else domusic=0;
			if (IsDlgButtonChecked(hwnd,IDC_DOSOUND)) dosound=1;
			else dosound=0;

			if (IsDlgButtonChecked(hwnd,IDC_DOSHADOW)) do_shadow=1;
			else do_shadow=0;

			GetDlgItemText(hwnd,IDC_CNAME,pdata.cname,79);
			GetDlgItemText(hwnd,IDC_DESC,pdata.desc,149);

			save_options();
			EndDialog(hwnd,0);
			quit=1;
			return 1;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case    IDOK:
#ifdef DOCONVERT
					convert(hwnd);
					create_pnglib(hwnd);
					exit(1);
#endif

					if (so_status) {
						MessageBeep(MB_ICONEXCLAMATION);
						break;
					}
					//if (IsDlgButtonChecked(hwnd,IDC_DOMUSIC)) domusic=1;
					//else domusic=0;
					if (IsDlgButtonChecked(hwnd,IDC_DOSOUND)) dosound=1;
					else dosound=0;
					if (IsDlgButtonChecked(hwnd,IDC_DOSHADOW)) do_shadow=1;
					else do_shadow=0;

					GetDlgItemText(hwnd,IDC_CNAME,pdata.cname,79);
					GetDlgItemText(hwnd,IDC_DESC,pdata.desc,158);
					GetDlgItemText(hwnd,IDC_PASS,passwd,15);

					randomize();

					if (sex==0) sex=random(2)+1;
					if (race==0) race=random(4)+1;

					if (sex==1) {
						if (race==1) race=3;
						else if (race==2) race=2;
						else if (race==3) race=4;
						else race=13;
					} else {
						if (race==1) race=77;
						else if (race==2) race=76;
						else if (race==3) race=78;
						else race=79;
					}

					save_options();
					{
						void so_connect(void*);
						_beginthread(so_connect,16384,(void*)hwnd);
					}

					return 1;

				case    IDCANCEL:
					//if (IsDlgButtonChecked(hwnd,IDC_DOMUSIC)) domusic=1;
					//else domusic=0;
					if (IsDlgButtonChecked(hwnd,IDC_DOSOUND)) dosound=1;
					else dosound=0;
					if (IsDlgButtonChecked(hwnd,IDC_DOSHADOW)) do_shadow=1;
					else do_shadow=0;

					GetDlgItemText(hwnd,IDC_CNAME,pdata.cname,79);
					GetDlgItemText(hwnd,IDC_DESC,pdata.desc,149);

					save_options();
					EndDialog(hwnd,0);
					quit=1;
					return 1;
				case    IDC_HELP:
					ShowWindow(hwnd,SW_MINIMIZE);
					ShellExecute(hwnd,"open",MHELP,NULL,NULL,SW_SHOWNORMAL);
					break;
				case    IDC_NEWS:
					ShowWindow(hwnd,SW_MINIMIZE);
					ShellExecute(hwnd,"open",MNEWS,NULL,NULL,SW_SHOWNORMAL);
					break;
				case    IDC_NEW:
					if (MessageBox(hwnd,new_msg1,"Are you sure?",MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL)!=IDYES)
						return 1;
					if (MessageBox(hwnd,new_msg2,"Are you really sure?",MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL)!=IDYES)
						return 1;
					okey.usnr=0;
					okey.pass1=0;
					okey.pass2=0;
					pdata.changed=1;
					strcpy(okey.name,"New Account");
					SetDlgItemText(hwnd,IDC_RNAME,okey.name);
					return 1;
				case IDC_SAVE:
					if (save_dialog(hwnd,buf)) save_char(hwnd,buf);
					return 1;
				case IDC_LOAD:
					if (load_dialog(hwnd,buf)) load_char(hwnd,buf);
					SetDlgItemText(hwnd,IDC_RNAME,okey.name);

					SetDlgItemText(hwnd,IDC_CNAME,pdata.cname);
					SetDlgItemText(hwnd,IDC_DESC,pdata.desc);

					translate_okey2race(&race,&sex);

					update_race(hwnd);
					update_alpha(hwnd);
					return 1;
				/*case IDC_DOMUSIC:
					opmusic=1-opmusic; CheckDlgButton(hwnd,IDC_DOMUSIC,opmusic);
					return 1;*/
				case IDC_DOSOUND:
                                        opsound=1-opsound; CheckDlgButton(hwnd,IDC_DOSOUND,opsound);
					return 1;
				case IDC_DOSHADOW:
					opshadow=1-opshadow; CheckDlgButton(hwnd,IDC_DOSHADOW,opshadow);
					return 1;					

				case IDC_CNAME:
					if (done && HIWORD(wParam)==EN_CHANGE) {
						n=SendDlgItemMessage(hwnd,LOWORD(wParam),EM_LINELENGTH,0,0);
						if (n>38) {
							GetDlgItemText(hwnd,LOWORD(wParam),buf,79);
							buf[38]=0;
							SetDlgItemText(hwnd,LOWORD(wParam),buf);
							MessageBeep(-1);
						}
						pdata.changed=1;
					}
					return 1;

				case IDC_DESC:
					if (done && HIWORD(wParam)==EN_CHANGE) {
						n=SendDlgItemMessage(hwnd,LOWORD(wParam),EM_LINELENGTH,0,0);
						if (n>150) {
							GetDlgItemText(hwnd,LOWORD(wParam),buf,149);
							buf[150]=0;
							SetDlgItemText(hwnd,LOWORD(wParam),buf);
							MessageBeep(-1);
						}
						pdata.changed=1;
					}
					return 1;

				case IDC_MALE:
					if (okey.usnr) MessageBox(hwnd,"Changing the gender has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else sex=1;
					update_race(hwnd);
					update_alpha(hwnd);
					return 1;

				case IDC_FEMALE:
					if (okey.usnr) MessageBox(hwnd,"Changing the gender has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else sex=2;
					update_race(hwnd);
					update_alpha(hwnd);
					return 1;

				case IDC_TEMP:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=1;
					update_race(hwnd);
					update_alpha(hwnd);
					return 1;

				case IDC_MERC:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=2;
					update_race(hwnd);
					update_alpha(hwnd);
					return 1;

				case IDC_HARA:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=3;
					update_race(hwnd);
					update_alpha(hwnd);
					return 1;

				case IDC_SEY:
				case IDC_ARCHTEMPLAR:
				case IDC_ARCHHARAKIM:
				case IDC_WARRIOR:
				case IDC_SORCERER:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else {
						MessageBox(hwnd,"This race has to be earned, it cannot be chosen.","Sorry",MB_OK|MB_ICONSTOP);
						race=2;
					}
					update_race(hwnd);
					update_alpha(hwnd);
					return 1;

				case IDC_FULLALPHA:
					do_alpha=2;
					update_alpha(hwnd);
					update_race(hwnd);
					return 1;
					
				case IDC_PARTIALALPHA:
					do_alpha=1;
					update_alpha(hwnd);
					update_race(hwnd);
					return 1;

				case IDC_NOALPHA:
					do_alpha=0;
					update_alpha(hwnd);
					update_race(hwnd);
					return 1;


				default:
					return 1;
			}
		case WM_INITDIALOG:
			if (ctl3don) ctl3don(hwnd,0xfffe);

			//CheckDlgButton(hwnd,IDC_DOMUSIC,opmusic);
			CheckDlgButton(hwnd,IDC_DOSOUND,opsound);
			CheckDlgButton(hwnd,IDC_DOSHADOW,opshadow);

			SetFocus(GetDlgItem(hwnd,IDOK));

			SetDlgItemText(hwnd,IDC_RNAME,okey.name);

			SetDlgItemText(hwnd,IDC_CNAME,pdata.cname);
			SetDlgItemText(hwnd,IDC_DESC,pdata.desc);			

			translate_okey2race(&race,&sex);

                        update_race(hwnd);
			update_alpha(hwnd);

			sprintf(buf,MNAME" v%d.%02d.%02d  --  Game Options",VERSION>>16,(VERSION>>8)&255,VERSION&255);
			SetWindowText(hwnd,buf);
			done=1;
			return 1;

		default:
			return 0;
	}
}

void options(void)
{
	opmusic=domusic; opsound=dosound; opshadow=do_shadow;

	if (DialogBox(hinst,MAKEINTRESOURCE(OPTIONS),desk_hwnd,OptionsProc)==-1) {
		MessageBeep(MB_ICONEXCLAMATION);
	}
	save_options();
}
