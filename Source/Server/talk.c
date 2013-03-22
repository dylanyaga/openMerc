/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "server.h"

// You HAVE to use DO_SAYX! not DO_SAY!!

void strlower(char *a)
{
        while (*a) { *a=tolower(*a); a++; }
}

// these words are ignore
char *fillword[]={"the","a","an","do","'","of","is","that","those","these","they","-","does","can","oh","me","about","to","if","for"};

int is_fillword(char *a)
{
        int n;

        for (n=0; n<sizeof(fillword)/sizeof(fillword[0]); n++) if (!strcmp(a,fillword[n])) return 1;

        return 0;
}

// left word is changed into right word before parsing the text
char *syn[]={
"1","one",
"2","two",
"3","three",
"4","four",
"5","five",
"6","six",
"7","seven",
"8","eight",
"9","nine",
"whats","what",
"which","what",
"wheres","where",
"dangers","danger",
"enemies","danger",
"enemy","danger",
"foe","danger",
"foes","danger",
"thieves","thief",
"trouble","danger",
"laby","labyrinth",
"rubies","ruby",
"joes","joe",
"skeletons","skeleton",
"templars","templar",
"outlaws","outlaw",
"merchants","merchant",
"hi","hello",
"hail","hello",
"greetings","hello",
"goodbye","bye",
"whos","who",
"thanks","thank",
"mission","quest",
"starting","start",
"damors","damor",
"jamils","jamil",
"sirjans","sirjan",
"point","place",
"1st","first",
"2nd","second",
"3rd","third",
"limitations","limitation",
"limits","limitation",
"limit","limitation",
"quit","exit",
"leave","exit",
"ratlings","ratling",
"eyes","eye",
"helmet","helm",
"shadows","shadow",
"poems","poem"
};

void replace_synonym(char *a)
{
        int n;

        for (n=0; n<sizeof(syn)/sizeof(syn[0]); n+=2) {
                if (!strcmp(a,syn[n])) {
                        strcpy(a,syn[n+1]);
                        return;
                }
        }
}

struct know
{
        char *word[20];         // up to 20 keywords which trigger the response
                                // !word means the word is essential, ?word means the word may or may not be in the input
                                // ? alone shows a question.
                                // example: "!where", "?haunted", "!castle", "?"
                                // "where" and "castle" have to be in the input, while "haunted" may be there or not.

        int value;              // difficulty of the question. used to keep some NPCs from telling everything about an area
        int area;               // area of the game the question belongs to
        int temp;               // ID of the NPC who know the question (example follows)
        char *answer;           // actual text of the answer. Note: Never more than 400 chars!
        int special;            // needs a special function (piece of code) to answer the question
};

/* example:

assume value=50, area=AR_ASTON and temp=42

the question will only get answered if the NPC has a knowledge of at least 50, he knows about AR_ASTON and has the ID 42 */

#define HEALTH          1
#define SHOP            2
#define GREET           3
#define WHOAMI          4
#define WHERE           5
#define STOP            6
#define MOVE            7
#define ATTACK          8
#define WAIT            9
#define FOLLOW          10
#define TIME            11
#define POINTS          12
#define BUYGOLD         13
#define BUYHEALTH       14
#define BUYMANA         15
#define BUYEXP          16
#define TRANSFER        17
#define SPELLINFO       18
#define QUIET           19

#define AR_GENERAL      0
#define AR_THIEF        1
#define AR_CASTLE       2
#define AR_GROLM        3
#define AR_ASTON        4       // locations in aston
#define AR_THIEF2       5
#define AR_TOMB         6
#define AR_JOE          7
#define AR_PURPLE       8
#define AR_SLORD        9
#define AR_OUTLAW       10
#define AR_MMAZE        11
#define AR_LIZARD       12
#define AR_UNDER_I      13
#define AR_KNIGHT       14
#define AR_MINE         15
#define AR_STRONGHOLD   16
#define AR_NEST         17

/* keep these in tune with RIDDLE_MIN_AREA/RIDDLE_MAX_AREA in Lab9.h! */
#define AR_RIDDLE1      21
#define AR_RIDDLE2      22
#define AR_RIDDLE3      23
#define AR_RIDDLE4      24
#define AR_RIDDLE5      25
#define RIDDLE_TEXT     "If you bring me the right volume of the Book of Wisdom, I will tell you a riddle. To answer the riddle, just say the word the riddle asks for. If you answer correctly, I will bring you to the next stage of your quest.\n"

#define AR_ALL          12345

struct know know[]={
{{"!where","!tavern","?",NULL},                         0,      AR_ASTON,       0,      "The tavern lies in the north part of the city, on Temple Street.",0},
{{"!where","!thief","?house","?",NULL},                 0,      AR_ASTON,       0,      "The Thieves House is located at the east end of Temple Street.",0},
{{"!where","?haunted","!castle","?",NULL},              0,      AR_ASTON,       0,      "The Haunted Castle is in the south-east corner of the city, on Castle Way.",0},
{{"!where","?cursed","!tomb","?",NULL},                 0,      AR_ASTON,       0,      "The Cursed Tomb is in the north-east corner of the city, on Rose Street.",0},
{{"!where","!joe","?house","?",NULL},                   0,      AR_ASTON,       0,      "Joe's House is in the middle of Castle Way.",0},
{{"!where","?skeleton","!lord","?",NULL},               0,      AR_ASTON,       0,      "The Skeleton Lord is in the dark, large building in the middle of Merchants Way.",0},
{{"!where","?templar","!outlaw","?",NULL},              0,      AR_ASTON,       0,      "The Templar Outlaws live in a fortified building in the south-west corner of Aston.",0},
{{"!where","?magic","!maze","?",NULL},                  0,      AR_ASTON,       0,      "The Magic Maze is in the north-west corner of Aston.",0},
{{"!where","!labyrinth","?",NULL},                      0,      AR_ASTON,       0,      "The entrance to the Labyrinth is in the middle of Merchants Way.",0},
{{"!where","!random","?dungeon","?",NULL},              0,      AR_ASTON,       0,      "The entrance to the Random Dungeon is in the middle of Merchants Way.",0},
{{"!where","!bank","?",NULL},                           0,      AR_ASTON,       0,      "The bank is in the north west corner of thy city, on Temple Street.",0},
{{"!where","!shop","?",NULL},                           0,      AR_ASTON,       0,      "Most shops are on Merchants Way.",0},
{{"!where","!buy","?",NULL},                            0,      AR_ASTON,       0,      "Most shops are on Merchants Way.",0},
{{"!have","!no","!money","?",NULL},                     0,      AR_ASTON,       0,      "You might find some valuable stuff in the Donations Room in one of the temples.",0},
{{"!where","!temple","!street","?",NULL},               0,      AR_ASTON,       0,      "The Temple Street is in the northern part of the city. It goes from the east to the west end of Aston.",0},
{{"!where","!castle","!way","?",NULL},                  0,      AR_ASTON,       0,      "The Temple Street is in the south-east corner of the city.",0},
{{"!where","!south","!end","?",NULL},                   0,      AR_ASTON,       0,      "South End? That's a street in the south-west corner of Aston.",0},
{{"!where","!rose","!street","?",NULL},                 0,      AR_ASTON,       0,      "Rose Street is on the eastern border of the city.",0},
{{"!where","!merchant","!way","?",NULL},                0,      AR_ASTON,       0,      "The Merchants Way divides the city in the middle. It goes from north to south.",0},
{{"!where","!new","!street","?",NULL},                  0,      AR_ASTON,       0,      "New Street is on the western border of Aston.",0},
{{"?where","?what","!aston","?",NULL},                  0,      AR_ASTON,       0,      "This city is called Aston. But you should know that!",0},
{{"!where","!temple","?skua","?",NULL},                 0,      AR_ASTON,       0,      "The Temple of Skua is in the eastern part of Temple Street.",0},
{{"!where","!jamil","?",NULL},                          0,      AR_ASTON,       0,      "Jamil lives on Temple Street, a bit east of the Temple of Skua.",0},
{{"!who","!jamil","?",NULL},                            0,      AR_ASTON,       0,      "Jamil? He's a merchant. Rumor has it he has some connections to the thieves.",0},
{{"!where","!sirjan","?",NULL},                         0,      AR_ASTON,       0,      "Sirjan lives on Merchants Way. Fairly close to Damor's Shop.",0},
{{"!where","!damor","?shop","?",NULL},                  0,      AR_ASTON,       0,      "Damor's Magic Shop? It's on Merchants Way.",0},
{{"!who","!damor","?",NULL},                            5,      AR_ASTON,       0,      "Damor came into Aston about 60 years ago. Noboby knows where he came from. He is very powerful, so no one dared to ask.",0},
{{"!tell","!damor","?",NULL},                           5,      AR_ASTON,       0,      "Damor came into Aston about 60 years ago. Noboby knows where he came from. He is very powerful, so no one dared to ask.",0},
{{"!ratling","?eye","?",NULL},                          0,      AR_ASTON,       0,      "You'd best ask Robin about that. He lives on South End.",0},
{{"!underground","?",NULL},                             0,      AR_ASTON,       0,      "You'd best ask Robin about that. He lives on South End.",0},
{{"!azrael","?",NULL},                                  0,      AR_ASTON,       0,      "You'd best ask Robin about that. He lives on South End.",0},
{{"!where","!mine","?",NULL},                           0,      AR_ASTON,       0,      "The mine is north of the Temple of Skua.",0},

{{"?what","?where","?good","!start","?place","?",NULL}, 0,      AR_ASTON,       0,      "A good place to start your adventurer's life? You'd best talk to Jamil.",0},
{{"?tell","?where","?good","!start","?place","?",NULL}, 0,      AR_ASTON,       0,      "A good place to start your adventurer's life? You'd best talk to Jamil.",0},

{{"!where","!thief","?house","?",NULL},                 0,      AR_THIEF,       25,     "The Thieves House is located at the east end of Temple Street.",0},
{{"!where","?thief","!house","?",NULL},                 0,      AR_THIEF,       25,     "The Thieves House is located at the east end of Temple Street.",0},
{{"!locked","!door","!thief","?house","?",NULL},        25,     AR_THIEF,       25,     "There are two locked doors in the Thieves House. For the first, you'll easily find the key.",0},
{{"!second","!door","!thief","?house","?",NULL},        25,     AR_THIEF,       25,     "The second locked door in the Thieves House? Don't go there!.",0},
{{"!danger","!thief","?house","?",NULL},                25,     AR_THIEF,       25,     "What would you expect in the Thieves' House? Thieves of course. They're very poor fighters.",0},
{{"!tell","!thief","?house","?",NULL},                  25,     AR_THIEF,       25,     "What would you expect in the Thieves' House? Thieves of course. They're very poor fighters.",0},
{{"!who","!thief","?house","?",NULL},                   25,     AR_THIEF,       25,     "What would you expect in the Thieves' House? Thieves of course. They're very poor fighters.",0},
{{"!why",",?want","!amulet","?",NULL},                  25,     AR_THIEF,       25,     "The thieves stole my amulet. I want it back.",0},
{{"!what","!amulet","?",NULL},                          25,     AR_THIEF,       25,     "It's just a small, golden amulet.",0},
{{"!tell","!amulet","?",NULL},                          25,     AR_THIEF,       25,     "It's just a small, golden amulet.",0},

{{"!where","?haunted","!castle","?",NULL},              0,      AR_CASTLE,      28,     "The Haunted Castle is in the south-east corner of the city, on Castle Way.",0},
{{"!locked","!door","?haunted","?castle","?",NULL},     50,     AR_CASTLE,      28,     "My guess is that you'll have to find the keys for the locked doors in the Haunted Castle, %s.",0},
{{"!key","?haunted","?castle","?",NULL},                50,     AR_CASTLE,      28,     "Not all walls are what they seem to be in the Haunted Castle, %s.",0},
{{"!danger","?haunted","?castle","?",NULL},             25,     AR_CASTLE,      28,     "There are ghosts in the Haunted Castle. They'll curse you! It takes a Corporal to survive that.",0},
{{"!tell","?haunted","?castle","?",NULL},               25,     AR_CASTLE,      28,     "There are ghosts in the Haunted Castle. They'll curse you! It takes a Corporal to survive that.",0},
{{"!who","!in","?haunted","?castle","?",NULL},          25,     AR_CASTLE,      28,     "There are ghosts in the Haunted Castle. They'll curse you! It takes a Corporal to survive that.",0},
{{"!why","?want","!belt","?",NULL},                     25,     AR_CASTLE,      28,     "I need the belt for my research.",0},
{{"!what","!belt","?",NULL},                            25,     AR_CASTLE,      28,     "It's a golden belt with a magical enchantment.",0},
{{"!tell","!belt","?",NULL},                            25,     AR_CASTLE,      28,     "It's a golden belt with a magical enchantment.",0},

{{"!where","?cursed","!tomb","?",NULL},                 0,      AR_TOMB,        50,     "The Cursed Tomb is in the north-east corner of the city, on Rose Street.",0},
{{"!where","!ruby","?cursed","?tomb","?",NULL},         25,     AR_TOMB,        50,     "The ruby is in a hidden area in the Cursed Tomb.",0},
{{"!how","!hidden","?cursed","?tomb","?",NULL},         50,     AR_TOMB,        50,     "There are some illusionary walls in the Cursed Tomb.",0},
{{"!danger","?cursed","?tomb","?",NULL},                25,     AR_TOMB,        50,     "The Cursed Tomb is inhibitated by skeletons. They're poor fighters, but you need strong armor to survive their magic.",0},
{{"!tell","?cursed","?tomb","?",NULL},                  25,     AR_TOMB,        50,     "The Cursed Tomb is inhibitated by skeletons. They're poor fighters, but you need strong armor to survive their magic.",0},
{{"!who","!in","?cursed","?tomb","?",NULL},             25,     AR_TOMB,        50,     "The Cursed Tomb is inhibitated by skeletons. They're poor fighters, but you need strong armor to survive their magic.",0},
{{"!why","?want","!ruby","?",NULL},                     25,     AR_TOMB,        50,     "The ruby will help me with my magical experiments.",0},
{{"!what","!ruby","?",NULL},                            25,     AR_TOMB,        50,     "It's a gem with magical properties. Rubies are used to build magical items.",0},
{{"!tell","!ruby","?",NULL},                            25,     AR_TOMB,        50,     "It's a gem with magical properties. Rubies are used to build magical items.",0},

{{"!where","!joe","?house","?",NULL},                   0,      AR_JOE,         64,     "Joe's House is in the middle of Castle Way.",0},
{{"!danger","?joe","?house","?",NULL},                  25,     AR_JOE,         64,     "It might be wise to assure that Joe will not get any help when you attack him.",0},
{{"!tell","?joe","?house","?",NULL},                    25,     AR_JOE,         64,     "It might be wise to assure that Joe will not get any help when you attack him.",0},
{{"!why","?want","?bronze","!armor""?",NULL},           25,     AR_JOE,         64,     "The bronze armor was a gift from my father. Joe stole it.",0},
{{"!what","?bronze","!armor""?",NULL},                  25,     AR_JOE,         64,     "The bronze armor was a gift from my father. Joe stole it.",0},
{{"!tell","?bronze","!armor""?",NULL},                  25,     AR_JOE,         64,     "The bronze armor was a gift from my father. Joe stole it.",0},

{{"?black","!stronghold","!coin","?",NULL},             25,     AR_STRONGHOLD,  72,     "Bring me one of the black candles from the Stronghold, and I'll give you the star part of the coin.",0},

{{"!help","?need","?stronghold","?",NULL},              0,      AR_STRONGHOLD,  518,    "Every few hours a horde of monsters from the Black Stronghold is attacking our outpost or the city entrance. If you could help PROTECT these places, or - even better - enter the STRONGHOLD and stop the monsters, we'd reward you.",0},
{{"!protect","?outpost","?city","?entrance","?",NULL},  0,      AR_STRONGHOLD,  518,    "Wait close to one of these points till the guards there shout alert. Then help them in the fight. They'll report your success and you can collect your REWARD here afterwards.",0},
{{"!reward","?protect","?city","?stronghold","?",NULL}, 0,      AR_STRONGHOLD,  518,    "Depending on your success you will get money, potions or experience. Ask about your POINTS...",0},
{{"!stronghold","?enter","?attack","?black","?",NULL},  0,      AR_STRONGHOLD,  518,    "The monsters are coming from the Black Stronghold. Some evil magic is creating them. As far as we know, this magic needs black candles. So if you can bring us some black candles we will know you were successful and REWARD you.",0},
{{"!points","?",NULL},                                  0,      AR_STRONGHOLD,  518,    "You have %d points. You can BUY GOLD at one coin per point, BUY HEALING potions for 6 points, BUY MANA potions for 9 points or BUY EXPerience at %d exp per point.",POINTS},
{{"!buy","!gold",NULL},                                 0,      AR_STRONGHOLD,  518,    NULL,BUYGOLD},
{{"!buy","!health",NULL},                               0,      AR_STRONGHOLD,  518,    NULL,BUYHEALTH},
{{"!buy","!healing",NULL},                              0,      AR_STRONGHOLD,  518,    NULL,BUYHEALTH},
{{"!buy","!mana",NULL},                                 0,      AR_STRONGHOLD,  518,    NULL,BUYMANA},
{{"!buy","!exp",NULL},                                  0,      AR_STRONGHOLD,  518,    NULL,BUYEXP},

{{"!where","!mine","?",NULL},                           0,      AR_MINE,        0,      "The mine is north of the Temple of Skua.",0},
{{"!danger","!mine","?",NULL},                          0,      AR_MINE,        0,      "Mining can be dangerous. Place supporting beams to prevent it from collapsing. And don't go into the lower levels too early.",0},
{{"!tell","!mine","?",NULL},                            0,      AR_MINE,        0,      "The mine was opened only a few years ago. At first, it was a very profitable business. But the workers fled when some of them were killed by skeletons.",0},

{{"!where","?skeleton","!lord","?",NULL},               0,      AR_SLORD,       90,     "The Skeleton Lord is in the dark, large building in the middle of Merchants Way.",0},
{{"!where","!potion","?skeleton","?lord","?",NULL},     50,     AR_SLORD,       90,     "The potion the Skeleton Lord has could be in a hidden area.",0},
{{"!danger","?skeleton","?lord","?",NULL},              50,     AR_SLORD,       90,     "The Skeleton Lord still has some guards who will protect him. A bold Sergeant should be able to kill them.",0},
{{"!tell","?skeleton","?lord","?",NULL},                50,     AR_SLORD,       90,     "The Skeleton Lord still has some guards who will protect him. A bold Sergeant should be able to kill them.",0},
{{"!who","!in","?skeleton","?lord","?",NULL},           50,     AR_SLORD,       90,     "The Skeleton Lord still has some guards who will protect him. A bold Sergeant should be able to kill them.",0},
{{"!why","?want","!potion","?",NULL},                   25,     AR_SLORD,       90,     "I created the potion when the lord was still alive. Now that he turned into a skeleton, he has no more need for it.",0},
{{"!what","!potion","?",NULL},                          25,     AR_SLORD,       90,     "It's a greater healing potion. I created the potion when the lord was still alive.",0},
{{"!tell","!potion","?",NULL},                          25,     AR_SLORD,       90,     "It's a greater healing potion. I created the potion when the lord was still alive.",0},

{{"!where","?templar","!outlaw","?",NULL},              0,      AR_OUTLAW,      91,     "The Templar Outlaws live in a fortified building in the south-west corner of Aston.",0},
{{"!danger","?templar","?outlaw","?",NULL},             0,      AR_OUTLAW,      91,     "The Templar Outlaws are very skilled fighters, and there are many of them. A Staff Sergeant could try to attack them.",0},
{{"!tell","?templar","?outlaw","?",NULL},               0,      AR_OUTLAW,      91,     "The Templar Outlaws are very skilled fighters, and there are many of them. A Staff Sergeant could try to attack them.",0},
{{"!why","?want","?barbarian","!sword","?",NULL},       0,      AR_OUTLAW,      91,     "It's a good weapon. I like it. And the templars are outlaws, taking from them is not stealing...",0},
{{"!what","?barbarian","!sword","?",NULL},              0,      AR_OUTLAW,      91,     "The barbarian sword is an old weapon. It was made by an now extinct race of barbarians.",0},
{{"!tell","?barbarian","!sword","?",NULL},              0,      AR_OUTLAW,      91,     "The barbarian sword is an old weapon. It was made by an now extinct race of barbarians.",0},

{{"?second","!door","?thief","?house","?",NULL},        25,     AR_THIEF2,      107,    "The second locked door in the Thieves House? You might have to pick the lock.",0},
{{"!danger","?thief","?house","?",NULL},                25,     AR_THIEF2,      107,    "Be careful behind the second locked door in the Thieves House. The thieves there know how to fight.",0},
{{"!tell","?thief","?house","?",NULL},                  25,     AR_THIEF2,      107,    "Be careful behind the second locked door in the Thieves House. The thieves there know how to fight.",0},
{{"!who","?thief","?house","?",NULL},                   25,     AR_THIEF2,      107,    "Be careful behind the second locked door in the Thieves House. The thieves there know how to fight.",0},
{{"!why","?want","?ruby","!amulet","?",NULL},           25,     AR_THIEF2,      107,    "The Ruby Amulet belongs to me. A few days after I showed it to Jamil it vanished. I'm sure the thieves took it.",0},
{{"!what","?ruby","!amulet","?",NULL},                  25,     AR_THIEF2,      107,    "It's a small golden amulet with a big ruby on it. It increases your ability to cast magic spells.",0},
{{"!tell","?ruby","!amulet","?",NULL},                  25,     AR_THIEF2,      107,    "It's a small golden amulet with a big ruby on it. It increases your ability to cast magic spells.",0},

{{"!where","?magic","!maze","?",NULL},                  0,      AR_MMAZE,       108,    "The Magic Maze is in the north-west corner of Aston.",0},
{{"!danger","?magic","?maze","?",NULL},                 50,     AR_MMAZE,       108,    "The Magic Maze is full of traps. Be careful and look for all clues you can get. There's also a powerful sorceress you will have to fight.",0},
{{"!tell","?magic","?maze","?",NULL},                   50,     AR_MMAZE,       108,    "The Magic Maze is full of traps. Be careful and look for all clues you can get. There's also a powerful sorceress you will have to fight.",0},
{{"!who","?magic","?maze","?",NULL},                    50,     AR_MMAZE,       108,    "The Magic Maze is full of traps. Be careful and look for all clues you can get. There's also a powerful sorceress you will have to fight.",0},
{{"!tell","sorceress","?",NULL},                        50,     AR_MMAZE,       108,    "The sorceress is called Jane. I don't know much about her, only that she killed a few people who survived her maze.",0},
{{"!why","?want","!potion","?",NULL},                   25,     AR_MMAZE,       108,    "It's a very strong mana potion and I need it to complete a certain magic spell I've been studying for several years now.",0},
{{"!what","!potion","?",NULL},                          25,     AR_MMAZE,       108,    "It's a very strong mana potion.",0},
{{"!tell","!potion","?",NULL},                          25,     AR_MMAZE,       108,    "It's a very strong mana potion.",0},

{{"!where","?stone","!sword","?",NULL},                 0,      AR_GENERAL,     109,    "Now where was that damn stone? Let me think. Ah! I remember. It's over there, in that corner.",0},
{{"!where","!stone","?sword","?",NULL},                 0,      AR_GENERAL,     109,    "Now where was that damn stone? Let me think. Ah! I remember. It's over there, in that corner.",0},
{{"?too","?weak",NULL},                                 0,      AR_GENERAL,     109,    "If you're not strong enough to take the sword now, you might need some form of enchantment. Why not go to Damor's Shop and see if he can help?",0},
{{"?not","?strong","?enough",NULL},                     0,      AR_GENERAL,     109,    "If you're not strong enough to take the sword now, you might need some form of enchantment. Why not go to Damor's Shop and see if he can help?",0},
{{"?how","!take","!sword",NULL},                        0,      AR_GENERAL,     109,    "If you're not strong enough to take the sword now, you might need some form of enchantment. Why not go to Damor's Shop and see if he can help?",0},
{{"?how","!get","!sword",NULL},                         0,      AR_GENERAL,     109,    "If you're not strong enough to take the sword now, you might need some form of enchantment. Why not go to Damor's Shop and see if he can help?",0},
{{"!why","?want","?stone","!sword","?",NULL},           0,      AR_GENERAL,     109,    "I've been staring at this stone for years. Can't you imagine I want the matter settled so I can throw the damn stone away?",0},
{{"!what","?stone","!sword","?",NULL},                  0,      AR_GENERAL,     109,    "The sword in the stone in that corner.",0},
{{"!tell","?stone","!sword","?",NULL},                  0,      AR_GENERAL,     109,    "The sword in the stone in that corner.",0},

{{"!how","?create","?mix","?potion","?life","?",NULL},  0,      AR_GENERAL,     111,    "If I knew how to mix the Potion of Life, I'd do it myself. All I can tell you is that you need three rare flowers.",0},
{{"!what","!ingredients","?potion","?life","?",NULL},   0,      AR_GENERAL,     111,    "All I can tell you is that you need three rare flowers to mix the Potion of Life.",0},
{{"!why","?want","!potion","?life","?",NULL},           0,      AR_GENERAL,     111,    "I'm sick. The Potion of Life would cure me.",0},
{{"!where","!potion","?life","?",NULL},                 0,      AR_GENERAL,     111,    "I don't think you can find the Potion of life somewhere. You'll have to create it.",0},

{{"!how","?first","?door","?grolm","?",NULL},           50,     AR_GROLM,       114,    "You need a key to get through the first door in the Grolm Gorge.",0},
{{"!how","!second","?door","?grolm","?",NULL},          75,     AR_GROLM,       114,    "You need a crown to get through the second door in the Grolm Gorge.",0},
{{"!how","!third","?door","?grolm","?",NULL},           75,     AR_GROLM,       114,    "You need a trident to get through the third door in the Grolm Gorge.",0},
{{"!what","!second","?door","?grolm","?",NULL},         75,     AR_GROLM,       114,    "You need a crown to get through the second door in the Grolm Gorge.",0},
{{"!what","!third","?door","?grolm","?",NULL},          75,     AR_GROLM,       114,    "You need a trident to get through the third door in the Grolm Gorge.",0},
{{"!where","!key","?grolm","?",NULL},                   100,    AR_GROLM,       114,    "The key for the first door in the Grolm Gorge is in a very hot place.",0},
{{"!where","!crown","?grolm","?",NULL},                 100,    AR_GROLM,       114,    "The crown from the Grolm Gorge? I'd expect the king wears it.",0},
{{"!where","!trident","?grolm","?",NULL},               100,    AR_GROLM,       114,    "The trident from the Grolm Gorge? The Grolm Mages are said to use tridents.",0},
{{"!what","!key","?grolm","?",NULL},                    100,    AR_GROLM,       114,    "The key is used to open the first door.",0},
{{"!what","!crown","?grolm","?",NULL},                  100,    AR_GROLM,       114,    "The crown from the Grolm Gorge? It's used to open the second door.",0},
{{"!what","!trident","?grolm","?",NULL},                100,    AR_GROLM,       114,    "The trident from the Grolm Gorge? It's used to open the third door.",0},

{{"!how","?first","?door","?lizard","?",NULL},          50,     AR_LIZARD,      162,    "You need a key to get through the first door in the Lizard Gorge.",0},
{{"!how","!second","?door","?lizard","?",NULL},         75,     AR_LIZARD,      162,    "You need a key to get through the second door in the Lizard Gorge.",0},
{{"!how","!third","?door","?lizard","?",NULL},          75,     AR_LIZARD,      162,    "You need a key to get through the third door in the Lizard Gorge.",0},
{{"!where","!key","?lizard","?",NULL},                  100,    AR_LIZARD,      162,    "The Merchants here will give you the keys for special items. Go to them to learn which items you need.",0},
{{"!where","!coconut","?lizard","?",NULL},              100,    AR_LIZARD,      162,    "The coconut? Well, coconuts grow in trees, you know?",0},
{{"!where","!potion","?agility","?",NULL},              100,    AR_LIZARD,      162,    "You have to mix the Potion of Superior Agility from some flower which grow here.",0},
{{"!where","!teeth","?lizard","?",NULL},                100,    AR_LIZARD,      162,    "Lizard's Teeth? I'd assume they have them in their mouths.",0},
{{"!how","?teeth","!necklace","?",NULL},                100,    AR_LIZARD,      162,    "To create the Lizard's Teeth Necklace, you need a leather string and teeth. One of the Merchants sells leather strings.",0},

{{"?ratling","!eye","?",NULL},                          100,    AR_UNDER_I,     246,    "Yes. I need them to create a very powerful stimulant. If you want to help me, buy a collector in Damor's shop and bring me a full set of Ratling's Eyes.",0},
{{"!ratling","?",NULL},                                 100,    AR_UNDER_I,     246,    "The Ratlings live below the city, in the Underground. They look like humans, except for the head, which resembles that of a rat. Be careful, some of the are very strong, and all of them can see in the dark.",0},
{{"!stimulant","?",NULL},                               100,    AR_UNDER_I,     246,    "The stimulant will be extremely powerful. I'll be able to create two potions of it. You'll get one of them as payment.",0},
{{"!potion","?",NULL},                                  100,    AR_UNDER_I,     246,    "The potion will be extremely powerful stimulant. I'll be able to create two potions of it. You'll get one of them as payment.",0},
{{"!payment","?",NULL},                                 100,    AR_UNDER_I,     246,    "I can create two potions from a complete set of eyes. I'll give you one of them as payment.",0},
{{"!how","powerful","?",NULL},                          100,    AR_UNDER_I,     246,    "Very.",0},
{{"!underground","?",NULL},                             100,    AR_UNDER_I,     246,    "The City Guards have recently discovered some holes in the ground of several buildings. They lead into a maze of rooms and corridors. It seems the thieves are using them too.",0},
{{"!thief","?",NULL},                                   100,    AR_UNDER_I,     246,    "A thief was trying to sell information about Azraels Helmet. A City Guard was chasing him, but he vanished in the Thief House. Later searches showed he used a hidden hole in the floor to escape.",0},
{{"!azrael","?helm","?",NULL},                          100,    AR_UNDER_I,     246,    "Lord Azrael of Aston once ruled this city. But he vanished during a fight in his castle. He possesed a powerful helmet, the Helm of Shadows.",0},
{{"!castle","?",NULL},                                  100,    AR_UNDER_I,     246,    "It is now the Haunted Castle. After Azrael vanished, it was abandoned. Now it's inhabitated by ghosts, and some say they hear Azraels cry in there.",0},
{{"!helm","?shadow","?",NULL},                          100,    AR_UNDER_I,     246,    "Lord Azrael's famous helmet, the Helm of Shadows. It's enhanced with powerful magic.",0},

{{"?ratling","!eye","?",NULL},                          100,    AR_UNDER_I,     343,    "Yes. I need it to complete my collection. Robin wants a full set and he promised be a powerful potion in exchange.",0},
{{"!ratling","?",NULL},                                 100,    AR_UNDER_I,     343,    "The Ratlings live below the city, in the Underground. They look like humans, except for the head, which resembles that of a rat. Be careful, some of the are very strong, and all of them can see in the dark.",0},
{{"!underground","?",NULL},                             100,    AR_UNDER_I,     343,    "The City Guards have recently discovered some holes in the ground of several buildings. They lead into a maze of rooms and corridors. It seems the thieves are using them too.",0},
{{"!thief","?",NULL},                                   100,    AR_UNDER_I,     343,    "A thief was trying to sell information about Azraels Helmet. A City Guard was chasing him, but he vanished in the Thief House. Later searches showed he used a hidden hole in the floor to escape.",0},
{{"!azrael","?helm","?",NULL},                          100,    AR_UNDER_I,     343,    "Lord Azrael of Aston once ruled this city. But he vanished during a fight in his castle. He possesed a powerful helmet, the Helm of Shadows.",0},
{{"!castle","?",NULL},                                  100,    AR_UNDER_I,     343,    "It is now the Haunted Castle. After Azrael vanished, it was abandoned. Now it's inhabitated by ghosts, and some say they hear Azraels cry in there.",0},
{{"!helm","?shadow","?",NULL},                          100,    AR_UNDER_I,     343,    "Lord Azrael's famous helmet, the Helm of Shadows. It's enhanced with powerful magic.",0},

{{"?what","!bartering","?",NULL},                       0,      AR_GENERAL,     0,      "Bartering will help you to get better prices from merchants.",0},
{{"?what","!enchant","!weapon","?",NULL},               0,      AR_GENERAL,     0,      "Enchant Weapon is a magic spell. It will make your weapon stronger when you use it.",0},
{{"?what","!recall","?",NULL},                          0,      AR_GENERAL,     0,      "Recall is a magic spell. It will teleport you back to the Temple of Skua when you use it. But beware, there is a small delay between casting and teleport.",0},
{{"?what","!repair","?",NULL},                          0,      AR_GENERAL,     0,      "Repair? That's the ability to repair your equipment.",0},
{{"?what","!stun","?",NULL},                            0,      AR_GENERAL,     0,      "Stun is a magic spell. If you can overcome your target's Resistance, he'll be unable to move for a few seconds.",0},
{{"?what","!lockpicking","?",NULL},                     0,      AR_GENERAL,     0,      "Lock-Picking?. It's used to pick locks, you know?",0},
{{"?what","!identify","?",NULL},                        0,      AR_GENERAL,     0,      "Identify is a magic spell. It'll give you some information about an item or person.",0},
{{"?what","!resistance","?",NULL},                      0,      AR_GENERAL,     0,      "Resistance is used against magic spells. If you're good at it, it's much harder to Curse or Stun you.",0},
{{"?what","!bless","?",NULL},                           0,      AR_GENERAL,     0,      "Bless is a powerful spell. It increases all your abilities.",0},
{{"?what","!curse","?",NULL},                           0,      AR_GENERAL,     0,      "Curse is a powerful spell. It decreases all your abilities.",0},
{{"?what","!guardian","!angel","?",NULL},               0,      AR_GENERAL,     0,      "The spell Guardian Angel will lessen the effects of death on you. You can buy it from Damor, in Aston.",0},
{{"?what","!heal","?",NULL},                            0,      AR_GENERAL,     0,      "Heal is a magic spell. You can use it to heal a persons injuries.",0},
{{"?what","!gate","?",NULL},                            0,      AR_GENERAL,     0,      "The last gate of the Labyrinth.",0},
{{"?what","!labyrinth","?",NULL},                       0,      AR_GENERAL,     0,      "The Labyrinth. It's a huge maze full of dangers. If you survive it, you'll become a Seyan'Du.",0},
{{"?what","!seyandu","?",NULL},                         0,      AR_GENERAL,     0,      "The Seyan'Du are very powerful. They do not have the limitations other people have.",0},
{{"?what","!limitation","?",NULL},                      0,      AR_GENERAL,     0,      "The Harakim are powerful sorcerers, the Templars strong fighters and the Mercenaries have a bit of both. But the Seyan'Du combine their abilities.",0},
{{"?what","!templar","?",NULL},                         0,      AR_GENERAL,     0,      "The Templars are powerful fighters. But they're not very good with magic.",0},
{{"?what","!harakim","?",NULL},                         0,      AR_GENERAL,     0,      "The Harakim are spellcasters. They don't fight very well.",0},
{{"?what","!mercenary","?",NULL},                       0,      AR_GENERAL,     0,      "A Mercenary is both, a fighter and a spellcaster.",0},
{{"!who","!skua","?",NULL},                             0,      AR_GENERAL,     0,      "Skua is the god of justice and order. He fights a perpetual battle against the Purple One.",0},
{{"!who","!purple","!one","?",NULL},                    0,      AR_GENERAL,     0,      "The Purple One? He's the god of chaos and disorder.",0},

{{"?what","!order","?purple","?one","?",NULL},          10,     AR_PURPLE,      180,    "Our order, the Cult of the Purple One, does not believes in rules. Join us, and you can do whatever you want.",0},
{{"?tell","!order","?purple","?one","?",NULL},          10,     AR_PURPLE,      180,    "Our order, the Cult of the Purple One, does not believes in rules. Join us, and you can do whatever you want.",0},
{{"?how","!join","?order","?purple","?one","?",NULL},   10,     AR_PURPLE,      180,    "If you join us, you will be able to kill your fellow players. But beware! Others can kill you as well, and this decision is irrevocable! Do you want to join?",0},
{{"?what","!happens","!join","?order","?purple","?one","?",NULL},       10,     AR_PURPLE,      180,    "If you join us, you will be able to kill your fellow players. But beware! Others can kill you as well, and this decision is irrevocable! Do you want to join?",0},
{{"!yes","!join","?want","!",NULL},                     10,     AR_PURPLE,      180,    "So be it. But you have to pass a test first: Kill me!",0},
{{"!yes","!",NULL},                                     10,     AR_PURPLE,      180,    "So be it. But you have to pass a test first: Kill me!",0},
{{"!kill","?you","?",NULL},                             10,     AR_PURPLE,      180,    "Yes. To join, you have to kill me. Go ahead, you coward!",0},

{{"!poem","?first","?",NULL},                           10,     AR_KNIGHT,      317,    "Where the sun rises in a clouded sky     A mere touch will reveal where hidden lie       A skull which, when enlighted, will give thee    A brave enemy and a precious key.",0},
{{"!poem","?second","?",NULL},                          10,     AR_KNIGHT,      317,    "A dark sky, a large tree                 Weavers work in crimson                         The corner of which guides thee                  To key's holder, the lord's son.",0},
{{"!second","?",NULL},                                  10,     AR_KNIGHT,      317,    "A dark sky, a large tree                 Weavers work in crimson                         The corner of which guides thee                  To key's holder, the lord's son.",0},
{{"!next","?",NULL},                                    10,     AR_KNIGHT,      317,    "A dark sky, a large tree                 Weavers work in crimson                         The corner of which guides thee                  To key's holder, the lord's son.",0},
{{"!other","?",NULL},                                   10,     AR_KNIGHT,      317,    "A dark sky, a large tree                 Weavers work in crimson                         The corner of which guides thee                  To key's holder, the lord's son.",0},

{{"?ice","!egg",NULL},                                  0,      AR_NEST,        615,    "I heard that the ice gargoyles guard an ice egg in their nest. I'd be most grateful if you could bring it to me. But hurry, before it melts!", 0},
{{"?ice","!cloak",NULL},                                0,      AR_NEST,        615,    "The Ice Cloak is a fine piece of armor, but it melts in due time. I can give you one if you obtain an ice egg for me.", 0},

{{"!how","!are","!you","?",NULL},                       0,      AR_GENERAL,     0,      NULL,HEALTH},
{{"!who","!are","!you","?",NULL},                       0,      AR_GENERAL,     0,      NULL,WHOAMI},
{{"!where","!are","!you","?",NULL},                     0,      AR_GENERAL,     0,      NULL,WHERE},
{{"!where","!am","!i","?",NULL},                        0,      AR_GENERAL,     0,      NULL,WHERE},
{{"!buy",NULL},                                         0,      AR_GENERAL,     0,      NULL,SHOP},
{{"!sell",NULL},                                        0,      AR_GENERAL,     0,      NULL,SHOP},
{{"!shop",NULL},                                        0,      AR_GENERAL,     0,      NULL,SHOP},
{{"!exit",NULL},                                        0,      AR_GENERAL,     0,      "Enter the backroom of a tavern to leave the game, %s.",0},
{{"!hello","!","$",NULL},                               0,      AR_GENERAL,     0,      NULL,GREET},
{{"!bye","!","$",NULL},                                 0,      AR_GENERAL,     0,      "Goodbye, %s.",0},
{{"!thank","?you","!","$",NULL},                        0,      AR_GENERAL,     0,      "You're welcome, %s.",0},
{{"!thank",NULL},                                       0,      AR_GENERAL,     0,      "You're welcome, %s.",0},

{{"!stop",NULL},                                        0,      AR_GENERAL, CT_COMPANION, NULL,STOP},
{{"!move",NULL},                                        0,      AR_GENERAL, CT_COMPANION, NULL,MOVE},
{{"!attack",NULL},                                      0,      AR_GENERAL, CT_COMPANION, NULL,ATTACK},
{{"!wait",NULL},                                        0,      AR_GENERAL, CT_COMPANION, NULL,WAIT},
{{"!follow",NULL},                                      0,      AR_GENERAL, CT_COMPANION, NULL,FOLLOW},
{{"!transfer",NULL},                                    0,      AR_GENERAL, CT_COMPANION, NULL,TRANSFER},
{{"!geronimo",NULL},                                    0,      AR_GENERAL, CT_COMPANION, NULL,SPELLINFO},
{{"!time","!what","?",NULL},                            0,      AR_GENERAL, 	0, 	  NULL,TIME},
{{"!quiet",NULL},                                       0,      AR_GENERAL, CT_COMPANION, NULL,QUIET},


{{"!riddle",NULL},                                     10,      AR_RIDDLE1,     899,    RIDDLE_TEXT,0},
{{"!riddles",NULL},                                    10,      AR_RIDDLE1,     899,    RIDDLE_TEXT,0},
{{"!riddle",NULL},                                     10,      AR_RIDDLE2,     905,    RIDDLE_TEXT,0},
{{"!riddles",NULL},                                    10,      AR_RIDDLE2,     905,    RIDDLE_TEXT,0},
{{"!riddle",NULL},                                     10,      AR_RIDDLE3,     911,    RIDDLE_TEXT,0},
{{"!riddles",NULL},                                    10,      AR_RIDDLE3,     911,    RIDDLE_TEXT,0},
{{"!riddle",NULL},                                     10,      AR_RIDDLE4,     912,    RIDDLE_TEXT,0},
{{"!riddles",NULL},                                    10,      AR_RIDDLE4,     912,    RIDDLE_TEXT,0},
{{"!riddle",NULL},                                     10,      AR_RIDDLE5,     913,    RIDDLE_TEXT,0},
{{"!riddles",NULL},                                    10,      AR_RIDDLE5,     913,    RIDDLE_TEXT,0}
};

int obey(int cn,int co)
{
        if (ch[cn].data[63]==co) return 1;
        if ((ch[cn].data[26]&ch[co].kindred) && (ch[cn].data[28]&1)) return 2;
        return 0;
}

void answer_spellinfo(int cn,int co)
{
        int n,in,found=0;

        if (obey(cn,co)==1) {
                for (n=0; n<20; n++) {
                        if ((in=ch[cn].spell[n])) {
                                do_sayx(cn,"%s, for %dm %ds.",
                                        it[in].name,it[in].active/(18*60),(it[in].active/18)%60);
                                found = 1;
                        }
                }
                if (!found) {
                        do_sayx(cn, "I have no spells on me at the moment.");
                }
        }
}

void answer_transfer(int cn,int co)
{
        if (obey(cn,co)==1) {
                do_sayx(cn,"I'd prefer to die in battle, %s. But I shall obey my master.",ch[co].name);
                do_give_exp(co,ch[cn].data[28],1,-1);
                fx_add_effect(6,0,ch[co].x,ch[co].y,0);
                fx_add_effect(7,0,ch[cn].x,ch[cn].y,0);
                die_companion(cn);
                ch[co].luck--;
        }
}

void answer_follow(int cn,int co)
{
        int n;

        if (obey(cn,co)==1) {
                for (n=80; n<92; n++) ch[cn].data[n]=0;

                ch[cn].attack_cn=0;
                ch[cn].goto_x=ch[cn].goto_y=0;
                ch[cn].misc_action=0;

                ch[cn].data[69]=co;
                ch[cn].data[29]=0;

                do_sayx(cn,"Yes, %s!",ch[co].name);
        }
}

void answer_wait(int cn,int co)
{
        int n;

        if (obey(cn,co)==1) {
                for (n=80; n<92; n++) ch[cn].data[n]=0;

                ch[cn].attack_cn=0;
                ch[cn].goto_x=ch[cn].goto_y=0;
                ch[cn].misc_action=0;

                ch[cn].data[29]=ch[cn].x+ch[cn].y*MAPX;
                ch[cn].data[30]=ch[cn].dir;

                ch[cn].data[69]=0;

                do_sayx(cn,"Yes, %s!",ch[co].name);
        }
}

void answer_stop(int cn,int co)
{
        int n;

        if (obey(cn,co)) {
                for (n=80; n<92; n++) ch[cn].data[n]=0;

                ch[cn].attack_cn=0;
                ch[cn].goto_x=ch[cn].goto_y=0;
                ch[cn].misc_action=0;

                ch[cn].data[78]=0;
                ch[cn].data[27]=globs->ticker;

                do_sayx(cn,"Yes master %s!",ch[co].name);
        }
}

void answer_move(int cn,int co)
{
        if (obey(cn,co)) {
                ch[cn].attack_cn=0;
                ch[cn].goto_x=ch[cn].x+4-RANDOM(9);
                ch[cn].goto_y=ch[cn].y+4-RANDOM(9);
                ch[cn].misc_action=0;

                do_sayx(cn,"Yes master %s!",ch[co].name);
        }
}

void answer_attack(int cn,int co,char *text)
{
        int n,best=9999,bestn=0,dist,idx;
        char name[50];

        if (obey(cn,co)) {
                while (isalpha(*text)) text++;
                while (isspace(*text)) text++;

                for (n=0; n<45 && *text; name[n++]=*text++) ;
                name[n]=0;
                if (n<1) return;

                for (n=1; n<MAXCHARS; n++) {
                        if (ch[n].used!=USE_ACTIVE) continue;
                        if (ch[n].flags&CF_BODY) continue;
                        if (!strcasecmp(ch[n].name,name)) {
                                dist=abs(ch[cn].x-ch[n].x)+abs(ch[cn].y-ch[n].y);
                                if (dist<best) {
                                        best=dist;
                                        bestn=n;
                                }
                        }
                }

                if (bestn && best<40) {
                        /* CS, 000209: Prevent attacks on self */
                        if (bestn == co) {
                                do_sayx(cn, "But %s, I would never attack you!", ch[co].name);
                                return;
                        }
                        if (bestn == cn) {
                                do_sayx(cn, "You want me to attack myself? That's silly, %s!", ch[co].name);
                                return;
                        }
                        if (!may_attack_msg(co,bestn,0)) {
                                do_sayx(cn,"The Gods would be angry if we did that, you didn't want to anger the Gods, %s did you?.",ch[co].name);
                                return;
                        }
                        ch[cn].attack_cn=bestn;
                        idx=bestn|(char_id(bestn)<<16);
                        ch[cn].data[80]=idx;
                        do_sayx(cn,"Yes %s, I will kill %s!",ch[co].name,ch[bestn].reference);
//                      do_sayx(cn,ch[cn].text[1],ch[bestn].name);
                        do_notify_char(bestn,NT_GOTMISS,co,0,0,0);
                }
        }
}

void answer_quiet(int cn, int co)
{
        if (!ch[cn].data[CHD_TALKATIVE]) {
                ch[cn].data[CHD_TALKATIVE] = ch_temp[ch[cn].temp].data[CHD_TALKATIVE];
                do_sayx(cn, "Thank you, %s, for letting me talk again!", ch[co].name);
        } else {
                do_sayx(cn, "Yes %s, I will shut up now.", ch[co].name);
                ch[cn].data[CHD_TALKATIVE] = 0;
        }
}


void answer_health(int cn,int co)
{
        if (ch[cn].a_hp>ch[cn].hp[5]*550) do_sayx(cn,"I'm fine, %s.",ch[co].name);
        else if (ch[cn].a_hp>ch[cn].hp[5]*250) do_sayx(cn,"I don't feel so good, %s.",ch[co].name);
        else do_sayx(cn,"I'm dying!!");
}

void answer_shop(int cn,int co)
{
        if (ch[cn].flags&CF_MERCHANT) do_sayx(cn,"Hold down ALT and right click on me to buy or sell, %s.",ch[co].name);
        else do_sayx(cn,"I'm not a merchant, %s.",ch[co].name);
}

void answer_greeting(int cn,int co)
{
/*	changed by SoulHunter 25.04.2000	*/
/*	simplified by DB 1.5.2000 */
        if (ch[cn].text[2][0] && ch[cn].text[2][0]!='#') 
	{
		if((ch[cn].temp == 180) && (ch[co].kindred & KIN_PURPLE))
		{
		    do_sayx(cn,"Greetings, %s!",ch[co].name);
		    return;
		}

	    do_sayx(cn,ch[cn].text[2],ch[co].name);
	    return;
	}
/*	--end	*/
}

void answer_whoami(int cn,int co)
{
        do_sayx(cn,"I am %s.",ch[cn].name);
}

void answer_where(int cn,int co)
{
        do_sayx(cn,get_area(cn,1));
}

void answer_time(int cn,int co)
{
        do_sayx(cn,"Today is the %d%s%s%s%s day of the Year %d. It is %d:%02d Astonian Standard Time.\n",
                globs->mdday,
                (globs->mdday==1 ? "st" : ""),
                (globs->mdday==2 ? "nd" : ""),
                (globs->mdday==3 ? "rd" : ""),
                (globs->mdday>3 ? "th" : ""),
                globs->mdyear,globs->mdtime/3600,(globs->mdtime/60)%60);
}

int stronghold_points(int cn)
{
        return  ch[cn].data[26]/25+     // kills below rank
                ch[cn].data[27]+        // kills at rank
                ch[cn].data[28]*2+      // kills above rank
                ch[cn].data[43]*25-     // candles
                ch[cn].data[41];
}

int stronghold_exp_per_pt(int cn)
{
        return min(125,max(1,(ch[cn].points_tot/45123)));
}


void answer_points(int cn,int co,int nr)
{
        int exp,pts;

        exp=stronghold_exp_per_pt(co);
        pts=stronghold_points(co);

        do_sayx(cn,know[nr].answer,pts,exp);
}

void answer_buygold(int cn,int co)
{
        int pts;

        pts=stronghold_points(co);
        pts=min(100,pts);

        if (pts<1) { do_sayx(cn,"But you don't have any points to spend, %s!",ch[co].name); return; }

        ch[co].data[41]+=pts;
        ch[co].gold+=pts*100;

        do_sayx(cn,"There you are, %s. %d gold coins. Thank you for your help!",ch[co].name,pts);
        chlog(co,"bought gold from cityguard (%d pts)",pts);
}

void answer_buyhealth(int cn,int co)
{
        int pts,in;

        pts=stronghold_points(co);
        if (pts<6) { do_sayx(cn,"But you don't have enough points to spend, %s!",ch[co].name); return; }

        ch[co].data[41]+=6;
        in=god_create_item(101);
        god_give_char(in,co);

        do_sayx(cn,"There you are, %s. A healing potion. Thank you for your help!",ch[co].name);
        chlog(co,"bought healing potion from cityguard");
}

void answer_buymana(int cn,int co)
{
        int pts,in;

        pts=stronghold_points(co);
        if (pts<9) { do_sayx(cn,"But you don't have enough points to spend, %s!",ch[co].name); return; }

        ch[co].data[41]+=9;
        in=god_create_item(102);
        god_give_char(in,co);

        do_sayx(cn,"There you are, %s. A mana potion. Thank you for your help!",ch[co].name);
        chlog(co,"bought mana potion from cityguard");
}

void answer_buyexp(int cn,int co)
{
        int exp,pts;

        pts=stronghold_points(co);
        exp=stronghold_exp_per_pt(co);

        if (pts<1) { do_sayx(cn,"But you don't have any points to spend, %s!",ch[co].name); return; }

        ch[co].data[41]+=pts;
        ch[co].points+=pts*exp;
        ch[co].points_tot+=pts*exp;
        do_check_new_level(co);

        do_sayx(cn,"Now I'll teach you a bit about life, the world and everything, %s. Thank you for your help!",ch[co].name);
        do_char_log(co,2,"You get %d experience points.\n",pts*exp);
        chlog(co,"bought %d exps from cityguard (%d pts)",pts*exp,pts);
}

void special_answer(int cn,int co,int spec,char *word,int nr)
{
        switch(spec) {
                case HEALTH:    answer_health(cn,co); break;
                case SHOP:      answer_shop(cn,co); break;
                case GREET:     answer_greeting(cn,co); break;
                case WHOAMI:    answer_whoami(cn,co); break;
                case WHERE:     answer_where(cn,co); break;
                case STOP:      answer_stop(cn,co); break;
                case MOVE:      answer_move(cn,co); break;
                case ATTACK:    answer_attack(cn,co,word); break;
                case WAIT:      answer_wait(cn,co); break;
                case FOLLOW:    answer_follow(cn,co); break;
                case TIME:      answer_time(cn,co); break;
                case POINTS:    answer_points(cn,co,nr); break;
                case BUYGOLD:   answer_buygold(cn,co); break;
                case BUYHEALTH: answer_buyhealth(cn,co); break;
                case BUYMANA:   answer_buymana(cn,co); break;
                case BUYEXP:    answer_buyexp(cn,co); break;
                case TRANSFER:  answer_transfer(cn,co); break;
                case SPELLINFO: answer_spellinfo(cn,co); break;
                case QUIET:     answer_quiet(cn,co); break;
                default:        break;

        }
}

// no user servicable parts below this line ;-)

void npc_hear(int cn,int co,char *text)
{
        char buf[512];
        char word[20][40];
        int n,cnt,z,flag=1,m;
        int exclam=0,question=0,name=0,misscost,found,hitcost,gotword[20];
        int hit,miss,bestconf=0,bestnr=-1,conf,talk;

        // got keyword meaning stop
        if (!strcasecmp(text,ch[cn].text[6])) {
                for (n=80; n<92; n++) ch[cn].data[n]=0;

                ch[cn].attack_cn=0;
                ch[cn].goto_x=ch[cn].goto_y=0;
                ch[cn].misc_action=0;

                ch[cn].data[78]=0;
                ch[cn].data[27]=globs->ticker;
                if (ch[cn].text[7][0]) do_sayx(cn,ch[cn].text[7]);
                return;
        }

        // dont talk to enemies
        if (!obey(cn,co)) for (n=80; n<92; n++) if ((ch[cn].data[n]&0xffff)==co) return;

        for (n=0; n<20; n++) word[n][0]=0;

        strcpy(buf,text);
        strlower(buf);

        for (z=n=cnt=0; buf[z]; z++) {
                if (buf[z]=='!') { exclam++; continue; }
                if (buf[z]=='?') { question++; continue; }
                if (!isspace(buf[z]) && !isalnum(buf[z])) continue;

                if (!isspace(buf[z]) && n<39) { word[cnt][n++]=buf[z]; flag=0; }
                else {
                        if (!flag) {
                                word[cnt][n]=0;
                                if (is_fillword(word[cnt])) ;
                                else if (!strcasecmp(word[cnt],ch[cn].name)) name=1;
                                else cnt++;
                                n=0;
                                if (cnt==20) break;
                                flag=1;
                        }
                }
        }
        if (n) {
                word[cnt][n]=0;
                if (is_fillword(word[cnt])) ;
                else if (!strcasecmp(word[cnt],ch[cn].name)) name=1;
                else { word[cnt][n]=0; cnt++; }
        }

        for (n=0; n<cnt; n++) replace_synonym(word[n]);

/*      for (n=0; n<cnt; n++) {
                do_sayx(cn,"%d: \"%s\".",cn,word[n]);
        }

        do_sayx(cn,"Question=%d, Exlam=%d, name=%d",question,exclam,name); */

        for (n=0; n<sizeof(know)/sizeof(know[0]); n++) {
                if (ch[cn].data[68]>=know[n].value &&
                    (ch[cn].data[72]==know[n].area || ch[cn].data[72]==AR_ALL || know[n].area==AR_GENERAL) &&
                    (!know[n].temp || know[n].temp==ch[cn].temp)) {

                        hit=miss=0;

                        for (z=0; z<cnt; z++) gotword[z]=0;

                        for (m=0; know[n].word[m]; m++) {
                                if (know[n].word[m][1]==0) {
                                        found=0;
                                        if (know[n].word[m][0]=='?') { if (question) found=1; }
                                        else if (know[n].word[m][0]=='!') { if (exclam) found=1; }
                                        else if (know[n].word[m][0]=='$') { if (name) found=1; }
                                        if (found) hit++; else miss++;
                                } else {
                                        if (know[n].word[m][0]=='?') { misscost=1; hitcost=1; }
                                        else if (know[n].word[m][0]=='!') { misscost=5; hitcost=2; }
                                        else hitcost=misscost=0;

                                        for (z=0; z<cnt; z++)
                                                if (!strcmp(word[z],know[n].word[m]+1)) {
                                                        gotword[z]=1;
                                                        break;
                                                }
                                        if (z==cnt) miss+=misscost;
                                        else hit+=hitcost;
                                }
                        }
                        for (z=0; z<cnt; z++) {
                                if (gotword[z]) hit++;
                                else miss++;
                        }
                        conf=hit-miss;
                        if (conf>bestconf) { bestconf=conf; bestnr=n; }
//                      do_sayx(cn,"%d: %s",conf,know[n].answer);
                }
        }

        talk=ch[cn].data[CHD_TALKATIVE]+name;
        if (obey(cn,co)) talk+=20;

        if (talk>0) {   // only talk if we're talkative or addressed directly
                if (bestconf>0) {
                        if (!know[bestnr].special) { do_sayx(cn,know[bestnr].answer,ch[co].name); chlog(cn,"answered \"%s\" with \"%s\".",text,know[bestnr].answer); }
                        else { special_answer(cn,co,know[bestnr].special,text,bestnr); chlog(cn,"answered \"%s\" with special %d",text,know[bestnr].special); }
                } else {
                        if (name) do_sayx(cn,"I don't know about that.");
                        if (bestconf<=0) chlog(cn,"Could not answer \"%s\".",text);
                }
        }

//      do_sayx(cn,"talk=%d, bestconf=%d",talk,bestconf);
}
