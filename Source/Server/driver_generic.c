/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include "server.h"
#include "driver.h"

int follow_driver(int cn,int co)
{
        int m,dir,x,y;

        if (co<=0 || co>=MAXCHARS) return 0;
        if (ch[co].tox<5 || ch[co].tox>MAPX-6 || ch[co].toy<5 || ch[co].toy>MAPY-6) return 0;

        if (!(IS_COMPANION(cn) && ch[cn].data[63]==co) && !do_char_can_see(cn,co)) return 0;

        m=ch[co].tox+ch[co].toy*MAPX;
        dir=ch[co].dir;

        switch(dir) {
                case DX_UP:             m=m+MAPX*2; break;
                case DX_DOWN:           m=m-MAPX*2; break;
                case DX_LEFT:           m=m+2; break;
                case DX_RIGHT:          m=m-2; break;
                case DX_LEFTUP:         m=m+2+MAPX*2; break;
                case DX_LEFTDOWN:       m=m+2-MAPX*2; break;
                case DX_RIGHTUP:        m=m-2+MAPX*2; break;
                case DX_RIGHTDOWN:      m=m-2-MAPX*2; break;
                default:                break;
        }

        if (map[m].ch==cn || map[m+1].ch==cn || map[m-1].ch==cn || map[m+MAPX].ch==cn ||
            map[m-MAPX].ch==cn || map[m+1+MAPX].ch==cn || map[m+1-MAPX].ch==cn || map[m-1+MAPX].ch==cn ||
            map[m-1-MAPX].ch==cn) {

            if (ch[cn].dir==dir) {
                ch[cn].misc_action=DR_IDLE;
                return 1;
            }

            ch[cn].misc_action=DR_TURN;
            x=ch[cn].x; y=ch[cn].y;

            switch(dir) {
                case DX_UP:             ch[cn].misc_target1=x; ch[cn].misc_target2=y-1; break;
                case DX_DOWN:           ch[cn].misc_target1=x; ch[cn].misc_target2=y+1; break;
                case DX_LEFT:           ch[cn].misc_target1=x-1; ch[cn].misc_target2=y; break;
                case DX_RIGHT:          ch[cn].misc_target1=x+1; ch[cn].misc_target2=y; break;
                case DX_LEFTUP:         ch[cn].misc_target1=x-1; ch[cn].misc_target2=y-1; break;
                case DX_LEFTDOWN:       ch[cn].misc_target1=x-1; ch[cn].misc_target2=y+1; break;
                case DX_RIGHTUP:        ch[cn].misc_target1=x+1; ch[cn].misc_target2=y-1; break;
                case DX_RIGHTDOWN:      ch[cn].misc_target1=x+1; ch[cn].misc_target2=y+1; break;
                default:                ch[cn].misc_action=DR_IDLE; break;
            }
            return 1;
        }

        if (plr_check_target(m)) m=m;
        else if (plr_check_target(m+1)) m=m+1;
        else if (plr_check_target(m-1)) m=m-1;
        else if (plr_check_target(m+MAPX)) m=m+MAPX;
        else if (plr_check_target(m-MAPX)) m=m-MAPX;
        else if (plr_check_target(m+1+MAPX)) m=m+1+MAPX;
        else if (plr_check_target(m+1-MAPX)) m=m+1-MAPX;
        else if (plr_check_target(m-1+MAPX)) m=m-1+MAPX;
        else if (plr_check_target(m-1-MAPX)) m=m-1-MAPX;
        else return 0;

        ch[cn].goto_x=m%MAPX;
        ch[cn].goto_y=m/MAPX;

        return 1;
}

void player_driver_med(int cn)
{
        int co; //,m,dir,x,y;

        if (ch[cn].data[12]+TICKS*15>globs->ticker) return;

        if ((co=ch[cn].data[10])!=0)
                follow_driver(cn,co);
}

void drv_moveto(int cn,int x,int y)
{
        int ret;

        /* why did i do that? it seems identical.
        if (ch[cn].flags&(CF_PLAYER)) ret=char_moveto(cn,x,y,0,0,0);
        else                          ret=char_moveto(cn,x,y,0,0,0); */

        ret=char_moveto(cn,x,y,0,0,0);

        if (ret) ch[cn].goto_x=0;
        if (ret==-1) ch[cn].last_action=ERR_FAILED;
        else if (ret==1) ch[cn].last_action=ERR_SUCCESS;
}

void drv_turnto(int cn,int x,int y)
{
        int dir;

        dir=drv_dcoor2dir(x-ch[cn].x,y-ch[cn].y);
        if (dir==ch[cn].dir) { ch[cn].misc_action=DR_IDLE; ch[cn].last_action=ERR_SUCCESS; }
        else {
                if (dir!=-1) { act_turn(cn,dir); }
                else ch[cn].last_action=ERR_FAILED;
        }
}

void drv_dropto(int cn,int x,int y)
{
        int ret;

        ret=char_dropto(cn,x,y);
        if (ret) ch[cn].misc_action=DR_IDLE;
        if (ret==-1) ch[cn].last_action=ERR_FAILED;
        else if (ret==1) ch[cn].last_action=ERR_SUCCESS;

}

void drv_pickupto(int cn,int x,int y)
{
        int ret;

        ret=char_pickupto(cn,x,y);
        if (ret) ch[cn].misc_action=DR_IDLE;
        if (ret==-1) ch[cn].last_action=ERR_FAILED;
        else if (ret==1) ch[cn].last_action=ERR_SUCCESS;

}

void drv_useto(int cn,int x,int y)
{
        int ret,m,in;

        ret=char_useto(cn,x,y);

        if (x<0 || x>=MAPX || y<0 || y>=MAPY) x=y=0;

        m=x+y*MAPX;
        in=map[m].it;

        if (ret && (!in || it[in].driver!=25)) ch[cn].misc_action=DR_IDLE;
        if (ret==-1) ch[cn].last_action=ERR_FAILED;
        else if (ret==1) ch[cn].last_action=ERR_SUCCESS;

}

void drv_use(int cn,int nr)
{
        int in;

        if (nr<20) {
                if (!(in=ch[cn].worn[nr])) { ch[cn].last_action=ERR_FAILED; ch[cn].use_nr=0; return; }
        } else {
                if (!(in=ch[cn].item[nr-20])) { ch[cn].last_action=ERR_FAILED; ch[cn].use_nr=0; return; }
        }

        use_driver(cn,in,1);    // HACK! This should go through plr_act as well!
        if (ch[cn].cerrno==ERR_SUCCESS) ch[cn].last_action=ERR_SUCCESS;
        if (ch[cn].cerrno==ERR_FAILED) ch[cn].last_action=ERR_FAILED;

        ch[cn].cerrno=ERR_NONE;
        ch[cn].use_nr=0;
}

void drv_attack_char(int cn,int co)
{
        int ret;

        ret=char_attack_char(cn,co);
        if (ret==-1) {
                ch[cn].attack_cn=0;
                ch[cn].last_action=ERR_FAILED;
        }
        else if (ret==1)
                ch[cn].last_action=ERR_SUCCESS;
}

void drv_give_char(int cn,int co)
{
        int ret;

        ret=char_give_char(cn,co);
        if (ret) ch[cn].misc_action=DR_IDLE;
        if (ret==-1) ch[cn].last_action=ERR_FAILED;
        else if (ret==1) ch[cn].last_action=ERR_SUCCESS;
}

void drv_bow(int cn)
{
        if (ch[cn].dir==DX_LEFTUP) { act_turn_left(cn); return; }
        if (ch[cn].dir==DX_LEFTDOWN) { act_turn_down(cn); return; }
        if (ch[cn].dir==DX_RIGHTUP) { act_turn_up(cn); return; }
        if (ch[cn].dir==DX_RIGHTDOWN) { act_turn_right(cn); return; }

        act_bow(cn);    // HACK!! this should go through driver_etc like moveto!
        ch[cn].misc_action=DR_IDLE;
        ch[cn].cerrno=ERR_NONE;
        ch[cn].last_action=ERR_SUCCESS;
}

void drv_wave(int cn)
{
        if (ch[cn].dir==DX_LEFTUP) { act_turn_left(cn); return; }
        if (ch[cn].dir==DX_LEFTDOWN) { act_turn_down(cn); return; }
        if (ch[cn].dir==DX_RIGHTUP) { act_turn_up(cn); return; }
        if (ch[cn].dir==DX_RIGHTDOWN) { act_turn_right(cn); return; }

        act_wave(cn); // HACK!! this should go through driver_etc like moveto!
        ch[cn].misc_action=DR_IDLE;
        ch[cn].cerrno=ERR_NONE;
        ch[cn].last_action=ERR_SUCCESS;
}

void drv_skill(int cn)
{
        if (ch[cn].dir==DX_LEFTUP) { act_turn_left(cn); return; }
        if (ch[cn].dir==DX_LEFTDOWN) { act_turn_down(cn); return; }
        if (ch[cn].dir==DX_RIGHTUP) { act_turn_up(cn); return; }
        if (ch[cn].dir==DX_RIGHTDOWN) { act_turn_right(cn); return; }

        act_skill(cn);          // HACK!! this should go through driver_etc like moveto!
        ch[cn].skill_target2=ch[cn].skill_nr;
        ch[cn].skill_nr=0;
        ch[cn].cerrno=ERR_NONE;
        ch[cn].last_action=ERR_SUCCESS;
}

void driver(int cn)
{
	unsigned long long prof;

        if (!(ch[cn].flags&(CF_PLAYER|CF_USURP))) { prof=prof_start(); npc_driver_high(cn); prof_stop(39,prof); }

        if (ch[cn].flags&CF_CCP) { prof=prof_start(); ccp_driver(cn); prof_stop(41,prof); }

        if (ch[cn].use_nr) { prof=prof_start(); drv_use(cn,ch[cn].use_nr); prof_stop(38,prof); return; }
        if (ch[cn].skill_nr) { prof=prof_start(); drv_skill(cn); prof_stop(37,prof); return; }

        if ((ch[cn].flags&(CF_PLAYER|CF_USURP)) && !ch[cn].attack_cn)
                { prof=prof_start(); player_driver_med(cn); prof_stop(40,prof); }

        if (ch[cn].goto_x) { prof=prof_start(); drv_moveto(cn,ch[cn].goto_x,ch[cn].goto_y); prof_stop(36,prof); return; }
        if (ch[cn].attack_cn) { prof=prof_start(); drv_attack_char(cn,ch[cn].attack_cn); prof_stop(35,prof); return; }

        switch(ch[cn].misc_action) {
                case    DR_IDLE:        if (ch[cn].flags&(CF_PLAYER|CF_USURP)) ;
                                        else { prof=prof_start(); npc_driver_low(cn); prof_stop(27,prof); }
                                        break;
                case    DR_DROP:        prof=prof_start(); drv_dropto(cn,ch[cn].misc_target1,ch[cn].misc_target2); prof_stop(28,prof); break;
                case    DR_PICKUP:      prof=prof_start(); drv_pickupto(cn,ch[cn].misc_target1,ch[cn].misc_target2); prof_stop(29,prof); break;
                case    DR_GIVE:        prof=prof_start(); drv_give_char(cn,ch[cn].misc_target1); prof_stop(30,prof); break;
                case    DR_USE:         prof=prof_start(); drv_useto(cn,ch[cn].misc_target1,ch[cn].misc_target2); prof_stop(31,prof); break;
                case    DR_BOW:         prof=prof_start(); drv_bow(cn); prof_stop(32,prof); break;
                case    DR_WAVE:        prof=prof_start(); drv_wave(cn); prof_stop(33,prof); break;
                case    DR_TURN:        prof=prof_start(); drv_turnto(cn,ch[cn].misc_target1,ch[cn].misc_target2);  prof_stop(34,prof); break;
                case    DR_SINGLEBUILD: break;
                case    DR_AREABUILD1:  break;
                case    DR_AREABUILD2:  break;
                default:                xlog("player_driver(): unknown misc_action %d",ch[cn].misc_action);
                                        ch[cn].misc_action=DR_IDLE;
        }
}

void driver_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4)
{
        // call driver dependend handler here:
        // if it handles the case, we should return.
        // otherwise we use the following defaults:

        if (ch[cn].stunned) return;     // stunned players don't get messages...

        if (!(ch[cn].flags&(CF_PLAYER|CF_USURP))) {
                if (npc_msg(cn,type,dat1,dat2,dat3,dat4)) return;
        }

        if (ch[cn].flags&CF_CCP) {
                ccp_msg(cn,type,dat1,dat2,dat3,dat4);
        }

        switch(type) {
                case    NT_GOTHIT:
                case    NT_GOTMISS:     if (!ch[cn].attack_cn &&
                                            !ch[cn].data[CHD_FIGHTBACK] && // fightback
                                            ch[cn].misc_action!=DR_GIVE) ch[cn].attack_cn=dat1;
                                        break;
        }
}

