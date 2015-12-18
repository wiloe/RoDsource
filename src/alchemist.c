/*
 *      Alchemist's spells/skills file
 *              Akira, Divinos, Spellsong, Eris, resident coders
 *                      May 7, 2000 started
 */
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

/*****************
* TEMPLAR SPELLS *
*****************/

/* PROTECTION */

void spell_armor2( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) )
    {
if (victim ==
ch)	  send_to_char("You are already protected.\n\r",ch);	else	  act("$N is already
armored.",ch,NULL,victim,TO_CHAR);	return;    }    af.type      = sn;    af.level	 =
level;    af.duration  = 10+level;    af.modifier  = -20;    af.location  = APPLY_AC;    af.bitvector =
0;    affect_to_char( victim, &af );    send_to_char( "You feel someone protecting you.\n\r", victim
);    if ( ch != victim )	act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);    return;}
