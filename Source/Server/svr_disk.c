/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "server.h"


struct map *map;
struct character *ch;
struct item *it;
struct character *ch_temp;
struct item *it_temp;
struct effect *fx;
struct global *globs;

/* Extend a file opened with <handle> to <sizereq> by writing blocks of size <sizeone>, optionally from <templ> */
int extend(int handle, long sizereq, size_t sizeone, void*templ)
{
        long length;
        void *buffer;

        length = lseek(handle, 0L, SEEK_END);
        if (length < sizereq) {
                xlog("Current size = %ldK, extending to %ldK", length/1024, sizereq/1024);
                buffer = templ;
                if (buffer == NULL) buffer = calloc(1, sizeone); // automatically clears memory
                if (buffer == NULL) return 0; // calloc() failure
                for ( ; length < sizereq; length+=sizeone) {
                        write(handle, buffer, sizeone);
                }
                if (templ == NULL) free(buffer);
        }
        return 1; // success
}


/* CS, 991113: Use extend() to extend files where necessary */
int load(void)
{
        int handle,flag=0;
        struct map tmap;

        /** MAP **/
        xlog("Loading MAP: Item size=%d, file size=%dK",
                sizeof(struct map),MAPSIZE>>10);

        handle=open(DATDIR"/map.dat",O_RDWR);
        if (handle==-1) {
                flag=1;
                xlog("Building map");
                handle=open(DATDIR"/map.dat",O_RDWR|O_CREAT,0600);
        }
        bzero(&tmap, sizeof(struct map));
        tmap.sprite = SPR_GROUND1;
        if (!extend(handle, MAPSIZE, sizeof(struct map), &tmap)) return -1;

        map=mmap(NULL,MAPSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (map==(void*)-1) return -1;
        close(handle);

        /** CHAR **/
        xlog("Loading CHAR: Item size=%d, file size=%dK",
                sizeof(struct character),CHARSIZE>>10);

        handle=open(DATDIR"/char.dat",O_RDWR);
        if (handle==-1) {
                xlog("Building characters");
                handle=open(DATDIR"/char.dat",O_RDWR|O_CREAT,0600);
        }
        if (!extend(handle, CHARSIZE, sizeof(struct character), NULL)) return -1;

        ch=mmap(NULL,CHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (ch==(void*)-1) return -1;
        close(handle);

        /** TCHAR **/
        xlog("Loading TCHAR: Item size=%d, file size=%dK",
                sizeof(struct character),TCHARSIZE>>10);

        handle=open(DATDIR"/tchar.dat",O_RDWR);
        if (handle==-1) {
                xlog("Building tcharacters");
                handle=open(DATDIR"/tchar.dat",O_RDWR|O_CREAT,0600);
        }
        if (!extend(handle, TCHARSIZE, sizeof(struct character), NULL)) return -1;

        ch_temp=mmap(NULL,TCHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (ch_temp==(void*)-1) return -1;
        close(handle);

        /** ITEM **/
        xlog("Loading ITEM: Item size=%d, file size=%dK",
                sizeof(struct item),ITEMSIZE>>10);

        handle=open(DATDIR"/item.dat",O_RDWR);
        if (handle==-1) {
                xlog("Building items");
                handle=open(DATDIR"/item.dat",O_RDWR|O_CREAT,0600);
        }
        if (!extend(handle, ITEMSIZE, sizeof(struct item), NULL)) return -1;


        it=mmap(NULL,ITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (it==(void*)-1) return -1;
        close(handle);

        /** TITEM **/
        xlog("Loading TITEM: Item size=%d, file size=%dK",
                sizeof(struct item),TITEMSIZE>>10);

        handle=open(DATDIR"/titem.dat",O_RDWR);
        if (handle==-1) {
                xlog("Building titems");
                handle=open(DATDIR"/titem.dat",O_RDWR|O_CREAT,0600);
        }
        if (!extend(handle, TITEMSIZE, sizeof(struct item), NULL)) return -1;

        it_temp=mmap(NULL,TITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (it==(void*)-1) return -1;
        close(handle);

        /** EFFECT **/
        xlog("Loading EFFECT: Item size=%d, file size=%dK",
                sizeof(struct effect),EFFECTSIZE>>10);

        handle=open(DATDIR"/effect.dat",O_RDWR);
        if (handle==-1) {
                xlog("Building effects");
                handle=open(DATDIR"/effect.dat",O_RDWR|O_CREAT,0600);
        }
        if (!extend(handle, EFFECTSIZE, sizeof(struct effect), NULL)) return -1;

        fx=mmap(NULL,EFFECTSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (fx==(void*)-1) return -1;
        close(handle);

        /** GLOBS **/
        xlog("Loading GLOBS: Item size=%d, file size=%db",
                sizeof(struct global),sizeof(struct global));

        handle=open(DATDIR"/global.dat",O_RDWR);
        if (handle==-1) {
                xlog("Building globals");
                handle=open(DATDIR"/global.dat",O_RDWR|O_CREAT,0600);
        }
        if (!extend(handle, GLOBSIZE, sizeof(struct global), NULL)) return -1;

        globs=mmap(NULL,sizeof(struct global),PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (globs==(void*)-1) return -1;
        close(handle);

        return 0;
}

void update(void)
{
	xlog("SYNC");
        if (msync(map,MAPSIZE,MS_ASYNC)) xlog("ERROR: msync(map) %s",strerror(errno));
        if (msync(ch,CHARSIZE,MS_ASYNC)) xlog("ERROR msync(ch) %s",strerror(errno));
        if (msync(it,ITEMSIZE,MS_ASYNC)) xlog("ERROR: msync(it) %s",strerror(errno));
        if (msync(ch_temp,TCHARSIZE,MS_ASYNC)) xlog("ERROR msync(ch_temp) %s",strerror(errno));
        if (msync(it_temp,TITEMSIZE,MS_ASYNC)) xlog("ERROR: msync(it_temp) %s",strerror(errno));
        if (msync(fx,EFFECTSIZE,MS_ASYNC)) xlog("ERROR: msync(fx) %s",strerror(errno));
        if (msync(globs,sizeof(struct global),MS_ASYNC)) xlog("ERROR: msync(globs) %s",strerror(errno));
}

void flush_char(int cn)
{
	int in,n;
	
	msync(&ch[cn],sizeof(struct character),MS_ASYNC);
	
	for (n=0; n<40; n++) {
		if (IS_SANEITEM(in=ch[cn].item[n]))
			msync(&it[in],sizeof(struct item),MS_ASYNC);
	}
	
	for (n=0; n<20; n++) {
		if (IS_SANEITEM(in=ch[cn].worn[n]))
			msync(&it[in],sizeof(struct item),MS_ASYNC);
	}
	
	for (n=0; n<20; n++) {
		if (IS_SANEITEM(in=ch[cn].spell[n]))
			msync(&it[in],sizeof(struct item),MS_ASYNC);
	}
	
	for (n=0; n<62; n++) {
		if (IS_SANEITEM(in=ch[cn].depot[n]))
			msync(&it[in],sizeof(struct item),MS_ASYNC);
	}
}

void flush(void)
{
        xlog("Flushing cache (sync)");
        if (msync(map,MAPSIZE,MS_SYNC)) xlog("ERROR: msync(map) %s",strerror(errno));
        if (msync(ch,CHARSIZE,MS_SYNC)) xlog("ERROR msync(ch) %s",strerror(errno));
        if (msync(it,ITEMSIZE,MS_SYNC)) xlog("ERROR: msync(it) %s",strerror(errno));
        if (msync(ch_temp,TCHARSIZE,MS_SYNC)) xlog("ERROR msync(ch_temp) %s",strerror(errno));
        if (msync(it_temp,TITEMSIZE,MS_SYNC)) xlog("ERROR: msync(it_temp) %s",strerror(errno));
        if (msync(fx,EFFECTSIZE,MS_SYNC)) xlog("ERROR: msync(fx) %s",strerror(errno));
        if (msync(globs,sizeof(struct global),MS_SYNC)) xlog("ERROR: msync(globs) %s",strerror(errno));
}

void unload(void)
{
        xlog("Unloading data files");
        if (munmap(map,MAPSIZE)) xlog("ERROR: munmap(map) %s",strerror(errno));
        if (munmap(ch,CHARSIZE)) xlog("ERROR: munmap(ch) %s",strerror(errno));
        if (munmap(it,ITEMSIZE)) xlog("ERROR: munmap(it) %s",strerror(errno));
        if (munmap(ch_temp,TCHARSIZE)) xlog("ERROR: munmap(ch_temp) %s",strerror(errno));
        if (munmap(it_temp,TITEMSIZE)) xlog("ERROR: munmap(it_temp) %s",strerror(errno));
        if (munmap(fx,EFFECTSIZE)) xlog("ERROR munmap(fx) %s",strerror(errno));
        if (munmap(globs,sizeof(struct global))) xlog("ERROR: munmap(globs) %s",strerror(errno));
}
