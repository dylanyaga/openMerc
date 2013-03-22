/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

// -- server --
void xlog(char *format,...) __attribute__ ((format(printf,1,2)));
void seclog(char *format,...) __attribute__ ((format(printf,1,2)));
void chlog(int cn,char *format,...) __attribute__ ((format(printf,2,3)));
void plog(int nr,char *format,...) __attribute__ ((format(printf,2,3)));
void prof_tick(void);
unsigned long long prof_start(void);
void prof_stop(int task,unsigned long long cycle);
int csend(int nr,unsigned char *buf,int len);
void pkt_list(void);

// rdtsc
unsigned long long rdtsc(void);

// -- global --
void global_tick(void);

// -- populate --
void populate(void);
void pop_remove(void);
void init_lights(void);
int pop_create_bonus_belt(int cn);
int pop_create_char(int n,int drop);
void pop_wipe(void);
void pop_reset_all(void);
void xload(void);
int pop_create_item(int temp,int cn);
void pop_update(void);
void pop_skill(void);
void reset_changed_items(void);
void pop_save_all_chars(void);
void pop_save_char(int nr);
void pop_load_all_chars(void);

// -- tick --
void tick(void);
void player_exit(int nr);
void plr_logout(int cn,int nr,int reason);
void cl_list(void);

// -- disk --
void update(void);
void flush(void);
int load(void);
void unload(void);
void flush_char(int cn);

// -- plr --
void plr_map_set(int cn);
void plr_map_remove(int cn);
void plr_reset_status(int cn);

void player_exit(int nr);
void xsend(int nr,unsigned char *buf,int len);
void plr_act(int nr);
void driver(int cn);
void driver_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
int ch_base_status(int n);

// -- tick --
void char_play_sound(int cn,int nr,int vol,int pan);
void init_group(void);
int group_active(int cn);

// ---- act ----
int plr_check_target(int m);
void act_move_up(int cn);
void act_move_down(int cn);
void act_move_left(int cn);
void act_move_right(int cn);
void act_move_leftup(int cn);
void act_move_leftdown(int cn);
void act_move_rightup(int cn);
void act_move_rightdown(int cn);
void act_turn_up(int cn);
void act_turn_down(int cn);
void act_turn_left(int cn);
void act_turn_right(int cn);
void act_turn_leftup(int cn);
void act_turn_leftdown(int cn);
void act_turn_rightup(int cn);
void act_turn_rightdown(int cn);
void act_turn(int cn,int dir);
void act_attack(int cn);                // attack character in front of the character
void act_pickup(int cn);                // get the object in front of the character
void act_drop(int cn);          // drops the current object in front of the character
void act_use(int cn);
void act_give(int cn);
void act_bow(int cn);
void act_wave(int cn);
void act_skill(int cn);

int char_wears_item(int cn,int tmp);

// --- god ---
void god_init_freelist(void);
int god_create_item(int temp);
int god_create_char(int temp,int withitems);
int god_drop_item(int nr,int x,int y);
int god_drop_item_fuzzy(int nr,int x,int y);
int god_drop_item_destroy_target(int nr,int x,int y);
int god_give_char(int in,int cn);
int god_take_from_char(int in,int cn);
int god_drop_char(int cn,int x,int y);
int god_drop_char_fuzzy(int nr,int x,int y);
int god_drop_char_fuzzy_large(int nr,int x,int y,int xs,int ys);
int god_transfer_char(int cn,int x,int y);
void god_goto(int cn,int co,char *cx,char *cy);
void god_info(int cn,int co);
void god_who(int cn);
void god_top(int cn);
void god_create(int cn,int in);
void god_summon(int cn, char *spec1, char *spec2, char *spec3);
int god_thrall(int cn, char *spec1, char *spec2);
void god_mirror(int cn, char *spec1, char *spec2); //ML 7.11. ff
int god_remove_item(int cn,int nr);
void god_tavern(int cn);
void god_build(int cn,int x);
void god_set_flag(int cn,int co,unsigned long long flag);
void god_erase(int cn,int co,int insist);
int god_cmp_item2tmp(int in,int tmp);
int god_getitemtmp(int in);
void god_drdata(int cn,int co,int n,int val);
void god_destroy_items(int cn);
void god_raise_char(int cn,int co,int v);
void god_lower_char(int cn,int co,int v);
void god_donate_item(int in,int place);
void god_gold_char(int cn,int co,int v,char *silver);
void god_skill(int cn,int co,int n,int val);
int god_save(int cn,int co);
void god_luck(int cn,int co,int luck);
void user_who(int cn);
void god_set_purple(int cn,int co);
void god_iinfo(int cn,int in);
void god_tinfo(int cn,int temp);
void god_slap(int cn,int co);
void god_unique(int cn);
void god_reset_description(int cn,int co);
void god_set_name(int cn,int co,char *name);
void god_kick(int cn,int co);
void god_usurp(int cn,int co);
void god_grolm(int cn);
void god_gargoyle(int cn);
void god_grolm_info(int cn);
void god_grolm_start(int cn);
void god_racechange(int co,int temp);
void god_minor_racechange(int cn,int t);
char *int2str(int val);
void god_force(int cn, char *whom, char *text);
void do_enemy(int cn, char *npc, char *victim);
void god_exit_usurp(int cn);
void god_spritechange(int cn,int co,int sprite);
int god_is_banned(int addr);
void god_add_ban(int cn,int co);
void god_del_ban(int cn,int nr);
void god_list_ban(int cn);
int god_change_pass(int cn,int co,char *pass);
void god_implist(int cn);
void god_mail_pass(int cn,int co);
void god_set_gflag(int cn,int flag);
void god_init_badnames(void);
int god_is_badname(char *name);
int god_read_banlist(void);
void god_shutup(int cn,int co);

// ---- do ----
void do_log(int cn,int font,char *text);
void do_attack(int cn,int co,int surround);
int do_char_can_see(int cn,int co);
int do_char_can_see_item(int cn,int in);
void do_update_char(int cn);
void do_regenerate(int cn);
int do_lookup_char_self(char *name, int cn);
void do_admin_log(int source, char *format,...);
void do_imp_log(int font,char *format,...);
void do_announce(int source, int author, char *format,...);

int do_raise_attrib(int cn,int nr);
int do_raise_skill(int cn,int nr);
int do_raise_hp(int cn);
int do_raise_end(int cn);
int do_raise_mana(int cn);
int do_lower_hp(int cn);
int do_lower_mana(int cn);

int attrib_needed(int v,int diff);
int hp_needed(int v,int diff);
int end_needed(int v,int diff);
int mana_needed(int v,int diff);
int skill_needed(int v,int diff);

void do_add_light(int x,int y,int stren);
int do_store_item(int cn);
int do_swap_item(int cn,int n);
void do_look_char(int cn,int co,int godflag,int autoflag,int lootflag);
void do_look_item(int cn,int in);
int do_char_calc_light(int cn,int light);
void do_char_log(int cn,int font,char *format,...) __attribute__ ((format(printf,3,4)));
void do_say(int cn,char *text);
int do_char_score(int cn);
void do_area_log(int cn,int co,int x,int y,int font,char *format,...) __attribute__ ((format(printf,6,7)));
int do_char_can_flee(int cn);
void remove_enemy(int cn);
void do_char_killed(int cn,int co);
void do_give(int cn,int co);
void do_area_sound(int cn,int co,int x,int y,int nr);
void do_notify_char(int cn,int type,int dat1,int dat2,int dat3,int dat4);
void do_area_notify(int cn,int co,int xs,int ys,int type,int dat1,int dat2,int dat3,int dat4);
void do_npc_shout(int cn,int type,int dat1,int dat2,int dat3,int dat4);
void do_shop_char(int cn,int co,int nr);
void do_give_exp(int cn,int p,int gflag,int rank);
int get_fight_skill(int cn);
int do_hurt(int cn,int co,int dam,int type);
void do_staff_log(int font,char *format,...) __attribute__ ((format(printf,2,3)));
int do_remove_unique(int cn);
void do_unique_info(int cn);
void do_look_depot(int cn,int co);
void do_depot_char(int cn,int co,int nr);
void do_sort(int cn,char *arg);
int do_depot_cost(int in);
void do_pay_depot(int cn);
int may_attack_msg(int cn,int co,int msg);
int do_maygive(int cn,int co,int in);
int do_item_value(int in);
void do_check_new_level(int cn);
int invis_level(int cn);
int dbatoi(char *text);
int dbatoi_self(int cn, char *text);
void do_seen(int cn, char *cco);
void do_spellignore(int cn);
int isgroup(int cn,int co);
void do_afk(int cn, char *msg);
void do_ransack_corpse(int cn, int co, char *msg);
void remember_pvp(int cn, int co);
void really_update_char(int cn);

//-- use --
void use_driver(int cn,int in,int worn);
void item_tick(void);
void map_tick(void);
void item_damage_armor(int cn,int dam);
void item_damage_weapon(int cn,int dam);
void item_damage_citem(int cn,int dam);
void item_damage_worn(int cn,int n,int dam);
int item_age(int in);
void step_driver_remove(int cn,int in);
int sub_door_driver(int cn,int in);
void finish_laby_teleport(int cn,int nr,int exp_pts);

//-- effect --
void effect_tick(void);
int fx_add_effect(int type,int duration,int d1,int d2,int d3);

// -- look --
void look_driver(int cn,int in);

//-- build --
void build_drop(int x,int y,int in);
void build_remove(int x,int y);
void pop_tick(void);

//-- helper --
void reset_go(int x,int y);
void remove_lights(int x,int y);
void add_lights(int x,int y);
int can_go(int x,int y,int tx,int ty);
int can_see(int cn,int x,int y,int tx,int ty,int maxdist);
int check_dlight(int x,int y);
void compute_dlight(int x,int y);
int char_id(int cn);
void char_item_expire(int cn);
extern char *rank_name[];
extern char *who_rank_name[];
int rankdiff(int cn,int co);
int absrankdiff(int cn,int co);
int in_attackrange(int cn, int co);
int in_grouprange(int cn, int co);
int points2rank(int v);
int points_tolevel(int curr_exp);
int scale_exps(int cn,int co,int exps);
int scale_exps2(int cn,int co_rank,int exps);
int create_special_item(int temp);
int killed_class(int cn,int val);
char *get_class_name(int nr);
void titlecase_str(char *str);
int prefix(char *name, char *abbr);
char *ago_string(int dt);
int use_labtransfer(int cn,int nr,int exp_pts);
void use_labtransfer2(int cn,int co);
void player_analyser(int cn,char *text);
void show_time(int cn);
void effectlist(int cn);
void init_badwords(void);
int is_badword(char *sentence);
int cap(int cn,int nr);
int use_soulstone(int cn,int in);
int load_mod(void);
void set_cap(int cn,int nr);

//-- skill_driver --
int spell_immunity(int power,int immun);
int friend_is_enemy(int cn,int cc);
void skill_driver(int cn,int nr);
void char_info(int cn,int co);
void item_info(int cn,int in,int look);
void spell_from_item(int cn,int in2);
int add_spell(int cn,int in);
void remove_spells(int cn);
int is_facing(int cn,int co);
int is_back(int cn,int co);
int spell_light(int cn,int co,int power);
int spell_protect(int cn,int co,int power);
int spell_enhance(int cn,int co,int power);
int spell_bless(int cn,int co,int power);
int spell_mshield(int cn,int co,int power);
int spell_heal(int cn,int co,int power);
int spell_curse(int cn,int co,int power);
int spell_stun(int cn,int co,int power);
int skill_lookup(char *skill);
int chance_base(int cn,int skill,int d20,int power);
int player_or_ghost(int cn,int co);

// -- driver --
int npc_driver_high(int cn);
void npc_driver_low(int cn);
int npc_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
void update_shop(int cn);
int step_driver(int cn,int in);
int count_uniques(int cn);
int npc_add_enemy(int cn,int co,int allways);
int npc_is_enemy(int cn,int co);
int npc_list_enemies(int npc, int cn);
int npc_remove_enemy(int npc, int enemy);
void die_companion(int cn);
int is_potion(int in);
int is_scroll(int in);
int is_unique(int in);

// --- talk ---
void npc_hear(int cn,int co,char *text);
void strlower(char *str);

// --- area ---
char *get_area(int cn,int verbose);
char *get_area_m(int x,int y,int verbose);

// --- path.c ---
int init_node(void);

// --- ccp ---
void ccp_driver(int cn);
void ccp_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
void ccp_shout(int cn,int co,char *text);
void ccp_tell(int cn,int co,char *text);
