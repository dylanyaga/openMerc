/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

/* lab9.c -- riddle mechanisms and data for lab 9 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

/* data for each of the 5 riddle givers */
int guesser[RIDDLEGIVERS] = { 0 };
int riddleno[RIDDLEGIVERS] = { 0 };
int riddletimeout[RIDDLEGIVERS] = { 0 };
int riddleattempts[RIDDLEGIVERS] = { 0 };

// teleport locations for successful solution
struct destination {
        int x;
        int y;
} destinations[RIDDLEGIVERS] = {
        { 39,729}, {40,749}, {40,769}, {40,789}, {34,806}
};

// riddle questions and answers for 5 riddlers, 11 to choose from.
// There are up to 3 correct answer words.
struct riddle {
        char *question;
        char *answer1, *answer2, *answer3;
} riddles[RIDDLEGIVERS][MAX_RIDDLES] = {
        { // Riddler 1
        {"This marvelous thing\n"
        "Though it sounds absurd\n"
        "Contains all our letters\n"
        "But is only a word\n"
        "What ist it?\n",
        "ALPHABET"},

        {"Like dogs shouting at the moon\n"
        "Or armor worn by the trees\n"
        "Like a sharply spoken command\n"
        "Or a tiny vessel upon the seas\n"
        "What is it?\n",
        "BARK"},

        {"All about the house\n"
        "With his Lady he dances\n"
        "Yet he always works\n"
        "And never romances\n"
        "What ist it?\n",
        "BROOM"},

        {"This engulfing thing is strange indeed\n"
        "The greater it grows the less you see\n"
        "What ist it?\n",
        "DARKNESS", "DARK"},

        {"I can not be seen only heard\n"
        "and I will not speak unless spoken to.\n"
        "What am I?\n",
        "ECHO"},

        {"Power enough to smash ships and crush roofs\n"
        "Yet it still must fear the sun\n"
        "What is it?\n",
        "ICE"},

        {"What is it that you must give before you can keep it?\n",
        "PROMISE"},

        {"Silently he stalks me\n"
        "Running as I run\n"
        "Creeping as I creep\n"
        "Dressed in black\n"
        "He disappears at night\n"
        "Only to return with the sun\n"
        "What is it?\n",
        "SHADOW"},

        {"It flies without wings\n"
        "Drops without fear\n"
        "But held in warm hands\n"
        "It will soon disappear\n"
        "What is it?\n",
        "SNOWFLAKE", "SNOW"},

        {"It never was before\n"
        "It is not now\n"
        "Fools wait for it forever\n"
        "What is it?\n",
        "FUTURE", "TOMORROW"},

        {"I am emeralds and diamonds,\n"
        "Lost by the moon.\n"
        "I am found by the sun\n"
        "And picked up soon.\n"
        "What am I?\n",
        "DEW"}
        }, // End Riddler 1

        { // Riddler 2
        {"Come up and let us go;\n"
        "go down and here we stay.\n"
        "What is it?\n",
        "ANCHOR"},

        {"This sparkling globe can float on water and weighs not more than a feather\n"
        "Yet despite its weight ten giants could never pick it up\n"
        "What is it?\n",
        "BUBBLE"},

        {"The one who made it didnt want it\n"
        "The one who bought it didnt need it\n"
        "The one who used it never saw it\n"
        "What is it?\n",
        "COFFIN", "GRAVE", "TOMB"},

        {"It can hold you\n"
        "But you cannot hold it\n"
        "And the more you remove\n"
        "The bigger it will get\n"
        "What is it?\n",
        "HOLE"},

        {"Tear one off and scratch my head,\n"
        "what once was red is black instead.\n",
        "What am I?\n",
        "MATCH"},

        {"Always old, sometimes new, never sad,\n"
        "sometimes blue. Never empty, sometimes full,\n"
        "never pushes, always pulls.\n"
        "What is it?\n",
        "MOON"},

        {"Drapes the hills all in white,\n"
        "swallows not but hard it bites.\n"
        "What is it?\n",
        "FROST"},

        {"What can bring back the dead; make us cry,\n"
        "make us laugh, make us young;\n"
        "born in an instant yet lasts a life time.\n"
        "What is it?\n",
        "MEMORY", "MEMORIES"},

        {"You tie these things\n"
        "Before you go\n"
        "And untie them\n"
        "After you stop\n"
        "What is it?\n",
        "SHOES", "SHOE"},

        {"The language of men can be mastered\n"
        "But what word is always pronounced wrong?\n",
        "WRONG"},

        {"Its tail is round and hollow\n"
        "Seems to get chewed a bit\n"
        "But youll rarely see this thing\n"
        "Unless the other end is lit\n"
        "What is it?\n",
        "PIPE"}

        }, // End Riddler 2

        { // Riddler 3
        {"After the final fire the winds will blow \n"
        "And these which are already dead will cover the ones who have yet to die \n"
        "What are these?\n",
        "ASHES"},

        {"When I live I cry, if you don't kill me I'll die.\n"
        "What am I? \n",
        "CANDLE"},

        {"Twins on either side of a ridge that smells\n"
        "They shall never see each other directly\n"
        "What are they?\n",
        "EYES", "EYE"},

        {"What is it the more you take, \n"
        "the more you leave behind? \n",
        "FOOTSTEPS", "STEPS", "STEP"},

        {"You see me oft\n"
        "In woods and town\n"
        "With my roots above\n"
        "I must grow down\n"
        "What am I?\n",
        "ICICLE", "ICICLES"},

        {"Passed from father to son\n"
        "And shared between brothers\n"
        "Its importance is unquestioned\n"
        "Though it is used more by others\n"
        "What is it?\n",
        "NAME", "NAMES"},

        {"Walk on the living, we don't even mumble.\n"
        "Walk on the dead, we mutter and grumble.\n"
        "What are we?\n",
        "LEAVES", "LEAF"},

        {"She has tasteful friends\n"
        "And tasteless enemies\n"
        "Tears are often shed on her behalf\n"
        "Yet never has she broken a heart\n"
        "What is it?\n",
        "ONION", "ONIONS"},

        {"This odd thing seems to get wetter\n"
        "The more it dries\n"
        "What is it?\n",
        "TOWEL"},

        {"He got it in the woods and brought it home in his hand because he couldn't find it\n"
        "The more he looked for it the more he felt it When he finally found it he threw it away\n"
        "What was it?\n",
        "THORN", "PRICK", "SLIVER"},

        {"Four legs in front two behind \n"
        "Its steely armor scratched and dented by rocks and sticks\n"
        "Still it toils as it helps feed the hungry\n"
        "What is it?\n",
        "PLOW"}

        }, // End Riddler 3

        { // Riddler 4
        {"Black when bought \n"
        "Red when used \n"
        "Grey when thrown away\n"
        "What is it?\n",
        "COALS", "COAL"},

        {"As I walked along the path \n"
        "I saw something with four fingers and one thumb, \n"
        "but it was not flesh, fish, bone or fowl. \n"
        "What was it?\n",
        "GLOVE", "GLOVES"},

        {"Look in my face I am somebody \n"
        "Look at my back I am nobody\n"
        "What am I?\n",
        "MIRROR"},

        {"A shimmering field that reaches far \n"
        "Yet it has no tracks \n"
        "And is crossed without paths \n"
        "What is it?\n",
        "OCEAN", "SEA"},

        {"A precious gift this \n"
        "Yet it has no end or beginning \n"
        "And in the middle nothing \n"
        "What is it?\n",
        "RING"},

        {"An untiring servant it is carrying loads across muddy earth\n"
        "But one thing that cannot be forced is a return to the place of its birth\n"
        "What is it?\n",
        "RIVER"},

        {"It can pierce the best armor \n"
        "And make swords crumble with a rub \n"
        "Yet for all its power \n"
        "It can't harm a club\n"
        "What is it?\n",
        "RUST"},

        {"No sooner spoken than broken. \n"
        "What is it? \n",
        "SILENCE"},

        {"One pace to the North\n"
        "Two paces to the East\n"
        "Two paces to the South\n"
        "Two paces to the West\n"
        "One pace to the North\n"
        "What is it?\n",
        "SQUARE"},

        {"This great thing can be swallowed\n"
        "But can also swallow us\n"
        "What is it?\n",
        "WATER"},

        {"Holes at the top \n"
        "Holes at the bottom \n"
        "Holes in the middle \n"
        "But still it holds water\n"
        "What is it?\n",
        "SPONGE", "SPONGES"}
        }, // End Riddler 4

        { // Riddler 5
        {"Feed me and I live, give me a drink and I die.\n"
        "What am I? \n",
        "FIRE"},

        {"What goes through a door \n"
        "but never goes in\n"
        "and never comes out? \n",
        "KEYHOLE"},

        {"Some will use me, while others will not, \n"
        "some have remembered, while others have forgot.\n"
        "For profit or gain, I'm used expertly, \n"
        "I can't be picked off the ground or tossed into the sea.\n"
        "Only gained from patience and time, \n"
        "can you unravel my rhyme? \n",
        "KNOWLEDGE", "WISDOM"},

        {"We love it more than life\n"
        "We fear it more than death\n"
        "The wealthy want for it\n"
        "The poor have it in plenty\n"
        "What is it?\n",
        "NOTHING"},

        {"If you have it, you want to share it. \n"
        "If you share it, you don't have it. \n"
        "What is it? \n",
        "SECRET"},

        {"Up and down they go but never move\n"
        "What are they?\n"
        "STAIRS", "STAIR", "STEPS"},

        {"You must keep this thing \n"
        "Its loss will affect your brothers\n"
        "For once yours is lost\n"
        "It will soon be lost by others\n"
        "What is it?\n",
        "TEMPER"},

        {"An open-ended barrel, it is shaped like a hive.\n"
        "It is filled with flesh, and the flesh is alive! \n"
        "What is it?\n",
        "THIMBLE"},

        {"Mountains will crumble and temples will fall,\n"
        "and no man can survive its endless call. \n"
        "What is it? \n",
        "TIME", "ETERNITY"},

        {"This old one runs forever\n"
        "But never moves at all\n"
        "He has not lungs nor throat\n"
        "Still a mighty roaring call\n"
        "What is it?\n",
        "WATERFALL", "FALLS"},

        {"You can hear me. \n"
        "You can see what I do. \n"
        "Me, you cannot see. \n"
        "What am I? \n",
        "WIND", "STORM"}
        } // End Riddler 5
};

/* ================================================================= */
/* Part 2: Switches                                                  */
/* ================================================================= */

#define FALSE   0
#define TRUE    (!FALSE)

struct switch_question {
        int truth;
        char *question;
} switch_questions[BANKS][BANK_QUESTIONS] = {
        { //bank 1
        { TRUE, "Jefferson gives the quest for repair." },
        { FALSE, "Steven asks you to bring him a greater healing potion." },
        { FALSE, "Gunthar lives in Rose Street." },
        { FALSE, "The golems in the Pentagram Quest say \"Dusdra gur, Hu-Har!\" when attacking." },
        { TRUE,  "Argha is a Master Sergeant." },
        { FALSE, "There are exactly 17 rooms in the Dungeon of Doors." },
        { TRUE,  "Serena is a templar." },
        { TRUE,  "There is a purple flower growing under the tree beside the Magic Shop." }
        }, //end bank 1

        { // bank 2
        { FALSE, "Ingrid gives the quest for recall." },
        { FALSE, "Nasir asks you to bring him a potion of life." },
        { TRUE,  "Serena lives in Temple Street." },
        { TRUE,  "If asked, Robin tells you about Lord Azrael of Aston." },
        { TRUE,  "The barkeeper in the Tavern of the Blue is a First Lieutenant." },
        { TRUE,  "In Stevens house is a hole, that leads into the Underground." },
        { FALSE, "Kira is a Staff Sergeant." },
        { FALSE, "Leopold is an old man of about 70 years." }
        }, // end bank 2

        { // bank 3
        { TRUE,  "Manfred gives the quest for sense magic." },
        { TRUE,  "Leopold wants you to bring him a Ratling Fighters Eye." },
        { FALSE, "The priest in the Temple of the Purple One is a Master Sergeant." },
        { TRUE,  "21 ghosts roam the Haunted Castle." },
        { FALSE, "Garna runs her shop right at the entrance to the mines." },
        { FALSE, "A golden ring adorned with a huge ruby raises you Intuition by 24 if activated." },
        { TRUE,  "Jefferson is a Second Lieutenant." },
        { FALSE, "There is a green flower growing behind the Leather Armor Shop." }
        }, // end bank 3

        { // bank 4
        { FALSE, "Sirjan gives the quest for identify." },
        { FALSE, "Cirrus wants you to bring him the Amulet of Resistance." },
        { TRUE,  "There are three Ratling Counts to be found in the Underground." },
        { FALSE, "The ghosts in the Haunted Castle praise Damor when they die." },
        { TRUE,  "Malte is a Corporal." },
        { FALSE, "Clara is wielding a golden dagger." },
        { TRUE,  "Hagen is running the Golden Armor Shop." },
        { TRUE,  "Nasir's left eye looks as if it was made of glass." }
        }, // end bank 4

        { // bank 5
        { FALSE, "Shiva is a Baron of Astonia." },
        { TRUE,  "Ursel is wearing bronze armor." },
        { TRUE,  "The Lizard Archmages carry 93 silver pieces." },
        { TRUE,  "The pentagram 134 is worth 2566 points." },
        { TRUE,  "The Greenling Prince is a Captain." },
        { FALSE, "Antonia runs the leather armor shop." },
        { FALSE, "The pentagram 139 is worth 2766 points." },
        { FALSE, "Cirrus is a fat old man." }
        } // end bank 5
        };

struct bank {
        int x1;
        int y1;
        int temp;
        int doorx;
        int doory;
        } banks[BANKS] = {
                { 23, 707, 1047, 23, 720 },
                { 23, 727, 1069, 23, 741 },
                { 23, 747, 1076, 23, 761 },
                { 23, 767, 1084, 31, 781 },
                { 23, 787, 1088, 19, 803 }
        };

/* The 6 question selected from the 11 available questions per bank. */
int questions[BANKS][SWITCHES];

/* Called by do_say() to check for a spoken riddle answer.
   This does not fit in very well with npc_hear(). */
int lab9_guesser_says(int cn, char *text)
{
        int m;
        char word[40];
        int riddler, ar, idx, found;
        struct riddle *riddle;

        // is the speaker a player?
        if (!IS_PLAYER(cn)) return 0;

        // does the riddler exist?
        riddler = ch[cn].data[CHD_RIDDLER];
        if (!IS_SANENPC(riddler)) {
                ch[cn].data[CHD_RIDDLER] = 0;
                return 0;
        }

        // is the riddler a certified riddler?
        ar = ch[riddler].data[72]; // area of knowledge
        if ((ar < RIDDLE_MIN_AREA) || (ar > RIDDLE_MAX_AREA)) {
                ch[cn].data[CHD_RIDDLER] = 0;
                return 0;
        }

        // does the riddler remember the guesser?
        idx = ar - RIDDLE_MIN_AREA;
        if (guesser[idx] != cn) {
                ch[cn].data[CHD_RIDDLER] = 0;
                return 0;
        }

        // does the player see the riddler?
        if (!do_char_can_see(cn, riddler)) {
                return 0;
        }

        riddle = &riddles[idx][riddleno[idx]-1];
        found = 0;

        // break his saying into words (copied from do_say())
        while (1) {
                m=0;
                while (isalnum(*text) && m<40) word[m++]=*text++;
                word[m]=0;
                // check if the word matches any solution text
                if (!strcasecmp(word, riddle->answer1) ||
                    (riddle->answer2 && !strcasecmp(word, riddle->answer2)) ||
                    (riddle->answer3 && !strcasecmp(word, riddle->answer3))) {
                        found = 1;
                        break;
                }
                while (*text && !isalnum(*text)) text++;
                if (!*text) break;
        }

        if (found) {
                do_sayx(riddler, "That's absolutely correct, %s! "
                        "For solving my riddle, I will advance you in your quest. "
                        "Close your eyes and...\n",
                        ch[cn].name);
                if (god_transfer_char(cn, destinations[idx].x, destinations[idx].y)) {
                        guesser[idx] = 0;
                        ch[cn].data[CHD_RIDDLER] = 0;
                } else {
                        do_sayx(riddler, "Oops! Something went wrong. Please try again a bit later.\n");
                }
                return 1;
        } else {
                riddleattempts[idx]--;
                if (riddleattempts[idx] > 0) {
                        do_sayx(riddler, "Sorry, that's not right. You have %d more attempt%s!",
                                riddleattempts[idx],
                                (riddleattempts[idx] == 1) ? "" : "s");
                } else {
                        do_sayx(riddler, "Sorry, that's not right. Now you'll have to bring me the book again to start over!\n");
                        guesser[idx] = 0;
                        ch[cn].data[CHD_RIDDLER] = 0;
                }
                return 0;
        }
}

/* the riddler asks the player a question chosen at random from his
   area of knowledge. */
void lab9_pose_riddle(int riddler, int co)
{
        int idx;
        int rno; // riddle number
        char *question;
        char *p, *q;

        idx = ch[riddler].data[72] - RIDDLE_MIN_AREA;
        rno = 1 + RANDOM(MAX_RIDDLES);
        question = riddles[idx][rno-1].question;
        guesser[idx] = co;
        riddleno[idx] = rno;
        riddletimeout[idx] = RIDDLE_TIMEOUT;
        riddleattempts[idx] = RIDDLE_ATTEMPTS;
        do_sayx(riddler, "Here is a riddle. You have 3 minutes and %d attempts to say the correct answer.\n",
                riddleattempts[idx]);
        for (p=question; *p; p=q+1) {
                q = strchr(p, '\n');
                if (!q) break;
                do_sayx(riddler, "%-.*s", q-p, p);
        }
        ch[co].data[CHD_RIDDLER] = riddler;
}

int lab9_check_door(int bankno)
{
        int bankidx, n, x, y, t, m, q, in, correct, temp, flags;

        if ((bankno < 1) || (bankno > BANKS)) {
                xlog("lab9_check_door(): panic: bad bank number!!");
                return 0;
        }
        bankidx = bankno - 1;

        x = banks[bankidx].x1;
        y = banks[bankidx].y1;
        t = banks[bankidx].temp;

        // Check all switches in bank for correct setting
        correct = 1;
        for (n=0; n<SWITCHES; n++) {
                m = XY2M(x,y);
                in = map[m].it;
                if (!in || (it[in].temp != t)) {
                        xlog("lab9_check_door(): panic: no switch at %d!!", m);
                        return 0;
                }
                q = questions[bankidx][n];
                if ((it[in].data[1] == 0) != switch_questions[bankidx][q-1].truth) correct = 0;
                y++;
        }
        m = XY2M(banks[bankidx].doorx,banks[bankidx].doory);
        in = map[m].it;
        if (!in) {
                xlog("lab9_check_door(): panic: no door at %d!!", m);
                return 0;
        }
        if (correct) {
                if (!it[in].active) { // open door
                        it[in].data[1] = 0;
                        it[in].active = it[in].duration;
                        it[in].flags&=~(IF_MOVEBLOCK|IF_SIGHTBLOCK);
                        do_area_sound(0,0,it[in].x,it[in].y,10);
                        reset_go(it[in].x,it[in].y);
                        add_lights(it[in].x,it[in].y);
                }
                return 1;
        } else {
                if (!correct && it[in].active) { // close door
                        it[in].data[1] = 1;
                        it[in].active = 0;
                        temp=it[in].temp;
                        flags=it_temp[temp].flags&IF_SIGHTBLOCK;
                        it[in].flags|=IF_MOVEBLOCK|flags;
                        do_area_sound(0,0,it[in].x,it[in].y,10);
                        reset_go(it[in].x,it[in].y);
                        add_lights(it[in].x,it[in].y);
                }
                return 0;
        }
}

/* Reset a given numbered switch bank */
void lab9_reset_bank(int bankno, int closedoor)
{
        int bankidx, x, y, t, n, q, m, in, unique, j, door;

        xlog("lab9: reset bank #%d", bankno);
        if ((bankno < 1) || (bankno > BANKS)) {
                xlog("lab9_reset_bank(): panic: bad bank number!!");
                return;
        }

        bankidx = bankno - 1;
        x = banks[bankidx].x1;
        y = banks[bankidx].y1;
        t = banks[bankidx].temp;

        // Reset switches and build description from random question
        for (n=0; n<SWITCHES; n++) {
                m = XY2M(x,y);
                in = map[m].it;
                if (!in || (it[in].temp != t)) {
                        xlog("reset_bank_at(): panic: no switch at %d!!", m);
                        return;
                }
                bankidx = it[in].data[0] - 1;
                it[in].data[1] = 1;
                it[in].active = 0;
                do {
                        q = 1 + RANDOM(BANK_QUESTIONS);
                        questions[bankidx][n] = q;
                        unique = 1;
                        for (j=0; j<n; j++)
                                if (questions[bankidx][j] == q)
                                        unique = 0;
                } while (!unique);
                sprintf(it[in].description, "It looks like a switch. Attached near the bottom is a note that reads: %s\n",
                        switch_questions[bankidx][q-1].question);
                y++;
        }
        m = XY2M(banks[bankidx].doorx, banks[bankidx].doory);
        door = map[m].it;
        if (closedoor) (void) use_lab9_door(0,door);
}


/* flip a switch in Lab 9 */
int use_lab9_switch(int cn, int in)
{
        chlog(cn, "flipped a switch.");
        it[in].data[1] = !it[in].data[1];
        do_area_sound(0,0,it[in].x,it[in].y,10);
        if (lab9_check_door(it[in].data[0])) {
                do_char_log(cn, 2, "You hear a door open nearby.\n");
        }
        return 1;
}

/* initialize things with a reset of all switch banks */
void init_lab9()
{
        int n;

        for (n=1; n<=BANKS; n++) lab9_reset_bank(n,1);
}

/* one way door in lab 9. mostly copied from use_door() */
/* data[3] = switch bank number (1..5) */
int use_lab9_door(int cn, int in)
{
        int temp,flags;

        if (map[it[in].x+it[in].y*MAPX].ch) return 0;

        // This statement allows free movement southward.
        if (cn==0) {
                it[in].active = 1; // just so it will close for sure
        } else if (!it[in].active &&
                   ((ch[cn].x > it[in].x) || (ch[cn].y < it[in].y))) {
                do_char_log(cn,0,"It's locked and no key will open it.\n");
                return 0;
        }
        reset_go(it[in].x,it[in].y);
        remove_lights(it[in].x,it[in].y);

        do_area_sound(0,0,it[in].x,it[in].y,10);

        if (!it[in].active) {   // open door
                it[in].flags&=~(IF_MOVEBLOCK|IF_SIGHTBLOCK);
                it[in].data[1]=0;
        } else {                // close door
                temp=it[in].temp;
                flags=it_temp[temp].flags&IF_SIGHTBLOCK;
                it[in].flags|=IF_MOVEBLOCK|flags;
                it[in].data[1]=1;
                lab9_reset_bank(it[in].data[3], 0);
        }
        reset_go(it[in].x,it[in].y);
        add_lights(it[in].x,it[in].y);
        do_area_notify(cn,0,ch[cn].x,ch[cn].y,NT_SEE,cn,0,0,0);

        return 1;
}


