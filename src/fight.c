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

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			                   *
*	ROM has been brought to you by the ROM consortium		               *
*	    Russ Taylor (rtaylor@hypercube.org)				                   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			               *
*	    Brian Moore (zump@rom.org)					                       *
*	By using this code, you have agreed to follow the terms of the	       *
*	ROM license, in the file Rom24/doc/rom.license			               *
*                                                                          *
* MODIFIED RECODED BY SPELLSONG FOR REALMS OF DISCORDIA AND EMBERMUD       *
* Email at nfn15571@naples.net as well for any requests for reuse. Until   *
* such permission is explicitly given, no reuse is authorized.             *
***************************************************************************/
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#define MAX_DAMAGE_MESSAGE 38 /*was 32 - spellsong*/

/*for weapon effects - SS*/
#define VAMP 1033
#define FIRE 1034
#define FROST 1035
#define SHOCK 1036
#define ACID 1037
#define SERRATION 1038

#define VORPAL_DAM -3000 /*just a number way out of range for use with vorpal - spellsong*/

/* command procedures needed */
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_disarm_trap);
DECLARE_DO_FUN(do_circle	); /*spellsong 3/1/2000*/


/*
 * Local functions.
 */

/*Spellsong Add*/
bool    check_counter   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) );
bool    check_critical  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_phase     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_circle		args( ( CHAR_DATA *ch, char *argument ) );
/*Bug Fix For Shield Block: i.e. it works now. Spellsong*/
bool    check_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    vorpal_kill		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dam_type ) );
/*End Spellsong Add*/

void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	dam_message		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt,
							 bool immune ) );
void	death_cry		args( ( CHAR_DATA *ch ) );
void	group_gain		args( ( CHAR_DATA *ch, CHAR_DATA *victim , bool vorpal ) );
int		xp_compute		args( ( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels,
							 int members, bool vorpal ) );
bool	is_safe			args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse		args( ( CHAR_DATA *ch ) );
void	one_hit			args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	second_one_hit  args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    mob_hit			args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	chaos_kill		args( ( CHAR_DATA *victim ) );
void	raw_kill		args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm			args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    chaos_log		args( ( CHAR_DATA *ch, char *argument ) );

void adjust_sharpness_affects(CHAR_DATA *ch,OBJ_DATA *obj);

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
		ch_next	= ch->next;

		if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
			continue;

		if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
			multi_hit( ch, victim, TYPE_UNDEFINED );
		else 
		{
			if (!IS_NPC(victim)) 
			{
				victim->pcdata->nemesis=victim->fighting;	
			}
			stop_fighting( ch, FALSE );
		}

		if ( ( victim = ch->fighting ) == NULL )
			continue;

	   mprog_hitprcnt_trigger( ch, victim );
	   mprog_fight_trigger( ch, victim );

		/*
		 * Fun for the whole family!
		 */
		check_assist(ch,victim);
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
		rch_next = rch->next_in_room;
		
		if (IS_AWAKE(rch) && rch->fighting == NULL)
		{
			if (RIDDEN(rch) == ch || MOUNTED(rch) == ch)
			{
				multi_hit(rch,victim,TYPE_UNDEFINED);
			}
			
			/* quick check for ASSIST_PLAYER */
			if (!IS_NPC(ch) && IS_NPC(rch) 
			&& IS_SET(rch->off_flags,ASSIST_PLAYERS)
			&&  rch->level + 6 > victim->level
			&& !IS_SET(ch->act, PLR_KILLER))
			{
				do_emote(rch,"screams and attacks!");
				multi_hit(rch,victim,TYPE_UNDEFINED);
				continue;
			}

			/* PCs next */
			if ((!IS_NPC(ch) && IS_NPC(victim)) 
			|| (IS_AFFECTED(ch,AFF_CHARM)) )
			{
				if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
				||     IS_AFFECTED(rch,AFF_CHARM)) 
				&&   is_same_group(ch,rch) )
				{
					if (rch != NULL && victim != NULL) /*just in case check Spellsong*/
						multi_hit (rch,victim,TYPE_UNDEFINED);
				}
				continue;
			}
  		
			/* now check the NPC cases */
			
 			if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
		
			{
				if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

				||   (IS_NPC(rch) && rch->race == ch->race 
				   && IS_SET(rch->off_flags,ASSIST_RACE))

				||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
				   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
					 ||  (IS_EVIL(rch)    && IS_EVIL(ch))
					 ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

				||   (rch->pIndexData == ch->pIndexData 
				   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   			{
					CHAR_DATA *vch;
					CHAR_DATA *target;
					int number;

					if (number_bits(1) == 0)
					continue;
				
					target = NULL;
					number = 0;
					for (vch = ch->in_room->people; vch; vch = vch->next)
					{
						if (can_see(rch,vch)
						&&  is_same_group(vch,victim)
						&&  number_range(0,number) == 0)
						{
							target = vch;
							number++;
						}
					}

					if (target != NULL && rch != NULL) /*just in case spellsong*/
					{
						do_emote(rch,"screams and attacks!");
						multi_hit(rch,target,TYPE_UNDEFINED);
					}
				}	
			}
		}
    }
	return; /*spellsong*/
}

/*NEW MULITHIT - Spellsong 22May2000*/
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *second_wield;
    int     chance;
	bool    dual_wield;
	bool    hasted;
	bool    slowed;
	bool    second_attack;
	bool    third_attack;

	dual_wield = FALSE;
	hasted = FALSE;
	slowed = FALSE;
	second_attack = FALSE;
	third_attack = FALSE;

    /*just in case - Spellsong*/
    if (ch == NULL || victim == NULL)
		return;
	
    /* Timer to prevent logging out -Rhys */
    if( !IS_NPC( ch ) )
        ch->pcdata->fight_timer = 2; /* # of ticks */

    /* decrement the wait */
    if (ch->desc == NULL)
		ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
		return;

    if (IS_NPC(ch))
    {
		mob_hit(ch,victim,dt);
		return;
    }

    if (MOUNTED(ch))
    {
		if (!mount_success(ch, MOUNTED(ch), FALSE))
		{
			send_to_char("You fall off your mount as you start your attacks!\n\r", ch);
			return;
		}
	}
	
	if ( (second_wield = get_eq_char( ch, WEAR_SECOND_WIELD ) ) != NULL )
    	dual_wield = TRUE;

	if (IS_AFFECTED(ch,AFF_HASTE))
		hasted = TRUE;
	
	if (IS_AFFECTED(ch,AFF_SLOW))
		slowed = TRUE;

	chance = get_skill(ch,gsn_second_attack);
    /* if ( number_percent( ) < chance ) */
    if ( number_percent( ) < chance && number_percent() < 50) // Eris 30 July 2000
    {
		second_attack = TRUE;
		check_improve(ch,gsn_second_attack,TRUE,5);
	}

    chance = get_skill(ch,gsn_third_attack)/2;
    /* if ( number_percent( ) < chance ) */
    if ( number_percent( ) < chance && number_percent() < 50) // Eris 30 July 2000
    {
		third_attack = TRUE;
		check_improve(ch,gsn_third_attack,TRUE,6);
    }
		
	//parse attacks

	//always atleast one hit
	one_hit( ch, victim, dt );
    
	if( !dual_wield && !hasted && !second_attack && !third_attack )
		return;

	//target still alive?
	if ( ch->fighting != victim )
		return;

	if( dual_wield && !hasted && !second_attack && !third_attack && slowed )
		return;
	else
	{
		if( dual_wield )
			second_one_hit( ch, victim, dt);
	}

	//target still alive?
	if ( ch->fighting != victim )
		return;
	
	if( hasted && !second_attack && !third_attack && slowed )
		return;
	else
	{
		if( hasted )
			one_hit( ch, victim, dt );
	}

	//target still alive or a backstab or circle victim?
	if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle )
		return;
	
	if( second_attack && !third_attack && slowed )
		return;
	else
	{
		if( second_attack )
			one_hit( ch, victim, dt );
	}

	//target still alive?
	if ( ch->fighting != victim )
		return;
	
	if( !third_attack || slowed )
		return;
	else
		one_hit( ch, victim, dt );
	
    return;
}

/*NEW MOB_HIT FUNCTION CODED BY SPELLSONG - 22May2000*/
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number,act_level;
    CHAR_DATA *vch, *vch_next;
	bool    hasted;
	bool    fast;
	bool    slowed;
	bool    second_attack;
	bool    third_attack;

	fast = FALSE;
	hasted = FALSE;
	slowed = FALSE;
	second_attack = FALSE;
	third_attack = FALSE;

    if(ch == NULL || victim == NULL) /*just in case spellsong*/
		return;

	//always will attack atleast once
	one_hit(ch,victim,dt);

    //target still there and alive?
	if (ch->fighting != victim)
		return;

    
	//area attacks not affected by slow
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
		for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
		{
			vch_next = vch->next;
			if ((vch != victim && vch->fighting == ch && vch != NULL)) /*null check added by spell*/
				one_hit(ch,vch,dt);
		}
    }

	//special offenses - slow does not affect them
    number = number_range(0,7);

    switch(number) 
    {
		case (0) :
		if (IS_SET(ch->off_flags,OFF_BASH))
			do_bash(ch,"");
		break;

		case (1) :
		if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
			do_berserk(ch,"");
		break;


		case (2) :
		if (IS_SET(ch->off_flags,OFF_DISARM) 
		|| (get_weapon_sn(ch) != gsn_hand_to_hand 
		&& (IS_SET(ch->act,ACT_WARRIOR)
   		||  IS_SET(ch->act,ACT_THIEF))))
			do_disarm(ch,"");
		break;

		case (3) :
		if (IS_SET(ch->off_flags,OFF_KICK))
			do_kick(ch,"");
		break;

		case (4) :
		if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
			do_dirt(ch,"");
		break;

		/*case (5) :
		if (IS_SET(ch->off_flags,OFF_TAIL))
			do_tail(ch,"");
		break; */

		case (6) :
		if (IS_SET(ch->off_flags,OFF_TRIP))
			do_trip(ch,"");
		break;

		/*case (7) :
		if (IS_SET(ch->off_flags,OFF_CRUSH))
			do_crush(ch,"");
		break;*/
    }

	if (IS_AFFECTED(ch,AFF_SLOW))
		slowed = TRUE;
	
	if (IS_AFFECTED(ch,AFF_HASTE)) 
		hasted = TRUE;
	
	if (IS_SET(ch->off_flags,OFF_FAST))
		fast = TRUE;
	    
	//parse extra attacks based on ACT flags
	if(IS_SET(ch->act,ACT_WARRIOR))
		act_level = 30;
	else
	{
		if(IS_SET(ch->act,ACT_THIEF))
			act_level = 35;
		else
		{
			if(IS_SET(ch->act,ACT_CLERIC))
				act_level = 45;
			else
				act_level = 60;
		}
	}

    chance = get_skill(ch,gsn_second_attack);
    if (number_percent() < chance && ch->level > act_level)
		second_attack = TRUE;

    chance = get_skill(ch,gsn_third_attack)/2;
    if (number_percent() < chance && ch->level > act_level)
		third_attack = TRUE;

	if( !hasted && !fast && !second_attack && !third_attack )
		return;
	
	if( hasted && slowed && !fast && !second_attack && !third_attack )
		return;
	else
	{
		if( hasted )
			one_hit(ch,victim,dt);
	}

	if (ch->fighting != victim || dt == gsn_backstab)
		return;

	if( slowed && fast && !second_attack && !third_attack )
		return;
	else
	{
		if( fast )
			one_hit(ch,victim,dt);
	}

	//target still there and alive?
	if (ch->fighting != victim)
		return;

    if( slowed && second_attack && !third_attack )
		return;
	else
	{
		if( second_attack )
			one_hit(ch,victim,dt);
	}

	//target still there and alive?
	if (ch->fighting != victim)
		return;

	if( !third_attack || slowed )
		return;
	else
		one_hit( ch, victim, dt );

	
	return;
}

/*NOTE: OLD MULTIHIT FUNCTION COMMENTED OUT AND REWRITTEN TO
        PROPERLY ACCOMODATE THE SLOW SPELL. - SPELLSONG - 22MAY2000*/

/*void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *second_wield;
    int     chance;

    
    if (ch == NULL || victim == NULL)
		return;
	
    
    if( !IS_NPC( ch ) )
        ch->pcdata->fight_timer = 2; 

    if (ch->desc == NULL)
		ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

    
    if (ch->position < POS_RESTING)
		return;

    if (IS_NPC(ch))
    {
		mob_hit(ch,victim,dt);
		return;
    }

    if (MOUNTED(ch))
    {
       if (!mount_success(ch, MOUNTED(ch), FALSE))
       send_to_char("You fall off your mount as you start your attacks!\n\r", ch);
    }

    one_hit( ch, victim, dt );
    
	if ( ch->fighting != victim ) 
			return;               

	if ( (second_wield = get_eq_char( ch, WEAR_SECOND_WIELD ) ) != NULL )
        second_one_hit( ch, victim, dt);

    if (ch->fighting != victim)
		return;

    if (IS_AFFECTED(ch,AFF_HASTE))
		one_hit(ch,victim,dt);

    if ( ch->fighting != victim || dt == gsn_backstab )
		return;
	
	if ( dt == gsn_circle )
		return;

    chance = get_skill(ch,gsn_second_attack);
    if ( number_percent( ) < chance )
    {
		one_hit( ch, victim, dt );
		check_improve(ch,gsn_second_attack,TRUE,5);
		if ( ch->fighting != victim )
			return;
    }

    chance = get_skill(ch,gsn_third_attack)/2;
    if ( number_percent( ) < chance )
    {
		one_hit( ch, victim, dt );
		check_improve(ch,gsn_third_attack,TRUE,6);
		if ( ch->fighting != victim )
			return;
    }

    return;
}*/

/* NOTE: sommented off and rewritten by spellsong for adding in slow - 22May2000 */
/*void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

    if(ch == NULL || victim == NULL) 
		return;

	one_hit(ch,victim,dt);

    if (ch->fighting != victim)
		return;
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
		for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
		{
			vch_next = vch->next;
			if ((vch != victim && vch->fighting == ch && vch != NULL)) 
				one_hit(ch,vch,dt);
		}
    }

    if (ch->fighting != victim) 
		return;
	
	if (IS_AFFECTED(ch,AFF_HASTE) || IS_SET(ch->off_flags,OFF_FAST))
		one_hit(ch,victim,dt);

    if (ch->fighting != victim || dt == gsn_backstab)
		return;

    chance = get_skill(ch,gsn_second_attack);
    if (number_percent() < chance)
    {
		one_hit(ch,victim,dt);
		if (ch->fighting != victim)
			return;
    }

    chance = get_skill(ch,gsn_third_attack)/2;
    if (number_percent() < chance)
    {
		one_hit(ch,victim,dt);
		if (ch->fighting != victim)
			return;
    } 

    if (ch->wait > 0)
		return;

    number = number_range(0,7);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_bash(ch,"");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_berserk(ch,"");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch) != gsn_hand_to_hand 
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_disarm(ch,"");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_kick(ch,"");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_dirt(ch,"");
	break;

    

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_trip(ch,"");
	break;

	return;
}*/
	

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
	/*spellsong add*/
	bool result;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
		return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
		return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char( ch, WEAR_WIELD );

    if ( dt == TYPE_UNDEFINED )
    {
		dt = TYPE_HIT;
		if ( wield != NULL && wield->item_type == ITEM_WEAPON )
			dt += wield->value[3];
		else 
			dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
		dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
		thac0_00 = 20;
		thac0_32 = -4;   /* as good as a thief */ 
		if (IS_SET(ch->act,ACT_WARRIOR))
			thac0_32 = -10;
		else if (IS_SET(ch->act,ACT_THIEF))
			thac0_32 = -4;
		else if (IS_SET(ch->act,ACT_CLERIC))
			thac0_32 = 2;
		else if (IS_SET(ch->act,ACT_MAGE))
			thac0_32 = 6;
    }
    else
    {
		thac0_00 = class_table[ch->class].thac0_00;
		thac0_32 = class_table[ch->class].thac0_32;
		if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
				thac0_32 += 4;
    }

    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
		thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

	/*make it harder to hit than backstab*/
	if (dt == gsn_circle)
		thac0 -= 2 * (100 - get_skill(ch,gsn_circle));

    switch(dam_type)
    {
		case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
		case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
		case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
		default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
		if (!ch->pIndexData->new_format)
		{
			dam = number_range( ch->level / 2, ch->level * 3 / 2 );
			if ( wield != NULL )
	    		dam += dam / 2;
		}
		else
			dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
		/* end if*/
    else
	{
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	
	/*if structure changed - spellsong*/
	if ( wield == NULL )
		dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
	else
	{
	    if (wield->pIndexData->new_format)
			dam = dice(wield->value[1],wield->value[2]) * skill/100;
	    else
	    	dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 21/20;

		/* sharpness! spellsong add from ROM2.4*/
	    if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
	    {
			int percent;

			if ((percent = number_percent()) <= (skill / 8))
				dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}    
    }

    /*
     * Bonuses.
     */
    if ( check_critical(ch,victim) )
        dam *= 1.20; /*Added by spellsong before enhanced damage bonus to minimize a bit*/

	if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += dam * diceroll/100;
        }
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
    else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    /*Buggy code...spellsong - could cause calculation of bonus twice*/
	/*if ( dt == gsn_backstab && wield != NULL) 
    	if ( wield->value[0] != 2 )
	    dam *= 2 + ch->level / 10; 
	else 
	    dam *= 2 + ch->level / 8;*/

	/*replaced with this -spellsong*/
	if ( dt == gsn_backstab && wield != NULL) 
	{    	
		if ( wield->value[0] != 2 )
			dam *= 2 + ch->level / 10; 
		else 
			dam *= 2 + ch->level / 8;
		/*end if*/
	}

	/*end if*/

	/*end spellsong fix*/

	/*circle stuff - spellsong*/
	if ( dt == gsn_circle && wield != NULL)
    {
		if ( wield->value[0] != 2 )
           dam *= 2+ (ch->level / 14);
        else
           dam *=2 + (ch->level / 12);
		/*end if*/
	}
	/*end if*/

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( dam <= 0 )
	dam = 1;
    	
	/*ok I am thinking a magical weapon meant to cut off heads should be impossible to
	  counter when working its magic...just a thought Spellsong*/
	if(wield != NULL && IS_WEAPON_STAT(wield,WEAPON_VORPAL))
	{
		if (vorpal_kill( ch, victim, dam, dam_type ) == TRUE )
		{	
			damage( ch, victim, VORPAL_DAM, dt, dam_type);
			tail_chain( );
			return;
		}
	}

	/*damage( ch, victim, dam, dt, dam_type );- comented by spellsong not sure how
	  we want to work this so i am countering all damage including bonuses - spellsong*/
	if ( !check_counter( ch, victim, dam, dt ) )
    {
		/*ok we didnt counter*/
		result = damage( ch, victim, dam, dt, dam_type);
    
		/* but do we have a funky weapon? */
		/*spellsong hack from rom 2.4*/
		if (result && wield != NULL)
		{ 
			int wdam;

			if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
			{
				int level;
				AFFECT_DATA *poison, af;

				if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
					level = wield->level;
				else
					level = poison->level;
			
				if (!saves_spell(level / 2,victim)) 
				{
					send_to_char("You feel poison coursing through your veins.",
						victim);
					act("$n is poisoned by the venom on $p.",
						victim,wield,NULL,TO_ROOM_FIGHT);
    				act("$n is poisoned by the venom on $p.",
						victim,wield,NULL,TO_CHAR_FIGHT);

    					af.type      = gsn_poison;
    					af.level     = level * 3/4;
    					af.duration  = level / 2;
    					af.location  = APPLY_STR;
    					af.modifier  = -1;
    					af.bitvector = AFF_POISON;
    					affect_join( victim, &af );
				}

				/* weaken the poison if it's temporary */
				if (poison != NULL)
				{
	    			poison->level = UMAX(0,poison->level - 2);
	    			poison->duration = UMAX(0,poison->duration - 1);
			
	    			if (poison->level == 0 || poison->duration == 0)
					act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR_FIGHT);
				}
 			}


    		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
			{
				wdam = number_range(1, wield->level / 5 + 1);
				act("$p draws life from $n.",victim,wield,NULL,TO_ROOM_FIGHT);
				act("You feel $p drawing your life away.",
				victim,wield,NULL,TO_CHAR_FIGHT);
				damage(ch,victim,wdam,1033,DAM_NEGATIVE);
				ch->alignment = UMAX(-1000,ch->alignment - 1);
				ch->hit += wdam/2;
			}

			if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
			{
				wdam = number_range(1,wield->level / 4 + 1);
				act("$n is burned by $p.",victim,wield,NULL,TO_ROOM_FIGHT);
				act("$p sears your flesh.",victim,wield,NULL,TO_CHAR_FIGHT);
				fire_effect( victim,wield->level/2,dam,TARGET_CHAR);
				damage(ch,victim,wdam,1034,DAM_FIRE);
			}

			if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
			{
				wdam = number_range(1,wield->level / 6 + 2);
				act("$p freezes $n.",victim,wield,NULL,TO_ROOM_FIGHT);
				act("The cold touch of $p surrounds you with ice.",
				victim,wield,NULL,TO_CHAR_FIGHT);
				cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
				damage(ch,victim,wdam,1035,DAM_COLD);
			}

			if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
			{
				wdam = number_range(1,wield->level/5 + 2);
				act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM_FIGHT);
				act("You are shocked by $p.",victim,wield,NULL,TO_CHAR_FIGHT);
				shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
				damage(ch,victim,wdam,1036,DAM_LIGHTNING);
			}

			if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_ACID))
			{
				wdam = number_range(1,wield->level/5 + 2);
				act("$n is burned by $p.",victim,wield,NULL,TO_ROOM_FIGHT);
				act("$p eats your flesh.",victim,wield,NULL,TO_CHAR_FIGHT);
				acid_effect(victim,wield->level/2,dam,TARGET_CHAR);
				damage(ch,victim,wdam,1037,DAM_ACID);
			}

			if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SERRATED))
			{
				int level;
				AFFECT_DATA  af;
				
				wdam = number_range(1,wield->level/5 + 2);
				act("$n is `Rbled`w by $p.",victim,wield,NULL,TO_ROOM_FIGHT);
				act("$p makes you `Rbleed`w.",victim,wield,NULL,TO_CHAR_FIGHT);
				/*serrated_effect(victim,wield->level/2,dam,TARGET_CHAR);*/
				damage(ch,victim,wdam,1038,DAM_SLASH);
				
				level = wield->level;
			
				if (!saves_spell(level / 2,victim)) 
				{
					send_to_char("You feel weak from blood loss.",
						victim);
					act("$n has a `RBLEEDING`w wound from $p.",
						victim,wield,NULL,TO_ROOM_FIGHT);
    				act("$n has a `RBLEEDING`w wound from $p.",
						victim,wield,NULL,TO_CHAR_FIGHT);

    					af.type      = skill_lookup( "weaken" );
    					af.level     = level * 3/4;
    					af.duration  = level / 2;
    					af.location  = APPLY_STR;
    					af.modifier  = -1;
    					af.bitvector = AFF_WEAKEN;
    					affect_join( victim, &af );
				}
 				
			}
		}/*end spelly hack*/
	}
        /* the damage is done, lets fix the sharp stuff */
        adjust_sharpness_affects(ch,wield);

	tail_chain( );

	return;
	/*end if*/
	/*return; commented per above - spell*/
}

void second_one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
	bool result;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char( ch, WEAR_SECOND_WIELD );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_second_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	/*well its a tad harder to hit with that second weapon is why all values
	  are different than first weild*/
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -6;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4; /*thieves get no penalties for dual wield*/
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 6;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 10;
    }
    else
    {
		thac0_00 = class_table[ch->class].thac0_00;
		thac0_32 = class_table[ch->class].thac0_32;
		/*same for character thieves*/
		if (ch->class != 2)
			thac0_32 += 4; /*add 4 to everyone else*/
		if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
				thac0_32 += 4;
    }

    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
		thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));
	
	switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( wield != NULL )
	    	dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
	
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	
    if ( wield == NULL )
		dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
	else
	{
	    if (wield->pIndexData->new_format)
			dam = dice(wield->value[1],wield->value[2]) * skill/100;
	    else
	    	dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 21/20;

		/* sharpness! spellsong add from ROM2.4*/
	    if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
	    {
			int percent;

			if ((percent = number_percent()) <= (skill / 8))
				dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}    
	
	}

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += dam * diceroll/100;
        }
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    /*buggy fight code backstab - spellsong*/
	/*if ( dt == gsn_backstab && wield != NULL) 
    	if ( wield->value[0] != 2 )
	    dam *= 2 + ch->level / 10; 
	else 
	    dam *= 2 + ch->level / 8;*/

	 /*replaced with this -spellsong*/
	if ( dt == gsn_backstab && wield != NULL) 
	{    	
		if ( wield->value[0] != 2 )
			dam *= 2 + ch->level / 14; 
		else 
			dam *= 2 + ch->level / 12;
		/*end if*/
	}
	/*end if*/
	/*end spellsong fix*/

	/*no circle stuff for second wield or critical - spellsong*/

	/*second wield is sucky compared to first - spellsong*/
	if(dam != 0)
	{
		if (ch->class != 2)
			dam = ( dam * 2 ) / 4; /*reduce 50%*/
		else
			dam = ( dam * 4 ) / 6; /*reduce by less for thieves*/
	}
    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( dam <= 0 )
		dam = 1;

	/*ok I am thinking a magical weapon meant to cut off heads should be impossible to
	  counter when working its magic...just a thought Spellsong*/
	if(wield != NULL && IS_WEAPON_STAT(wield,WEAPON_VORPAL))
	{
		if (vorpal_kill( ch, victim, dam, dam_type ) == TRUE )
		{	
			damage( ch, victim, VORPAL_DAM, dt, dam_type);
			tail_chain( );
			return;
		}
	}
	
	/*damage( ch, victim, dam, dt, dam_type ) - spellsong*/
    	
	/*counter left out from here purposefully*/
	result = damage( ch, victim, dam, dt, dam_type);

	/* but do we have a funky weapon? */
	/*spellsong hack from rom 2.4*/
	if (result && wield != NULL)
	{ 
		int dam;

		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
		{
			int level;
			AFFECT_DATA *poison, af;

			if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
				level = wield->level;
			else
				level = poison->level;
		
			if (!saves_spell(level / 2,victim)) 
			{
				send_to_char("You feel poison coursing through your veins.",
					victim);
				act("$n is poisoned by the venom on $p.",
					victim,wield,NULL,TO_ROOM_FIGHT);
    			act("$n is poisoned by the venom on $p.",
						victim,wield,NULL,TO_CHAR_FIGHT);

    				af.type      = gsn_poison;
    				af.level     = level * 3/4;
    				af.duration  = level / 2;
    				af.location  = APPLY_STR;
    				af.modifier  = -1;
    				af.bitvector = AFF_POISON;
    				affect_join( victim, &af );
			}

			/* weaken the poison if it's temporary */
			if (poison != NULL)
			{
	    		poison->level = UMAX(0,poison->level - 2);
	    		poison->duration = UMAX(0,poison->duration - 1);
		
	    		if (poison->level == 0 || poison->duration == 0)
				act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR_FIGHT);
			}
 		}


    	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
		{
			dam = number_range(1, wield->level / 5 + 1);
			act("$p draws life from $n.",victim,wield,NULL,TO_ROOM_FIGHT);
			act("You feel $p drawing your life away.",
			victim,wield,NULL,TO_CHAR_FIGHT);
			damage(ch,victim,dam,1033,DAM_NEGATIVE);
			ch->alignment = UMAX(-1000,ch->alignment - 1);
			ch->hit += dam/2;
		}

		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
		{
			dam = number_range(1,wield->level / 4 + 1);
			act("$n is burned by $p.",victim,wield,NULL,TO_ROOM_FIGHT);
			act("$p sears your flesh.",victim,wield,NULL,TO_CHAR_FIGHT);
			fire_effect( victim,wield->level/2,dam,TARGET_CHAR);
			damage(ch,victim,dam,1034,DAM_FIRE);
		}

		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
		{
			dam = number_range(1,wield->level / 6 + 2);
			act("$p freezes $n.",victim,wield,NULL,TO_ROOM_FIGHT);
			act("The cold touch of $p surrounds you with ice.",
			victim,wield,NULL,TO_CHAR_FIGHT);
			cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
			damage(ch,victim,dam,1035,DAM_COLD);
		}

		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
		{
			dam = number_range(1,wield->level/5 + 2);
			act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM_FIGHT);
			act("You are shocked by $p.",victim,wield,NULL,TO_CHAR_FIGHT);
			shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
			damage(ch,victim,dam,1036,DAM_LIGHTNING);
		}

		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_ACID))
		{
			dam = number_range(1,wield->level/5 + 2);
			act("$n is burned by $p.",victim,wield,NULL,TO_ROOM_FIGHT);
			act("$p eats your flesh.",victim,wield,NULL,TO_CHAR_FIGHT);
			acid_effect(victim,wield->level/2,dam,TARGET_CHAR);
			damage(ch,victim,dam,1037,DAM_ACID);
		}

		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SERRATED))
			{
				int level;
				AFFECT_DATA  af;
				
				dam = number_range(1,wield->level/5 + 2);
				act("$n is `Rbled`w by $p.",victim,wield,NULL,TO_ROOM_FIGHT);
				act("$p makes you `Rbleed`w.",victim,wield,NULL,TO_CHAR_FIGHT);
				/*serrated_effect(victim,wield->level/2,dam,TARGET_CHAR);*/
				damage(ch,victim,dam,1038,DAM_SLASH);
				
				level = wield->level;
			
				if (!saves_spell(level / 2,victim)) 
				{
					send_to_char("You feel weak from blood loss.",
						victim);
					act("$n has a `RBLEEDING`w wound from $p.",
						victim,wield,NULL,TO_ROOM_FIGHT);
    				act("$n has a `RBLEEDING`w wound from $p.",
						victim,wield,NULL,TO_CHAR_FIGHT);

    					af.type      = skill_lookup( "weaken" );
    					af.level     = level * 3/4;
    					af.duration  = level / 2;
    					af.location  = APPLY_STR;
    					af.modifier  = -1;
    					af.bitvector = AFF_WEAKEN;
    					affect_join( victim, &af );
				}
 				
			}
	}/*end spelly hack*/
		
        /* the damage is done, lets fix the sharp stuff */
        adjust_sharpness_affects(ch,wield);
	tail_chain( );
	return;
}

/*
 * Inflict damage from a hit.
 */
bool damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type )
{
    char buf[200];
	OBJ_DATA *corpse;
    bool immune;
    extern bool chaos;
    int chaos_points;

	buf[0] = '\0';

	immune = FALSE;
    
	if ( ch == NULL || victim == NULL )
		return FALSE;
	
	if ( victim->position == POS_DEAD )
		return FALSE;
	
	/*ok dt passed is 0?*/

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 100000 )
    {
		bug( "Damage: %d: more than 100000 points!", dam );
		dam = 100000;
		if (!IS_IMMORTAL(ch))
		{
			OBJ_DATA *obj;
			obj = get_eq_char( ch, WEAR_WIELD );
			send_to_char("You really shouldn't cheat.\n\r",ch);
			if (obj != NULL) /*check added - spellsong*/
	    		extract_obj(obj);
		}

    }

	if(dt>=0 && dt<MAX_SKILL) /*could be spell*/
	{
		if(skill_table[dt].component)
		{
		if(!(lacks_components(ch,dt,'e',TRUE)))
		{
			send_to_char("You spell becomes stronger!\n\r",ch);
			dam=(dam*5)/3;
		}
		/*else
			lacks_components(ch,dt,'e',TRUE);*/
		}
	}

    if ( dt < VAMP || dt > SERRATION) /*the others just defined for consistency*/
	{							  /*remember to change SERRATION if others added after*/
		/* damage reduction */
		/*if ( dam > 20)
			dam = (dam - 20)/2 + 20;
		if ( dam > 65)
			dam = (dam - 65)/2 + 65;
   		*/
		if ( victim != ch )
		{
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */
		if ( is_safe( ch, victim ) )
			return FALSE;
		
		check_killer( ch, victim );

		if ( victim->position > POS_STUNNED )
		{
			if ( victim->fighting == NULL )
				set_fighting( victim, ch );
			if (victim->timer <= 4)
	    		victim->position = POS_FIGHTING;
		}

		if ( victim->position > POS_STUNNED )
		{
			if ( ch->fighting == NULL )
				set_fighting( ch, victim );

			/*
			 * If victim is charmed, ch might attack victim's master.
			 */
			if ( IS_NPC(ch)
			&&   IS_NPC(victim)
			&&   IS_AFFECTED(victim, AFF_CHARM)
			&&   victim->master != NULL
			&&   victim->master->in_room == ch->in_room
			&&   number_bits( 3 ) == 0 )
			{
				if (!IS_NPC(victim)) victim->pcdata->nemesis=victim->fighting;	
					stop_fighting( ch, FALSE );
				multi_hit( ch, victim->master, TYPE_UNDEFINED );
				return FALSE;
			}
		}

		/*
		 * More charm stuff.
		 */
		if ( victim->master == ch )
			stop_follower( victim );
		}

		/*
		 * Inviso attacks ... not.
		 */
		if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
		{
		affect_strip( ch, gsn_invis );
		affect_strip( ch, gsn_mass_invis );
		REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
		act( "`K$n fades into existence.`w", ch, NULL, NULL, TO_ROOM_FIGHT );
		}

		if ( IS_AFFECTED(ch, AFF_HIDE)
			|| IS_AFFECTED(ch, AFF_SNEAK))
		{
			affect_strip(ch, gsn_sneak);
			affect_strip(ch, gsn_hide);
			REMOVE_BIT(ch->affected_by, AFF_HIDE);
			REMOVE_BIT(ch->affected_by, AFF_SNEAK);
		}

		/*
		 * Damage modifiers. - but not for vorpal (already checked and is magical effect)
		 */

		if ( dam != VORPAL_DAM )
		{
			/*drunk check added by spellsong*/
			if ( dam > 1 && !IS_NPC(ch) 
			&&   ch->pcdata->condition[COND_DRUNK]  > 10 )
				dam = 6 * dam / 10;

			/*dam > 1 checks added by spellsong*/
			if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
				dam /= 2;
			
			/* Eris new spell add 30 April 2000 */
			if ( dam > 1 && IS_AFFECTED(victim, AFF_PRESERVATION) )
				dam -= dam / 3.5;

			if ( dam > 1 && 
                         IS_AFFECTED(victim,AFF_PROTECT_EVIL) && IS_EVIL(ch) )
				dam -= dam / 4;

                        if ( dam > 1 &&
			   IS_AFFECTED(victim,AFF_PROTECT_GOOD) && IS_GOOD(ch))
				dam -= dam/4;

			/* Eris ice_shield and fire_shield added code 2 July 2000 */
			if ( dam > 1 &&
 			  dam_type == DAM_COLD &&
			    IS_AFFECTED(victim, skill_lookup("ice shield")))
			      	dam /= 2; 

			if ( dam > 1 &&
                          dam_type == DAM_FIRE &&
                            IS_AFFECTED(victim, skill_lookup("fire shield")))
                                dam /= 2;

			immune = FALSE;


			/*
			 * Check for parry, and dodge.
			 */
			if ( dt >= TYPE_HIT && ch != victim)
			{
				if ( check_parry( ch, victim ) )
					return FALSE;
				if ( check_dodge( ch, victim ) )
					return FALSE;
				/*Check for phase - Spellsong*/
				if ( check_phase( ch, victim) )
					return FALSE;
				/*shield block bug fix -spellsong*/
				if ( check_block( ch, victim ) )
					return FALSE;
			}
		}
	}		

    /*vorpal check*/
	if ( dam != VORPAL_DAM )
	{
		switch(check_immune(victim,dam_type))
		{
		case(IS_IMMUNE):
			immune = TRUE;
			dam = 0;
			break;
		case(IS_RESISTANT):	
			dam -= dam/3;
			break;
		case(IS_VULNERABLE):
			dam += dam/2;
			break;
		}

		dam_message( ch, victim, dam, dt, immune );

		if (dam == 0)
			return FALSE;

		/*
		 * Hurt the victim.
		 * Inform the victim of his new state.
		 */
		victim->hit -= dam;
		if ( !IS_NPC(victim)
		&&   victim->level >= LEVEL_IMMORTAL
		&&   victim->hit < 1 )
		victim->hit = 1;
		update_pos( victim );

		switch( victim->position )
		{
		case POS_MORTAL:
		act( "`R$n is mortally wounded, and will die soon, if not aided.`w",
			victim, NULL, NULL, TO_ROOM_FIGHT );
		send_to_char( 
			"`RYou are mortally wounded, and will die soon, if not aided.\n\r`w",
			victim );
		break;

		case POS_INCAP:
		act( "`R$n is incapacitated and will slowly die, if not aided.`w",
			victim, NULL, NULL, TO_ROOM_FIGHT );
		send_to_char(
			"`RYou are incapacitated and will slowly die, if not aided.\n\r`w",
			victim );
		break;

		case POS_STUNNED:
		act( "`R$n is stunned, but will probably recover.`w",
			victim, NULL, NULL, TO_ROOM_FIGHT);
		send_to_char("`RYou are stunned, but will probably recover.\n\r`w",
			victim );
		break;

		case POS_DEAD:
		mprog_death_trigger( victim );
		act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM_FIGHT );
		send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
		break;

		default:
		if ( dam > victim->max_hit / 4 )
			send_to_char( "`RThat really did HURT`R!\n\r`w", victim );
		if ( victim->hit < victim->max_hit / 4 )
			send_to_char( "`RYou sure are BLEEDING`R!\n\r`w", victim );
		break;
		}

		/*
		 * Sleep spells and extremely wounded folks.
		 */
		if ( !IS_AWAKE(victim) ) 
		{
			if (!IS_NPC(victim)) victim->pcdata->nemesis=victim->fighting;	
				stop_fighting( victim, FALSE );
		}
	
	}

	if ( dam == VORPAL_DAM )
	{
		char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;
        int parts_buf;
		
		dam = victim->max_hit;
		dam_message( ch, victim, dam, dt, FALSE );
		
		victim->hit = -20;
		update_pos( victim );

        name            = IS_NPC(victim) ? victim->short_descr : victim->name;
        obj             = create_object( get_obj_index( OBJ_VNUM_SEVERED_HEAD ), 0 );
        
        sprintf( buf, obj->short_descr, name );
        free_string( obj->short_descr );
        obj->short_descr = str_dup( buf );

        sprintf( buf, obj->description, name );
        free_string( obj->description );
        obj->description = str_dup( buf );

        if (obj->item_type == ITEM_FOOD)
        {
            if (IS_SET(victim->form,FORM_POISON))
                obj->value[3] = 1;
            else if (!IS_SET(victim->form,FORM_EDIBLE))
                obj->item_type = ITEM_TRASH;
        }

        obj_to_room( obj, ch->in_room );
        
        /* parts_buf is used to remove all parts of the body so the death_cry 
           function does not create any other body parts */

        parts_buf = victim->parts;
        victim->parts = 0;
        
        victim->parts = parts_buf;
        
		act( "`R$n has been DECAPITATED!!`w", victim, 0, 0, TO_ROOM_FIGHT );
		send_to_char( "`RYour head flies of your shoulders!! You are dead!\n\r\n\r`w", victim );
	
		mprog_death_trigger( victim );		

	}

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
		if ( dam != VORPAL_DAM )
			group_gain( ch, victim, FALSE );
		else
			group_gain( ch, victim, TRUE );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level. ****Changed to 50%-25% by spellsong****
	     */
	    if ( !chaos && victim->exp > 0 && ( (victim->pcdata->nemesis == NULL) || 
	    	IS_NPC(victim->pcdata->nemesis) ) )
		/*gain_exp( victim, -1*(victim->exp/2) ); -OLD STUFF (SS)-*/
			
		/* Ok I didn't like the old system so here is the deal. Experience lost can be
		 * extremely time consuming thing on an already difficult mud. Realms of Dicordia
		 * had been made to be a challenge, so there will be many many player deaths.
		 * Plus this just makes us that much more original.      -Spellsong */
		{
			if (victim->level < 10)
				gain_exp( victim, -1*(victim->exp/2) );/*losing 50%xp is a bitch when you need*/
			else if (victim->level < 30)               /*90million - so keep it for low levels*/
				gain_exp( victim, -1*(victim->exp/3) );
			else if (victim->level < 60)
				gain_exp( victim, -1*(victim->exp/3.5) ); /*lets give these higher levels their*/
			else                            			  /*just reward for accomplishment*/
				gain_exp( victim, -1*(victim->exp/4) );
			/*end if*/
		}
	}

	if ( chaos )
	{
	  chaos_points = 0;

	  if (ch->level < victim->level)
		chaos_points = 2*(victim->level - ch->level);
	
	  chaos_points = chaos_points + victim->level;
	  ch->pcdata->chaos_score = chaos_points;
	}

	    if (!IS_NPC(victim)) {
            sprintf(buf, "%s has been slain by %s!", victim->name,
            IS_NPC(ch) ? ch->short_descr : ch->name);
            do_sendinfo(ch, buf);
        }

        sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);

        if (IS_NPC(victim))
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
        else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

	if (chaos)
	chaos_kill( victim );

        if (!chaos)
	raw_kill( victim );

        /* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "gold corpse");
            
	    /*buggy code - spellsong (poorley written)*/
		/*if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
				return TRUE;*/  /* leave if corpse has treasure */
	    /*else
			do_sacrifice( ch, "corpse" );*/

		/*spellsong fix*/
		if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	{  
			if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
				return TRUE;  /* leave if corpse has treasure */
	    
			else
				do_sacrifice( ch, "corpse" );
			/*end if*/
		}		
		/*else
			do_sacrifice( ch, "corpse" );*/ /*oops! my own bug fix here - SS */
		/*end if*/
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	{
		do_flee( victim, "" );
    }
	}

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	{
		do_flee( victim, "" );
	}
    tail_chain( );
    return TRUE;
}

bool new_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type, bool show ) 
{
    char buf[200];
    OBJ_DATA *corpse;
    bool immune;
    extern bool chaos;
    int chaos_points;

	buf[0] = '\0';
	
	immune = FALSE;

    if (ch == NULL || victim == NULL )
		return FALSE;

	if ( victim->position == POS_DEAD )
		return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 100000 )
    {
	bug( "Damage: %d: more than 100000 points!", dam );
	dam = 100000;
	if (!IS_IMMORTAL(ch))
	{
	    OBJ_DATA *obj;
	    obj = get_eq_char( ch, WEAR_WIELD );
	    send_to_char("You really shouldn't cheat.\n\r",ch);
	    if (obj != NULL) /*check added - spellsong*/
	    	extract_obj(obj);
	}

    }

	if ( dt < VAMP || dt > SERRATION) /*the others just defined for consistency*/
	{							  /*remember to change SERRATION if others added after*/
    
		/* damage reduction */
		if ( dam > 20)
			dam = (dam - 20)/2 + 20;
		if ( dam > 65)
			dam = (dam - 65)/2 + 65;
   
		if ( victim != ch )
		{
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */
		if ( is_safe( ch, victim ) )
			return FALSE;
		check_killer( ch, victim );

		if ( victim->position > POS_STUNNED )
		{
			if ( victim->fighting == NULL )
			set_fighting( victim, ch );
			if (victim->timer <= 4)
	    		victim->position = POS_FIGHTING;
		}

		if ( victim->position > POS_STUNNED )
		{
			if ( ch->fighting == NULL )
			set_fighting( ch, victim );

			/*
			 * If victim is charmed, ch might attack victim's master.
			 */
			if ( IS_NPC(ch)
			&&   IS_NPC(victim)
			&&   IS_AFFECTED(victim, AFF_CHARM)
			&&   victim->master != NULL
			&&   victim->master->in_room == ch->in_room
			&&   number_bits( 3 ) == 0 )
			{
					if (!IS_NPC(victim)) victim->pcdata->nemesis=victim->fighting;	
			stop_fighting( ch, FALSE );
			multi_hit( ch, victim->master, TYPE_UNDEFINED );
			return FALSE;
			}
		}

		/*
		 * More charm stuff.
		 */
		if ( victim->master == ch )
			stop_follower( victim );
		}

		/*
		 * Inviso attacks ... not.
		 */
		if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
		{
		affect_strip( ch, gsn_invis );
		affect_strip( ch, gsn_mass_invis );
		REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
		act( "`K$n fades into existence.`w", ch, NULL, NULL, TO_ROOM_FIGHT );
		}

		/*
		 * Damage modifiers. - but not for vorpal (already checked and is magical effect)
		 */

		if ( dam != VORPAL_DAM )
		{
			/*drunk check added by spellsong*/
			if ( dam > 1 && !IS_NPC(ch) 
			&&   ch->pcdata->condition[COND_DRUNK]  > 10 )
				dam = 6 * dam / 10;

			/*dam > 1 checks added by spellsong*/
			if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
				dam /= 2;
			
			/* Eris new spell add 30 April 2000 */
			if ( dam > 1 && IS_AFFECTED(victim, AFF_PRESERVATION) )
				dam -= dam / 3.5;

			if ( dam > 1 &&
			IS_AFFECTED(victim,AFF_PROTECT_EVIL) && IS_EVIL(ch) )
				dam -= dam / 4;

			if ( dam > 1 &&
			   IS_AFFECTED(victim,AFF_PROTECT_GOOD) && IS_GOOD(ch))
				dam -= dam /4;

			/* Eris ice_shield and fire_shield added code 2 July 2000 */
                        if ( dam > 1 &&
                          dam_type == DAM_COLD &&
                            IS_AFFECTED(victim, skill_lookup("ice shield")))
                                dam /= 2;

                        if ( dam > 1 &&
                          dam_type == DAM_FIRE &&
                            IS_AFFECTED(victim, skill_lookup("fire shield")))
                                dam /= 2;

			immune = FALSE;


			/*
			 * Check for parry, and dodge.
			 */
			if ( dt >= TYPE_HIT && ch != victim)
			{
				if ( check_parry( ch, victim ) )
					return FALSE;
				if ( check_dodge( ch, victim ) )
					return FALSE;
				/*check for phase - Spellsong*/
				if ( check_phase( ch, victim) )
					return FALSE;
				/*shield block bug fix -spellsong*/
				if ( check_block( ch, victim ) )
					return FALSE;
			}
		}
	}
	
	/*vorpal check*/
	if ( dam != VORPAL_DAM )
	{
		switch(check_immune(victim,dam_type))
		{
		case(IS_IMMUNE):
			immune = TRUE;
			dam = 0;
			break;
		case(IS_RESISTANT):	
			dam -= dam/3;
			break;
		case(IS_VULNERABLE):
			dam += dam/2;
			break;
		}

		if (show)
		dam_message( ch, victim, dam, dt, immune );

		if (dam == 0)
		return FALSE;

		/*
		 * Hurt the victim.
		 * Inform the victim of his new state.
		 */
		victim->hit -= dam;
		if ( !IS_NPC(victim)
		&&   victim->level >= LEVEL_IMMORTAL
		&&   victim->hit < 1 )
		victim->hit = 1;
		update_pos( victim );

		switch( victim->position )
		{
		case POS_MORTAL:
		act( "`R$n is mortally wounded, and will die soon, if not aided.`w",
			victim, NULL, NULL, TO_ROOM_FIGHT );
		send_to_char( 
			"`RYou are mortally wounded, and will die soon, if not aided.\n\r`w",
			victim );
		break;

		case POS_INCAP:
		act( "`R$n is incapacitated and will slowly die, if not aided.`w",
			victim, NULL, NULL, TO_ROOM_FIGHT );
		send_to_char(
			"`RYou are incapacitated and will slowly die, if not aided.\n\r`w",
			victim );
		break;

		case POS_STUNNED:
		act( "`R$n is stunned, but will probably recover.`w",
			victim, NULL, NULL, TO_ROOM_FIGHT );
		send_to_char("`RYou are stunned, but will probably recover.\n\r`w",
			victim );
		break;

		case POS_DEAD:
		mprog_death_trigger( victim );
		act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM_FIGHT );
		send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
		break;

		default:
		if ( dam > victim->max_hit / 4 )
			send_to_char( "`RThat really did HURT`R!\n\r`w", victim );
		if ( victim->hit < victim->max_hit / 4 )
			send_to_char( "`RYou sure are BLEEDING`R!\n\r`w", victim );
		break;
		}

		/*
		 * Sleep spells and extremely wounded folks.
		 */
		if ( !IS_AWAKE(victim) ) 
		{
			if (!IS_NPC(victim)) victim->pcdata->nemesis=victim->fighting;	
		stop_fighting( victim, FALSE );
		}
	}

	if ( dam == VORPAL_DAM )
	{
		char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;
        int parts_buf;
		
		dam = victim->max_hit;
		dam_message( ch, victim, dam, dt, FALSE );
		
		victim->hit = -20;
		update_pos( victim );

        name            = IS_NPC(victim) ? victim->short_descr : victim->name;
        obj             = create_object( get_obj_index( OBJ_VNUM_SEVERED_HEAD ), 0 );
        
        sprintf( buf, obj->short_descr, name );
        free_string( obj->short_descr );
        obj->short_descr = str_dup( buf );

        sprintf( buf, obj->description, name );
        free_string( obj->description );
        obj->description = str_dup( buf );

        if (obj->item_type == ITEM_FOOD)
        {
            if (IS_SET(victim->form,FORM_POISON))
                obj->value[3] = 1;
            else if (!IS_SET(victim->form,FORM_EDIBLE))
                obj->item_type = ITEM_TRASH;
        }

        obj_to_room( obj, ch->in_room );
        
        /* parts_buf is used to remove all parts of the body so the death_cry 
           function does not create any other body parts */

        parts_buf = victim->parts;
        victim->parts = 0;
        
        victim->parts = parts_buf;
        
		act( "`R$n has been DECAPITATED!!`w", victim, 0, 0, TO_ROOM_FIGHT );
		send_to_char( "`RYour head flies of your shoulders!! You are dead!\n\r\n\r`w", victim );
	
		mprog_death_trigger( victim );		

	}

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
		if ( dam != VORPAL_DAM )
			group_gain( ch, victim, FALSE );
		else
			group_gain( ch, victim, TRUE );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( !chaos && victim->exp > 0 && ( (victim->pcdata->nemesis == NULL) || 
	    	IS_NPC(victim->pcdata->nemesis) ) )
		gain_exp( victim, -1*(victim->exp/2) );
	}
	if ( chaos )
	{
	  chaos_points = 0;

	  if (ch->level < victim->level)
		chaos_points = 2*(victim->level - ch->level);
	  /*end if*/
	  chaos_points = chaos_points + victim->level;
	  ch->pcdata->chaos_score = chaos_points;
	}
	/*end if*/
	    if (!IS_NPC(victim)) 
		{
            sprintf(buf, "%s has been slain by %s!", victim->name,
            IS_NPC(ch) ? ch->short_descr : ch->name);
            do_sendinfo(ch, buf);
        }
		/*end if*/

	if (chaos)
	chaos_kill( victim );

        if (!chaos)
	raw_kill( victim );

        /* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "gold corpse");
            
	    /*Buggy code here - spellsong*/
		/*if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
				return TRUE;*/  /* leave if corpse has treasure */
	      /*else
		do_sacrifice( ch, "corpse" );*/

		/*Spellsong fix*/
		if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	{  
			if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
				return TRUE;  /* leave if corpse has treasure */
	    
			else
				do_sacrifice( ch, "corpse" );
			/*end if*/
		}		
		/*else
			do_sacrifice( ch, "corpse" ); */ /*oops! my own bug fix here SS*/
		/*end if*/
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	{
		do_flee( victim, "" );
	}
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    tail_chain( );
    return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim )
{

/* no killing NPCs with ACT_NO_KILL */

	if (ch == NULL || victim == NULL )
		return TRUE;
	
	if (IS_NPC(victim) && IS_SET(victim->act,ACT_NO_KILL))
	{
	 send_to_char("I don't think the gods would approve.\n\r",ch);
	 return TRUE;
	}

    /* no fighting in safe rooms */
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    {
	send_to_char("Not in this room.\n\r",ch);
	return TRUE;
    }

    if (victim->fighting == ch)
	return FALSE;

    if (IS_NPC(ch))
    {
 	/* charmed mobs and pets cannot attack players */
	if (!IS_NPC(victim) && (IS_AFFECTED(ch,AFF_CHARM)
			    ||  IS_SET(ch->act,ACT_PET)))
	    return TRUE;

      	return FALSE;
     }

     else /* Not NPC */
     {	
	if (IS_IMMORTAL(ch))
	    return FALSE;

	/* no pets */
	if (IS_NPC(victim) && IS_SET(victim->act,ACT_PET))
	{
            act("But $N looks so cute and cuddly...",ch,NULL,victim,TO_CHAR_FIGHT);
            return TRUE;
	}

	/* no charmed mobs unless char is the the owner */
	if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	{
            send_to_char("You don't own that monster.\n\r",ch);
	    return TRUE;
	}

	return FALSE;
    }
	return FALSE; /*just in case something get through spellsong*/
}


/*old function - commented by spellsong 29March2000*/
/*bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    
    if (ch == victim)
		return TRUE;
		
    if (IS_IMMORTAL(victim) &&  area)
		return TRUE;
    
    if (IS_NPC(victim) && IS_SET(victim->act,ACT_NO_KILL))
		return TRUE;

    if (victim->fighting == ch || victim == ch)
		return FALSE;
		
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
        return TRUE;
 
    if (IS_NPC(ch))
    {
        if (!IS_NPC(victim)) && (IS_AFFECTED(ch,AFF_CHARM)
        ||  IS_SET(ch->act,ACT_PET)))
            return TRUE;
			
        if (IS_NPC(victim) && area)
            return TRUE;
 
        return FALSE;
    }
 
    else 
    {
        if (IS_IMMORTAL(ch) && !area)
            return FALSE;
 
        
        if (IS_NPC(victim) && IS_SET(victim->act,ACT_PET))
            return TRUE;
 
        
        if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
            return TRUE;
 
        
        if ( !IS_NPC(victim) )
    	{
          if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
          {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return TRUE;
          }
	}
	
	if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	    return TRUE;
 
        return FALSE;
    }
}*/

/*rom2.4 addin and rehack - spellsong*/

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    
	if (ch == NULL || victim == NULL ) /*just in case - SS*/
		return TRUE;
	
	if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area)
	return FALSE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {
		/* safe room? */
		if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
			return TRUE;

		if (victim->pIndexData->pShop != NULL)
			return TRUE;

		/* spellsong rehack for no_kill */
		if (IS_NPC(victim) && IS_SET(victim->act,ACT_NO_KILL))
			return TRUE;
		   

		if (!IS_NPC(ch))
		{
			/* no pets */
			if (IS_SET(victim->act,ACT_PET))
	   		return TRUE;

			/* no charmed creatures unless owner */
			if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
			return TRUE;

			/* legal kill? -- cannot hit mob fighting non-group member */
			if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
			return TRUE;
		}
		else
		{
			/* area effect spells do not hit other mobs */
			if (area && !is_same_group(victim,ch->fighting))
			return TRUE;
		}
    }
    /* killing players */
    else
    {
		if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
			return TRUE;

		/* NPC doing the killing */
		if (IS_NPC(ch))
		{
			/* charmed mobs and pets cannot attack players while owned */
			if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
			&&  ch->master->fighting != victim)
			return TRUE;
		
			/* safe room? */
			if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
			return TRUE;

			/* legal kill? -- mobs only hit players grouped with opponent*/
			if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
			return TRUE;
		}

		/* player doing the killing */
		else
		{
			if ( ch->pcdata->nokilling != 0 && !IS_NPC(ch))
        {
            send_to_char( "You have been banned from killing other players by the gods.\n\r", ch);
            return TRUE;
        }
			/*spellsong - rehack*/
			if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
			{
				send_to_char( "You can only kill other player killers.\n\r", ch );
				return TRUE;
			}
			
			/*put religion check here perhaps*/

			/*put level check as below if we dont want level 100's picking on
			 *level 10 killers
			 */

			/*if (ch->level > victim->level + 8)
				return TRUE;*/
		}

    }
    return FALSE;
}

/*
 * See if an attack justifies a KILLER flag.
 */ 
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];

    if( ch == NULL || victim == NULL )
		return; /*just in case spellsong*/
	
	/*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
		victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||   IS_SET(victim->act, PLR_KILLER)
    ||   IS_SET(victim->act, PLR_THIEF) )
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}
/*
	send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
  	SET_BIT(ch->master->act, PLR_KILLER);
*/
	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL
    ||   IS_SET(ch->act, PLR_KILLER) )
	return;

    send_to_char( "*** You are now a KILLER!! ***\n\r", ch );
    SET_BIT(ch->act, PLR_KILLER);
    sprintf(buf,"$N is attempting to murder %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
    save_char_obj( ch );
    return;
}



/*
 * Check for parry.
 */

/*old - crappily done - Spellsong*/
/*bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
    {
	chance	= UMIN( 30, victim->level );
    }
    else
    {
	if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
	    return FALSE;
	chance	= victim->pcdata->learned[gsn_parry] / 2;
    }

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "`BYou parry $n's attack.`w",  ch, NULL, victim, TO_VICT    );
    act( "`B$N parries your attack.`w", ch, NULL, victim, TO_CHAR_FIGHT    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}*/

/*replaced with rom2.4 with a hack - spellsong*/
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;
	
	/*hack - SS*/
	if(IS_NPC(victim))
		chance = UMIN( 30, victim->level );
	else
		chance = get_skill(ch,gsn_parry) / 2;
	/*end hack*/

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
		if (IS_NPC(victim))
			chance /= 2;
		else
			return FALSE;
    }

    if (!can_see(ch,victim))
		chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
		return FALSE;

    act( "`BYou parry $n's attack.`w",  ch, NULL, victim, TO_VICT    );
    act( "`B$N parries your attack.`w", ch, NULL, victim, TO_CHAR_FIGHT    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( MOUNTED(victim) )
       return FALSE;

    if ( IS_NPC(victim) )
        chance  = UMIN( 30, victim->level );
    else
        chance  = victim->pcdata->learned[gsn_dodge] / 2;

    /*spellsong hack - duh*/
	if (!can_see(victim,ch))
		chance /= 2;
	
	if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "`BYou dodge $n's attack.`w", ch, NULL, victim, TO_VICT    );
    act( "`B$N dodges your attack.`w", ch, NULL, victim, TO_CHAR_FIGHT    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}

/*Spellsong Bug Fix for shield block - i.e. it does something now*/
/*Thanks to Ron Cole for finding this bug*/
bool check_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chancea;
    int chance;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    if ( IS_NPC(victim) )
    {
        chance = UMIN( 30, victim->level );
    }
    else
    {
        chance = victim->pcdata->learned[gsn_shield_block] / 4;
    }
    
    /* Must get a successful check before a block can be attempted */ 
    if ( !IS_NPC(victim) )
    {
       if ( number_percent( ) > victim->pcdata->learned[gsn_shield_block])
          return FALSE;
    }

    chancea = 0;
    
	/*blocker cant see the attacker*/
    if (!can_see(victim,ch))    
    chance  -= 25; 

	/*attacker cant see the blocker*/
    if (!can_see(ch,victim))    
    chancea  += 25; 

    chance  += get_curr_stat(victim,STAT_DEX)/4;
    chancea += (get_curr_stat(ch,STAT_DEX)/4) + ((ch->level)/2)+(get_curr_stat(ch,STAT_WIS)/3);


/* A high chance is good.  A low chance means a failed parry */
    if ( number_percent( ) >= chance + ((victim->level)/2) - chancea )
        return FALSE;

    act( "`BYou block $n's attack.`w",  ch, NULL, victim, TO_VICT    );
    act( "`B$N blocks your attack.`w", ch, NULL, victim, TO_CHAR_FIGHT    );

    if ( ((victim->level) - 5)>(ch->level) )
    return TRUE;   

    check_improve(victim,gsn_shield_block,TRUE,6);
    return TRUE;
}
/*end spellsong bug fix*/


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
		victim->position = POS_DEAD;
		return;
    }

    if ( victim->hit <= -11 )
    {
		victim->position = POS_DEAD;
		return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? ch->default_pos : POS_STANDING;
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

	buf[0] = '\0'; /*initialize buffer spellsong*/

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold ), corpse );
	    ch->gold = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 100, 160 ); /* used to be 25, 40 */
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	if (!IS_SET(ch->act,PLR_KILLER) && !IS_SET(ch->act,PLR_THIEF))
	    corpse->owner = str_dup(ch->name);
	else
	    corpse->owner = NULL;
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    if ( IS_NPC(ch) || (ch->pcdata->nemesis == NULL) 
    	|| (!IS_NPC(ch) && IS_NPC(ch->pcdata->nemesis)) ) 
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
		bool floating = FALSE; /*spellsong hack*/

		obj_next = obj->next_content;

		if (obj->wear_loc == WEAR_FLOATING)
			floating = TRUE;

		obj_from_char( obj );
		
		if (obj->item_type == ITEM_POTION)
			obj->timer = number_range(500,1000);
		
		if (obj->item_type == ITEM_SCROLL)
			obj->timer = number_range(1000,2500);
		
		/*spell rewrite*/
		if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
		{
			obj->timer = number_range(5,10);
			REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
		}

		REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
		
		/*spellsong rewrite*/
		if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
		{
			
			extract_obj( obj );
			
		}
		else
			if(!floating)
				obj_to_obj( obj, corpse );
			else
			{
				act("$p gently floats to the floor.",ch,obj,NULL,TO_ROOM_FIGHT);
				obj_to_room(obj,ch->in_room);
			}
    }
			
    obj_to_room( corpse, ch->in_room );
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "`YYou hear $n's death cry.`w";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "`R$n splatters blood on your armor.`w";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "`R$n spills $s guts all over the floor.`w";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "`R$n's severed head plops on the ground.`w";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "`R$n's heart is torn from $s chest.`w";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "`R$n's arm is sliced from $s dead body.`w";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "`R$n's leg is sliced from $s dead body.`w";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "`R$n's head is shattered, and $s brains splash all over you.`w";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM_FIGHT );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "`RYou hear something's death cry.`w";
    else
	msg = "`RYou hear someone's death cry.`w";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM_FIGHT );
	}
    }
    ch->in_room = was_in_room;

    return;
}

void chaos_kill( CHAR_DATA *victim)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    
	buf[0] = '\0'; /*spellsong initialize buffer*/
	
	stop_fighting( victim, TRUE );
	for ( obj = victim->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            obj_from_char( obj );
            obj_to_room( obj, victim->in_room );
        }
     	act( "`B$n's corpse is sucked into the ground!!`w", victim, 0, 0, TO_ROOM_FIGHT );
	if ( !IS_NPC(victim) )
	{	
 	   sprintf(buf, "was slain with %d chaos points.", victim->pcdata->chaos_score);
    	   chaos_log( victim, buf);
	}
    d = victim->desc;
    extract_char( victim, TRUE );
    if ( d != NULL )
        close_socket( d );
  return;
}


void raw_kill( CHAR_DATA *victim )
{
    int i;

	stop_fighting( victim, TRUE );
    /*death_cry( victim );*/
	make_corpse( victim );
	

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= 0;
    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    /* RT added to prevent infinite deaths */
    REMOVE_BIT(victim->act, PLR_THIEF);
    REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
/*  save_char_obj( victim ); */
/* Add back race affects */
    victim->affected_by = victim->affected_by|race_table[victim->race].aff;
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim, bool vorpal )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;
	sh_int pets=0;
	
	buf[0] = '\0';
    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( !IS_NPC(victim) || victim == ch )
	return;
    
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += gch->level;
		if(IS_NPC(gch))
			pets++;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

	if ( gch->level - lch->level >= 7 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -7 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}

	xp = xp_compute( gch, victim, group_levels, members, vorpal );  
        if(pets==0) /*Akira's doing. Change as needed */
        {
                if(members==2)
                        xp*=1.5;
                if(members==3)
                        xp*=2.5;
        }
	if (xp<0)
	{
		sprintf( buf, "`WYou are morally shaken by your deeds. You LOSE %d experience points.\n\r`w", xp );
	}
	else
	{
		sprintf( buf, "`WYou receive %d experience points.\n\r`w", xp );
	}
	send_to_char( buf, gch );
	gain_exp( gch, xp );

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "`WYou are zapped by $p.`w", ch, obj, NULL, TO_CHAR_FIGHT );
		act( "`W$n is zapped by $p.`w",   ch, obj, NULL, TO_ROOM_FIGHT );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
	    }
	}
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels, int members, bool vorpal )
{
    const int alignMod = 20;
	int xp,base_exp=0;
    int align;
    int change;

    /* compute the base exp */
    switch (victim->level)
    {
    	case 0  :	base_exp =   50;		break;
 	case 1  : 	base_exp =   100;		break;
	case 2  :	base_exp =   200;		break;
	case 3  :	base_exp =   250;		break;
	case 4  :	base_exp =   350;		break;
	case 5  : 	base_exp =   550;		break;
	case 6  :	base_exp =   1000;		break;
	case 7  :	base_exp =   3000;		break;
	case 8  :	base_exp =   5000;		break;
	case 9  :	base_exp =   7500;		break;
	case 10 :	base_exp =   10000;		break;
	case 11 :	base_exp =   15000;		break;
	case 12 :	base_exp =   23000;		break;
	case 13 :	base_exp =   35000;		break;
	case 14 :	base_exp =   50000;		break;
	case 15 :	base_exp =   65000;		break;
 	case 16 : 	base_exp =   80000;		break;
	case 17 :	base_exp =   95000;		break;
	case 18 :	base_exp =   110000;		break;
	case 19 :	base_exp =   135000;		break;
	case 20 : 	base_exp =   150000;		break;
	case 21 :	base_exp =   165000;		break;
	case 22 :	base_exp =   180000;		break;
	case 23 :	base_exp =   200000;		break;
	case 24 :	base_exp =   220000;		break;
	case 25 :	base_exp =   240000;		break;
	case 26 :	base_exp =   260000;		break;
	case 27 :	base_exp =   280000;		break;
	case 28 :	base_exp =   300000;		break;
	case 29 :	base_exp =   320000;		break;
	case 30 :	base_exp =   340000;		break;
 	case 31 : 	base_exp =   360000;		break;
	case 32 :	base_exp =   380000;		break;
	case 33 :	base_exp =   400000;		break;
	case 34 :	base_exp =   420000;		break;
	case 35 : 	base_exp =   440000;		break;
	case 36 :	base_exp =   460000;		break;
	case 37 :	base_exp =   480000;		break;
	case 38 :	base_exp =   500000;		break;
	case 39 :	base_exp =   520000;		break;
	case 40 :	base_exp =   540000;		break;
	case 41 :	base_exp =   560000;		break;
	case 42 :	base_exp =   580000;		break;
	case 43 :	base_exp =   600000;		break;
	case 44 :	base_exp =   620000;		break;
	case 45 :	base_exp =   640000;		break;
 	case 46 : 	base_exp =   660000;		break;
	case 47 :	base_exp =   680000;		break;
	case 48 :	base_exp =   700000;		break;
	case 49 :	base_exp =   720000;		break;
	case 50 : 	base_exp =   740000;		break;
	case 51 :	base_exp =   760000;		break;
	case 52 :	base_exp =   780000;		break;
	case 53 :	base_exp =   800000;		break;
	case 54 :	base_exp =   820000;		break;
	case 55 :	base_exp =   840000;		break;
	case 56 :	base_exp =   860000;		break;
	case 57 :	base_exp =   880000;		break;
	case 58 :	base_exp =   900000;		break;
	case 59 :	base_exp =   920000;		break;
	case 60 :	base_exp =   940000;		break;
	case 61 :	base_exp =   960000;		break;
	case 62 :	base_exp =   1000000;		break;
	case 63 :	base_exp =   1100000;		break;
	case 64 :	base_exp =   1200000;		break;
	case 65 :	base_exp =   1300000;		break;
	case 66 :	base_exp =   1400000;		break;
	case 67 :	base_exp =   1500000;		break;
 	case 68 : 	base_exp =   1600000;		break;
	case 69 :	base_exp =   1700000;		break;
	case 70 :	base_exp =   1800000;		break;
	case 71 :	base_exp =   1900000;		break;
	case 72 : 	base_exp =   2000000;		break;
	case 73 :	base_exp =   2100000;		break;
	case 74 :	base_exp =   2200000;		break;
	case 75 :	base_exp =   2300000;		break;
	case 76 :	base_exp =   2400000;		break;
	case 77 :	base_exp =   2500000;		break;
	case 78 :	base_exp =   2600000;		break;
	case 79 :	base_exp =   2700000;		break;
	case 80 :	base_exp =   2800000;		break;
	case 81 :	base_exp =   2900000;		break;
	case 82 :	base_exp =   3000000;		break;
	case 83 :	base_exp =   3100000;		break;
	case 84 :	base_exp =   3200000;		break;
	case 85 : 	base_exp =   3300000;		break;
	case 86 :	base_exp =   3400000;		break;
	case 87 :	base_exp =   3500000;		break;
	case 88 :	base_exp =   3600000;		break;
	case 89 :	base_exp =   3700000;		break;
	case 90 :	base_exp =   3800000;		break;
	case 91 :	base_exp =   4000000;		break;
	case 92 :	base_exp =   4500000;		break;
	case 93 :	base_exp =   5000000;		break;
	case 94 :	base_exp =   5500000;		break;
	case 95 :	base_exp =   6000000;		break;
	case 96 :	base_exp =   6500000;		break;
	case 97 :	base_exp =   7000000;		break;
	case 98 :	base_exp =   7500000;		break;
	case 99 :	base_exp =   8000000;		break;
	case 100 :	base_exp =   10000000;		break;
    } 
    
    /* do alignment computations */
   
    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

	/*ALIGNMENT SHIFT ENTIRELY TOO FAST - CONSTANT USED TO ADUJST FOR ALL GROUPS
	 *IF PLAYTESTING PROVES IT TO FAST OR SLOW JUST ADJUST THE CONSTANT AND
	 *OTHER NUMBERS. PAY ATTENTION THIS IS A TAD CONFUSING - SPELLSONG*/

    else if (align < -500) /* monster is more evil than slayer */
    {
		change = (align + 500) / alignMod; 
		change = 0 - change;
        
		if (gch->alignment > 750) /*make it hard to get extremely evil*/
			change = change / 2;

		gch->alignment = gch->alignment + change;
		
		if( gch->alignment > 1000 )
			gch->alignment = 1000;
    }

    else if (align > 500) /* monster is more good than slayer */
    {
		change = (align - 500) / alignMod;
		
		if (gch->alignment < -750) /*make it hard to get extremely evil*/
			change = change / 2;

		gch->alignment = gch->alignment - change;
		
		if( gch->alignment < -1000 )
			gch->alignment = -1000;
    }

    else 
    {
				
		if( gch->alignment > 500 && align > -500 ) /*really good killing good*/
		{	
			change = (align + 500) / (alignMod/4) + 20;
			gch->alignment = gch->alignment - change; 
		}

		if( gch->alignment < -500 && align < 500 ) /*really evil killing evil*/
		{	
			change = (align - 500) / (alignMod/4) - 20;
			gch->alignment = gch->alignment - change; 
		}

		if( gch->alignment <= 500 && gch->alignment >= -500 )
		{
			if( gch->alignment <= 150 && gch->alignment >= -150 && align <= 150 && align >= -150 )
			{
				change = align / (alignMod/4);
				change = 0 - change;
				gch->alignment = gch->alignment + change;
			}
			else
			{
				change = align / alignMod;
				change = 0 - change;
				gch->alignment = gch->alignment + change;
			}
		}
    }
    
    /* calculate exp multiplier also based
	   on align -fixed/unfucked/balanced
	   spellsong*/
    if (IS_SET(victim->act,ACT_NOALIGN))
		xp = base_exp;

	else if (gch->alignment > 500)  /* for goodie two shoes */
    {
		/*ordering fucked - fixed spellsong*/
		if (victim->alignment < -750)
			xp = (base_exp * 5)/3;
   
 		else if (victim->alignment < -500)
			xp = (base_exp * 3)/2;

		else if (victim->alignment > 750)
			xp = (-1 * base_exp) /2 ; 

   		else if (victim->alignment > 500)
			xp = (-1 * base_exp) / 3; 

		else if (victim->alignment > 300)
			xp = (-1 * base_exp) /6; 

		else
			xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
		if (victim->alignment > 750)
			xp = (base_exp * 12)/9;
		
  		else if (victim->alignment > 500)
			xp = (base_exp * 11)/9;

   		else if (victim->alignment < -750)
			xp = base_exp/4;

		else if (victim->alignment < -500)
			xp = base_exp /3; 

		else if (victim->alignment < -250)
			xp = base_exp /2; 

		else
			xp = (base_exp*8)/11;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

		if (victim->alignment < -500)
			xp = (base_exp * 7)/6;

 		else if (victim->alignment > 750)
			xp = (base_exp * 1)/2;

		else if (victim->alignment > 0)
			xp = (base_exp * 3)/4; 
		
		else
			xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
		if (victim->alignment > 500)
			xp = (base_exp * 7)/6;
 
		else if (victim->alignment < -750)
			xp = (base_exp * 1)/2;

		else if (victim->alignment < 0)
			xp = (base_exp * 3)/4;

		else
			xp = base_exp;
    }

    else /* neutral */
    {
/*
		if (victim->alignment > 500 || victim->alignment < -500)
			xp = (base_exp * 5)/4;
*/

		/* 
		 * Removed by Eris - neutral people still learn from killing animals
		 * Makes things hard for neutral players, doesn't really fit the world
		 * Note me before you change this back - would like to explain.
		else if (victim->alignment < 200 && victim->alignment > -200)
			xp = (base_exp * 1)/2;
             */
/*
 		else
*/
			xp = (base_exp*9)/10;
    }

    /* more exp at the low levels */
/*    if (gch->level < 6)
    	xp = 10 * xp / (gch->level + 4);*/

    /* less at high */
    /* CHANGED BY ERIS  18 March 2000 from 35 to 65 */
/*    if (gch->level > 65 )
	xp =  15 * xp / (gch->level - 20 );*/

    /* reduce for playing time */
/*    else
    {
	/ compute quarter-hours per level /
	time_per_level = 4 *
			 (gch->played + (int) (current_time - gch->logon))/3600
			 / gch->level;
	time_per_level = URANGE(2,time_per_level,12);
	if (gch->level < 15)  / make it a curve /
	    time_per_level = UMAX(time_per_level,(15 - gch->level));
	xp = xp * time_per_level / 12;
    }*/
   
    /* randomize the rewards - Randomization reduced by Eris - too much variance*/
    xp = number_range (xp * 7/8, xp * 9/8);

    /* adjust for grouping */
   /* if (members > 1 ) xp = xp * (((gch->level/total_levels)+(1/members))/2);
*/
	if (vorpal == TRUE)
		xp = xp/2; // Used to be (xp*4)/2 which makes no sense - should get less
      
	if (members > 1 ) xp = (gch->level * (xp / total_levels));
      
    return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

	buf1[0] = '\0';
	buf2[0] = '\0';
	buf3[0] = '\0'; /*buffer initialize*/

	 if ( dam ==   0 ) { vs = "miss";	vp = "misses";		}
    else if ( dam <=   4 ) { vs = "scratch";	vp = "scratches";	}
    else if ( dam <=   8 ) { vs = "graze";	vp = "grazes";		}
    else if ( dam <=  12 ) { vs = "hit";	vp = "hits";		}
    else if ( dam <=  16 ) { vs = "injure";	vp = "injures";		}
    else if ( dam <=  20 ) { vs = "wound";	vp = "wounds";		}
    else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";		}
    else if ( dam <=  28 ) { vs = "decimate";	vp = "decimates";	}
    else if ( dam <=  32 ) { vs = "devastate";	vp = "devastates";	}
    else if ( dam <=  36 ) { vs = "maim";	vp = "maims";		}
    else if ( dam <=  40 ) { vs = "`rMUTILATE`w";	vp = "`rMUTILATES`w";	}
    else if ( dam <=  44 ) { vs = "`rPULVERIZE`w";	vp = "`rPULVERIZES`w";	}
    else if ( dam <=  48 ) { vs = "`rDISMEMBER`w";	vp = "`rDISMEMBERS`w";	}
    else if ( dam <=  52 ) { vs = "`rMASSACRE`w";	vp = "`rMASSACRES`w";	}
    else if ( dam <=  56 ) { vs = "`rMANGLE`w";	vp = "`rMANGLES`w";		}
    else if ( dam <=  60 ) { vs = "`R*** DEMOLISH ***";
			     vp = "`R*** DEMOLISHES ***`w";			}
    else if ( dam <=  75 ) { vs = "`R*** DEVASTATE ***`w";
			     vp = "`R*** DEVASTATES ***`w";			}
    else if ( dam <= 100)  { vs = "`R=== OBLITERATE ===`w";
			     vp = "`R=== OBLITERATES ===`w";		}
    else if ( dam <= 125)  { vs = "`R>>> ANNIHILATE <<<`w";
			     vp = "`R>>> ANNIHILATES <<<`w";		}
    else if ( dam <= 150)  { vs = "`R<<< ERADICATE >>>`w";
			     vp = "`R<<< ERADICATES >>>`w";			}
    else if ( dam <= 165)  { vs = "`R<><><> BUTCHER <><><>`w";
    			     vp = "`R<><><> BUTCHERS <><><>`w";		}
    else if ( dam <= 185)  { vs = "`R<><><> DISEMBOWEL <><><>`w";
    			     vp = "`R<><><> DISEMBOWELS <><><>`w";		}
    else                   { vs = "do `RUNSPEAKABLE`w damage to";
			     vp = "does `RUNSPEAKABLE`w damage to";		}

    punct   = (dam <= 24) ? '.' : '!';

    /*below hack from 2.4 - spellsong*/
	if ( dt == TYPE_HIT )
    {
		if (ch  == victim)
		{
			sprintf( buf1, "$n %s $melf%c",vp,punct);
			sprintf( buf2, "You %s yourself%c",vs,punct);
		}
		else
		{
			sprintf( buf1, "$n %s $N%c",  vp, punct );
			sprintf( buf2, "You %s $N%c", vs, punct );
			sprintf( buf3, "$n %s you%c", vp, punct );
		}
    }
    else
    {
		if ( dt >= 0 && dt < MAX_SKILL ) /*maxskill-155*/
			attack	= skill_table[dt].noun_damage;
		else if ( dt >= TYPE_HIT
		&& dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE) /*maxdamMsg = 32*/ /*type_hit = 1000*/
			attack	= attack_table[dt - TYPE_HIT].name;
		else
		{
			bug( "Dam_message: bad dt %d.", dt );
			dt  = TYPE_HIT;
			attack  = attack_table[0].name;
		}

		if (immune)
		{
			if (ch == victim)
			{
				sprintf(buf1,"$n is unaffected by $s own %s.",attack);
				sprintf(buf2,"Luckily, you are immune to that.");
			} 
			else
			{
	    		sprintf(buf1,"$N is unaffected by $n's %s!",attack);
	    		sprintf(buf2,"$N is unaffected by your %s!",attack);
	    		sprintf(buf3,"$n's %s is powerless against you.",attack);
			}
		}
		else
		{
			if (ch == victim)
			{
				sprintf( buf1, "$n's %s %s $m%c",attack,vp,punct);
				sprintf( buf2, "Your %s %s you%c",attack,vp,punct);
			}
			else
			{
	    		sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
	    		sprintf( buf2, "Your %s %s $N%c",  attack, vp, punct );
	    		sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
			}
		}
    }

    if (ch == victim)
    {
		act(buf1,ch,NULL,NULL,TO_ROOM_FIGHT);
		act(buf2,ch,NULL,NULL,TO_CHAR_FIGHT);
    }
    else
    {
    	act( buf1, ch, NULL, victim, TO_NOTVICT);
    	act( buf2, ch, NULL, victim, TO_CHAR_FIGHT );
    	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
		act("`W$S weapon won't budge!`w",ch,NULL,victim,TO_CHAR_FIGHT);
		act("`W$n tries to disarm you, but your weapon won't budge!`w",
			ch,NULL,victim,TO_VICT);
		act("`W$n tries to disarm $N, but fails.`w",ch,NULL,victim,TO_NOTVICT);
		return;
    }

    act( "`W$n disarms you and sends your weapon flying!`w", 
	 ch, NULL, victim, TO_VICT    );
    act( "`WYou disarm $N!`w",  ch, NULL, victim, TO_CHAR_FIGHT    );
    act( "`W$n disarms $N!`w",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
		obj_to_char( obj, victim );
    else
    {
		obj_to_room( obj, victim->in_room );
		
		if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
			get_obj(victim,obj,NULL);
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("`RYour pulse races as you are consumned by rage!\n\r`w",ch);
	act("`W$n gets a wild look in $s eyes.`w",ch,NULL,NULL,TO_ROOM_FIGHT);
	check_improve(ch,gsn_berserk,TRUE,2);

	af.type		= gsn_berserk;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (ch->level/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);
 
    if ( MOUNTED(ch) )
    {
        send_to_char("You can't bash while riding!\n\r", ch);
        return;
    }
     
    if ( (chance = get_skill(ch,gsn_bash)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {	
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if ( !IS_NPC(victim) && !IS_NPC(ch))
    {
        if ( ch->pcdata->nokilling != 0)
        {
            send_to_char( "You have been banned from killing other players by the gods.\n\r", ch);
            return;
        }
	if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR_FIGHT);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR_FIGHT);
	return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 25;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= get_curr_stat(victim,STAT_DEX) * 4/3;

    /* speed */
    
	/*start Spellsong fix*/
	
	/*if (IS_SET(ch->off_flags,OFF_FAST))
		chance += 10;
    if (IS_SET(ch->off_flags,OFF_FAST))
		chance -= 20;*/
	
	/*The original version was both increasing and decreasing the ch's 
	chance for having the FAST flag set.  The new version allows the 
	victim to have a bonus for being fast and also has affect haste
	give the same bonus so mobs do not have an unfair advantage. -Spellsong*/
	
	if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
		chance += 10;
	if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
		chance -= 20;

	/*end Spellsong fix*/

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* now the attack */
    if (number_percent() < chance)
    {
    
	act("`W$n sends you sprawling with a powerful bash!`w",
		ch,NULL,victim,TO_VICT);
	act("`WYou slam into $N, and send $M flying!`w",ch,NULL,victim,TO_CHAR_FIGHT);
	act("`W$n sends $N sprawling with a powerful bash.`w",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	WAIT_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH);

       if (RIDDEN(victim))
       {
            mount_success(RIDDEN(victim), victim, FALSE);
       }	
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH);
	act("`BYou fall flat on your face!`w",
	    ch,NULL,victim,TO_CHAR_FIGHT);
	act("`B$n falls flat on $s face.`w",
	    ch,NULL,victim,TO_NOTVICT);
	act("`BYou evade $n's bash, causing $m to fall flat on $s face.`w",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
}

/*COUNTER SKILL - Brian Babey (aka Varen)
				  Adapted by Spellsong for use with this mud*/

bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
    int chance;
    int dam_type;
    OBJ_DATA *wield;

    if ( ch == NULL || victim == NULL )
		return FALSE;
	
	if (    ( get_eq_char(victim, WEAR_WIELD) == NULL ) ||
            ( !IS_AWAKE(victim) ) ||
            ( !can_see(victim,ch) ) ||
            ( get_skill(ch,gsn_counter) < 1 )
       )
       return FALSE;

    wield = get_eq_char(victim,WEAR_WIELD);

    chance = get_skill(ch,gsn_counter) / 6;
    chance += ( victim->level - ch->level ) / 2;
    chance += 2 * (get_curr_stat(victim,STAT_DEX) - get_curr_stat(ch,STAT_DEX));
    chance += get_weapon_skill(victim,get_weapon_sn(victim)) -
                    get_weapon_skill(ch,get_weapon_sn(ch));
    chance += (get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR) );

    if ( number_percent( ) >= chance )
        return FALSE;

    dt = gsn_counter;

    if ( dt == TYPE_UNDEFINED )
    {
		dt = TYPE_HIT;
		if ( wield != NULL && wield->item_type == ITEM_WEAPON )
			dt += wield->value[3];
		else 
			dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    {
		if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    }
	else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
		dam_type = DAM_BASH;

    act( "You reverse $n's attack and `Rcounter`w with your own!", ch, NULL, victim, TO_VICT    );
    act( "$N `Rreverses`w your attack!", ch, NULL, victim, TO_CHAR_FIGHT    );

    
	/*damage(victim,ch,dam/2, gsn_counter , dam_type ,TRUE ); buggy from ver diff.*/  /* DAM MSG NUMBER!! */
	damage(victim,ch,dam/3, gsn_counter , dam_type); /*modified to this - Spellsong*/
    	
	check_improve(victim,gsn_counter,TRUE,6);

    return TRUE;
}
/*CRITICAL STRIKE SKILL - Brian Babey (aka Varen)
				  Adapted by Spellsong for use with this mud*/
bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim)
{
        OBJ_DATA *obj;

        obj = get_eq_char(ch,WEAR_WIELD);

        if (
             ( get_eq_char(ch,WEAR_WIELD) == NULL ) || 
             ( get_skill(ch,gsn_critical)  <  1 ) ||
             ( get_weapon_skill(ch,get_weapon_sn(ch))  !=  100 ) ||
             ( number_range(0,100) > get_skill(ch,gsn_critical) )
           )
                return FALSE;


        if ( number_range(0,100) > 25 )
                return FALSE;


        /* Now, if it passed all the tests... */

        act("$p `RCRITICALLY STRIKES`w $n!",victim,obj,NULL,TO_NOTVICT);
        act("`RCRITICAL STRIKE!`w",ch,NULL,victim,TO_VICT);
        check_improve(ch,gsn_critical,TRUE,6);
        return TRUE;
}

/*PHASE SKILL - Brian Babey (aka Varen)
				  Adapted by Spellsong for use with this mud*/
bool check_phase( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

	if(IS_NPC(victim))
		if(str_cmp(race_table[victim->race].name,"spiritfolk"))
  	  		return FALSE;
		else
			chance=9*ch->level/10 + 10;
	else
		chance=victim->pcdata->learned[gsn_phase]/2;


    if (!can_see(victim,ch))
	chance /= 2;

    if( chance < 3 )
	return FALSE; /*Just fail*/

	if( number_percent() >= chance + ch->level - victim->level )
		return FALSE;
    /* Eris 8 July 2000, added if chance >1 bit to avoid phasing without the skill 
    if ( chance >= 2 && (number_percent( ) >= chance + victim->level - ch->level) )
        return FALSE;
*/
    act( "`BYour body phases to avoid $n's attack.`w", ch, NULL, victim, TO_VICT    );
    act( "`B$N's body phases to avoid your attack.`w", ch, NULL, victim, TO_CHAR_FIGHT    );
    check_improve(victim,gsn_phase,TRUE,6);
    return TRUE;
}


void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't dirt while riding!\n\r", ch);
        return;
    }

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$e's already been blinded.",ch,NULL,victim,TO_CHAR_FIGHT);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;
    if ( !IS_NPC(victim) && !IS_NPC(ch))
    {
	if ( ch->pcdata->nokilling != 0)
        {
            send_to_char( "You have been banned from killing other players by the gods.\n\r", ch);
            return;
        }
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }


    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR_FIGHT);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
		chance += 10;
    
	/*begin spellsong fix*/
	
	/*if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,OFF_FAST))*/
	if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
		chance -= 25;
	/*end spellsong fix - note the obvious bug here...lol*/

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("`W$n is blinded by the dirt in $s eyes!`w",victim,NULL,NULL,TO_ROOM_FIGHT);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE);
	send_to_char("`WYou can't see a thing!\n\r`w",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.type 	= gsn_dirt;
	af.level 	= ch->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't trip while riding!\n\r", ch);
        return;
    }
    
    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( !IS_NPC(victim) && !IS_NPC(ch))
    {
	if ( ch->pcdata->nokilling != 0)
        {
            send_to_char( "You have been banned from killing other players by the gods.\n\r", ch);
            return;
        }
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR_FIGHT);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR_FIGHT);
	return;
    }

    if (victim == ch)
    {
	send_to_char("`BYou fall flat on your face!\n\r`w",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM_FIGHT);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR_FIGHT);
	return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance)
    {
	act("`W$n trips you and you go down!`w",ch,NULL,victim,TO_VICT);
	act("`WYou trip $N and $N goes down!`w",ch,NULL,victim,TO_CHAR_FIGHT);
	act("`W$n trips $N, sending $M to the ground.`w",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	WAIT_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
}



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && !IS_NPC(ch))
    {
  	if ( ch->pcdata->nokilling != 0)
        {
            send_to_char( "You have been banned from killing other players by the gods.\n\r", ch);
            return;
        }
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR_FIGHT );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    buf[0] = '\0'; /*spellsong*/
	
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR_FIGHT );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if (IS_NPC(ch))
	sprintf(buf, "`YHelp! I am being attacked by %s!`w",ch->short_descr);
    else
    	sprintf( buf, "`YHelp!  I am being attacked by %s!`w", ch->name );
    do_yell( victim, buf );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( ch->pcdata->learned[gsn_backstab] < 2 )
    {
        send_to_char("Huh?\n\r",ch);
    }

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't backstab while riding!\n\r", ch);
        return;
    }
 
    if ( arg[0] == '\0' )
    {
	send_to_char( "Backstab whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;
    if ( !IS_NPC(victim) && !IS_NPC(ch))
    {
	if ( ch->pcdata->nokilling != 0)
        {
            send_to_char( "You have been banned from killing other players by the gods.\n\r", ch);
            return;
        }
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
    {
	send_to_char( "You can't backstab a fighting person.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit && !IS_AFFECTED(ch, AFF_SNEAK) )
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR_FIGHT );
	return;
    }
    
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < ch->pcdata->learned[gsn_backstab] )
    {
	check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE );
    }

    return;
}


/*Ok this is officially mine now because the original didnt work and i rewrote much of it
 * -SPELLSONG- */

void do_circle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't circle while riding!\n\r", ch);
        return;
    }
	if (arg[0] == '\0')
    {
        send_to_char("Circle whom?\n\r",ch);
        return;
    }
    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }
    if ( is_safe( ch, victim ) )
      return;

    if (IS_NPC(victim) &&
         victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to circle.\n\r", ch );
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You must be fighting in order to circle.\n\r", ch );
        return;
    }

	if (victim == ch)
    {
		send_to_char("`BYou try in vain to circle around yourself!\n\r`w",ch);
		WAIT_STATE(ch, 5 * PULSE_VIOLENCE); /*delay*/
		/*WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);*/
		act("$n lamely spins around in circles!",ch,NULL,NULL,TO_ROOM_FIGHT);
		return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
		act("$N is your beloved master.",ch,NULL,victim,TO_CHAR_FIGHT);
		return;
    }

    check_killer( ch, victim );
	WAIT_STATE(ch, 5 * PULSE_VIOLENCE); /*delay*/
    /*WAIT_STATE( ch, skill_table[gsn_circle].beats );*/

    if ( number_percent( ) < get_skill(ch,gsn_circle)
    || ( get_skill(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,gsn_circle,TRUE,1);
        multi_hit( ch, victim, TYPE_UNDEFINED );
    }
    else
    {
        check_improve(ch,gsn_circle,FALSE,1);
        damage( ch, victim, 0, TYPE_UNDEFINED,DAM_NONE);
    }
    return;
}


void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	int xpMod;
    int attempt;

	xpMod = 0;

    if ( ( victim = ch->fighting ) == NULL && !IS_NPC(ch) ) //Eris add to allow turning to flee
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM_FIGHT );
	ch->in_room = now_in;

	/*spellsong rehack/newcode*/
	if ( !IS_NPC(ch) )
	{
		/*lets give thieves a chance to get away safely*/
		if( (ch->class == 2) 
			&& (number_percent() < 3*((ch->level/3)-3) ) )
		{
			send_to_char( "You flee from combat!\n\r", ch );
			send_to_char( "You slip away safely.\n\r", ch);
		}
		else
			{
				xpMod = (ch->exp * .01); /*fleeing you lose 1% of you xp*/
				gain_exp( ch, 0-xpMod );
				sprintf(buf,"You flee from combat!  You lose %d exps.\n\r", xpMod );
				send_to_char( buf, ch );
			}
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "`RPANIC! You couldn't escape!\n\r`w", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !is_same_group(ch,victim))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_rescue] )
    {
	send_to_char( "`BYou fail the rescue.\n\r`w", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "`WYou rescue $N!`w",  ch, NULL, victim, TO_CHAR_FIGHT    );
    act( "`W$n rescues you!`w", ch, NULL, victim, TO_VICT    );
    act( "`W$n rescues $N!`w",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't kick while riding!\n\r", ch);
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && !IS_NPC(ch) )
    {
  	if ( ch->pcdata->nokilling != 0)
        {
            send_to_char( "You have been banned from killing other players by the gods.\n\r", ch);
            return;
        }
        if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_kick] )
    {
	damage( ch, victim, number_range( 1, ch->level ), gsn_kick,DAM_BASH );
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH );
	check_improve(ch,gsn_kick,FALSE,1);
    }

    return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    hth = 0;


    if ( ( victim = ch->fighting ) == NULL )
    {
		if (get_skill(ch,gsn_disarm_trap) != 0)
		{
			do_disarm_trap(ch,argument);
			return;
		}
    }

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
	act("`BYou fail to disarm $N.`w",ch,NULL,victim,TO_CHAR_FIGHT);
	act("`B$n tries to disarm you, but fails.`w",ch,NULL,victim,TO_VICT);
	act("`B$n tries to disarm $N, but fails.`w",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    extern bool chaos;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "`RYou slay $M in cold blood!`w",  ch, NULL, victim, TO_CHAR_FIGHT    );
    act( "`R$n slays you in cold blood!`w", ch, NULL, victim, TO_VICT    );
    act( "`R$n slays $N in cold blood!`w",  ch, NULL, victim, TO_NOTVICT );
    if ( !IS_NPC(victim) ) victim->pcdata->nemesis=NULL;

   if (chaos)
	chaos_kill( victim );

   if (!chaos)
    raw_kill( victim );

    return;
}

void chaos_log( CHAR_DATA *ch, char *argument )
{
    append_file( ch, CHAOS_FILE , argument );
    return;
}


bool raf_damage( CHAR_DATA *victim, int dam, int dam_type , char *dam_name) 
{
    char buf[200];
    bool immune;
    extern bool chaos;

	buf[0] = '\0';
    
	if ( victim->position == POS_DEAD )
	return FALSE;

   


    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    /* Eris new spell add 30 April 2000 */
    if ( dam > 1 && IS_AFFECTED(victim, AFF_PRESERVATION) )
	dam -= dam / 3.5;

    /* Eris ice_shield and fire_shield added code 2 July 2000 */
                        if ( dam > 1 &&
                          dam_type == DAM_COLD &&
                            IS_AFFECTED(victim, skill_lookup("ice shield")))
                                dam /= 2;

                        if ( dam > 1 &&
                          dam_type == DAM_FIRE &&
                            IS_AFFECTED(victim, skill_lookup("fire shield")))
                                dam /= 2;

    immune = FALSE;


    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "`R$n is mortally wounded, and will die soon, if not aided.`w",
	    victim, NULL, NULL, TO_ROOM_FIGHT );
	send_to_char( 
	    "`RYou are mortally wounded, and will die soon, if not aided.\n\r`w",
	    victim );
	break;

    case POS_INCAP:
	act( "`R$n is incapacitated and will slowly die, if not aided.`w",
	    victim, NULL, NULL, TO_ROOM_FIGHT );
	send_to_char(
	    "`RYou are incapacitated and will slowly die, if not aided.\n\r`w",
	    victim );
	break;

    case POS_STUNNED:
	act( "`R$n is stunned, but will probably recover.`w",
	    victim, NULL, NULL, TO_ROOM_FIGHT );
	send_to_char("`RYou are stunned, but will probably recover.\n\r`w",
	    victim );
	break;

    case POS_DEAD:
	mprog_death_trigger( victim );
	act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM_FIGHT );
	send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "`RThat really did HURT`R!\n\r`w", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "`RYou sure are BLEEDING`R!\n\r`w", victim );
	break;
    }


    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name, dam_name,	victim->in_room->vnum );
	    log_string( log_buf );

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( !chaos && victim->exp > 0 )
		gain_exp( victim, -1*(victim->exp/2) );
	}

       if (!IS_NPC(victim)) 
	 {
            sprintf(buf, "%s has been slain by %s!", victim->name, dam_name);
            do_sendinfo(victim, buf);
	 }
       
       if (chaos)
	chaos_kill( victim );

        if (!chaos)
	raw_kill( victim );
    }
   

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */

       if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
	 {
	    if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
		  &&   victim->hit < victim->max_hit / 5) 
		||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
		      &&     victim->master->in_room != victim->in_room ) )
	      do_flee( victim, "" );
	 }
       
       if ( !IS_NPC(victim)
	   &&   victim->hit > 0
	   &&   victim->hit <= victim->wimpy
	   &&   victim->wait < PULSE_VIOLENCE / 2 )
	 do_flee( victim, "" );
       
       tail_chain( );
       return TRUE;
}

/***************************************************************************
* CODED BY SPELLSONG FOR REALMS OF DISCORDIA AND EMBERMUD                  *
* Email at nfn15571@naples.net as well for any requests for reuse. Until   *
* such permission is explicitly given, no reuse is authorized.             *
***************************************************************************/

bool vorpal_kill( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dam_type)
{
    int roll;
	int chance;
	
	if ( ch == NULL || victim == NULL ) /*just in case spellsong*/
	{
		log_string("BUG: null pointer in vorpal_kill");
		return FALSE;
	}

    if ( victim->position == POS_DEAD ) /*cant kill a dead person*/
        return FALSE;

    if ( victim == ch )
    {
        log_string("BUG: victim == ch in vorpal_kill"); /*cant vorp yourself*/
        return FALSE;
    }
    
	if ( is_safe( ch, victim ) ) /* Should never get to this point if safe */
		return FALSE;			 /* but just in case SS */

	if ( !IS_SET(victim->parts,PART_HEAD) ) /*gotta have a head to cut off :)*/
		return FALSE;

	if ( check_immune(victim,dam_type) == IS_IMMUNE ) /*can't be immune*/
		return FALSE;

	if ( IS_IMMORTAL(victim) && !IS_IMMORTAL(ch) ) /*only immortals decapitate immortals*/
		return FALSE;

	/*get level difference*/
    chance = ch->level - victim->level;
	
	/*dexterity makes a difference*/
	chance += get_curr_stat(ch,STAT_DEX) - get_curr_stat(victim,STAT_DEX);

	/*damage checks for bonuses*/
	if ( dam > 50 )
		chance += 1;

	if ( dam > 100 ) /*yes they are cumalative!*/
		chance += 1;

	/*max chance*/
	if ( chance > 10 )
		chance = 10;

	/*min chance*/
	if (chance < 1 )
		chance = 1;
	
    /*ok...perform the check*/
	roll = number_range( 1, 300 );
	
	if ( roll <= chance ) /*ok the magic of the blade is working*/
	{
		if ( number_percent() < (chance * 2) +10 ) /*now are we in position???*/
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		return FALSE;
	}
	/*just in case*/
	return FALSE;

}

