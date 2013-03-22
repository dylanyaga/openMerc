/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include "server.h"

struct s_skilltab skilltab[MAXSKILL]={
        {0,     'C',    "Hand to Hand", "Fighting without weapons.",                    {AT_BRAVE,AT_AGIL,AT_STREN}},
/*u*/   {1,     'C',    "Karate",       "Fighting without weapons and doing damage.",   {AT_BRAVE,AT_AGIL,AT_STREN}},
        {2,     'C',    "Dagger",       "Fighting with daggers or similiar weapons.",   {AT_BRAVE,AT_AGIL,AT_INT}},
        {3,     'C',    "Sword",        "Fighting with swords or similiar weapons.",    {AT_BRAVE,AT_AGIL,AT_STREN}},
/*u*/   {4,     'C',    "Axe",          "Fighting with axes or similiar weapons.",      {AT_BRAVE,AT_STREN,AT_STREN}},
/*u*/   {5,     'C',    "Staff",        "Fighting with staffs or similiar weapons.",    {AT_AGIL,AT_STREN,AT_STREN}},
        {6,     'C',    "Two-Handed",   "Fighting with two-handed weapons.",            {AT_AGIL,AT_STREN,AT_STREN}},

        {7,     'G',    "Lock-Picking", "Opening doors without keys.",                  {AT_INT,AT_WILL,AT_AGIL}},
        {8,     'G',    "Stealth",      "Moving without being seen or heard.",          {AT_INT,AT_WILL,AT_AGIL}},
        {9,     'G',    "Perception",   "Seeing and hearing.",                          {AT_INT,AT_WILL,AT_AGIL}},

/*u*/   {10,    'M',    "Swimming",     "Moving through water without drowning.",       {AT_INT,AT_WILL,AT_AGIL}},
        {11,    'R',    "Magic Shield", "Spell: Create a magic shield.",                {AT_BRAVE,AT_INT,AT_WILL}},

        {12,    'G',    "Bartering",    "Getting good prices from merchants.",          {AT_BRAVE,AT_INT,AT_WILL}},
        {13,    'G',    "Repair",       "Repairing items.",                             {AT_INT,AT_WILL,AT_AGIL}},

        {14,    'R',    "Light",        "Spell: Create light.",                         {AT_BRAVE,AT_INT,AT_WILL}},
        {15,    'R',    "Recall",       "Spell: Teleport to temple.",                   {AT_BRAVE,AT_INT,AT_WILL}},
        {16,    'R',    "Guardian Angel","Spell: Teleport to temple if hp<15%%.",       {AT_BRAVE,AT_INT,AT_WILL}},
        {17,    'R',    "Protection",   "Spell: Enhance Armor of target.",              {AT_BRAVE,AT_INT,AT_WILL}},
        {18,    'R',    "Enhance Weapon","Spell: Enhance Weapon of target.",            {AT_BRAVE,AT_INT,AT_WILL}},
        {19,    'R',    "Stun",         "Spell: Make target motionless.",               {AT_BRAVE,AT_INT,AT_WILL}},
        {20,    'R',    "Curse",        "Spell: Decrease attributes of target.",        {AT_BRAVE,AT_INT,AT_WILL}},
        {21,    'R',    "Bless",        "Spell: Increase attributes of target.",        {AT_BRAVE,AT_INT,AT_WILL}},
        {22,    'R',    "Identify",     "Spell: Read stats of item/character.",         {AT_BRAVE,AT_INT,AT_WILL}},

        {23,    'G',    "Resistance",   "Resist against magic.",                        {AT_BRAVE,AT_WILL,AT_STREN}},

        {24,    'R',    "Blast",        "Spell: Inflict injuries to target.",           {AT_BRAVE,AT_INT,AT_WILL}},
        {25,    'R',    "Dispel Magic", "Spell: Removes all magic from target.",        {AT_BRAVE,AT_INT,AT_WILL}},
        {26,    'R',    "Heal",         "Spell: Heal injuries.",                        {AT_BRAVE,AT_INT,AT_WILL}},
        {27,    'R',    "Ghost Companion","Spell: Create ghostly slave to assist you.", {AT_BRAVE,AT_INT,AT_WILL}},

        {28,    'B',    "Regenerate",   "Regenerate Hitpoints faster.",                 {AT_STREN,AT_STREN,AT_STREN}},
        {29,    'B',    "Rest",         "Regenerate Endurance faster.",                 {AT_AGIL,AT_AGIL,AT_AGIL}},
        {30,    'B',    "Meditate",     "Regenerate Mana faster.",                      {AT_INT,AT_WILL,AT_WILL}},

        {31,    'G',    "Sense Magic",  "Find out who casts what at you.",              {AT_BRAVE,AT_INT,AT_WILL}},
        {32,    'G',    "Immunity",     "Partial immunity against negative magic.",     {AT_BRAVE,AT_AGIL,AT_STREN}},
        {33,    'G',    "Surround Hit", "Hit all your enemies at once.",                {AT_BRAVE,AT_AGIL,AT_STREN}},
        {34,    'G',    "Concentrate",  "Reduces mana cost for all spells.",            {AT_WILL,AT_WILL,AT_WILL}},
        {35,    'G',    "Warcry", 	"Frighten all enemies in hearing distance.",    {AT_BRAVE,AT_BRAVE,AT_STREN}}
};
