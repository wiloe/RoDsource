/*
 *      Mystic's spells/skills file
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

/* command procedures needed */
DECLARE_DO_FUN(do_look		);

char *target_name;

/****************
* HEALING GROUP *
****************/

/*replace cure light wounds*/
void spell_patch_wounds( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int patch;

    /*do the healing*/
	patch = dice(1, 8) + level / 3;
    victim->hit = UMIN( victim->hit + patch, victim->max_hit );
    
	update_pos( victim );
    
	if ( ch != victim )
	{
		act( "$n patches the wounds on $n!",ch ,NULL, victim, TO_ROOM );
		act( "$n patches your wounds.",ch ,NULL, victim, TO_VICT );
		send_to_char( "You succesfully patch their wounds.\n\r", ch );
	}

	send_to_char( "You feel better!\n\r", victim );
    return;
}

/*replace cure serious wounds*/
void spell_bind_wounds( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int bind;

    /*do the healing*/
	bind = dice(2, 8) + level /2 ;
    victim->hit = UMIN( victim->hit + bind, victim->max_hit );
    
	update_pos( victim );

    if ( ch != victim )
	{
		act( "$n binds the wounds on $n!",ch ,NULL, victim, TO_ROOM );
		act( "$n binds your wounds.",ch ,NULL, victim, TO_VICT );
		send_to_char( "You summon devine power and bind their wounds.\n\r", ch );
	}

	send_to_char( "You feel better!\n\r", victim );
    return;
}

/*replace cure critical*/
void spell_mend_wounds( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int mend;

    /*do the healing*/
	mend = (dice(3, 8) + level/2) + level/4;
    victim->hit = UMIN( victim->hit + mend, victim->max_hit );
    
	update_pos( victim );

    if ( ch != victim )
	{
		act( "$n mends the wounds on $N!",ch ,NULL, victim, TO_ROOM );
		act( "$n mends your wounds.",ch ,NULL, victim, TO_VICT );
		send_to_char( "You succesfully mend their wounds.\n\r", ch );
	}

	send_to_char( "You feel much better!\n\r", victim );
    return;
}

/*replace heal*/
void spell_repair_wounds( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    
	victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    
	update_pos( victim );
    
	send_to_char( "Your wounds begin to repair themselves.\n\r", victim );
    
	act( "The wounds on $n begin to repair themselves!",victim ,NULL, NULL, TO_ROOM );
	
    return;
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    if (victim->max_move == victim->move)
	send_to_char("You feel fully refreshed!\n\r",victim);
    else
    	send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

/***************
* ADVANCE HEAL *
***************/

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = skill_lookup("repair wounds");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
		if ((IS_NPC(ch) && IS_NPC(gch)) ||
			(!IS_NPC(ch) && !IS_NPC(gch)))
		{
			spell_repair_wounds(heal_num,level,ch,(void *) gch);
			spell_refresh(refresh_num,level,ch,(void *) gch);  
		}
    }
}

/*as is from old*/
void spell_restoration( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    
	victim->hit = UMIN( victim->hit + 150, victim->max_hit );
    
	update_pos( victim );
    
	act("$n glows with a blinding light.",ch,NULL,NULL,TO_ROOM);
    
	send_to_char( "A warm feeling fills your body.\n\r", victim );
    
	if ( ch != victim )
		send_to_char( "You surge with healing power.\n\r", ch );
    return;
}

void spell_succor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    
	victim->hit = UMIN( victim->hit + 200, victim->max_hit );
    
	update_pos( victim );
    
	act("$n glows with power.",ch,NULL,NULL,TO_ROOM);
    
	send_to_char( "A jolt shoots though your body.\n\r", victim );
    
	if ( ch != victim )
		send_to_char( "You nearly explode with healing power.\n\r", ch );
    return;
}

/*as is from old*/
void spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_REGENERATION) )
    {
        if (victim == ch)
          send_to_char("Your body is already in a regenerative state.\n\r",ch);
        else
          act("$N's body is already in a regerative state.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
	/*apply affect*/
	af.type      = sn;
    af.level     = level;
    af.duration  = level/8+5; /*changed from /10 for pulse speed*/
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_REGENERATION;
    affect_to_char( victim, &af );
    
	send_to_char( "Your body tingles.\n\r", victim );
    
	if ( ch != victim )
		act("You imbue $N's body with regenerative powers." ,ch,NULL,victim,TO_CHAR);
    return;
}

void spell_energy_transferance( int sn, int level, CHAR_DATA *ch, void *vo )
{
	const int ENERGY = 35;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	
	/*this spell will transfer 35 mana from ch to victim*/
	if( victim == ch )
	{
		send_to_char("You tranfer energy to yourself.\n\r", ch );
		return;
	}

	if( ch->mana - ENERGY < 1 )
	{
		victim->mana = UMIN( victim->mana + ch->mana, victim->max_mana );
		ch->mana = 0;
	}
	else
	{
		victim->mana = UMIN( victim->mana + ENERGY, victim->max_mana );
		ch->mana -= ENERGY;
	}

	act("$n glows with energy.",ch,NULL,NULL,TO_ROOM);
    send_to_char( "You feel drained of energy.\n\r", ch );
	send_to_char( "You feel imbued with energy.\n\r", victim );


	return;
}

/*************
* HARM GROUP *
*************/

/*same as cause light*/
void spell_mar( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("$n touches $n.", ch, NULL, victim, TO_ROOM);
	act("$n reaches out and touches you!",ch ,NULL, victim, TO_VICT );
	
	act("Bruises appear on the body of $n!", victim, NULL, NULL, TO_ROOM);
	send_to_char( "Bruises appear on your body!\n\r", victim );

    damage( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 2, sn, DAM_HARM );
    return;
}


/*same as cause serious*/
void spell_abuse( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("$n touches $n.", ch, NULL, victim, TO_ROOM);
	act("$n reaches out and touches you!",ch ,NULL, victim, TO_VICT );
	
	act("The body of $n looks badly abused!", victim, NULL, NULL, TO_ROOM);
	send_to_char( "You feel badly abused!\n\r", victim );

    damage( ch, (CHAR_DATA *) vo, dice(2, 8) + level + 4, sn, DAM_HARM );
    return;
}

/*same as cause critical*/
void spell_injure( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("$n touches $n.", ch, NULL, victim, TO_ROOM);
	act("$n reaches out and touches you!",ch ,NULL, victim, TO_VICT );
	
	act("Injuries appear on the body of $n!", victim, NULL, NULL, TO_ROOM);
	send_to_char( "Injuries appear on your body!\n\r", victim );

    damage( ch, (CHAR_DATA *) vo, dice(3, 8) + level - 2, sn, DAM_HARM );
    return;
}

/*same as harm*/
void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
	act("$n grabs $n.", ch, NULL, victim, TO_ROOM);
	act("$n grabs you!",ch ,NULL, victim, TO_VICT );
	
	act("$n is wracked with pain!", victim, NULL, NULL, TO_ROOM);
	send_to_char( "You are wracked with pain!\n\r", victim );
	
	dam = 100 + dice(4, 8) + (level/3);
	if ( saves_spell( level, victim ) )
		dam = dam/2;

	damage( ch, victim, dam, sn, DAM_HARM );
    return;
}

void spell_inflict( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
	act("$n glares at $n.", ch, NULL, victim, TO_ROOM);
	act("$n glares at you!",ch ,NULL, victim, TO_VICT );
	
	act("A dark shadow appears over $n!", victim, NULL, NULL, TO_ROOM);
	send_to_char( "A dark shadow appears over you!\n\r", victim );
	
	dam = 150 + dice(4, 8) + (level/3);
	if ( saves_spell( level, victim ) )
		dam = dam/2;

	damage( ch, victim, dam, sn, DAM_HARM );
    return;
}

void spell_avenge( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
	act("$n points at $n.", ch, NULL, victim, TO_ROOM);
	act("$n points at you!",ch ,NULL, victim, TO_VICT );
	
	act("A black bolt of energy flies towards $n!", victim, NULL, NULL, TO_ROOM);
	send_to_char( "A black bolt flies towards you!\n\r", victim );
	
	dam = 200 + dice(4, 8) + (level/3);
	if ( saves_spell( level, victim ) )
		dam = dam/2;

	damage( ch, victim, dam, sn, DAM_HARM );
    return;
}

/***********
* CURATIVE *
***********/

/*new spell*/
void spell_awaken( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_sleep ) )
    {
        act("$N doesn't appear to be in a magical slumber.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
	/*dispel at level +5, its more powerful than cancel.*/
    if (check_dispel(level+5,victim,gsn_sleep))
    {
		send_to_char("The embrace of magical slumber leaves you.\n\r",victim);
	}
    else
		send_to_char("Spell failed.\n\r",ch);
	return;
}
 
void spell_cure_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_blindness ) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\n\r",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,gsn_blindness))
    {
        send_to_char( "Your vision returns!\n\r", victim );
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);

	return;
}

void spell_cure_disease( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char erisbuf[MAX_STRING_LENGTH]; /* Eris 1 May 2000 */
    int expgain;
    erisbuf[0] = '\0'; /* Eris 1 May 2000 */

    if ( !is_affected( victim, gsn_plague ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\n\r",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level,victim,gsn_plague))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
      /* Healers now get experience for curing - Eris 1 May 2000 */
	expgain = ch->level * ch->level *10;
	ch->exp += expgain;
	sprintf(erisbuf, "You gain %d experience points for your efforts!\n\r", expgain);
	send_to_char(erisbuf, ch);	
    }
    else
	send_to_char("Spell failed.\n\r",ch);

	return;
}

void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char erisbuf[MAX_STRING_LENGTH]; /* Eris 1 May 2000 */
    int expgain;
    erisbuf[0]  = '\0';
 
    if ( !is_affected( victim, gsn_poison ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,gsn_poison))
    {
        send_to_char("A warm feeling runs through your body.\n\r",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
        /* Healers now get experience for curing - Eris 1 May 2000 */
	  expgain = ch->level * ch->level *10;
	  ch->exp += expgain;
	  sprintf(erisbuf, "You gain %d experience points for your efforts!\n\r", expgain);
	  send_to_char(erisbuf, ch);	
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}

/*************
* Protective *
*************/

void spell_barkskin( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
		if (victim == ch)
		  send_to_char("Your skin is already covered in bark.\n\r",ch);
		else
		  act("$N is already protected by bark.",ch,NULL,victim,TO_CHAR);
		return;
    }

	/*apply affects*/
    af.type      = sn;
    af.level	 = level;
    af.duration  = (level*3)/2;
    af.modifier  = -15;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    
	send_to_char( "You skin is encased with bark.\n\r", victim );
    
	if ( ch != victim )
		act("$N is protected by bark.",ch,NULL,victim,TO_CHAR);
    
	return;
}

void spell_preservation( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PRESERVATION) )
    {
	if (victim == ch)
	  send_to_char("You are already affected by preservation.\n\r",ch);
	else
	  act("$N is already affected by preservation.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PRESERVATION;
    affect_to_char( victim, &af );
    act( "$n smiles briefly.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are now under preservation.\n\r", victim );
    return;
}

void spell_protection_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if( ch != victim )
	{
		send_to_char("You cannot protect another from this.\n\r",ch);
		return;
	}

    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) ) 
	{
		send_to_char("You are already protected.\n\r",ch);
		return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level+10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PROTECT_EVIL; 
    affect_to_char( victim, &af );
    send_to_char( "You feel protected and imbued with light.\n\r", victim );
    	
    return;
}

void spell_protection_good( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if( ch != victim )
	{
		send_to_char("You cannot protect another from this.\n\r",ch);
		return;
	}

    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) ) 
	{
		send_to_char("You are already protected.\n\r",ch);
		return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level+10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PROTECT_GOOD; 
    affect_to_char( victim, &af );
    send_to_char( "You feel protected and seeded with darkness.\n\r", victim );
    	
    return;
}

void spell_psychic_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if( ch != victim )
	{
		send_to_char("You cannot protect another with this.\n\r",ch);
		return;
	}

    if ( is_affected( victim, sn ) )
    {
		send_to_char("You are already protected by a psychic shield.\n\r",ch);
		return;
    }

	/*apply affects*/
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = -30;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    
	send_to_char( "You are protected by a psychic shield.\n\r", victim );
    return;
}

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
		if (victim == ch)
		  send_to_char("You are already in sanctuary.\n\r",ch);
		else
		  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
		return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    
	act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    return;
}

void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if( ch != victim )
	{
		send_to_char("You cannot protect another with this.\n\r",ch);
		return;
	}

    if ( is_affected( ch, sn ) )
    {
		send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
		return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to stone.\n\r", victim );
    return;
}



/*includes Cancel, Dispel */

/****************
* WEATHER GROUP *
****************/

void spell_call_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !IS_OUTSIDE(ch) )
    {
		send_to_char( "You must be outside for this.\n\r", ch );
		return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
		send_to_char( "You need bad weather.\n\r", ch );
		return;
    }

    dam = dice(level/2, 8);

    send_to_char( "The gods' lightning strikes your foes!\n\r", ch );
    act( "$n calls the gods' lightning to strike $s foes!",
		ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
		vch_next	= vch->next;

		if ( vch->in_room == NULL )
			continue;
		
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
			{
				if (saves_spell(level,vch))
					damage( ch, vch, dam/2, sn, DAM_LIGHTNING );
				else
				{
					damage( ch, vch, dam, sn, DAM_LIGHTNING );
					
					/*ok failed save lets see if we should check to torch something*/ 
					
					if (number_percent() < 15)
						shock_effect(vch,level,dam,TARGET_CHAR); 
				}
			}
			
			continue;
		}

		if ( vch->in_room->area == ch->in_room->area
			&&   IS_OUTSIDE(vch)
			&&   IS_AWAKE(vch) )
			send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }
 	
	/*regardless torch stuff in room*/ 	
	if(ch->in_room != NULL)
		shock_effect(ch->in_room, level, dam, TARGET_ROOM);

    return;
}

void spell_control_weather( int sn, int level, CHAR_DATA *ch, void *vo )
{
    if ( !str_cmp( target_name, "better" ) )
		weather_info.change += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
		weather_info.change -= dice( level / 3, 4 );
    else
		send_to_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
		return;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level+10; 
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    
	send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
		if ( !IS_NPC(ich) && IS_SET(ich->act, PLR_WIZINVIS) )
			continue;

		if ( ich == ch || saves_spell( level, ich ) )
			continue;

		affect_strip ( ich, gsn_invis			);
		affect_strip ( ich, gsn_mass_invis		);
		affect_strip ( ich, gsn_sneak			);
		
		REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
		REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
		REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
		
		act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
		send_to_char( "You are revealed!\n\r", ich );
    }

    return;
}


void spell_tornado( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
	int dam;

    send_to_char( "A howling tornado appears at your call!\n\r", ch );
    act( "A howling tornado appears before $n.",
		ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
		vch_next = vch->next;

		if ( vch->in_room == NULL )
			continue;
		
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
			{
				
				dam = dice(12, 4) + level + ((level * 5)/4);

				if (IS_AFFECTED(vch,AFF_FLYING))
				{
					dam += (dam/3); /*flying charaters take the brunt of this!*/
					
					act( "The wind buffets $n!",vch, NULL, NULL, TO_ROOM );
					send_to_char( "You are tossed and turned by the wind!\n\r", vch );
				}

				if( saves_spell( level, vch ) )
					dam = dam/2;
				
				damage( ch, vch, dam , sn, DAM_BASH );
			}
			continue;
		}

		if ( vch->in_room->area == ch->in_room->area )
			send_to_char( "You hear the loud howl of a powerful wind.\n\r", vch );
    }

    return;
}

void spell_typhoon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
	int dam;

    send_to_char( "A raging typhoon appears in the air around you!\n\r", ch );
    act( "$n is encircled by a raging typhoon!",ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
		vch_next = vch->next;

		if ( vch->in_room == NULL )
			continue;
		
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
			{
				
				dam = dice(16, 4) + level + ((level * 5)/3);

				if (IS_AFFECTED(vch,AFF_FLYING))
				{
					dam += (dam/3); /*flying charaters take the brunt of this!*/
					
					act( "The wind and rain buffets $n!",vch, NULL, NULL, TO_ROOM );
					send_to_char( "Blinding rain and stinging wind buffet you!\n\r", vch );
				}

				if( saves_spell( level, vch ) )
					dam = dam/2;
				
				damage( ch, vch, dam , sn, DAM_BASH );
			}
			continue;
		}

		if ( vch->in_room->area == ch->in_room->area )
			send_to_char( "You hear the ominous rumble of a powerful storm.\n\r", vch );
    }

    return;
}

/******************
* ELEMENTAL GROUP *
******************/

/*level 10*/
void spell_geyser( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	    
	dam = dice(3, 8) + level/4;
    
	if ( saves_spell( level, victim ) )
		dam /= 2;
	
    act("$n shoots forth a steaming blast of water.",ch,NULL,NULL,TO_ROOM);
    
	damage( ch, victim, dam, sn, DAM_FIRE );
	
	return;
}

/*level 20*/
void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	bool saved;
 	
	saved = FALSE; 
    
	dam = dice(6, 8) + level/4;
    
	if ( saves_spell( level, victim ) )
	{ 	
		dam /= 2;
		saved = TRUE;
	}

    act("$n calls down a column of fire from the sky.",ch,NULL,NULL,TO_ROOM);
    
	damage( ch, victim, dam, sn, DAM_FIRE );

    if( saved == FALSE && number_percent() < 10 )
		fire_effect( victim, level, dam, TARGET_CHAR ); 	
	
	return;
}

/*level 30*/
void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
		vch_next = vch->next;

		if ( vch->in_room == NULL )
			continue;
		
		if ( vch->in_room == ch->in_room )
		{
			if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
			{
				if (IS_AFFECTED(vch,AFF_FLYING))
					damage(ch,vch,0,sn,DAM_BASH);
				else
					damage( ch, vch, dice(6, 8) + level, sn, DAM_BASH );
			}
			continue;
		}

		if ( vch->in_room->area == ch->in_room->area )
			send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return;
}

/*level 40*/
void spell_ice_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/*A slight protective effect - 5 AC bonus and reduces cold damage by 1/2*/
CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

        if( ch != victim )
        {
                send_to_char("You cannot protect another with this.\n\r",ch);
                return;
        }

    if ( is_affected( victim, sn ) )
    {
                send_to_char("You are already protected by a shield of ice.\n\r",ch);
                return;
    }

        /*apply affects*/
    af.type      = sn;
    af.level     = level;
    af.duration  = level/3;
    af.modifier  = -5;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );

        send_to_char( "You are protected by a shield of ice.\n\r", victim );
	return;
}

/*level 50*/
void spell_fire_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/*A protective effect - 10 AC bonus and reduces fire damage by 1/2*/
CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

        if( ch != victim )
        {
                send_to_char("You cannot protect another with this.\n\r",ch);
                return;
        }

    if ( is_affected( victim, sn ) )
    {
                send_to_char("You are already protected by a shield of fire.\n\r",ch);
                return;
    }

        /*apply affects*/
    af.type      = sn;
    af.level     = level;
    af.duration  = level/3;
    af.modifier  = -10;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );

        send_to_char( "You are protected by a shield of fire.\n\r", victim );
	return;
}

/*level 61*/
void spell_sandblast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
	dam = dice(6, 8) + level + ( (level * 7) / 3);
    
	if ( saves_spell( level, victim ) )
		dam /= 2;
	
    act("Sand, powered by a gail force wind, errupts from $n!",ch,NULL,NULL,TO_ROOM);
    
	damage( ch, victim, dam, sn, DAM_BASH );
    	
	
	return;
}

/*level 70*/
void spell_summon_earth_elemental( int sn, int level, CHAR_DATA *ch, void *vo )
{
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *elemental;
	int i;
   
	//I dont think so....
	if ( ch->pet != NULL )
	{
	   send_to_char("You already have a follower.\n\r",ch);
	   return;
	}

	//perform this in a fight...yeah right!
	if(ch->position == POS_FIGHTING)
	{
	  send_to_char("You can't perform the ritual while fighting!\n\r",ch);
	  return;
	}

	//add water stuff to this check as well
	if( ch->in_room == NULL || ch->in_room->sector_type == SECT_AIR )
	{
	   send_to_char("This area lacks the element you need.\n\r",ch);
	   return;
	}
	
	//grab the pointer and create the mob
	pMobIndex = get_mob_index(MOB_VNUM_ELEMENTAL);
	elemental = create_mobile( pMobIndex );

	//important stats
	elemental->level = level - 5;

	if( elemental->level < 1 )
		elemental->level = 1;
	
	elemental->mana = elemental->max_mana = number_fuzzy(elemental->level * 20);
	elemental->hit = elemental->max_hit = number_fuzzy(elemental->level * 20);
	
	//assign armor
	for(i = 0; i < 4; i++)
	   elemental->armor[i] = number_fuzzy( 100 - (elemental->level * 5) );
	
	//assign hit 
	elemental->hitroll = number_fuzzy(level / 10);
	elemental->damage[DICE_NUMBER] = 1;//automob_table[elemental->level].hitnum;
	elemental->damage[DICE_TYPE]   = elemental->level;//automob_table[elemental->level].hitdice;
	elemental->damage[DICE_BONUS]  = 0;//automob_table[elemental->level].hitbonus;;	

	//visuals
	send_to_char("You invoke the element of earth.\n\r",ch);
	act( "$n begins to envoke the element of earth!", ch, NULL, NULL, TO_ROOM );
	
	char_to_room( elemental, ch->in_room );
	WAIT_STATE(ch, 2 * PULSE_MOBILE);
		
    act( "$n rises forth out of the ground!", elemental, NULL, NULL, TO_ROOM );
    	
	//do the follower stuff
	add_follower( elemental, ch );
	elemental->leader = ch;
	ch->pet = elemental;
	SET_BIT(elemental->affected_by, AFF_CHARM);
	
	//drains all players movement
	ch->move = 0;  

	return;
}


/****************
* PSYCHIC GROUP *
****************/

/*all spells caster only*/

/*level 12*/
void spell_chakra( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/*adds save vrs spell*/
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if( victim != ch )
	{
		send_to_char("You may not ward others against magic.\n\r",ch);
		return;
	}
    
	if( is_affected( victim, sn ) )
    {
		send_to_char("You are already warded against hostile magic.\n\r",ch);
		return;
    }

	/*apply affects*/
    af.type      = sn;
    af.level	 = level;
    af.duration  = 1 + level/3;
    af.modifier  = 0 - (1 + level / 20);
    af.location  = APPLY_SAVING_SPELL;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    
	send_to_char( "You are warded against magic.\n\r", victim );
    
	act("$N is warded against magic.",ch,NULL,victim,TO_CHAR);
    
	return;
	
}

/*level 25*/
void spell_mind_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/*improves resistance to dispel*/
	return;
}

/*level 38*/
void spell_deflect( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/*chance to turn next spell back on caster*/
	return;
}

/*level 55*/
void spell_enslave( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
		send_to_char( "You follow your own orders even better!\n\r", ch );
		return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim ) )
		return;


    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
		send_to_char( "Something prevents your attempt.\n\r",ch);
		return;
    }

    if ( victim->master )
		stop_follower( victim );

    add_follower( victim, ch );
    
	victim->leader = ch;
    
	af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 2 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    
	act( "You will is bent to follow $n!", ch, NULL, victim, TO_VICT );
    
	if ( ch != victim )
		act("$N will is bent by your power!",ch,NULL,victim,TO_CHAR);
    
	return;
}

/*level 72*/
void spell_spirit_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if( ch != victim )
	{
		send_to_char("You cannot protect another with this.\n\r",ch);
		return;
	}

    if ( is_affected( victim, sn ) )
    {
		send_to_char("You are already protected by a spirit shield.\n\r",ch);
		return;
    }

	/*apply affects*/
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/3;
    af.modifier  = -40;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    
	send_to_char( "You are protected by a spirit shield.\n\r", victim );
    return;
}


/******************
* DIVINATION GROUP *
******************/


void spell_aura( int sn, int level, CHAR_DATA *ch, void *vo )
{
    /*allows caster to detect good and evil. i.e. see auras*/
	return;
}

void spell_revelation( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/*this allows the caster to detect hidden and invis*/
}

void spell_foresight( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/*this gives limited information about a mob. Maybe attack forms
	  and some special abilities. Depending on level difference and such*/
}

void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf[MAX_INPUT_LENGTH];
    char buffer[4*MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    buffer[0] = '\0'; 	
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) 
    	||   (!IS_IMMORTAL(ch) && number_percent() > 2 * level)
	||   ch->level < obj->level)
	    continue;

	found = TRUE;
        number++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see)
	{
	    sprintf( buf, "%s carried by %s\n\r",
		obj->short_descr, PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "%s in %s [Room %d]\n\r",
		    obj->short_descr, 
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else
	    	sprintf( buf, "%s in %s\n\r",
		    obj->short_descr, in_obj->in_room == NULL
		    	? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	strcat(buffer,buf);

    	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_char( "Nothing like that anywhere in the land.\n\r", ch );
    else if (ch->lines)
	page_to_char(buffer,ch);
    else
	send_to_char(buffer,ch);

    return;
}

void spell_vision( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *original;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_MENTAL))
    ||   (IS_NPC(victim) && saves_spell( level, victim ) ) )
    {
        send_to_char( "There is a disturbance in the ether.\n\r", ch );
        return;
    }

    
    act("$n falls into a trance.",ch,NULL,NULL,TO_ROOM);
    send_to_char("Through a cloudy fog, you have a vision.\n\r",ch);
    
	original = ch->in_room;
    
	char_from_room(ch);
    char_to_room(ch,victim->in_room);

    send_to_char("You feel a shiver down your spine.\n\r",victim);
    
	do_look(ch,"auto");
    
	char_from_room(ch);
    char_to_room(ch, original);
}

/********************
* MALADICTION GROUP *
********************/

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
		if (victim == ch)
		  send_to_char("You've already been changed.\n\r",ch);
		else
		  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
		return;
    }

    if (saves_spell(level , victim))
		return;	
    
	af.type      = sn;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_SEX;
    
	do
    {
		af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    
	af.bitvector = 0;
    affect_to_char( victim, &af );
    
	send_to_char( "You feel different.\n\r", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
    
	return;
}


/* new spell - encase */
void spell_encase( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int drain;

	drain = dice( 3, level/2 );

	if( saves_spell( level, victim ) )
		drain /= 2;

	victim->move -= UMIN( 0 , drain );

	send_to_char( "You feel very exhausted.\n\r", victim );
    act("$n looks out of breath and exhausted.",victim,NULL,NULL,TO_ROOM);
	
	return;
}

void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell( level, victim ) )
		return;

    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 10+level; 
    af.bitvector = AFF_BLIND; 
    
	affect_to_char( victim, &af );
    
	send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim ) )
    {
		act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
		send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
		return;
    }
    af.type      = sn;
    af.level     = level;
    af.duration  = level+10; 
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = AFF_POISON;

    affect_join( victim, &af );
    
	send_to_char( "You feel very sick.\n\r", victim );
    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level,victim) || 
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
		if (ch == victim)
		  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
		else
		  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
		return;
    }

    af.type 	  = sn;
    af.level	  = level * 3/4 +10; 
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -5; 
    af.bitvector = AFF_PLAGUE;

    affect_join(victim,&af);
   
    send_to_char("You scream in agony as plague sores erupt from your skin.\n\r",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",victim,NULL,NULL,TO_ROOM);

	return;
}

void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim ) )
		return;

    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2 +10;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 10);
    af.bitvector = AFF_WEAKEN;
    
	affect_to_char( victim, &af );
    
	send_to_char( "You feel weaker.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_CURSE) || saves_spell( level, victim ) )
		return;

    af.type      = sn;
    af.level     = level;
    af.duration  = 2*level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    
	if ( ch != victim )
		act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( saves_spell( level, victim ) )
    {
		send_to_char("You feel a momentary chill.\n\r",victim);  	
		return;
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);
    if ( victim->level <= 2 )
    {
		dam		 = ch->hit + 1;
    }
    else
    {
		gain_exp( victim, 0 -  5 * number_range( level/2, 3 * level / 2 ) );
		victim->mana	/= 2;
		dam		 = dice(1, level);
		ch->hit		+= dam;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);
    damage( ch, victim, dam, sn, DAM_NEGATIVE );

    return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo )
{
	//subtracts dex and strips an attack if more than 1 attack or dual wield
	//like reverse haste
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim ) )
		return;

    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 * (level / 25);
    af.bitvector = AFF_SLOW;
    
	affect_to_char( victim, &af );
    
	send_to_char( "You begin to move in slow motion.\n\r", victim );
    act("$n begins to move on slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}


/***********************
* TRANSPORTATION GROUP *
***********************/

void spell_water_breathing( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/*requries underwater coding*/
	return;
}

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;
    
    if (IS_NPC(victim))
      return;
   
    if ((location = get_room_index( ROOM_VNUM_TEMPLE)) == NULL)
    {
		send_to_char("You are completely lost.\n\r",victim);
		return;
    } 

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL) || IS_AFFECTED(victim,AFF_CURSE))
    {
		send_to_char("Spell failed.\n\r",victim);
		return;
    }

    if (victim->fighting != NULL)
		stop_fighting(victim,TRUE);
    
    ch->move /= 2;
    
	act("$n disappears.",victim,NULL,NULL,TO_ROOM);
    
	char_from_room(victim);
    char_to_room(victim,location);
    
	act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
    do_look(victim,"auto");
}

void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    extern bool chaos;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   (chaos)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))  
    ||   (IS_NPC(victim) && saves_spell( level, victim ) ) )
    {
		send_to_char( "A disturbance interrupts your summons.\n\r", ch );
		return;
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );

    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    
	act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
    
	do_look( victim, "auto" );
    return;
}

void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
		if (victim == ch)
		  send_to_char("You are already airborne.\n\r",ch);
		else
		  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
		return;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 10;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    
	affect_to_char( victim, &af );
    
	send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_wind_walk( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    extern bool chaos;
    bool gate_pet;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (chaos)
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim ) ) )
    {
        send_to_char( "A doldrum in the atmosphere prevents your transportation.\n\r", ch );
        return;
    }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
		gate_pet = TRUE;
    else
		gate_pet = FALSE;
    
    act("A light breeze blows and $n vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You are swept up in a wind.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    act("$n slowly materializes.",ch,NULL,NULL,TO_ROOM);
    do_look(ch,"auto");

    if (gate_pet)
    {
		act("A light breeze blows and $n vanishes.",ch->pet,NULL,NULL,TO_ROOM);
		send_to_char("You are swept up in a wind.\n\r",ch->pet);
		char_from_room(ch->pet);
		char_to_room(ch->pet,victim->in_room);
		act("$n slowly materializes.",ch->pet,NULL,NULL,TO_ROOM);
		do_look(ch->pet,"auto");
    }
    if( MOUNTED(ch) )
    {
        act("A light breeze blows and $n vanishes.",MOUNTED(ch),NULL,NULL,TO_ROOM);
        send_to_char("You are swept up in a wind.\n\r",MOUNTED(ch));
        char_from_room(MOUNTED(ch));
        char_to_room(MOUNTED(ch),victim->in_room);
        act("$n slowly materializes.",MOUNTED(ch),NULL,NULL,TO_ROOM);
        do_look(MOUNTED(ch),"auto");
    }
}

/*replaces pass-door for mystic*/
void spell_smoke_form( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
		if (victim == ch)
		  send_to_char("You are already incorpreal.\n\r",ch);
		else
		  act("$N is already incorpreal.",ch,NULL,victim,TO_CHAR);
		return;
    }
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 5 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

    act( "$n turns smokey.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn smokey.\n\r", victim );
    return;
}

/*similiar to teleport*/
void spell_shift( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;
    extern bool chaos;

    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( chaos )
    || ( victim != ch
    && ( saves_spell( level, victim ) || saves_spell( level, victim ) ) ) )
	{
		if (victim != ch )
			send_to_char( "You failed to shift.\n\r", ch );
		else
			send_to_char( "Your surroundings momentarily flicker.\n\r", ch );
		
		return;
	}

    for ( ; ; )
    {
		pRoomIndex = get_room_index( number_range( 0, 65535 ) );
		
		if ( pRoomIndex != NULL )
		{
			if ( can_see_room(ch,pRoomIndex)
			&&   !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
			&&   !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
			&&   !IS_SET(pRoomIndex->room_flags, ROOM_NOTELEPORT) )
				break;
		}
    }

    if (victim != ch)
		send_to_char("You have been shifted elsewhere!\n\r",victim);
	
	if (victim == ch)
		send_to_char("Your surroundings begin to shift!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

/********************
* BENEDICTION GROUP *
********************/

void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
    {
		if (victim == ch)
		  send_to_char("You are already blessed.\n\r",ch);
		else
		  act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
		return;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = 6+level;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = 0;
    
	affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    
	affect_to_char( victim, &af );
    
	send_to_char( "You feel righteous.\n\r", victim );
    if ( ch != victim )
		act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
		if (vch->position == POS_FIGHTING)
		{
			count++;
			if (IS_NPC(vch))
			  mlevel += vch->level;
			else
			  mlevel += vch->level/2;
			high_level = UMAX(high_level,vch->level);
		}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
		for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   		{
			if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
					IS_SET(vch->act,ACT_UNDEAD)))
			  return;

			if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
			||  is_affected(vch,skill_lookup("frenzy")))
			  return;
			
			send_to_char("A wave of calm passes over you.\n\r",vch);

			if (vch->fighting || vch->position == POS_FIGHTING)
			  stop_fighting(vch,FALSE);

			af.type = sn;
  			af.level = level;
			af.duration = level/4;
			af.location = APPLY_HITROLL;
			
			if (!IS_NPC(vch))
			  af.modifier = -5;
			else
			  af.modifier = -2;
			
			af.bitvector = AFF_CALM;
			affect_to_char(vch,&af);

			af.location = APPLY_DAMROLL;
			affect_to_char(vch,&af);
		}
    }
}

void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;
   
    bless_num = skill_lookup("bless");
    curse_num = skill_lookup("curse"); 
    frenzy_num = skill_lookup("frenzy");

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of divine power.\n\r",ch);
 
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

		if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
			(IS_EVIL(ch) && IS_EVIL(vch)) ||
			(IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
		{
 		  send_to_char("You feel full more powerful.\n\r",vch);
		  spell_blood_lust(frenzy_num,level,ch,(void *) vch); 
		  spell_bless(bless_num,level,ch,(void *) vch);
		}

		else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
			 (IS_EVIL(ch) && IS_GOOD(vch)) )
		{
		  if (!is_safe_spell(ch,vch,TRUE))
		  {
				spell_curse(curse_num,level,ch,(void *) vch);
				act("$n screams in agony as a holy word is burned into $s body",
					vch,NULL,NULL,TO_NOTVICT);
				send_to_char("You are struck down!\n\r",vch);
				dam = dice(level,6);
				damage(ch,vch,dam,sn,DAM_HOLY);
		  }
		}

			else if (IS_NEUTRAL(ch))
		{
		  if (!is_safe_spell(ch,vch,TRUE))
		  {
			spell_curse(curse_num,level/2,ch,(void *) vch);
			act("$n screams in agony as a holy word is burned into $s body",
				vch,NULL,NULL,TO_NOTVICT);
			send_to_char("You are struck down!\n\r",vch);
			dam = dice(level,4);
			damage(ch,vch,dam,sn,DAM_HOLY);
   		  }
		}
    }  
    
    send_to_char("You feel drained.\n\r",ch);
    gain_exp( ch, -1 * number_range(1,10) * 10);
    ch->move = 0;
    ch->hit /= 2;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
    OBJ_DATA *obj;
    int iWear;

    if (check_dispel(level,victim,gsn_curse))
    {
	send_to_char("You feel better.\n\r",victim);
	act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
    }

    for ( iWear = 0; (iWear < MAX_WEAR && !found); iWear ++)
    {
	if ((obj = get_eq_char(victim,iWear)) == NULL)
	    continue;

        if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        {   /* attempt to remove curse */
	    if (!saves_dispel(level,obj->level,0))
	    {
		found = TRUE; 
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
	    	act("$p glows blue.",victim,obj,NULL,TO_CHAR);
		act("$p glows blue.",victim,obj,NULL,TO_ROOM);
	    }
	 }
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        {   /* attempt to remove curse */
            if (!saves_dispel(level,obj->level,0))
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
                act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
            }
         }
    }
}

void spell_blood_lust(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
		if (victim == ch)
		  send_to_char("You are already lust for blood.\n\r",ch);
		else
		  act("$N is already lusting for blood.",ch,NULL,victim,TO_CHAR);
		return;
    }

    if (is_affected(victim,skill_lookup("calm")))
    {
		if (victim == ch)
		  send_to_char("Why don't you just relax for a while?\n\r",ch);
		else
		  act("$N doesn't look like $e wants to fight anymore.",ch,NULL,victim,TO_CHAR);
		return;
    }
   
	/*apply affect*/
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 2;/*Changed for fast tick from level/3 alone - Spellsong*/
    af.modifier  = dice(1, 4) + level/20;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 6);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with `Rblood`w lust!\n\r",victim);
    act("$n gets a hungry look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

/***************
* ATTACK GROUP *
***************/

/*suggest level 1*/
void spell_mystic_hammer( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
   
	static const int dam_each[] = 
    {
	 1,	 2,  3,  4,  5,  6,	 7,  7,  7,  8,
	 8,  8,  8,  8,  9,  9,	 9,  9,  9, 10,
	10,	10, 10, 10, 11, 11,	11, 11, 11, 12,
	12,	12, 12, 12, 13, 13,	13, 13, 13, 14,
	14,	14, 14, 14, 15, 15,	15, 15, 15, 16,
	16,	16, 16, 18, 18, 18,	20, 20, 20, 22
    };
	
	int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 ); 
	

    if ( saves_spell( level, victim ) )
		dam /= 2;
    act("A huge hammer flies from $n's outstretched hand.",ch,NULL,NULL,TO_ROOM);
    damage( ch, victim, dam, sn, DAM_BASH );
    return;
}

void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
		victim = ch;
  
    if ( IS_GOOD(victim) )
    {
		act( "The gods protect $N.", ch, NULL, victim, TO_ROOM );
		return;
    }

    if ( IS_NEUTRAL(victim) )
    {
		act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
		return;
    }

    if (victim->hit > (ch->level * 2))
      dam = dice( 2, level )+level;
    else
      dam = UMAX(victim->hit, dice(2, level)+level);

    if ( saves_spell( level, victim ) )
		dam /= 2;

	if( dam > 150 )
		dam = 150;
    
	damage( ch, victim, dam, sn, DAM_HOLY );
    return;
}

/*opposite of dispel evil - suggest level 15 for both*/
void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
		victim = ch;
  
    if ( IS_EVIL(victim) )
    {
		act( "The gods protect $N.", ch, NULL, victim, TO_ROOM );
		return;
    }

    if ( IS_NEUTRAL(victim) )
    {
		act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
		return;
    }

    if (victim->hit > (ch->level * 2))
      dam = dice( 2, level )+ level;
    else
      dam = UMAX(victim->hit, dice(2, level))+level;

    if ( saves_spell( level, victim ) )
		dam /= 2;

	if( dam > 150 )
		dam = 150;
    
	damage( ch, victim, dam, sn, DAM_HOLY );
    return;
}

/*suggest level 25*/
void spell_wrath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
   
	static const int dam_each[] = 
    {
	 1,	 2,  3,  4,  5,  6,	 7,  8,  9, 10,
	11, 12, 13, 13, 13, 14,	14, 14, 15, 15,
	15,	16, 16, 16, 17, 17,	17, 18, 18, 18,
	19,	19, 19, 20, 20, 20,	21, 21, 21, 22,
	22,	22, 24, 24, 24, 26,	26, 26, 28, 28,
	28,	30, 30, 30, 32, 32,	32, 34, 34, 36
    };
	
	int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 ); 
	

    if ( saves_spell( level, victim ) )
		dam /= 2;
    act("$n calls down the wrath of the gods.",ch,NULL,NULL,TO_ROOM);
    damage( ch, victim, dam, sn, DAM_BASH );
    return;
}

/*suggest level 40*/
void spell_fury( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
   
	static const int dam_each[] = 
    {
	28,	28, 28, 28, 28, 28,	28, 28, 28, 28,
	28,	28, 28, 28, 28, 28,	28, 28, 28, 28,
	28,	28, 28, 28, 28, 28,	28, 28, 28, 28,
	28,	28, 28, 28, 28, 28,	28, 28, 28, 28,
	32,	36, 40, 40, 40, 40,	40, 45, 45, 45,
	45,	45, 50, 50, 50, 50,	50, 55, 55, 55,
	60,	60, 60, 60, 60, 65,	65, 65, 65, 65
    };
	
	int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 ); 
	

    if ( saves_spell( level, victim ) )
		dam /= 2;
    
	act( "$n calls down the fury of the Gods on $N!",ch ,NULL, victim, TO_ROOM );
	act( "$n calls down the fury of the Gods on you!",ch ,NULL, victim, TO_VICT );
    
	damage( ch, victim, dam, sn, DAM_HOLY );
    return;
}

/*suggest level 60*/
void spell_ray_of_truth( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
		victim = ch;
  
    if ( IS_GOOD(victim) )
    {
		act( "The gods are protecting $N.", ch, NULL, victim, TO_ROOM );
		return;
    }

    if ( IS_NEUTRAL(victim) )
    {
		act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
		return;
    }

    if (victim->hit > (ch->level * 4))
      dam = dice( 4, level )+(level/2);
    else
      dam = UMAX(victim->hit, dice(4, level)+(level/2));

    if ( saves_spell( level, victim ) )
		dam /= 2;

	if( dam > 300 )
		dam = 300;

	act( "$n sends a blinding ray of truth towards $N!",ch ,NULL, victim, TO_ROOM );
	act( "$n sends a blinding ray of truth towards you!",ch ,NULL, victim, TO_VICT );
    
	damage( ch, victim, dam, sn, DAM_HOLY );
    return;
}

/*suggest level 60*/
void spell_corruption( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
		victim = ch;
  
    if ( IS_EVIL(victim) )
    {
		act( "The gods are protecting $N.", ch, NULL, victim, TO_ROOM );
		return;
    }

    if ( IS_NEUTRAL(victim) )
    {
		act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
		return;
    }

    if (victim->hit > (ch->level * 4))
      dam = dice( 4, level )+(level/2);
    else
      dam = UMAX(victim->hit, dice(4, level)+(level/2));

    if ( saves_spell( level, victim ) )
		dam /= 2;

	if( dam > 300 )
		dam = 300;

	act( "$n sends a dark ray of corruption towards $N!",ch ,NULL, victim, TO_ROOM );
	act( "$n sends a dark ray of corruption towards you!",ch ,NULL, victim, TO_VICT );
    
	damage( ch, victim, dam, sn, DAM_HOLY );
    return;
}

/*suggest level 80*/
void spell_immolate(int sn, int level, CHAR_DATA *ch, void *vo)
{
    
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    /* first strike */

    act("A fire imp appears near $n and flies towards $N.",
        ch,NULL,victim,TO_ROOM);
    act("A fire imp appears near you and flies towards $N.",
		ch,NULL,victim,TO_CHAR);
    act("A fire imp appears near $n and flies towards you!",
		ch,NULL,victim,TO_VICT);  

    dam = dice(level,4);
    
	if (saves_spell(level,victim))
 		dam /= 2;

    damage(ch,victim,dam,sn,DAM_FIRE);
    
	last_vict = victim;
    
	level -= 6;   /* decrement damage */

    /* new targets */
    while (level > 0)
    {
		found = FALSE;
		for (tmp_vict = ch->in_room->people; 
			 tmp_vict != NULL; 
			 tmp_vict = next_vict) 
		{
		  next_vict = tmp_vict->next_in_room;
		  if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
		  {
			found = TRUE;
			last_vict = tmp_vict;
			
			act("The imp flies towards $n!",tmp_vict,NULL,NULL,TO_ROOM);
			act("The imp flies towards you!",tmp_vict,NULL,NULL,TO_CHAR);
			
			dam = dice(level,4);
			
			if (saves_spell(level,tmp_vict))
				dam /= 2;
			
			damage(ch,tmp_vict,dam,sn,DAM_FIRE);
			level -= 4;  /* decrement damage */
		  }
		}   /* end target searching loop */
		
		if (!found) /* no target found, hit the caster */
		{
		  if (ch == NULL)
     			return;

		  if (last_vict == ch) /* no double hits */
		  {
			act("The imp disappears in a puff of smoke.",ch,NULL,NULL,TO_ROOM);
			act("The imp disappears into your body.",ch,NULL,NULL,TO_CHAR);
			return;
		  }
		
		  last_vict = ch;

		  act("The imp flies out of control towards $n!",ch,NULL,NULL,TO_ROOM);
		  send_to_char("The imp singes you!\n\r",ch);
		  
		  dam = dice(level,6);
		  
		  if (saves_spell(level,ch))
			dam /= 3;
		  
		  damage(ch,ch,dam,sn,DAM_FIRE);
		  
		  level -= 4;  /* decrement damage */
		  
		  if (ch == NULL) 
			return;
		}
    /* now go back and find more targets */
    }
}

/*****************
* CREATION GROUP *
*****************/

void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = 5 + level;
    obj_to_room( mushroom, ch->in_room );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    return;
}

void spell_create_spring( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level+10;
    obj_to_room( spring, ch->in_room );
    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    return;
}

void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}

void spell_continual_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *light;

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}

void spell_feast(int sn, int level, CHAR_DATA *ch, void *vo)
{
	/*creates a feast enough for many people*/
	OBJ_DATA *roast;
	OBJ_DATA *juice;
	int x;

    for( x = 1; x < level/20 ; x++ )
	{
		roast = create_object( get_obj_index( OBJ_VNUM_ROAST ), 0 );
		obj_to_room( roast, ch->in_room );

		act( "$p appears out of thin air.", ch, roast, NULL, TO_ROOM );
		act( "$p appears out of thin air.", ch, roast, NULL, TO_CHAR );
	
		juice = create_object( get_obj_index( OBJ_VNUM_JUICE ), 0 );
		
		juice->timer = level/3;
		juice->value[0] = level/5;
		juice->value[1] = level/5;
		obj_to_room( juice, ch->in_room );

		act( "$p appears out of thin air.", ch, juice, NULL, TO_ROOM );
		act( "$p appears out of thin air.", ch, juice, NULL, TO_CHAR );
	
	}
    
	return;
}

/* Eris - old spells added for old potions already in the world
   02 September 2000 */

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
        victim = ch;
        send_to_char("The demons turn upon you!\n\r",ch);
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);

    if (victim != ch)
    {
        act("$n calls forth the demons of Hell upon $N!",
            ch,NULL,victim,TO_ROOM);
        act("$n has assailed you with the demons of Hell!",
            ch,NULL,victim,TO_VICT);
        send_to_char("You conjure forth the demons of hell!\n\r",ch);
    }
    dam = dice( level, 10 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE );
}



void spell_web( int sn, int level, CHAR_DATA *ch, void *vo )
{
send_to_char("Nothing happens.\n\r",ch);

}







/************************
* MYSTIC SKILL STUFFAGE *
************************/

/* NEW SKILLS:

  Stoke - Adds a flame effect temporarily to a weapon
  Divine - If using a divining rod, character can detect traps similiar to
           the thief skill
  Animal Form *optional if we dump shapeshifter* - allows mystic to turn into
         - animal forms based on level
  Butcher - Can butcher flagged mobs and make food and skins
*/

/* OLD SKILLS:

  Flail, Mace, Spear, Polearm, Whip, Riding, Recall, Wands, Scrolls, Staves,
  Lore, Meditation, Fast Healing, Second Attack, Dodge */

/*****************************
*SUGGESTED SKILL/SPELL LEVELS*
*****************************/
/*	Level 1 - wands, staffs, scrolls - ALL Classes ), Riding, Swimming, Recall
              Mace, Spear, Flail, Whip, Polearm, Patch Wounds, Mystic Hammer
	Level 2 - Mar, Change Sex
    Level 3 - Lore
	Level 4 - Create light
	Level 5 - Barkskin, Refresh
	Level 7 - Fly
	Level 8 - Create Water
	Level 9 - Butcher
	Level 10 - Geyser. Preservation
	Level 11 - Dispel Magic
	Level 12 - Chakra
	Level 13 - Water Breathing
	Level 14 - Meditation
	Level 15 - Dispel Evil
	Level 16 - Dispel Good
	Level 17 - Cancellation
	Level 18 - Awaken, Create Food
	Level 19 - Bless
	Level 20 - Faerie Fire, Flamestrike
	Level 21 - Shift
	Level 22 - Bind_Wounds
	Level 23 - Abuse
	Level 24 - Protect good/evil
	Level 25 - Wrath, Mind Shield
	Level 26 - Blindness
	Level 27 - Cure Blindness
	Level 28 - Aura
	Level 29 - Create Spring
	Level 30 - Divine, EarthQuake
	Level 32 - Remove Curse, Curse
	Level 33 - Blood_lust
	Level 34 - Smoke_Form
	Level 35 - Energy Drain
	Level 36 - Faerie Fog
	Level 37 - Injure, Mend Wounds
	Level 38 - Deflect
	Level 39 - encase
	Level 40 - Fury, Ice Shield
	Level 41 - control Weather
	Level 42 - Dodge, Word of Recall
	Level 43 - Weaken, Psychic Shield
	Level 44 - Stoke
	Level 45 - Sanctuary
	Level 46 - Repair Wounds
	Level 47 - Harm
	Level 48 - Control Weather
	Level 49 - Call Lightning
	Level 50 - Second Attack, Fire Shield
	Level 51 - Slow
	Level 52 - Revelation
	Level 53 - Feast
	Level 54 - Fast Healing
	Level 55 - Enslave
	Level 56 - Poison, Cure Poison
	Level 57 - Summon
	Level 58 - Vision
	Level 59 - Slow
	Level 60 - Ray of Truth, Corruption
	Level 61 - Sand Blast, Mass Heal
	Level 62 - Wind Walk, Plague
	Level 63 - Cure Disease
	Level 65 - Calm
	Level 66 - Energy Transferance
	Level 67 - Stoneskin
	Level 68 - Tornado
	Level 69 - Restoration, Inflict
	Level 70 - Summon Earth Elemental , *Animal Form*
	Level 72 - Spirit Shield
	Level 75 - Holy Word
	Level 78 - Foresight
	Level 79 - Regeneration
	Level 80 - Immolate
	Level 85 - Typhoon
	Level 90 - Avenge
	Level 95 - Succor
	
 */

 /**************************
* SUGGESTED DEFAULT GROUPS*
**************************/

/*
 * HEALING, PROTECTION, ATTACK, HARM, BENEDICTION, 
 *
 * OTHER DEFAULT SKILLS:
 * 1 WEAPON, LORE, MEDITATION
 *
 */
