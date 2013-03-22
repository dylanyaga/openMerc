#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dir.h>
#pragma hdrstop
#include "dd.h"
#include "common.h"
#include "inter.h"
#include "merc.rh"

char path[256]="";

void *xmalloc(int size);

extern HWND desk_hwnd;

extern int RED,GREEN,BLUE,RGBM,MAXXOVER;

struct gfx {
	int xs,ys;
	int off;
};

void convert(HWND hwnd)
{
	unsigned short *dat;
	int m,n,handle,hidx,hdat,len,xs,ys,offset=0,size,c,x,y;
	unsigned int r,g,b;
	char buf[512];
	unsigned char *ptr,*bits;
	BITMAPINFO *info;
	BITMAPFILEHEADER *file_hdr;
	struct gfx gfx;

	hidx=open("gx00.idx",O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0600);
	hdat=open("gx00.dat",O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0600);

	for (n=0; n<40000; n++) {

		sprintf(buf,"STATUS: Preparing gx00.dat: %3.0f%% done",100.0/40000.0*n);
		SetDlgItemText(hwnd,IDC_STATUS,buf);

		sprintf(buf,"gfx\\%05d.bmp",n);

		handle=open(buf,O_RDONLY|O_BINARY);
		if (handle==-1) {
			gfx.xs=gfx.ys=gfx.off=0;
			write(hidx,&gfx,sizeof(gfx));
			continue;
		}

		len=filelength(handle);
		ptr=malloc(len);
		read(handle,ptr,len);
		close(handle);

		file_hdr=(void*)ptr;
		bits=ptr+file_hdr->bfOffBits;
		info=(void*)&ptr[sizeof(BITMAPFILEHEADER)];

		xs=info->bmiHeader.biWidth;
		ys=info->bmiHeader.biHeight;
		size=xs*ys;
		dat=malloc(size*2);

		if (info->bmiHeader.biPlanes!=1 ||
		    info->bmiHeader.biBitCount!=24 ||
		    info->bmiHeader.biCompression) {
			free(ptr);
			gfx.xs=gfx.ys=gfx.off=0;
			write(hidx,&gfx,sizeof(gfx));
			continue;
		}

		lseek(hidx,n*sizeof(struct gfx),SEEK_SET);
		gfx.xs=xs;
		gfx.ys=ys;
		gfx.off=offset;
		write(hidx,&gfx,sizeof(gfx));

		for (c=0,y=ys-1; y>=0; y--) {
			m=y*xs*3;
			for (x=0; x<xs*3; x+=3,c++) {
				r=bits[m+x]>>3;
				g=bits[m+x+1]>>2;
				b=bits[m+x+2]>>3;
				dat[c]=(unsigned short)((r)+(g<<5)+(b<<11));
			}
		}
		write(hdat,dat,size*2); offset+=size*2;

		free(ptr);
		free(dat);
	}
	close(hidx);
	close(hdat);
}

struct gfx *gfx;
int gfxlen=0;
int hdat;

void conv_init(void)
{
	int hidx,len;
	char file[256];

	sprintf(file,"%sgx00.idx",path);

	hidx=open(file,O_RDONLY|O_BINARY);
	if (hidx==-1) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Graphics file gx00.idx not found!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}

	len=filelength(hidx);
	gfx=xmalloc(len);
	if (!gfx) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Error allocating menory!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}
	if (len<sizeof(struct gfx)*40000) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Graphics file gx00.idx corrupt!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}
	read(hidx,gfx,len);
	close(hidx);

	sprintf(file,"%sgx00.dat",path);

	hdat=open(file,O_RDONLY|O_BINARY);
	if (hidx==-1) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Graphics file gx00.dat not found!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}
}

void *conv_load(int nr,int *xs,int *ys)
{
	unsigned int r,g1,g2,b;
	int len,n,size;
	unsigned short *ptr;

	if (nr>=40000 || gfx[nr].xs==0)	return NULL;

	if (gfx[nr].xs<0 || gfx[nr].xs>1000 || gfx[nr].ys<0 || gfx[nr].ys>1000) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Graphics file gx00.idx is corrupt!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}

	len=gfx[nr].xs*gfx[nr].ys*2;
	ptr=xmalloc(len);
	if (!ptr) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Error allocating memory!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}

	lseek(hdat,gfx[nr].off,SEEK_SET);
	read(hdat,ptr,len);

	*xs=gfx[nr].xs;
	*ys=gfx[nr].ys;

	if (RGBM==0) return ptr;

	size=gfx[nr].xs*gfx[nr].ys;

	for (n=0; n<size; n++) {
		r=ptr[n]>>11;
		g1=(ptr[n]&0x7e0)>>5;
		g2=(ptr[n]&0x7e0)>>6;
		b=ptr[n]&0x1f;

		switch (RGBM) {
			case  1:        ptr[n]=(unsigned short)((r<<10)|(g2<<5)|(b)); break;
			case  2:        ptr[n]=(unsigned short)((r)|(g1<<5)|(b<<11)); break;
		}
	}

	return ptr;
}

int create_pnglib(HWND hwnd)
{
	int n,nr,hidx,hpng,hin,off;
	char *buf;
	int *idx;
	struct ffblk ff;

	hidx=open("pnglib.idx",O_WRONLY|O_TRUNC|O_CREAT|O_BINARY,0666);
	hpng=open("pnglib.dat",O_WRONLY|O_TRUNC|O_CREAT|O_BINARY,0666);

	if (hidx==-1 || hpng==-1) {
		MessageBox(0,"creating files failed in create_pnglib()","oops",MB_OK);
		return 0;
	}

	idx=calloc(MAXSPRITE,sizeof(int));
	buf=malloc(1024*256);

	write(hpng,"SCAR",4); off=4;

	chdir("gfx");
	if (!findfirst("*.png",&ff,FA_ARCH)) do {
		nr=atoi(ff.ff_name);
		if (nr<0 || nr>=MAXSPRITE) continue;
		sprintf(buf,"STATUS: Preparing pnglib.dat: %d",nr);
		SetDlgItemText(hwnd,IDC_STATUS,buf);

		hin=open(ff.ff_name,O_RDONLY|O_BINARY);
		if (hin==-1) continue;

		n=read(hin,buf,1024*256);
		close(hin);
		if (n<1) continue;
		write(hpng,buf,n);

		idx[nr]=off;
		off+=n;

		//printf("off=%d, nr=%d (%d)\n",off,nr,idx[nr]);
	} while (!findnext(&ff));
	chdir("..");

	for (n=0; n<MAXSPRITE; n++) {
            write(hidx,idx+n,sizeof(int));
	}

	close(hidx);
	close(hpng);	

        return 1;
}

static FILE *fpng=NULL;
static int *idx=NULL;

int init_pnglib(void)
{
	int hidx,n;
	char file[80];

	idx=calloc(MAXSPRITE,sizeof(int));

	sprintf(file,"%spnglib.idx",path);
	hidx=open(file,O_RDONLY|O_BINARY);
	if (hidx==-1) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Graphics file pnglib.idx not found!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}

	for (n=0; n<MAXSPRITE; n++) {
			if (read(hidx,idx+n,sizeof(int))<sizeof(int)) break;
	}
	/*if (n<MAXSPRITE) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Graphics file pnglib.idx corrupt!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}*/
	sprintf(file,"%spnglib.dat",path);
	fpng=fopen(file,"rb");
	if (fpng==NULL) {
		dd_deinit();
		save_options();
		MessageBox(desk_hwnd,"Graphics file pnglib.dat not found!","Error",MB_ICONSTOP|MB_OK);
		exit(1);
	}

	return 1;
}

FILE *load_pnglib(int nr)
{
	int off;

	if (nr<0 || nr>=MAXSPRITE) return NULL;

	off=idx[nr];
    if (!off) return NULL;

    fseek(fpng,off,SEEK_SET);

	return fpng;
}
