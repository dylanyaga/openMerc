/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#include "gendefs.h"
#include "data.h"
#include "driver.h"

#define CHARSIZE (sizeof(struct character)*MAXCHARS)

struct character *ch;

static int load(void) 
{
	int handle;
	
	handle=open(DATDIR"/char.dat",O_RDWR);
	if (handle==-1)	{ 
		fprintf(stderr,"char.dat does not exist.\n"); 
		return -1;
	}

	ch=mmap(NULL,CHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
	if (ch==(void*)-1) { 
		fprintf(stderr,"cannot mmap char.dat.\n"); 
		return -1;
	}
	close(handle);	

	return 0;
}

static void unload(void)
{
	if (munmap(ch,CHARSIZE)) perror("munmap(ch)");
}

int main(int argc,char *args[]) 
{
	int cn;
	
	if (argc!=2) {
		fprintf(stderr,"Usage: %s <name>\n",args[0]);
		exit(1);
	}
	
	if (load()) { fprintf(stderr,"Cannot access server data. Exiting...\n"); unload(); exit(1); }
	
	for (cn=1; cn<MAXCHARS; cn++) {
		if (!ch[cn].used) continue;
		if (!(ch[cn].flags&CF_PLAYER)) continue;
		if (strcasecmp(ch[cn].name,args[1])==0) break;
	}
	
	if (cn==MAXCHARS) {
		printf("Could not find player %s.\n",args[1]);
		unload();
		exit(2);
	}
	
	ch[cn].pass1=random();
	ch[cn].pass2=random();
	
	printf("Changed player %s (%d)'s password.\n",ch[cn].name,cn);
	
	unload();
	
	return 0;
}
