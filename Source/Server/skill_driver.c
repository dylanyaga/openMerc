/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

int friend_is_enemy(int cn,int cc)
{
        int co;

        if (!(co=ch[cn].attack_cn)) return 0;
        if (may_attack_msg(cc,co,0)) return 0;
        return 1;
}

int player_or_ghost(int cn,int co)
{
	if (!(ch[cn].flags&CF_PLAYER)) return 1;
	if (ch[co].flags&CF_PLAYER) return 1;
	if (ch[co].data[63]==cn) return 1;
	
	return 0;
}

int spellcost(int cn,int cost)
{
	// concentrate:
	if (ch[cn].skill[SK_CONCEN][0]) {
		int t;
		
		t=cost*ch[cn].skill[SK_CONCEN][5]/300;
		if (t>cost) cost=1;
		else cost-=t;
		
	}
        if (cost*1000>ch[cn].a_mana) {
                do_char_log(cn,0,"You don't have enough mana.\n");
                return -1;
        }
        ch[cn].a_mana-=cost*1000;
        return 0;
}

int chance_base(int cn,int skill,int d20,int power)
{
        int chance;

        chance=d20*skill/max(1,power);

        if (ch[cn].flags&(CF_PLAYER)) if (ch[cn].luck<0) chance+=ch[cn].luck/500-1;

        if (chance<0) chance=0;
        if (chance>18) chance=18;

        if (RANDOM(20)>chance || power>skill+(skill/2)) {
                do_char_log(cn,0,"You lost your focus!\n");
                return -1;
        }
        return 0;
}

int chance(int cn,int d20)
{
        if (ch[cn].flags&(CF_PLAYER)) if (ch[cn].luck<0) d20+=ch[cn].luck/500-1;

        if (d20<0) d20=0;
        if (d20>18) d20=18;

        if (RANDOM(20)>d20) {
                do_char_log(cn,0,"You lost your focus!\n");
                return -1;
        }
        return 0;
}

int spell_immunity(int power,int immun)
{
        immun/=2;
        if (power<=immun) return 1;
        else return power-immun;
}

int spell_race_mod(int power,int kindred)
{
        double mod;

	if (kindred&KIN_ARCHHARAKIM) mod=1.05;
	else if (kindred&KIN_ARCHTEMPLAR) mod=0.95;
	else if (kindred&KIN_SORCERER) mod=1.10;
	else if (kindred&KIN_WARRIOR) mod=1.10;
	else if (kindred&KIN_SEYAN_DU) mod=0.95;
	else if (kindred&KIN_HARAKIM) mod=1.00;
	else if (kindred&KIN_MERCENARY) mod=1.05;
	else if (kindred&KIN_TEMPLAR) mod=0.90;
	else mod=1.00; 	

	if (globs->newmoon) mod-=0.15;
	if (globs->fullmoon) mod+=0.15;

        return (int)(power*mod);
}

int add_spell(int cn,int in)
{
        int n,in2,weak=999,weakest=99;
        int m;

        m=ch[cn].x+ch[cn].y*MAPX;
        if (map[m].flags&CF_NOMAGIC) return 0;

        // overwrite spells if same spell is cast twice and the new spell is more powerfull
        for (n=0; n<20; n++)
                if ((in2=ch[cn].spell[n])!=0)
                        if (it[in2].temp==it[in].temp) {
                                if (it[in].power<it[in2].power && it[in2].active>TICKS*60) { it[in].used=USE_EMPTY; return 0; }
                                it[in2].used=USE_EMPTY;
                                break;
                        }

        if (n==20) {
                for (n=0; n<20; n++) {
                        if (!(in2=ch[cn].spell[n])) break;
                        if (it[in2].power<weak) { weak=it[in2].power; weakest=n; }
                }
                if (n==20) {    // overwrite weakest spell if it is weaker than the new spell
                        if (weak<999 && weak<it[in].power) {
                                n=weakest;
                                if ((in2=ch[cn].spell[n])!=0) it[in2].used=USE_EMPTY;
                        } else { it[in].used=USE_EMPTY; return 0; }
                }
        }

        ch[cn].spell[n]=in;
        it[in].carried=cn;

        do_update_char(cn);

        return 1;
}

int is_exhausted(int cn)
{
        int n,in;

        for (n=0; n<20; n++)
                if ((in=ch[cn].spell[n])!=0 && it[in].temp==SK_BLAST) break;
        if (n<20) {
                do_char_log(cn,0,"You are still exhausted from your last spell!\n");
                return 1;
        }
        return 0;
}

void add_exhaust(int cn,int len)
{
        int in;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in add_exhaust"); return; }

        strcpy(it[in].name,"Spell Exhaustion");
        it[in].flags|=IF_SPELL;
        it[in].sprite[1]=97;
        it[in].duration=it[in].active=len;
        it[in].temp=SK_BLAST;
        it[in].power=255;

        add_spell(cn,in);
}

void spell_from_item(int cn,int in2)
{
        int in,n;

        if (ch[cn].flags&CF_NOMAGIC) {
                do_char_log(cn,0,"The magic didn't work! Must be external influences.\n");
                return;
        }

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_from_item"); return; }

        strcpy(it[in].name,it[in2].name);
        it[in].flags|=IF_SPELL;

        it[in].armor[1]=it[in2].armor[1];
        it[in].weapon[1]=it[in2].weapon[1];

        it[in].hp[1]=it[in2].hp[1];
        it[in].end[1]=it[in2].end[1];
        it[in].mana[1]=it[in2].mana[1];
        it[in].sprite_override=it[in2].sprite_override;

        for (n=0; n<5; n++)
                it[in].attrib[n][1]=it[in2].attrib[n][1];
        for (n=0; n<50; n++)
                it[in].skill[n][1]=it[in2].skill[n][1];

        if (it[in2].data[0]) it[in].sprite[1]=it[in2].data[0];
        else it[in].sprite[1]=93;
        it[in].duration=it[in].active=it[in2].duration;
        if (it[in2].data[1]) it[in].temp=it[in2].data[1];
        else it[in].temp=101;
        it[in].power=it[in2].power;

        if (!add_spell(cn,in)) {
                do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                return;
        }
        do_char_log(cn,1,"You feel changed.\n");

        char_play_sound(cn,ch[cn].sound+1,-150,0);
}

int spell_light(int cn,int co,int power)
{
        int in;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_light"); return 0; }

	power=spell_race_mod(power,ch[cn].kindred);
        strcpy(it[in].name,"Light");
        it[in].flags|=IF_SPELL;
        it[in].light[1]=min(250,power*4);
        it[in].sprite[1]=85;
        it[in].duration=it[in].active=18*60*30;
        it[in].temp=SK_LIGHT;
        it[in].power=power;

        if (cn!=co) {
                if (!add_spell(co,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                if (ch[co].skill[SK_SENSE][5]+10>power)
                        do_char_log(co,1,"%s cast light on you.\n",ch[cn].reference);
                else do_char_log(co,1,"You start to emit light.\n");
                do_area_log(co,0,ch[co].x,ch[co].y,2,"%s starts to emit light.\n",ch[co].name);
                char_play_sound(co,ch[cn].sound+1,-150,0);
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                chlog(cn,"Cast Light on %s",ch[co].name);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
        } else {
                if (!add_spell(cn,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                do_char_log(cn,1,"You start to emit light.\n");
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Cast Light");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        return 1;
}

void skill_light(int cn)
{
        int co;

	if (ch[cn].flags&CF_PLAYER) {
		ch[cn].data[71]+=CNTSAY;
		if (ch[cn].data[71]>MAXSAY) {
			do_char_log(cn,0,"Oops, you're a bit too fast for me!\n");
			return;
		}
	}

        if ((co=ch[cn].skill_target1)) ;
        else co=cn;

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        if (is_exhausted(cn)) return;

        if (spellcost(cn,5)) return;
        if (chance(cn,18)) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_LIGHT][5]+5)
                        do_char_log(co,1,"%s tried to cast light on you but failed.\n",ch[cn].reference);
                return;
        }

        spell_light(cn,co,ch[cn].skill[SK_LIGHT][5]);

        add_exhaust(cn,TICKS/4);
}

int spellpower(int cn)
{
        return ch[cn].attrib[AT_AGIL][0]+ch[cn].attrib[AT_STREN][0]+ch[cn].attrib[AT_INT][0]+ch[cn].attrib[AT_WILL][0]+ch[cn].attrib[AT_BRAVE][0];
}

int spell_protect(int cn,int co,int power)
{
        int in,tmp;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_protect"); return 0; }

        if (power>(tmp=spellpower(co))) {
                if (cn!=co) {
                        do_char_log(cn,1,"Seeing that %s is not powerful enough for your spell, you reduced its strength.\n",ch[co].reference);
                } else {
                        do_char_log(cn,1,"You are not powerful enough to use the full strength of this spell.\n");
                }
                power=tmp;
        }

	power=spell_race_mod(power,ch[cn].kindred);
        strcpy(it[in].name,"Protection");
        it[in].flags|=IF_SPELL;
        it[in].armor[1]=power/4+4;
        it[in].sprite[1]=86;
        it[in].duration=it[in].active=18*60*10;
        it[in].temp=SK_PROTECT;
        it[in].power=power;

        if (cn!=co) {
                if (!add_spell(co,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                if (ch[co].skill[SK_SENSE][5]+10>power)
                        do_char_log(co,1,"%s cast protect on you.\n",ch[cn].reference);
                else do_char_log(co,0,"You feel protected.\n");
                do_char_log(cn,1,"%s is now protected.\n",ch[co].name);
                char_play_sound(co,ch[cn].sound+1,-150,0);
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                chlog(cn,"Cast Protect on %s",ch[co].name);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
        } else {
                if (!add_spell(cn,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                do_char_log(cn,1,"You feel protected.\n");
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Cast Protect");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        return 1;
}

void skill_protect(int cn)
{
        int co;

        if (!ch[cn].skill[SK_PROTECT][5]) return;

        if ((co=ch[cn].skill_target1)) ;
        else co=cn;

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        if (is_exhausted(cn)) return;

        if (!player_or_ghost(cn,co)) {
                do_char_log(cn,0,"Changed target of spell from %s to %s.\n",ch[co].name,ch[cn].name);
                co=cn;
        }

        if (spellcost(cn,15)) return;
        if (chance(cn,18)) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_PROTECT][5]+5)
                        do_char_log(co,1,"%s tried to cast protect on you but failed.\n",ch[cn].reference);
                return;
        }

        spell_protect(cn,co,ch[cn].skill[SK_PROTECT][5]);

        add_exhaust(cn,TICKS/2);
}

int spell_enhance(int cn,int co,int power)
{
        int in,tmp;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_enhance"); return 0; }

        if (power>(tmp=spellpower(co))) {
                if (cn!=co) {
                        do_char_log(cn,1,"Seeing that %s is not powerful enough for your spell, you reduced its strength.\n",ch[co].reference);
                } else {
                        do_char_log(cn,1,"You are not powerful enough to use the full strength of this spell.\n");
                }
                power=tmp;
        }

	power=spell_race_mod(power,ch[cn].kindred);
        strcpy(it[in].name,"Enhance Weapon");
        it[in].flags|=IF_SPELL;
        it[in].weapon[1]=power/4+4;
        it[in].sprite[1]=87;
        it[in].duration=it[in].active=18*60*10;
        it[in].temp=SK_ENHANCE;
        it[in].power=power;

        if (cn!=co) {
                if (!add_spell(co,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                if (ch[co].skill[SK_SENSE][5]+10>power)
                        do_char_log(co,1,"%s cast enhance weapon on you.\n",ch[cn].reference);
                else do_char_log(co,0,"Your weapon feels stronger.\n");
                do_char_log(cn,1,"%s's weapon is now stronger.\n",ch[co].name);
                char_play_sound(co,ch[cn].sound+1,-150,0);
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                chlog(cn,"Cast Enhance on %s",ch[co].name);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
        } else {
                if (!add_spell(cn,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                do_char_log(cn,1,"Your weapon feels stronger.\n");
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Cast Enhance");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        return 1;
}

void skill_enhance(int cn)
{
        int co;

        if ((co=ch[cn].skill_target1)) ;
        else co=cn;

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        if (is_exhausted(cn)) return;

        if (!player_or_ghost(cn,co)) {
                do_char_log(cn,0,"Changed target of spell from %s to %s.\n",ch[co].name,ch[cn].name);
                co=cn;
        }

        if (spellcost(cn,15)) return;
        if (chance(cn,18)) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_ENHANCE][5]+5)
                        do_char_log(co,1,"%s tried to cast enhance weapon on you but failed.\n",ch[cn].reference);
                return;
        }

        spell_enhance(cn,co,ch[cn].skill[SK_ENHANCE][5]);

        add_exhaust(cn,TICKS/2);
}

int spell_bless(int cn,int co,int power)
{
        int in,n,tmp;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_bless"); return 0; }

        if (power>(tmp=spellpower(co))) {
                if (cn!=co) {
                        do_char_log(cn,1,"Seeing that %s is not powerful enough for your spell, you reduced its strength.\n",ch[co].reference);
                } else {
                        do_char_log(cn,1,"You are not powerful enough to use the full strength of this spell.\n");
                }
                power=tmp;
        }

	power=spell_race_mod(power,ch[cn].kindred);
        strcpy(it[in].name,"Bless");
        it[in].flags|=IF_SPELL;
        for (n=0; n<5; n++)
                it[in].attrib[n][1]=power/5+3;
        it[in].sprite[1]=88;
        it[in].duration=it[in].active=18*60*10;
        it[in].temp=SK_BLESS;
        it[in].power=power;

        if (cn!=co) {
                if (!add_spell(co,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                if (ch[co].skill[SK_SENSE][5]+10>power)
                        do_char_log(co,1,"%s cast bless on you.\n",ch[cn].reference);
                else do_char_log(co,0,"You have been blessed.\n");
                do_char_log(cn,1,"%s was blessed.\n",ch[co].name);
                char_play_sound(co,ch[cn].sound+1,-150,0);
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                chlog(cn,"Cast Bless on %s",ch[co].name);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
        } else {
                if (!add_spell(cn,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                do_char_log(cn,1,"You have been blessed.\n");
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Cast Bless");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        return 1;
}

void skill_bless(int cn)
{
        int co;

        if ((co=ch[cn].skill_target1)) ;
        else co=cn;

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        if (is_exhausted(cn)) return;

        if (!player_or_ghost(cn,co)) {
                do_char_log(cn,0,"Changed target of spell from %s to %s.\n",ch[co].name,ch[cn].name);
                co=cn;
        }

        if (spellcost(cn,35)) return;
        if (chance(cn,18)) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_BLESS][5]+5)
                        do_char_log(co,1,"%s tried to cast bless on you but failed.\n",ch[cn].reference);
                return;
        }

        spell_bless(cn,co,ch[cn].skill[SK_BLESS][5]);

        add_exhaust(cn,TICKS);
}

void skill_wimp(int cn)
{
        int in,n;

        for (n=0; n<20; n++) {
                if ((in=ch[cn].spell[n])!=0) {
                        if (it[in].temp==SK_WIMPY) {
                                do_char_log(cn,1,"Guardian Angel no longer active.\n");
                                it[in].used=USE_EMPTY;
                                ch[cn].spell[n]=0;
                                do_update_char(cn);
                                chlog(cn,"Removed Wimp");

                                return;
                        }
                }
        }

        if (ch[cn].a_end<20000) {
                do_char_log(cn,0,"You're too exhausted to call on your Guardian Angel.\n");
                return;
        }

        ch[cn].a_end-=20000;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_wimp"); return; }

        strcpy(it[in].name,"Guardian Angel");
        it[in].flags|=IF_SPELL|IF_PERMSPELL;
        it[in].hp[0]=-1;
        it[in].end[0]=-1;
        it[in].mana[0]=-1;
        it[in].sprite[1]=94;
        it[in].duration=it[in].active=18*60*60*2;
        it[in].temp=SK_WIMPY;
        it[in].power=ch[cn].skill[SK_WIMPY][5];

        if (!add_spell(cn,in))  {
                do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                return;
        }
        do_char_log(cn,1,"Guardian Angel active!\n");
        char_play_sound(cn,ch[cn].sound+1,-150,0);
        chlog(cn,"Cast Wimp");

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);
        fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);

//      ch[cn].errno=ERR_SUCCESS;
}

int spell_mshield(int cn,int co,int power)
{
        int in;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_mshield"); return 0; }

        strcpy(it[in].name,"Magic Shield");
        it[in].flags|=IF_SPELL;
        it[in].sprite[1]=95;
        //it[in].duration=it[in].active=spell_race_mod(ch[cn].skill[SK_MSHIELD][5]*256,ch[cn].kindred);
        it[in].duration=it[in].active=spell_race_mod(power*256,ch[cn].kindred);
        it[in].armor[1]=it[in].active/1024+1;
        it[in].temp=SK_MSHIELD;
        it[in].power=it[in].active/256;

        if (cn!=co) {
                if (!add_spell(co,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                if (ch[co].skill[SK_SENSE][5]+10>power)
                        do_char_log(co,1,"%s cast magic shield on you.\n",ch[cn].reference);
                else do_char_log(co,0,"Magic Shield active!\n");
                do_char_log(cn,1,"%s's Magic Shield activated.\n",ch[co].name);
                char_play_sound(co,ch[cn].sound+1,-150,0);
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                chlog(cn,"Cast Magic Shield on %s",ch[co].name);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
        } else {
                if (!add_spell(cn,in))  {
                        do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                        return 0;
                }
                do_char_log(cn,1,"Magic Shield active!\n");
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Cast Magic Shield");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        return 1;
}

void skill_mshield(int cn)
{
        if (is_exhausted(cn)) return;

        if (spellcost(cn,25)) return;
        if (chance(cn,18)) return;

        spell_mshield(cn,cn,ch[cn].skill[SK_MSHIELD][5]);

        add_exhaust(cn,TICKS*3);
}

int spell_heal(int cn,int co,int power)
{
        if (cn!=co) {
                ch[co].a_hp+=spell_race_mod(power*2500,ch[cn].kindred);
                if (ch[co].a_hp>ch[co].hp[5]*1000) ch[co].a_hp=ch[co].hp[5]*1000;
                if (ch[co].skill[SK_SENSE][5]+10>power)
                        do_char_log(co,1,"%s cast heal on you.\n",ch[cn].reference);
                else do_char_log(co,0,"You have been healed.\n");
                do_char_log(cn,1,"%s was healed.\n",ch[co].name);
                char_play_sound(co,ch[cn].sound+1,-150,0);
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                chlog(cn,"Cast Heal on %s",ch[co].name);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
        } else {
                ch[cn].a_hp+=power*2500;
                if (ch[co].a_hp>ch[co].hp[5]*1000) ch[co].a_hp=ch[co].hp[5]*1000;
                do_char_log(cn,1,"You have been healed.\n");
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                if (ch[cn].flags&(CF_PLAYER)) chlog(cn,"Cast Heal");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        return 1;
}

void skill_heal(int cn)
{
        int co;

        if ((co=ch[cn].skill_target1)) ;
        else co=cn;

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        if (is_exhausted(cn)) return;

        if (!player_or_ghost(cn,co)) {
                do_char_log(cn,0,"Changed target of spell from %s to %s.\n",ch[co].name,ch[cn].name);
                co=cn;
        }

        if (spellcost(cn,25)) return;
        if (chance(cn,18)) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_HEAL][5]+5)
                        do_char_log(co,1,"%s tried to cast heal on you but failed.\n",ch[cn].reference);
                return;
        }

        spell_heal(cn,co,ch[cn].skill[SK_HEAL][5]);

        add_exhaust(cn,TICKS*2);
}

int spell_curse(int cn,int co,int power)
{
        int in,n;

        if (ch[co].flags&CF_IMMORTAL) return 0;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in spell_curse"); return 0; }

        power=spell_immunity(power,ch[co].skill[SK_IMMUN][5]);

	power=spell_race_mod(power,ch[cn].kindred);
        strcpy(it[in].name,"Curse");
        it[in].flags|=IF_SPELL;
        for (n=0; n<5; n++)
                it[in].attrib[n][1]=-(power/3); //(power/3+10);
        it[in].sprite[1]=89;
        it[in].duration=it[in].active=18*60*2;
        it[in].temp=SK_CURSE;
        it[in].power=power;

        if (!add_spell(co,in))  {
                do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                return 0;
        }
        if (ch[co].skill[SK_SENSE][5]+10>power)
                do_char_log(co,1,"%s cast curse on you.\n",ch[cn].reference);
        else
                do_char_log(co,0,"You have been cursed.\n");
        do_char_log(cn,1,"%s was cursed.\n",ch[co].name);
        if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTHIT,cn,0,0,0);
        do_notify_char(cn,NT_DIDHIT,co,0,0,0);
        char_play_sound(co,ch[cn].sound+7,-150,0);
        char_play_sound(cn,ch[cn].sound+1,-150,0);
        chlog(cn,"Cast Curse on %s",ch[co].name);
        fx_add_effect(5,0,ch[co].x,ch[co].y,0);

        return 1;
}

void skill_curse(int cn)
{
        int co,co_orig,m;

        if ((co=ch[cn].skill_target1)) ;
        else if (ch[cn].attack_cn!=0) co=ch[cn].attack_cn;
        else co=cn;

        if (cn==co) { do_char_log(cn,0,"You cannot curse yourself!\n"); return; }

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        /* CS, 000209: Remember PvP attacks */
        remember_pvp(cn,co);
        if (is_exhausted(cn)) return;

        if (spellcost(cn,35)) return;

        if (!may_attack_msg(cn,co,1)) {
                chlog(cn,"Prevented from attacking %s (%d)",ch[co].name,co);
                return;
        }

        if (chance_base(cn,ch[cn].skill[SK_CURSE][5],10,ch[co].skill[SK_RESIST][5])) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_CURSE][5]+5) {
                        do_char_log(co,0,"%s tried to cast curse on you but failed.\n",ch[cn].reference);
                        if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTMISS,cn,0,0,0);
                }
                return;
        }

        if (ch[co].flags&CF_IMMORTAL) {
                do_char_log(cn,0,"You lost your focus.\n");
                return;
        }

        spell_curse(cn,co,ch[cn].skill[SK_CURSE][5]);

        co_orig=co;

        m=ch[cn].x+ch[cn].y*MAPX;
        if ((co=map[m+1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                if (ch[cn].skill[SK_CURSE][5]+RANDOM(20)>ch[co].skill[SK_RESIST][5]+RANDOM(20)) spell_curse(cn,co,ch[cn].skill[SK_CURSE][5]);
        }
        if ((co=map[m-1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                if (ch[cn].skill[SK_CURSE][5]+RANDOM(20)>ch[co].skill[SK_RESIST][5]+RANDOM(20)) spell_curse(cn,co,ch[cn].skill[SK_CURSE][5]);
        }
        if ((co=map[m+MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                if (ch[cn].skill[SK_CURSE][5]+RANDOM(20)>ch[co].skill[SK_RESIST][5]+RANDOM(20)) spell_curse(cn,co,ch[cn].skill[SK_CURSE][5]);
        }
        if ((co=map[m-MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                if (ch[cn].skill[SK_CURSE][5]+RANDOM(20)>ch[co].skill[SK_RESIST][5]+RANDOM(20)) spell_curse(cn,co,ch[cn].skill[SK_CURSE][5]);
        }

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        add_exhaust(cn,TICKS*4);

//      ch[cn].errno=ERR_SUCCESS;
}

int warcry(int cn,int co,int power)
{
	int n,in;
	
	// dont affect good NPCs unless they're a direct target
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) return  0;
	
	// dont attack those we may not attack anyway
	if (!may_attack_msg(cn,co,0)) return 0;
	
	// ignore those whose resistance is greater than our skill
	if (power<ch[co].skill[SK_RESIST][5]) return 0;
	
	// ignore those we grouped with
	for (n=1; n<10; n++) if (ch[cn].data[n]==co) return 0;
	
	// ignore the immortals
        if (ch[co].flags&CF_IMMORTAL) return 0;

        if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTHIT,cn,0,0,0);

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_warcry"); return 0; }

        strcpy(it[in].name,"War-Stun");
        it[in].flags|=IF_SPELL;
        it[in].sprite[1]=91;
        it[in].duration=it[in].active=TICKS*3;
        it[in].temp=SK_WARCRY2;
        it[in].power=power;

        add_spell(co,in);

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_warcry"); return 0; }

        strcpy(it[in].name,"Warcry");
        it[in].flags|=IF_SPELL;
        for (n=0; n<5; n++)	
        	it[in].attrib[n][1]=-15;
        it[in].sprite[1]=89;
        it[in].duration=it[in].active=18*60;
        it[in].temp=SK_WARCRY;
        it[in].power=power/2;

        add_spell(co,in);

        chlog(cn,"Cast Warcry on %s",ch[co].name);

        fx_add_effect(5,0,ch[co].x,ch[co].y,0);

        return 1;
}

void skill_warcry(cn)
{
	int power,xf,yf,xt,yt,x,y,co,hit=0,miss=0;
	
	if (ch[cn].a_end<150*1000) {
		do_char_log(cn,0,"You're too exhausted!\n");
		return;
	}
	
	ch[cn].a_end-=150*1000;
	
	power=ch[cn].skill[SK_WARCRY][5];
	
	xf=max(1,ch[cn].x-10);
	yf=max(1,ch[cn].y-10);
	xt=min(MAPX-1,ch[cn].x+10);
	yt=min(MAPY-1,ch[cn].y+10);
	
	for (x=xf; x<xt; x++) {
		for (y=yf; y<yt; y++) {
			if ((co=map[x+y*MAPX].ch)) {
				if (warcry(cn,co,power)) {
                                        /* CS, 000209: Remember PvP attacks */
                                        remember_pvp(cn,co);
					do_char_log(co,0,"You hear %s's warcry. You feel frightened and immobilized.\n",ch[cn].name);
					hit++;
				} else {
					do_char_log(co,0,"You hear %s's warcry.\n",ch[cn].name);
					miss++;
				}
			}
		}
	}
	do_char_log(cn,1,"You cry out loud and clear. You affected %d of %d creatures in range.\n",
		hit,hit+miss);
}

char *at_name[5]={
"Braveness",
"Willpower",
"Intuition",
"Agility",
"Strength"};

void item_info(int cn,int in,int look)
{
        int n;

        // if (!look) {
        do_char_log(cn,1,"%s:\n",it[in].name);
        // }
        do_char_log(cn,1,"Stat         Mod0 Mod1 Min\n");
        for (n=0; n<5; n++) {
                if (!it[in].attrib[n][0] && !it[in].attrib[n][1] && !it[in].attrib[n][2]) continue;
                do_char_log(cn,1,"%-12.12s %+4d %+4d %3d\n",
                        at_name[n],it[in].attrib[n][0],it[in].attrib[n][1],it[in].attrib[n][2]);
        }

        if (it[in].hp[0] || it[in].hp[1] || it[in].hp[2])
                do_char_log(cn,1,"%-12.12s %+4d %+4d %3d\n",
                        "Hitpoints",it[in].hp[0],it[in].hp[1],it[in].hp[2]);

        if (it[in].end[0] || it[in].end[1] || it[in].end[2])
                do_char_log(cn,1,"%-12.12s %+4d %+4d %3d\n",
                        "Endurance",it[in].end[0],it[in].end[1],it[in].end[2]);

        if (it[in].mana[0] || it[in].mana[1] || it[in].mana[2])
                do_char_log(cn,1,"%-12.12s %+4d %+4d %3d\n",
                        "Mana",it[in].mana[0],it[in].mana[1],it[in].mana[2]);

        for (n=0; n<50; n++) {
                if (!it[in].skill[n][0] && !it[in].skill[n][1] && !it[in].skill[n][2]) continue;
                do_char_log(cn,1,"%-12.12s %+4d %+4d %3d\n",
                        skilltab[n].name,it[in].skill[n][0],it[in].skill[n][1],it[in].skill[n][2]);
        }

        if (it[in].weapon[0] || it[in].weapon[1])
                do_char_log(cn,1,"%-12.12s %+4d %+4d\n",
                        "Weapon",it[in].weapon[0],it[in].weapon[1]);
        if (it[in].armor[0] || it[in].armor[1])
                do_char_log(cn,1,"%-12.12s %+4d %+4d\n",
                        "Armor",it[in].armor[0],it[in].armor[1]);
        if (it[in].light[0] || it[in].light[1])
                do_char_log(cn,1,"%-12.12s %+4d %+4d\n",
                        "Light",it[in].light[0],it[in].light[1]);

        if (it[in].power)
                do_char_log(cn,1,"%-12.12s %+4d",
                        "Power",it[in].power);

       if (it[in].min_rank)
       		do_char_log(cn,1,"%-12.12s %+4d",
       			"Min. Rank",it[in].min_rank);
}

void char_info(int cn,int co)
{
        int n,in,flag=0,n1=-1,n2=-1;

        do_char_log(cn,1,"%s:\n",ch[co].name);
        do_char_log(cn,1," \n");
        for (n=0; n<20; n++) {
                if ((in=ch[co].spell[n])!=0) {
                        do_char_log(cn,1,"%s for %dm %ds power of %d\n",
                                it[in].name,it[in].active/(18*60),(it[in].active/18)%60,it[in].power);
                        flag=1;
                }
        }
        if (!flag) do_char_log(cn,1,"No spells active.\n");
        do_char_log(cn,1," \n");

        for (n=0; n<50; n++) {
                if (ch[co].skill[n][0] && n1==-1) n1=n;
                else if (ch[co].skill[n][0] && n2==-1) n2=n;

                if (n1!=-1 && n2!=-1) {
                        do_char_log(cn,1,"%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
                                skilltab[n1].name,ch[co].skill[n1][0],ch[co].skill[n1][5],
                                skilltab[n2].name,ch[co].skill[n2][0],ch[co].skill[n2][5]);
                        n1=-1;
                        n2=-1;
                }
        }

        if (n1!=-1)
                do_char_log(cn,1,"%-12.12s %3d/%3d\n",
                        skilltab[n1].name,ch[co].skill[n1][0],ch[co].skill[n1][5]);

        do_char_log(cn,1,"%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
                at_name[0],ch[co].attrib[0][0],ch[co].attrib[0][5],
                at_name[1],ch[co].attrib[1][0],ch[co].attrib[1][5]);
        do_char_log(cn,1,"%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
                at_name[2],ch[co].attrib[2][0],ch[co].attrib[2][5],
                at_name[3],ch[co].attrib[3][0],ch[co].attrib[3][5]);
        do_char_log(cn,1,"%-12.12s %3d/%3d\n",
                at_name[4],ch[co].attrib[4][0],ch[co].attrib[4][5]);

        do_char_log(cn,1," \n");
}

void skill_identify(int cn)
{
        int co=0,in=0,power;

        if (is_exhausted(cn)) return;

        if (spellcost(cn,25)) return;

        if ((in=ch[cn].citem)!=0 && IS_SANEITEM(in)) power=it[in].power;
        else {
                if ((co=ch[cn].skill_target1)!=0) power=ch[co].skill[SK_RESIST][5];
                else { co=cn; power=10; }
                in=0;
        }

        if (chance_base(cn,ch[cn].skill[SK_IDENT][5],18,power)) return;

        char_play_sound(cn,ch[cn].sound+1,-150,0);
        chlog(cn,"Cast Identify");

        if (in) {
                item_info(cn,in,0);
                it[in].flags ^= IF_IDENTIFIED;
                if (!(it[in].flags & IF_IDENTIFIED)) {
                        do_char_log(cn, 1, "Identify data removed from item.\n");
                }
        } else {
                char_info(cn,co);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
        }

        add_exhaust(cn,TICKS*2);
        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

//      ch[cn].errno=ERR_SUCCESS;
}

void skill_blast(int cn)
{
        int co,dam,m,co_orig,tmp,power,cost;

        if ((co=ch[cn].skill_target1)) ;
        else if (ch[cn].attack_cn!=0) co=ch[cn].attack_cn;
        else co=cn;

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        if (cn==co) {
                do_char_log(cn,0,"You cannot blast yourself!\n");
                return;
        }

        if (ch[co].flags&CF_STONED) {
                do_char_log(cn,0,"Your target is lagging. Try again later.\n");
                return;
        }

        if (!may_attack_msg(cn,co,1)) {
                chlog(cn,"Prevented from attacking %s (%d)",ch[co].name,co);
                return;
        }

        /* CS, 000209: Remember PvP attacks */
        remember_pvp(cn,co);
        if (is_exhausted(cn)) return;

        power=ch[cn].skill[SK_BLAST][5];

        power=spell_race_mod(spell_immunity(power,ch[co].skill[SK_IMMUN][5]),ch[cn].kindred);

        dam=power*2;

        cost=dam/8+5;
        if ((ch[cn].flags&CF_PLAYER) && (ch[cn].kindred&(KIN_HARAKIM|KIN_ARCHHARAKIM)))
        	cost/=3;

        if (spellcost(cn,cost)) return;

        if (chance(cn,18)) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_BLAST][5]+5) {
                        do_char_log(co,1,"%s tried to cast blast on you but failed.\n",ch[cn].reference);
                        if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTMISS,cn,0,0,0);
                }
                return;
        }

        do_area_sound(co,0,ch[co].x,ch[co].y,ch[cn].sound+6);
        char_play_sound(co,ch[cn].sound+6,-150,0);

        chlog(cn,"Cast Blast on %s",ch[co].name);

        tmp=do_hurt(cn,co,dam,1);

        if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
        else do_char_log(cn,1,"You blast %s for %d HP.\n",ch[co].reference,tmp);

        fx_add_effect(5,0,ch[co].x,ch[co].y,0);

        co_orig=co;

        dam=dam/2+dam/4;

        m=ch[cn].x+ch[cn].y*MAPX;
        if ((co=map[m+1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                chlog(cn,"Cast Blast on %s",ch[co].name);

                tmp=do_hurt(cn,co,dam,1);

                if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
                else do_char_log(cn,1,"You blast %s for %d HP.\n",ch[co].reference,tmp);

                fx_add_effect(5,0,ch[co].x,ch[co].y,0);
        }
        if ((co=map[m-1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                chlog(cn,"Cast Blast on %s",ch[co].name);

                tmp=do_hurt(cn,co,dam,1);

                if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
                else do_char_log(cn,1,"You blast %s for %d HP.\n",ch[co].reference,tmp);

                fx_add_effect(5,0,ch[co].x,ch[co].y,0);
        }
        if ((co=map[m+MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                chlog(cn,"Cast Blast on %s",ch[co].name);

                tmp=do_hurt(cn,co,dam,1);

                if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
                else do_char_log(cn,1,"You blast %s for %d HP.\n",ch[co].reference,tmp);

                fx_add_effect(5,0,ch[co].x,ch[co].y,0);
        }
        if ((co=map[m-MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                chlog(cn,"Cast Blast on %s",ch[co].name);

                tmp=do_hurt(cn,co,dam,1);

                if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
                else do_char_log(cn,1,"You blast %s for %d HP.\n",ch[co].reference,tmp);

                fx_add_effect(5,0,ch[co].x,ch[co].y,0);
        }

        add_exhaust(cn,TICKS*6);

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);
}

void skill_repair(int cn)
{
        int in,chan,die,in2;

        if ((in=ch[cn].citem)==0) {
                do_char_log(cn,0,"Repair. Repair what?\n");
                return;
        }
        if (it[in].damage_state==0) {
                do_char_log(cn,0,"That isn't damaged.\n");
                return;
        }
        if (it[in].power>ch[cn].skill[SK_REPAIR][5] || (it[in].flags&IF_NOREPAIR)) {
                do_char_log(cn,0,"That's too difficult for you.\n");
                return;
        }
        if (ch[cn].a_end<it[in].power*1000) {
                do_char_log(cn,0,"You're too exhausted to repair that.\n");
                return;
        }
        ch[cn].a_end-=it[in].power*1000;

        if (it[in].power) chan=ch[cn].skill[SK_REPAIR][5]*15/it[in].power;
        else chan=18;
        if (chan<0) chan=0;
        if (chan>18) chan=18;

        die=RANDOM(20);

        if (die<=chan) {
                in2=god_create_item(it[in].temp);
                if (!in2) {
                        do_char_log(cn,0,"You failed.\n");
                        return;
                }
                it[in].used=USE_EMPTY;
                ch[cn].citem=in2;
                it[in2].carried=cn;
                do_char_log(cn,1,"Success!\n");
        } else {
                do_char_log(cn,0,"You failed.\n");
                item_damage_citem(cn,1000000);  // make sure it reaches the next state
                if (die-chan>3) item_damage_citem(cn,1000000);
                if (die-chan>6) item_damage_citem(cn,1000000);
        }
        chlog(cn,"Cast Repair");

}

void skill_recall(int cn)
{
        int in;

        if (is_exhausted(cn)) return;

        if (spellcost(cn,15)) return;
        if (chance(cn,18)) return;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_recall"); return; }

        strcpy(it[in].name,"Recall");
        it[in].flags|=IF_SPELL;
        it[in].sprite[1]=90;
        it[in].duration=it[in].active=max(TICKS/2,60-(ch[cn].skill[SK_RECALL][5]/4));
        it[in].temp=SK_RECALL;
        it[in].power=ch[cn].skill[SK_RECALL][5];
        it[in].data[0]=ch[cn].temple_x;
        it[in].data[1]=ch[cn].temple_y;

        if (!add_spell(cn,in)) {
                do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                return;
        }
        chlog(cn,"Cast Recall");

        add_exhaust(cn,TICKS);

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

//      ch[cn].errno=ERR_SUCCESS;
}

int spell_stun(int cn,int co,int power)
{
        int in;

        if (ch[co].flags&CF_IMMORTAL) return 0;

        in=god_create_item(1);
        if (!in) { xlog("god_create_item failed in skill_stun"); return 0; }

        power=spell_immunity(power,ch[co].skill[SK_IMMUN][5]);

	power=spell_race_mod(power,ch[cn].kindred);
        strcpy(it[in].name,"Stun");
        it[in].flags|=IF_SPELL;
        it[in].sprite[1]=91;
        it[in].duration=it[in].active=power+TICKS;
        it[in].temp=SK_STUN;
        it[in].power=power;

        if (ch[co].skill[SK_SENSE][5]+10>power)
                do_char_log(co,1,"%s cast stun on you.\n",ch[cn].reference);
        else do_char_log(co,0,"You have been stunned.\n");
        do_char_log(cn,1,"%s was stunned.\n",ch[co].reference);
        if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTHIT,cn,0,0,0);
        do_notify_char(cn,NT_DIDHIT,co,0,0,0);
        char_play_sound(co,ch[cn].sound+7,-150,0);
        char_play_sound(cn,ch[cn].sound+1,-150,0);
        chlog(cn,"Cast Stun on %s",ch[co].name);
        if (!add_spell(co,in)) {
                do_char_log(cn,1,"Magical interference neutralised the %s's effect.\n",it[in].name);
                return 0;
        }

        fx_add_effect(5,0,ch[co].x,ch[co].y,0);

        return 1;
}

void skill_stun(int cn)
{
        int co,m,co_orig;

        if ((co=ch[cn].skill_target1)) ;
        else if (ch[cn].attack_cn!=0) co=ch[cn].attack_cn;
        else co=cn;

        if (cn==co) { do_char_log(cn,0,"You cannot stun yourself!\n"); return; }

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        /* CS, 000209: Remember PvP attacks */
        remember_pvp(cn,co);
        if (is_exhausted(cn)) return;

        if (!may_attack_msg(cn,co,1)) {
                chlog(cn,"Prevented from attacking %s (%d)",ch[co].name,co);
                return;
        }

        if (spellcost(cn,20)) return;

        if (chance_base(cn,ch[cn].skill[SK_STUN][5],12,ch[co].skill[SK_RESIST][5])) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_STUN][5]+5) {
                        do_char_log(co,1,"%s tried to cast stun on you but failed.\n",ch[cn].reference);
                        if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTMISS,cn,0,0,0);
                }
                return;
        }

        if (ch[co].flags&CF_IMMORTAL) {
                do_char_log(cn,0,"You lost your focus.\n");
                return;
        }
        spell_stun(cn,co,ch[cn].skill[SK_STUN][5]);

        co_orig=co;

        m=ch[cn].x+ch[cn].y*MAPX;
        if ((co=map[m+1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                if (ch[cn].skill[SK_STUN][5]+RANDOM(20)>ch[co].skill[SK_RESIST][5]+RANDOM(20)) spell_stun(cn,co,ch[cn].skill[SK_STUN][5]);
        }
        if ((co=map[m-1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                if (ch[cn].skill[SK_STUN][5]+RANDOM(20)>ch[co].skill[SK_RESIST][5]+RANDOM(20)) spell_stun(cn,co,ch[cn].skill[SK_STUN][5]);
        }
        if ((co=map[m+MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                if (ch[cn].skill[SK_STUN][5]+RANDOM(20)>ch[co].skill[SK_RESIST][5]+RANDOM(20)) spell_stun(cn,co,ch[cn].skill[SK_STUN][5]);
        }
        if ((co=map[m-MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
                if (ch[cn].skill[SK_STUN][5]+RANDOM(20)>ch[co].skill[SK_RESIST][5]+RANDOM(20)) spell_stun(cn,co,ch[cn].skill[SK_STUN][5]);
        }

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        add_exhaust(cn,TICKS*3);

//      ch[cn].errno=ERR_SUCCESS;
}

void remove_spells(int cn)
{
        int in,n;

        for (n=0; n<20; n++) {
                if ((in=ch[cn].spell[n])==0) continue;
                it[in].used=USE_EMPTY;
                ch[cn].spell[n]=0;
        }
        do_update_char(cn);
}

void skill_dispel(int cn)
{
        int in,co,n,pwr=0,nr=-1;

        if ((co=ch[cn].skill_target1)) ;
        else co=cn;

        if (!do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        if (is_exhausted(cn)) return;

        /* Try removing curse from target */
        for (n=0; n<20; n++) {
                if ((in=ch[co].spell[n])==0) continue;
                if (it[in].temp==SK_CURSE) { nr=n; break; }
        }

        /* Try dispelling self */
        if (nr==-1 && co==cn) {
                for (n=0; n<20; n++) {
                        in = ch[co].spell[n];
                        if (in == 0) continue;
                        if (it[in].temp == SK_WIMPY) continue;
                        nr = n;
                        break;
                }
                if (n==20) {
                        do_char_log(cn,0,"But you aren't spelled!\n");
                        return;
                }
        }

        /* Try dispelling someone else (but not curse) */
        if (nr==-1) {
                for (n=0; n<20; n++) {
                        in = ch[co].spell[n];
                        if (in == 0) continue;
                        if (it[in].temp == SK_WIMPY) continue;
                        nr = n;
                        break;
                }
                if (n==20) {
                        do_char_log(cn,0,"%s isn't spelled!\n",ch[co].name);
                        return;
                }
                if (!may_attack_msg(cn, co, 1)) {
                        chlog(cn, "Prevented from dispelling %s\n", ch[co].name);
                        return;
                }
        }

        pwr=it[in].power;

        if (spellcost(cn,25)) return;

        if (chance_base(cn,spell_race_mod(ch[cn].skill[SK_DISPEL][5],ch[cn].kindred),12,pwr)) {
                if (cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_DISPEL][5]+5)
                        do_char_log(co,1,"%s tried to cast dispel magic on you but failed.\n",ch[cn].reference);
                return;
        }

        if ((co=ch[cn].skill_target1)) {
                in=ch[co].spell[nr];
                it[in].used=USE_EMPTY;
                ch[co].spell[nr]=0;
                do_update_char(co);
                /* CS, 000209: Remember PvP attacks */
                if (it[in].temp != SK_CURSE) remember_pvp(cn,co);

                if (ch[co].skill[SK_SENSE][5]+10>ch[cn].skill[SK_DISPEL][5])
                        do_char_log(co,1,"%s cast dispel magic on you.\n",ch[cn].reference);
                else do_char_log(co,0,"%s has been removed.\n",it[in].name);
                do_char_log(cn,1,"Removed %s from %s.\n",it[in].name,ch[co].name);
                if (it[in].temp!=SK_CURSE && !(ch[co].flags&(CF_PLAYER))) {
                        if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTHIT,cn,0,0,0);
                        do_notify_char(cn,NT_DIDHIT,co,0,0,0);
                }
                char_play_sound(co,ch[cn].sound+1,-150,0);
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                chlog(cn,"Cast Dispel on %s",ch[co].name);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
        } else {
                in=ch[cn].spell[nr];
                it[in].used=USE_EMPTY;
                ch[cn].spell[nr]=0;
                do_update_char(co);

                do_char_log(cn,1,"%s has been removed.\n",it[in].name);
                char_play_sound(cn,ch[cn].sound+1,-150,0);
                chlog(cn,"Cast Dispel");
                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0);
        }

        add_exhaust(cn,TICKS*2);

        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

//      ch[cn].errno=ERR_SUCCESS;
}

void skill_ghost(int cn)
{
        int co,cc,n,base=0,tmp,pts=0,z,m,idx;

        if (IS_BUILDING(cn)) {
                do_char_log(cn,0,"Not in build mode.\n");
                return;
        }

        if ((ch[cn].flags&CF_PLAYER) && (co=ch[cn].data[CHD_COMPANION])) {
                if (!IS_SANECHAR(co) || ch[co].data[63]!=cn || (ch[co].flags&CF_BODY) || ch[co].used==USE_EMPTY) co=0;
                if (co) {
                        do_char_log(cn,0,"You may not have more than one Ghost Companion (%d).\n",co);
                        return;
                }
        }

        if ((co=ch[cn].skill_target1)) ;
        else co=0;

        if (cn==co) co=0;

        if (co && !do_char_can_see(cn,co)) {
                do_char_log(cn,0,"You cannot see your target.\n");
                return;
        }

        if (is_exhausted(cn)) return;

        if (co && !may_attack_msg(cn,co,1)) {
                chlog(cn,"Prevented from attacking %s (%d)",ch[co].name,co);
                return;
        }

        if (spellcost(cn,45)) return;

        /* CS, 000109: No GC in Gatekeeper's room */
        if (ch[cn].x >= 39 && ch[cn].x <= 47 && ch[cn].y >= 594 && ch[cn].y <= 601) {
                do_char_log(cn, 0, "You must fight this battle alone.\n");
                return;
        }

        if (chance(cn,15)) {
                if (co && cn!=co && ch[co].skill[SK_SENSE][5]>ch[cn].skill[SK_GHOST][5]+5) {
                        do_char_log(co,1,"%s tried to cast ghost companion on you but failed.\n",ch[cn].reference);
                        if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTMISS,cn,0,0,0);
                }
                return;
        }

        cc=god_create_char(CT_COMPANION,1);
        if (!god_drop_char_fuzzy(cc,ch[cn].x,ch[cn].y)) {
                ch[cc].used=USE_EMPTY;
                do_char_log(cn,0,"The ghost companion could not materialize.\n");
                return;
        }

        if (co) {
                if (!IS_IGNORING_SPELLS(co)) do_notify_char(co,NT_GOTHIT,cn,0,0,0);
                do_notify_char(cn,NT_DIDHIT,co,0,0,0);
        }

        if (ch[cn].flags&CF_PLAYER) ch[cn].data[CHD_COMPANION]=cc;

        base=(ch[cn].skill[SK_GHOST][5]*4)/11;

        base=spell_race_mod(base,ch[cn].kindred);

        ch[cc].data[29]=0;                              // reset experience earned
        ch[cc].data[42]=65536+cn;                       // set group
        ch[cc].kindred&=~(KIN_MONSTER);

        if (co) {
                ch[cc].attack_cn=co;
                idx=co|(char_id(co)<<16);
                ch[cc].data[80]=idx;            // add enemy to kill list
        }
        ch[cc].data[63]=cn;
        ch[cc].data[69]=cn;
        if (ch[cn].flags&CF_PLAYER) ch[cc].data[CHD_COMPANION]=0;  // player GCs stay forever
        else ch[cc].data[CHD_COMPANION]=globs->ticker+TICKS*60*5;  // NPC GCs stay only for 5 minutes
        ch[cc].data[98]=globs->ticker + COMPANION_TIMEOUT;

        strcpy(ch[cc].text[0],"#14#Yes! %s buys the farm!");
        strcpy(ch[cc].text[1],"#13#Yahoo! An enemy! Prepare to die, %s!");
        strcpy(ch[cc].text[3],"My successor will avenge me, %s!");
        ch[cc].data[CHD_TALKATIVE] = ch_temp[CT_COMPANION].data[CHD_TALKATIVE];

        ch[cc].data[48]=33;

        for (n=0; n<5; n++) {
                tmp=base;
                tmp=tmp*3/max(1,ch[cc].attrib[n][3]);
                ch[cc].attrib[n][0]=max(10,min(ch[cc].attrib[n][2],tmp));
        }

        for (n=0; n<50; n++) {
                tmp=base;
                tmp=tmp*3/max(1,ch[cc].skill[n][3]);
                if (ch[cc].skill[n][2]) ch[cc].skill[n][0]=min(ch[cc].skill[n][2],tmp);
        }

        ch[cc].hp[0]=max(50,min(ch[cc].hp[2],base*5));
        ch[cc].end[0]=max(50,min(ch[cc].end[2],base*5));
        ch[cc].mana[0]=0;

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
        ch[cc].gold=0;
        ch[cc].a_hp=999999;
        ch[cc].a_end=999999;
        ch[cc].a_mana=999999;

        ch[cc].alignment=ch[cn].alignment/2;

        if (ch[cc].attrib[AT_AGIL][0]>=90 && ch[cc].attrib[AT_STREN][0]>=90) { // titanium
                ch[cc].armor_bonus=48+32;
                ch[cc].weapon_bonus=40+32;
        } else if (ch[cc].attrib[AT_AGIL][0]>=72 && ch[cc].attrib[AT_STREN][0]>=72) { //crystal
                ch[cc].armor_bonus=36+28;
                ch[cc].weapon_bonus=32+28;
        } else if (ch[cc].attrib[AT_AGIL][0]>=40 && ch[cc].attrib[AT_STREN][0]>=40) { //gold
                ch[cc].armor_bonus=30+24;
                ch[cc].weapon_bonus=24+24;
        } else if (ch[cc].attrib[AT_AGIL][0]>=24 && ch[cc].attrib[AT_STREN][0]>=24) { //steel
                ch[cc].armor_bonus=24+20;
                ch[cc].weapon_bonus=16+20;
        } else if (ch[cc].attrib[AT_AGIL][0]>=16 && ch[cc].attrib[AT_STREN][0]>=16) { //bronze
                ch[cc].armor_bonus=18+16;
                ch[cc].weapon_bonus=8+16;
        } else if (ch[cc].attrib[AT_AGIL][0]>=12 && ch[cc].attrib[AT_STREN][0]>=12) { //leather
                ch[cc].armor_bonus=12+12;
                ch[cc].weapon_bonus=8+12;
        } else if (ch[cc].attrib[AT_AGIL][0]>=10 && ch[cc].attrib[AT_STREN][0]>=10) { //cloth
                ch[cc].armor_bonus=6+8;
                ch[cc].weapon_bonus=8+8;
        }

        xlog("Created %s (%d) with base %d as Ghost Companion for %s",ch[cc].name,cc,base,ch[cn].reference);

        /* CS, 000109: Less chatty GC */
        if (co) {
                do_sayx(cc,ch[cc].text[1],ch[co].name);
        } else if (points2rank(ch[cc].points_tot) < 6) { // GC not yet Master Sergeant
                do_sayx(cc,"I shall defend you and obey your commands, %s. "
                           "I will WAIT, FOLLOW , be QUIET or ATTACK for you and tell you WHAT TIME. "
                           "You may also command me to TRANSFER my experience to you, though I'd rather you didn't.",
                        ch[cn].name);
        } else {
                do_sayx(cc,"Thank you for creating me, %s!", ch[cn].name);
        }

        do_update_char(cc);

        add_exhaust(cn,TICKS*4);

        fx_add_effect(6,0,ch[cc].x,ch[cc].y,0);
        fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);

        return;
}

int is_facing(int cn,int co)
{
        int ok=0;

        switch(ch[cn].dir) {
                case    DX_RIGHT:       if (ch[cn].x+1==ch[co].x && ch[cn].y==ch[co].y) ok=1; break;
                case    DX_LEFT:        if (ch[cn].x-1==ch[co].x && ch[cn].y==ch[co].y) ok=1; break;
                case    DX_UP:          if (ch[cn].x==ch[co].x && ch[cn].y-1==ch[co].y) ok=1; break;
                case    DX_DOWN:        if (ch[cn].x==ch[co].x && ch[cn].y+1==ch[co].y) ok=1; break;
                default: break;
        }

        return ok;
}

int is_back(int cn,int co)
{
        int ok=0;

        switch(ch[cn].dir) {
                case    DX_LEFT:        if (ch[cn].x+1==ch[co].x && ch[cn].y==ch[co].y) ok=1; break;
                case    DX_RIGHT:       if (ch[cn].x-1==ch[co].x && ch[cn].y==ch[co].y) ok=1; break;
                case    DX_DOWN:        if (ch[cn].x==ch[co].x && ch[cn].y-1==ch[co].y) ok=1; break;
                case    DX_UP:          if (ch[cn].x==ch[co].x && ch[cn].y+1==ch[co].y) ok=1; break;
                default: break;
        }

        return ok;
}

/*void skill_bash(int cn)
{
        int co,dam;

        if ((co=ch[cn].attack_cn)==0) {
                do_char_log(cn,0,"But you're not fighting anybody!\n");
                return;
        }

        if (!is_facing(cn,co)) {
                do_char_log(cn,0,"You must be facing your enemy!\n");
                return;
        }

        dam=ch[cn].skill[SK_BASH][5]*250;

        if (ch[cn].a_end<dam) {
                do_char_log(cn,0,"You're too exhausted to do that!\n");
                return;
        }
        ch[cn].a_end-=dam;

        if (ch[cn].skill[SK_BASH][5]<ch[co].attrib[AT_AGIL][5]+RANDOM(20)) {
                do_char_log(cn,0,"%s evades your bash. You lose your balance.\n",ch[co].reference);
                ch[cn].a_end-=dam*5;
                if (ch[cn].a_end<1000) ch[cn].a_end=1000;
        } else {
                do_char_log(cn,1,"%s falls and loses %d Endurance.\n",ch[co].reference,dam/100);
                do_char_log(co,0,"%s bashes you. You fall and lose %d Endurance.\n",ch[cn].reference,dam/100);
                ch[co].a_end-=dam*10;
                if (ch[co].a_end<1000) ch[co].a_end=1000;
        }
}*/

void nomagic(int cn)
{
        do_char_log(cn,0,"Your magic fails. You seem to be unable to cast spells.\n");
}

/* Look up skills by name. Return the index of the skill in skilltab or -1 if not found.
   The matching algorithm tries to be very tolerant: It succeeds when either of the words
   ends or hits a blank with no discrepancy found so far.
   For compatibility reasons, it also decodes numeric skill values. */
int skill_lookup(char *name)
{
        int n, j;
        char *p, *q;

        if (*name == '\0') return -1;                   // empty string does not match
        if (name[0]=='0' && name[1]=='\0') return 0;    // special case: "0".
        n = atoi(name);                                 // try numeric
        if (!SANESKILL(n)) return -1;                   // bad numeric
        if (n > 0) return n;                            // good numeric
                                                        // try alpha
        for (j=0; j<MAXSKILL; j++) {
                for (p=name,q=skilltab[j].name; 1; p++,q++)
                {
                        if (*p=='\0' || *q=='\0' || *q==' ') return j;
                        if (tolower(*p) != tolower(*q)) break;
                }
        }
        // fallen out of loop: not found
        return -1;
}

void skill_driver(int cn,int nr)
{
//      ch[cn].errno=ERR_FAILED;        // will be overriden later if another result is desired

        if (!ch[cn].skill[nr][0]) {
                do_char_log(cn,0,"You cannot use this skill/spell.\n");
                return;
        }

        switch(nr) {
                case    SK_LIGHT:       if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_light(cn); break;
                case    SK_PROTECT:     if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_protect(cn); break;
                case    SK_ENHANCE:     if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_enhance(cn); break;
                case    SK_BLESS:       if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_bless(cn); break;
                case    SK_CURSE:       if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_curse(cn); break;
                case    SK_IDENT:       if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_identify(cn); break;
                case    SK_BLAST:       if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_blast(cn); break;
                case    SK_REPAIR:      skill_repair(cn); break;
                case    SK_LOCK:        do_char_log(cn,0,"You cannot use this skill directly. Hold a lock-pick under your mouse cursor and click on the door.\n");
                                        break;
                case    SK_RECALL:      if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_recall(cn); break;
                case    SK_STUN:        if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_stun(cn); break;
                case    SK_DISPEL:      if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_dispel(cn); break;
                case    SK_WIMPY:       if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_wimp(cn); break;

                case    SK_HEAL:        if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_heal(cn); break;

                case    SK_GHOST:       if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_ghost(cn); break;
                case    SK_MSHIELD:     if (ch[cn].flags&CF_NOMAGIC) nomagic(cn); else skill_mshield(cn); break;

                case    SK_IMMUN:       do_char_log(cn,0,"You use this skill automatically when someone casts evil spells on you.\n"); break;

                case    SK_REGEN:
                case    SK_REST:
                case    SK_MEDIT:       do_char_log(cn,0,"You use this skill automatically when you stand still.\n");
                                        break;

                case    SK_DAGGER:
                case    SK_SWORD:
                case    SK_AXE:
                case    SK_STAFF:
                case    SK_TWOHAND:
                case    SK_SURROUND:    do_char_log(cn,0,"You use this skill automatically when you fight.\n");
                                        break;

                case 	SK_CONCEN:	do_char_log(cn,0,"You use this skill automatically when you cast spells.\n");
                			break;
                			
                case	SK_WARCRY:	skill_warcry(cn); break;

                default:                do_char_log(cn,0,"You cannot use this skill/spell.\n"); break;
        }
}
