/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

int npc_dist(int cn,int co);
int npc_try_spell(int cn,int co,int spell);
int npc_check_target(int x,int y);

// -- stunrun.c
int npc_stunrun_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
int npc_stunrun_high(int cn);
void npc_stunrun_low(int cn);

// -- cityattack.c
int npc_cityattack_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
int npc_cityattack_high(int cn);
void npc_cityattack_low(int cn);

// -- npc_malte.c
int npc_malte_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
int npc_malte_high(int cn);
void npc_malte_low(int cn);
