#include <stdio.h>
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <time.h>
#include <math.h>
#pragma hdrstop
#define INITGUID
#include <ddraw.h>
#include <dsound.h>
#include "dd.h"
#include "common.h"
#include "inter.h"
#include "lpng/png.h"

int tricky_flag=0;

int maxmem=16*1024*1024,maxvid=2*1024*1024;
int usedmem=0,usedvid=0;
char *DDERR;
extern int do_alpha;
extern int do_shadow;

HANDLE heap=NULL;

int blockcnt=0,blocktot=0,blockgc=0;
int alphapix=0,fullpix=0;

unsigned short *dd_load_png(FILE *fp,int *xs,int *ys,unsigned char **alpha_ptr,int *alphacnt_ptr);
void display_alpha(unsigned char *alpha,int alphacnt,int xf,int yf,int effect);
FILE *load_pnglib(int nr);

void init_xalloc(void)
{
	heap=HeapCreate(HEAP_GENERATE_EXCEPTIONS,maxmem+4096*1024,0);
}

void *xmalloc(int size)
{
	char *ptr;

	blockcnt++; blocktot++;

	ptr=HeapAlloc(heap,0,size);
	usedmem+=HeapSize(heap,0,ptr)+16;

	return ptr;
}

void xfree(void *ptr)
{
	blockcnt--;
	usedmem-=HeapSize(heap,0,ptr)+16;

	HeapFree(heap,0,ptr);
}

void *xcalloc(int size1,int size2)
{
	int size;
	unsigned char *ptr;

	size=size1*size2;

	blockcnt++; blocktot++;

	ptr=HeapAlloc(heap,HEAP_ZERO_MEMORY,size);
	usedmem+=HeapSize(heap,0,ptr)+16;

	return ptr;
}

extern HWND desk_hwnd;

#define TILE 32
unsigned short background=0;
int invisible=0;

void one(void);
void two(void);
void three(void);
void four(void);
void five(void);

int MAXX,MAXY;
int MAXX1,MAXY1;
int RED,GREEN,BLUE;
int RGBM=-1;

LPDIRECTDRAW dd=NULL;

LPDIRECTDRAWSURFACE sur1=NULL,sur2=NULL,suro=NULL;
LPDIRECTDRAWCLIPPER clip=NULL;

void *load_file(char *file);
void dd_invalidate_cache(void);
void *dd_get_ptr(LPDIRECTDRAWSURFACE sur);
int dd_release_ptr(LPDIRECTDRAWSURFACE sur);
void dd_flip(void);

int MAXCACHE;
int MAXXOVER;

int cachex,cachey;

DWORD total,left;

void dd_error(HWND hwnd,char *msg,long err);

char *get_dderr(long err)
{
	char *ptr;

	switch (err) {
		case DDERR_INVALIDOBJECT:                       ptr="INVALIDOBJECT"; break;
		case DDERR_INVALIDPARAMS:                       ptr="INVALIDPARAMS"; break;
		case DDERR_OUTOFMEMORY:                         ptr="OUTOFMEMORY"; break;
		case DDERR_SURFACEBUSY:                         ptr="SURFACEBUSY"; break;
		case DDERR_SURFACELOST:                         ptr="SURFACELOST"; break;
		case DDERR_WASSTILLDRAWING:                 ptr="WASSTILLDRAWING"; break;
		case DDERR_INCOMPATIBLEPRIMARY:             ptr="INCOMPATIBLEPRIMARY"; break;
		case DDERR_INVALIDCAPS:                   ptr="INVALIDCAPS"; break;
		case DDERR_INVALIDPIXELFORMAT:            ptr="INVALIDPIXELFORMAT"; break;
		case DDERR_NOALPHAHW:                     ptr="NOALPHAHW"; break;
		case DDERR_NOCOOPERATIVELEVELSET:         ptr="NOCOOPERATIVELEVELSET"; break;
		case DDERR_NODIRECTDRAWHW:                ptr="NODIRECTDRAWHW"; break;
		case DDERR_NOEMULATION:                   ptr="NOEMULATION"; break;
		case DDERR_NOEXCLUSIVEMODE:               ptr="NOEXCLUSIVEMODE"; break;
		case DDERR_NOFLIPHW:                      ptr="NOFLIPHW"; break;
		case DDERR_NOMIPMAPHW:                    ptr="NOMIPMAPHW"; break;
		case DDERR_NOOVERLAYHW:                   ptr="NOOVERLAYHW"; break;
		case DDERR_NOZBUFFERHW:                   ptr="NOZBUFFERHW"; break;
		case DDERR_OUTOFVIDEOMEMORY:              ptr="OUTOFVIDEOMEMORY"; break;
		case DDERR_PRIMARYSURFACEALREADYEXISTS:   ptr="PRIMARYSURFACEALREADYEXISTS"; break;
		case DDERR_UNSUPPORTEDMODE:               ptr="UNSUPPORTEDMODE"; break;
		case DDERR_EXCEPTION:                           ptr="EXCEPTION"; break;
		case DDERR_GENERIC:                             ptr="GENERIC"; break;
		case DDERR_INVALIDRECT:                         ptr="INVALIDRECT"; break;
		case DDERR_NOTFLIPPABLE:                        ptr="NOTFLIPPABLE"; break;
		case DDERR_UNSUPPORTED:                         ptr="UNSUPPORTED"; break;
		default:                                                ptr="Unknown Error"; break;
	}
	return ptr;
}

void dd_clear(void)
{
	unsigned short *ptr;
	int n;

	ptr=dd_get_ptr(sur1);
	if (!ptr) return;
	if (ptr) for (n=0; n<MAXX1*MAXY1; n++) ptr[n]=0;
	dd_release_ptr(sur1);

	ptr=dd_get_ptr(sur2);
	if (!ptr) return;
	if (ptr) for (n=0; n<MAXX1*MAXY1; n++) ptr[n]=0;
	dd_release_ptr(sur2);

	ptr=dd_get_ptr(suro);
	if (!ptr) return;
	if (ptr) for (n=0; n<MAXCACHE*1024; n++) ptr[n]=0;
	dd_release_ptr(suro);
}

struct vtab {
	int x,y;
};

struct vtab vtab[]= {
	{   1024,   2048},
	{   800,    2048},
	{   640,    2048},
	{   512,    2048},
	{   1024,   1024},
	{   800,    1024},
	{   640,    1024},
	{   512,    1024},
	{   1024,   600},
	{   800,    600},
	{   640,    600},
	{   512,    600},
	{   1024,   512},
	{   800,    512},
	{   640,    512},
	{   512,    512},
	{   1024,   480},
	{   800,    480},
	{   640,    480},
	{   512,    480},
	{   1024,   256},
	{   800,    256},
	{   640,    256},
	{   512,    256},
/*	{	1024,	128	},
	{	800,	128	},
	{	640,	128	},
	{	512,	128	},
	{	1024,	64	},
	{	800,	64	},
	{	640,	64	},
	{	512,	64	},
	{	256,	64	},
	{	128,	64	},
	{	64,	64	} */};

int usedvidmem=0;

#ifndef DB_WINDOWED
int dd_init(HWND hwnd,int x,int y)
{
	DDSURFACEDESC surface;
	long ret;
	int ysize,n;

	MAXX=x; MAXY=y;

	if ((ret=DirectDrawCreate(NULL,&dd,NULL))!=DD_OK) {
		DDERR=get_dderr(ret);
		return -1;
	}

	ret=dd->lpVtbl->SetCooperativeLevel(dd,hwnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
	if (ret!=DD_OK) {
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -2;
	}

	ret=dd->lpVtbl->SetDisplayMode(dd,x,y,16);
	if (ret!=DD_OK) {
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -3;
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_ALL;
	ret=dd->lpVtbl->GetDisplayMode(dd,&surface);
	if (ret!=DD_OK) {
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -4;
	}

	if (!(surface.ddpfPixelFormat.dwFlags&DDPF_RGB)) {
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -5;
	}

	RED=surface.ddpfPixelFormat.u2.dwRBitMask;
	GREEN=surface.ddpfPixelFormat.u3.dwGBitMask;
	BLUE=surface.ddpfPixelFormat.u4.dwBBitMask;

	if (RED==0xF800 && GREEN==0x07E0 && BLUE==0x001F) RGBM=0;
	else if (RED==0x7C00 && GREEN==0x03E0 && BLUE==0x001F) RGBM=1;
	else if (RED==0x001F && GREEN==0x07E0 && BLUE==0xF800) RGBM=2;

	if (tricky_flag) {
		surface.dwSize=sizeof(surface);
		surface.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
		surface.dwWidth=800;
		surface.dwHeight=600;

		dd->lpVtbl->CreateSurface(dd,&surface,&suro,NULL);
	}

	memset(&surface,0,sizeof(surface));
	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
	surface.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX;
	surface.dwBackBufferCount=1;

	ret=dd->lpVtbl->CreateSurface(dd,&surface,&sur1,NULL);
	if (ret!=DD_OK) {
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -6;
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS;
	surface.ddsCaps.dwCaps=DDSCAPS_BACKBUFFER;
	ret=sur1->lpVtbl->GetAttachedSurface(sur1,&surface.ddsCaps,&sur2);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -7;
	}

	if (tricky_flag) {
		suro->lpVtbl->Release(suro);
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_PITCH|DDSD_HEIGHT;
	ret=sur1->lpVtbl->GetSurfaceDesc(sur1,&surface);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -8;
	}
	MAXX1=surface.u1.lPitch/2;
	MAXY1=surface.dwHeight;
	MAXX=MAXX1;
	MAXY=MAXY1;

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS;
	ret=sur1->lpVtbl->GetSurfaceDesc(sur2,&surface);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -108;
	}
	usedvidmem=surface.ddsCaps.dwCaps&DDSCAPS_VIDEOMEMORY;

	// trying some combinations
	if (usedvidmem) {
		for (n=0; n<sizeof(vtab)/sizeof(struct vtab); n++) {
			surface.dwSize=sizeof(surface);
			surface.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
			surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
			surface.dwWidth=vtab[n].x;
			surface.dwHeight=vtab[n].y;

			ret=dd->lpVtbl->CreateSurface(dd,&surface,&suro,NULL);
			if (ret==DD_OK)	break;
		}
	} else ret=!DD_OK;

	if (ret!=DD_OK) {
		// trying some combinations
		for (n=0; n<sizeof(vtab)/sizeof(struct vtab); n++) {
			surface.dwSize=sizeof(surface);
			surface.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
			surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
			surface.dwWidth=vtab[n].x;
			surface.dwHeight=vtab[n].y;

			ret=dd->lpVtbl->CreateSurface(dd,&surface,&suro,NULL);
			if (ret==DD_OK)	break;
		}
		if (ret!=DD_OK) {
			DDERR=get_dderr(ret);
			return -9;
		}
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_PITCH|DDSD_CAPS|DDSD_WIDTH;
	ret=suro->lpVtbl->GetSurfaceDesc(suro,&surface);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -10;
	}
	MAXXOVER=surface.u1.lPitch/2;		// God, I hate DirectDraw!! It sucks!!
	ysize=surface.dwHeight;			// Absolutely everying seems to be variable.

	if (!(surface.ddsCaps.dwCaps&DDSCAPS_VIDEOMEMORY)) {
		xlog(0,"Could not allocate sprite cache (not enough video memory).");
		xlog(0,"Performance will suffer severely.");
	}

	cachex=MAXXOVER/TILE;
	cachey=ysize/TILE;

	MAXCACHE=min(65535,cachex*cachey);

	dd_clear();

	return 0;
}

void dd_deinit(void)
{
	if (sur1) {
		sur1->lpVtbl->Release(sur1); sur1=NULL;
	}
	if (dd) {
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		dd=NULL;
	}
}
#else
int dd_init(HWND hwnd,int x,int y)
{
	DDSURFACEDESC surface;
	long ret;
	int ysize,n;

	printf("DirectDrawCreate\n");
	if ((ret=DirectDrawCreate(NULL,&dd,NULL))!=DD_OK) {
		DDERR=get_dderr(ret); return -1;
	}

	printf("SetCooperativeLevel\n");
	ret=dd->lpVtbl->SetCooperativeLevel(dd,hwnd,DDSCL_NORMAL);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -2;
	}

	printf("CreateSurface\n");
	memset(&surface,0,sizeof(surface));
	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS;
	surface.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;

	ret=dd->lpVtbl->CreateSurface(dd,&surface,&sur1,NULL);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -3;
	}

	printf("CreateSurface 2\n");
	memset(&surface,0,sizeof(surface));
	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
	surface.dwWidth=x;
	surface.dwHeight=y;

	surface.ddpfPixelFormat.dwSize=sizeof(surface.ddpfPixelFormat);
	surface.ddpfPixelFormat.dwFlags=DDPF_RGB;
	surface.ddpfPixelFormat.u1.dwRGBBitCount=16;
	surface.ddpfPixelFormat.u2.dwRBitMask=0xf800;
	surface.ddpfPixelFormat.u3.dwGBitMask=0x07e0;
	surface.ddpfPixelFormat.u4.dwBBitMask=0x001f;
	surface.ddpfPixelFormat.u5.dwRGBAlphaBitMask=0;

	ret=dd->lpVtbl->CreateSurface(dd,&surface,&sur2,NULL);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -4;
	}

	printf("CreateClipper\n");
	ret=dd->lpVtbl->CreateClipper(dd,0,&clip,NULL);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -5;
	}

	printf("Attach Clipper to Window\n");
	ret=clip->lpVtbl->SetHWnd(clip,0,hwnd);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -6;
	}

	printf("Attach Clipper to Primary\n");
	ret=sur1->lpVtbl->SetClipper(sur1,clip);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -7;
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_PITCH|DDSD_HEIGHT;
	ret=sur2->lpVtbl->GetSurfaceDesc(sur2,&surface);
	if (ret!=DD_OK) {
		sur2->lpVtbl->Release(sur2);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -8;
	}
	MAXX1=surface.u1.lPitch/2;
	MAXY1=surface.dwHeight;
	MAXX=MAXX1;
	MAXY=MAXY1;

	RED=0xF800;
	GREEN=0x07E0;
	BLUE=0x001F;
	RGBM=0;

	for (n=0; n<sizeof(vtab)/sizeof(struct vtab); n++) {
		surface.dwSize=sizeof(surface);
		surface.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
		surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
		surface.dwWidth=vtab[n].x;
		surface.dwHeight=vtab[n].y;
		surface.ddpfPixelFormat.dwSize=sizeof(surface.ddpfPixelFormat);
		surface.ddpfPixelFormat.dwFlags=DDPF_RGB;
		surface.ddpfPixelFormat.u1.dwRGBBitCount=16;
		surface.ddpfPixelFormat.u2.dwRBitMask=0x7c00;
		surface.ddpfPixelFormat.u3.dwGBitMask=0x03e0;
		surface.ddpfPixelFormat.u4.dwBBitMask=0x001f;
		surface.ddpfPixelFormat.u5.dwRGBAlphaBitMask=0;

		ret=dd->lpVtbl->CreateSurface(dd,&surface,&suro,NULL);
		if (ret==DD_OK)	break;
	}
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		sur2->lpVtbl->Release(sur2);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -9;
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_PITCH|DDSD_CAPS|DDSD_WIDTH;
	ret=suro->lpVtbl->GetSurfaceDesc(suro,&surface);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		sur2->lpVtbl->Release(sur2);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -10;
	}
	MAXXOVER=surface.u1.lPitch/2;	// God, I hate DirectDraw!! It sucks!!
	ysize=surface.dwHeight;		// Absolutely everying seems to be variable.

	if (!(surface.ddsCaps.dwCaps&DDSCAPS_VIDEOMEMORY)) {
		xlog(0,"Could not allocate sprite cache (not enough video memory).");
		xlog(0,"Performance will suffer severely.");
	}

	cachex=MAXXOVER/TILE;
	cachey=ysize/TILE;

	MAXCACHE=min(65535,cachex*cachey);

	return 0;
}

void dd_deinit(void)
{
	if (sur1) {
		sur1->lpVtbl->Release(sur1); sur1=NULL;
	}

	if (dd) {
		dd->lpVtbl->Release(dd); dd=NULL;
	}
}
#endif
static int locks=0;

void *dd_get_ptr(LPDIRECTDRAWSURFACE sur)
{
	DDSURFACEDESC surface;
	long ret;

	surface.dwSize=sizeof(surface);
	ret=sur->lpVtbl->Lock(sur,NULL,&surface,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if (ret!=DD_OK) {
		dd_error(0,"dd_get_ptr",ret);
		return NULL;
	}

	locks++;

	return surface.lpSurface;
}

int dd_release_ptr(LPDIRECTDRAWSURFACE sur)
{
	long ret;

	locks--;
	ret=sur->lpVtbl->Unlock(sur,NULL);
	if (ret!=DD_OK) {
		dd_error(0,"dd_release_ptr",ret); return -1;
	}

	return 0;
}

void *_dd_get_ptr(LPDIRECTDRAWSURFACE sur)
{
	DDSURFACEDESC surface;
	long ret;

	surface.dwSize=sizeof(surface);

	ret=sur->lpVtbl->Lock(sur,NULL,&surface,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if (ret!=DD_OK) {
		dd_error(0,"dd_get_ptr",ret); return NULL;
	}

	return surface.lpSurface;
}

int _dd_release_ptr(LPDIRECTDRAWSURFACE sur)
{
	long ret;

	ret=sur->lpVtbl->Unlock(sur,NULL);

	if (ret!=DD_OK) {
		dd_error(0,"dd_release_ptr",ret); return -1;
	}

	return 0;
}

void islocked(void)
{
	unsigned short *ptr;
	int n;

	if (!locks)	return;

	ptr=_dd_get_ptr(sur1);
	if (!ptr) return;

	for (n=0; n<10; n++) {
		ptr[n]=0xF800;
		ptr[n+MODEX]=0xF800;
		ptr[n+MODEX*2]=0xF800;
	}
	_dd_release_ptr(sur1);
}

void dd_showbar(int xf,int yf,int xs,int ys,unsigned short col)
{
	unsigned short *ptr;
	int x,y,off;
	int xt,yt,r,g,b;

	ptr=dd_get_ptr(sur2);
	if (ptr==NULL) return;

	xt=xf+xs;
	yt=yf+ys;

	for (y=yf; y<yt; y++) {
		for (x=xf,off=y*MAXX+xf; x<xt; x++,off++) {
			switch (RGBM) {
				case 0:	  // 565
					r=((ptr[off]&0xF800)>>11)+((col&0xF800)>>11);
					g=((ptr[off]&0x07E0)>>5)+((col&0x07E0)>>5);
					b=((ptr[off]&0x001F))+((col&0x001F));

					r/=3;
					g/=3;
					b/=3;

					ptr[off]=(unsigned short)((r<<11)+(g<<5)+b);
					break;
				case 1:	 // 555
					r=((ptr[off]&0x7C00)>>10)+((col&0x7C00)>>10);
					g=((ptr[off]&0x03E0)>>5)+((col&0x03E0)>>5);
					b=((ptr[off]&0x001F))+((col&0x001F));

					r/=3;
					g/=3;
					b/=3;

					ptr[off]=(unsigned short)((r<<10)+(g<<5)+b);
					break;
				case 2:	 // 555
					b=((ptr[off]&0x7C00)>>10)+((col&0x7C00)>>10);
					g=((ptr[off]&0x03E0)>>5)+((col&0x03E0)>>5);
					r=((ptr[off]&0x001F))+((col&0x001F));

					r/=3;
					g/=3;
					b/=3;

					ptr[off]=(unsigned short)((b<<10)+(g<<5)+r);
					break;
			}
		}
	}

	dd_release_ptr(sur2);
}

void dd_showbox(int xf,int yf,int xs,int ys,unsigned short col)
{
	unsigned short *ptr;
	int x,y,off;
	int xt,yt;

	ptr=dd_get_ptr(sur2);
	if (ptr==NULL) return;

	xt=xf+xs;
	yt=yf+ys;

	for (y=yf; y<=yt; y++) {
		for (x=xf,off=y*MAXX+xf; x<=xt; x++,off++) {
			if (x!=xf && x!=xt && y!=yf && y!=yt) continue;
			ptr[off]=col;
		}
	}

	dd_release_ptr(sur2);
}

int dd_set_background(int color)
{
	DDCOLORKEY key;
	long ret;

	background=(unsigned short)color;
	key.dwColorSpaceLowValue=color;
	key.dwColorSpaceHighValue=color;

	ret=suro->lpVtbl->SetColorKey(suro,DDCKEY_SRCBLT,&key);

	if (ret!=DD_OK) {
		dd_error(0,"dd_set_background",ret); return -1;
	}

	return 0;
}

// BLTFast compatible (fairly)
int DBBltFast(LPDIRECTDRAWSURFACE sur1,int xt,int yt,LPDIRECTDRAWSURFACE sur2,RECT *rect)
{
	unsigned short *from,*to;
	DDSURFACEDESC surface;
	int ret,ys,xs,x;

	surface.dwSize=sizeof(surface);
	ret=sur1->lpVtbl->Lock(sur1,NULL,&surface,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if (ret!=DD_OK) {
		dd_error(0,"dd_get_ptr",ret); return ret;
	}
	to=surface.lpSurface;

	surface.dwSize=sizeof(surface);
	ret=sur2->lpVtbl->Lock(sur2,NULL,&surface,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if (ret!=DD_OK) {
        sur1->lpVtbl->Unlock(sur1,NULL);
		dd_error(0,"dd_get_ptr",ret); return ret;
	}
	from=surface.lpSurface;

	from+=rect->top*MAXXOVER+rect->left;
	to+=yt*MAXX+xt;
	xs=rect->right-rect->left;
	
	for (ys=rect->bottom-rect->top; ys; ys--) {
		for (x=0; x<xs; x++,from++,to++) {
			if (*from!=RED+BLUE) *to=*from;
		}
		from+=MAXXOVER-xs;
		to+=MAXX-xs;
	}

	sur1->lpVtbl->Unlock(sur1,NULL);
	sur2->lpVtbl->Unlock(sur2,NULL);

	return DD_OK;
}

// copy tile nr from offscreen to x,y at surface sur
int dd_copytile(int nr,int x,int y,LPDIRECTDRAWSURFACE sur,int mapcheck)
{
	RECT rect;
	long ret;
	int xs=0,ys=0,xe=0,ye=0;

	if (!mapcheck) {
		if (x<-31 || y<-31 || x>=MODEX || y>=MODEY)	return 0;

		if (x<0) {
			xs=-x;    x=0;
		}
		if (y<0) {
			ys=-y; y=0;
		}

		if (x+32>=MODEX) xe=x-MODEX+32;
		if (y+32>=MODEY) ye=y-MODEY+32;
	} else {
		if (x<-31 || y<170 || x>=MODEX || y>=MODEY)	return 0;

		if (x<0) {
			xs=-x;    x=0;
		}
		if (y<200) {
			ys=-y+200; y=200;
		}

		if (x+32>=MODEX) xe=x-MODEX+32;
		if (y+32>=MODEY) ye=y-MODEY+32;
	}

	rect.left=(nr%cachex)*TILE+xs;  rect.right=(nr%cachex)*TILE+TILE-xe;
	rect.top=(nr/cachex)*TILE+ys;   rect.bottom=(nr/cachex)*TILE+TILE-ye;


	//ret=DBBltFast(sur,x,y,suro,&rect);	
	ret=sur->lpVtbl->BltFast(sur,x,y,suro,&rect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);	

	if (ret!=DD_OK) {
		dd_error(0,"dd_copytile",ret); return -1;
	}
	return 0;
}

void dd_alphaeffect_magic_1(int nr,int str,int xpos,int ypos,int xoff,int yoff)
{
	int rx,ry;
	unsigned short *dst,d;
	int x,y,dstx,dsty,e;
	int r,g,b,r1,g1,b1,e2;

	rx=(xpos/2)+(ypos/2)-(2*16)+32+XPOS;
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+YPOS-2*32;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	dst=dd_get_ptr(sur2);

	dstx=rx;
	dsty=ry;

	for (y=0; y<64; y++) {
		if (dsty+y<200 || dsty+y>=MODEY) continue;
		for (x=0; x<64; x++) {
			if (dstx+x<0 || dstx+x>=MODEX) continue;

			d=dst[(dstx+x)+(dsty+y)*MODEX];

			r1=(d&0x7C00)>>10;
			g1=(d&0x03E0)>>5;
			b1=(d&0x001F);

			e=32;
			if (x<32) e-=32-x;
			if (x>31) e-=x-31;
			if (y<16) e-=16-y;
			if (y>55) e-=(y-55)*2;
			if (e<0) e=0;
			e/=max(1,str);

			e2=0;
			if (nr&1) e2+=e;
			if (nr&2) e2+=e;
			if (nr&4) e2+=e;

			r=r1-e2/2; if (r<0)	r=0;
			g=g1-e2/2; if (g<0)	g=0;
			b=b1-e2/2; if (b<0)	b=0;

			if (nr&1) {
				r=r+e;
				if (r>31) r=31;
			}
			if (nr&2) {
				g=g+e;
				if (g>31) g=31;
			}
			if (nr&4) {
				b=b+e;
				if (b>31) b=31;
			}
			dst[(dstx+x)+(dsty+y)*MODEX]=(unsigned short)((r<<10)+(g<<5)+b);
		}
	}

	dd_release_ptr(sur2);

}

void dd_alphaeffect_magic_0(int nr,int str,int xpos,int ypos,int xoff,int yoff)
{
	int rx,ry;
	unsigned short *dst,d;
	int x,y,dstx,dsty,e;
	int r,g,b,r1,g1,b1,e2;

	rx=(xpos/2)+(ypos/2)-(2*16)+32+XPOS;
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+YPOS-2*32;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	dst=dd_get_ptr(sur2);

	dstx=rx;
	dsty=ry;

	for (y=0; y<64; y++) {
		if (dsty+y<200 || dsty+y>=MODEY) continue;
		for (x=0; x<64; x++) {
			if (dstx+x<0 || dstx+x>=MODEX) continue;

			d=dst[(dstx+x)+(dsty+y)*MAXX];

			r1=(d&0xF800)>>11;
			g1=(d&0x07E0)>>5;
			b1=(d&0x001F);

			e=32;
			if (x<32) e-=32-x;
			if (x>31) e-=x-31;
			if (y<16) e-=16-y;
			if (y>55) e-=(y-55)*2;
			if (e<0) e=0;
			e/=max(1,str);

			e2=0;
			if (nr&1) e2+=e;
			if (nr&2) e2+=e;
			if (nr&4) e2+=e;

			r=r1-e2/2; if (r<0)	r=0;
			g=g1-e2; if (g<0) g=0;
			b=b1-e2/2; if (b<0)	b=0;

			if (nr&1) {
				r=r+e;
				if (r>31) r=31;
			}
			if (nr&2) {
				g=g+e*2;
				if (g>63) g=63;
			}
			if (nr&4) {
				b=b+e;
				if (b>31) b=31;
			}
			dst[(dstx+x)+(dsty+y)*MAXX]=(unsigned short)((r<<11)+(g<<5)+b);
		}
	}

	dd_release_ptr(sur2);
}

void dd_alphaeffect_magic(int nr,int str,int xpos,int ypos,int xoff,int yoff)
{
	if (RGBM==1) dd_alphaeffect_magic_1(nr,str,xpos,ypos,xoff,yoff);
	else dd_alphaeffect_magic_0(nr,str,xpos,ypos,xoff,yoff);	
}

HDC dd_getDC(LPDIRECTDRAWSURFACE sur)
{
	HDC hdc;

	suro->lpVtbl->GetDC(sur,&hdc);

	return hdc;
}

void dd_releaseDC(HDC hdc,LPDIRECTDRAWSURFACE sur)
{
	suro->lpVtbl->ReleaseDC(sur,hdc);
}

#ifndef DB_WINDOWED
void dd_flip(void)
{
	long ret;

	ret=sur1->lpVtbl->Flip(sur1,NULL,DDFLIP_WAIT);
	if (ret!=DD_OK) {
		dd_error(0,"dd_flip",ret);
	}
}
#else
void dd_flip(void)
{
	RECT rect;

	GetWindowRect(desk_hwnd,&rect);

	// determine bounds of window in a more sensible way !!!
	rect.bottom-=1;
	rect.left+=1;
	rect.right-=1;
	rect.top+=20;

	sur1->lpVtbl->Blt(sur1,&rect,sur2,NULL,DDBLT_WAIT,NULL);
}
#endif

/* Translate direct draw error code to human readable form. */
void dd_error(HWND hwnd,char *msg,long err)
{
	char *ptr;
	HDC hdc;

	ptr=get_dderr(err);

	if (err!=DDERR_SURFACELOST) {
		hdc=GetWindowDC(hwnd);
		TextOut(hdc,10,10,msg,strlen(msg));
		TextOut(hdc,10,30,ptr,strlen(ptr));
		ReleaseDC(hwnd,hdc);
	}
}


/* -------------
   Helper routines for dd_load_bitmap
   ------------- */

static void*__mem=NULL;
static int __msize=0;

void *load_file(char *file)
{
	int handle;
	int size;
	unsigned char *ptr;

	if (locks) xlog(0,"trying to read while locked");

	handle=open(file,O_RDONLY|O_BINARY);
	if (handle==-1) {
		return NULL;
	} // xlog(0,"File not found: %s",strerror(errno));
	size=lseek(handle,0L,SEEK_END);
	lseek(handle,0L,SEEK_SET);
	if (size>__msize) {
		if (__mem) xfree(__mem);
		__msize=max(2*1024*2024,size);
		__mem=xmalloc(__msize);
	}
	ptr=__mem;
	if (!ptr) {
		xlog(0,"memory low"); return NULL;
	}
	read(handle,ptr,size);
	close(handle);

	return ptr;
}

#pragma argsused
static void unload_file(void *ptr)
{
	//xfree(ptr);
	;
}

/* Load a bitmap and return it's contents */

void *dd_load_bitmap(char *name,int *xs,int *ys,LPDIRECTDRAWSURFACE sur)
{
	BITMAPINFO *info;
	BITMAPFILEHEADER *file_hdr;
	HDC hdc,hdcMem;
	char *ptr,*target;
	void *bits;
	HBITMAP hbm;

	ptr=load_file(name);
	if (ptr==NULL) return NULL;

	if (sur->lpVtbl->GetDC(sur,&hdc)!=DD_OK) {
		unload_file(ptr); sur->lpVtbl->ReleaseDC(sur,hdc); xlog(0,"cannot get dc"); return NULL;
	}

	file_hdr=(void*)ptr;
	bits=ptr+file_hdr->bfOffBits;
	info=(void*)&ptr[sizeof(BITMAPFILEHEADER)];

	*xs=info->bmiHeader.biWidth;
	*ys=info->bmiHeader.biHeight;

	hdcMem=CreateCompatibleDC(hdc);
	if (hdcMem==0) {
		unload_file(ptr); sur->lpVtbl->ReleaseDC(sur,hdc); xlog(0,"cannot create hdcmem"); return NULL;
	}

	hbm=CreateDIBitmap(hdc,&info->bmiHeader,CBM_INIT,bits,info,DIB_RGB_COLORS);
	if (hbm==0) {
		unload_file(ptr); sur->lpVtbl->ReleaseDC(sur,hdc); DeleteDC(hdcMem); xlog(0,"cannot create hbm (%d)",GetLastError()); return NULL;
	}

	target=xmalloc(*xs**ys*2);

	GetBitmapBits(hbm,*xs**ys*2,target);

	DeleteObject(hbm);

	DeleteDC(hdcMem);

	sur->lpVtbl->ReleaseDC(sur,hdc);

	unload_file(ptr);

	return target;
}

/* ------------------
     sprite manager
   ------------------*/

// note:
// tiles in the upper left corner of the cache area will be destroyed by loading a new
// sprite

#define MAXEFFECT	1024

#define current_tick GetTickCount()

static MEMORYSTATUS memstat;

struct sprtab {
	unsigned short *image;			// null means not loaded	
	unsigned char *alpha;			// null means no alpha information present
	unsigned short *cache;
	unsigned char xs;			// in tiles
	unsigned char ys;			// in tiles
	unsigned short alphacnt;
	unsigned int ticker;
	unsigned short avgcol;
};

struct cachetab {
	unsigned int sprite;
	unsigned int ticker;
	unsigned int effect;
	unsigned int visible;
};

static struct sprtab *sprtab=NULL;
static struct cachetab *cachetab=NULL;

int dd_cache_hit=0,dd_cache_miss=0;

void free_2nd_cache(void)
{
	int n,m,old=0,tmp,t;

	t=GetTickCount();

	while (usedmem+256*1024>maxmem) {
		for (n=0; n<MAXSPRITE; n++) {
			if (!sprtab[n].image) continue;
			if (!sprtab[n].ticker) continue;
			tmp=t-sprtab[n].ticker;
			if (tmp>old) old=tmp;
		}
		old-=old/3;
		for (n=0; n<MAXSPRITE; n++) {
			if (!sprtab[n].image) continue;
			if (!sprtab[n].ticker) continue;
			tmp=t-sprtab[n].ticker;

			if (tmp>old) {
				for (m=0; m<MAXEFFECT*sprtab[n].xs*sprtab[n].ys; m++) {
					tmp=sprtab[n].cache[m];
					if (cachetab[tmp].sprite) {
						usedvid--;
						cachetab[tmp].sprite=0;
						cachetab[tmp].ticker=0;
						cachetab[tmp].effect=0;
						cachetab[tmp].visible=0;
					}
				}
				xfree(sprtab[n].cache); blockgc++;
				xfree(sprtab[n].image); blockgc++;

				if (sprtab[n].alpha) { free(sprtab[n].alpha); }

				sprtab[n].image=NULL;
				sprtab[n].alpha=NULL;
				sprtab[n].alphacnt=0;
				sprtab[n].ticker=0;
				sprtab[n].avgcol=0;
			}

		}
	}
}

unsigned short avgcolor(unsigned short *ptr,int xs,int ys)
{
	int size=xs*ys,n;
	int r=0,g=0,b=0,c=0;

	for (n=0; n<size; n++) {
		switch (RGBM) {
			case 0:
				if (ptr[n]!=0xf81f) {
					r+=((ptr[n]&0xF800)>>11);
					g+=((ptr[n]&0x07E0)>>5);
					b+=((ptr[n]&0x001F));
					c++;
				}
				break;
			case 1:
				if (ptr[n]!=0x7c1f) {
					r+=((ptr[n]&0x7C00)>>10);
					g+=((ptr[n]&0x03E0)>>5);
					b+=((ptr[n]&0x001F));
					c++;
				}
				break;
			case 2:
				if (ptr[n]!=0xf81f) {
					b+=((ptr[n]&0x7C00)>>10);
					g+=((ptr[n]&0x03E0)>>5);
					r+=((ptr[n]&0x001F));
					c++;
				}
				break;
		}
	}
	if (c) {
		r/=c;
		g/=c;
		b/=c;
	}

	switch (RGBM) {
		case 0: return(unsigned short)((r<<11)+(g<<5)+b);
		case 1: return(unsigned short)((r<<10)+(g<<5)+b);
		case 2: return(unsigned short)((b<<11)+(g<<5)+r);
		default: return 42;
	}
}

int get_avgcol(int nr)
{
	return sprtab[nr].avgcol;
}

void *conv_load(int nr,int *xs,int *ys);

extern char path[];

void dd_load_sprite(int nr)
{
	int xs,ys,alphacnt=0;
	unsigned short *optr=NULL;
	unsigned char *alpha=NULL;
	char buf[80];
	FILE *fp;

	if (usedmem>maxmem) free_2nd_cache();

	if (!optr) {	// try to load from single png file
		sprintf(buf,"%sgfx\\%05d.png",path,nr);
		fp=fopen(buf,"rb");
		if (fp) {
			optr=dd_load_png(fp,&xs,&ys,&alpha,&alphacnt);
			fclose(fp);
		}
	}	
        if (!optr) {	// try to load from pnglib
		fp=load_pnglib(nr);
		if (fp) optr=dd_load_png(fp,&xs,&ys,&alpha,&alphacnt);
	} 
        if (!optr) {	// try to load from single bmp file
	    sprintf(buf,"%sgfx\\%05d.bmp",path,nr);
	    optr=dd_load_bitmap(buf,&xs,&ys,suro);
	}
	if (!optr) {	// try to load from bmplib
		optr=conv_load(nr,&xs,&ys);
	}
	if (!optr) {	// not found!
		sprintf(buf,"Sprite %d is missing",nr);
		xlog(0,buf);
		return;
	}

	sprtab[nr].image=optr;
	sprtab[nr].alpha=alpha;
	sprtab[nr].alphacnt=(unsigned short)alphacnt;
	sprtab[nr].xs=(unsigned char)(xs/TILE);
	sprtab[nr].ys=(unsigned char)(ys/TILE);
	sprtab[nr].avgcol=avgcolor(optr,xs,ys);

	sprtab[nr].cache=xcalloc(MAXEFFECT*sizeof(short),sprtab[nr].xs*sprtab[nr].ys);
}

void dd_invalidate_cache(void)
{
	int n;

	for (n=0; n<MAXCACHE; n++) {
		cachetab[n].ticker=0;
		cachetab[n].sprite=0;
		cachetab[n].effect=0;
		cachetab[n].visible=0;
	}
	usedvid=0;
}

void dd_init_sprites(void)
{
	GlobalMemoryStatus(&memstat);
	maxmem=max(8192*1024,memstat.dwTotalPhys-8192*1024);
	maxvid=MAXCACHE;

	if (!sprtab) sprtab=xcalloc(MAXSPRITE,sizeof(struct sprtab));
	if (!cachetab) cachetab=xcalloc(MAXCACHE,sizeof(struct cachetab));

	dd_set_background(RED+BLUE);
}

#define LEFFECT	(gamma-4880)   //120

int gamma=5000;

#pragma argsused
unsigned short do_effect(unsigned short val,int effect,int seed1,int seed2,int sprite)
{
	int r,g,b,invis=0,tmp,grey=0,infra=0,water=0,red=0,green=0;

	if (effect&16) { effect-=16; red=1; }//red border
	if (effect&32) { effect-=32; green=1; }//green border
	if (effect&64) { effect-=64; invis=1; } //blackened out
	if (effect&128) { effect-=128; grey=1; } //grey scale
	if (effect&256) { effect-=256; infra=1; } //grey scale
	if (effect&512) { effect-=512; water=1; } //grey scale

	switch (RGBM) {
		case 0:
			if (val!=0xf81f) {
				r=(val&0xf800)>>11;
				g=(val&0x07e0)>>5;
				b=(val&0x001f);

				/* !!!!!!!!  color channel stuff
				if (sprite>2000+30*1024) {
					int r2=0,g2=0,b2=0;

					r2+=r*300/256;
					g2+=r*300/256;
					b2+=r*300/256; 

					r2+=g*120/512;	// 224
					g2+=g*50/512;	// 167
					b2+=g*20/512;	// 144

					r2+=b*256/256;
					g2+=b*  0/256;
					b2+=b*  0/256;

                                        r=r2;
					g=g2*2;
					b=b2;

					if (r>31) r=31;
					if (g>63) g=63;
					if (b>31) b=31;

				} !!!!!! */

				if (effect) {
					r=(r*LEFFECT)/(effect*effect+LEFFECT);
					g=(g*LEFFECT)/(effect*effect+LEFFECT);
					b=(b*LEFFECT)/(effect*effect+LEFFECT);
				}

                                if (grey) {
					tmp=(r+(g/2)+b)/6;
					r=tmp;
					b=tmp;
					g=tmp*2;
				}

				if (infra) {
					tmp=(r+(g/2)+b)/3;
					r=tmp;
					b=0;
					g=0;
				}

				if (water) {
					tmp=(r+(g/2)+b)/2;
					if (tmp>31)	tmp=31;
					r=(r+tmp)/3;
					b=(b+tmp); if (b>31) b=31;
					g=((g+tmp)/3)*2;
				}

				if (gamma!=5000) {
					r=r*gamma/5000; if (r>31) r=31;
					g=g*gamma/5000; if (g>63) g=63;
					b=b*gamma/5000; if (b>31) b=31;
				}

				//if (red) { r+=15; if (r>31) r=31; }

				if (red) { 
					r*=2; if (r>31) r=31;
					g*=2; if (g>63) g=63;
					b*=2; if (b>31) b=31;					
				}

				if (green) { g+=31; if (g>63) g=63; }

				if (invis) {
					r=g=b=0;
				}

                                return(unsigned short)((r<<11)+(g<<5)+b);
			} else return val;

		case 1:
			if (val!=0x7C1F) {
				r=(val&0x7C00)>>10;
				g=(val&0x03E0)>>5;
				b=(val&0x001F);

				if (effect) {
					r=(r*LEFFECT)/(effect*effect+LEFFECT);
					g=(g*LEFFECT)/(effect*effect+LEFFECT);
					b=(b*LEFFECT)/(effect*effect+LEFFECT);
				}

				if (grey) {
					tmp=(r+g+b)/6;
					r=tmp;
					b=tmp;
					g=tmp;
				}

				if (infra) {
					tmp=(r+g+b)/3;
					r=tmp;
					b=0;
					g=0;
				}

				if (water) {
					tmp=(r+g+b)/2;
					if (tmp>31)	tmp=31;
					r=(r+tmp)/3;
					b=(b+tmp); if (b>31) b=31;
					g=(g+tmp)/3;
				}

				if (gamma!=5000) {
					r=r*gamma/5000; if (r>31) r=31;
					g=g*gamma/5000; if (g>31) g=31;
					b=b*gamma/5000; if (b>31) b=31;
				}

				if (red) { r+=15; if (r>31) r=31; }
				if (green) { g+=15; if (g>31) g=31; }

				if (invis) {
					r=g=b=0;
				}

				return(unsigned short)((r<<10)+(g<<5)+b);
			} else return val;

		case 2:
			if (val!=0xf81f) {
				b=(val&0xf800)>>11;
				g=(val&0x07e0)>>5;
				r=(val&0x001f);

				if (effect) {
					r=(r*LEFFECT)/(effect*effect+LEFFECT);
					g=(g*LEFFECT)/(effect*effect+LEFFECT);
					b=(b*LEFFECT)/(effect*effect+LEFFECT);
				}

				if (invis) {
					r=g=b=0;
				}

				if (grey) {
					tmp=(r+(g/2)+b)/6;
					r=tmp;
					b=tmp;
					g=tmp*2;
				}

				if (infra) {
					tmp=(r+(g/2)+b)/3;
					r=tmp;
					b=0;
					g=0;
				}

				if (water) {
					tmp=(r+g+b)/2;
					if (tmp>31)	tmp=31;
					r=(r+tmp)/3;
					b=(b+tmp); if (b>31) b=31;
					g=(g+tmp)/3;
				}

				if (gamma!=5000) {
					r=r*gamma/5000; if (r>31) r=31;
					g=g*gamma/5000; if (g>63) g=63;
					b=b*gamma/5000; if (b>31) b=31;
				}

				return(unsigned short)((b<<11)+(g<<5)+r);
			} else return val;

		default:    return val;
	}
}

int tile2cache(int tile,int sprite,int xpos,int ypos,int xs,int effect)
{
	int cx,cy,sx,sy,x,y,visible=0;
	unsigned short *screen,*image;
	extern int ticker;

	screen=dd_get_ptr(suro);
	if (!screen) return 0;

	image=sprtab[sprite].image;

	cx=(tile%cachex)*TILE;
	cy=(tile/cachex)*TILE;

	sx=xpos*32;
	sy=ypos*32;

	screen+=cx+cy*MAXXOVER;
	image+=sx+sy*xs*32;

	for (y=0; y<32; y++) {
		for (x=0; x<32; x++) {
			if ((*screen=do_effect(*image,effect,x,y,sprite))!=background)	visible=1;
			image++; screen++;
		}
		screen+=MAXXOVER-32;
		image+=(xs-1)*32;
	}


	dd_release_ptr(suro);

	return visible;
}

int gettile(unsigned int sprite,unsigned int effect,int x,int y,int xs)
{
	int n,old=0;
	unsigned int nr,oldrec=0;

	nr=(sprite<<16)+x+y*xs;

	n=sprtab[sprite].cache[(x+y*xs)*MAXEFFECT+effect];
	if (cachetab[n].sprite==nr && cachetab[n].effect==effect) {
		cachetab[n].ticker=0;
		dd_cache_hit++;
		if (!cachetab[n].visible) {
			invisible++; return -1;
		}
		return n;
	}

	for (n=0; n<MAXCACHE; n++) {
		if (cachetab[n].sprite==0) {
			oldrec=0x7fffffff; old=n;
		}
		if (cachetab[n].ticker>oldrec) {
			oldrec=cachetab[n].ticker; old=n;
		}
		cachetab[n].ticker++;
	}
	dd_cache_miss++;

	if (oldrec==0x7fffffff)	usedvid++;

	cachetab[old].visible=tile2cache(old,sprite,x,y,xs,effect);
	cachetab[old].sprite=nr;
	cachetab[old].ticker=random(1024)+24;
	cachetab[old].effect=effect;
	sprtab[sprite].cache[(x+y*xs)*MAXEFFECT+effect]=(short)old;

	if (!cachetab[old].visible) {
		invisible++; return -1;
	}

	return old;
}

void copysprite(int nr,int effect,int xpos,int ypos,int xoff,int yoff)
{
	unsigned int x,y,xs,ys,n,rx,ry;

	if (nr==0) return;

        // image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	xs=sprtab[nr].xs;
	ys=sprtab[nr].ys;
	sprtab[nr].ticker=current_tick;

	rx=(xpos/2)+(ypos/2)-(xs*16)+32+XPOS-((TILEX-34)/2*32);
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+YPOS-ys*32;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	for (y=0; y<ys; y++) {
		for (x=0; x<xs; x++) {
			n=gettile(nr,effect,x,y,xs);
			if (n==-1) continue;
			dd_copytile(n,rx+x*32,ry+y*32,sur2,1);
		}
	}
	if (sprtab[nr].alpha) display_alpha(sprtab[nr].alpha,sprtab[nr].alphacnt,rx,ry,effect);
}

void copyspritex(int nr,int xpos,int ypos,int effect)
{
	unsigned int x,y,xs,ys,n;

	if (nr==0) return;

	// image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	xs=sprtab[nr].xs;
	ys=sprtab[nr].ys;
	sprtab[nr].ticker=current_tick;

	for (y=0; y<ys; y++) {
		for (x=0; x<xs; x++) {
			n=gettile(nr,effect,x,y,xs);
			if (n==-1) continue;
			dd_copytile(n,xpos+x*32,ypos+y*32,sur2,0);
		}
	}
	if (sprtab[nr].alpha) display_alpha(sprtab[nr].alpha,sprtab[nr].alphacnt,xpos,ypos,effect);
}

void dd_savescreen(void)
{
	unsigned char *ptr;
	int n;
	HBITMAP hbm;
	HDC hdc;
	unsigned char *pix;
	static int lasttime=0;

	if (time(NULL)-lasttime<10) {
		xlog(0,"One moment please, I'm counting my bits!");
		return;
	}

	lasttime=time(NULL);

	if (!OpenClipboard(desk_hwnd)) return;
	if (!EmptyClipboard()) return;

	sur1->lpVtbl->GetDC(sur1,&hdc);

	hbm=CreateCompatibleBitmap(hdc,MODEX,MODEY);
	if (hbm==NULL) return;

	sur1->lpVtbl->ReleaseDC(sur1,hdc);

	pix=xmalloc(MODEX*MODEY*2);
	ptr=dd_get_ptr(sur1);
	if (!ptr) return;

	for (n=0; n<MODEX*MODEY*2; n++)	pix[n]=ptr[n];

	dd_release_ptr(sur1);

	if (!SetBitmapBits(hbm,MODEX*MODEY*2,pix)) return;
	if (!SetClipboardData(CF_BITMAP,hbm)) return;
	if (!CloseClipboard()) return;

	xfree(pix);

	xlog(2,"Placed screenshot in clipboard.");
}

void dd_gputc(int xpos,int ypos,int font,int c)
{
	unsigned short *fptr,*tptr;
	int x,y,off,nr;

	if (c>127) return;
	c-=32;

	if (font<0 || font>3) return;
	nr=700+font;

	// image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	off=c*6;
	tptr=dd_get_ptr(sur2);
	if (!tptr) return;
	tptr+=xpos+ypos*MAXX;
	fptr=sprtab[nr].image+off+576;
	sprtab[nr].ticker=current_tick;

	for (y=0; y<9; y++) {
		for (x=0; x<6; x++,tptr++,fptr++) {
			if (*fptr!=background)
				*tptr=*fptr;
		}
		tptr+=MAXX-6;
		fptr+=576-6;
	}

	dd_release_ptr(sur2);

	return;
}

void dd_gputtext(int xpos,int ypos,int font,char *text,int xoff,int yoff)
{
	int rx,ry,len;

	len=strlen(text);

	rx=(xpos/2)+(ypos/2)+32-((len*5)/2)+XPOS;
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+YPOS-64;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	while (*text) {
		if (rx<0 || rx>MODEX-7 || ry<0 || ry>MODEY-10) return;
		dd_gputc(rx,ry,font,*text);
		text++; rx+=6;
	}
}

void dd_putc(int xpos,int ypos,char font,int c)
{
	unsigned short *fptr,*tptr;
	int x,y,off,nr;

	if (c>127) return;
	c-=32;

	if (font<0 || font>3) return;

	nr=700+font;

	// image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	off=c*6;
	tptr=dd_get_ptr(sur2);
	if (!tptr) return;
	tptr+=xpos+ypos*MAXX;
	fptr=sprtab[nr].image+off+576;
	sprtab[nr].ticker=current_tick;

	for (y=0; y<9; y++) {
		for (x=0; x<6; x++,tptr++,fptr++) {
			if (*fptr!=background)
				*tptr=*fptr;
		}
		tptr+=MAXX-6;
		fptr+=576-6;
	}

	dd_release_ptr(sur2);

	return;
}

void dd_puttext(int x,int y,int font,char *text)
{
	while (*text) {
		dd_putc(x,y,(char)font,*text);
		text++; x+=6;
	}
}

void dd_xputtext(int x,int y,int font,char *format,...)
{
	va_list args;
	char buf[1024];

	va_start(args,format);
	vsprintf(buf,format,args);
	dd_puttext(x,y,font,buf);
	va_end(args);
}

int dd_isvisible(void)
{
	long ret;

	ret=sur1->lpVtbl->IsLost(sur1);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Restore(sur1);
	}
	ret=sur2->lpVtbl->IsLost(sur2);
	if (ret!=DD_OK) {
		sur2->lpVtbl->Restore(sur2);
	}
	ret=suro->lpVtbl->IsLost(suro);
	if (ret!=DD_OK) {
		suro->lpVtbl->Restore(suro); dd_invalidate_cache();
	}

	ret=sur1->lpVtbl->IsLost(sur1);
	if (ret!=DD_OK)	return 0;
	ret=sur2->lpVtbl->IsLost(sur2);
	if (ret!=DD_OK)	return 0;
	ret=suro->lpVtbl->IsLost(suro);
	if (ret!=DD_OK)	return 0;

	return 1;
}

void dd_show_map(unsigned short *src,int xo,int yo)
{
	unsigned short *dst;
	int x,y,d,s;

	dst=dd_get_ptr(sur2);
	if (!dst) return;

	for (y=0; y<128; y++) {
		d=(y+471)*MAXX+3;
		s=(y+yo)*1024+xo;
		for (x=0; x<128; x++) {
			dst[d++]=src[s++];
		}
	}
	dd_release_ptr(sur2);
}

void do_rgb8_effect(int *r1,int *g1,int *b1,int effect)
{
    int r,g,b,invis=0,tmp,grey=0,infra=0,water=0,red=0,green=0;

    if (effect&16) { effect-=16; red=1; } //red border
    if (effect&32) { effect-=32; green=1; } //green border
    if (effect&64) { effect-=64; invis=1; } //blackened out
    if (effect&128) { effect-=128; grey=1; } //grey scale
    if (effect&256) { effect-=256; infra=1; } //infrared
    if (effect&512) { effect-=512; water=1; } //under water
		
    r=*r1;
    g=*g1;
    b=*b1;

    if (effect) {
	    r=(r*LEFFECT)/(effect*effect+LEFFECT);
	    g=(g*LEFFECT)/(effect*effect+LEFFECT);
	    b=(b*LEFFECT)/(effect*effect+LEFFECT);
    }

    if (grey) {
	    tmp=(r+g+b)/6;
	    r=tmp;
	    b=tmp;
	    g=tmp;
    }

    if (infra) {
	    tmp=(r+g+b)/3;
	    r=tmp;
	    b=0;
	    g=0;
    }

    if (water) {
	    tmp=(r+g+b)/2;
	    if (tmp>31)	tmp=31;
	    r=(r+tmp)/3;
	    b=(b+tmp); if (b>31) b=31;
	    g=(g+tmp)/3;
    }

    if (gamma!=5000) {
	    r=r*gamma/5000; if (r>255) r=255;
	    g=g*gamma/5000; if (g>255) g=255;
	    b=b*gamma/5000; if (b>255) b=255;
    }

    //if (red) { r+=128; if (r>255) r=255; }
    if (red) { 
	    r*=2; if (r>255) r=255; 
	    g*=2; if (g>255) g=255; 
	    b*=2; if (b>255) b=255; 
	}
    if (green) { g+=128; if (g>255) g=255; }

    if (invis) {
	    r=g=b=0;
    }

    *r1=r;
    *g1=g;
    *b1=b;
}


void display_alpha(unsigned char *alpha,int alphacnt,int xf,int yf,int effect)
{
    unsigned short *dst,val;
    int x,y,r1,g1,b1,r2,b2,g2,a1,a2,r,g,b,n,pos;

    dst=dd_get_ptr(sur2);
    if (!dst) return;

    for (n=0; n<alphacnt; n+=6) {	
	x=alpha[n+0];
	y=alpha[n+1];
        r1=alpha[n+2];
	g1=alpha[n+3];
	b1=alpha[n+4];
	a1=alpha[n+5];
	a2=256-a1;

	do_rgb8_effect(&r1,&g1,&b1,effect);

	pos=x+xf+y*MAXX+yf*MAXX;
	val=dst[pos];

	if (RGBM==0) {
	    r2=(val&RED)>>8; 
	    g2=(val&GREEN)>>3;
	    b2=(val&BLUE)<<3;
	} else {
	    r2=(val&RED)>>7;
	    g2=(val&GREEN)>>2;
	    b2=(val&BLUE)<<3;
	}	

	r=(r1*a1+r2*a2)/256;
	g=(g1*a1+g2*a2)/256;
	b=(b1*a1+b2*a2)/256;

        if (RGBM==0) dst[pos]=(unsigned short)(((r<<8)&RED)+((g<<3)&GREEN)+((b>>3)&BLUE));
	else dst[pos]=(unsigned short)(((r<<7)&RED)+((g<<2)&GREEN)+((b>>3)&BLUE));
	
    }

    dd_release_ptr(sur2);
}

// displays a png in RGBA 8 bit format (memory blocks, not the silly rows-concept)
void display_png(unsigned char *png,int xf,int yf,int xs,int ys)
{
    unsigned short *dst;
    int x,y,r1,g1,b1,r2,b2,g2,a1,a2,r,g,b;

    dst=dd_get_ptr(sur2);
    if (!dst) return;

    for (y=0; y<ys; y++) {
	for (x=0; x<xs; x++) {
	    r1=png[x*4+y*xs*4+0];
	    g1=png[x*4+y*xs*4+1];
	    b1=png[x*4+y*xs*4+2];
	    a1=png[x*4+y*xs*4+3];
	    a2=256-a1;

            if (RGBM==0) {
		r2=(dst[x+xf+y*MAXX+yf*MAXX]&RED)>>8;
		g2=(dst[x+xf+y*MAXX+yf*MAXX]&GREEN)>>3;
		b2=(dst[x+xf+y*MAXX+yf*MAXX]&BLUE)<<3;
	    } else {
		r2=(dst[x+xf+y*MAXX+yf*MAXX]&RED)>>7;
		g2=(dst[x+xf+y*MAXX+yf*MAXX]&GREEN)>>2;
		b2=(dst[x+xf+y*MAXX+yf*MAXX]&BLUE)<<3;
	    }	

	    r=(r1*a1+r2*a2)/256;
	    g=(g1*a1+g2*a2)/256;
	    b=(b1*a1+b2*a2)/256;

            if (RGBM==0) dst[x+xf+y*MAXX+yf*MAXX]=(unsigned short)(((r<<8)&RED)+((g<<3)&GREEN)+((b>>3)&BLUE));
	    else dst[x+xf+y*MAXX+yf*MAXX]=(unsigned short)(((r<<7)&RED)+((g<<2)&GREEN)+((b>>3)&BLUE));
	}
    }

    dd_release_ptr(sur2);
}

// reads a PNG file and transforms the data into a 16 bit RGB data (returned as ptr to memory block)
// the alpha information is stored in alpha_ptr & alphacnt_ptr in the format x,y,r,g,b,a for all
// pixels actually having alpha information.
// pixel with alpha!=255 (i.e. partially or fully transparent pixels) are marked as the transparent
// color in the 16 bit RGB data to allow blitting them with FastBLT.
// the alpha information may be used, but doesnt need to - it just looks better...
unsigned short *dd_load_png(FILE *fp,int *xs,int *ys,unsigned char **alpha_ptr,int *alphacnt_ptr)
{
    int x,y,n,m,alphacnt=0,alphamax=0,noalpha=0,mul=4,tmp,r,g,b,a;
    unsigned char **row,*alpha=NULL;
    unsigned short *bmp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (!png_ptr) {
	xlog(0,"create read"); return 0;
    }

    info_ptr=png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL); xlog(0,"create info1"); return 0;
    }

    end_info=png_create_info_struct(png_ptr);
    if (!end_info) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"create info2"); return 0;
    }

    png_init_io(png_ptr,fp);

    png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_PACKING,NULL);

    row=png_get_rows(png_ptr,info_ptr);
    if (!row) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"read row"); return 0;
    }

    x=png_get_image_width(png_ptr,info_ptr);
    y=png_get_image_height(png_ptr,info_ptr);

    tmp=png_get_rowbytes(png_ptr,info_ptr);
    if (tmp==x*3) { noalpha=1; mul=3; }
    else if (tmp!=x*4) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"rowbytes!=x*4 (%d)",tmp); return 0;
    }
    if (png_get_bit_depth(png_ptr,info_ptr)!=8) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"bit depth!=8"); return 0;
    }
    if (png_get_channels(png_ptr, info_ptr)!=mul) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"channels!=mul"); return 0;
    }

    bmp=xmalloc(x*y*2);

    for (n=0; n<y; n++) {
	for (m=0; m<x; m++) {
	    if (noalpha) {
		if (row[n][m*mul]>250 && row[n][m*mul+1]<5 && row[n][m*mul+2]>250) a=0;
		else a=255;
	    } else a=row[n][m*mul+3];

	    // this gets the rgb info from the png and scales the colors to their original value. 3dsmax will mix the image
	    // color and the background color AND supply alpha information, which is just plain wrong...
	    if (a) {
		    r=min(255,row[n][m*mul+0]*255/a);
		    g=min(255,row[n][m*mul+1]*255/a);
		    b=min(255,row[n][m*mul+2]*255/a);
	    } else {
		    r=row[n][m*mul+0];
		    g=row[n][m*mul+1];
		    b=row[n][m*mul+2];
	    }

	    if ((do_alpha==0 && a<64) ||
		(do_alpha==1 && a<192) ||
		(do_alpha==2 && a<255))	bmp[m+n*x]=(unsigned short)(RED|BLUE);
	    else if (RGBM==0) bmp[m+n*x]=(unsigned short)(((r<<8)&RED)+((g<<3)&GREEN)+((b>>3)&BLUE));
	    else bmp[m+n*x]=(unsigned short)(((r<<7)&RED)+((g<<2)&GREEN)+((b>>3)&BLUE));

	    if (!noalpha && ((do_alpha==1 && a>63 && a<192) || (do_alpha==2 && a!=0 && a!=255))) {
		if (alphacnt>=alphamax-8) {
		    alphamax+=32;
		    alpha=realloc(alpha,alphamax);
		}
		*(unsigned char*)(alpha+alphacnt)=(unsigned char)m; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=(unsigned char)n; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=r; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=g; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=b; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=a; alphacnt+=1;

		alphapix++;
	    } else fullpix++;
	}
    }

    if (alpha) alpha=realloc(alpha,alphacnt);

    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);

    *alpha_ptr=alpha;
    *alphacnt_ptr=alphacnt;
    *xs=x;
    *ys=y;

    return bmp;
}

void dd_invalidate_alpha(void)
{
    int n,tmp,m;

    for (n=0; n<MAXSPRITE; n++) {
	if (!sprtab[n].image) continue;

	for (m=0; m<MAXEFFECT*sprtab[n].xs*sprtab[n].ys; m++) {
            tmp=sprtab[n].cache[m];
		if (cachetab[tmp].sprite) {
			usedvid--;
			cachetab[tmp].sprite=0;
			cachetab[tmp].ticker=0;
			cachetab[tmp].effect=0;
			cachetab[tmp].visible=0;
		}
	}
	xfree(sprtab[n].cache); blockgc++;
	xfree(sprtab[n].image); blockgc++;

	if (sprtab[n].alpha) { free(sprtab[n].alpha); }

	sprtab[n].image=NULL;
	sprtab[n].alpha=NULL;
	sprtab[n].alphacnt=0;
	sprtab[n].ticker=0;
	sprtab[n].avgcol=0;
    }
}

static unsigned char *shadowmap;

void dd_shadow_clear(void)
{
	if (!shadowmap) shadowmap=calloc(MAXX,MAXY);
	else memset(shadowmap,0,MAXX*MAXY);
}

void dd_shadow(int nr,int xpos,int ypos,int xoff,int yoff)
{
	unsigned int x,y,xs,ys,rx,ry,m,p,v,ok=0,disp,swap=0,n;
	unsigned short *dst,*src;

	if (nr==0) return;

	if (nr>=2000 && nr<16336) { ok=1; disp=14; }
	if (nr>17360) { ok=1; disp=14; }

	if (!ok) return;

        // image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	xs=sprtab[nr].xs;
	ys=sprtab[nr].ys;
	sprtab[nr].ticker=current_tick;

	rx=(xpos/2)+(ypos/2)-(xs*16)+32+XPOS-((TILEX-34)/2*32);
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+YPOS-ys*32;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	ry+=ys*32-disp;

	dst=dd_get_ptr(sur2);
	if (!dst) return;

	src=sprtab[nr].image;

	for (y=ry+ys*8,m=0; y>ry; y--,m+=xs*96) {
                for (x=rx; x<rx+xs*32; x++,m++) {
                        if (src[m]!=(RED|BLUE)) {
                                
				if (swap==1) p=x+y*MAXX+(int)((x-rx)*0.885)*MAXX;
				else if (swap==2) p=x+y*MAXX-(int)((x-rx)*0.885)*MAXX;
                                else p=x+y*MAXX;

				if (p>=(unsigned)(MAXX*MAXY)) continue;
				if (shadowmap[p]) continue;

				v=dst[p];
				v>>=1;
				if (RGBM==0) v&=(0x7800|0x03E0|0x000F); else v&=(0x3C00|0x01E0|0x000F);
				dst[p]=v;

				shadowmap[p]=1;
			}
		}
	}

	if (sprtab[nr].alpha) {	
		for (n=0; n<(unsigned)sprtab[nr].alphacnt; n+=6) {	
			
			x=sprtab[nr].alpha[n+0]+rx;
			y=ry+ys*8-(sprtab[nr].alpha[n+1]/4);

			if (swap==1) p=x+y*MAXX+(int)((x-rx)*0.885)*MAXX;
			else if (swap==2) p=x+y*MAXX-(int)((x-rx)*0.885)*MAXX;
			else p=x+y*MAXX;
	
			if (p>=(unsigned)(MAXX*MAXY)) continue;
			if (shadowmap[p]) continue;
	
			v=dst[p];
			v>>=1;
			if (RGBM==0) v&=(0x7800|0x03E0|0x000F); else v&=(0x3C00|0x01E0|0x000F);
                        dst[p]=v;
	
			shadowmap[p]=1;
		}
	}
        
	dd_release_ptr(sur2);
}

/*
	switch(nr) {
		case 106:	disp=12; ok=1; break;		// big pillar

		case 460:
		case 461:
		case 462:
		case 463:
		case 464:       disp=12; ok=1; break;		// big candle on

		case 594:
		case 595:	disp=12; ok=1; break;		// big street lamp

		case 792:	disp=12; ok=1; break;		// shrine

		case 946:	disp=12; ok=1; break;		// thick pillar

		case 968:	disp=8; ok=1; swap=2; break;	// iron bars
		case 976:	disp=22; ok=1; swap=1; break;	// iron bars
		case 978:	disp=22; ok=1; swap=1; break;	// iron bars with door

		case 997:	disp=30; ok=1; swap=1; break;	// iron door open
		case 998:	disp=8; ok=1; swap=2; break;	// iron door closed

		case 1102:	
		case 1104:
		case 1106:
		case 1108:	disp=11; ok=1; break;		// golden candle on

		case 1026:	
		case 1028:
		case 1030:
		case 1032:	disp=11; ok=1; break;		// blue candle on

		case 1112:	disp=10; ok=1; break;		// tree
		case 1114:	disp=10; ok=1; break;		// tree

		case 1154:      disp=8; ok=1; break;		// red flower
		case 1155:	disp=8; ok=1; break;		// small green plant
		case 1184:	disp=8; ok=1; break;		// red flower

		case 1160:	disp=12; ok=1; break;		// marble pillar

		case 1162:      disp=11; ok=1; break;		// golden candle off

		case 1166:      disp=11; ok=1; break;		// blue candle off

		case 1186:	disp=24; ok=1; swap=1; break;	// street sign
		case 1190:	disp=24; ok=1; swap=1; break;	// street sign
		case 1194:	disp=0; ok=1; swap=2; break;	// street sign
		case 1196:	disp=0; ok=1; swap=2; break;	// street sign

		case 1234:
		case 1235:	disp=8; ok=1; break;		// purple flower
		case 1236:	disp=8; ok=1; break;		// small green plant
		case 1238:	disp=12; ok=1; break;		// tree
		case 1240:	disp=12; ok=1; break;		// tree

		case 1625:	disp=12; ok=1; break;		// titan helmet

		case 16510:					// fire in pot
		case 16511:					// fire in pot
		case 16512:					// fire in pot
		case 16513:					// fire in pot
		case 16514:	disp=8; ok=1; break;		// fire in pot
				
		case 16969:	disp=8; ok=1; break;		// trashcan
		

				
	}
*/
