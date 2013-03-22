/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

/* lab9.h -- definitions and data for lab 9 quest */

// How many NPCs are giving out riddles in Lab 9
#define RIDDLEGIVERS    5

// How many riddles each riddle giver knows
#define MAX_RIDDLES     11

// How long before the time for the riddle is up (3 minutes)
#define RIDDLE_TIMEOUT  (3*60*TICKS)

extern int guesser[RIDDLEGIVERS];
// extern int riddleno[RIDDLEGIVERS];
extern int riddletimeout[RIDDLEGIVERS];
// extern int riddleattempts[RIDDLEGIVERS];

// Areas of knowledge define a riddlegiver
#define RIDDLE_MIN_AREA         21
#define RIDDLE_MAX_AREA         25

// how many attempts a player has to solve.
#define RIDDLE_ATTEMPTS         3

// Number of true-false switch banks
#define BANKS                   5
// Number of switches per bank
#define SWITCHES                6
// Number of questions available (PER_BLOCK chosen at random)
#define BANK_QUESTIONS          8

/* Prototypes */
int lab9_guesser_says(int cn, char *text);
void lab9_pose_riddle(int riddler, int co);
void lab9_reset_bank(int bankno, int closedoor);
int use_lab9_switch(int cn, int in);
void init_lab9();
int use_lab9_door(int cn, int in);
