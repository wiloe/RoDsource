/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"

#define SLEV 102

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[]		=
{
    { 	"hit",		-1		},  /*  0 */
    {	"slice", 	DAM_SLASH	},	
    {   "stab",		DAM_PIERCE	},
    {	"slash",	DAM_SLASH	},
    {	"whip",		DAM_SLASH	},
    {   "claw",		DAM_SLASH	},  /*  5 */
    {	"blast",	DAM_BASH	},
    {   "pound",	DAM_BASH	},
    {	"crush",	DAM_BASH	},
    {   "grep",		DAM_SLASH	},
    {	"bite",		DAM_PIERCE	},  /* 10 */
    {   "pierce",	DAM_PIERCE	},
    {   "suction",	DAM_BASH	},
    {	"beating",	DAM_BASH	},
    {   "digestion",	DAM_ACID	},
    {	"charge",	DAM_BASH	},  /* 15 */
    { 	"slap",		DAM_BASH	},
    {	"punch",	DAM_BASH	},
    {	"wrath",	DAM_ENERGY	},
    {	"magic",	DAM_ENERGY	},
    {   "divine power",	DAM_HOLY	},  /* 20 */
    {	"cleave",	DAM_SLASH	},
    {	"scratch",	DAM_PIERCE	},
    {   "peck",		DAM_PIERCE	},
    {   "peck",		DAM_BASH	},
    {   "chop",		DAM_SLASH	},  /* 25 */
    {   "sting",	DAM_PIERCE	},
    {   "smash",	DAM_BASH	},
    {   "shocking bite",DAM_LIGHTNING	},
    {	"flaming bite", DAM_FIRE	},
    {	"freezing bite", DAM_COLD	},  /* 30 */
    {	"acidic bite", 	DAM_ACID	},
    {	"chomp",	DAM_PIERCE	},
	/*start spellsong adds*/
	{	"life drain", 	DAM_NEGATIVE	},/*33*/
	{	"flame", 	DAM_FIRE	},
	{	"frost", 	DAM_COLD	},
	{	"shock", 	DAM_LIGHTNING	},/*36*/
	{	"acid", 	DAM_ACID	},
	{	"rending", 	DAM_SLASH	}/*38*/
};

/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",       WIZ_PREFIX,     IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {    "newbies",      WIZ_NEWBIE,     IM },
   {    "spam",         WIZ_SPAM,       L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",        WIZ_FLAGS,      L5 },
   {    "penalties",    WIZ_PENALTIES,  L5 },
   {    "saccing",      WIZ_SACCING,    L5 },
   {    "levels",       WIZ_LEVELS,     IM },
   {    "load",         WIZ_LOAD,       L2 },
   {    "restore",      WIZ_RESTORE,    L2 },
   {    "snoops",       WIZ_SNOOPS,     L2 },
   {    "switches",     WIZ_SWITCHES,   L2 },
   {    "secure",       WIZ_SECURE,     L1 },
   {    NULL,           0,              0  }
};


/* 
 * race table, must put all PC races immediately after unique.
 * Races added by Eris 25 Jan 2000
 */

const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    },
*/
   { "unique",		FALSE, 0, 0, 0, 0, 0, 0, 0, 0 },

    {
	"Dwarf",		TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_MAGIC|RES_POISON|RES_DISEASE, VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
        "Spiritfolk",             TRUE,
        0,              AFF_FLYING|AFF_PASS_DOOR,              0,
        0,              0,              VULN_MAGIC|VULN_HOLY,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"Elf",			TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_CHARM,	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"Human",		TRUE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },
   {

      "Haitorin",              TRUE,
        0,          AFF_HASTE,        0,
	0,          RES_MENTAL,       0,
        A|C|G|H|M|V,       A|B|C|D|E|F|G|H|I|J|K|M|Q|U|V  
   },
   {

      "Tervadi",              TRUE,
        0,          AFF_FLYING,        0,
	0,          0,       VULN_POISON,
        A|C|G|H|M|V,       A|B|C|D|E|F|G|H|I|J|K|M|Q|U|V  
   },
   {

      "Lacerti",              TRUE,
        0,          0,        0,
	IMM_POISON,          0,       0,
        A|C|G|H|M|V,       A|B|C|D|E|F|G|H|I|J|K|M|Q|U|V  
   },
   {

      "Nymph",              TRUE,
        0,          0,         0,
	0,          0,         0,
        A|C|G|H|M|V,       A|B|C|D|E|F|G|H|I|J|K|M|Q|U|V  
   },    
   // note need to fix shapeshifter Eris quick hack 17 March 2000  

   {
     "Shapeshifter",        TRUE,
        0,          0,         0,
        0,          0,         0, 
         A|C|G|H|M|V,       A|B|C|D|E|F|G|H|I|J|K|M|Q|U|V  
   },

   {
	"Drow",		FALSE,
	0,		AFF_DARK_VISION,	0,
	0,		RES_CHARM,	VULN_IRON|VULN_LIGHT,
        A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
   },

    {
	"Giant",		FALSE,
	0,		0,		0,
	0,		RES_FIRE|RES_COLD,	VULN_MENTAL|VULN_LIGHTNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"Hobbit",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_MAGIC|RES_POISON|RES_DISEASE,	VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },


    {
	"Troll",		FALSE,
	0,		AFF_REGENERATION|AFF_INFRARED,
	OFF_BERSERK,
 	0,	RES_CHARM|RES_BASH,	VULN_FIRE|VULN_ACID,
	B|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    {
      "Ghost",                FALSE,
        0,              AFF_PASS_DOOR|AFF_FLYING,    0,
        0,                RES_HOLY||RES_BASH,                VULN_MAGIC,
        I|K|L,          A|B|C|D|E|F|G|H|I|J|K|L|M
    }, 

{
        "myconid",    FALSE,
        0,              0,            0,
        0,              RES_POISON,   VULN_LIGHT|VULN_FIRE,
        A|B|C|G|H|M|cc,     A|B|C|D|E|F|G|H|I|J|K|M|N

},


{
        "Gargoyle",             FALSE,
        0,              AFF_FLYING|AFF_DARK_VISION,   0,
        IMM_LIGHTNING,    RES_POISON,         VULN_ACID|VULN_LIGHT,
        A|G|C|cc,        A|B|C|D|E|F|G|H|I|J|K|U|V|W
    }, 

    {
	"Wolf",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q|V
    },

    {
	"Wyvern",		FALSE,
	0,		AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,	0,	VULN_LIGHT,
	B|Z|cc,		A|C|D|E|F|H|J|K|Q|U|V|X
    },


    {
	"bat",			FALSE,
	0,		AFF_FLYING|AFF_DARK_VISION,	OFF_DODGE|OFF_FAST,
	0,		0,		VULN_LIGHT,
	A|G|W,		A|C|D|E|F|H|J|K|P
    },

    {
	"bear",			FALSE,
	0,		0,		OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,		RES_BASH|RES_COLD,	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V
    },

    {
	"cat",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {
	"centipede",		FALSE,
	0,		AFF_DARK_VISION,	0,
	0,		RES_PIERCE|RES_COLD,	VULN_BASH
    },

    {
	"dog",			FALSE,
	0,		0,		OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|U|V
    },

    {
	"doll",			FALSE,
	0,		0,		0,
	IMM_MAGIC,	RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,	A|B|C|G|H|K
    },

    {
	"fido",			FALSE,
	0,		0,		OFF_DODGE|ASSIST_RACE,
	0,		0,			VULN_MAGIC,
	B|G|V,		A|C|D|E|F|H|J|K|Q|V
    },		
   
    {
	"fox",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|V
    },

    {
	"goblin",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_MAGIC,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"hobgoblin",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE|RES_POISON,	0
    },

    {
	"kobold",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_POISON,	VULN_MAGIC,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q
    },

    {
	"lizard",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V
    },

    {
	"modron",		FALSE,
	0,		AFF_INFRARED,		ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,	0,
	H,		A|B|C|G|H|J|K
    },

    {
	"orc",			FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_LIGHT,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"pig",			FALSE,
	0,		0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K
    },	

    {
	"rabbit",		FALSE,
	0,		0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K
    },

    { 
	"sailor",		FALSE, 
	bb,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"school monster",	FALSE,
	ACT_NOALIGN,		0,		0,
	IMM_CHARM|IMM_SUMMON,	0,		VULN_MAGIC,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U
    },

    {
	"shiriff",		FALSE,
	T,		0,	L|P,
	0,		0, 	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"snake",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|R|X|Y|cc,	A|D|E|F|K|L|Q|V|X
    },
 
    {
	"song bird",		FALSE,
	0,		AFF_FLYING,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {
	"thain",		FALSE,
	T,		0,	L|P,
	0,		0, 	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"water fowl",		FALSE,
	0,		AFF_SWIM|AFF_FLYING,	0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },		
  
    {
	NULL, 0, 0, 0, 0, 0, 0
    }
};

/*
 * New Races Added by Eris 25 Jan 2000
 */

const	struct	pc_race_type	pc_race_table	[]	=
{
    { "null race", "", 0, { 100, 100, 100, 100 },
      { "" }, { 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18 }, 0 },
 
/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills },
	{ base stats },		{ max stats },		size 
    },
*/
  
    {
	"dwarf",	"Dwarf",	4,	{ 150, 100, 125, 100,150,125,200 },
	{ "berserk" },
	{ 1, -1, 1, -3, 2 },	{ 20, 16, 19, 15, 21 }, SIZE_MEDIUM
    },

    // need to fix spiritfolk - quick hack by eris 17 march 2000

    {
        "Spiritfolk",        "Sprit",       9 ,      { 110, 110, 110, 110,110, 110, 200 },
        { "phase" },
        { -1,-1,-1,-1,-1 },    { 20,20,20,20,20 }, SIZE_MEDIUM
    },

    { 	
	"elf",		" Elf ",	4,	{ 100, 125,  100, 120, 110, 125,200 }, 
	{ "sneak", "hide" },
	{ -1, 1, 0, 2, -1 },	{ 16, 20, 18, 21, 17 }, SIZE_MEDIUM
    },

    {
	"human",	"Human",	0,	{100,100,100,100,100,100,200 },
	{ "" },
	{ 0, 0, 0, 0, 0 },	{ 19, 19, 19, 19, 19 },	SIZE_MEDIUM
    },

    {
        "haitorin",        "Htrn ",       7,      { 150, 125 , 100, 100,145,120,200},
        { "sneak", "hide" },
	{ 2, -1, -1, 2, 0 },          { 21, 16, 16, 21, 19 },       SIZE_MEDIUM
    },

    {
        "tervadi",        "Terva",        5,     { 100, 100, 125,140,115,120,200 },
        { "vision" },
	{ -1, 2, 2, 0, -1},          { 16, 21, 21, 19, 16 },       SIZE_MEDIUM
    },

    {
        "Lacerti",        "Lcrti",        7,     { 110, 110, 110, 110, 110, 110, 200 },
        { "" },
	{ 0, 0, 0, 0, 0 },          { 19, 19, 19, 19, 19 },       SIZE_MEDIUM
    },

    {
        "nymph",        "Nymph",        6,      { 100, 100, 100,130,110,110,200 },
        { "captivate" },
	{ -1, 1, 0, 1, 0 },          { 17, 20, 19, 19, 17 },       SIZE_MEDIUM
    },

 // need to fix shapeshifters - quick hack by eris 17 march 2000

    {
        "Shapeshifter",        "Shape",       14,      { 200, 200, 200, 200 , 200, 200, 100},
        { "" },
        { 0,0,0,0,0 },    { 18,18,18,18,18 }, SIZE_MEDIUM
    }

};

	
      	
/*
 * 3 New classes added by Eris on 16 Feb 2000
 */


/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
	"Sorcerer", "Sor",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3018, 9618 },  75,  18, 6,  6,  8, TRUE,
	"sorcerer basics", "sorcerer default"
    },

    {
	"Mystic", "Mys",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 3003, 9619 },  75,  18, 2,  7, 10, TRUE,
	"mystic basics", "mystic default"
    },

    {
	"Thief", "Thi",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3028, 9639 },  75,  18,  -4,  8, 13, TRUE,
	"thief basics", "thief default"
    },

    {
	"Warrior", "War",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 3022, 9633 },  80,  18,  -10,  11, 15, TRUE,
	"warrior basics", "warrior default"
    },


    // note: The following are all quick hacks. Need to set up proper groups and the like for
    // these additional classes. And fix the shapeshifters guild.
    {
      "Alchemist", "Alc", STAT_INT, OBJ_VNUM_SCHOOL_SWORD,
      { 9610, 9618 }, 70, 18, -4, 8, 12, TRUE,
      "alchemist basics", "alchemist default"
    },

    {
      "Templar", "Tmp", STAT_WIS, OBJ_VNUM_SCHOOL_SWORD,
      {9610, 9619 }, 75, 18, -5, 9, 14, TRUE,
      "templar basics", "templar default"
    },

    {
      "Shapeshifter", "Sps", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,
      {9610, 9633 }, 50, 18, -5, 8, 13, TRUE,
      "warrior basics", "warrior default"
    }

};



/*
 * Titles.
 */
char *	const			title_table	[MAX_CLASS][MAX_LEVEL+1][2] =
{
    {

/*0*/	{ "Man",			"Woman"				},
	{ "Newborn",			"Newborn"			},
	{ "Go-for",			"Go-for"			},
	{ "Wannabe",			"Wannabe"			},
	{ "Adolescent",			"Adolescent"			},

/*5*/	{ "Apprentice of Magic",	"Apprentice of Magic"		},
	{ "Spell Student",		"Spell Student"			},
	{ "Reader of Tomes",		"Reader of Tomes"		},
	{ "Practicing Scribe",		"Practicing Scribe"		},
	{ "Scribe of Magic",		"Scribe of Magic"		},

/*10*/	{ "Delver in Spells",		"Delver in Spells"		},
	{ "Scholar of Magic",		"Scholar of Magic"		},
	{ "Medium of Magic",		"Medium of Magic"		},
	{ "Magician",			"Magician"			},
	{ "Warlock",			"Witch"				},
	{ "Seer",			"Seeress"			},

/*15*/	{ "Sage",			"Sage"				},
	{ "Illusionist",		"Illusionist"			},
	{ "Abjurer",			"Abjuress"			},
	{ "Invoker",			"Invoker"			},
	{ "Enchanter",			"Enchantress"			},

/*20*/	{ "Conjurer",			"Conjuress"			},
	{ "Creator",			"Creator"			},
	{ "Savant",			"Savant"			},
	{ "Magus",			"Craftess"			},
	{ "Wizard",			"Wizard"			},

/*25*/	{ "Elder Warlock",		"Elder Witch"			},
	{ "Grand Warlock",		"Grand Witch"			},
	{ "Warlock Prime",		"Witch Prime"			},
	{ "Sorcerer",			"Sorceress"			},
	{ "Elder Sorcerer",		"Elder Sorceress"		},

/*30*/	{ "Grand Sorcerer",		"Grand Sorceress"		},
	{ "Great Sorcerer",		"Great Sorceress"		},
	{ "Golem Maker",		"Golem Maker"			},
	{ "Greater Golem Maker",	"Greater Golem Maker"		},
	{ "Maker of Stones",		"Maker of Stones",		},

/*35*/	{ "Maker of Potions",		"Maker of Potions",		},
	{ "Maker of Scrolls",		"Maker of Scrolls",		},
	{ "Maker of Wands",		"Maker of Wands",		},
	{ "Maker of Staves",		"Maker of Staves",		},
	{ "Demon Summoner",		"Demon Summoner"		},

/*40*/	{ "Greater Demon Summoner",	"Greater Demon Summoner"	},
	{ "Dragon Charmer",		"Dragon Charmer"		},
	{ "Greater Dragon Charmer",	"Greater Dragon Charmer"	},
	{ "Master of all Magic",	"Master of all Magic"		},
	{ "Master Mage",		"Master Mage"			},

/*45*/  { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },

/*50*/  { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },

/*55*/  { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },
        { "Master Mage",                "Master Mage"                   },

/*60*/	{ "Mage Hero",			"Mage Heroine"			},
	{ "Avatar of Magic",		"Avatar of Magic"		},
	{ "Angel of Magic",		"Angel of Magic"		},
	{ "Demigod of Magic",		"Demigoddess of Magic"		},
	{ "Immortal of Magic",		"Immortal of Magic"		},

/*65*/	{ "God of Magic",		"Goddess of Magic"		},
	{ "Deity of Magic",		"Deity of Magic"		},
	{ "Supremity of Magic",		"Supremity of Magic"		},
	{ "Creator",			"Creator"			},
	{ "Implementor",		"Implementress"			}
    },

    {
	{ "Man",			"Woman"				},

	{ "Believer",			"Believer"			},
	{ "Attendant",			"Attendant"			},
	{ "Acolyte",			"Acolyte"			},
	{ "Novice",			"Novice"			},
	{ "Missionary",			"Missionary"			},

	{ "Adept",			"Adept"				},
	{ "Deacon",			"Deaconess"			},
	{ "Vicar",			"Vicaress"			},
	{ "Priest",			"Priestess"			},
	{ "Minister",			"Lady Minister"			},

	{ "Canon",			"Canon"				},
	{ "Levite",			"Levitess"			},
	{ "Curate",			"Curess"			},
	{ "Monk",			"Nun"				},
	{ "Healer",			"Healess"			},

	{ "Chaplain",			"Chaplain"			},
	{ "Expositor",			"Expositress"			},
	{ "Bishop",			"Bishop"			},
	{ "Arch Bishop",		"Arch Lady of the Church"	},
	{ "Patriarch",			"Matriarch"			},

	{ "Elder Patriarch",		"Elder Matriarch"		},
	{ "Grand Patriarch",		"Grand Matriarch"		},
	{ "Great Patriarch",		"Great Matriarch"		},
	{ "Demon Killer",		"Demon Killer"			},
	{ "Greater Demon Killer",	"Greater Demon Killer"		},

	{ "Cardinal of the Sea",	"Cardinal of the Sea"		},
	{ "Cardinal of the Earth",	"Cardinal of the Earth"		},
	{ "Cardinal of the Air",	"Cardinal of the Air"		},
	{ "Cardinal of the Ether",	"Cardinal of the Ether"		},
	{ "Cardinal of the Heavens",	"Cardinal of the Heavens"	},

	{ "Avatar of an Immortal",	"Avatar of an Immortal"		},
	{ "Avatar of a Deity",		"Avatar of a Deity"		},
	{ "Avatar of a Supremity",	"Avatar of a Supremity"		},
	{ "Avatar of an Implementor",	"Avatar of an Implementor"	},
	{ "Master of all Divinity",	"Mistress of all Divinity"	},

	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},	
	{ "Master Cleric",		"Master Cleric"			},

	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},

	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},
	{ "Master Cleric",		"Master Cleric"			},

	{ "Holy Hero",			"Holy Heroine"			},
	{ "Holy Avatar",		"Holy Avatar"			},
	{ "Angel",			"Angel"				},
	{ "Demigod",			"Demigoddess",			},
	{ "Immortal",			"Immortal"			},
	{ "God",			"Goddess"			},
	{ "Deity",			"Deity"				},
	{ "Supreme Master",		"Supreme Mistress"		},
        { "Creator",                    "Creator"                       },
	{ "Implementor",		"Implementress"			}
    },

    {
	{ "Man",			"Woman"				},

	{ "Pilferer",			"Pilferess"			},
	{ "Footpad",			"Footpad"			},
	{ "Filcher",			"Filcheress"			},
	{ "Pick-Pocket",		"Pick-Pocket"			},
	{ "Sneak",			"Sneak"				},

	{ "Pincher",			"Pincheress"			},
	{ "Cut-Purse",			"Cut-Purse"			},
	{ "Snatcher",			"Snatcheress"			},
	{ "Sharper",			"Sharpress"			},
	{ "Rogue",			"Rogue"				},

	{ "Robber",			"Robber"			},
	{ "Magsman",			"Magswoman"			},
	{ "Highwayman",			"Highwaywoman"			},
	{ "Burglar",			"Burglaress"			},
	{ "Thief",			"Thief"				},

	{ "Knifer",			"Knifer"			},
	{ "Quick-Blade",		"Quick-Blade"			},
	{ "Killer",			"Murderess"			},
	{ "Brigand",			"Brigand"			},
	{ "Cut-Throat",			"Cut-Throat"			},

	{ "Spy",			"Spy"				},
	{ "Grand Spy",			"Grand Spy"			},
	{ "Master Spy",			"Master Spy"			},
	{ "Assassin",			"Assassin"			},
	{ "Greater Assassin",		"Greater Assassin"		},

	{ "Master of Vision",		"Mistress of Vision"		},
	{ "Master of Hearing",		"Mistress of Hearing"		},
	{ "Master of Smell",		"Mistress of Smell"		},
	{ "Master of Taste",		"Mistress of Taste"		},
	{ "Master of Touch",		"Mistress of Touch"		},

	{ "Crime Lord",			"Crime Mistress"		},
	{ "Infamous Crime Lord",	"Infamous Crime Mistress"	},
	{ "Greater Crime Lord",		"Greater Crime Mistress"	},
	{ "Master Crime Lord",		"Master Crime Mistress"		},
	{ "Godfather",			"Godmother"			},

        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },

        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },

        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },
        { "Master Thief",               "Master Thief"                  },

	{ "Assassin Hero",		"Assassin Heroine"		},
	{ "Avatar of Death",		"Avatar of Death",		},
	{ "Angel of Death",		"Angel of Death"		},
	{ "Demigod of Assassins",	"Demigoddess of Assassins"	},
	{ "Immortal Assasin",		"Immortal Assassin"		},
	{ "God of Assassins",		"God of Assassins",		},
	{ "Deity of Assassins",		"Deity of Assassins"		},
	{ "Supreme Master",		"Supreme Mistress"		},
        { "Creator",                    "Creator"                       },
	{ "Implementor",		"Implementress"			}
    },

    {
	{ "Man",			"Woman"				},

	{ "Swordpupil",			"Swordpupil"			},
	{ "Recruit",			"Recruit"			},
	{ "Sentry",			"Sentress"			},
	{ "Fighter",			"Fighter"			},
	{ "Soldier",			"Soldier"			},

	{ "Warrior",			"Warrior"			},
	{ "Veteran",			"Veteran"			},
	{ "Swordsman",			"Swordswoman"			},
	{ "Fencer",			"Fenceress"			},
	{ "Combatant",			"Combatess"			},

	{ "Hero",			"Heroine"			},
	{ "Myrmidon",			"Myrmidon"			},
	{ "Swashbuckler",		"Swashbuckleress"		},
	{ "Mercenary",			"Mercenaress"			},
	{ "Swordmaster",		"Swordmistress"			},

	{ "Lieutenant",			"Lieutenant"			},
	{ "Champion",			"Lady Champion"			},
	{ "Dragoon",			"Lady Dragoon"			},
	{ "Cavalier",			"Lady Cavalier"			},
	{ "Knight",			"Lady Knight"			},

	{ "Grand Knight",		"Grand Knight"			},
	{ "Master Knight",		"Master Knight"			},
	{ "Paladin",			"Paladin"			},
	{ "Grand Paladin",		"Grand Paladin"			},
	{ "Demon Slayer",		"Demon Slayer"			},

	{ "Greater Demon Slayer",	"Greater Demon Slayer"		},
	{ "Dragon Slayer",		"Dragon Slayer"			},
	{ "Greater Dragon Slayer",	"Greater Dragon Slayer"		},
	{ "Underlord",			"Underlord"			},
	{ "Overlord",			"Overlord"			},

	{ "Baron of Thunder",		"Baroness of Thunder"		},
	{ "Baron of Storms",		"Baroness of Storms"		},
	{ "Baron of Tornadoes",		"Baroness of Tornadoes"		},
	{ "Baron of Hurricanes",	"Baroness of Hurricanes"	},
	{ "Baron of Meteors",		"Baroness of Meteors"		},

	{ "Master Warrior",		"Master Warrior"		},
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },

        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },

        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },
        { "Master Warrior",             "Master Warrior"                },

	{ "Knight Hero",		"Knight Heroine"		},
	{ "Avatar of War",		"Avatar of War"			},
	{ "Angel of War",		"Angel of War"			},
	{ "Demigod of War",		"Demigoddess of War"		},
	{ "Immortal Warlord",		"Immortal Warlord"		},
	{ "God of War",			"God of War"			},
	{ "Deity of War",		"Deity of War"			},
	{ "Supreme Master of War",	"Supreme Mistress of War"	},
        { "Creator",                    "Creator"                       },
	{ "Implementor",		"Implementress"			}
    },
{
/*0*/   { "Man",                        "Woman"                         },
        { "Newborn",                    "Newborn"                       }
},

{
/*0*/   { "Man",                        "Woman"                         },
        { "Newborn",                    "Newborn"                       }
}

};



/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  3, 225, 30 },
    {  3,  3, 250, 35 }, /* 20  */
    {  4,  4, 300, 40 },
    {  4,  4, 350, 45 },
    {  5,  4, 400, 50 },
    {  5,  4, 450, 55 },
    {  5,  5, 500, 60 }  /* 25   */
};



const	struct	int_app_type	int_app		[26]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 }	/* 25 */
};



const	struct	wis_app_type	wis_app		[26]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 10 */
    { 2 },
    { 2 },
    { 3 },
    { 3 },
    { 3 },	/* 15 */
    { 3 },
    { 3 },
    { 4 },	/* 18 */
    { 4 },
    { 4 },	/* 20 */
    { 4 },
    { 5 },
    { 5 },
    { 5 },
    { 6 }	/* 25 */
};



const	struct	dex_app_type	dex_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};


const	struct	con_app_type	con_app		[26]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 }    /* 25 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
    { "water",			"clear",	{  0, 1, 10 }	},  /*  0 */
    { "beer",			"amber",	{  3, 2,  5 }	},
    { "wine",			"rose",		{  5, 2,  5 }	},
    { "ale",			"brown",	{  2, 2,  5 }	},
    { "dark ale",		"dark",		{  1, 2,  5 }	},

    { "whisky",			"golden",	{  6, 1,  4 }	},  /*  5 */
    { "lemonade",		"pink",		{  0, 1,  8 }	},
    { "firebreather",		"boiling",	{ 10, 0,  0 }	},
    { "local specialty",	"everclear",	{  3, 3,  3 }	},
    { "slime mold juice",	"green",	{  0, 4, -8 }	},

    { "milk",			"white",	{  0, 3,  6 }	},  /* 10 */
    { "tea",			"tan",		{  0, 1,  6 }	},
    { "coffee",			"black",	{  0, 1,  6 }	},
    { "blood",			"red",		{  0, 2, -1 }	},
    { "salt water",		"clear",	{  0, 1, -2 }	},

    { "cola",			"cherry",	{  0, 1,  5 }	},   /* 15 */
	{ "mocha latte",			"brown",	{  0, 1,  5 }	},
	{ "vodka",			"clear",	{  7, 1,  5 }	},
	{ "honey mead",			"amber",	{  4, 1,  5 }	},
	{ "green tea",			"amber",	{  0, 1,  5 }	},
	{ "fruit juice",			"cherry",	{  0, 1,  5 }	},/* 20 */
	{ "hot chocolate",			"brown",	{  0, 1,  5 }	}
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

/*
 * Components: Akira 2000
 *
 *   The components string. If no parts are required for the spell,
 * or for brewing/scribing the spell, leave this as NULL.
 *
 *    Otherwise, the string is composed of a letter and a vnum, with
 * spaces for multiples. The different letters are:
 *   E: Less damage is done if this vnum not present
 *   S: Require this vnum for casting
 *   B: Require this vnum for brewing/scribing
 *      Note: All required for casting are required for brewing/scribing
 *            automatically.
 *	Example: "S134 S253 B175 E191"
 *		This means you must have objects 134 and 253 to cast,
 *		And objects 134, 253, and 175 to brew/scribe.
 *		The spell will be weaker if you do not have obj 191.
 */

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
 * Magic spells.
 */

	{
	"reserved",		 { SLEV,SLEV, SLEV, SLEV, SLEV, SLEV, SLEV },
{ 99, 99, 99, 99, 99, 99, 99 },
	0,			TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( 0),	 0,	 0,
	"",			"",	NULL
    },

//Religion Spells:

//Aristea
	{
	"avenging angel",	{ 60, 60 , 60, 60, 60, 60, 60},
	{ 1, 1, 1, 1, 1, 1, 1 },	spell_avenging_angel,
TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_avenging_angel,			SLOT(0),	30,	7,
"heavenly curse",	"The heavenly curse lifts.", NULL
    },


	{
	"fasting", { 15,15, 15,15,15,15,15  },
	{ 1,  1,  1,  1, 1, 1, 1}, spell_fasting,
TAR_CHAR_DEFENSIVE,
POS_STANDING,
	NULL,			SLOT(0),	5,	0,	"",
"", NULL
    },


	{
	"faith", { 35,35, 35,35,35,35,35  },
	{ 1,  1,  1,  1, 1, 1, 1}, spell_faith,
TAR_CHAR_SELF,
POS_STANDING,
	NULL,			SLOT(0),	25,	0,	"",
"You return from your deep assurance of faith.", NULL
    },


	/***************
	*TEMPLAR SPELLS*
	***************/

	//Protective

	{
	"holy aura", { SLEV,SLEV, SLEV, SLEV, SLEV,9, SLEV  },
	{ 0,  0,  0,  0, 0, 2, 0}, spell_holy_aura,	TAR_CHAR_SELF,	POS_FIGHTING,
	NULL,			SLOT(200),	5,	0,	"",		"You feel less holy.", NULL
    },

	{
	"divine protection", { SLEV,SLEV, SLEV, SLEV, SLEV,78, SLEV  },
	{ 0,  0,  0,  0, 0, 2, 0}, spell_divine_protection,	TAR_CHAR_SELF,	POS_FIGHTING,
	NULL,			SLOT(201),	15,	0,	"",		"The protection of the Gods fade.", NULL
    },

	//healing spells

	{
	"lay hands", { SLEV,SLEV, SLEV, SLEV, SLEV,1, SLEV  },
	{ 0,  0,  0,  0,  0, 1, 0},	spell_lay_hands,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(202),	15,	0,	"",			"!Lay Hands!", NULL
    },

	{
	"aid", { SLEV,SLEV, SLEV, SLEV, SLEV,38, SLEV  },
	{ 0,  0,  0,  0,  0, 2, 0},	spell_aid,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(203),	25,	0,	"",			"!Aid!", NULL
    },

	{
	"divine intervention", { SLEV,SLEV, SLEV, SLEV, SLEV,70, SLEV  },
	{ 0,  0,  0,  0,  0, 3, 0},	spell_divine_intervention,	TAR_CHAR_DEFENSIVE,POS_FIGHTING,
	NULL,			SLOT(204),	50,	0,	"",			"!Divine Intervention!", NULL
    },

	//harm

	{
	"bruise", { SLEV,SLEV, SLEV, SLEV, SLEV,4, SLEV  },
	{ 0,  0,  0,  0,  0, 1, 0},	spell_bruise,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(205),	15,	0,	"spell", "!Bruise!", "E8001" 
    },

	{
	"wound", { SLEV,SLEV, SLEV, SLEV, SLEV,40, SLEV  },
	{ 0,  0,  0,  0,  0, 2, 0},	spell_wound,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(206),	25,	0,	"spell", "!Wound!", "E8002" 
    },

	{
	"wither", { SLEV,SLEV, SLEV, SLEV, SLEV,72, SLEV  },
	{ 0,  0,  0,  0,  0, 3, 0},	spell_wither,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(207),	50,	0,	"spell", "!Wither!", NULL
    },

	//benediction
	{
	"inspire", { SLEV,SLEV, SLEV, SLEV, SLEV,20, SLEV  },
	{ 0,  0,  0,  0,  0, 2, 0},	spell_inspire,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(208),	15,	0,	"",			"You feel less inspired.", NULL
    },

	{
	"holy wrath", { SLEV,SLEV, SLEV, SLEV, SLEV,48, SLEV  },
	{ 0,  0,  0,  0,  0, 3, 0},	spell_holy_wrath,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(209),	15,	0,	"",			"The wrath of the Gods subsides.", NULL
    },

	{
	"divine grace", { SLEV,SLEV, SLEV, SLEV, SLEV,77, SLEV  },
	{ 0,  0,  0,  0,  0, 3, 0},	spell_divine_grace,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(210),	25,	0,	"",			"You feel less graceful.", NULL
    },

	//creation
/*	
	{
	"illuminate", { SLEV,SLEV, SLEV, SLEV, SLEV,7, SLEV  },
	{ 0,  0,  0,  0,  0, 1, 0},	spell_illuminate,	TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(211),	10,	0,	"",			"!Illuminate!", NULL
    },
*/
	{
	"call mount", { SLEV,SLEV, SLEV, SLEV, SLEV,1, SLEV  },
	{ 0,  0,  0,  0,  0, 3, 0},	spell_call_mount,	TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(212),	100,	0,"","!Call Mount!", NULL
    },

	{
	"satiate", { SLEV,SLEV, SLEV, SLEV, SLEV,75, SLEV  },
	{ 0,  0,  0,  0,  0, 3, 0},	spell_satiate,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(213),	50,	0,	"",			"!Satiate!", NULL
    },

	//maladiction|curative
	{
	"vex", { SLEV,SLEV, SLEV, SLEV, SLEV,34, SLEV  },
	{ 0,  0,  0,  0,  0, 3, 0},	spell_vex,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(214),	25,	0,	"", "!Vex!", NULL
    },

	/*end templar add*/

	/**************
	*MYSTIC SPELLS*
	**************/

	//healing
	{
	"patch wounds",	{ SLEV,1, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  1,  0,  0,0,0,0},	spell_patch_wounds,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(16),	15,	0,	"",			"!Patch Wounds", NULL
        },

	{
	"bind wounds",		{ SLEV,22, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  1,  0,  0,0,0,0},	spell_bind_wounds,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(61),	20,	0,	"",			"!Bind Wounds!", NULL
    },

	{
	"mend wounds",		{ SLEV,37, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  1,  0,  0,0,0,0},	spell_mend_wounds,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(15),	25,	0,	"",			"!Mend Wounds!", NULL
    },

	{
	"repair wounds",		{ SLEV,46, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  2,  0,  0,0,0,0},	spell_repair_wounds,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(28),	50,	0,	"",			"!Repair Wounds!", NULL
    },

	{
	"refresh",		{ 12,5, SLEV, SLEV, SLEV,16, SLEV  },
	{ 2,  1,  0,  0,0,2,0},	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(81),	12,	0,	"refresh",		"!Refresh!"
    },


	//advanced heal
	
	{
	"mass healing",		{ SLEV,61, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  2,  0,  0,0,0,0},	spell_mass_healing,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(508),	100,	0,	"",			"!Mass Healing!"
    },

	{
	"restoration",		{ SLEV,69, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  2,  0,  0,0,0,0},	spell_restoration,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(215),	75,	0,	"",			"!Restore!", NULL
    },

	{
	"regeneration",		{ SLEV,79, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  3,  0,  0,0,0,0},	spell_regeneration,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(509),	50,	12,	"",			"Your metabolism decreases.", NULL
    },

	{
	"succor",		{ SLEV,95, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  3,  0,  0,0,0,0},	spell_succor,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(216),	100,	12,	"",			"!Succor!", NULL
    },
	
	{
	"energy transferance",		{ SLEV,66, SLEV, SLEV, SLEV,SLEV, SLEV  },
	{ 0,  3,  0,  0,0,0,0},	spell_energy_transferance,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(217),	35,	12,	"",			"!Energy Transferance!", NULL
    },

	//harm
	{
	"mar", { SLEV, 2, SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0,  1,  0,  0,  0, 0, 0},	spell_mar,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(62),	15,	10,	"spell", "!Mar!", NULL
    },

	{
	"abuse", { SLEV, 23, SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0,  1,  0,  0,  0, 0, 0},	spell_abuse,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(64),	20,	10,	"abuse spell", "!Abuse!", NULL
    },

    {
	"injure", { SLEV, 37, SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0,  1,  0,  0,  0, 0, 0},	spell_injure,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(63),	25,	10,	"injure spell", "!Injure!", NULL
    },

	{
	"harm", { SLEV, 47, SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0,  2,  0,  0,  0, 0, 0},	spell_harm,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(27),	35,	10,	"harm spell", "!Harm!", NULL
    },
		
	{
	"inflict", { SLEV, 69, SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0,  2,  0,  0,  0, 0, 0},	spell_inflict,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(218),	50,	14,	"infliction spell", "!Inflict!", NULL
    },

	{
	"avenge", { SLEV, 90, SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0,  3,  0,  0,  0, 0, 0},	spell_avenge,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(219),	75,	14,	"vengance", "!Avenge!", NULL
    },

	//curative

	{
	"awaken",	{ SLEV, 18, SLEV, SLEV, SLEV, 22, SLEV  },
	{ 0,  2,  0,  0, 0, 2, 0},	spell_awaken,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(220),	 15,	0,	"",			"!Awaken!", NULL
    }, //awaken needs an empty slot

	{
	"cure blindness",	{ SLEV, 27 , SLEV, SLEV, SLEV, 46 , SLEV  },
	{ 0,  1,  0,  0,0,3,0},	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(14),	 15,	0,	"",			"!Cure Blindness!", NULL
    },
    
    {
	"cure disease",		{ SLEV, 63, SLEV, SLEV, SLEV, 58, SLEV  },
	{ 0,  1,  0,  0, 0, 3, 0},	spell_cure_disease,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(501),	20,	0,	"",			"!Cure Disease!", NULL
    },

    {
	"cure poison",		{ SLEV, 56, SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0,  1,  0,  0, 0, 0, 0},	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(43),	 5,	0,	"",			"!Cure Poison!", NULL
    },

	//protective

	{
	"barkskin", { SLEV,5 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 1, 0, 0, 0, 0}, spell_barkskin,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(1),	5,	0,	"",		"The bark leaves your skin.", NULL
    },

	{
	"cancellation",		{ 16, 17, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 1,  2,  0, 0, 0, 0, 0}, spell_cancellation,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(507),	20,	12,	""			"!cancellation!", NULL
    },

	{
	"dispel magic",		{ 11, 11, SLEV, SLEV, SLEV, SLEV, SLEV },  
	{ 1,  2,  0,  0, 0, 0, 0},	spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(59),	15,	12,	"",			"!Dispel Magic!", NULL
    },

	{
	"preservation",		{ 9, 10, SLEV, SLEV, SLEV, 62, SLEV }, 
    { 1,  1,  0,  0, 0, 2, 0},	spell_preservation,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(900),	75,	0,	"",			"Your feeling of preservation dies.", NULL
    },
	
	{
	"protection good",	{ 22, 24, SLEV, SLEV, SLEV, 24, SLEV },
	{ 1,  1,  0,  0, 0, 2, 0},	spell_protection_good,	TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,			SLOT(221),	 5,	0,	"",			"You feel less protected.", NULL
    },

	{
	"protection evil",	{ 22, 24, SLEV, SLEV, SLEV, 24, SLEV },
	{ 1,  1,  0,  0, 0, 2, 0},	spell_protection_evil,	TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,			SLOT(34),	 5,	0,	"",			"You feel less protected.", NULL
    },

	{
	"psychic shield", { SLEV, 43 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 1, 0, 0, 0, 0, 0}, spell_psychic_shield,	TAR_CHAR_SELF,	POS_FIGHTING,
	NULL,			SLOT(222),	10,	0,	"",		"Your shield of mental energy disappears.", NULL
    },

	{
	"sanctuary",		{ 43, 45, SLEV, SLEV, SLEV, SLEV, SLEV }, 
    { 2,  2,  0,  0, 0, 0, 0},	spell_sanctuary,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(36),	75,	0,	"", "The white aura around your body fades.", NULL 
    },

	{
	"stone skin",		{ 58, 67, SLEV, SLEV, SLEV, SLEV, SLEV }, 
    { 2,  2,  0,  0, 0, 0, 0},	spell_stone_skin,	TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,			SLOT(66),	30,	0,	"",			"Your skin feels soft again.", NULL
    },

	/*weather*/
	{
	"call lightning",	{ SLEV, 49, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_call_lightning,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT( 6),	20,	12,	"lightning bolt",	"!Call Lightning!", NULL 
    },

	{
	"control weather",	{ SLEV, 41, SLEV, SLEV, SLEV, SLEV, SLEV }, 
    { 0,  2,  0,  0, 0, 0, 0 },	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(11),	25,	12,	"",			"!Control Weather!",NULL
    },

	{
	"faerie fire",		{ SLEV, 20, SLEV, SLEV, SLEV, SLEV, SLEV },    
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_faerie_fire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(72),	 5,	12,	"faerie fire",		"The pink aura around you fades away.",NULL
    },

    {
	"faerie fog",		{ SLEV, 36, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  2,  0,  0, 0, 0, 0 },	spell_faerie_fog,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(73),	12,	12,	"faerie fog",		"!Faerie Fog!",NULL
    },

	{
	"tornado",	{ SLEV, 68, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  3,  0,  0, 0, 0, 0 },	spell_tornado,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(223),	75,	12,	"wind blast",	"!Tornado!",NULL
    },

	{
	"typhoon",	{ SLEV, 85, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  3,  0,  0, 0, 0, 0 },	spell_typhoon,	TAR_CHAR_OFFENSIVE, POS_FIGHTING,
	NULL,			SLOT(224),	100,	12,	"typhoon",	"!Typhoon!",NULL
    },

	//elemental
	
	{
	"geyser",	{ SLEV, 10, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_geyser,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(225),	15,	12,	"water blast",	"!Geyser!",NULL
    }, //needs a slot

	{
	"flamestrike",		{ SLEV, 20, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_flamestrike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(65),	15,	12,	"flamestrike",		"!Flamestrike!", NULL
    },

	{
	"earthquake",		{ SLEV, 30, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  2,  0,  0, 0, 0, 0 },	spell_earthquake,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(23),	25,	12,	"earthquake",		"!Earthquake!", "E8019" 
    },

	{
	"ice shield", { SLEV, 40 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0}, spell_ice_shield,	TAR_CHAR_SELF,	POS_FIGHTING,
	NULL,			SLOT(226),	30,	0,	"",		"Your ice shield disappears.", NULL
    }, 

	{
	"fire shield", { SLEV, 50 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0}, spell_fire_shield,	TAR_CHAR_SELF,	POS_FIGHTING,
	NULL,			SLOT(227),	30,	0,	"",		"Your fire shield disappears.", NULL
    }, 

	{
	"sandblast",	{ SLEV, 61, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  2,  0,  0, 0, 0, 0 },	spell_sandblast,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(228),	50,	12,	"sandblast",	"!Sandblast!",NULL
    }, 

	{
	"summon earth elemental",	{ SLEV, 70, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  3,  0,  0, 0, 0, 0 },	spell_summon_earth_elemental,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(229),	150,	20,	"",	"!Summon Earth Elemental!",NULL
    }, //needs a slot

	//Psychic Group
	{
	"chakra", { SLEV, 12 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0}, spell_chakra,	TAR_CHAR_SELF,	POS_FIGHTING,
	NULL,			SLOT(230),	30,	10,	"",		"You feel more vulnerable.", NULL
    }, 
/*
	{
	"mind shield", { SLEV, 25 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 3, 0, 0, 0, 0, 0}, spell_mind_shield,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,			SLOT(231),	30,	10,	"",		"Your magicks seem weaker.", NULL
    }, 

	{
	"deflect", { SLEV, 38 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 4, 0, 0, 0, 0, 0}, spell_deflect,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,			SLOT(232),	30,	0,	"",		"You are no longer turning spells.", NULL
    }, 
*/	
	{
	"enslave",		{ SLEV, 55 ,SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  3,  0,  0, 0, 0, 0 },	spell_enslave,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 233 ),	 75, 12, "",	"You feel in control fo your mind.", NULL
    },

	{
	"spirit shield", { SLEV, 72 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 }, spell_spirit_shield,	TAR_CHAR_SELF,	POS_FIGHTING,
	NULL,			SLOT(234),	30,	10,	"",		"Your spirit feels vulnerable.", NULL
    },
	
	//divination
/*
	{
	"aura",		{ SLEV, 28 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },	spell_aura,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(235),	 15,	10,	"",			"You no longer can see auras.", NULL
    },

	{
	"revelation",		{ SLEV, 78 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },	spell_revelation,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(236),	 20,	10,	"",			"Your revelation fades.", NULL
    },

	{
	"foresight",		{ SLEV, 72 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 3, 0, 0, 0, 0, 0 },	spell_foresight,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(237),	 30,	0,	"",			"!Foresight!", NULL
    },
*/
	{
	"vision",		{ SLEV, 58 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 3, 0, 0, 0, 0, 0 },	spell_vision,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(507),	 30,	20,	"",			"!Vision!", NULL
    },

	{
    "locate object",        { SLEV, 14 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },    spell_locate_object,    TAR_IGNORE,  POS_STANDING,
    NULL,                   SLOT(31),       20,     0,
    "",                     "!Locate Object!"
    },

	//maladictions

	{
	"change sex",		{ 5, 2 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 1, 0, 0, 0, 0, 0 },	spell_change_sex,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(82),	5,	0,	"",			"Your body feels familiar again.", NULL
    },

	{
	"blindness",		{ 22, 26 ,SLEV, SLEV, SLEV, 60, SLEV  },
	{ 1, 1, 0, 0, 0, 2, 0 },	spell_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_blindness,		SLOT( 4),	 5,	12,	"",			"You can see again.", NULL
    },

    {
	"curse",		{ 30, 32 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 1, 0, 0, 0, 0, 0 },	spell_curse,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_curse,		SLOT(17),	20,	12,	"curse",		"The curse wears off.",	NULL
    },

	{
	"energy drain",		{ 33, 35 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 3, 3, 0, 0, 0, 0, 0 },	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(25),	35,	12,	"energy drain",		"!Energy Drain!", NULL
    },

    {
	"plague",		{ 65, 62 ,SLEV, SLEV, SLEV, 73, SLEV  },
	{ 3, 3, 0, 0, 0, 4, 0 },	spell_plague,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_plague,		SLOT(503),	20,	12,	"sickness",		"Your sores vanish.", NULL
    },

    {
	"poison",		{ 57, 56 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 2, 2, 0, 0, 0, 0, 0 },	spell_poison,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_poison,		SLOT(33),	20,	12,	"poison",		"You feel less sick.", NULL
    },

	{
	"weaken",		{ 41, 43 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 2, 2, 0, 0, 0, 0, 0 },	spell_weaken,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(68),	20,	12,	"spell",		"You feel stronger.", NULL
    },

	{
	"encase",		{ SLEV, 39 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },	spell_encase,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 238 ),	20,	12,	"",		"!Encase!", NULL
    }, //needs a slot

	{
	"slow",		{ 50, 51 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 3, 3, 0, 0, 0, 0, 0 },	spell_slow,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 239 ),	30,	12,	"",		"You speed up.", NULL
    }, //needs a slot

	//transportation spells

	{
	"fly",			{ SLEV, 7 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 1, 0, 0, 0, 0, 0 },	spell_fly,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(56),	10,	0,	"",			"You slowly float to the ground.", NULL
    },
/*
	{
	"water breathing",			{ SLEV, 13 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 1, 0, 0, 0, 0, 0 },	spell_water_breathing,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(240),	10,	0,	"",			"You can no longer breath water.", NULL
    }, //needs a slot
*/
	{
	"wind walk",			{ SLEV, 62 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 3, 0, 0, 0, 0, 0 },	spell_wind_walk,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(241),	80,	0,	"",			"!Wind Walk!", NULL
    },

	{
	"word of recall",	{ SLEV, 42 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 1, 0, 0, 0, 0, 0 },	spell_word_of_recall,	TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,			SLOT(42),	 5,	0,	"",			"!Word of Recall!", NULL
    },

	{
	"shift",		{ SLEV, 21 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },	spell_shift,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,	 		SLOT(242),	25,	12,	"",			"!Shift!", NULL
    },

	{
	"smoke form",		{ SLEV, 34 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },	spell_smoke_form,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(243),	15,	0,	"",			"You feel solid again.", NULL
    }, 

	{
	"summon",		{ SLEV, 57 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },	spell_summon,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(40),	50,	12,	"",			"!summon!", NULL
    },

	//benedictions

	{
    "blood lust",	{ SLEV, 33 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },        spell_blood_lust,   TAR_CHAR_DEFENSIVE, POS_FIGHTING,
    NULL,           SLOT(504),  30, 24, "", "Your rage ebbs.", NULL
    },

	{
    "bless",	{ SLEV, 19 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 1, 0, 0, 0, 0, 0 },        spell_bless,   TAR_CHAR_DEFENSIVE, POS_FIGHTING,
    NULL,           SLOT(3),  15, 0, "", "You feel less righteous.", NULL
    },

	{
    "calm",	{ SLEV, 65 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 2, 0, 0, 0, 0, 0 },        spell_calm,   TAR_CHAR_DEFENSIVE, POS_FIGHTING,
    NULL,           SLOT(509),  30, 4, "", "Your feeling of peace subsides.", NULL
    },
	
	{
    "holy word",	{ SLEV, 75 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 0, 3, 0, 0, 0, 0, 0 },        spell_holy_word,   TAR_IGNORE, POS_FIGHTING,
    NULL,           SLOT(506),  200, 24, "divine wrath", "!Holy Word!", NULL
    },

	{
    "remove curse",	{ 65, 32 ,SLEV, SLEV, SLEV, 32, SLEV  },
	{ 3, 1, 0, 0, 0, 3, 0 },        spell_remove_curse,   TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,           SLOT(35),  15, 0, "", "!remove Curse!", NULL
    },

	/*attack spells*/
	{
	"mystic hammer",	{ SLEV, 1, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_mystic_hammer,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(244),	15,	12,	"mystic hammer",	"!Mystic Hammer!",NULL
    },
	
	{
	"dispel evil",	{ SLEV, 15, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(22),	15,	12,	"dispel evil",	"!Dispel Evil!",NULL
    },
	
	{
	"dispel good",	{ SLEV, 16, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_dispel_good,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(245),	15,	12,	"dispel good",	"!Dispel Good!",NULL
    },

	{
	"wrath",	{ SLEV, 25, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_wrath,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(246),	20,	12,	"holy wrath",	"!Wrath!",NULL
    },

	{
	"fury",	{ SLEV, 40, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_fury,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(247),	20,	12,	"holy fury",	"!Fury!",NULL
    },

	{
	"ray of truth",	{ SLEV, 60, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_ray_of_truth,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(248),	25,	12,	"ray of truth",	"!Ray of Truth!",NULL
    },

	{
	"corruption",	{ SLEV, 60, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_corruption,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(249),	25,	12,	"ray of corruption",	"!Corruption!",NULL
    },

	{
	"immolate",	{ SLEV, 80, SLEV, SLEV, SLEV, SLEV, SLEV },
	{ 0,  1,  0,  0, 0, 0, 0 },	spell_immolate,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(250),	35,	12,	"fire imp",	"!Immolate!",NULL
    },

	//creation

	{
	"continual light",			{ 2, 4 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 1, 0, 0, 0, 0, 0 },	spell_continual_light,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(57),	5,	0,	"",			"!Continual Light!", NULL
    },

	{
	"create food",			{ 14, 18 ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 1, 0, 0, 0, 0, 0 },	spell_create_food,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(12),	5,	0,	"",			"!Create Food!", NULL
    },

	{
	"create spring",			{ 26, 29 ,SLEV, SLEV, SLEV, 76, SLEV  },
	{ 2, 2, 0, 0, 0, 1, 0 },	spell_create_spring,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(80),	25,	0,	"",			"!Create Spring!", NULL
    },
    
	{
	"create water",			{ 10, 8 ,SLEV, SLEV, SLEV, 13, SLEV  },
	{ 1, 1, 0, 0, 0, 1, 0 },	spell_create_water,TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(13),	15,	0,	"",			"!Create Water!", NULL
    },

	{
	"feast",	{ 68, 58 ,SLEV, SLEV, SLEV, SLEV, SLEV  },		
	{ 2, 2, 0, 0, 0, 0, 0 },	spell_feast,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(251),	35,	0,	"",			"!Feast!", NULL
    },//needs a slot
    

	/*end mystic add*/

	//SORCERER INSERT

	/*basic attack*/
	{
	"energy bolt",	{ 1, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_energy_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(32),	15,	12,	"bolt of energy",	"!Energy Bolt!", NULL
    },

	{
	"jolt",	{ 5, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_jolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(252),	15,	12,	"jolt",
"!Jolt!", NULL
    },

	{
	"steam burst",	{ 10, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_steam_burst,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(8),	15,	12,	"burst of steam",	"!Steam Burst!", NULL
    },

	{
	"lightning bolt",	{ 15, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(30),	15,	12,	"lightning bolt",	"!Lightning Bolt!", "E8006" 
    },

	{
	"shatter",	{ 20, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_shatter,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(5),	15,	12,	"shattering",	"!Shatter!", "E8007" 
    },

	{
	"flame burst",	{ 30, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 2, 0, 0, 0, 0, 0, 0 },	spell_flame_burst,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(10),	15,	12,	"burst of flame",	"!Flame Burst!", NULL
    },

	{
	"cold ray",	{ 40, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 2, 0, 0, 0, 0, 0, 0 },	spell_cold_ray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(70),	15,	12,	"cold ray",	"!Cold Ray!", NULL
    },

	{
	"corrosion",	{ 50, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 2, 0, 0, 0, 0, 0, 0 },	spell_corrosion,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(26),	15,	12,	"corrosion",	"!Corrosion!", NULL
    },
	
	{
	"disintegrate",	{ 60, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 3, 0, 0, 0, 0, 0, 0 },	spell_disintegrate,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(503),	20,	12,	"disintegration",	"!Disintegrate!", NULL
    },

	{
	"prismatic ray",	{ 70, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 3, 0, 0, 0, 0, 0, 0 },	spell_prismatic_ray, TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(504),	20,	12,	"prismatic ray",	"!Prismatic Ray!", NULL
    },

	{
	"heart breaker",	{ 80, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 4, 0, 0, 0, 0, 0, 0 },	spell_heart_breaker, TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(253),	30,	12,	"magic",	"heart strain", NULL
    },

	{
	"finger of death",	{ 90, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 4, 0, 0, 0, 0, 0, 0 },	spell_finger_of_death, TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(254),	50,	12,	"death ray",	"death ray", NULL
    },

	//enchantments
	{
	"enchant armor",	{ 40, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 4, 0, 0, 0, 0, 0, 0 },	spell_enchant_armor, TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(510),	100,	24,	"",	"!Enchant Armor!", NULL
    },

	{
	"enchant weapon",	{ 50, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 4, 0, 0, 0, 0, 0, 0 },	spell_enchant_weapon, TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(24),	100,	24,	"",	"!Enchant Weapon!", NULL
    },

	{
	"recharge",	{ 30, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 3, 0, 0, 0, 0, 0, 0 },	spell_recharge, TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(255),	75,	24,	"",	"!Recharge!", NULL
    },

	{
	"glow",		{ 20, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 2, 0, 0, 0, 0, 0, 0 },	spell_glow, TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(256),	50,	12,	"",	"!Glow!", NULL
    },

	{
	"fireproof",	{ 60, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 4, 0, 0, 0, 0, 0, 0 },	spell_fireproof, TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(257),	200,	24,	"",	"!Fireproof!", NULL
    },

	//Alterations
	{
	"invis",		{  15, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_invis,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_invis,		SLOT(29),	 10,	12,	"",			"You are no longer invisible.",	NULL
    },

    {
	"mass invis",		{ 48, SLEV, SLEV, SLEV, SLEV,SLEV,SLEV },
	{ 3,  0,  0,  0, 0,0,0},	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(69),	30,	12,	"",			"!Mass Invis!",	NULL
    },

	{
	"giant strength",		{  20, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_giant_strength,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(39),	 30,	12,	"",			"You feel weaker.",	NULL
    },

	{
	"haste",		{  30, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_haste,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(502),	 30,	12,	"",			"You feel yourself slow down.",	NULL
    },

	{
	"infravision",		{  18, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_infravision,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(77),	 10,	0,	"",			"You no longer see in the dark.",	NULL
    },
	
	//Protective
	
	{
	"warding",		{  3, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_warding,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(258),	 5,	12,	"",			"You are no longer warded from harm.",	NULL
    },

	{
	"harden",		{  18, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_harden,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(67),	 12,	12,	"",			"The hardness leaves your skin.",	NULL
    },
/*
	{
	"spell turning",		{  58, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 3,  0,  0,  0,0,0,0},	spell_spell_turning,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(259),	 50,	12,	"",			"You are no longer turn spells.",	NULL
    },
*/
	{
	"resist",		{  38, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 3,  0,  0,  0,0,0,0},	spell_resist,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(260),	 30,	12,	"",			"Your resistance to magic decreases.",	NULL
    },

	//detection
	{
	"detect evil",		{  5, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_detect_evil,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(18),	 5,	12,	"",
"You can no longer see evil auras.",	NULL
    },

	{
	"detect good",		{  5, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_detect_good,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(261),	 5,	12,	"",			"You can no longer see good auras.",	NULL
    },
/*
	{
	"detect traps",		{  20, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_detect_traps,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(262),	 25,	12,	"",			"!Detect Traps!",
NULL
    },
*/
	{
	"detect hidden",		{  9, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_detect_hidden,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(44),	 5,	12,	"",
"You can no longer see hidden things.",	NULL
    },

	{
	"detect invisible",		{  8, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_detect_invis,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(19),	 5,	12,	"",
"You can no longer see invisible things.",	NULL
    },

	{
	"detect magic",		{  4, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_detect_magic,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(20),	 5,	12,	"",
"You can no longer see magic auras.",	NULL
    },

	{
	"detect poison",		{  4, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_detect_poison,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(21),	 5,	12,	"",
"!Detect Poison!",	NULL
    },

	{
	"know alignment",		{  15, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_know_alignment,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(58),	 20,	12,	"",
"!Know Alignment!",	NULL
    },

	{
	"identify",		{  1, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_identify,		TAR_OBJ_INV,
POS_STANDING,
	NULL,		SLOT(53),	 15,	12,	"",
"!Identify!",	NULL
    },

	{
	"mount",		{  5, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_mount,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(263),	 50,	12,	"",			"!Mount!",	NULL
    },

	{
	"find familiar",		{  15, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_find_familiar,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(264),	 100,	12,	"",			"!Find Familiar!",
NULL
    },

	{
	"magic sword",		{  25, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_magic_sword,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(265),	 30,	12,	"",			"!Magic Sword!",
NULL
    },

	{
	"insect swarm",		{  40, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_insect_swarm,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(266),	 40,	12,	"",			"!Insect Swarm!",
NULL
    },

	{
	"conjuring",		{  60, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_conjuring,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(267),	 50,	12,	"",			"!Conjuring!",	NULL
    },

	{
	"creeping doom",		{  81, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_creeping_doom,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		SLOT(268),	 60,	12,	"",			"!Creeping Doom!",
NULL
    },

/*	{
	"nexus",		{  50, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_nexus,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(269),	 75,	12,	"",			"!Nexus!",	NULL
    },

*/
	{
	"pass door",		{  20, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_pass_door,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(74),	 15,	12,	"",
"You become more solid.",	NULL
    },

	{
	"teleport",		{  19, SLEV, SLEV, SLEV,SLEV,SLEV,SLEV },
	{ 1,  0,  0,  0,0,0,0},	spell_teleport,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		SLOT(2),	 15,	12,	"",
"!Teleport!",	NULL
    },

	{
	"volley",	{12 , SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_volley,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(270),	15,	12,	"arrow",
"!Volley!", NULL
    },

	{
	"arc",	{25, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_arc,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(271),	15,	12,	"spark",
"!Arc!", NULL
    },

	{
	"energy storm",	{37, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_energy_storm,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(272),	15,	12,	"energy",
"!Energy Storm!", NULL
    },

	{
	"fireball",	{ 47, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_fireball,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(273),	15,	12,
"fireball",	"!Fireball!", "E8000" 
    },

	{
	"ice storm",	{ 62, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_ice_storm,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(506),	15,	12,	"ice
storm",	"!Ice Storm!", NULL
    },

	{
	"incinerate",	{ 71, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_incinerate,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(274),	15,	12,	"fire",
"!Incinerate!", NULL
    },

	{
	"chain lightning",	{ 81, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_chain_lightning,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(500),	15,	12,
"lightning",	"!Chain Lightning!", NULL
    },

	{
	"prismatic spray",	{ 93, SLEV ,SLEV, SLEV, SLEV, SLEV, SLEV  },
	{ 1, 0, 0, 0, 0, 0, 0 },	spell_prismatic_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(275),	15,	12,	"color",
"!Prismatic Sparay!", NULL
    },

	/*end sorcerer*/

    {
	"charm person",		{ 0, 0, 0, 0,0,0,0 },     { 1,  1,  1,  1,1,1,2},
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 7),	 5,	12,
	"",			"You feel more self-confident.",	NULL
    },

	

    

	

    {
	"sleep",		{ 0, 0, 0, 0,0,0,0 },     { 1,  1,  1,  1,1,1,2},
	spell_sleep,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_sleep,		SLOT(38),	15,	12,
	"",			"You feel less tired.",	NULL
    },


    {
       "riding",               {  1,  1,  1,  1,1,1,1 },     { 2, 2, 2, 2,1,1,1},
       spell_null,             TAR_IGNORE,             POS_STANDING,
       &gsn_riding,            SLOT( 0),       0,      12,
       "",                     "!Riding!",	NULL
     },
/* combat and weapons skills */


    {
	"axe",			{ 1, SLEV, 1,  1,1,1,1 },	{ 5, 5, 4, 3,4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_axe,            	SLOT( 0),       0,      0,
        "",                     "!Axe!",	NULL
    },

    {
        "dagger",               {  1,  SLEV,  1,  1,1,1,1 },     { 2, 3, 2, 2,2,2,2},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dagger,            SLOT( 0),       0,      0,
        "",                     "!Dagger!",	NULL
    },
 
    {
	"flail",		{ 1,  1, 1,  1,1,1,1 },	{ 5, 3, 5, 4,3,5,5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_flail,            	SLOT( 0),       0,      0,
        "",                     "!Flail!",	NULL
    },

    {
	"mace",			{ 1,  1,  1,  1,1,1,1 },	{ 5, 2, 3, 3,3,3,3},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_mace,            	SLOT( 0),       0,      0,
        "",                     "!Mace!",	NULL
    },

    {
	"polearm",		{ 1, 1, 1,1,1,1,  1 },	{ 4, 4, 4, 3,4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_polearm,           SLOT( 0),       0,      0,
        "",                     "!Polearm!",	NULL
    },
    
    {
	"shield block",		{ 103,  103, 10,  3,10,10,10 },	{ 0, 0, 4, 2,4,4,4},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_shield_block,	SLOT(0),	0,	0,
	"",			"!Shield!",	NULL
    },

    { /*  edoc  */
	/* sharpen, warrior only */
        "sharpen",	{ 102,102,102,1,102,102,102 },	{ 0,0,0,3,0,0,0 },
        spell_null,		TAR_IGNORE,	POS_RESTING,
        &gsn_sharpen,		SLOT(0),	0,	24,
	"",	"!Sharpen!"
    },

    { /*  spellsong  */
        "push",      { 50,40,10,1,40,1,10 },  { 4,3,3,2,3,2,2 },
        spell_null,             TAR_IGNORE,     POS_STANDING,
        &gsn_push,           SLOT(0),        0,      24,
        "",     "!Push!"
    }, 

    {
	"spear",		{  1,  SLEV,  1,  1,1,1,1 },	{ 4, 4, 4, 3,4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_spear,            	SLOT( 0),       0,      0,
        "",                     "!Spear!",	NULL
    },

    {
	"sword",		{ 1, SLEV,  1,  1,1,1,1},	{ 4, 4, 3, 2,3,3,3},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_sword,            	SLOT( 0),       0,      0,
        "",                     "!sword!",	NULL
    },

    {
	"whip",			{ 1, SLEV,  1,  1,1,1,1},	{ 5, 4, 4, 3,4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_whip,            	SLOT( 0),       0,      0,
        "",                     "!Whip!",	NULL
    },

    {
        "backstab",             { 103, 103,  7, 103,103,103,103 },     { 0, 0, 4, 0,0,0,0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_backstab,          SLOT( 0),        0,     36,
        "backstab",             "!Backstab!",	NULL
    },

	/*coded by Spellsong - idea and original coding MorningGlory. Idea credit here only*/
	{
        "circle",              { 103, 103,  50, 103,103,103,103 },     { 0, 0, 5, 0,0,0,0},
        spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
        &gsn_circle,           SLOT( 0),       0,      0,
        "",        "!Circle!",	NULL
    },

	/*end spellsong*/

	{
	"captivate",		{ 0, 0, 0, 0, 0, 0, 0 },	{ 9, 9, 9, 9, 9, 9, 9 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_captivate,		SLOT(0),	0, 	0,
	"",	"!Captivate!",	NULL
	},
    {
	"bash",			{ 103, 103, 103,  25, 35, 35, 55 },	{ 0, 0, 0, 4,5,5,5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_bash,            	SLOT( 0),       0,      24,
        "bash",                 "!Bash!",	NULL
    },

    {
	"berserk",		{ 103, 103, 103, 42, 103,103,103 },	{ 0, 0, 0, 5,0,0,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_berserk,        	SLOT( 0),       0,      24,
        "",                     "You feel your pulse slow down.",	NULL
    },
	
	/*Critical Strike skill by Brian Babey (aka Varen) - adapted by Spellsong*/
	{
        "critical strike",            { 103, 103, 50, 35, 58, 58, 103 },     { 0, 0, 5, 4,5,5,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_critical,          SLOT( 0),       0,      0,
        "",                     "!Critical Strike!",	NULL
    },

	/*Counter skill by Brian Babey (aka Varen) - adapted by Spellsong*/
	{
        "counterattack",              { 103, 103, 25, 25, 25,25,45  },     { 0, 0, 5, 4,5,5,5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_counter,           SLOT( 0),       0,      0,
        "counterattack",        "!Counter!",	NULL
    },

	/*Phase skill by Brian Babey (aka Varen) - adapted by Spellsong*/
	{
        "phase",                { 0, 0, 0, 0,0,0,0 },     { 9,9,9,9,9,9,9},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_phase,             SLOT( 0),       0,      0,
        "",                     "!Phase!",	NULL
    },

    {
	"dirt kicking",		{ 103, 103,  22,  22, 103, 103, 103 },	{ 0, 0, 4, 4,0,0,0}, 
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_dirt,		SLOT( 0),	0,	24,
	"kicked dirt",		"You rub the dirt out of your eyes.",	NULL
    },

    {
        "disarm",               { 103, 103, 103, 19,103,103,103 },     { 0, 0, 0, 4,0,0,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_disarm,            SLOT( 0),        0,     24,
        "",                     "!Disarm!",	NULL
    },
 
    {
        "dodge",                { 35, 30,  25, 16,25,25,25 },     { 6, 6, 5, 4,5,5,5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dodge,             SLOT( 0),        0,      0,
        "",                     "!Dodge!",	NULL
    },
 
    {
        "enhanced damage",      { 103, 103, 103,  28, 58, 58, 103 },     { 0, 0, 0, 4, 6, 6, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_enhanced_damage,   SLOT( 0),        0,      0,
        "",                     "!Enhanced Damage!",	NULL
    },

    {
	"hand to hand",		{ 103,  SLEV, 103, 15,103,103,1 },	{ 0, 0, 0, 2,0,0,1},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_hand_to_hand,	SLOT( 0),	0,	0,
	"",			"!Hand to Hand!",	NULL
    },

    {
        "kick",                 { 103, 103, 103,  26,103,103,103 },     { 0, 0, 0, 3,0,0,0},
        spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_kick,              SLOT( 0),        0,      12,
        "kick",                 "!Kick!",	NULL
    },

    {
        "parry",                { 103, 103, 35,  18,0,0,0 },     { 0, 0, 5, 4,0,0,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_parry,             SLOT( 0),        0,      0,
        "",                     "!Parry!",	NULL
    },

    {
        "rescue",               { 103, 103, 52, 32, 52,52,52 },     { 0, 0, 4, 2,4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_rescue,            SLOT( 0),        0,     12,
        "",                     "!Rescue!",	NULL
    },

    {
	"trip",			{ 103, 103,  22, 26,103,103,103 },	{ 0, 0, 3, 6,6,6,6},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_trip,		SLOT( 0),	0,	24,
	"trip",			"!Trip!",	NULL
    },

    {
        "second attack",        { 59, 55, 30,  12, 30, 30, 30 },     { 6, 6, 4, 3,4,4,4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_second_attack,     SLOT( 0),        0,      0,
        "",                     "!Second Attack!",	NULL
    },

    {
        "third attack",         { 103, 103, 57, 40, 60, 60, 65 },     { 0, 0, 6, 4, 6, 6, 6},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_third_attack,      SLOT( 0),        0,      0,
        "",                     "!Third Attack!",	NULL
    },

/* non-combat skills */


	/*coded by Spellsong*/

	{
        "disarm trap",              { 103, 103,  1, 103,103,103,103 },     { 0, 0, 5, 0,0,0,0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_disarm_trap,           SLOT( 0),       0,      0,
        "",        "!Disarm Trap!",	NULL
    },

	{
        "detect trap",              { 103, 103,  1, 103,103,103,103 },     { 0, 0, 5, 0,0,0,0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_detect_trap,           SLOT( 0),       0,      0,
        "",      "!Detect Trap!",	NULL
    },


    { 
	"fast healing",		{ 50, 40, 40,  20, 40, 40, 20 },	{ 5, 5, 4, 3,4,4,1},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_fast_healing,	SLOT( 0),	0,	0,
	"",			"!Fast Healing!",	NULL
    },

    {
	"haggle",		{ 14, 16,  1, 10,5,16,20 },	{ 2, 2, 1, 2,2,2,2},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_haggle,		SLOT( 0),	0,	0,
	"",			"!Haggle!",	NULL
    },

    {
	"hide",			{ 0, 0,  1, 0,0,0,0 },	{ 9, 9, 1, 9,9,9,9},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_hide,		SLOT( 0),	 0,	0,
	"",			"!Hide!",	NULL
    },

    {
	"lore",			{ 18, 5,  18, 33,18,8,33 },	{ 3, 2, 3, 6,3,3,6},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_lore,		SLOT( 0),	0,	0,
	"",			"!Lore!",	NULL
    },

    {
	"meditation",		{  15,  10, 103, 103, 15,15,103 },	{ 3, 3, 0, 0,5,5,0},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_meditation,	SLOT( 0),	0,	0,
	"",			"Meditation",	NULL
    },

    {
	"peek",			{  103, 103,  5, 103,103,103,103 },	{ 0, 0, 3, 0,0,0,0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,		SLOT( 0),	 0,	 0,
	"",			"!Peek!",	NULL
    },

    {
	"pick lock",		{ 103, 103,  13, 103,103,103,20 },	{ 0, 0, 4, 0,0,0,4},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,		SLOT( 0),	 0,	0,
	"",			"!Pick!",	NULL
    },

    {
        "sap",                	{ 0, 0,  26,  0,0,0,0 }, { 9, 9, 1, 9,9,9,9},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_sap,             	SLOT( 0),        0,     0,
        "",                     "!Sap!", NULL
    },

    {
	"sneak",		{ 0, 0,  1,  0,0,0,0 },	{ 9, 9, 1, 9,9,9,9},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT( 0),	 0,	0,
	"",			"You no longer feel stealthy.",	NULL
    },

    {
	"steal",		{ 103, 103,  5, 103,103,103,103 },	{ 0, 0, 4, 0,0,0,0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,		SLOT( 0),	 0,	24,
	"",			"!Steal!",	NULL
    },

    {
	"scrolls",		{  3,  5,  27,  30, 4,15,30 },	{ 1, 2, 3, 3,1,2,3},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_scrolls,		SLOT( 0),	0,	0,
	"",			"!Scrolls!",	NULL
    },

    {
	"staves",		{  5,  9,  33,  45, 5,17,45 },	{ 2, 3, 5, 6,2,3,6},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_staves,		SLOT( 0),	0,	0,
	"",			"!Staves!",	NULL
    },
    
    {
	"wands",		{  1,  4,  20,  20, 1,4,20 },	{ 1, 2, 3, 3,1,2,3},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_wands,		SLOT( 0),	0,	0,
	"",			"!Wands!",	NULL
    },

    {
	"recall",		{  1,  1,  1,  1,1,1,1 },	{ 2, 2, 2, 2,2,2,2},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_recall,		SLOT( 0),	0,	0,
	"",			"!Recall!",	NULL
    },

    {
        "brew",                 { 50, 103, 103, 103, 9, 103, 103 },     { 0, 0, 0, 0,3,0,0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_brew,              SLOT( 0),	0,       24,
        "",                     "!Brew!",	NULL
    },

    {
        "scribe",               { 40, 103, 103, 103,103,103,103 },     { 4, 0, 0, 0,0,0,0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_scribe,            SLOT( 0),	0,       24,
        "",                     "!Scribe!",	NULL
    },

    {
        "charge",             { 103, 103,  103, 103,103,92,103 },     { 0, 0, 0, 0,0,3,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_charge,          SLOT( 0),        0,     24,
        "",             "!Charge!",   NULL
    },

    {
        "trample",             { 103, 103,  103, 103,103,38,103 },     { 0, 0, 0, 0,0,3,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_trample,          SLOT( 0),        0,     24,
        "",             "!Trample!",   NULL
    },

    {
        "turn",               { 103, 103, 103, 103,103, 27 ,103 },     { 0, 0, 0, 0,0,3,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_turn,            SLOT( 0),       0,       24,
        "",                     "!Turn!",     NULL
    },

{
        "web",          {  103, 103, 103, 103,103,103,103 },     { 1,  1,  2,  2,2,1,2},
        spell_web,              TAR_CHAR_DEFENSIVE,             POS_FIGHTING,
        NULL,                   SLOT(511),      20,     0,
        "",                     "The webs disolve."
    },


{
        "demonfire",            { 103, 103, 103, 103,103,103,103 },     { 1,  1,  2,  2,2,1,2},
        spell_demonfire,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(505),      20,     12,
        "torments",             "!Demonfire!"
    },


{
        "gate",                 { 103, 103, 103, 103,103,103,103 },     { 1,  1,  2,  2,2,1,2},
        spell_gate,             TAR_IGNORE,             POS_FIGHTING,
        NULL,                   SLOT(83),       80,     0,
        "",                     "!Gate!"
    }


};

//NOTE: 38 cp for a human = 78% exp use that for a guideline for
//      basics - Spellsong

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics",		{ 0, 0, 0, 0,0,0,0 },
	{ "recall" }
    },

    {
	"sorcerer basics",		{ 0, -1, -1, -1, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"mystic basics",	{ -1, 30, -1, -1, -1, -1, -1 },
	{ "mace", "mystic attack", "mystic benedictions", "mystic harm",
	"mystic healing", "mystic protection", "lore", "meditation",
        "mystic curative", "mystic basics1" }
    },

    {
    "mystic basics1",	{ -1, 0, -1, -1, -1, -1, -1 },
    { "staves", "scrolls", "mace" }
    },  
 
    {
	"thief basics",		{ -1, -1, 30, -1, -1, -1, -1 },
	{ "backstab", "dagger", "detect trap", "dirt kicking", "disarm trap", "haggle", 
          "hide", "lore", "peek",
          "pick lock", "sap", "scrolls", "sneak", "second attack", "staves", "steal", "wands"}
    },

    {
	"warrior basics",	{ -1, -1, -1, 30, -1, -1, 0 },
	{ "sword", "axe", "dagger", "wands", "second attack", "third attack", "rescue", 
         "hand to hand", "disarm", "push", "sharpen", "shield block", "kick" }
    },

    {
	"alchemist basics",	{ -1, -1, -1, -1, 0, -1, -1 },
	{ "sword" }
    },

    {
	"templar basics", 	{ -1, -1, -1, -1, -1, 30, -1 },
	{ "sword", "riding", "templar healing", "templar creation", "templar benediction",
	  /*"sanctify",*/ "shield block", "second attack", "turn"
	  "polearm", "templar basics1"}
    },
		
	{
	"templar basics1",		{ -1, -1, -1, -1, -1, 0, -1 },
	{ "trample", "charge", "disarm"}
    },

    {
	"sorcerer default",		{ 30, -1, -1, -1, -1, -1, -1 },
	{"dagger", "lore", "scrolls", "staves", "wands", "haggle",
         "basic attack", "enchantment", "alteration", "sorcerer protective", 
	 "conjuring"}
    },

    /*{
	"mystic default",	{ -1, 0, -1, -1, -1, -1, -1 },
	{ "flail" }
    },*/
 
    {
	"thief default",	{ -1, -1, 0, -1, -1, -1, -1 },
	{ "mace" }
    },

    {
	"warrior default",	{ -1, -1, -1, 0, -1, -1, 0 },
	{ "dagger" }
    },
    
    {
	"alchemist default",	{-1, -1, -1, -1, 0, -1, -1 },
	{ "mace" }
    },
    
    {
	"templar default",	{ -1, -1, -1, -1, -1, 0, -1 },
	{ "axe"}
    },

	//MYSTIC DEFAULT SPELL GROUPS
	{
	"mystic attack",		{ -1, 0, -1, -1, -1, -1, -1 },
	{ "mystic hammer", "dispel evil", "dispel good", "wrath", "fury",
	  "ray of truth", "corruption", "immolate" }
    }, 

	{
	"mystic benedictions",		{ -1, 0, -1, -1, -1, -1, -1 },
	{ "blood lust", "bless", "calm", "holy word", "remove curse" }
    },

	{
	"mystic harm",		{ -1, 0, -1, -1, -1, -1, -1 },
	{ "mar", "abuse", "injure", "harm", "inflict", "avenge" }
    },

	{
	"mystic healing",		{ -1, 0, -1, -1, -1, -1, -1 },
	{ "refresh", "patch wounds", "bind wounds", "mend wounds", "repair wounds" }
    },
		
	{
	"mystic protection",		{ -1, 0, -1, -1, -1, -1, -1 },
	{ "barkskin", "cancellation", "dispel magic", "preservation", "protection good",
	"protection evil", "psychic shield", "sanctuary", "stone skin" }
    },


	{
	"mystic curative",		{ -1, 0, -1, -1, -1, -1, -1 },
	{ "awaken", "cure blindness", "cure disease", "cure poison" }
    },

	//MYSTIC GROUPS

	{
	"advanced healing",		{ -1, 5, -1, -1, -1, -1, -1 },
	{ "mass healing", "restoration", "regeneration", "succor", "energy transferance" }
    },
		
	{
	"creation",		{ 1, 1, -1, -1, -1, -1, -1 },
	{ "continual light", "create food", "create spring", "create water", "feast" }
    }, 


	{
	"divination",		{ -1, 2, -1, -1, -1, -1, -1 },
	{ "revelation", "foresight", "vision", "aura" }
    }, 

	{
	"elemental",		{ -1, 4, -1, -1, -1, -1, -1 },
	{ "geyser", "flamestrike", "earthquake", "ice shield", "fire shield",
	  "sandblast", "summon earth elemental" }
    }, 
	
	{
	"maladictions",		{ -1, 4, -1, -1, -1, -1, -1 },
	{ "change sex", "blindness", "curse", "energy drain", "plague",
	  "poison", "weaken", "encase", "slow" }
    }, 
	
	{
	"psychic",		{ -1, 3, -1, -1, -1, -1, -1 },
	{ "chakra", "mind shield", "deflect", "enslave", "spirit shield" }
    }, 

	{
	"travel",		{ -1, 3, -1, -1, -1, -1, -1 },
	{ "fly", "wind walk", "word of recall", "shift",
	  "smoke form", "summon" }
    }, 
		
	{
	"weather",		{ -1, 3, -1, -1, -1, -1, -1 },
	{ "call lightning", "control weather", "faerie fire", "faerie fog", "tornado", "typhoon" }
    },
		
	//TEMPLAR DEFAULT SPELL GROUPS
	{
	"templar healing",		{ -1, -1, -1, -1, -1, 0, -1 },
	{ "lay hands", "aid", "divine intervention" }
    },
		
	{
	"templar creation",		{ -1, -1, -1, -1, -1, 0, -1 },
	{ "call mount", "create water", "create spring", 
	  "satiate" }
    },

	{
	"templar benediction",		{ -1, -1, -1, -1, -1, 0, -1 },
	{ "holy wrath", "inspire", "divine grace" }
    },

	//TEMPLAR SPELL GROUPS
	{
	"wounding",		{ -1, -1, -1, -1, -1, 4, -1 },
	{ "bruise", "wound", "wither" }
    },

	{
	"warding",		{ -1, -1, -1, -1, -1, 5, -1 },
	{ "holy aura", "divine protection", "protection good", "protection evil",
	  "preservation" }
    },

	{
	"inflictions",		{ -1, -1, -1, -1, -1, 6, -1 },
	{ "vex", "remove curse", "blindness", "cure blindness", "cure disease"
	  "plague", "awaken"}
    },

	//SORCERER DEFAULT SPELL GROUPS
	{
	"basic attack",		{ 8, -1, -1, -1, -1, -1, -1 },
        { "energy bolt", "jolt", "steam burst", "lightning bolt", "shatter",
          "flame burst", "cold ray", "corrosion", "disintegrate", 
          "prismatic ray", "heart breaker", "finger of death"}
    },

	{
	"enchantment",		{ 4, -1, -1, -1, -1 ,-1, -1},
	{ "enchant armor", "enchant weapon", "recharge", "glow", "fireproof"}
    },

	{
	"alteration",		{ 5, -1, -1, -1, -1, -1, -1 },
	{ "invis", "mass invis", "haste", "giant strength", "infravision", "refresh"}
    },

	{
	"sorcerer protective",	{ 6, -1, -1, -1, -1, -1, -1 },
	{ "warding", "harden", "protection good", "protection evil", "stone skin", 
          "sanctuary", "preservation" }
    },

	{
	"countering",		{ 4, -1, -1, -1, -1, -1, -1 },
	{ "resist", "dispel magic", "cancellation", "remove curse" }
    },

	{
	"conjuring",		{ 5, -1, -1, -1, -1, -1, -1 },
	{ "mount", "find familiar", "magic sword", "insect swarm", "conjuring",
	  "creeping doom"}
    },

	{
	"transportation",	{ 6, -1, -1, -1, -1, -1, -1 },
	{ "fly", "word of recall", "summon", "pass door", "gate", "teleport"}
    },

	//SORCERER SPELL GROUPS

	{
	"anathema",		{5, -1, -1, -1, -1, -1, -1 },
	{ "plague", "slow", "blindness", "weaken", "cure", "poison", "energy drain",
	  "change sex"}
    },

	{
	"detection",		{ 4, -1, -1, -1, -1, -1, -1},
	{ "detect evil", "detect good", "detect hidden", "detect invis", "detect magic",
	  "detect poison", "know alignment", "identify"}
    },

	{
	"advanced attack",	{ 7, -1, -1, -1, -1, -1, -1 },
	{ "volley", "arc", "energy storm", "fireball", "ice storm", "incinerate",
	  "chain lightning", "prismatic spray"}
    }
};
