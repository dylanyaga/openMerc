#include <stdio.h>
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <errno.h>
#pragma hdrstop
#include "dsound.h"

#include "common.h"
#include "inter.h"

int domusic=0;
int dosound=1;

#define BUFFSIZE 	(1024*32)
#define BUFFCHUNK	(1024*8)

extern HWND desk_hwnd;

void sound_error(int nr,HWND hwnd,char *title);
void sounder(void *dummy);

LPDIRECTSOUND ds;
LPDIRECTSOUNDBUFFER pb=NULL;

LPDIRECTSOUNDBUFFER sb[10];
static unsigned char used[10]={0,0,0,0,0,0,0,0,0,0};
static int cpos[10];
static int shandle[10];
static int start[10]={0,0,0,0,0,0,0,0,0,0};
static int volume[10];
static int pan[10];
char sname[10][256];

int init_sound(HWND hwnd)
{
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC buffdesc;
	HRESULT hr;
	int n;

	if (!dosound && !domusic) return -1;

	hr=DirectSoundCreate(NULL,&ds,NULL);
	if (hr!=DS_OK) { dosound=domusic=0; return -1; }

	hr=ds->lpVtbl->SetCooperativeLevel(ds,hwnd,DSSCL_EXCLUSIVE);
	if (hr!=DS_OK) { dosound=domusic=0; return -1; }

	memset(&pcmwf,0,sizeof(PCMWAVEFORMAT));
	pcmwf.wf.wFormatTag=WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels=2;
	pcmwf.wf.nSamplesPerSec=22050;
	pcmwf.wf.nBlockAlign=2;       //4
	pcmwf.wf.nAvgBytesPerSec=pcmwf.wf.nSamplesPerSec*pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample=8;       //16

	buffdesc.dwSize=sizeof(DSBUFFERDESC);
	buffdesc.dwFlags=DSBCAPS_PRIMARYBUFFER|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLPAN;
	buffdesc.dwReserved=0;
	buffdesc.dwBufferBytes=0;
	buffdesc.lpwfxFormat=NULL;

	hr=ds->lpVtbl->CreateSoundBuffer(ds,&buffdesc,&pb,NULL);
	if (hr!=DS_OK) { sound_error(hr,hwnd,"CreateSoundBuffer"); dosound=domusic=0; return -1; }

	hr=pb->lpVtbl->SetFormat(pb,(LPCWAVEFORMATEX)&pcmwf);
	if (hr!=DS_OK) { sound_error(hr,hwnd,"SetFormat"); dosound=domusic=0; return -1; }

	buffdesc.dwSize=sizeof(DSBUFFERDESC);
	buffdesc.dwFlags=DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|DSBCAPS_GLOBALFOCUS;
	buffdesc.dwReserved=0;
	buffdesc.dwBufferBytes=BUFFSIZE;
	buffdesc.lpwfxFormat=(LPWAVEFORMATEX)&pcmwf;

	for (n=0; n<10; n++) {
		hr=ds->lpVtbl->CreateSoundBuffer(ds,&buffdesc,&sb[n],NULL);
		if (hr!=DS_OK) { sound_error(hr,hwnd,"CreateSoundBuffer 2"); dosound=domusic=0; return -1; }
	}

	if (dosound || domusic) _beginthread(sounder,4096,NULL);
	return 0;
}

#pragma argsused
void sound_error(int nr,HWND hwnd,char *title)
{
	/*char *err;

	switch(nr) {
		case DSERR_INVALIDCALL:			err="DSERR_INVALIDCALL"; break;
		case DSERR_PRIOLEVELNEEDED: 		err="DSERR_PRIOLEVELNEEDED"; break;
		case DSERR_UNSUPPORTED:			err="DSERR_UNSUPPORTED"; break;
		case DSERR_UNINITIALIZED:		err="DSERR_UNINITIALIZED"; break;
		case DSERR_OUTOFMEMORY:			err="DSERR_OUTOFMEMORY"; break;
		case DSERR_NOAGGREGATION:		err="DSERR_NOAGGREGATION"; break;
		case DSERR_INVALIDPARAM: 		err="DSERR_INVALIDPARAM"; break;
		case DSERR_BADFORMAT:			err="DSERR_BADFORMAT"; break;
		case DSERR_ALLOCATED:			err="DSERR_ALLOCATED"; break;
		default: 				err="unknown"; break;
	}
	MessageBox(hwnd,err,title,MB_ICONSTOP|MB_OK); disabled for now. check what's wrong!!*/
}

int play_sound(char *file,int vol,int p)
{
	int n;

	if (!dosound) return 0;

	for (n=1; n<10; n++) if (used[n]==0 && start[n]==0) break;
	if (n==10) { /* xlog(0,"Not enough Sound Buffers"); */ return -1; }
	//xlog("Request to play %s scheduled to slot %d",file,n);

	strcpy(sname[n],file);
	volume[n]=vol;
	pan[n]=p;
	start[n]=1;

	return n;
}

#pragma argsused
void sounder(void *dummy)
{
	extern int quit;
	int n;
	HRESULT hr;
	DWORD pos,status;
	void *ptr1,*ptr2;
	unsigned long size1,size2,len,rsize;
	static int song=0;

	if (!dosound && !domusic) return;

	while (!quit) {
		Sleep(5);

		for (n=0; n<10; n++) {
			hr=sb[n]->lpVtbl->GetStatus(sb[n],&status);
			if (hr!=DS_OK) { sound_error(hr,desk_hwnd,"GetStatus"); continue; }
			if ((status&DSBSTATUS_PLAYING)==0) {
				if (n==0 && domusic) { // channel 0 is reserved for background music...
					start[n]=1;
					switch(song) {
						case	0:	strcpy(sname[n],"sfx\\s2.wav"); break;
						case	1:	strcpy(sname[n],"sfx\\intro.wav"); break;
						case	2:	strcpy(sname[n],"sfx\\mission.wav"); song=0; break; // song=-1;
					}
					song++;
					volume[n]=-300;
                                        pan[n]=0;
				}
				used[n]=0;
			}
			if (status&DSBSTATUS_PLAYING && n==0 && domusic==0) {
				sb[n]->lpVtbl->Stop(sb[n]);
				used[n]=0;
			}

			if (start[n]) {
					shandle[n]=open(sname[n],O_RDONLY|O_BINARY|O_DENYNONE);
					lseek(shandle[n],0x2e,SEEK_SET); // skip wave header
					if (shandle[n]==-1) {
						xlog(0,"%s: %s",sname[n],strerror(errno),sname[n]);
						start[n]=0;
						continue;
					}
					used[n]=1;
					cpos[n]=0;
			}
			if (!used[n]) continue;

			if (!start[n]) {
				hr=sb[n]->lpVtbl->GetCurrentPosition(sb[n],&pos,NULL);
				if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"GetCurrentPosition"); continue; }
				if (pos<cpos[n]+BUFFCHUNK && pos+(BUFFSIZE/2)>cpos[n]) continue;
				rsize=BUFFCHUNK;
			} else { pos=0; rsize=BUFFSIZE; start[n]=0; }

			//xlog("%X: pos=%dK, cpos[n]=%dK",n,pos>>10,cpos[n]>>10);

			hr=sb[n]->lpVtbl->Lock(sb[n],cpos[n],rsize,&ptr1,&size1,&ptr2,&size2,0);
			if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"Lock"); used[n]=0; close(shandle[n]); continue; }

			if (shandle[n]!=-1)
				len=read(shandle[n],ptr1,size1);
			else len=0;

			//xlog("size=%dK len=%dK",size1>>10,len>>10);
			if (len<size1) {
				memset((unsigned char*)ptr1+len,127,size1-len);
				if (pos<cpos[n]) {
					hr=sb[n]->lpVtbl->Play(sb[n],0,0,0);
					if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"Play"); close(shandle[n]); }
					hr=sb[n]->lpVtbl->SetVolume(sb[n],volume[n]);
					if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"SetVolume"); close(shandle[n]); }
					hr=sb[n]->lpVtbl->SetPan(sb[n],pan[n]);
					if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"SetPan"); close(shandle[n]); }
				} else {
					hr=sb[n]->lpVtbl->GetStatus(sb[n],&status);
					if (hr!=DS_OK) { sound_error(hr,desk_hwnd,"GetStatus"); continue; }
					if ((status&DSBSTATUS_PLAYING)==0) {
						hr=sb[n]->lpVtbl->Play(sb[n],0,0,0);
						if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"Play"); close(shandle[n]); }
						hr=sb[n]->lpVtbl->SetVolume(sb[n],volume[n]);
						if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"SetVolume"); close(shandle[n]); }
						hr=sb[n]->lpVtbl->SetPan(sb[n],pan[n]);
						if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"SetPan"); close(shandle[n]); }
					}
				}
				close(shandle[n]);
				shandle[n]=-1;
			} else {
				hr=sb[n]->lpVtbl->Play(sb[n],0,0,DSBPLAY_LOOPING);
				if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"Play"); continue; }
				hr=sb[n]->lpVtbl->SetVolume(sb[n],volume[n]);
				if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"SetVolume"); close(shandle[n]); }
				hr=sb[n]->lpVtbl->SetPan(sb[n],pan[n]);
				if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"SetPan"); close(shandle[n]); }
			}

			hr=sb[n]->lpVtbl->Unlock(sb[n],ptr1,size1,ptr2,0);
			if (hr!=DD_OK) { sound_error(hr,desk_hwnd,"Unlock"); used[n]=0; close(shandle[n]); continue; }

			cpos[n]+=size1;

			if (cpos[n]==BUFFSIZE) cpos[n]=0;
		}
	}
}
