/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "funcs.h"

int sub_door_driver(int cn,int in)
{
        if (it[in].data[0]==65500) return 0;
        if (it[in].data[0]==65501 || it[in].data[0]==65502) { // star door in black stronghold
                int empty=0,star=0,circle=0,in2,n;
                static int loctab[4]={344487,343463,344488,343464};

                for (n=0; n<4; n++) {
                        in2=map[loctab[n]].it;
                        if (!in2) continue;
                        if (it[in2].data[1]!=n) continue;

                        if (it[in2].temp==761) star++;
                        if (it[in2].temp==762) circle++;
                        if (it[in2].temp==763) empty++;
                }

                if (it[in].data[0]==65501 && empty==3 && star==1) return 1;
                else if (it[in].data[0]==65502 && empty==3 && circle==1) return 1;
                else return 0;
        }

        return 0;
}

int use_door(int cn,int in)
{
        int in2,lock=0,n,skill,power,temp,flags;

        if (map[it[in].x+it[in].y*MAPX].ch) return 0;

        if (it[in].data[0]) {
                if (cn==0) {
                        lock=1;
                } else if (it[in].data[0]>=65500) {
                        lock=sub_door_driver(cn,in);
                } else if ((in2=ch[cn].citem)!=0 && !(in2&0x80000000) && it[in2].temp==it[in].data[0]) {
                        lock=1;
                        if (it[in].data[3]) {
                                ch[cn].citem=0;
                                it[in2].used=USE_EMPTY;
                                do_char_log(cn,1,"The key vanished.\n");
                        }
                } else {
                        for (n=0; n<40; n++)
                                if ((in2=ch[cn].item[n])!=0)
                                        if (it[in2].temp==it[in].data[0]) break;
                        if (n<40) {
                                lock=1;
                                if (it[in].data[3]) {
                                        ch[cn].item[n]=0;
                                        it[in2].used=USE_EMPTY;
                                        do_char_log(cn,1,"The key vanished.\n");
                                }
                        }
                }
                if (!lock && (in2=ch[cn].citem)!=0 && !(in2&0x80000000) && it[in2].driver==3) { // try to pick the lock
                        skill=ch[cn].skill[SK_LOCK][5]+it[in2].data[0];
                        power=it[in].data[2];
                        if (!power || skill<power+RANDOM(20)) {
                                do_char_log(cn,0,"You failed to pick the lock.\n");
                        } else lock=1;
                        item_damage_citem(cn,1);
                }
                if (it[in].data[1] && !lock) {
                        do_char_log(cn,0,"It's locked and you don't have the right key.\n");
                        return 0;
                }
        }
        reset_go(it[in].x,it[in].y);
        remove_lights(it[in].x,it[in].y);

        do_area_sound(0,0,it[in].x,it[in].y,10);

        if (!it[in].active) {
                it[in].flags&=~(IF_MOVEBLOCK|IF_SIGHTBLOCK);
                it[in].data[1]=0;
        } else {
                temp=it[in].temp;
                flags=it_temp[temp].flags&IF_SIGHTBLOCK;

                it[in].flags|=IF_MOVEBLOCK|flags;
                if (lock) it[in].data[1]=1;
        }
        reset_go(it[in].x,it[in].y);
        add_lights(it[in].x,it[in].y);
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        return 1;
}

int use_create_item(int cn,int in)
{
        int in2,n;

        if (cn==0) return 0;
        if (it[in].active) return 0;

        n=it[in].data[0];

        if (n<=0 || n>=MAXTITEM) return 0;

        in2=god_create_item(n);

        if (!god_give_char(in2,cn)) {
                do_char_log(cn,0,"Your backpack is full, so you can't take the %s.\n",it[in2].reference);
                it[in2].used=USE_EMPTY;
                return 0;
        }
        do_char_log(cn,1,"You got a %s.\n",it[in2].reference);
        chlog(cn,"Got %s from %s",it[in2].name,it[in].name);

        if (it[in].data[1] && it[in].driver==53) {
                char buf[300];

                do_char_log(cn,0,"You feel yourself form a magical connection with the %s.\n",it[in2].reference);
                it[in2].data[0]=cn;

                sprintf(buf,"%s Engraved in it are the letters \"%s\".",
                        it[in2].description,ch[cn].name);
                if (strlen(buf)<200) strcpy(it[in2].description,buf);
        }

        if (it[in].driver==54) {
                do_area_notify(cn,0,it[in].x,it[in].y,NT_HITME,cn,0,0,0);
        }

        return 1;
}

int use_create_gold(int cn,int in)
{
        int n;

        if (cn==0) return 0;
        if (it[in].active) return 0;

        n=it[in].data[0];

        do_char_log(cn,1,"You got a %dG.\n",n);
        chlog(cn,"Got %dG from %s",n,it[in].name);
        ch[cn].gold+=n*100;

        return 1;
}

int use_create_item2(int cn,int in)
{
        int in2,n,in3;

        if (cn==0) return 0;
        if (it[in].active) return 0;

        if ((in3=ch[cn].citem)==0 || (in3&0x80000000)) return 0;
        if (it[in3].temp!=it[in].data[1]) return 0;

        n=it[in].data[0];

        if (n<=0 || n>=MAXTITEM) return 0;

        in2=god_create_item(n);

        if (!god_give_char(in2,cn)) {
                do_char_log(cn,0,"Your backpack is full, so you can't take the %s.\n",it[in2].reference);
                it[in2].used=USE_EMPTY;
                return 0;
        }
        do_char_log(cn,1,"You got a %s.\n",it[in2].reference);
        chlog(cn,"Got %s from %s",it[in2].name,it[in].name);

        it[in3].used=USE_EMPTY;
        ch[cn].citem=0;

        return 1;
}

int use_create_item3(int cn,int in)
{
        int in2,n;

        if (cn==0) return 0;
        if (it[in].active) return 0;

        for (n=0; n<10; n++) if (!it[in].data[n]) break;
        if (n==0) return 0;

        n=RANDOM(n);
        n=it[in].data[n];

        if (n<=0 || n>=MAXTITEM) return 0;

        switch(n) {
                case 57:
                case 59:
                case 63:
                case 65:
                case 69:
                case 71:
                case 75:
                case 76:
                case 94:
                case 95:
                case 981:
                case 982:       in2=create_special_item(n); break;

                default:        in2=god_create_item(n); break;
        }

        if (!in2) {
                do_char_log(cn,1,"It's empty...\n");
                return 1;
        }


        if (!god_give_char(in2,cn)) {
                do_char_log(cn,0,"Your backpack is full, so you can't take anything.\n");
                it[in2].used=USE_EMPTY;
                return 0;
        }
        do_char_log(cn,1,"You got a %s.\n",it[in2].reference);
        chlog(cn,"Got %s from %s",it[in2].name,it[in].name);

        return 1;
}

int use_mix_potion(int cn,int in)
{
        int in2,in3=0;

        if (cn==0) return 0;
        if ((in2=ch[cn].citem)==0 || (in2&0x80000000)) {
                do_char_log(cn,0,"What do you want to do with it?\n");
                return 0;
        }

        if (it[in].carried==0) { do_char_log(cn,0,"Too difficult to do on the ground.\n"); return 0; }

        chlog(cn,"Trying to mix %s with %s",it[in].name,it[in2].name);

        if (it[in].temp==100) {         // empty OK
                switch(it[in2].temp) {
                        case 18:        in3=101; break;
                        case 46:        in3=102; break;
                        case 141:       in3=145; break;
                        case 140:       in3=144; break;
                        case 142:       in3=143; break;
                        case 197:       in3=219; break;
                        case 198:       in3=220; break;
                        case 199:       in3=218; break;
                        case 294:       in3=295; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==143) {  // green
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 140:       in3=147; break;
                        case 141:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=146; break;
                        case 198:       in3=146; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==144) {  // yellow
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 140:       in3=146; break;
                        case 141:       in3=146; break;
                        case 142:       in3=147; break;
                        case 197:       in3=146; break;
                        case 198:       in3=146; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==145) {  // blue
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 140:       in3=146; break;
                        case 141:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=146; break;
                        case 198:       in3=146; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==146) {  // black
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 140:       in3=146; break;
                        case 141:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=146; break;
                        case 198:       in3=146; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==147) {  // orange
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 141:       in3=148; break;
                        case 140:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=146; break;
                        case 198:       in3=146; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==218) {  // yellow green OK
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 141:       in3=146; break;
                        case 140:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=223; break;
                        case 198:       in3=221; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==219) {  // red green OK
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 141:       in3=146; break;
                        case 140:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=146; break;
                        case 198:       in3=222; break;
                        case 199:       in3=223; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==220) {  // blue green OK
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 141:       in3=146; break;
                        case 140:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=222; break;
                        case 198:       in3=146; break;
                        case 199:       in3=221; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==221) {  // blue yellow green
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 141:       in3=146; break;
                        case 140:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=224; break;
                        case 198:       in3=146; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==222) {  // blue red green
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 141:       in3=146; break;
                        case 140:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=146; break;
                        case 198:       in3=146; break;
                        case 199:       in3=224; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==223) {  // jungle I
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 141:       in3=146; break;
                        case 140:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=146; break;
                        case 198:       in3=224; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else if (it[in].temp==295) {  // yellow tulip
                switch(it[in2].temp) {
                        case 18:        in3=146; break;
                        case 46:        in3=146; break;
                        case 141:       in3=146; break;
                        case 140:       in3=146; break;
                        case 142:       in3=146; break;
                        case 197:       in3=146; break;
                        case 198:       in3=146; break;
                        case 199:       in3=146; break;
                        case 294:       in3=146; break;
                        default:        do_char_log(cn,0,"Sorry?\n"); return 0;
                }
        } else { do_char_log(cn,0,"Sorry?\n"); return 0; }

        in3=god_create_item(in3);
        it[in3].flags|=IF_UPDATE;

        it[in2].used=USE_EMPTY;
        ch[cn].citem=0;

        god_take_from_char(in,cn);
        it[in].used=USE_EMPTY;

        god_give_char(in3,cn);

        return 1;
}

int use_chain(int cn,int in)
{
        int in2,in3=0;

        if (cn==0) return 0;
        if ((in2=ch[cn].citem)==0 || (in2&0x80000000)) {
                do_char_log(cn,0,"What do you want to do with it?\n");
                return 0;
        }

        if (it[in].carried==0) { do_char_log(cn,0,"Too difficult to do on the ground.\n"); return 0; }
        if (it[in2].temp!=206) { do_char_log(cn,0,"Sorry?\n"); return 0; }

        if (it[in].temp>=it[in].data[0]) { do_char_log(cn,0,"It won't fit anymore.\n"); return 0; }

        chlog(cn,"Added %s to %s",it[in2].name,it[in].name);

        in3=it[in].temp+1;

        in3=god_create_item(in3);
        it[in3].flags|=IF_UPDATE;

        it[in2].used=USE_EMPTY;
        ch[cn].citem=0;

        god_take_from_char(in,cn);
        it[in].used=USE_EMPTY;

        god_give_char(in3,cn);

        return 1;
}

int stone_sword(int cn,int in)
{
        int n,in2;

        if (cn==0) return 0;
        if (it[in].active) return 0;

        n=it[in].data[0];

        if (n<=0 || n>=MAXTITEM) return 0;

        if (ch[cn].attrib[AT_STREN][5]<100) {
                do_char_log(cn,0,"You're not strong enough.\n");
                return 0;
        }

        in2=god_create_item(n);

        chlog(cn,"Got %s from %s",it[in2].name,it[in].name);

        god_give_char(in2,cn);
        do_char_log(cn,1,"You got a %s.\n",it[in2].reference);

        return 1;
}

void finish_laby_teleport(int cn,int nr,int exp)
{
        int n,in2;

        if (ch[cn].data[20]<nr) {
                ch[cn].data[20]=nr;
                do_char_log(cn,0,"You have solved the %d%s%s%s%s part of the Labyrinth.\n",nr,
                        nr==1 ? "st" : "",
                        nr==2 ? "nd" : "",
                        nr==3 ? "rd" : "",
                        nr>=4 ? "th" : "");

                do_give_exp(cn,exp,0,-1);
                chlog(cn,"Solved Labyrinth Part %d",nr);
        }
        if ((in2=ch[cn].citem) && !(in2&0x80000000) && (it[in2].flags&IF_LABYDESTROY)) {
                ch[cn].citem=0; it[in2].used=USE_EMPTY;
                do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
        }
        for (n=0; n<40; n++) {
                if ((in2=ch[cn].item[n]) && (it[in2].flags&IF_LABYDESTROY)) {
                        ch[cn].item[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }
        for (n=0; n<20; n++) {
                if ((in2=ch[cn].worn[n]) && (it[in2].flags&IF_LABYDESTROY)) {
                        ch[cn].worn[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }

	for (n=0; n<20; n++) {
                if ((in2=ch[cn].spell[n])) {
                        ch[cn].spell[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].name);
                }
        }

        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        god_transfer_char(cn,512,512);
        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);

        ch[cn].temple_x=ch[cn].tavern_x=ch[cn].x;
        ch[cn].temple_y=ch[cn].tavern_y=ch[cn].y;
}


int is_nolab_item(int in)
{
	if (!IS_SANEITEM(in)) return 0;
	
	switch(it[in].temp) {
		
		case 331:	// tavern scroll
		case 500:	// lag scroll
		case 592:	// gorn scroll	
		case 903:	// forest scroll
		case 1114:	// staffers corner scroll
		case 1118:	// inn scroll
		case 1144:	// arena scroll
				return 1;
		default:	return 0;
	}
}

int teleport(int cn,int in)
{
        int n,in2;

        if (!cn) return 1;
        if (it[in].flags&IF_USEACTIVATE && !(it[in].active)) return 1;

        if ((in2=ch[cn].citem) && is_nolab_item(in2)) {
                ch[cn].citem=0; it[in2].used=USE_EMPTY;
                do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
        }

        for (n=0; n<40; n++) {
                if ((in2=ch[cn].item[n]) && is_nolab_item(in2)) {
                        ch[cn].item[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }

        for (n=0; n<20; n++) {
                if ((in2=ch[cn].spell[n]) && (it[in2].temp==SK_RECALL)) {
                        ch[cn].spell[n]=0; it[in2].used=USE_EMPTY;
                }
        }

        if (it[in].data[2]) {   // lab-solved teleport
                return use_labtransfer(cn,it[in].data[2],it[in].data[3]);
        } else {
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                god_transfer_char(cn,it[in].data[0],it[in].data[1]);
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        return 1;
}

int teleport2(int cn,int in2)
{
        int in;

        if (!cn) return 1;

	chlog(cn,"Used teleport scroll to %d,%d (%s)",
		it[in2].data[0],
		it[in2].data[1],
		get_area_m(it[in2].data[0],it[in2].data[1],0));
					
	if (it[in2].data[2] && it[in2].data[2]+TICKS*60*4<globs->ticker) {
		chlog(cn,"Lag Scroll Time Difference: %d ticks (%.2fs)",
			globs->ticker-it[in2].data[2],
			(globs->ticker-it[in2].data[2])/(double)TICKS);
		
		do_char_log(cn,0,"Sorry, this lag scroll was too old. You need to use it four minutes after lagging out or earlier!\n");
		
		return 1;
	}

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in teleport2"); return 0; }

        strcpy(it[in].name,"Teleport");
        it[in].flags|=IF_SPELL;
        it[in].sprite[1]=90;
        it[in].duration=it[in].active=180;
        it[in].temp=SK_RECALL;
        it[in].power=it[in2].power;
        it[in].data[0]=it[in2].data[0];
        it[in].data[1]=it[in2].data[1];

        if (!add_spell(cn,in)) {
                do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                return 0;
        }
        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        return 1;
}

int use_labyrinth(int cn,int in)
{
        int n,in2,flag=0;

        if ((in2=ch[cn].citem) && is_nolab_item(in2)) {
                ch[cn].citem=0; it[in2].used=USE_EMPTY;
                do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
        }

        for (n=0; n<40; n++) {
                if ((in2=ch[cn].item[n]) && is_nolab_item(in2)) {
                        ch[cn].item[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }

        for (n=0; n<20; n++) {
                if ((in2=ch[cn].spell[n]) && (it[in2].temp==SK_RECALL)) {
                        ch[cn].spell[n]=0; it[in2].used=USE_EMPTY;
                }
        }

        switch(ch[cn].data[20]) {
                case    0:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,64,56);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    1:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,95,207);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    2:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,74,240);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    3:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,37,370);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    4:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,114,390);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    5:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,28,493);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    6:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,24,534);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    7:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,118,667);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    8:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,63,720);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                case    9:      fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                flag=god_transfer_char(cn,33,597);
                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                                break;
                default:        do_char_log(cn,0,"You have already solved all existing parts of the labyrinth. Please come back later.\n");
                                break;
        }

        if (flag) {
                ch[cn].temple_x=ch[cn].tavern_x=ch[cn].x;
                ch[cn].temple_y=ch[cn].tavern_y=ch[cn].y;
        }

        return 1;
}

int use_ladder(int cn,int in)
{
        god_transfer_char(cn,it[in].x+it[in].data[0],it[in].y+it[in].data[1]);

        return 1;
}

int use_bag(int cn,int in)
{
        int co, owner;

        co=it[in].data[0];
        owner=ch[co].data[CHD_CORPSEOWNER];

        // prevent graverobbing by the unauthorized
        if (owner && owner!=cn && !may_attack_msg(cn,owner,0) && ch[owner].data[CHD_ALLOW]!=cn) {
                do_char_log(cn, 0, "This is %s's grave, not yours. "
                                   "You may only search it with %s permission.\n",
                            ch[owner].name, HIS_HER(owner));
                if (IS_ACTIVECHAR(co) && ch[owner].x) {
                        do_char_log(owner, 0, "%s just tried to search your grave. "
                                           "You must #ALLOW %s if you want %s to.\n",
                                    ch[cn].name, ch[cn].name, HIM_HER(cn));
                }
                return 0;
        }

        do_char_log(cn,1,"You search the remains of %s.\n",ch[co].reference);
        do_look_char(cn,co,0,0,1);
        return 1;
}

int use_scroll(int cn,int in)
{
        int nr,pts,v;

        nr=it[in].data[0];

        if (ch[cn].skill[nr][0]) {
                if (it[in].data[1]) {
                        do_char_log(cn,0,"You already know %s.\n",skilltab[nr].name);
                        return 0;
                }
                v=ch[cn].skill[nr][0];
                if (v>=ch[cn].skill[nr][2]) {
                        do_char_log(cn,0,"You cannot raise %s any higher.\n",skilltab[nr].name);
                        return 0;
                }
                do_char_log(cn,1,"Raised %s by one.\n",skilltab[nr].name);
                chlog(cn,"Used scroll to raise %s by one",skilltab[nr].name);
                pts=skill_needed(v,ch[cn].skill[nr][3]);
                ch[cn].points_tot+=pts;
                ch[cn].skill[nr][0]++;

                do_check_new_level(cn);
        } else if (!ch[cn].skill[nr][2]) {
                do_char_log(cn,0,"This scroll teaches %s, which you cannot learn.\n",skilltab[nr].name);
                return 0;
        } else {
                ch[cn].skill[nr][0]=1;
                do_char_log(cn,1,"You learned %s!\n",skilltab[nr].name);
                chlog(cn,"Used scroll to learn %s",skilltab[nr].name);
        }

        it[in].used=USE_EMPTY;
        god_take_from_char(in,cn);

        do_update_char(cn);

        return 1;
}

int use_scroll2(int cn,int in)
{
        int nr,pts,v;

        nr=it[in].data[0];

        v=ch[cn].attrib[nr][0];
        if (v>=ch[cn].attrib[nr][2]) {
                do_char_log(cn,0,"You cannot raise %s any higher.\n",at_name[nr]);
                return 0;
        }
        do_char_log(cn,1,"Raised %s by one.\n",at_name[nr]);
        chlog(cn,"Used scroll to raise %s by one",at_name[nr]);

        pts=attrib_needed(v,ch[cn].attrib[nr][3]);
        ch[cn].points_tot+=pts;
        ch[cn].attrib[nr][0]++;

        do_check_new_level(cn);

        it[in].used=USE_EMPTY;
        god_take_from_char(in,cn);

        do_update_char(cn);

        return 1;
}

int use_scroll3(int cn,int in)
{
        int am,pts=0,v,n;

        am=it[in].data[0];

        v=ch[cn].hp[0];
        if (v>=ch[cn].hp[2]) {
                do_char_log(cn,0,"You cannot raise Hitpoints any higher.\n");
                return 0;
        }
        do_char_log(cn,1,"Raised Hitpoints by %d.\n",am);
        chlog(cn,"Used scroll to raise Hitpoints by %d",am);

        for (n=0; n<am; n++)
                pts+=hp_needed(n+v,ch[cn].hp[3]);
        ch[cn].points_tot+=pts;
        ch[cn].hp[0]+=am;

        do_check_new_level(cn);

        it[in].used=USE_EMPTY;
        god_take_from_char(in,cn);

        do_update_char(cn);

        return 1;
}

int use_scroll4(int cn,int in)
{
        int am,pts=0,v,n;

        am=it[in].data[0];

        v=ch[cn].end[0];
        if (v>=ch[cn].end[2]) {
                do_char_log(cn,0,"You cannot raise Endurance any higher.\n");
                return 0;
        }
        do_char_log(cn,1,"Raised Endurance by %d.\n",am);
        chlog(cn,"Used scroll to raise Endurance by %d",am);

        for (n=0; n<am; n++)
                pts+=end_needed(n+v,ch[cn].end[3]);
        ch[cn].points_tot+=pts;
        ch[cn].end[0]+=am;

        do_check_new_level(cn);

        it[in].used=USE_EMPTY;
        god_take_from_char(in,cn);

        do_update_char(cn);

        return 1;
}

int use_scroll5(int cn,int in)
{
        int am,pts=0,v,n;

        am=it[in].data[0];

        v=ch[cn].mana[0];
        if (v>=ch[cn].mana[2]) {
                do_char_log(cn,0,"You cannot raise Mana any higher.\n");
                return 0;
        }
        do_char_log(cn,1,"Raised Mana by %d.\n",am);
        chlog(cn,"Used scroll to raise Mana by %d",am);

        for (n=0; n<am; n++)
                pts+=mana_needed(n+v,ch[cn].mana[3]);
        ch[cn].points_tot+=pts;
        ch[cn].mana[0]+=am;

        do_check_new_level(cn);

        it[in].used=USE_EMPTY;
        god_take_from_char(in,cn);

        do_update_char(cn);

        return 1;
}

int use_crystal_sub(int cn,int in)
{
        int group,n,cnt,cc,base,pts=0,m,z,tmp,sbase,miss=0;
        static int temps[6]={2,4,76,78,150,151};        // templar?
        int baseg[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

        group=it[in].data[0];

        for (n=1,cnt=0; n<MAXCHARS; n++) {
                if (ch[n].used==USE_ACTIVE && !(ch[n].flags&CF_BODY) && ch[n].data[42]==group) {
                        base=ch[n].data[0];
                        if (base>99) base=99;
                        baseg[base/5]++;
                        cnt++;
                }
        }

        miss=it[in].data[1]-cnt;
        if (miss<=0) return 0;

        for (n=0,sbase=0,tmp=999; n<20; n++) {
                if (baseg[n]<tmp) {
                        tmp=baseg[n]; sbase=n;
                }
        }

        xlog("Randoms: smallest base is %d with %d (miss=%d)",sbase,tmp,miss);

        tmp=temps[RANDOM(6)];

        cc=god_create_char(tmp,0);
        if (!god_drop_char_fuzzy(cc,it[in].x,it[in].y)) {
                xlog("use_crystal (%d,%d): drop_char failed",it[in].x,it[in].y);
                ch[cc].used=USE_EMPTY;
                return 0;
        }

        ch[cc].data[42]=group;


        do {
                m=RANDOM(64)+128+(RANDOM(64)+64)*MAPX;
        } while (!plr_check_target(m));

        ch[cc].goto_x=m%MAPX;
        ch[cc].goto_y=m/MAPX;
        ch[cc].data[60]=18*20;
        ch[cc].data[62]=1;

        strcpy(ch[cc].text[0],"Yes! Die, %s!");
        strcpy(ch[cc].text[1],"Yahoo! An enemy! Prepare to die, %s!");
        strcpy(ch[cc].text[3],"Thank you %s! Everything is better than being here.");
        ch[cc].data[48]=33;

        base=sbase*5+RANDOM(5);
        ch[cc].data[0]=base;

        for (n=0; n<5; n++) {
                tmp=base+RANDOM(15);
                tmp=tmp*3/max(1,ch[cc].attrib[n][3]);
                ch[cc].attrib[n][0]=max(10,min(ch[cc].attrib[n][2],tmp));
        }

        for (n=0; n<50; n++) {
                tmp=base+RANDOM(15);
                tmp=tmp*3/max(1,ch[cc].skill[n][3]);
                if (ch[cc].skill[n][2]) ch[cc].skill[n][0]=min(ch[cc].skill[n][2],tmp);
        }

        ch[cc].hp[0]=max(50,min(ch[cc].hp[2],base*5+RANDOM(50)));
        ch[cc].end[0]=max(50,min(ch[cc].end[2],base*5+RANDOM(50)));
        ch[cc].mana[0]=max(50,min(ch[cc].mana[2],base*5+RANDOM(50)));

        // calculate experience
        for (z=0; z<5; z++)
                for (m=10; m<ch[cc].attrib[z][0]; m++)
                        pts+=attrib_needed(m,3);

        for (m=50; m<ch[cc].hp[0]; m++)
                pts+=hp_needed(m,3);

        for (m=50; m<ch[cc].end[0]; m++)
                pts+=end_needed(m,2);

        for (m=50; m<ch[cc].mana[0]; m++)
                pts+=mana_needed(m,3);

        for (z=0; z<50; z++)
                for (m=1; m<ch[cc].skill[z][0]; m++)
                        pts+=skill_needed(m,2);

        ch[cc].points_tot=pts;
        ch[cc].gold=base*base+1;
        ch[cc].a_hp=999999;
        ch[cc].a_end=999999;
        if (ch[cc].skill[SK_MEDIT][0]>0) ch[cc].a_mana=1000000;
        else ch[cc].a_mana=RANDOM(4)*RANDOM(4)*RANDOM(4)*RANDOM(4)*RANDOM(4)*RANDOM(4)*100;

        ch[cc].alignment=-RANDOM(7500);

        xlog("Created %s (%d) with base %d in Random Dungeon",ch[cc].name,cc,base);

        if (ch[cc].attrib[AT_AGIL][0]>=90 && ch[cc].attrib[AT_STREN][0]>=90) { // titanium
                tmp=ch[cc].worn[WN_HEAD]=pop_create_item(94,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_BODY]=pop_create_item(95,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_ARMS]=pop_create_item(98,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_LEGS]=pop_create_item(97,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_FEET]=pop_create_item(99,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_CLOAK]=pop_create_item(96,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].attrib[AT_AGIL][0]>=72 && ch[cc].attrib[AT_STREN][0]>=72) { //crystal
                tmp=ch[cc].worn[WN_HEAD]=pop_create_item(75,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_BODY]=pop_create_item(76,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_ARMS]=pop_create_item(79,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_LEGS]=pop_create_item(78,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_FEET]=pop_create_item(80,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_CLOAK]=pop_create_item(77,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].attrib[AT_AGIL][0]>=40 && ch[cc].attrib[AT_STREN][0]>=40) { //gold
                tmp=ch[cc].worn[WN_HEAD]=pop_create_item(69,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_BODY]=pop_create_item(71,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_ARMS]=pop_create_item(73,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_LEGS]=pop_create_item(72,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_FEET]=pop_create_item(74,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_CLOAK]=pop_create_item(70,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].attrib[AT_AGIL][0]>=24 && ch[cc].attrib[AT_STREN][0]>=24) { //steel
                tmp=ch[cc].worn[WN_HEAD]=pop_create_item(63,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_BODY]=pop_create_item(65,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_ARMS]=pop_create_item(67,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_LEGS]=pop_create_item(66,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_FEET]=pop_create_item(68,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_CLOAK]=pop_create_item(64,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].attrib[AT_AGIL][0]>=16 && ch[cc].attrib[AT_STREN][0]>=16) { //bronze
                tmp=ch[cc].worn[WN_HEAD]=pop_create_item(57,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_BODY]=pop_create_item(59,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_ARMS]=pop_create_item(61,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_LEGS]=pop_create_item(60,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_FEET]=pop_create_item(62,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_CLOAK]=pop_create_item(58,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].attrib[AT_AGIL][0]>=12 && ch[cc].attrib[AT_STREN][0]>=12) { //leather
                tmp=ch[cc].worn[WN_HEAD]=pop_create_item(51,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_BODY]=pop_create_item(53,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_ARMS]=pop_create_item(55,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_LEGS]=pop_create_item(54,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_FEET]=pop_create_item(56,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_CLOAK]=pop_create_item(52,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].attrib[AT_AGIL][0]>=10 && ch[cc].attrib[AT_STREN][0]>=10) { //cloth
                tmp=ch[cc].worn[WN_HEAD]=pop_create_item(39,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_BODY]=pop_create_item(42,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_ARMS]=pop_create_item(44,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_LEGS]=pop_create_item(43,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_FEET]=pop_create_item(41,cc);
                it[tmp].carried=cc;
                tmp=ch[cc].worn[WN_CLOAK]=pop_create_item(40,cc);
                it[tmp].carried=cc;
        }

        if (ch[cc].skill[SK_TWOHAND][0]>=60 && ch[cc].attrib[AT_AGIL][0]>=50 && ch[cc].attrib[AT_STREN][0]>=75) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(125,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_TWOHAND][0]>=45 && ch[cc].attrib[AT_AGIL][0]>=40 && ch[cc].attrib[AT_STREN][0]>=60) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(38,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_TWOHAND][0]>=30 && ch[cc].attrib[AT_AGIL][0]>=30 && ch[cc].attrib[AT_STREN][0]>=40) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(37,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_TWOHAND][0]>=15 && ch[cc].attrib[AT_AGIL][0]>=20 && ch[cc].attrib[AT_STREN][0]>=24) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(36,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_TWOHAND][0]>=1 && ch[cc].attrib[AT_AGIL][0]>=10 && ch[cc].attrib[AT_STREN][0]>=12) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(35,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_SWORD][0]>=60 && ch[cc].attrib[AT_AGIL][0]>=32 && ch[cc].attrib[AT_STREN][0]>=56) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(524,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_SWORD][0]>=45 && ch[cc].attrib[AT_AGIL][0]>=24 && ch[cc].attrib[AT_STREN][0]>=48) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(34,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_SWORD][0]>=30 && ch[cc].attrib[AT_AGIL][0]>=16 && ch[cc].attrib[AT_STREN][0]>=32) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(33,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_SWORD][0]>=15 && ch[cc].attrib[AT_AGIL][0]>=12 && ch[cc].attrib[AT_STREN][0]>=16) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(32,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_SWORD][0]>=1 && ch[cc].attrib[AT_AGIL][0]>=8 && ch[cc].attrib[AT_STREN][0]>=8) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(31,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_DAGGER][0]>=60 && ch[cc].attrib[AT_AGIL][0]>=30 && ch[cc].attrib[AT_STREN][0]>=18) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(523,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_DAGGER][0]>=45 && ch[cc].attrib[AT_AGIL][0]>=24 && ch[cc].attrib[AT_STREN][0]>=14) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(30,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_DAGGER][0]>=30 && ch[cc].attrib[AT_AGIL][0]>=16 && ch[cc].attrib[AT_STREN][0]>=10) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(29,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_DAGGER][0]>=15 && ch[cc].attrib[AT_AGIL][0]>=12 && ch[cc].attrib[AT_STREN][0]>=8) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(28,cc);
                it[tmp].carried=cc;
        } else if (ch[cc].skill[SK_DAGGER][0]>=1 && ch[cc].attrib[AT_AGIL][0]>=1 && ch[cc].attrib[AT_STREN][0]>=1) {
                tmp=ch[cc].worn[WN_RHAND]=pop_create_item(27,cc);
                it[tmp].carried=cc;
        }

        if (RANDOM(30)==0 && base>5) {
                god_give_char(tmp=god_create_item(RANDOM(2)+273),cc);
                xlog("  got %s",it[tmp].name);
        }

        if (RANDOM(60)==0 && base>15) {
                god_give_char(tmp=god_create_item(RANDOM(2)+192),cc);
                xlog("  got %s",it[tmp].name);
        }

        if (RANDOM(150)==0 && base>20) {
                god_give_char(tmp=god_create_item(RANDOM(9)+181),cc);
                xlog("  got %s",it[tmp].name);
        }

        do_update_char(cc);

        return miss;
}

int use_crystal(int cn,int in)
{
        int cnt=0;

        while (use_crystal_sub(cn,in)>4 && cnt<4) cnt++;

        if (!cnt) return 1; else return 0;
}

int use_mine_respawn(int cn,int in)
{
        int group,n,cnt,cc,m,tmp,in2,maxcnt;

        group=it[in].data[0];
        tmp=it[in].data[1];
        maxcnt=it[in].data[2];

        for (n=3; n<10; n++) {
                if ((m=it[in].data[n])==0) break;
                if ((in2=map[m].it)==0) return 0;
                if (it[in2].driver!=26) return 0;
        }

        for (n=1,cnt=0; n<MAXCHARS; n++) {
                if (ch[n].used==USE_ACTIVE && !(ch[n].flags&CF_BODY) && ch[n].data[42]==group) {
                        cnt++;
                }
        }

        if (cnt>maxcnt) return 0;

        cc=pop_create_char(tmp,0);
        if (!god_drop_char_fuzzy(cc,it[in].x,it[in].y)) {
                xlog("mine_respawn (%d,%d): drop_char failed",it[in].x,it[in].y);
                ch[cc].used=USE_EMPTY;
                return 0;
        }

        do_update_char(cc);

        return 1;
}

int rat_eye(int cn,int in)
{
        int in2,n;

        if (cn==0) return 0;

        if ((in2=ch[cn].citem)==0 || (in2&0x80000000)) {
                do_char_log(cn,0,"What do you want to do with it?\n");
                return 0;
        }

        if (it[in].carried==0) { do_char_log(cn,0,"Too difficult to do on the ground.\n"); return 0; }

        for (n=0; n<9; n++)
                if (it[in].data[n] && it[in].data[n]==it[in2].temp) break;
        if (n==9) { do_char_log(cn,0,"This doesnt fit.\n"); return 0; }

        chlog(cn,"Added %s to %s",it[in2].name,it[in].name);

        it[in].data[n]=0;
        it[in].sprite[0]++;
        it[in].flags|=IF_UPDATE;
        it[in].temp=0;

        it[in2].used=USE_EMPTY;
        ch[cn].citem=0;

        for (n=0; n<9; n++)
                if (it[in].data[n]) break;
        if (n==9) {
                int in3;

                in3=it[in].data[9];

                in3=god_create_item(in3);
                it[in3].flags|=IF_UPDATE;

                god_take_from_char(in,cn);
                it[in].used=USE_EMPTY;

                god_give_char(in3,cn);
        }

        return 1;
}

int skua_protect(int cn,int in)
{
        if (ch[cn].worn[WN_RHAND]!=in) {
                do_char_log(cn,0,"You cannot use Skua's weapon if you're not wielding it.\n");
                return 0;
        }

        if (ch[cn].kindred&KIN_PURPLE) {
                do_char_log(cn,0,"How dare you to call on Skua to help you? Slave of the Purple One!\n");
                do_char_log(cn,0,"Your weapon vanished.\n");
                ch[cn].worn[WN_RHAND]=0;
                it[in].used=USE_EMPTY;
        } else {
                do_char_log(cn,0,"You feel Skua's presence protect you.\n");
                do_char_log(cn,0,"He takes away His weapon and replaces it by a common one.\n");
                spell_from_item(cn,in);
                it[in].used=USE_EMPTY;
                in=god_create_item(it[in].data[2]);
                it[in].carried=cn;
                ch[cn].worn[WN_RHAND]=in;
                it[in].flags|=IF_UPDATE;
        }

        return 1;
}

int purple_protect(int cn,int in)
{
        if (ch[cn].worn[WN_RHAND]!=in) {
                do_char_log(cn,0,"You cannot use the Purple One's weapon if you're not wielding it.\n");
                return 0;
        }

        if (!(ch[cn].kindred&KIN_PURPLE)) {
                do_char_log(cn,0,"How dare you to call on the Purple One to help you? Slave of Skua!\n");
                do_char_log(cn,0,"Your weapon vanished.\n");
                ch[cn].worn[WN_RHAND]=0;
                it[in].used=USE_EMPTY;
        } else {
                do_char_log(cn,0,"You feel the Purple One's presence protect you.\n");
                do_char_log(cn,0,"He takes away His weapon and replaces it by a common one.\n");
                spell_from_item(cn,in);
                it[in].used=USE_EMPTY;
                in=god_create_item(it[in].data[2]);
                it[in].carried=cn;
                ch[cn].worn[WN_RHAND]=in;
                it[in].flags|=IF_UPDATE;
        }

        return 1;
}

int use_lever(int cn,int in)
{
        int in2,m;

        m=it[in].data[0];
        in2=map[m].it;

        if (!in2) return 0;
        if (it[in2].active) return 0;

        use_driver(0,in2,0);
        it[in2].active=it[in2].duration;
        if (it[in2].light[0]!=it[in2].light[1])
                do_add_light(it[in2].x,it[in2].y,it[in2].light[1]-it[in2].light[0]);

        return 1;
}

int use_spawn(int cn,int in)
{
        int in2,temp;

        if (it[in].active) return 0;

        if (cn && it[in].data[1]) {
                if (!(in2=ch[cn].citem) || (in2&0x80000000)) return 0;
                if (it[in2].temp!=it[in].data[1]) return 0;
                it[in2].used=USE_EMPTY;
                ch[cn].citem=0;
        }

        if ((temp=it[in].data[2])!=0)
                fx_add_effect(2,TICKS*10,ch_temp[temp].x,ch_temp[temp].y,temp);

        return 1;
}

int use_pile(int cn,int in)
{
        int in2,x,y,m,level,tmp,chance;
        static int find[]={
                361,361,339,342,345,339,342,345,359,359,        // silver, small jewels, skeleton
                361,361,339,342,345,339,342,345,359,359,        // silver, small jewels, skeleton
                361,361,339,342,345,339,342,345,359,359,        // silver, small jewels, skeleton

                361,361,361,340,343,346,371,371,371,371,        // silver, med jewels, golem
                361,361,361,340,343,346,371,371,371,371,        // silver, med jewels, golem
                361,361,361,340,343,346,371,371,371,371,        // silver, med jewels, golem

                360,341,344,347,372,372,372,487,372,372,        // gold, big jewels, gargoyle
                360,341,344,347,372,372,372,488,372,372,        // gold, big jewels, gargoyle
                360,341,344,347,372,372,372,489,372,372};       // gold, big jewels, gargoyle

        if (it[in].active) return 0;

        // destroy this object
        it[in].used=USE_EMPTY;
        x=it[in].x;
        y=it[in].y;
        m=x+y*MAPX;
        level=it[in].data[0];
        map[m].it=0;

        // decide what the player's gonna find
        chance=6;
        if (ch[cn].luck<0) chance++;
        if (ch[cn].luck<=-100) chance++;
        if (ch[cn].luck<=-500) chance++;
        if (ch[cn].luck<=-1000) chance++;
        if (ch[cn].luck<=-2000) chance++;
        if (ch[cn].luck<=-3000) chance++;
        if (ch[cn].luck<=-4000) chance++;
        if (ch[cn].luck<=-6000) chance++;
        if (ch[cn].luck<=-8000) chance++;
        if (ch[cn].luck<=-10000) chance++;

        if (!RANDOM(chance)) { // something there?
                tmp=RANDOM(30)+level*30;
                tmp=find[tmp];

                // create it and give it to player (unless it's the monster, then have it attack)
                in2=god_create_item(tmp);
                if (it[in2].flags&IF_TAKE) {    // takeable object?
                        god_give_char(in2,cn);
                        do_char_log(cn,0,"You've found a %s!\n",it[in2].reference);
                } else { // no? then it's an object which creates a monster
                        god_drop_item(in2,x,y);
                        fx_add_effect(9,16,in2,it[in2].data[0],0);
                }
        }
        return 1;
}

int use_grave(int cn,int in)
{
        int cc;

	// get previously spawned character
	cc=it[in].data[0];
	// still alive? then don't spawn new one
	if (ch[cc].data[0]==in && !(ch[cc].flags&CF_BODY) && ch[cc].used) return 1;	

        cc=pop_create_char(328,0);
        if (!god_drop_char_fuzzy(cc,it[in].x,it[in].y)) {
		god_destroy_items(cc);
		ch[cc].used=USE_EMPTY;
		return 1;
	}

	// create link between item and character
	ch[cc].data[0]=in;
	it[in].data[0]=cc;	

        return 1;
}

int mine_wall(int in,int x,int y)
{
        int temp,carried;

        if (!in) in=map[x+y*MAPX].it;
        if (!in) return 0;

        if (it[in].data[3]) // add rebuild wall effect
                fx_add_effect(10,TICKS*60*15,it[in].x,it[in].y,it[in].temp);

        temp=it[in].data[0];
        x=it[in].x;
        y=it[in].y;
        carried=it[in].carried;

        it[in]=it_temp[temp];
        it[in].x=x;
        it[in].y=y;
        it[in].carried=carried;
        it[in].temp=temp;
        if (carried) it[in].flags|=IF_UPDATE;

        return it[in].data[2];
}

int mine_state(int x,int y)
{
        int in;

        in=map[x+y*MAPX].it;
        if (!in) return 0;
        if (it[in].driver!=25) return 0;

        return it[in].data[2];
}

int use_mine(int cn,int in)
{
        int tmp,in2,str;

        str=ch[cn].attrib[AT_STREN][5];

        // substract endurance
        if (ch[cn].a_end<1500) {
                do_char_log(cn,0,"You're too exhausted to continue digging.\n");
                ch[cn].misc_action=DR_IDLE;
                return 0;
        }

        ch[cn].a_end-=1000;

        // check for proper tools!
        if ((in2=ch[cn].worn[WN_RHAND])!=0) {
                if (it[in2].temp==458) {
                        item_damage_weapon(cn,str/10);
                        str*=2;
                } else {
                        item_damage_weapon(cn,str*10);
                        str/=4;
                }
                char_play_sound(cn,11,-150,0);
                do_area_sound(cn,0,ch[cn].x,ch[cn].y,11);
        } else {
                str/=10;
                if (ch[cn].a_hp<10000) {
                        do_char_log(cn,0,"You don't want to kill yourself beating at this wall with your bare hands, so you stop.\n");
                        ch[cn].misc_action=DR_IDLE;
                        return 0;
                }
                ch[cn].a_hp-=500;
        }

        tmp=it[in].data[1]-str;
        if (tmp<=0) {
                reset_go(it[in].x,it[in].y);
                remove_lights(it[in].x,it[in].y);

                tmp=mine_wall(in,0,0)+1;

                reset_go(it[in].x,it[in].y);
                add_lights(it[in].x,it[in].y);
        } else it[in].data[1]=tmp;

        return 0;
}

int use_mine_fast(int cn,int in)
{
        if (!cn) return 0;
        if (it[in].carried) return 0;

        fx_add_effect(10,TICKS*60*15,it[in].x,it[in].y,it[in].temp);

        reset_go(it[in].x,it[in].y);
        remove_lights(it[in].x,it[in].y);

        map[it[in].x+it[in].y*MAPX].it=0;
        it[in].used=USE_EMPTY;

        reset_go(it[in].x,it[in].y);
        add_lights(it[in].x,it[in].y);

        return 1;
}

int build_ring(int cn,int in)
{
        int t1,t2,in2,r,in3;

        t1=it[in].temp;

        in2=ch[cn].citem;
        if (!in2) t2=0;
        else t2=it[in2].temp;

        if (t1==360 && t2==0) r=337;
        else if (t1==361 && t2==0) r=338;
        else if (t1==337) { // plain golden ring
                switch(t2) {
                        case    339:    r=362; break; // small ruby
                        case    340:    r=363; break; // med ruby
                        case    341:    r=364; break; // big ruby
                        case    342:    r=365; break; // small emerald
                        case    343:    r=366; break; // med emerald
                        case    344:    r=367; break; // big emerald
                        case    345:    r=368; break; // small saphire
                        case    346:    r=369; break; // med saphire
                        case    347:    r=370; break; // big saphire
                        case    487:    r=490; break; // huge ruby
                        case    488:    r=491; break; // huge emerald
                        case    489:    r=492; break; // huge saphire
                        default:        return 0;
                }
        } else if (t1==338) { // plain silver ring
                switch(t2) {
                        case    339:    r=348; break; // small ruby
                        case    340:    r=349; break; // med ruby
                        case    341:    r=350; break; // big ruby
                        case    342:    r=351; break; // small emerald
                        case    343:    r=352; break; // med emerald
                        case    344:    r=353; break; // big emerald
                        case    345:    r=354; break; // small saphire
                        case    346:    r=355; break; // med saphire
                        case    347:    r=356; break; // big saphire
                        case    487:
                        case    488:
                        case    499:    do_char_log(cn,0,"This stone is too powerful for a silver ring.\n");
                        default:        return 0;
                }
        } else return 0;

        in3=god_create_item(r);
        it[in3].flags|=IF_UPDATE;

        if (in2) {
                ch[cn].citem=0;
                it[in2].used=USE_EMPTY;
        }

        god_take_from_char(in,cn);
        it[in].used=USE_EMPTY;

        god_give_char(in3,cn);

        return 1;
}

int build_amulet(int cn,int in)
{
        int t1,t2,in2,r,in3;

        t1=it[in].temp;

        in2=ch[cn].citem;
        if (!in2 || (in2&0x80000000)) {
                do_char_log(cn,1,"Nothing happens.\n");
                return 0;
        }

        t2=it[in2].temp;

        if (t1==471 && t2==472) r=476;
        else if (t1==472 && t2==471) r=476;
        else if (t1==471 && t2==473) r=474;
        else if (t1==473 && t2==471) r=474;
        else if (t1==472 && t2==473) r=475;
        else if (t1==473 && t2==472) r=475;
        else if (t1==471 && t2==475) r=466;
        else if (t1==475 && t2==471) r=466;
        else if (t1==472 && t2==474) r=466;
        else if (t1==474 && t2==472) r=466;
        else if (t1==473 && t2==476) r=466;
        else if (t1==476 && t2==473) r=466;
        else {
                do_char_log(cn,1,"That doesn't fit.\n");
                return 0;
        }

        in3=god_create_item(r);
        it[in3].flags|=IF_UPDATE;

        if (in2) {
                ch[cn].citem=0;
                it[in2].used=USE_EMPTY;
        }

        god_take_from_char(in,cn);
        it[in].used=USE_EMPTY;

        god_give_char(in3,cn);

        return 1;
}

int use_gargoyle(int cn,int in)
{
        int cc;

        if (!cn) return 0;
        if (!it[in].carried) return 0;

        cc=god_create_char(325,1);
        if (!god_drop_char_fuzzy(cc,ch[cn].x,ch[cn].y)) {
                ch[cc].used=USE_EMPTY;
                do_char_log(cn,0,"The Gargoyle could not materialize.\n");
                return 0;
        }

        god_take_from_char(in,cn);
        it[in].used=USE_EMPTY;

        ch[cc].data[42]=65536+cn;                       // set group
        ch[cc].data[59]=65536+cn;                       // protect all other members of this group

        ch[cc].data[63]=cn;                             // obey and protect char
        ch[cc].data[69]=cn;                             // follow char
        ch[cc].data[64]=globs->ticker+(TICKS*60*15);    // self destruction

        return 1;
}

int use_grolm(int cn,int in)
{
        int cc;

        if (!cn) return 0;
        if (!it[in].carried) return 0;

        cc=god_create_char(577,1);
        if (!god_drop_char_fuzzy(cc,ch[cn].x,ch[cn].y)) {
                ch[cc].used=USE_EMPTY;
                do_char_log(cn,0,"The Grolm could not materialize.\n");
                return 0;
        }

        god_take_from_char(in,cn);
        it[in].used=USE_EMPTY;

        ch[cc].data[42]=65536+cn;                       // set group
        ch[cc].data[59]=65536+cn;                       // protect all other members of this group

        ch[cc].data[63]=cn;                             // obey and protect char
        ch[cc].data[69]=cn;                             // follow char
        ch[cc].data[64]=globs->ticker+(TICKS*60*15);    // self destruction

        return 1;
}

void boost_char(int cn,int divi)
{
	int n,in,rank,exp;
	char buf[80];
	
	for (n=0; n<5; n++) {
                if (ch[cn].attrib[n][0]>divi) ch[cn].attrib[n][0]+=RANDOM(ch[cn].attrib[n][0]/divi);
        }
        for (n=0; n<MAXSKILL; n++) {
                if (ch[cn].skill[n][0]>divi) ch[cn].skill[n][0]+=RANDOM(ch[cn].skill[n][0]/divi);
        }

        sprintf(buf,"Strong %s",ch[cn].name);
        strncpy(ch[cn].name,buf,39); ch[cn].name[39]=0;
	strncpy(ch[cn].reference,buf,39); ch[cn].reference[39]=0;

        in=god_create_item(1146);
        if (in) {
        	exp=ch[cn].points_tot/10+RANDOM(ch[cn].points_tot/20+1);
        	rank=points2rank(exp);
        	
        	sprintf(it[in].name,"Soulstone");
        	sprintf(it[in].reference,"soulstone");
        	sprintf(it[in].description,"Level %d soulstone, holding %d exp.",rank,exp);
        	
        	it[in].data[0]=rank;
        	it[in].data[1]=exp;
        	it[in].temp=0;
        	it[in].driver=68;
        	
        	god_give_char(in,cn);
        }
}

int spawn_penta_enemy(int in)
{
        int cn,tmp;

        if (it[in].data[9]==10) tmp=RANDOM(2)+9;
        else if (it[in].data[9]==11) tmp=RANDOM(2)+11;
        else if (it[in].data[9]==17) tmp=RANDOM(2)+17;
        else if (it[in].data[9]==18) tmp=RANDOM(2)+18;
        else if (it[in].data[9]==21) tmp=22;
        else if (it[in].data[9]==22) tmp=23;
        else if (it[in].data[9]==23) tmp=24;
        else tmp=RANDOM(3)-1+it[in].data[9];

        if (tmp<0) tmp=0;
	
	if (tmp>=22) {
                tmp-=22;
                if (tmp>3) tmp=3;
                cn=pop_create_char(1094+tmp,0);
        } else if (tmp>17) {
                tmp-=17;
                if (tmp>4) tmp=4;
                cn=pop_create_char(538+tmp,0);
        } else cn=pop_create_char(364+tmp,0);

        if (!cn) return 0;
        ch[cn].flags&=~CF_RESPAWN;
        ch[cn].data[0]=in;
        ch[cn].data[29]=it[in].x+it[in].y*MAPX;
        ch[cn].data[60]=TICKS*60*2;
        ch[cn].data[73]=8;
        ch[cn].dir=1;

        if (!RANDOM(25)) boost_char(cn,5);

        if (!god_drop_char_fuzzy(cn,it[in].x,it[in].y)) {
                god_destroy_items(cn);
                ch[cn].used=USE_EMPTY;
                return 0;
        } else return cn;
}

static int penta_needed=54;

void solved_pentagram(int cn,int in)
{
        int n,bonus;

        bonus=(it[in].data[0]*it[in].data[0]*3)/7+1;
        ch[cn].data[18]+=bonus;

        do_char_log(cn,0,"You solved the pentagram quest. Congratulations! You will get %d bonus experience points.\n",bonus);
        chlog(cn,"Solved pentagram quest");

        for (n=1; n<MAXCHARS; n++) {
                if (ch[n].used==USE_EMPTY) continue;
                if (!(ch[n].flags&(CF_PLAYER|CF_USURP))) continue;

                if (ch[n].used==USE_ACTIVE) {
                        if (n!=cn) do_char_log(n,0,"%s solved the pentagram quest!\n",ch[cn].name);
                }

                if (ch[n].data[18]) {
                        do_give_exp(n,ch[n].data[18],0,-1);
                        ch[n].data[18]=0;
                }
        }

        for (n=1; n<MAXITEM; n++) {
                if (it[n].used==USE_EMPTY) continue;
                if (it[n].driver!=33) continue;
                if (it[n].active==0) {
                        if (it[n].light[0]!=it[n].light[1] && it[n].x>0) {
                                do_add_light(it[n].x,it[n].y,it[n].light[1]-it[n].light[0]);
                        }
                }
                it[n].duration=it[n].active=TICKS*(10+RANDOM(20));
        }
        penta_needed=globs->players_online+RANDOM(20)+RANDOM(20)+RANDOM(20);
	xlog("New solve will be at %d (%d online)",penta_needed,globs->players_online);
}

int use_pentagram(int cn,int in)
{
        int n,v,tot=0,act=0,b[5]={0,0,0,0,0},bv[5]={0,0,0,0,0},exp=0,co,m;
        int r1,r2;

        if (it[in].active) {
                if (cn) do_char_log(cn,0,"This pentagram is already active.\n");
                else {
                        for (m=1; m<4; m++) {
                                if ((co=it[in].data[m])==0 || ch[co].data[0]!=in || ch[co].used==USE_EMPTY || (ch[co].flags&CF_BODY)) {
                                        it[in].data[m]=spawn_penta_enemy(in);
                                }
                        }
                }
                return 0;
        }

        r1=points2rank(ch[cn].points_tot);
        r2=it[in].data[9];
        if (r2<5) r2+=5;
        else if (r2<7) r2+=6;
        else if (r2<9) r2+=7;
        else if (r2<11) r2+=8;
        else r2+=9;

        if (r1>r2) {
                do_char_log(cn,0,"You cannot use this pentagram. It is reserved for rank %s and below.\n",
                        rank_name[min(23,max(0,r2))]);
                return 0;
        }

        v=it[in].data[0];
        it[in].data[8]=cn;
        it[in].duration=-1;

        do_char_log(cn,1,"You activated the pentagram with the value %d. It is worth %d experience point%s.\n",v,(v*v)/7+1,v==1 ? "" : "s");

        for (n=1; n<MAXITEM; n++) {
                if (it[n].used==USE_EMPTY) continue;
                if (it[n].driver!=33) continue;
                tot++;
                if (n!=in && it[n].active!=-1) continue;
                act++;
                if (it[n].data[8]!=cn) continue;

                v=it[n].data[0];
                if (v>bv[0]) {
                        bv[4]=bv[3]; bv[3]=bv[2]; bv[2]=bv[1]; bv[1]=bv[0]; bv[0]=v;
                        b[4]=b[3]; b[3]=b[2]; b[2]=b[1]; b[1]=b[0]; b[0]=n;
                } else if (v>bv[1]) {
                        bv[4]=bv[3]; bv[3]=bv[2]; bv[2]=bv[1]; bv[1]=v;
                        b[4]=b[3]; b[3]=b[2]; b[2]=b[1]; b[1]=n;
                } else if (v>bv[2]) {
                        bv[4]=bv[3]; bv[3]=bv[2]; bv[2]=v;
                        b[4]=b[3]; b[3]=b[2]; b[2]=n;
                } else if (v>bv[3]) {
                        bv[4]=bv[3]; bv[3]=v;
                        b[4]=b[3]; b[3]=n;
                } else if (v>bv[4]) {
                        bv[4]=v;
                        b[4]=n;
                }
        }

        if (b[0]) do_char_log(cn,1,"You're holding:\n");

        for (n=0; n<5; n++) {
                if (b[n]) {
                        do_char_log(cn,1,"Pentagram %3d, worth %5d point%s.\n",
                                bv[n],(bv[n]*bv[n])/7+1,bv[n]==1 ? "" : "s");
                        exp+=(bv[n]*bv[n])/7+1;
                }
        }
        ch[cn].data[18]=exp;
        do_char_log(cn,1,"Your pentagrammas are worth a total of %d experience points. Note that only the highest 5 pentagrammas count towards your experience bonus.\n",exp);
        do_char_log(cn,1,"There are %d pentagrammas total, of which %d are active.\n",tot,act);

        chlog(cn,"Activated pentagram %d (%d of %d)",it[in].data[0],act,penta_needed);

        if (act>=penta_needed) { solved_pentagram(cn,in); return 0; }

        for (m=1; m<4; m++) {
                if ((co=it[in].data[m])==0 || ch[co].data[0]!=in || ch[co].used==USE_EMPTY || (ch[co].flags&CF_BODY)) {
                        it[in].data[m]=spawn_penta_enemy(in);
                }
        }

        return 1;
}

int use_shrine(int cn,int in)
{
        int in2,val,rank,m;

        if (!cn) return 0;

        if ((in2=ch[cn].citem)==0) {
                do_char_log(cn,1,"You get the feeling that it would be apropriate to give the gods a present.\n");
                return 0;
        }

        if (IS_SANEITEM(in2) && (strcmp(it[in2].description,"ONE FIRE POINT")==0 || strcmp(it[in2].description,"ONE FAKE POINT")==0)) {
                int better=0,worse=0,equal=0,bestval=0,bestcn=0,bestcount=0;

                if (strcmp(it[in2].description,"ONE FIRE POINT")==0) {
                        ch[cn].data[70]++;
                        do_char_log(cn,1,"One fire point accounted for. You now have %d fire points.\n",ch[cn].data[70]);
                } else {
                        do_char_log(cn,1,"Err, that's a fake point. You have %d fire points.\n",ch[cn].data[70]);
                }

                it[in2].used=USE_EMPTY;
                ch[cn].citem=0;

                for (m=1; m<MAXCHARS; m++) {
                        if (!IS_USEDCHAR(m)) continue;
                        if (!IS_SANEPLAYER(m)) continue;
                        if (!ch[m].data[70]) continue;

                        if (ch[m].data[70]>ch[cn].data[70]) better++;
                        else if (ch[m].data[70]<ch[cn].data[70]) worse++;
                        else equal++;
                        if (ch[m].data[70]>bestval) { bestval=ch[m].data[70]; bestcn=m; bestcount=0; }
                        if (ch[m].data[70]==bestval) bestcount++;
                }
                if (equal>1) do_char_log(cn,1,"Your rank is %d, there are %d participating players of the same rank, %d are worse.\n",better+1,equal-1,worse);
                else do_char_log(cn,1,"Your rank is %d and %d participating players are worse.\n",better+1,worse);

                do_char_log(cn,1," \n");

                if (bestcount==1) do_char_log(cn,1,"First place holder is %s with %d fire points.\n",ch[bestcn].name,bestval);
                else {
                        do_char_log(cn,1,"First place is shared by %d players, all with %d fire points:\n",bestcount,bestval);
                        do_char_log(cn,1," \n");
                        for (m=1; m<MAXCHARS; m++) {
                                if (!IS_USEDCHAR(m)) continue;
                                if (!IS_SANEPLAYER(m)) continue;
                                if (!ch[m].data[70]) continue;

                                if (ch[m].data[70]==bestval) do_char_log(cn,1,"%s\n",ch[m].name);
                        }
                }
                do_char_log(cn,1," \n");


                return 0;
        }

        if (in2&0x80000000) {
                val=in2&0x7fffffff;
                ch[cn].citem=0;
        } else {
                val=it[in2].value;
                if (it[in2].flags&IF_UNIQUE) val*=4;
                it[in2].used=USE_EMPTY;
                ch[cn].citem=0;
        }

        rank=points2rank(ch[cn].points_tot)+1;
        rank=rank*rank*rank*4;
        val+=RANDOM(val+1);

//      do_char_log(cn,2,"rank=%d, val=%d\n",rank,val);

        if (val>=rank) {
                if (ch[cn].a_mana<ch[cn].mana[5]*1000) {
                        ch[cn].a_mana=ch[cn].mana[5]*1000;
                        do_char_log(cn,0,"You feel the hand of the Goddess of Magic touch your mind.\n");
                        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
                }

                if (val>=rank*64) {
//                      ch[cn].luck+=64;
                        do_char_log(cn,1,"The gods are madly in love with your offer.\n");
                } else if (val>=rank*32) {
//                      ch[cn].luck+=32;
                        do_char_log(cn,1,"The gods love your offer very much.\n");
                } else if (val>=rank*16) {
//                      ch[cn].luck+=16;
                        do_char_log(cn,1,"The gods love your offer.\n");
                } else if (val>=rank*8) {
//                      ch[cn].luck+=8;
                        do_char_log(cn,1,"The gods are very pleased with your offer.\n");
                } else if (val>=rank*4) {
//                      ch[cn].luck+=4;
                        do_char_log(cn,1,"The gods are pleased with your offer.\n");
                } else if (val>=rank*2) {
//                      ch[cn].luck+=2;
                        do_char_log(cn,1,"The gods deemed your offer apropriate.\n");
                } else {
                        do_char_log(cn,1,"The gods accepted your offer.\n");
                }
                if (val && rank) {
                        m=val/rank;
                        ch[cn].luck+=m;
                        if (in2&0x80000000)
                                chlog(cn,"Increased luck by %d to %d for %dG %dS",m,ch[cn].luck,val/100,val%100);
                        else
                                chlog(cn,"Increased luck by %d to %d for %dG %dS (%s)",m,ch[cn].luck,val/100,val%100,it[in2].name);
                }
        } else {
                if (val<rank/8) {
                        do_char_log(cn,1,"You have angered the gods with your unworthy gift.\n");
                        ch[cn].luck-=2;
                } else if (val<rank/4) {
                        do_char_log(cn,1,"The gods sneer at your gift.\n");
                        ch[cn].luck--;
                } else if (val<rank/2) {
                        do_char_log(cn,1,"The gods think you're cheap.\n");
                } else {
                        do_char_log(cn,1,"You feel that it takes more than this to please the gods.\n");
                }
        }

        do_char_log(cn,1," \n");

        if (ch[cn].luck<-10000)
                do_char_log(cn,1,"You feel that the gods are mad at you.\n");
        else if (ch[cn].luck<0)
                do_char_log(cn,1,"You feel that the gods are angry at you.\n");
        else if (ch[cn].luck<100)
                do_char_log(cn,1,"You feel that the gods stance towards you is neutral.\n");
        else if (ch[cn].luck<1000)
                do_char_log(cn,1,"You feel that the gods are pleased with you.\n");
        else do_char_log(cn,1,"You feel that the gods are very fond of you.\n");

        return 1;
}

int use_kill_undead(int cn,int in)
{
        int x,y,co;

        if (!cn) return 0;

        if (ch[cn].worn[WN_RHAND]!=in) return 0;

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        for (y=ch[cn].y-8; y<ch[cn].y+8; y++) {
                if (y<1 || y>=MAPY) continue;
                for (x=ch[cn].x-8; x<ch[cn].x+8; x++) {
                        if (x<1 || x>=MAPX) continue;
                        if ((co=map[x+y*MAPX].ch)!=0) {
                                if (ch[co].flags&CF_UNDEAD) {
                                        do_hurt(cn,co,500,2);
                                        fx_add_effect(5,0,ch[co].x,ch[co].y,0);
                                }
                        }
                }
        }

        item_damage_worn(cn,WN_RHAND,500);

        return 1;
}

int teleport3(int cn,int in)    // out of labyrinth
{
        int n,in2;

        if (!cn) return 1;
        if (it[in].flags&IF_USEACTIVATE && !(it[in].active)) return 1;

        if ((in2=ch[cn].citem) && is_nolab_item(in2)) {
                ch[cn].citem=0; it[in2].used=USE_EMPTY;
                do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
        }

        for (n=0; n<40; n++) {
                if ((in2=ch[cn].item[n]) && is_nolab_item(in2)) {
                        ch[cn].item[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }

        for (n=0; n<20; n++) {
                if ((in2=ch[cn].spell[n]) && (it[in2].temp==SK_RECALL)) {
                        ch[cn].spell[n]=0; it[in2].used=USE_EMPTY;
                }
        }

        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        god_transfer_char(cn,it[in].data[0],it[in].data[1]);
        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);

        if ((in2=ch[cn].citem) && !(in2&0x80000000) && (it[in2].flags&IF_LABYDESTROY)) {
                ch[cn].citem=0; it[in2].used=USE_EMPTY;
                do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
        }
        for (n=0; n<40; n++) {
                if ((in2=ch[cn].item[n]) && (it[in2].flags&IF_LABYDESTROY)) {
                        ch[cn].item[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }
        for (n=0; n<20; n++) {
                if ((in2=ch[cn].worn[n]) && (it[in2].flags&IF_LABYDESTROY)) {
                        ch[cn].worn[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }

	if (ch[cn].kindred&KIN_PURPLE) {
		ch[cn].temple_x=ch[cn].tavern_x=558;
        	ch[cn].temple_y=ch[cn].tavern_y=542;
	} else if (ch[cn].flags&CF_STAFF) {
		ch[cn].temple_x=ch[cn].tavern_x=813;
        	ch[cn].temple_y=ch[cn].tavern_y=165;
	} else {
	        ch[cn].temple_x=ch[cn].tavern_x=512;
        	ch[cn].temple_y=ch[cn].tavern_y=512;
        }

        return 1;
}

int use_seyan_shrine(int cn,int in)
{
        struct item tmp;
        int in2,n,bits;
        unsigned int bit;

        if (!cn) return 0;

        if (!(ch[cn].kindred&KIN_SEYAN_DU)) {
                do_char_log(cn,0,"You have the feeling you're in the wrong place here.\n");
                return 0;
        }

        // remove old sword if necessary
        if (!(in2=ch[cn].worn[WN_RHAND]) || it[in2].driver!=40 || it[in2].data[0]!=cn) {
                for (n=1; n<MAXITEM; n++) {
                        if (it[n].used!=USE_ACTIVE) continue;
                        if (it[n].driver!=40) continue;
                        if (it[n].data[0]!=cn) continue;

                        tmp=it_temp[683];
                        tmp.x=it[n].x;
                        tmp.y=it[n].y;
                        tmp.carried=it[n].carried;
                        tmp.temp=683;
                        it[n]=tmp;
                        it[n].flags|=IF_UPDATE;
                }

                if (ch[cn].luck<50) {
                        do_char_log(cn,0,"Kwai, the great goddess of war, deemed you unworthy to receive a new blade.\n");
                        return 0;
                }

                in2=god_create_item(682);
                god_give_char(in2,cn);
                it[in2].data[0]=cn;
                do_char_log(cn,0,"Kwai, the great goddess of war, deemed you worthy to receive a new blade.\n");
                ch[cn].luck-=50;
        }

        if (!(ch[cn].data[21]&it[in].data[0])) {
                ch[cn].data[21]|=it[in].data[0];

                do_char_log(cn,0,"You found a new shrine of Kwai!\n");
                ch[cn].luck+=10;
        }

        for (bits=0,bit=1; bit; bit<<=1) {
                if (ch[cn].data[21]&bit) bits++;
        }

        do_char_log(cn,1,"You have visited %d of the 20 shrines of Kwai.\n",bits);

        it[in2].weapon[0]=15+bits*4;
        it[in2].flags|=IF_UPDATE;
        it[in2].temp=0;

        sprintf(it[in2].description,"A huge, two-handed sword, engraved with runes and magic symbols. It bears the name %s.",
                ch[cn].name);

        do_update_char(cn);

        return 0;
}

int use_seyan_door(int cn,int in)
{
        if (cn && !(ch[cn].kindred&KIN_SEYAN_DU)) return 0;
        else return use_door(cn,in);
}

int use_seyan_portal(int cn,int in)
{
        int in2,n;

        if (!cn) return 0;

        if (ch[cn].kindred&KIN_SEYAN_DU) {
                do_char_log(cn,0,"You're already Seyan'Du, aren't you?\n");
        } else {
                do_char_log(cn,0,"The Seyan'Du welcome you among their ranks, %s!\n",ch[cn].name);
                if (ch[cn].kindred&KIN_MALE) god_racechange(cn,13);
                else god_racechange(cn,79);

                in2=god_create_item(682);
                god_give_char(in2,cn);
                it[in2].data[0]=cn;
        }

        if ((in2=ch[cn].citem) && !(in2&0x80000000) && (it[in2].flags&IF_LABYDESTROY)) {
                ch[cn].citem=0; it[in2].used=USE_EMPTY;
                do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
        }
        for (n=0; n<40; n++) {
                if ((in2=ch[cn].item[n]) && (it[in2].flags&IF_LABYDESTROY)) {
                        ch[cn].item[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }
        for (n=0; n<20; n++) {
                if ((in2=ch[cn].worn[n]) && (it[in2].flags&IF_LABYDESTROY)) {
                        ch[cn].worn[n]=0; it[in2].used=USE_EMPTY;
                        do_char_log(cn,1,"Your %s vanished.\n",it[in2].reference);
                }
        }

        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        god_transfer_char(cn,it[in].data[0],it[in].data[1]);
        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);

        return 1;
}

int spell_scroll(int cn,int in)
{
        int spell,power,charges,ret,co;

        spell=it[in].data[0];
        power=it[in].data[1];
        charges=it[in].data[2];

        if (!charges) {
                do_char_log(cn,0,"Nothing happened!\n");
                return 0;
        }

        if ((co=ch[cn].skill_target1)) ;
        else co=cn;

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return 0;
        }

        if (spell==SK_CURSE || spell==SK_STUN) {
                if (!may_attack_msg(cn,co,1)) {
                        chlog(cn,"Prevented from attacking %s (%d)",ch[co].name,co);
                return 0;
        }
        } else if (!player_or_ghost(cn,co)) {
                do_char_log(cn,0,"Changed target of spell from %s to %s.\n",ch[co].name,ch[cn].name);
                co=cn;
        }

        switch(spell) {
                case    SK_LIGHT:       ret=spell_light(cn,co,power); break;
                case    SK_ENHANCE:     ret=spell_enhance(cn,co,power); break;
                case    SK_PROTECT:     ret=spell_protect(cn,co,power); break;
                case    SK_BLESS:       ret=spell_bless(cn,co,power); break;
                case    SK_MSHIELD:     ret=spell_mshield(cn,co,power); break;
                case    SK_CURSE:       if (chance_base(cn,power,10,ch[co].skill[SK_RESIST][5])) ret=1;
                                        else ret=spell_curse(cn,co,power);
                                        break;
                case    SK_STUN:        if (chance_base(cn,power,12,ch[co].skill[SK_RESIST][5])) ret=1;
                                        else ret=spell_stun(cn,co,power);
                                        break;
                default:                ret=0;
        }

        if (ret) {
                charges--;
                it[in].data[2]=charges;
                it[in].value/=2;
                if (charges<1) return 1;
        }

        return 0;
}

int use_blook_penta(int cn,int in)
{
        if (!cn) return 0;

        do_char_log(cn,1,"You try to wipe off the blood, but it seems to be coming back slowly.\n");
        it[in].data[0]=1;
        it[in].sprite[0]=it[in].data[1]+it[in].data[0];

        return 1;
}

int use_create_npc(int cn,int in)
{
        int co;

        if (it[in].active) return 0;
        if (!cn) return 0;

        co=pop_create_char(it[in].data[0],0);
        if (!co) return 0;

        if (!god_drop_char_fuzzy(co,it[in].x,it[in].y)) {
                god_destroy_items(co);
                ch[co].used=USE_EMPTY;
                return 0;
        }

        ch[co].data[0]=cn;

        return 1;
}

int use_rotate(int cn,int in)
{
        if (!cn) return 0;

        it[in].data[1]++;
        if (it[in].data[1]>3) it[in].data[1]=0;
        it[in].sprite[0]=it[in].data[0]+it[in].data[1];
        it[in].flags|=IF_UPDATE;

        return 1;
}

/* CS, 991127: assemble lab8 key */
int use_lab8_key(int cn, int in)
{
        // data[0] = matching key part
        // data[1] = resulting key part
        // data[2] = (optional) other matching key part
        // data[3] = (optional) other resulting key part

        int in2,in3=0;

        if (cn == 0) return 0;

        in2=ch[cn].citem;
        if (!in2 || (in2&0x80000000)) {
                do_char_log(cn,1,"Nothing happens.\n");
                return 0;
        }

        if (it[in].carried==0) {
                do_char_log(cn,0,"Too difficult to do on the ground.\n");
                return 0;
        }

        // Check for one of the 1 or 2 matching parts
        if (it[in].data[0] == it[in2].temp) in3 = it[in].data[1];
        if (it[in].data[2] == it[in2].temp) in3 = it[in].data[3];
        if (in3 == 0) {
                do_char_log(cn, 0, "Those don't fit together.\n");
                return 0;
        }

        chlog(cn,"Added %s to %s",it[in2].name,it[in].name);

        god_take_from_char(in,cn);
        it[in].used=USE_EMPTY;

        ch[cn].citem=0;
        it[in2].used=USE_EMPTY;

        in3=god_create_item(in3);
        it[in3].flags|=IF_UPDATE;
        god_give_char(in3,cn);

        return 1;
}

/* CS, 991127: Shrines in Lab 8 */
int use_lab8_shrine(int cn, int in)
{
        // data[0] = item accepted as offering
        // data[1] = item returned as gift

        int offer, gift;

        if (cn == 0) return 0;

        if ((offer=ch[cn].citem) == 0) {
                do_char_log(cn,1,"You get the feeling that it would be apropriate to give the Goddess a present.\n");
                return 0;
        }

        /* CS, 991130: Forgot to check for money here. <Slap Elrac> */
        if ((offer & 0x80000000) ||
            (it[offer].temp != it[in].data[0])) {
                do_char_log(cn,1,"The Goddess only wants her property back, and rejects your offer.\n");
                return 0;
        }

        ch[cn].citem = 0;
        it[offer].used = USE_EMPTY;

        chlog(cn, "Offered %s at %s", it[offer].reference, it[in].reference);

        gift = god_create_item(it[in].data[1]);
        if (!god_give_char(gift, cn)) {
                ch[cn].citem = gift;
                it[gift].carried = cn;
        }
        do_char_log(cn, 1, "The Goddess has given you a %s in return!\n",
                it[gift].reference);

        return 1;
}

/* CS, 991127: Shrines in Lab 8 */
int use_lab8_moneyshrine(int cn, int in)
{
        // data[0] = minimum offering accepted
        // data[1] = teleport coordinate x
        // data[2] = teleport coordinate y

        int offer;

        if (cn == 0) return 0;

        if ((offer=ch[cn].citem) == 0) {
                do_char_log(cn,1,"You get the feeling that it would be apropriate to give the Goddess a present.\n");
                return 0;
        }

        if ((offer&0x80000000) == 0) {
                do_char_log(cn,1,"Only money is accepted at this shrine.\n");
                return 0;
        }

        offer &= 0x7fffffff;
        if (offer < it[in].data[0]) {
                do_char_log(cn,1,"Your offering is not sufficient, and was rejected.\n");
                return 0;
        }

        chlog(cn, "offered %dG at %s", offer/100, it[in].reference);

        ch[cn].citem = 0;
        god_transfer_char(cn,it[in].data[1],it[in].data[2]);
        if (ch[cn].a_mana<ch[cn].mana[5]*1000) {
                ch[cn].a_mana=ch[cn].mana[5]*1000;
                do_char_log(cn,0,"You feel the hand of the Goddess of Magic touch your mind.\n");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }
        return 1;
}

void change_to_archtemplar(int cn)
{
        if (ch[cn].attrib[AT_AGIL][0]<90) {
                do_char_log(cn,1,"Your agility is too low. There is still room for improvement.\n");
                return;
        }
        if (ch[cn].attrib[AT_STREN][0]<90) {
                do_char_log(cn,1,"Your strength is too low. There is still room for improvement.\n");
                return;
        }

        if (ch[cn].kindred&KIN_MALE) god_minor_racechange(cn,544);
        else god_minor_racechange(cn,549);

        do_char_log(cn,1,"You are truly worthy to become a Archtemplar. Congratulations, %s.\n",ch[cn].name);
}

void change_to_archharakim(int cn)
{
        if (ch[cn].attrib[AT_WILL][0]<90) {
                do_char_log(cn,1,"Your willpower is too low. There is still room for improvement.\n");
                return;
        }
        if (ch[cn].attrib[AT_INT][0]<90) {
                do_char_log(cn,1,"Your intuition is too low. There is still room for improvement.\n");
                return;
        }

        if (ch[cn].kindred&KIN_MALE) god_minor_racechange(cn,545);
        else god_minor_racechange(cn,550);

        do_char_log(cn,1,"You are truly worthy to become a Archharakim. Congratulations, %s.\n",ch[cn].name);
}

void change_to_warrior(int cn)
{
        if (ch[cn].attrib[AT_AGIL][0]<60) {
                do_char_log(cn,1,"Your agility is too low. There is still room for improvement.\n");
                return;
        }
        if (ch[cn].attrib[AT_STREN][0]<60) {
                do_char_log(cn,1,"Your strength is too low. There is still room for improvement.\n");
                return;
        }

        if (ch[cn].kindred&KIN_MALE) god_minor_racechange(cn,547);
        else god_minor_racechange(cn,552);

        do_char_log(cn,1,"You are truly worthy to become a Warrior. Congratulations, %s.\n",ch[cn].name);
}

void change_to_sorcerer(int cn)
{
        if (ch[cn].attrib[AT_WILL][0]<60) {
                do_char_log(cn,1,"Your willpower is too low. There is still room for improvement.\n");
                return;
        }
        if (ch[cn].attrib[AT_INT][0]<60) {
                do_char_log(cn,1,"Your intuition is too low. There is still room for improvement.\n");
                return;
        }

        if (ch[cn].kindred&KIN_MALE) god_minor_racechange(cn,546);
        else god_minor_racechange(cn,551);

        do_char_log(cn,1,"You are truly worthy to become a Sorcerer. Congratulations, %s.\n",ch[cn].name);
}

int shrine_of_change(int cn,int in)
{
        int in2;

        if (!cn) return 0;

        if (!(in2=ch[cn].citem) || in2&0x80000000) {
                do_char_log(cn,1,"Read the notes, my friend.\n");
                return 0;
        }

        if (it[in2].temp==148) {        // potion of life
                if (ch[cn].kindred&KIN_TEMPLAR) change_to_archtemplar(cn);
                else if (ch[cn].kindred&KIN_HARAKIM) change_to_archharakim(cn);
                else do_char_log(cn,1,"You are neither Templar nor Harakim.\n");
                return 0;
        }

        if (it[in2].temp==127 || it[in2].temp==274) {   // greater healing potion
                if (ch[cn].kindred&KIN_MERCENARY) change_to_warrior(cn);
                else do_char_log(cn,1,"You are not a Mercenary.\n");
                return 0;
        }

        if (it[in2].temp==131 || it[in2].temp==273) {   // greater mana potion
                if (ch[cn].kindred&KIN_MERCENARY) change_to_sorcerer(cn);
					 else do_char_log(cn,1,"You are not a Mercenary.\n");
                return 0;
        }
        do_char_log(cn,1,"Read the notes, my friend.\n");
        return 0;
}

int explorer_point(int cn,int in)
{
        int exp;

        if (!(ch[cn].data[46]&it[in].data[0]) && !(ch[cn].data[47]&it[in].data[1]) &&
            !(ch[cn].data[48]&it[in].data[2]) && !(ch[cn].data[49]&it[in].data[3])) {

                ch[cn].data[46]|=it[in].data[0];
                ch[cn].data[47]|=it[in].data[1];
                ch[cn].data[48]|=it[in].data[2];
                ch[cn].data[49]|=it[in].data[3];

                do_char_log(cn,0,"You found a new exploration point!\n");
                ch[cn].luck+=10;

                exp=it[in].data[4]/2+RANDOM(it[in].data[4]);    // some randomness
                exp=min(ch[cn].points_tot/10,exp);              // not more than 10% of total experience
                exp+=RANDOM(exp/10+1);                          // some more randomness (needs +1 to avoid division by zero)

                xlog("exp point giving %d (%d) exp, char has %d exp",exp,it[in].data[4],ch[cn].points_tot);
                do_give_exp(cn,exp,0,-1);

        } else do_char_log(cn,1,"Hmm. Seems somewhat familiar. You've been here before...\n");

        return 1;
}

int use_garbage(int cn,int in)
{
        int in2, val;
		
        if (!cn) return 0;
	/* if no character, do nothing */

        if ((in2=ch[cn].citem)==0) {
                do_char_log(cn,1,"You feel that you could dispose of unwanted items in this digusting garbage can.\n");
                return 0;
        }
        if (in2&0x80000000) {
               	val=in2&0x7fffffff;
               	ch[cn].citem=0;
				
	        	do_char_log(cn,1,"You disposed of %u gold and %u silver.\n",val/100,val%100);
		} else {
	        	ch[cn].citem=0;
	   	     	it[in2].used=USE_EMPTY;
				/* JC, 000615: set item in hand to nothing, effectively destroying it */
				
				do_char_log(cn,1,"You disposed of the %s.\n",it[in2].reference);
		}
		
        return 1;
}

void use_driver(int cn,int in,int carried)
{
        int ret,m;

        if (in<=0 || in>=MAXITEM) return;
        if (cn<0 || cn>=MAXCHARS) return;
        if (cn && (ch[cn].flags&CF_BUILDMODE)) return;

        if (cn && !carried) ch[cn].cerrno=ERR_FAILED;   // will be overriden later if another result is desired

        if ((it[in].flags&IF_USE) || !cn) {
                // check prerequisites (cost, minium stats etc) !!!
                // remove cost !!!

                if (!carried) {
                        m=it[in].x+it[in].y*MAPX;
                        if (map[m].ch || map[m].to_ch) return;
                }

                if (it[in].flags&IF_USESPECIAL) {
                        switch(it[in].driver) {
                                case 1:         ret=use_create_item(cn,in); break; // uses may-deactivate
                                case 2:         ret=use_door(cn,in); break;
                                case 3:         do_char_log(cn,0,"You use cannot the lock-pick directly. Hold it under your mouse cursor and click on the door...\n");
                                                ret=0; break;
                                case 4:         ret=use_mix_potion(cn,in); break;
                                case 5:         ret=stone_sword(cn,in); break;
                                case 6:         ret=teleport(cn,in); break;
                                case 7:         ret=use_bag(cn,in); break;
                                case 8:         ret=use_scroll(cn,in); break; // skills
                                case 9:         ret=use_crystal(cn,in); break;
                                case 10:        ret=use_scroll2(cn,in); break; // attribs
                                case 11:        ret=use_scroll3(cn,in); break; // hp
                                case 12:        ret=use_scroll4(cn,in); break; // endurance
                                case 13:        ret=use_scroll5(cn,in); break; // mana
                                case 14:        ret=use_chain(cn,in); break; // leather necklace + lizards teeth
                                case 15:        ret=use_labyrinth(cn,in); break; // teleport into labyrinth special
                                case 16:        ret=use_ladder(cn,in); break;
                                case 17:        ret=rat_eye(cn,in); break;
                                case 18:        ret=skua_protect(cn,in); break;
                                case 19:        ret=use_lever(cn,in); break;
                                case 20:        ret=use_door(cn,in); break;
                                case 21:        ret=use_spawn(cn,in); break;
                                case 22:        ret=use_pile(cn,in); break;
                                case 23:        ret=teleport2(cn,in); break;
                                case 24:        ret=build_ring(cn,in); break;
                                case 25:        ret=use_mine(cn,in); break;
                                case 26:        ret=use_mine_fast(cn,in); break;
                                case 27:        ret=use_mine_respawn(cn,in); break;
                                case 28:        ret=use_gargoyle(cn,in); break;
                                case 29:        ret=use_grave(cn,in); break;
                                case 30:        ret=use_create_item2(cn,in); break;
                                case 31:        ret=0; break; // empty, hole water
                                case 32:        ret=build_amulet(cn,in); break;
                                case 33:        ret=use_pentagram(cn,in); break;
                                case 34:        ret=use_seyan_shrine(cn,in); break;
                                case 35:        ret=use_seyan_door(cn,in); break;
                                case 36:        ret=0; break; // magic portal 1 in lab13
                                case 37:        ret=0; break; // traps
                                case 38:        ret=0; break; // magic portal 2 in lab13
                                case 39:        ret=purple_protect(cn,in); break;
                                case 40:        ret=0; break; // seyan'du sword
                                case 41:        ret=use_shrine(cn,in); break;
                                case 42:        ret=use_create_item3(cn,in); break;
                                case 43:        ret=0; break; // spiderweb
                                case 44:        ret=use_kill_undead(cn,in); break;
                                case 45:        ret=use_seyan_portal(cn,in); break;
                                case 46:        ret=teleport3(cn,in); break;
                                case 47:        ret=0; break; // arena portal
                                case 48:        ret=spell_scroll(cn,in); break;
                                case 49:        ret=use_blook_penta(cn,in); break; // blood-penta
                                case 50:        ret=use_create_npc(cn,in); break;
                                case 51:        ret=use_rotate(cn,in); break;
                                case 52:        ret=0; break; // personal item
                                case 53:        ret=use_create_item(cn,in); break; // for personal items
                                case 54:        ret=use_create_item(cn,in); break; // attack trigger
                                case 55:        ret=shrine_of_change(cn,in); break;
                                case 56:        ret=0; break; // greenling green ball
                                case 57:        ret=explorer_point(cn,in); break;
                                case 58:        ret=use_grolm(cn,in); break;
                                case 59:        ret=use_create_gold(cn,in); break;
                                /* CS, 991127: assembling Lab 8 key */
                                case 61:        ret=use_lab8_key(cn,in); break;
                                /* CS, 991127: offering to shrines in Lab 8 */
                                case 63:        ret=use_lab8_shrine(cn,in); break;
                                /* CS, 991127: offering money to shrine in Lab 8 */
                                case 64:        ret=use_lab8_moneyshrine(cn,in); break;
                                /* CS, 991219: switches in lab9 */
                                case 65:        ret=use_lab9_switch(cn,in); break;
                                /* CS, 991219: lab9 one way door */
                                case 66:        ret=use_lab9_door(cn,in); break;
				/* JC, 000615: garbage disposal */
                                case 67:        ret=use_garbage(cn,in); break;
                                case 68:	ret=use_soulstone(cn,in); break;
                                case 69:	ret=0; break;
                                default:        xlog("use_driver (use_driver.c): Unknown use_driver %d for item %d",it[in].driver,in);
                                                ret=0; break;
                        }
                        if (cn) {
                                if (!ret) { if (!carried) ch[cn].cerrno=ERR_FAILED; return; }
                                else if (!carried) ch[cn].cerrno=ERR_SUCCESS;
                        }
			do_update_char(cn);
                }
                if (!cn) return;        // item_tick does activate and deactive as well

                if (it[in].active) {
                        if (it[in].flags&IF_USEDEACTIVATE) {
                                it[in].active=0;
                                if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
                                        do_add_light(it[in].x,it[in].y,it[in].light[0]-it[in].light[1]);
                                if (carried) {
                                        it[in].flags|=IF_UPDATE;
                                        do_update_char(cn);
                                }
                                if (cn && !carried) ch[cn].cerrno=ERR_SUCCESS;
                        }
                } else {
                        if (it[in].flags&IF_USEACTIVATE) {
                                it[in].active=it[in].duration;
                                if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
                                        do_add_light(it[in].x,it[in].y,it[in].light[1]-it[in].light[0]);
                                if (carried) {
                                        it[in].flags|=IF_UPDATE;
                                        do_update_char(cn);
                                }
                                if (cn && !carried) ch[cn].cerrno=ERR_SUCCESS;
                        }
                        if (carried && (it[in].flags&IF_USEDESTROY)) {

                        	if (it[in].min_rank>points2rank(ch[cn].points_tot)) {
                        		do_char_log(cn,0,"You're not experienced enough to use this.\n");
                        		return;
                        	}

                                chlog(cn,"Used %s",it[in].name);

                                ch[cn].a_hp+=it[in].hp[0]*1000;
                                if (ch[cn].a_hp<0) ch[cn].a_hp=0;
                                ch[cn].a_end+=it[in].end[0]*1000;
                                if (ch[cn].a_end<0) ch[cn].a_end=0;
                                ch[cn].a_mana+=it[in].mana[0]*1000;
                                if (ch[cn].a_mana<0) ch[cn].a_mana=0;

                                if (it[in].duration) spell_from_item(cn,in);

                                it[in].used=USE_EMPTY;
                                god_take_from_char(in,cn);

                                if (ch[cn].a_hp<500) {
                                	do_area_log(cn,0,ch[cn].x,ch[cn].y,0,"%s was killed by %s.\n",
                                		ch[cn].name,it[in].reference);
                                		
                                	do_char_log(cn,0,"You were killed by %s.\n",
                                		it[in].reference);
                                		
	                                do_char_killed(0,cn);
	                        }

                                do_update_char(cn);
                        }
                }
        }
}

int item_age(int in)
{
        int act,st;

        if (it[in].active) act=1;
        else act=0;

        if ((it[in].max_age[act] && it[in].current_age[act]>it[in].max_age[act]) ||
                (it[in].max_damage && it[in].current_damage>it[in].max_damage)) {
                it[in].flags|=IF_UPDATE;

                it[in].current_damage=0;

                it[in].current_age[0]=0;
                it[in].current_age[1]=0;

                it[in].damage_state++;

                it[in].value/=2;

                if (it[in].damage_state>1) {

                        st=max(0,4-it[in].damage_state);

                        if (it[in].armor[0]>st) it[in].armor[0]--;
                        if (it[in].armor[1]>st) it[in].armor[1]--;

                        if (it[in].weapon[0]>st*2) {
                                it[in].weapon[0]--;
                                if (it[in].weapon[0]>0) it[in].weapon[0]--;
                        }
                        if (it[in].weapon[1]>st*2) {
                                it[in].weapon[1]--;
                                if (it[in].weapon[1]>0) it[in].weapon[1]--;
                        }
                }

                if (it[in].max_age[0]) it[in].sprite[0]++;
                if (it[in].max_age[1]) it[in].sprite[1]++;

                return 1;
        }

	// expire no-age items in one jump after thirty minutes
	// lag scrolls vanish after 60 seconds
        if (!it[in].max_age[act] &&
            (it[in].current_age[act]>TICKS*60*30 ||
             (it[in].temp==500 && it[in].current_age[act]>TICKS*60*2))) {
                it[in].damage_state=5;
                return 1;
        }

        return 0;
}

void item_damage_worn(int cn,int n,int dam)
{
        int in;

        if ((in=ch[cn].worn[n])!=0 && it[in].max_damage) {
                it[in].current_damage+=dam;
                if (item_age(in)) {
                        if (it[in].damage_state==1) do_char_log(cn,1,"The %s you are using is showing signs of use.\n",
                                                        it[in].reference);
                        else if (it[in].damage_state==2) do_char_log(cn,1,"The %s you are using was slightly damaged.\n",
                                                        it[in].reference);
                        else if (it[in].damage_state==3) do_char_log(cn,1,"The %s you are using was damaged.\n",
                                                        it[in].reference);
                        else if (it[in].damage_state==4) do_char_log(cn,0,"The %s you are using was badly damaged.\n",
                                                        it[in].reference);
                        else if (it[in].damage_state==5) {
                                ch[cn].worn[n]=0;
                                it[in].used=USE_EMPTY;
                                do_char_log(cn,0,"The %s you were using was destroyed.\n",
                                                it[in].reference);
                        }
                        do_update_char(cn);
                }
        }
}

void item_damage_citem(int cn,int dam)
{
        int in;

        if ((in=ch[cn].citem)!=0 && !(in&0x80000000) && it[in].max_damage) {
                it[in].current_damage+=dam;
                if (item_age(in)) {
                        if (it[in].damage_state==1) do_char_log(cn,1,"The %s you are using is showing signs of use.\n",
                                                        it[in].reference);
                        else if (it[in].damage_state==2) do_char_log(cn,1,"The %s you are using was slightly damaged.\n",
                                                        it[in].reference);
                        else if (it[in].damage_state==3) do_char_log(cn,1,"The %s you are using was damaged.\n",
                                                        it[in].reference);
                        else if (it[in].damage_state==4) do_char_log(cn,0,"The %s you are using was badly damaged.\n",
                                                        it[in].reference);
                        else if (it[in].damage_state==5) {
                                ch[cn].citem=0;
                                it[in].used=USE_EMPTY;
                                do_char_log(cn,0,"The %s you were using was destroyed.\n",
                                                it[in].reference);
                        }
                }
        }
}

void item_damage_armor(int cn,int dam)
{
        int n;

        dam=dam/4+1;

        for (n=0; n<20; n++)
                if (n!=WN_RHAND && n!=WN_LHAND) if (RANDOM(3)) item_damage_worn(cn,n,dam);
}

void item_damage_weapon(int cn,int dam)
{
        dam=dam/4+1;
        item_damage_worn(cn,WN_RHAND,dam);
}

void lightage(int in,int multi)
{
        int m,cn,light,act;

        if ((cn=it[in].carried)!=0) m=ch[cn].x+ch[cn].y*MAPX;
        else m=it[in].x+it[in].y*MAPX;

        if (m<1 || m>=MAPX*MAPY) return;

        light=map[m].light;
        if (light<1) return;
        if (light>250) light=250;

        light*=multi;

        if (it[in].active) act=1;
        else act=0;

        it[in].current_age[act]+=light*2;
}

/* CS, 991114: age messages for ice stuff and other stuff */
void age_message(int cn, int in, char *where)
{
        char *msg = "whatzit";
        int font = 1;

        if (it[in].driver == 60) { // ice egg or cloak
                switch (it[in].damage_state) {
                case 1:
                        msg = "The %s %s is beginning to melt.\n";
                        break;
                case 2:
                        msg = "The %s %s is melting fairly rapidly.\n";
                        break;
                case 3:
                        msg = "The %s %s is melting down as you look and dripping water everywhere.\n";
                        break;
                case 4:
                        msg = "The %s %s has melted down to a small icy lump and large puddles of water.\n";
                        font = 0;
                        break;
                case 5:
                        font = 0;
                        msg = "The %s %s has completely melted away, leaving you all wet.\n";
                }
        } else { // anything else
                switch (it[in].damage_state) {
                case 1:
                        msg = "The %s %s is showing signs of age.\n";
                        break;
                case 2:
                        msg = "The %s %s is getting fairly old.\n";
                        break;
                case 3:
                        msg = "The %s %s is getting old.\n";
                        break;
                case 4:
                        msg = "The %s %s is getting very old and battered.\n";
                        font = 0;
                        break;
                case 5:
                        font = 0;
                        msg = "The %s %s was so old and battered that it finally vanished.\n";
                }
        }
        do_char_log(cn, font, msg, it[in].reference, where);
}

void char_item_expire(int cn)
{
        int n,in,act;
        /* CS, 991114: Age ice cloak more slowly if not worn */
        static int clock4 = 0;
        int must_update = 0;

        if (IS_BUILDING(cn)) return;

        clock4++;
        /* age items in backpack */
        for (n=0; n<40; n++) {
                in=ch[cn].item[n];
                if (in) {
                        if (it[in].active) act=1;
                        else act=0;

                        if ((act==0 && (it[in].flags&IF_ALWAYSEXP1)) || (act==1 && (it[in].flags&IF_ALWAYSEXP2))) {
                                // ice cloak ages more slowly when not worn or held
                                if (it[in].driver == 60 && (clock4 % 4 != 0)) continue;
                                it[in].current_age[act]++;
                                if (it[in].flags&IF_LIGHTAGE) lightage(in,1);
                                if (item_age(in)) {
                                        must_update = 1;
                                        age_message(cn, in, "in your backpack");
                                        if (it[in].damage_state==5) {
                                                ch[cn].item[n]=0;
                                                it[in].used=USE_EMPTY;
                                        }
                                }
                        }
                }
        }

        /* age items in inventory */
        for (n=0; n<20; n++) {
                in=ch[cn].worn[n];
                if (in) {
                        if (it[in].active) act=1;
                        else act=0;

                        if ((act==0 && (it[in].flags&IF_ALWAYSEXP1)) || (act==1 && (it[in].flags&IF_ALWAYSEXP2))) {
                                it[in].current_age[act]++;
                                if (it[in].flags&IF_LIGHTAGE) lightage(in,1);
                                if (item_age(in)) {
                                        must_update = 1;
                                        if (it[in].damage_state==5) {
                                                age_message(cn, in, "you were using");
                                                ch[cn].worn[n]=0;
                                                it[in].used=USE_EMPTY;
                                        } else {
                                                age_message(cn, in, "you are using");
                                        }
                                }
                        }
                }
        }

        /* age item under mouse cursor (held) */
        in=ch[cn].citem;
        if (in && !(in&0x80000000)) {
                if (it[in].active) act=1;
                else act=0;

                if ((act==0 && (it[in].flags&IF_ALWAYSEXP1)) || (act==1 && (it[in].flags&IF_ALWAYSEXP2))) {
                        it[in].current_age[act]++;
                        if (it[in].flags&IF_LIGHTAGE) lightage(in,1);
                        if (item_age(in)) {
                                must_update = 1;
                                if (it[in].damage_state==5) {
                                        age_message(cn, in, "you were using");
                                        ch[cn].citem=0;
                                        it[in].used=USE_EMPTY;
                                } else {
                                        age_message(cn, in, "you are using");
                                }
                        }
                }
        }
        if (must_update) do_update_char(cn);
}

int may_deactivate(int in)
{
        int n,m,in2;

        if (it[in].driver!=1) return 1;

        for (n=1; n<10; n++) {
                if ((m=it[in].data[n])==0) return 1;
                if ((in2=map[m].it)==0) return 0;
                if (it[in2].driver!=26) return 0;
        }
        return 1;
}



void pentagram(int in)
{
        int n,cn;

        if (it[in].active) return;
        if (RANDOM(18)) return;

        for (n=1; n<4; n++) {
                if ((cn=it[in].data[n])==0 || ch[cn].data[0]!=in || ch[cn].used==USE_EMPTY || (ch[cn].flags&CF_BODY)) {
                        it[in].data[n]=spawn_penta_enemy(in);
                        break;
                }
        }
}

void spiderweb(int in)
{
        int n,cn;

        if (it[in].active) return;
        if (RANDOM(60)) return;

        for (n=1; n<4; n++) {
                if ((cn=it[in].data[n])==0 || ch[cn].data[0]!=in || ch[cn].used==USE_EMPTY || (ch[cn].flags&CF_BODY)) {
                        cn=pop_create_char(390+RANDOM(3),0);
                        if (!cn) continue;
                        ch[cn].flags&=~CF_RESPAWN;
                        ch[cn].data[0]=in;
                        ch[cn].data[29]=it[in].x+it[in].y*MAPX;
                        ch[cn].data[60]=TICKS*60*2;
                        ch[cn].data[73]=8;
                        ch[cn].dir=1;
                        if (!god_drop_char_fuzzy(cn,it[in].x,it[in].y)) {
                                god_destroy_items(cn);
                                ch[cn].used=USE_EMPTY;
                        } else it[in].data[n]=cn;
                        break;
                }
        }
}

void greenlingball(int in)
{
        int n,cn;

        if (it[in].active) return;
        if (RANDOM(20)) return;

        for (n=1; n<4; n++) {
                if ((cn=it[in].data[n])==0 || ch[cn].data[0]!=in || ch[cn].used==USE_EMPTY || (ch[cn].flags&CF_BODY)) {
                        cn=pop_create_char(553+it[in].data[0],0);
                        if (!cn) continue;
                        ch[cn].flags&=~CF_RESPAWN;
                        ch[cn].data[0]=in;
                        ch[cn].data[29]=it[in].x+it[in].y*MAPX;
                        ch[cn].data[60]=TICKS*60*2;
                        ch[cn].data[73]=8;
                        ch[cn].dir=1;
                        if (!god_drop_char_fuzzy(cn,it[in].x,it[in].y)) {
                                god_destroy_items(cn);
                                ch[cn].used=USE_EMPTY;
                        } else it[in].data[n]=cn;
                        break;
                }
        }
}

void expire_blood_penta(int in)
{
        if (it[in].data[0]) {
                it[in].data[0]++;
                if (it[in].data[0]>7) it[in].data[0]=0;
                it[in].sprite[0]=it[in].data[1]+it[in].data[0];
        }
}

void expire_driver(int in)
{
        switch(it[in].driver) {
                case    49:     expire_blood_penta(in); break;
                default:        xlog("unknown expire driver %d for item %s (%d)\n",
                                        it[in].driver,it[in].name,in);
                                break;
        }
}

#define EXP_TIME        (MAPY/4)

void item_tick_expire(void)
{
        static int y=0;
        int x,in,m,act,cn;

        for (x=0,m=y*MAPX; x<MAPX; x++,m++) {
                if ((in=map[m].it)!=0) {
                        if ((it[in].flags&IF_REACTIVATE) && !it[in].active) {
                                if (!map[m].ch && !map[m].to_ch) {
                                        it[in].active=it[in].duration;
                                        if (it[in].light[0]!=it[in].light[1])
                                                do_add_light(x,y,it[in].light[1]-it[in].light[0]);
                                }
                        }
                        // active and expire
                        if (it[in].active && it[in].active!=0xffffffff) {
                                if (it[in].active<=EXP_TIME) {
                                        if (may_deactivate(in) && !map[m].ch && !map[m].to_ch) {
                                                use_driver(0,in,0);
                                                it[in].active=0;
                                                if (it[in].light[0]!=it[in].light[1])
                                                        do_add_light(x,y,it[in].light[0]-it[in].light[1]);
                                        }
                                } else  it[in].active-=EXP_TIME;
                        }

                        // legacy drivers, replace by IF_EXPIREPROC!
                        if (it[in].driver==33) pentagram(in);
                        if (it[in].driver==43) spiderweb(in);
                        if (it[in].driver==56) greenlingball(in);

                        if (it[in].flags&IF_EXPIREPROC) expire_driver(in);

                        if (!(it[in].flags&IF_TAKE) && it[in].driver!=7) goto noexpire;
                        if ((map[m].flags&MF_NOEXPIRE) && it[in].driver!=7) goto noexpire;      // yuck!
                        if (it[in].driver==37) goto noexpire;

                        if (it[in].flags&IF_NOEXPIRE) goto noexpire;

                        if (it[in].active) act=1;
                        else act=0;

                        it[in].current_age[act]+=EXP_TIME;      // each place is only checked every MAPY ticks
                                                                // so we add MAPY instead of one

                        if (it[in].flags&IF_LIGHTAGE) lightage(in,EXP_TIME);


                        if (item_age(in) && it[in].damage_state==5) {
                                if (it[in].light[act]) do_add_light(x,y,-it[in].light[act]);
                                map[m].it=0;
                                it[in].used=USE_EMPTY;
                                globs->expire_cnt++;

                                if (it[in].driver==7) { // tomb
                                        int co,temp;

                                        co=it[in].data[0];
                                        temp=ch[co].temp;

                                        god_destroy_items(co);
                                        ch[co].used=USE_EMPTY;

                                        if (temp && (ch[co].flags&CF_RESPAWN)) {
						if (temp==189 || temp==561) {
							fx_add_effect(2,TICKS*60*20+RANDOM(TICKS*60*5),ch_temp[temp].x,ch_temp[temp].y,temp);
						} else {
							fx_add_effect(2,TICKS*60*1+RANDOM(TICKS*60*1),ch_temp[temp].x,ch_temp[temp].y,temp);
						}
						xlog("respawn %d (%s): YES",co,ch[co].name);
					} else xlog("respawn %d (%s): NO",co,ch[co].name);
                                }
                        }
                }

                noexpire:
                // checker
                if ((cn=map[m].ch)!=0) {
                        if (ch[cn].x!=x || ch[cn].y!=y || ch[cn].used!=USE_ACTIVE) {
                                xlog("map[%d,%d].ch reset from %d (%s) to 0",x,y,cn,ch[cn].reference);
                                map[m].ch=0;
                                globs->lost_cnt++;
                        }
                }
                if ((cn=map[m].to_ch)!=0) {
                        if (ch[cn].tox!=x || ch[cn].toy!=y || ch[cn].used!=USE_ACTIVE) {
                                xlog("map[%d,%d].to_ch reset from %d (%s) to 0",x,y,cn,ch[cn].reference);
                                map[m].to_ch=0;
                                globs->lost_cnt++;
                        }
                }
                if ((in=map[m].it)!=0) {
                        if (it[in].x!=x || it[in].y!=y || it[in].used!=USE_ACTIVE) {
                                xlog("map[%d,%d].it reset from %d (%s) to 0",x,y,in,it[in].reference);
                                map[m].it=0;
                                globs->lost_cnt++;
                        }
                }
        }

        y++; if (y>=MAPY) { globs->expire_run++; globs->lost_run++; y=0; }
}

void item_tick_gc(void)
{
        static int off=0,cnt=0;
        int n,m,cn,z,in2;

        m=min(off+256,MAXITEM);

        for (n=off; n<m; n++) {
                if (it[n].used==USE_EMPTY) continue;
                cnt++;

                // hack: make reset seyan swords unuable
                if (it[n].driver==40 && !it[n].data[0]) {
                        struct item tmp;

                        tmp=it_temp[683];
                        tmp.x=it[n].x;
                        tmp.y=it[n].y;
                        tmp.carried=it[n].carried;
                        tmp.temp=683;
                        it[n]=tmp;
                        it[n].flags|=IF_UPDATE;

                        cn=it[n].carried;

                        if (cn) do_update_char(cn);

                        xlog("reset sword from %s (%d)",ch[cn].name,cn);
                }

                if ((cn=it[n].carried)!=0) {
                        if (IS_SANECHAR(cn) && ch[cn].used) {
                                for (z=0; z<40; z++)
                                        if (ch[cn].item[z]==n) break;
                                if (z<40) continue;
                                for (z=0; z<20; z++) {
                                        if (ch[cn].worn[z]==n) break;
                                        if (ch[cn].spell[z]==n) break;
                                }
                                if (z<20) continue;
                                if (ch[cn].citem==n) continue;
                                if (ch[cn].flags&CF_PLAYER) {
                                        for (z=0; z<62; z++)
                                                if (ch[cn].depot[z]==n) break;
                                        if (z<62) continue;
                                }
                        }
                } else {
                        in2=map[it[n].x+it[n].y*MAPX].it;
                        if (in2==n) continue;
                }
                xlog("Garbage: Item %d (%s) (%d, %d,%d)",n,it[n].reference,it[n].carried,it[n].x,it[n].y);
                it[n].used=USE_EMPTY;
                globs->gc_cnt++;
        }
        off+=256;
        if (off>=MAXITEM) {
                off=0;
                globs->item_cnt=cnt;
                globs->gc_run++;
                cnt=0;
        }
}

void item_tick(void)
{
        item_tick_expire(); item_tick_expire(); item_tick_expire(); item_tick_expire();
        item_tick_gc();
}

void trap1(int cn,int in)
{
        int n;

        if ((n=it[in].data[1])) {
                in=map[n].it;
                if (!in || it[in].active || it[in].data[0]) {
                        do_char_log(cn,0,"You stepped on a trap, but nothing happened!\n");
                        return;
                }
        }

        n=RANDOM(12);
        in=ch[cn].worn[n];

        if (in) {
                do_char_log(cn,0,"You triggered an acid attack. Your %s desintegrated.\n",it[in].name);
                chlog(cn,"Stepped on Acid Trap, %s (t=%d) vanished",it[in].name,it[in].temp);
                it[in].used=USE_EMPTY;
                ch[cn].worn[n]=0;
                do_update_char(cn);
        } else {
                do_char_log(cn,0,"You triggered an acid attack, but it hit only your skin.\n");
                chlog(cn,"Stepped on Acid Trap");
                do_hurt(0,cn,350,0);
        }
}

void trap2(int cn,int tmp)
{
        int cc;

        cc=pop_create_char(tmp,0);
        if (!god_drop_char_fuzzy(cc,ch[cn].x,ch[cn].y)) {
                xlog("drop failed");
                ch[cc].used=USE_EMPTY;
                return;
        }
        do_update_char(cc);
        ch[cc].attack_cn=cn;
}

/* traps:
   data[0] = type
*/

void start_trap(int cn,int in)
{
        if (it[in].duration) {
                it[in].active=it[in].duration;
                if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
                        do_add_light(it[in].x,it[in].y,it[in].light[1]-it[in].light[0]);
        }

        switch(it[in].data[0]) {
                case    0:      chlog(cn,"Stepped on Arrow Trap");
                                do_char_log(cn,0,"You feel a sudden pain!\n");
                                do_hurt(0,cn,250,0);
                                break;

                case    1:      chlog(cn,"Stepped on Attack Trigger Trap");
                                do_char_log(cn,0,"You hear a loud croaking noise!\n");
                                do_area_notify(cn,0,it[in].x,it[in].y,NT_HITME,cn,0,0,0);
                                break;

                case    2:      trap1(cn,in); break;
                case    3:      trap2(cn,323); break;
                case    4:      trap2(cn,324); break;
                default:        do_char_log(cn,0,"Phew. Must be your lucky day today.\n"); break;
        }
}

int step_trap(int cn,int in)
{
        if (ch[cn].flags&(CF_PLAYER)) start_trap(cn,in);
        else do_char_log(cn,0,"You stepped on a trap. Fortunately, nothing happened.\n");

        return 0;
}

void step_trap_remove(int cn,int in)
{
        if (it[in].active) {
                it[in].active=0;
                if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
                        do_add_light(it[in].x,it[in].y,it[in].light[0]-it[in].light[1]);
        }
}

int step_portal1_lab13(int cn,int in)
{
        int flag=0;
        int n;

        if (!(ch[cn].kindred&KIN_HARAKIM) && !(ch[cn].kindred&KIN_TEMPLAR) && !(ch[cn].kindred&KIN_MERCENARY) &&
            !(ch[cn].kindred&KIN_SORCERER) && !(ch[cn].kindred&KIN_WARRIOR)) {
                do_char_log(cn,0,"This portal opens only for Harakim, Templars, Mercenaries, Warrior and Sorcerers.\n");
                return -1;
        }

        if (ch[cn].citem) flag=1;

        for (n=0; n<40 && !flag; n++)
                if (ch[cn].item[n]) flag=1;

        for (n=0; n<20 && !flag; n++)
                if (ch[cn].worn[n]) flag=1;

        if (flag) {
                do_char_log(cn,0,"You may not pass unless you leave all your items behind.\n");
                return -1;
        }

        for (n=0; n<20; n++) {
                if ((in=ch[cn].spell[n])!=0) {
                        it[in].used=USE_EMPTY;
                        ch[cn].spell[n]=0;
                }
        }
        do_update_char(cn);

        return 1;
}

int step_portal2_lab13(int cn,int in)
{
        int x,y,co,n,flag=0,in2;

        if (!IS_PLAYER(cn)) return -1;

        for (x=48; x<=80 && !flag; x++) {
                for (y=594; y<=608 && !flag; y++) {
                        if ((co=map[x+y*MAPX].ch)!=0 && co!=cn && (ch[co].flags&(CF_PLAYER))) flag=1;
                        if ((in2=map[x+y*MAPX].it)!=0 && (it[in2].temp==664 || it[in2].temp==170)) flag=2;
                }
        }

        for (x=38; x<=48 && !flag; x++) {
                for (y=593; y<=602 && !flag; y++) {
                        if ((co=map[x+y*MAPX].ch)!=0 && co!=cn && (ch[co].flags&(CF_PLAYER))) flag=1;
                        if ((in2=map[x+y*MAPX].it)!=0 && (it[in2].temp==664 || it[in2].temp==170)) flag=2;
                }
        }

        if (flag==2) {
                do_char_log(cn,0,"The Final Test is waiting for a certain item to expire, please try again later.\n");
                return -1;
        }

        if (flag) {
                do_char_log(cn,0,"You may not pass while another player is inside.\n");
                return -1;
        }

        for (n=flag=0; n<MAXCHARS; n++) {
                if (ch[n].used!=USE_ACTIVE || (ch[n].flags&CF_BODY)) continue;
                if (ch[n].temp!=51) continue;
                if (ch[n].a_hp>ch[n].hp[5]*900 && ch[n].a_mana>ch[n].mana[5]*900) flag=1;
                break;
        }
        if (!flag) {
                do_char_log(cn,0,"The Gatekeeper is currently busy. Please try again in a few minutes.\n");
                return -1;
        }

        if (!it[15220].data[1]) {
                do_char_log(cn,0,"The doors aren't closed again yet. Please try again in a few minutes.\n");
                return -1;
        }

        for (n=0; n<20; n++) {
                if ((in=ch[cn].spell[n])!=0) {
                        it[in].used=USE_EMPTY;
                        ch[cn].spell[n]=0;
                }
        }

        for (n=0; n<40; n++) {
                if ((in2=ch[cn].item[n])!=0 && it[in2].temp==664) {
                        ch[cn].item[n]=0;
                        it[in2].used=USE_EMPTY;
                }
        }
        if ((in2=ch[cn].citem)!=0 && !(in2&0x80000000) && it[in2].temp==664) {
                ch[cn].citem=0;
                it[in2].used=USE_EMPTY;
        }
        do_update_char(cn);

        return 1;
}

int step_portal_arena(int cn,int in)
{
        int nr,co,in2,n,flag=0;
        int xs,ys,xe,ye,x,y;

        if ((in2=ch[cn].citem) && !(in2&0x80000000) && it[in2].temp==687) {
                ch[cn].citem=0;
                it[in2].used=USE_EMPTY;
                flag=1;
        }

        for (n=0; n<40; n++) {
                if ((in2=ch[cn].item[n])!=0 && it[in2].temp==687) {
                        flag=1;
                        ch[cn].item[n]=0;
                        it[in2].used=USE_EMPTY;
                }
        }

        if (flag) {
                do_char_log(cn,1,"A winner! You gain one arena-rank!\n");
                ch[cn].data[22]++;
                ch[cn].data[23]=1;
                return 1;
        }

        nr=ch[cn].data[22];
        nr+=364;
        if (nr>381) {
                do_char_log(cn,1,"Please tell the gods to add more potent monsters to the arena.\n");
                return -1;
        }

        xs=it[in].data[1]%MAPX;
        ys=it[in].data[1]/MAPX;
        xe=it[in].data[2]%MAPX;
        ye=it[in].data[2]/MAPX;

        if (ch[cn].frx>=xs && ch[cn].frx<=xe && ch[cn].fry>=ys && ch[cn].fry<=ye) {
                do_char_log(cn,1,"You forfeit this fight.\n");
                return 1;
        }

        for (x=xs; x<=xe; x++) {
                for (y=ys; y<=ye; y++) {
                        if (map[x+y*MAPX].ch) {
                                do_char_log(cn,1,"The arena is busy. Please come back later.\n");
                                return -1;
                        }
                }
        }

        co=pop_create_char(nr,0);
        if (!god_drop_char_fuzzy(co,it[in].data[0]%MAPX,it[in].data[0]/MAPX)) {
                do_char_log(cn,1,"Please tell the gods that the arena isn't working.\n");
                return -1;
        }

        ch[co].data[64]=globs->ticker+TICKS*60*5;

        in=god_create_item(687);
        god_give_char(in,co);

        ch[cn].data[23]=0;

        return 1;
}

/* CS, 991127: Teleport tile */
int step_teleport(int cn, int in)
{
        int m, x, y, j, m2, m3;
        static int loc_off[] = { 0, -MAPX, MAPX, 1, -1 };

        if (cn <= 0) {
                xlog("step_teleport(): cn = %d", cn);
                return -1;
        }
        x = it[in].data[0];
        y = it[in].data[1];
        if (!SANEXY(x, y)) {
                xlog("step_teleport(): bad coordinates in item %d", in);
                return -1;
        }
        m = XY2M(x, y);
        m3 = 0;
        /* check for unoccupied landing spot */
        for (j=0; j<ARRAYSIZE(loc_off); j++) {
                m2 = m + loc_off[j];
                if (map[m2].flags&MF_MOVEBLOCK) continue;
                if (map[m2].ch) continue;
                if (map[m2].to_ch) continue;
                if ((in=map[m2].it)!=0 && (it[in].flags&IF_MOVEBLOCK)) continue;
                if (map[m2].flags&(MF_TAVERN|MF_DEATHTRAP)) continue;
                m3 = m2; break;
        }
        if (m3 == 0) {
                // target occupied: fail silently
                return -1;
        }
        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        plr_map_remove(cn);

        /* mostly copied from plr_map_set() */
        ch[cn].status=0;
        ch[cn].attack_cn=0;
        ch[cn].skill_nr=0;
        ch[cn].goto_x=0;


        // instead of plr_map_set(cn);
        map[m3].ch=cn;
        map[m3].to_ch=0;
        ch[cn].x = m3 % MAPX;
        ch[cn].y = m3 / MAPX;
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        return 2; // TELEPORT_SUCCESS
}

int step_firefloor(int cn,int in)
{
	int in2;
	
	do_char_log(cn,0,"Outch!\n");
	
	in2=god_create_item(1);
	
	strcpy(it[in2].name,"Fire");
	strcpy(it[in2].reference,"fire");
	strcpy(it[in2].description,"Fire.");
	
	it[in2].hp[0]=-5000;
	it[in2].active=it[in2].duration=1;
	it[in2].flags=IF_SPELL|IF_PERMSPELL;	
	it[in2].temp=it[in].temp;
	it[in2].sprite[1]=it[in].sprite[1];
	
	add_spell(cn,in2);
	
	return 0;
}

void step_firefloor_remove(int cn,int in)
{
	int n,in2;
	
	for (n=0; n<20; n++)
		if ((in2=ch[cn].spell[n]) && it[in2].temp==it[in].temp) break;
	if (n==20) return;
	
	it[in2].used=USE_EMPTY;
	ch[cn].spell[n]=0;
}

int step_driver(int cn,int in)
{
        int ret=0;

        switch(it[in].driver) {
                case    36:     ret=step_portal1_lab13(cn,in); break;
                case    37:     ret=step_trap(cn,in); break;
                case    38:     ret=step_portal2_lab13(cn,in); break;
                case    47:     ret=step_portal_arena(cn,in); break;
                case    62:     ret=step_teleport(cn,in); break;
                case	69:	ret=step_firefloor(cn,in); break;
                default:        xlog("unknown step driver %d for item %s (%d)",it[in].driver,it[in].name,in); break;
        }

        return ret;
}

void step_driver_remove(int cn,int in)
{
        switch(it[in].driver) {
                case    36:     break;
                case    37:     step_trap_remove(cn,in); break;
                case    38:     break;
                case    47:     break;
                case    62:     break;
                case	69:	step_firefloor_remove(cn,in); break;
                default:        xlog("unknown step driver %d for item %s (%d)",it[in].driver,it[in].name,in); break;
        }
}
