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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "grant.h"

bool	check_social	args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );



/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2



/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;



/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_STANDING, 0,0,   0,  LOG_NEVER, 0 },
    { "east",		do_east,	POS_STANDING, 0,0, 0,  LOG_NEVER, 0 },
    { "south",		do_south,	POS_STANDING,  0,0,	 0,  LOG_NEVER, 0 },
    { "west",		do_west,	POS_STANDING,  0,0,	 0,  LOG_NEVER, 0 },
    { "up",		do_up,		POS_STANDING,  0,0,	 0,  LOG_NEVER, 0 },
    { "down",		do_down,	POS_STANDING,  0,0,	 0,  LOG_NEVER, 0 },
	/*moved here to take preference over grat channel - spellsong*/
	{ "group",		do_group,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    /*Immm commons*/
    { "at",             do_at,          POS_DEAD,    FUN_AT,
GROUP_FUN, L6,  LOG_NORMAL, 1 },
    { "goto",           do_goto,        POS_DEAD,    FUN_GOTO ,GROUP_FUN, L8,  LOG_NORMAL, 1 },
    { "sockets",        do_sockets,	POS_DEAD,    INF_SOCKETS, GROUP_INF, L4,  LOG_NORMAL, 1 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,    INF_WIZHELP, GROUP_INF, HE,  LOG_NORMAL, 1 },
    { "ooc",            do_ooc,         POS_DEAD,    0,0, 0, LOG_NORMAL, 1 },
     /*Mort commons*/
    { "auction",        do_auction,     POS_SLEEPING,  0,0,  0,  LOG_NORMAL, 1 },
    { "buy",		do_buy,		POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "cast",		do_cast,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "channels",       do_channels,    POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "exits",		do_exits,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "get",		do_get,		POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "hit",		do_kill,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 0 },
    { "inventory",	do_inventory,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "kill",		do_kill,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "look",		do_look,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "mount",         do_mount,       POS_FIGHTING,   0,0,      0,  LOG_NORMAL, 1 },
    { "dismount",      do_dismount,    POS_FIGHTING,   0,0,      0,  LOG_NORMAL, 1 },
    { "order",		do_order,	POS_RESTING,   0,0,	 0,  LOG_ALWAYS, 1 },
    { "learn",       do_learn,	POS_SLEEPING,  0,0,  0,LOG_NORMAL, 1 },
    { "practice",       do_practice,	POS_SLEEPING,  0,0,  0,  LOG_NORMAL, 1 },
    { "rest",		do_rest,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "sit",		do_sit,		POS_SLEEPING,  0,0,  0,  LOG_NORMAL, 1 },
    { "scan",		do_scan,	POS_RESTING,   0,0,  0,  LOG_NORMAL, 1 },
    { "stand",		do_stand,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "tell",		do_tell,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "wield",		do_wear,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },


    /*
     * Informational commands.
     */
    { "affects",	do_effects,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "areas",		do_areas,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "bug",		do_bug,		POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "changes",	do_changes,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "commands",	do_commands,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "compare",	do_compare,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "consider",	do_consider,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "count",		do_count,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "credits",	do_credits,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "cwho",		do_cwho,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
	/*spellsong for traps*/
    { "detect trap",do_detect_trap, POS_STANDING,  0,0,  0,  LOG_NORMAL, 1 },
    { "equipment",	do_equipment,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "examine",	do_examine,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "effects",	do_effects,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "sgroups",	do_groups,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "help",		do_help,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "idea",		do_idea,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "info",       do_info,    POS_SLEEPING,  0,0,  0,  LOG_NORMAL, 1 },
    { "levels",		do_levels,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "motd",		do_motd,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "news",		do_news,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "read",		do_read,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "report",		do_report,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "rules",		do_rules,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "score",		do_score,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "search",		do_search,	POS_STANDING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "show",		do_show,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "skills",		do_skills,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "socials",	do_socials,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "spells",		do_spells,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "story",		do_story,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "time",		do_time,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "typo",		do_typo,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "weather",	do_weather,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "who",		do_who,		POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "whois",		do_finger,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "wizlist",	do_wizlist,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "worth",		do_worth,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },

    /*
     * Configuration commands.
     */
    { "afk",		do_afk,		POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "alias",		do_alias,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "autolist",	do_autolist,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "autoassist",	do_autoassist,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "autoexit",	do_autoexit,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "autogold",	do_autogold,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "autoloot",	do_autoloot,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "autosac",	do_autosac,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "autosplit",	do_autosplit,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "brief",		do_brief,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "channels",	do_channels,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "combine",	do_combine,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "compact",	do_compact,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "config",		do_autolist,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "description",	do_description,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "delet",		do_delet,	POS_DEAD,      0,0,	 0,  LOG_ALWAYS, 0 },
    { "delete",		do_delete,	POS_DEAD,      0,0,	 0,  LOG_ALWAYS, 1 },
    { "nofollow",	do_nofollow,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "noloot",		do_noloot,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "nosummon",	do_nosummon,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "nocolor",	do_nocolor,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "color",		do_nocolor,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "outfit",		do_outfit,	POS_RESTING,   0,0,	 0,  LOG_ALWAYS, 1 },
    { "password",	do_password,	POS_DEAD,      0,0,	 0,  LOG_NEVER,  1 },
    { "pk",		do_pk,		POS_DEAD,      0,0,	 0,  LOG_ALWAYS,  1 },
    { "prompt",		do_prompt,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    { "scroll",		do_scroll,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "title",		do_title,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "unalias",	do_unalias,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "wimpy",		do_wimpy,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },

    /*
     * Communication commands.
     */
    { "page",           do_beep,        POS_SLEEPING,  0,0,  0,  LOG_NORMAL, 1 },
    { "auction",	do_auction,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "deaf",		do_deaf,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "emote",		do_emote,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "gossip",		do_gossip,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { ":",		do_emote,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 0 },
    { "gtell",		do_gtell,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { ";",		do_gtell,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 0 },
    { "note",		do_note,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "pose",		do_pose,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "quiet",		do_quiet,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "reply",		do_reply,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "say",		do_say,		POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "osay",		do_osay,	POS_RESTING,	0,0,	 0,  LOG_NORMAL, 1 },
    { "'",		do_say,		POS_RESTING,   0,0,	 0,  LOG_NORMAL, 0 },
    { "ignore",		do_ignore,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "unread",		do_unread,	POS_SLEEPING,  0,0,  0,  LOG_NORMAL, 1 },
    { "yell",		do_yell,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "pray", 		do_pray,	POS_DEAD,      0,0,  0,  LOG_NORMAL, 1 },
    /*
     * Object manipulation commands.
     */
    { "brandish",	do_brandish,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "close",		do_close,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "donate",		do_donate,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "drink",		do_drink,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "drop",		do_drop,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "eat",		do_eat,		POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "fill",		do_fill,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
	/*Games added by spellsong*/
    { "game",		do_game,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
	{ "give",		do_give,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "heal",		do_heal,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "hold",		do_wear,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
	/*Identify added by spellsong*/
    { "identify",   do_identify, POS_STANDING, 0,0,  0,  LOG_NORMAL, 1 }, 
	{ "list",		do_list,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "lock",		do_lock,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "open",		do_open,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "pick",		do_pick,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "pour",           do_pour,        POS_RESTING,   0,0,  0,  LOG_NORMAL, 1 },
    { "put",		do_put,		POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "quaff",		do_quaff,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "recite",		do_recite,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "remove",		do_remove,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    /*research|lore added by spellsong*/
	{ "research",   do_lore,    POS_RESTING,   0,0,  0,  LOG_NORMAL, 1 },
	{ "sell",		do_sell,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "take",		do_get,		POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "sacrifice",	do_sacrifice,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "junk",           do_junk,	POS_RESTING,   0,0,  0,  LOG_NORMAL, 0 },
    { "tap",      	do_sacrifice,   POS_RESTING,   0,0,  0,  LOG_NORMAL, 0 },
    { "unlock",		do_unlock,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "value",		do_value,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "wear",		do_wear,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "zap",		do_zap,		POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "sharpen",	do_sharpen,	POS_RESTING,	0,0,	0,   LOG_NORMAL, 1 }, /*  edoc  */
    { "push",        do_push,     POS_STANDING,    0,0,    0,   LOG_NORMAL, 1 },
    { "drag",        do_drag,     POS_STANDING,    0,0,    0,   LOG_NORMAL, 1 },
    { "captivate",	do_captivate,	POS_STANDING,  0,0, 0, LOG_NORMAL,	1 },
    { "turn",		do_turn,	POS_STANDING,  0,0,0, LOG_NORMAL,	1 }, /* Eris */	
    /*
     * Combat commands.
     */
    { "backstab",	do_backstab,	POS_STANDING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "bash",		do_bash,	POS_FIGHTING,  0,0,  0,  LOG_NORMAL, 1 },
    { "bs",		do_backstab,	POS_STANDING,  0,0,	 0,  LOG_NORMAL, 0 },
    { "berserk",	do_berserk,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },

	{ "circle",		do_circle,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "dirt",		do_dirt,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "disarm",		do_disarm,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "flee",		do_flee,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "kick",		do_kick,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "rescue",		do_rescue,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 0 },
    { "trip",		do_trip,	POS_FIGHTING,  0,0,  0,  LOG_NORMAL, 1 },
    { "sap",		do_sap,		POS_STANDING,  0,0, 0,	     LOG_NORMAL, 1 },
    { "trample",        do_trample,     POS_STANDING,  0,0, 0,       LOG_NORMAL, 1 },
    { "charge",		do_charge,	POS_STANDING,  0,0,0,	     LOG_NORMAL, 1 },
 
    /*
     * Miscellaneous commands.
     */

	 /*spellsong for traps */
    { "disarm trap",do_disarm_trap, POS_STANDING,  0,0,  0,  LOG_NORMAL, 1 },
    { "follow",		do_follow,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "gain",		do_gain,	POS_STANDING,  0,0,	 0,  LOG_NORMAL, 1 },
    
    { "sgroups",	do_groups,	POS_SLEEPING,  0,0,  0,  LOG_NORMAL, 1 },
    { "hide",		do_hide,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "qui",		do_qui,		POS_DEAD,      0,0,	 0,  LOG_NORMAL, 0 },
    { "quit",		do_quit,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "recall",		do_recall,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "/",		do_recall,	POS_FIGHTING,  0,0,	 0,  LOG_NORMAL, 0 },
    { "rent",		do_rent,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 0 },
    { "save",		do_save,	POS_DEAD,      0,0,	 0,  LOG_NORMAL, 1 },
    { "sleep",		do_sleep,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "sneak",		do_sneak,	POS_STANDING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "split",		do_split,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "steal",		do_steal,	POS_STANDING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "train",		do_train,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "visible",	do_visible,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "wake",		do_wake,	POS_SLEEPING,  0,0,	 0,  LOG_NORMAL, 1 },
    { "where",		do_where,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "brew",		do_brew,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    { "scribe",		do_scribe,	POS_RESTING,   0,0,	 0,  LOG_NORMAL, 1 },
    /*Enter*/
    { "enter",		do_enter,	POS_STANDING,  0,0,	0,   LOG_NORMAL, 1 },


    /*
     * Immortal commands.
     */
    /*Info commands at imm level for some reason*/
  /*  { "murde",		do_murde,	POS_FIGHTING,  0,0,	IM,  LOG_NORMAL, 0 },
   { "murder",		do_murder,	POS_FIGHTING,  0,0,	IM,  LOG_ALWAYS, 1 },
   */
   /*commands after this point that are commented are commands that nobody knows how to use, or
     are only of real use to me.. */

{ "grant",	do_grant,	POS_DEAD,INF_GRANT, GROUP_INF,  ML, LOG_ALWAYS,   1 },
{ "revoke",	do_revoke,	POS_DEAD,INF_REVOKE, GROUP_INF, ML, LOG_ALWAYS,  1 },
{ "addword",	do_addword,	POS_DEAD,SEC_ADVANCE, GROUP_SEC,L5,LOG_ALWAYS, 1 },
{ "advance",	do_advance,	POS_DEAD,SEC_ADVANCE, GROUP_SEC,L1,  LOG_ALWAYS, 1 },
{ "dump",	do_dump,	POS_DEAD,SEC_DUMP, GROUP_SEC,	ML,  LOG_ALWAYS, 0 },
{ "trust",	do_trust,	POS_DEAD,SEC_TRUST, GROUP_SEC,	ML,  LOG_ALWAYS, 1 },
{ "copyover",   do_copyover,    POS_DEAD,SEC_COPYOVER,GROUP_SEC, ML,  LOG_ALWAYS, 1 },
/*Prehaps for the future
{ "violate",	do_violate,	POS_DEAD,SEC_VIOLATE, GROUP_SEC,L1,  LOG_ALWAYS, 1 },
*/
{ "allow",	do_allow,	POS_DEAD,SEC_ALLOW, GROUP_SEC,	L1,  LOG_ALWAYS, 1 },
{ "ban",	do_ban,		POS_DEAD,SEC_BAN, GROUP_SEC,	L1,  LOG_ALWAYS, 1 },
{ "deny",	do_deny,	POS_DEAD,SEC_DENY, GROUP_SEC,	L1,  LOG_ALWAYS, 1 },
{ "cut",        do_new_discon,  POS_DEAD,  SEC_DISCONNECT2,GROUP_SEC,     ML,  LOG_ALWAYS, 1 },
{ "disconnect",	do_disconnect,	POS_DEAD,SEC_DISCONNECT, GROUP_SEC,  L4,  LOG_ALWAYS, 1 },
/*  for future use. PORT FLAG.
{ "flag",	do_flag,	POS_DEAD,SEC_FLAG, GROUP_SEC,	L4,  LOG_ALWAYS, 1 },
*/
{ "freeze",	do_freeze,	POS_DEAD,SEC_FREEZE, GROUP_SEC,	L3,  LOG_ALWAYS, 1 },
/*Future use
{ "protect",	do_protect,	POS_DEAD,SEC_PROTECT, GROUP_SEC,L1,  LOG_ALWAYS, 1 },
*/
{ "sedit",      do_sedit,       POS_DEAD,FUN_SEDIT, GROUP_FUN,  L5,  LOG_ALWAYS, 1 },
{ "hedit",      do_hedit,       POS_DEAD,FUN_HEDIT, GROUP_FUN,  L5,  LOG_ALWAYS, 1 },
{ "set",	do_set,		POS_DEAD,SEC_SET, GROUP_SEC,	L2,  LOG_ALWAYS, 1 },
{ "shutdow",	do_shutdow,	POS_DEAD,SEC_SHUTDOWN, GROUP_SEC,ML,  LOG_NORMAL, 0 },
{ "shutdown",	do_shutdown,	POS_DEAD,SEC_SHUTDOWN, GROUP_SEC,ML,  LOG_ALWAYS, 1 },
{ "wizlock",	do_wizlock,	POS_DEAD,SEC_WIZLOCK, GROUP_SEC,L1,  LOG_ALWAYS, 1 },
/*Repop has the FUN_FOR flag, i'm too lazy to change it*/
{ "repop",	do_repop,	POS_DEAD,FUN_FOR, GROUP_FUN,    L8,  LOG_ALWAYS, 1 },
{ "force",	do_force,	POS_DEAD,SEC_FORCE,   GROUP_SEC,L6,  LOG_ALWAYS, 1 },
{ "load",	do_load,	POS_DEAD,FUN_LOAD, GROUP_FUN,	L6,  LOG_ALWAYS, 1 },
/*"Whonames" might get added later, but it can all go under the same flag*/
{ "sinfo",       do_sendinfo,        POS_DEAD,FUN_NAME, GROUP_FUN,   L8, LOG_NORMAL, 1 },
{ "newlock",	do_newlock,	POS_DEAD,SEC_NEWLOCK, GROUP_SEC,L2,  LOG_ALWAYS, 1 },
{ "nochannels",	do_nochannels,	POS_DEAD,SEC_NOCHANNELS,GROUP_SEC,L5,  LOG_ALWAYS, 1 },
{ "noemote",	do_noemote,	POS_DEAD,SEC_NOEMOTE, GROUP_SEC,	L4,  LOG_ALWAYS, 1 },
{ "noshout",	do_noshout,	POS_DEAD,SEC_NOSHOUT, GROUP_SEC,	L4,  LOG_ALWAYS, 1 },
{ "notell",	do_notell,	POS_DEAD,SEC_NOTELL,  GROUP_SEC,	L4,  LOG_ALWAYS, 1 },
{ "pecho",	do_pecho,	POS_DEAD,FUN_PECHO, GROUP_FUN,	L4,  LOG_ALWAYS, 1 },
{ "pardon",	do_pardon,	POS_DEAD,SEC_PARDON, GROUP_SEC,	L5,  LOG_ALWAYS, 1 },
{ "purge",	do_purge,	POS_DEAD,SEC_PURGE, GROUP_SEC, L7,  LOG_ALWAYS, 1 },
{ "rename",	do_rename,	POS_DEAD, INF_RENAME,GROUP_SEC, L2, LOG_NORMAL, 1 },
{ "restore",	do_restore,	POS_DEAD,FUN_RESTORE, GROUP_FUN,	L6,  LOG_ALWAYS, 1 },
{ "sla",	do_sla,		POS_DEAD,FUN_SLAY, GROUP_FUN,	L4,  LOG_NORMAL, 0 },
{ "slay",	do_slay,	POS_DEAD,FUN_SLAY, GROUP_FUN,	L4,  LOG_ALWAYS, 1 },
/*Slayroom/vict, future use*
{ "slayroom", 	do_slayroom,	POS_DEAD,FUN_SLAYROOM, GROUP_FUN,	L4,  LOG_ALWAYS, 1 },
{ "slayvict",	do_slayvict,	POS_DEAD,FUN_SLAYVICT, GROUP_FUN,	L4,  LOG_ALWAYS, 1 },
*above is for future use*/
{ "teleport",	do_transfer,    POS_DEAD,FUN_TRANSFER, GROUP_FUN,	L8,  LOG_ALWAYS, 1 },
{ "transfer",	do_transfer,	POS_DEAD,FUN_TRANSFER, GROUP_FUN,	L8,  LOG_ALWAYS, 1 },
/*Future use
{ "transin", 	do_transin, 	POS_DEAD,FUN_TRANSIN,  GROUP_FUN, 	L8, LOG_ALWAYS, 1 },
{ "transout",	do_transout,    POS_DEAD,FUN_TRANSIN, GROUP_FUN,       L8, LOG_ALWAYS, 1},
{ "transvict", 	do_transvict,   POS_DEAD,FUN_TRANSIN, GROUP_FUN,       L8, LOG_ALWAYS, 1},
*/
/*EmberMUD*/
{ "rip",           do_sever,       POS_STANDING,  FUN_RIP,GROUP_FUN,     ML,  LOG_ALWAYS, 0 },
{ "beacon",	do_beacon,	POS_DEAD,	FUN_BEACON, GROUP_FUN,   L8,  LOG_NORMAL,0 },
{ "avatar",	do_avatar,	POS_DEAD,      FUN_BEACON,GROUP_FUN,	IM,  LOG_NORMAL, 1 },
{ "chaos",	do_chaos,	POS_DEAD,      FUN_CHAOS,GROUP_FUN,	ML,  LOG_ALWAYS, 1 },
{ "rlist",		do_rlist,	POS_DEAD,  FUN_RLIST,GROUP_FUN,	IM,  LOG_NORMAL, 0 },
/*  Doesn't do anything at the moment
 { "raffect",       do_raffect,     POS_DEAD,      0,0, 	L1,  LOG_ALWAYS, 1 },
*/
{ "dice",	do_dice,	POS_DEAD,INF_DICE,GROUP_INF,
L4,LOG_NORMAL, 1
},

/*Also doesn't appear to do anything
     { "test",          do_test,        POS_DEAD,      0,0, 	IM,  LOG_NORMAL, 0 },
*/
/****/

/*added Spellsong*/
{ "award",  do_award,   POS_DEAD,INF_AWARD,GROUP_INF,   L4,  LOG_ALWAYS, 1 },
/*end added Spellsong*/

{ "poofin",	do_bamfin,	POS_DEAD,FUN_POOFIN, GROUP_FUN,	L8,  LOG_NORMAL, 1 },
{ "poofout",	do_bamfout,	POS_DEAD,FUN_POOFOUT, GROUP_FUN,	L8,  LOG_NORMAL, 1 },
{ "gecho",	do_echo,	POS_DEAD,FUN_GECHO, GROUP_FUN,	L5,  LOG_ALWAYS, 1 },
{ "clan",	do_clan,	POS_DEAD,      SEC_GUILD,GROUP_SEC,	ML,  LOG_ALWAYS, 0 },

{ "holylight",	do_holylight,	POS_DEAD,FUN_HOLYLIGHT, GROUP_FUN,	IM,  LOG_NORMAL, 1 },
/*{ "incognito",	do_incognito,	POS_DEAD,FUN_INCOGNITO, GROUP_FUN,	HE,  LOG_NORMAL, 1 },*/
{ "invis",	do_invis,	POS_DEAD,FUN_WIZINVIS,  GROUP_FUN,	HE,  LOG_NORMAL, 0 },
{ "log",	do_log,		POS_DEAD,SEC_LOG, GROUP_SEC,	L1,  LOG_ALWAYS, 1 },
{ "memory",	do_memory,	POS_DEAD,INF_MEMORY, GROUP_INF,	IM,  LOG_NORMAL, 1 },
{ "mwhere",	do_mwhere,	POS_DEAD,INF_MWHERE, GROUP_INF,	IM,  LOG_NORMAL, 1 },
{ "owhere",	do_owhere,	POS_DEAD,INF_OWHERE, GROUP_INF,	IM,  LOG_NORMAL, 1 },
{ "peace",	do_peace,	POS_DEAD,SEC_PEACE, GROUP_SEC,  L7,  LOG_NORMAL, 1 },
/*futrure
{ "penalty",	do_penalty,	POS_DEAD,INF_PENALTY, GROUP_INF,	L5,  LOG_NORMAL, 1 },
*/
{ "echo",	do_recho,	POS_DEAD,FUN_ECHO, GROUP_FUN,	L6,  LOG_ALWAYS, 1 },
{ "return",     do_return,      POS_DEAD,FUN_RETURN, GROUP_FUN,       L6,  LOG_NORMAL, 1 },
{ "snoop",	do_snoop,	POS_DEAD,SEC_SNOOP, GROUP_FUN,	L5,  LOG_ALWAYS, 1 },
{ "stat",	do_stat,	POS_DEAD,INF_STAT, GROUP_INF,	IM,  LOG_NORMAL, 1 },
{ "string",	do_string,	POS_DEAD,FUN_STRING, GROUP_FUN,	L6,  LOG_ALWAYS, 1 },
{ "switch",	do_switch,	POS_DEAD,FUN_SWITCH, GROUP_FUN,	L6,  LOG_ALWAYS, 1 },
{ "wizinvis",   do_invis,	POS_DEAD,FUN_WIZINVIS, GROUP_FUN,	HE,  LOG_NORMAL, 1 },
{ "vnum",	do_vnum,	POS_DEAD,INF_VNUM, GROUP_INF,	L7,  LOG_NORMAL, 1 },

{ "clone",	do_clone,	POS_DEAD,FUN_CLONE, GROUP_FUN,	L6,  LOG_ALWAYS, 1 },
{ "wiznet",	do_wiznet,	POS_DEAD,INF_WIZNET, GROUP_INF,	IM,  LOG_NORMAL, 1 },
{ "immtalk",	do_immtalk,	POS_DEAD,INF_IMMTALK, GROUP_INF,IM,  LOG_NORMAL, 1 },
/*imptalk channel added by spellsong per nyobe*/
{ "imptalk",	do_imptalk,	POS_DEAD,INF_IMPTALK, GROUP_INF,L2,  LOG_NORMAL, 1 },
{ "imotd",      do_imotd,       POS_DEAD,INF_IMOTD, GROUP_INF,  IM,  LOG_NORMAL, 1 },
{ ",",		do_immtalk,	POS_DEAD,INF_IMMTALK, GROUP_INF,IM,  LOG_NORMAL, 0 },
/*future use
{ "prefi",	do_prefi,	POS_DEAD,FUN_PREFIX, GROUP_FUN,	IM,  LOG_NORMAL, 0 },
{ "prefix",	do_prefix,	POS_DEAD,FUN_PREFIX, GROUP_FUN,	IM,  LOG_NORMAL, 1 },
*/


/******************************************************************************************
 **OLD COMMAND TABLE FOR IMMS, without grant stuff*******
 ** Use this to reinstitute level checks ****************
 ********************************************************
     { "advance",	do_advance,	POS_DEAD,      0,0,	ML,  LOG_ALWAYS, 1 },
     { "dump",		do_dump,	POS_DEAD,     SEC_DUMP, GROUP_SEC, 	ML,  LOG_ALWAYS, 0 },
     { "trust",		do_trust,	POS_DEAD,      0,0,	ML,  LOG_ALWAYS, 1 },
     { "allow",		do_allow,	POS_DEAD,      0,0,	L2,  LOG_ALWAYS, 1 },
     { "ban",		do_ban,		POS_DEAD,      0,0,	L2,  LOG_ALWAYS, 1 },
     { "cut",           do_new_discon,  POS_DEAD,      0,0,     ML,  LOG_ALWAYS, 1 },
     { "deny",		do_deny,	POS_DEAD,      0,0,	L1,  LOG_ALWAYS, 1 },
     { "disconnect",	do_disconnect,	POS_DEAD,      0,0,	L3,  LOG_ALWAYS, 1 },
     { "freeze",	do_freeze,	POS_DEAD,      0,0,	L3,  LOG_ALWAYS, 1 },
     { "reboo",		do_reboo,	POS_DEAD,      0,0,	L1,  LOG_NORMAL, 0 },
     { "reboot",	do_reboot,	POS_DEAD,      0,0,	L1,  LOG_ALWAYS, 1 },
     { "repop",		do_repop,	POS_DEAD,      0,0,	L8,  LOG_ALWAYS, 1 },
     { "sinfo",	  	do_sendinfo,	POS_DEAD,      0,0,	L1,  LOG_ALWAYS, 1 },
     { "set",		do_set,		POS_DEAD,      0,0,	L2,  LOG_ALWAYS, 1 },
     { "shutdow",	do_shutdow,	POS_DEAD,      0,0,	L1,  LOG_NORMAL, 0 },
     { "shutdown",	do_shutdown,	POS_DEAD,      0,0,	L1,  LOG_ALWAYS, 1 },
     { "wizlock",	do_wizlock,	POS_DEAD,      0,0,	L2,  LOG_ALWAYS, 1 },
     { "force",		do_force,	POS_DEAD,      0,0,	L7,  LOG_ALWAYS, 1 },
     { "load",		do_load,	POS_DEAD,      0,0,	L7,  LOG_ALWAYS, 1 },
     { "newlock",	do_newlock,	POS_DEAD,      0,0,	L4,  LOG_ALWAYS, 1 },
     { "nochannels",	do_nochannels,	POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "noemote",	do_noemote,	POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "noshout",	do_noshout,	POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "notell",	do_notell,	POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "pecho",		do_pecho,	POS_DEAD,      0,0,	L4,  LOG_ALWAYS, 1 },
     { "pardon",	do_pardon,	POS_DEAD,      0,0,	L3,  LOG_ALWAYS, 1 },
     { "purge",		do_purge,	POS_DEAD,      0,0,	L7,  LOG_ALWAYS, 1 },
     { "restore",	do_restore,	POS_DEAD,      0,0,	L4,  LOG_ALWAYS, 1 },
     { "sla",		do_sla,		POS_DEAD,      0,0,	L3,  LOG_NORMAL, 0 },
     { "slay",		do_slay,	POS_DEAD,      0,0,	L3,  LOG_ALWAYS, 1 },
     { "teleport",	do_transfer,    POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "transfer",	do_transfer,	POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "poofin",	do_bamfin,	POS_DEAD,      0,0,	L8,  LOG_NORMAL, 1 },
     { "poofout",	do_bamfout,	POS_DEAD,      0,0,	L8,  LOG_NORMAL, 1 },
     { "gecho",		do_echo,	POS_DEAD,      0,0,	L4,  LOG_ALWAYS, 1 },
     { "holylight",	do_holylight,	POS_DEAD,      0,0,	IM,  LOG_NORMAL, 1 },
     { "invis",		do_invis,	POS_DEAD,      0,0,	IM,  LOG_NORMAL, 0 },
     { "log",		do_log,		POS_DEAD,      0,0,	L1,  LOG_ALWAYS, 1 },
     { "memory",	do_memory,	POS_DEAD,      0,0,	IM,  LOG_NORMAL, 1 },
     { "mwhere",	do_mwhere,	POS_DEAD,      0,0,	IM,  LOG_NORMAL, 1 },
     { "peace",		do_peace,	POS_DEAD,      0,0,	L5,  LOG_NORMAL, 1 },
     { "echo",		do_recho,	POS_DEAD,      0,0,	L6,  LOG_ALWAYS, 1 },
     { "return",        do_return,      POS_DEAD,      0,0,     L6,  LOG_NORMAL, 1 },
     { "rip",           do_sever,       POS_STANDING,  0,0,     ML,  LOG_ALWAYS, 0 },
     { "snoop",		do_snoop,	POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "stat",		do_stat,	POS_DEAD,      0,0,	IM,  LOG_NORMAL, 1 },
     { "string",	do_string,	POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "switch",	do_switch,	POS_DEAD,      0,0,	L6,  LOG_ALWAYS, 1 },
     { "wizinvis",	do_invis,	POS_DEAD,      0,0,	IM,  LOG_NORMAL, 1 },
     { "vnum",		do_vnum,	POS_DEAD,      0,0,	L7,  LOG_NORMAL, 1 },
     { "clone",		do_clone,	POS_DEAD,      0,0,	L5,  LOG_ALWAYS, 1 },
     { "wiznet",        do_wiznet,      POS_DEAD,      0,0, 	IM,  LOG_NORMAL, 1 },
     { "immtalk",	do_immtalk,	POS_DEAD,      0,0,	HE,  LOG_NORMAL, 1 },
     { "imotd",         do_imotd,       POS_DEAD,      0,0, 	HE,  LOG_NORMAL, 1 },
     { ",",		do_immtalk,	POS_DEAD,      0,0,	HE,  LOG_NORMAL, 0 },
     { "beacon",        do_beacon,      POS_DEAD,      0,0, 	L7,  LOG_NORMAL, 1 },
     { "chaos",		do_chaos,	POS_DEAD,      0,0,	ML,  LOG_ALWAYS, 1 },
     { "clan",		do_clan,	POS_DEAD,      0,0,	ML,  LOG_ALWAYS, 0 },
     { "rlist",		do_rlist,	POS_DEAD,      0,0,	IM,  LOG_NORMAL, 0 },
     { "raffect",       do_raffect,     POS_DEAD,      0,0, 	L1,  LOG_ALWAYS, 1 },
     { "test",          do_test,        POS_DEAD,      0,0, 	IM,  LOG_NORMAL, 0 },
     { "avatar",	do_avatar,	POS_DEAD,      0,0,	IM,  LOG_NORMAL, 1 },
*****************************************************************************************/
	{ "spellcheck",	do_spellcheck,		POS_DEAD,SEC_EDIT,GROUP_SEC,L8,LOG_ALWAYS, 1 },
{"rgrant",do_rgrant, POS_DEAD, INF_RELIGION,GROUP_INF, L3, LOG_ALWAYS, 0},
{"convert",do_convert, POS_STANDING, 0,0, 1, LOG_ALWAYS, 0},
{"excommunicate",do_excommunicate, POS_STANDING, 0,0, 1, LOG_ALWAYS, 0},
{"reltalk",do_reltalk, POS_SLEEPING, 0,0, 1, LOG_NORMAL, 0},
    /*
     * OLC
     */
	{ "edit",	do_olc,		POS_DEAD,SEC_EDIT, GROUP_SEC,L8,  LOG_NORMAL, 1 },
	{ "asave",      do_asave,	POS_DEAD,SEC_ASAVE,GROUP_SEC,L8,  LOG_NORMAL, 1 },
	{ "alist",	do_alist,	POS_DEAD,INF_ALIST,GROUP_INF,L8,  LOG_NORMAL, 1 },
	{ "resets",	do_resets,	POS_DEAD,INF_RESETS,GROUP_INF,L8, LOG_NORMAL, 1 },
	/*future use
	{ "hsave",      do_save_helps,  POS_DEAD,SEC_HSAVE,GROUP_SEC,ML,  LOG_ALWAYS, 1 },
	 */
     /*****PREGRANT olc command table***
    { "edit",		do_olc,		POS_DEAD,   0,0, HE,  LOG_ALWAYS, 1 },
    { "asave",          do_asave,	POS_DEAD,   0,0, HE,  LOG_ALWAYS, 1 },
    { "alist",		do_alist,	POS_DEAD,   0,0, HE,  LOG_NORMAL, 1 },
    { "resets",		do_resets,	POS_DEAD,   0,0, HE,  LOG_ALWAYS, 1 },
      ***/
    /*
     * MOBprogram commands.
     */
    { "mpstat",         do_mpstat,      POS_DEAD,   0,0,    L7,  LOG_NORMAL, 1 },
    { "mpasound",       do_mpasound,    POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpjunk",         do_mpjunk,      POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpecho",         do_mpecho,      POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpechoat",       do_mpechoat,    POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpechoaround",   do_mpechoaround,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpkill",         do_mpkill      ,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpmload",        do_mpmload     ,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpoload",        do_mpoload     ,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mppurge",        do_mppurge     ,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpgoto",         do_mpgoto      ,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpat",           do_mpat        ,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mptransfer",     do_mptransfer  ,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },
    { "mpforce",        do_mpforce     ,POS_DEAD,   0,0,     0,  LOG_NORMAL, 0 },

    /*
     * End of list.
     */
    { "",		0,		POS_DEAD,   0,0,	 0,  LOG_NORMAL, 0 }
};
/*********
 ** NEW interpert
 ******************
 */

void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    bool found;

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    /*
     * No hiding..."well kinda" -Akira
     */

	if(!IS_AFFECTED(ch, AFF_SNEAK))
	    REMOVE_BIT( ch->affected_by, AFF_HIDE );

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name )
	&& ( ( ( ( (cmd_table[cmd].group == GROUP_SEC ) && ( cmd_table[cmd].flag & ch->secflags ) )
	 ||
	 ( (cmd_table[cmd].group == GROUP_FUN ) && ( cmd_table[cmd].flag & ch->funflags ) )
	 ||
	 ( (cmd_table[cmd].group == GROUP_INF ) && ( cmd_table[cmd].flag & ch->infflags ) ) ) )

	||
	   ( cmd_table[cmd].level < HE ) ) )
	{

	    found = TRUE;
	    break;
	}
    }

    /*
     * Log and snoop.
     */
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   cmd_table[cmd].log == LOG_ALWAYS )
    {
	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
	if ( !check_social( ch, command, argument ) )
	    send_to_char( "Huh?\n\r", ch );
	return;
    }

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain( );
    return;
}


/*OLD interp****/

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
/*oldinterp
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    bool found;

    *
     * Strip leading spaces.
     *
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    *
     * No hiding.
     *
    REMOVE_BIT( ch->affected_by, AFF_HIDE );

    *
     * Implement freeze command.
     *
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    *
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     *
    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    *
     * Look for command in command table.
     *
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name )
	&&   cmd_table[cmd].level <= trust )
	{
	    found = TRUE;
	    break;
	}
    }

    if (IS_SET(ch->act,PLR_AFK) && !IS_NPC(ch) && trust < L2 )
      {
	if (str_prefix(command,"afk"))
	{
	 send_to_char( "You don't seem to be afk to me!\n\r", ch );
	 return;
	}
      }

    *
     * Log and snoop.
     *
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   cmd_table[cmd].log == LOG_ALWAYS )
    {
	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
	*
	 * Look for command in socials table.
	 *
	if ( !check_social( ch, command, argument ) )
	    send_to_char( "Huh?\n\r", ch );
	return;
    }

    *
     * Character not in position for command?
     *
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

    *
     * Dispatch the command.
     *
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain( );
    return;
}

*/

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim)
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;

    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

	buf[0] = '\0'; /*spellsong add*/
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
	&&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}
/*New wizhelp with grant*/
void do_wizhelp( CHAR_DATA *ch, char *argument )
{
  int col;
  int cmd;
  char buf[MAX_STRING_LENGTH];
  col = 0;
  buf[0] = '\0'; /*spellsong add*/

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if (
	( ( (cmd_table[cmd].group == GROUP_SEC ) && ( cmd_table[cmd].flag & ch->secflags ) )
	 ||
	 ( (cmd_table[cmd].group == GROUP_FUN ) && ( cmd_table[cmd].flag & ch->funflags ) )
	 ||
	 ( (cmd_table[cmd].group == GROUP_INF ) && ( cmd_table[cmd].flag & ch->infflags ) ) )
	 &&
	   ( cmd_table[cmd].level > HE )

	   )
	{
	    sprintf(buf,"`B[`M%d`B] `M%-12s ",
                         cmd_table[cmd].level,        
                         cmd_table[cmd].name);
	    send_to_char(buf,ch);
            if ( ++col % 4 == 0 )
              send_to_char( "\n\r", ch );
        }
    }
   if ( col % 4 != 0 )
      send_to_char( "\n\r", ch );
   return;
}


/*Old wizhelp
void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( cmd_table[cmd].level >= LEVEL_HERO
	&&   cmd_table[cmd].level <= get_trust( ch )
	&&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }

    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}
*/
