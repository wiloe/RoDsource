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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "religion.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_stand		);


char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};



/*
 * Local functions.
 */
int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
bool    check_web       args( ( CHAR_DATA *ch) );


void move_char( CHAR_DATA *ch, int door, bool follow )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
	
	buf[0] = '\0'; /*spellsong add*/

    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    if ( MOUNTED(ch) )
    {
       if (MOUNTED(ch)->position < POS_FIGHTING)
       {
           send_to_char("Your mount must be standing.\n\r", ch);
            return; 
        }
       if (!mount_success(ch, MOUNTED(ch), FALSE))
       {
            send_to_char("Your mount stubbornly refuses to go that way.\n\r", ch);
            return;
        }
    }
  
    if (ch->at_obj!=NULL) /*move off furniture when you move*/
       char_off_obj(ch);

    if (IS_AFFECTED(ch, AFF_WEB))
	{
	if (check_web(ch))
	    {
	     act( "You break free of the webs and fall down from the effort.", ch, NULL, NULL, TO_CHAR );
	     act( "$n breaks free of the webs and falls downs from the effort.", ch, NULL,NULL,TO_ROOM);
	     WAIT_STATE( ch, 5 );
	     ch->move -= 5;
	     affect_strip(ch,skill_lookup("web"));
	     REMOVE_BIT( ch->affected_by, AFF_WEB);
	     ch->position = POS_RESTING;
	     return;
	    }
	else
	    {
	     act( "You struggle against the webs.", ch, NULL, NULL, TO_CHAR );
             act( "$n struggles weakly in the webs.", ch, NULL,NULL,TO_ROOM);
	     WAIT_STATE( ch, 5 );
             ch->move -= 5;
	     return;
    	    }
	/* The end? */
	}


    if(IS_AFFECTED(ch,AFF_SEVERED))
    {
        send_to_char("Move without legs? How?\n\r",ch);
        return;
    }




    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room))
    {
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
	&& IS_SET(pexit->exit_info, EX_HIDDEN))
	{
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
        return;
	}

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   !IS_AFFECTED(ch, AFF_PASS_DOOR) )
    {
	act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	act( "$n tries unsuccessfully to walk through the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
	&& IS_SET(pexit->exit_info, EX_PASSPROOF)
        && IS_AFFECTED(ch, AFF_PASS_DOOR) )

	{
	act( "A magical force prevents you from passing through the $d.", ch, NULL, pexit->keyword, TO_CHAR );
	act( "$n looks puzzled by not passing through the $d.", ch, NULL, pexit->keyword, TO_ROOM );
        return;
	}

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( room_is_private( to_room ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) )
    {
	int iClass, iGuild;
	int move;

	if(is_rel_protected(ch,to_room->vnum))
	{
		send_to_char("A powerful force keeps you from passing into sacred grounds.\n\r",ch);
		return;
	}

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)	
	    {
	    	if ( iClass != ch->class
	    	&&   to_room->vnum == class_table[iClass].guild[iGuild] )
	    	{
		    send_to_char( "You aren't allowed in there.\n\r", ch );
		    return;
		}
	    }
	}
	/*allow you to drop yourself on your head, but not move out of a sect_air room*/
	if ( in_room->sector_type == SECT_AIR
	||   (to_room->sector_type == SECT_AIR && door == DIR_UP))
	{
            if ( MOUNTED(ch) )
            {
               if( !IS_AFFECTED(MOUNTED(ch), AFF_FLYING) )
               {
                   send_to_char( "Your mount can't fly.\n\r", ch );
                    return;
                }
           }
           else
           {
               if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
               {
                   send_to_char( "You can't fly.\n\r", ch );
                   return;
               }
	    }
	}

        if ( (in_room->sector_type == SECT_WATER_NOSWIM
        ||    to_room->sector_type == SECT_WATER_NOSWIM )
        &&    (MOUNTED(ch) && IS_AFFECTED(MOUNTED(ch), AFF_FLYING) ))
        {
            sprintf( buf,"You can't take your mount there.\n\r");
            send_to_char(buf, ch);
            return;
        } 

	if (( in_room->sector_type == SECT_WATER_NOSWIM
	||    to_room->sector_type == SECT_WATER_NOSWIM )
  	&&    !IS_AFFECTED(ch,AFF_FLYING))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}

	/*trap check here and deposit character back in room*/
	if( trap_movement( ch->in_room->contents, ch, TRAP_CHECK(door) ) )
		return;

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

       move /= 2;  /* i.e. the average */


       if ( !MOUNTED(ch) )
       {
           /* conditional effects */
           if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
               move /= 2;

	   if (IS_AFFECTED(ch,AFF_SLOW))/*spellsong add*/
	       move *= 2;

           if ( ch->move < move )
           {
               send_to_char( "You are too exhausted.\n\r", ch );
               return;
           }
       }
       else
       {
           if (IS_AFFECTED( MOUNTED(ch), AFF_FLYING)
           || IS_AFFECTED( MOUNTED(ch), AFF_HASTE))
           {
                move /= 2;
           }

           if ( MOUNTED(ch)->move < move )
           {
                send_to_char( "Your mount is too exhausted.\n\r", ch );
                return;
           }
        }
 
        WAIT_STATE( ch, 1 );

	// Divinos -- May 3rd
	// Tervadi at full health have no movememnt cost. 
        if (!MOUNTED(ch))
	{
		if( (ch->race == race_lookup("tervadi")) &&
		    (ch->hit == ch->max_hit) )
			move = 0;

             		ch->move -= move;
	}
        else
             MOUNTED(ch)->move -= move;
     }
 
     if (RIDDEN(ch))
     {
        CHAR_DATA *rch;
        rch = RIDDEN(ch);
 
        if (!mount_success(rch, ch, FALSE))
        {
           act( "Your mount escapes your control, and leaves $T.", rch, NULL, dir_name[door], TO_CHAR );
            if (RIDDEN(ch))
                ch = RIDDEN(ch);
        }
        else
	{
            send_to_char("You steady your mount.\n\r", rch);
	    return;
	}
    }

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
    {
       if (MOUNTED(ch))
       {
           if( !IS_AFFECTED(MOUNTED(ch), AFF_FLYING) )
               sprintf(buf, "$n leaves $T, riding on %s.", MOUNTED(ch)->short_descr);
            else
               sprintf(buf, "$n soars $T, on %s.", MOUNTED(ch)->short_descr);
            act( buf, ch, NULL, dir_name[door], TO_ROOM_MOVE );
        }
       else
       {
           act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM_MOVE );
        }
    }

    char_from_room( ch );
    char_to_room( ch, to_room );
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
    {
       if(!MOUNTED(ch))
       {
           act( "$n has arrived.", ch, NULL, NULL, TO_ROOM_MOVE );
       }
       else
       {
           if ( !IS_AFFECTED(MOUNTED(ch), AFF_FLYING) )
               act( "$n has arrived, riding on $N.", ch, NULL, MOUNTED(ch), TO_ROOM_MOVE );
           else
               act( "$n soars in, riding on $N.", ch, NULL, MOUNTED(ch), TO_ROOM_MOVE );
        }
    }

    if (MOUNTED(ch))
    {
        char_from_room( MOUNTED(ch) );
        char_to_room( MOUNTED(ch), to_room );
    }
    do_look( ch, "auto" );

    if (in_room == to_room) /* no circular follows */
	return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
	&&   fch->position < POS_STANDING)
	    do_stand(fch,"");

	if ( fch->master == ch && fch->position == POS_STANDING )
	{

	    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
	    {
		act("You can't bring $N into the city.",
		    ch,NULL,fch,TO_CHAR);
		act("You aren't allowed in the city.",
		    fch,NULL,NULL,TO_CHAR);
		return;
	    }

	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door, TRUE );
	}
    }
    mprog_entry_trigger( ch );
    mprog_greet_trigger( ch );
    return;
}



void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE );
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You can't reach the door from your mount.\n\r", ch);
        return;
    }
    
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch (obj->item_type) /*Portals 04141999*/
	 {
		 case ITEM_CONTAINER:
		 {
			 /* 'open object' */
			 if ( !IS_SET(obj->value[1], CONT_CLOSED) )
				{ send_to_char( "It's already open.\n\r",      ch ); return; }
			 if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
				{ send_to_char( "You can't do that.\n\r",      ch ); return; }
			 if ( IS_SET(obj->value[1], CONT_LOCKED) )
				{ send_to_char( "It's locked.\n\r",            ch ); return; }

			 REMOVE_BIT(obj->value[1], CONT_CLOSED);
			 send_to_char( "Ok.\n\r", ch );
			 act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
			 
			 /*trap check here for open object - Spellsong*/
			 trap_open( ch->in_room->contents, ch, TRAP_OBJECT );
			 
			 return;
		 }
		 break;
	case ITEM_PORTAL:
	 {
	    if (!IS_SET(obj->value[1], EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED))
	    {
		send_to_char("It's already open.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED))
	    {
		send_to_char("It's locked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_CLOSED);
	    act("You open $p.",ch,obj,NULL,TO_CHAR);
	    act("$n opens $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}
	default:
	send_to_char("You can't do that.\n\r",ch);
	 return;
      }
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
		/* 'open door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];
		if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
			{ send_to_char( "It's already open.\n\r",      ch ); return; }
		if (  IS_SET(pexit->exit_info, EX_LOCKED) )
			{ send_to_char( "It's locked.\n\r",            ch ); return; }

		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
		send_to_char( "Ok.\n\r", ch );

		/* open the other side */
		if ( ( to_room   = pexit->u1.to_room            ) != NULL
		&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room )
		{
			CHAR_DATA *rch;

			REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
			for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
			act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
		}
		/*the door opens but is still trapped - Spellsong Trap Check*/
		trap_door( ch->in_room->contents, ch, TRAP_CHECK( door ) );
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You can't reach the lock from your mount.\n\r", ch);
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	switch (obj->item_type)
	 {
	  case ITEM_CONTAINER:
		if ( IS_SET(obj->value[1], CONT_CLOSED) )
		 { send_to_char( "It's already closed.\n\r",    ch ); return; }
		if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
		    { send_to_char( "You can't do that.\n\r",      ch ); return; }

		SET_BIT(obj->value[1], CONT_CLOSED);
		send_to_char( "Ok.\n\r", ch );
		act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
		return;
	  break;
	case ITEM_PORTAL:
	 if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's already closed.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_CLOSED);
	    act("You close $p.",ch,obj,NULL,TO_CHAR);
	    act("$n closes $p.",ch,obj,NULL,TO_ROOM);
	    return;
	  break;
	 default:
	   send_to_char("You can't do that.\n\r",ch);
	   break;
	 }
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	switch (obj->item_type)
	 {
	 case ITEM_CONTAINER:
		if ( !IS_SET(obj->value[1], CONT_CLOSED) )
		    { send_to_char( "It's not closed.\n\r",        ch ); return; }
		if ( obj->value[2] < 0 )
		    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
		if ( !has_key( ch, obj->value[2] ) )
		    { send_to_char( "You lack the key.\n\r",       ch ); return; }
		if ( IS_SET(obj->value[1], CONT_LOCKED) )
		    { send_to_char( "It's already locked.\n\r",    ch ); return; }

		SET_BIT(obj->value[1], CONT_LOCKED);
		send_to_char( "*Click*\n\r", ch );
		act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
		return;
	 break;
	 case ITEM_PORTAL:
	  if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0 )
	    {
		send_to_char("It can't be locked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already locked.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_LOCKED);
	    act("You lock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n locks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	   break;
	 default:
	   send_to_char("You can't do that.\n\r",ch);
	   break;
	  }
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	switch (obj->item_type)
	{
	 case ITEM_CONTAINER:

		if ( !IS_SET(obj->value[1], CONT_CLOSED) )
		    { send_to_char( "It's not closed.\n\r",        ch ); return; }
		if ( obj->value[2] < 0 )
		    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
		if ( !has_key( ch, obj->value[2] ) )
		    { send_to_char( "You lack the key.\n\r",       ch ); return; }
		if ( !IS_SET(obj->value[1], CONT_LOCKED) )
		    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		send_to_char( "*Click*\n\r", ch );
		act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
		return;
	      break;
	 case ITEM_PORTAL:
	   if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already unlocked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	   break;
	  default:
	   send_to_char("You can't do that.\n\r",ch);
	   break;
	 }
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You can't pick locks while mounted.\n\r", ch);
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_pick_lock] )
    {
		send_to_char( "You failed.\n\r", ch);
		check_improve(ch,gsn_pick_lock,FALSE,2);
		/*ruh roh...trap is triggered for failed lock pick - spellsong*/
		
		/*object trap*/
		if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
		{
			trap_open( ch->in_room->contents, ch, TRAP_OBJECT );
			return;
		}

		/*door check*/
		if ( ( door = find_door( ch, arg ) ) >= 0 )
		{
			trap_door( ch->in_room->contents, ch, TRAP_CHECK( door ) );
			return;
		}
		
		return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	{
		send_to_char( "You failed.\n\r",             ch ); 
		/*evil bastard spellsong - autotrap trigger*/
		trap_open( ch->in_room->contents, ch, TRAP_OBJECT );
		return; 
	}

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	check_improve(ch,gsn_pick_lock,TRUE,2);
	act( "$n picks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
		/* 'pick door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];
		if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
			{ send_to_char( "It's not closed.\n\r",        ch ); return; }
		if ( pexit->key < 0 && !IS_IMMORTAL(ch))
			{ send_to_char( "It can't be picked.\n\r",     ch ); return; }
		if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
			{ send_to_char( "It's already unlocked.\n\r",  ch ); return; }
		if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
		{ 
			send_to_char( "You failed.\n\r",             ch ); 
			/*evil bastard spellsong - autotrap trigger*/
			trap_door( ch->in_room->contents, ch, TRAP_CHECK( door ) );
			return; 
		}

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		send_to_char( "*Click*\n\r", ch );
		act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
		check_improve(ch,gsn_pick_lock,TRUE,2);

		/* pick the other side */
		if ( ( to_room   = pexit->u1.to_room            ) != NULL
		&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room )
		{
			REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
		}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
   argument= one_argument(argument,arg); /*to check for the word on*/
  if (arg[0] !='\0'){
      if (!str_cmp(arg, "on"))          /*Strip the word on out of it*/
	  argument=one_argument(argument,arg);
      obj=get_obj_list(ch,arg,ch->in_room->contents);  /*find it in the room*/
      if (obj==NULL){
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
       }
       if (!IS_STANDANY(obj->furn_flags)){
	  send_to_char("You can't stand on that.\n\r",ch);
	  return;
       }
     if (ch->at_obj !=NULL)
	 char_off_obj(ch);

    /*Sit on*/
     switch (ch->position)
    {
      case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
	send_to_char( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up on $p.", ch, obj, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	char_at_obj(ch,obj);
	break;

    case POS_RESTING:
    case POS_SITTING:
	send_to_char( "You stand up.\n\r", ch );
	act( "$n stands up on $p.", ch, obj, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	char_at_obj(ch,obj);
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are fighting!\n\r", ch );
	break;

 }
    return;
   }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }

	send_to_char( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING:
    case POS_SITTING:
	send_to_char( "You stand up.\n\r", ch );
	act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  if (MOUNTED(ch))
  {
        send_to_char("You can't rest while mounted.\n\r", ch);
        return;
  }

  if (RIDDEN(ch))
  {
        send_to_char("You can't rest while being ridden.\n\r", ch);
        return;
  }

  argument= one_argument(argument,arg); /*to check for the word on*/
  if (arg[0] !='\0'){
	if (!str_cmp(arg, "on"))          /*Strip the word on out of it*/
	  argument=one_argument(argument,arg);
      obj=get_obj_list(ch,arg,ch->in_room->contents);  /*find it in the room*/
      if (obj==NULL){
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
       }
       if (!IS_RESTANY(obj->furn_flags)){
	  send_to_char("You can't rest on that.\n\r",ch);
	  return;
       }
     if (ch->at_obj !=NULL)
	 char_off_obj(ch);

    /*Sit on*/
     switch (ch->position)
    {
     case POS_SLEEPING:
	if ( IS_AFFECTED(ch,AFF_SLEEP))
	 { send_to_char("You can't wake up!\n\r",ch); return; }
	send_to_char( "You wake up and start resting.\n\r", ch );
	act ("$n wakes up and starts restingon $p.",ch,obj,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	char_at_obj(ch,obj);
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	send_to_char( "You rest.\n\r", ch );
	act( "$n sits down and rests on $p.", ch, obj, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	char_at_obj(ch,obj);
	break;

    case POS_SITTING:
	send_to_char("You rest.\n\r",ch);
	act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	char_at_obj(ch,obj);
	break;

    case POS_FIGHTING:
	send_to_char( "You are too busy fighting!\n\r", ch );
	break;


    }
    return;
   }
    /*Rest*/
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch,AFF_SLEEP))
	 { send_to_char("You can't wake up!\n\r",ch); return; }
	send_to_char( "You wake up and start resting.\n\r", ch );
	act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	send_to_char( "You rest.\n\r", ch );
	act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	send_to_char("You rest.\n\r",ch);
	act("$n rests.",ch,NULL,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  if (MOUNTED(ch))
  {
      send_to_char("You can't sit while mounted.\n\r", ch);
      return;
  }

  if (RIDDEN(ch))
  {
      send_to_char("You can't sit while being ridden.\n\r", ch);
      return;
  }

  argument= one_argument(argument,arg); /*to check for the word on*/
  if (arg[0] !='\0'){
      if (!str_cmp(arg, "on"))          /*Strip the word on out of it*/
	  argument=one_argument(argument,arg);
      obj=get_obj_list(ch,arg,ch->in_room->contents);  /*find it in the room*/
      if (obj==NULL){
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
       }
       if (!IS_SITANY(obj->furn_flags)){
	  send_to_char("You can't sit on that.\n\r",ch);
	  return;
       }
     if (ch->at_obj !=NULL)
	 char_off_obj(ch);

    /*Sit on*/
     switch (ch->position)
    {
	case POS_SLEEPING:
	    if ( IS_AFFECTED(ch,AFF_SLEEP))
	     { send_to_char("You can't wake up!\n\r",ch); return; }
	    act("You wake up and sit on $p.\n\r",ch,obj,NULL,TO_CHAR);
	    act("$n wakes and sits up on $p.\n\r",ch,obj,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    char_at_obj(ch, obj);
	    break;
	case POS_RESTING:
	    send_to_char("You stop resting.\n\r",ch);
	    act("You sit on $p\n\r",ch,obj,NULL,TO_CHAR);
	    ch->position = POS_SITTING;
	    char_at_obj(ch,obj);
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_FIGHTING:
	    send_to_char("Maybe you should finish this fight first?\n\r",ch);
	    break;
	case POS_STANDING:
	    send_to_char("You sit down.\n\r",ch);
	    act("$n sits down on $p.\n\r",ch,obj,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    char_at_obj(ch,obj);
	    break;
    }
    return;
   }
   /*Sit*/
     switch (ch->position)
    {
	case POS_SLEEPING:
	    if ( IS_AFFECTED(ch,AFF_SLEEP))
	     { send_to_char("You can't wake up!\n\r",ch); return; }
	    send_to_char("You wake up.\n\r",ch);
	    act("$n wakes and sits up.\n\r",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    break;
	case POS_RESTING:
	    send_to_char("You stop resting.\n\r",ch);
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_FIGHTING:
	    send_to_char("Maybe you should finish this fight first?\n\r",ch);
	    break;
	case POS_STANDING:
	    send_to_char("You sit down.\n\r",ch);
	    act("$n sits down on the ground.\n\r",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    break;
    }
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  if (MOUNTED(ch))
  {
      send_to_char("You can't sleep while mounted.\n\r", ch);
      return;
  }

  if (RIDDEN(ch))
  {
      send_to_char("You can't sleep while being ridden.\n\r", ch);
      return;
  }

  argument= one_argument(argument,arg); /*to check for the word on*/
  if (arg[0] !='\0'){
	 if (!str_cmp(arg, "on"))          /*Strip the word on out of it*/
	  argument=one_argument(argument,arg);
      obj=get_obj_list(ch,arg,ch->in_room->contents);  /*find it in the room*/
      if (obj==NULL){
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
       }
       if (!IS_SLEEPANY(obj->furn_flags)){
	  send_to_char("You can't sleep on that.\n\r",ch);
	  return;
       }
     if (ch->at_obj !=NULL)
	 char_off_obj(ch);

    /*Sleep on*/
     switch (ch->position)
    {
     case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
	send_to_char( "You go to sleep.\n\r", ch );
	act( "$n goes to sleep on $p.", ch, obj, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	char_at_obj(ch,obj);
	break;

    case POS_FIGHTING:
	send_to_char( "No way! You are busy fighting!\n\r", ch );
	break;

    }
    return;
   }
   /*Sleep*/
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
	send_to_char( "You go to sleep.\n\r", ch );
	act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	break;

    case POS_FIGHTING:
	send_to_char( "No Way! You are busy fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }


    victim->position = POS_STANDING;
    char_off_obj(ch);
    act( "You wake $M.", ch, NULL, victim, TO_CHAR );
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if (MOUNTED(ch))
    {
        send_to_char("You can't sneak while riding.\n\r", ch);
        return;
    }

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_sneak] )
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.type      = gsn_sneak;
	af.level     = ch->level;
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    if (MOUNTED(ch))
    {
        send_to_char("You can't hide while riding.\n\r", ch);
        return;
    }

    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_hide] )
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
	check_improve(ch,gsn_hide,TRUE,3);
    }
    else
	check_improve(ch,gsn_hide,FALSE,3);

    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

	buf[0] = '\0'; /*spellsong add*/

    if( ch == NULL ) /*spellsong add*/
	{
		/*bug - ch is null*/
		log_string( "BUG: Act_Move.c : DO_RECALL : ch is null" );
		return;
	}
	
	if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET) && !ch->mount )
    {
		send_to_char("Only players can recall.\n\r",ch);
		return;
    }
		
    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

    if (!IS_NPC(ch))
    {
    	if ( ch->pcdata->recall_room == NULL)
	    {
	     ch->pcdata->recall_room = get_room_index( ROOM_VNUM_TEMPLE);
	    }
		location = ch->pcdata->recall_room;
    }
    else
	{
		if(ch->master != NULL) /*add if structure and log - Spellsong*/
			location = ch->master->pcdata->recall_room;	
		else
		{
			/*bug master is null*/
			log_string( "BUG: Act_Move.c : DO_RECALL : ch->master is null" );
			return;
		}


	}

	if( location == NULL ) /*spellsong add*/
	{
		log_string( "BUG: Act_Move.c : DO_RECALL : recall location is NULL" );
		return;
	}

    if ( ch->in_room == location )
	{
	send_to_char( "Look around, look familiar?\n\r", ch);
	return;
	}

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE) )
    {
	send_to_char( "The gods have forsaken you.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	if (IS_NPC(ch))
	    skill = 40 + ch->level;
	else
	    skill = ch->pcdata->learned[gsn_recall];

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,gsn_recall,FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\n\r");
	    send_to_char( buf, ch );
	    return;
	}
	/***********BITS COMMENTED AND REPLACE BY SPELLSONG*****************/
	/*
	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );*/
	lose = (ch->exp * .03);
	gain_exp( ch, 0 - lose );
	check_improve(ch,gsn_recall,TRUE,4);
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
	
    }

    ch->move /= 2;
    send_to_char("You pray to the gods for transportation!", ch);
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    
    if (ch->pet != NULL)
	do_recall(ch->pet,"");

    if (ch->mount != NULL)
        do_recall( ch->mount, "" );
    return;
}



void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost;

	buf[0] = '\0'; /*spellsong add*/

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	send_to_char( buf, ch );
	argument = "foo";
    }

    /* cost changed from 1 to 3 by Eris - 16 April 2000 */

    cost = 3;

    if ( !str_cmp( argument, "str" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_STR )
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if ( !str_cmp( argument, "int" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_INT )
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if ( !str_cmp( argument, "wis" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_WIS )
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if ( !str_cmp( argument, "dex" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_DEX )
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if ( !str_cmp( argument, "con" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_CON )
	    cost    = 1;
	stat	    = STAT_CON;
	pOutput     = "constitution";
    }

    else if ( !str_cmp(argument, "hp" ) )
	cost = 1;

    else if ( !str_cmp(argument, "mana" ) )
	cost = 1;

    else
    {
	strcpy( buf, "You can train:" );
	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    strcat( buf, " str" );
	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    strcat( buf, " int" );
	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    strcat( buf, " wis" );
	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    strcat( buf, " dex" );
	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    strcat( buf, " con" );
	strcat( buf, " hp mana");

	if ( buf[strlen(buf)-1] != ':' )
	{
	    strcat( buf, ".\n\r" );
	    send_to_char( buf, ch );
	}
	else
	{
	    /*
	     * This message dedicated to Jordan ... you big stud!
	     */
	    act( "You have nothing left to train, you $T!",
		ch, NULL,
		ch->sex == SEX_MALE   ? "big stud" :
		ch->sex == SEX_FEMALE ? "hot babe" :
					"wild thing",
		TO_CHAR );
	}

	return;
    }

    if (!str_cmp("hp",argument))
    {
    	if ( cost > ch->train )
    	{
       	    send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }
 
	ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
        act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }
 
    if (!str_cmp("mana",argument))
    {
        if ( cost > ch->train )
        {
            send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana += 10;
        act( "Your power increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's power increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
	send_to_char( "You don't have enough training sessions.\n\r", ch );
	return;
    }

    ch->train		-= cost;
  
    ch->perm_stat[stat]		+= 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
    return;
}
bool check_web( CHAR_DATA *ch)
{
     AFFECT_DATA *af;
     int chance;
     int orig_dur;
	
     if IS_IMMORTAL(ch)
	return TRUE;

     
     af = affect_find(ch->affected , skill_lookup("web") );
     orig_dur = (af->level / 3);
     chance = (get_curr_stat(ch,STAT_STR) + (ch->level / 4));
     if (af->duration < (orig_dur / 9))
	chance += 20;
     if (af->duration > (orig_dur / 3))
	chance -= 20;
     if (number_percent() < chance)
     	return TRUE;

	return FALSE;
}

void do_beacon( CHAR_DATA *ch )
{
ch->pcdata->recall_room = ch->in_room;
send_to_char( "Beacon set.\n\r", ch );
return;
}

