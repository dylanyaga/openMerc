/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include "server.h"

void look_rat_eye(int cn,int in)
{
	int n;

	do_char_log(cn,1,"%s\n",it[in].description);

	for (n=0; n<9; n++) {
		if (it[in].data[n]) {
			do_char_log(cn,1,"The slot for a %s is free.\n",it_temp[it[in].data[n]].name);
		}
	}

}

void look_spell_scroll(int cn,int in)
{
	int n;

	do_char_log(cn,1,"%s\n",it[in].description);

	n=it[in].data[2];

	do_char_log(cn,1,"There are %d charge%s left.\n",n,(n==1 ? "s" : ""));
}

void look_driver(int cn,int in)
{
	switch(it[in].driver) {
		case	17:	look_rat_eye(cn,in); break;
		case	48:	look_spell_scroll(cn,in); break;
		default:	xlog("Unknown look_driver %d",it[in].driver); break;
	}
}
