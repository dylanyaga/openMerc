/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include "gendefs.h"
#include "data.h"

/* CS, 991113: TCHARSIZE and TITEMSIZE now in data.h */

struct character *ch;
struct item *it;
struct global *globs;

static int load(void)
{
        int handle;
        char cwd[128];

        handle=open(DATDIR"/tchar.dat",O_RDWR);
        if (handle==-1) {
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                        fprintf(stderr, "cwd: %s\n", cwd);
                perror(DATDIR"/tchar.dat");
                return -1;
        }

        ch=mmap(NULL,TCHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (ch==(void*)-1) {
                fprintf(stderr,"cannot mmap tchar.dat.\n");
                return -1;
        }
        close(handle);

        handle=open(DATDIR"/titem.dat",O_RDWR);
        if (handle==-1) {
                fprintf(stderr,"titem.dat does not exist.\n");
                return -1;
        }

        it=mmap(NULL,TITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (it==(void*)-1) {
                fprintf(stderr,"cannot mmap titem.dat.\n");
                return -1;
        }
        close(handle);

        handle=open(DATDIR"/global.dat",O_RDWR);
        if (handle==-1) return -1;

        globs=mmap(NULL,sizeof(struct global),PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (globs==(void*)-1) return -1;
        close(handle);

        return 0;
}

static void unload(void)
{
        if (munmap(ch,TCHARSIZE)) perror("munmap(ch)");
        if (munmap(it,TITEMSIZE)) perror("munmap(it)");
        if (munmap(globs,sizeof(struct global))) perror("munmap(globs)");
}


int main(int argc, char *args[])
{
        int n;

        chdir("/home/merc");

        load();
        
        n=atoi(args[1]);
        if (n==0) {
        	for (n=0; n<MAXTCHARS; n++) {
        		if (strstr(ch[n].name,args[1]))  printf("%d: %s (%d)\n",n,ch[n].name,ch[n].class);
        	}
        } else {
	        globs->reset_char=n;
	        printf("%d: %s\n",n,ch[n].name);
	}


	unload();
	exit(0);

}
