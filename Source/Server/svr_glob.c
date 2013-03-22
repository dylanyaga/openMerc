/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>

#include "server.h"

#define MD_HOUR 3600
#define MD_DAY	(MD_HOUR*24)
#define MD_YEAR	300

void pay_rent(void)
{
	int cn;

	for (cn=1; cn<MAXCHARS; cn++) {
		if (ch[cn].used==USE_EMPTY) continue;
		if (!(ch[cn].flags&(CF_PLAYER))) continue;

		do_pay_depot(cn);
	}
}

void do_misc(void)
{
	int cn;

	for (cn=1; cn<MAXCHARS; cn++) {
		if (ch[cn].used==USE_EMPTY) continue;
		if (!(ch[cn].flags&(CF_PLAYER))) continue;

		if (count_uniques(cn)>1) {
			if (ch[cn].used==USE_ACTIVE) {
				ch[cn].luck-=5;
				chlog(cn,"reduced luck by 5 to %d for having more than one unique",ch[cn].luck);
			}
		} else {
			if (ch[cn].luck<0) ch[cn].luck++;
			if (ch[cn].luck<0) ch[cn].luck++;
		}
		ch[cn].flags&=~(CF_SHUTUP|CF_NODESC|CF_KICKED);
	}
}

void global_tick(void)
{
	int tmp;

	globs->mdtime++;
	if (globs->mdtime>=MD_DAY) {
		globs->mdday++;
		globs->mdtime=0;
		xlog("day %d of the year %d begins",globs->mdday,globs->mdyear);
		pay_rent();
		do_misc();
	} 

	if (globs->mdday>=MD_YEAR) {
		globs->mdyear++;
		globs->mdday=1;
	}

	if (globs->mdtime<MD_HOUR*6) globs->dlight=0;
	else if (globs->mdtime<MD_HOUR*7) globs->dlight=(globs->mdtime-MD_HOUR*6)*255/MD_HOUR;
	else if (globs->mdtime<MD_HOUR*22) globs->dlight=255;
	else if (globs->mdtime<MD_HOUR*23) globs->dlight=(MD_HOUR*23-globs->mdtime)*255/MD_HOUR;
	else globs->dlight=0;

	tmp=globs->mdday%28+1;

	globs->newmoon=0;
	globs->fullmoon=0;

        if (tmp==1) { globs->newmoon=1; return; }
        if (tmp==15) globs->fullmoon=1;

	if (tmp>14) tmp=28-tmp;
	if (tmp>globs->dlight) globs->dlight=tmp;
}
