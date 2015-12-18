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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_captivate	);

/*
 * Local functions.
 */  /*moved to merc.h for outside use*/
/*void	say_spell	args( ( CHAR_DATA *ch, int sn ) );*/



/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;
/*	return 0; Delete this line to re-enable slots -Akira */
    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}


/* Eris add - 24 April 2000 with Spellsong rehack :) ...hehe*/
void spell_imprint( int sn, int level, CHAR_DATA *ch, OBJ_DATA *obj )
{
    int       sp_slot, i, mana;
    char      buf[ MAX_STRING_LENGTH ];
	 	buf[0] = '\0'; 	sp_slot = 1; 	 	if ( obj == NULL ) 	{ 		bug( "Null pointer passed to spell_imprint in magic.c.", sp_slot ); 		return; 	} 	 	if (skill_table[sn].spell_fun == spell_null )
    {
		send_to_char("That is not a spell.\n\r",ch);
		return;
    }

    /* counting the number of spells contained within */

    for (sp_slot = i = 1; i < 4; i++) 
	{ 		if (obj->value[i] != -1)
			sp_slot++; 	}

    /*check to see if trying to add too many spells*/ 	if (sp_slot > 3)
    {
		act ("$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR);
		return;
    }

   /* scribe/brew costs 4 times the normal mana required to cast the spell */

    mana = 4 * mana_cost(ch, sn, level);
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
		send_to_char( "You don't have enough mana.\n\r", ch );
		return;
    }
      

    if ( number_percent( ) > ch->pcdata->learned[sn] )
    {
		send_to_char( "You lost your concentration.\n\r", ch );
		ch->mana -= mana / 2;
		return;
    }

    /* executing the imprinting process */
    ch->mana -= mana;
    obj->value[sp_slot] = sn;

    /* Making it successively harder to pack more spells into potions or 
       scrolls - JH */ 

    switch( sp_slot )
    {
   
		default:
		bug( "sp_slot has more than %d spells.", sp_slot );
		return;

		case 1:
			if ( number_percent() > 80 )
			{ 
			  sprintf(buf, "The magic enchantment has failed --- %s vanishes.\n\r", obj->short_descr );
			  send_to_char( buf, ch );
			  extract_obj( obj );
			  return;
			}     
		break;
		case 2:
			if ( number_percent() > 25 )
			{ 
			  sprintf(buf, "The magic enchantment has failed --- %s vanishes.\n\r", obj->short_descr );
			  send_to_char( buf, ch );
			  extract_obj( obj );
			  return;
			}     
		break;

		case 3:
			if ( number_percent() > 10 )
			{ 
			  sprintf(buf, "The magic enchantment has failed --- %s vanishes.\n\r", obj->short_descr );
			  send_to_char( buf, ch );
			  extract_obj( obj );
			  return;
			}     
		break;
    } 
  

    /* labeling the item */

/*    sprintf ( buf, "a %s of ", obj->short_descr ); */

	if(obj->item_type==ITEM_POTION)
		sprintf( buf, "a potion of ");
	else
		sprintf(buf, "a scroll of ");
    for (i = 1; i <= sp_slot ; i++)
      if (obj->value[i] != -1)
      {
		strcat (buf, skill_table[obj->value[i]].name);
			(i != sp_slot ) ? strcat (buf, ", ") : strcat (buf, "") ; 
      }
    free_string (obj->short_descr);
    obj->short_descr = str_dup(buf);
	
    sprintf( buf, "%s %s", obj->name, obj->short_descr );
    free_string( obj->name );
    obj->name = str_dup( buf );        
	
    send_to_char( "You have imbued a new spell to it!" , ch );

    return;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0'; /*spellsong*/ 	buf2[0]	= '\0'; /*spellsong*/ 
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act( ch->class==rch->class ? buf : buf2, ch, NULL, rch, TO_VICT );
    }

    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim )
{
    int save;

    save = 50 + ( victim->level - level - victim->saving_throw ) * 5;
    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/2;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* RT save for dispels */

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 5;  
      /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if (!saves_dispel(dis_level,af->level,af->duration))
                {
                    affect_strip(victim,sn);
        	    if ( skill_table[sn].msg_off )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }
		    return TRUE;
		}
		else
		    af->level--;
            }
        }
    }
    return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
    if (ch->level + 2 == level)
	return 1000;
    return UMAX(min_mana,(100/(2 + ch->level - level)));
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;

    /*
     * Charmed NPC's can't cast spells, but others can.
     */
    if ( IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
	return; 

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Cast which what where?\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup( arg1 ) ) <= 0
    || ( !IS_NPC(ch) && ch->level < skill_table[sn].skill_level[ch->class] ))
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }
  
    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) );

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/


	if ( !IS_NPC(ch) )
	{

            if (is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return; 
	    }
	}

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",
		ch );
	    return;
	}

	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	break;
    }
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char( "You don't have enough mana.\n\r", ch );
	return;
    }

	if( !IS_NPC(ch) && lacks_components(ch, sn,'s',FALSE))
	{
		send_to_char("You seem to be lacking something to cast this spell.\n\r",ch);
		return;
	}
	if(!IS_NPC(ch))
		lacks_components(ch,sn,'s',TRUE);

    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
	say_spell( ch, sn );
      
    WAIT_STATE( ch, skill_table[sn].beats );
      
    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[sn] )
    {
	send_to_char( "You lost your concentration.\n\r", ch );
	check_improve(ch,sn,FALSE,1);
	ch->mana -= mana / 2;
    }
    else
    {
	ch->mana -= mana;
	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo );
	check_improve(ch,sn,TRUE,1);
	if ( !IS_NPC(ch))
          ch->exp += mana;

    }

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&   victim->master != ch)
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe_spell(ch,victim,FALSE) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo );

    

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Spell functions.
 */


void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
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
	send_to_char(
	    "The mayor does not allow charming in the city limits.\n\r",ch);
	return;
    }

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
	act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}




void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   level < victim->level
    ||   saves_spell( level, victim ) )
	return;

    af.type      = sn;
    af.level     = level;
    af.duration  = level+10/2; /*Changed for fast tick from level alone - Spellsong*/;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    /*routine changed by spellsong for integration with trap effects*/
	if ( IS_AWAKE(victim) )
    {
		send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
		act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
		
		if( ch == victim)
			send_to_char( "You go to sleep!\n\r", ch );
		
		victim->position = POS_SLEEPING;
    }

    return;
}

void do_captivate(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
    char *b_sleep;
    int temp, count = 0;


	arg1[0] = '\0';
	arg2[0] = '\0';

    b_sleep = one_argument( argument, arg1 );
    one_argument( b_sleep, arg2 );

	if(ch->race != 8)
	{
		send_to_char("What do you think you are? A nymph?\n\r", ch);
		return;
	}

	if(ch->racial_abil != 0)
	{
		send_to_char("You must wait until your powers return.\n\r", ch);
		return;
	}

	if(arg1[0] == '\0')
	{
		send_to_char("Whom do you wish to beguile?\n\r", ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
                send_to_char( "They aren't here.\n\r", ch );
                return;
        } 

	if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
	{
        	send_to_char(
            	"Such matters are best kept outside of city limits.\n\r",ch);
        	return;
	}

	if (ch == victim)
	{
		send_to_char("Yourself? You're kidding, right?\n\r", ch);
		return;
	}

	ch->racial_abil = 18; // Eris - was 36 but felt that was too slow

	for(temp = ch->level; temp != 0; temp = temp / 10)
		count++;

	ch->racial_abil = ch->racial_abil / count;

	if ( arg2[0] != 's')    // Charm them.
	{	

    		if ( IS_AFFECTED(victim, AFF_CHARM)
    		||   IS_AFFECTED(ch, AFF_CHARM)
    		||   ch->level < victim->level
    		||   IS_SET(victim->imm_flags,IMM_CHARM)
    		||   saves_spell( (ch->level + 5), victim ) )
    		{
			send_to_char("Something went wrong with your magics.\n\r", ch);
			act( "$n is trying to pull her usual nymph tricks on you. Watch out.\n\r", ch, NULL, victim, TO_VICT );
			return;
   		}



    		if ( victim->master )
        		stop_follower( victim );
    		add_follower( victim, ch );
    		victim->leader = ch;
    		af.type      = skill_lookup("charm person");
    		af.level     = (ch->level + 5);
    		af.duration  = number_fuzzy( ch->level / 3 );
    		af.level     = (ch->level + 5);
    		af.location  = 0;
    		af.modifier  = 0;
    		af.bitvector = AFF_CHARM;
    		affect_to_char( victim, &af );
    		act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    		act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
	} //end charm

	else // the nymph is putting them to sleep.
	{

  
		if ( IS_AFFECTED(victim, AFF_SLEEP)
		||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
		||   ch->level < victim->level
		||   saves_spell( ch->level, victim ) )
		{
			send_to_char("Something went wrong with your magics.\n\r", ch);
			act( "$n is trying to pull her usual nymph tricks on you. Watch out.", ch, NULL, victim, TO_VICT );
			return;
  		}
    
		af.type      = skill_lookup("sleep");
		af.level     = ch->level;
		af.duration  = (ch->level + 10) / 2; //Changed for fast tick from level alone-SS 
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_SLEEP;
		affect_join( victim, &af );

		/*routine changed by spellsong for integration with trap effects*/
        	if ( IS_AWAKE(victim) )
		{
                	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
                	act( "$n is coaxed into slumber by nymphsh means.", victim, NULL, NULL, TO_ROOM );
               		victim->position = POS_SLEEPING;
		}

	} // end sleep
    return;
}

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if (saves_spell(level, victim))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }

    /* begin running through the spells */ 

    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("calm")))
    {
        found = TRUE;
        act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("fire shield")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("haste")))
    {
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("ice shield")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("protection")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim,skill_lookup("sanctuary")))
    {
	REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("web")))
    {
        act("The webs around $n disolve.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim) && 
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
        (IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed, try dispel magic.\n\r",ch);
	return;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */
 
    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("calm")))
    {
	found = TRUE;
	act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel more like yourself again.\n\r",victim);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
  
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
	act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("protection")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("web")))
    {
        act("The webs around $n disolve.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

/* Akira June 2000. Should be pretty self explainitory...or maybe not */

bool  lacks_components (CHAR_DATA *ch, int sn, char key, bool extract) 
{
	char *s;

	if(!skill_table[sn].component)
		return FALSE;

	s=skill_table[sn].component;
	while(s)
	{
		char *ptr;
		char word[MAX_STRING_LENGTH];
		sh_int vnum;	
		OBJ_DATA *obj;

		s=one_argument(s,word);

		if(word[0]=='\0')
			break;
/*		send_to_char(word,ch); */
		if(word[0]!=key)
			continue;
		ptr=word;
		if(!is_number(++ptr))
		{
			char bugF[MAX_STRING_LENGTH];
			sprintf(bugF,"Bad component string: %d", sn);
			bug(bugF,0);
			return TRUE; /*Make it stop*/
		}

		vnum=atoi(ptr);

		for(obj=ch->carrying;obj;obj=obj->next)
			if(obj->pIndexData->vnum==vnum)
				break;
		if(!obj)
			return TRUE;
		if(!extract)
			continue;
		act("In a blast of magical power, $P disappears.",ch,NULL,obj,TO_CHAR);
		extract_obj(obj);
	}
	return FALSE; /*appearantly not*/
}


//Aristean spells

void spell_avenging_angel(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim=(CHAR_DATA*)vo;
	AFFECT_DATA af;
	sh_int dam;
	
	if(!IS_EVIL(victim) || !IS_GOOD(ch))
	{
		send_to_char("The heavens do not aid you in this fight.\n\r",ch);
		return;
	}

	send_to_char("You summon an angel to strike down the evil infront of you.\n\r",ch);
	act("$n summons an angel from the heavens to strike down evil.",ch, NULL,NULL,TO_ROOM);

	if(!is_affected(victim,sn))
	{
  	   af.type      = sn; 
  	   af.level     = level;
  	   af.duration  = ch->level;
  	   af.location  = APPLY_AC;
 	   af.modifier  = ch->level;
 	   af.bitvector = 0;
 	   affect_to_char( victim, &af );			
	}

	dam= ch->level/100.0 * (ch->alignment)/3.0;

        damage( ch, victim, dam, sn, DAM_HOLY );
}

void spell_fasting( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *tar=(CHAR_DATA *)vo;
	if(tar==ch)
		send_to_char("Lutessa defers your need for food and water.\n\r",ch);
	else
	{
    act( "You grant $N respite from mortal needs using Lutessa's power.",ch, NULL, tar, TO_CHAR );
    act( "$n grants you respite from food and water using Lutessa's power.",ch, NULL,tar, TO_VICT);
	}

	if(IS_NPC(tar))
		return;
	ch->pcdata->condition[COND_THIRST] = 	ch->pcdata->condition[COND_FULL] = 200;
}

void spell_faith( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
                send_to_char("You are already steeped in faith.\n\r",ch);
                return;
    }

	send_to_char("You place your faith in the hands of Lutessa.\n\r",ch);

    af.type      = sn;
    af.level     = level;
    af.duration  = level*2;
    af.location  = APPLY_AC;
    af.modifier  = -50-level;   
    af.bitvector = 0;
    affect_to_char( ch, &af );
                	
}
