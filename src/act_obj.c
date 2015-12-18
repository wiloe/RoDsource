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
#include <stdlib.h>
#include "merc.h"
#include "magic.h"
#include "olc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN( do_brew         );
DECLARE_DO_FUN( do_scribe       );
DECLARE_DO_FUN( do_imprint      );



/*
 * Local functions.
 */
#define CD CHAR_DATA
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( ( CHAR_DATA *ch ) );
int	get_cost	args( ( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
#undef	CD

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
	return TRUE;

    if (!obj->owner || obj->owner == NULL)
	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;

    if (owner == NULL)
	return TRUE;

    if (!str_cmp(ch->name,owner->name))
	return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->act,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE;

    return FALSE;
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

	buffer[0] = '\0'; /*spellsong add*/

	/*TRAP CHECK - muhahahahahaha Spellsong*/
	if( obj->item_type == ITEM_TRAP )
	{
		/*check for auto detect and disarm*/
		do_trap_effects( ch, obj, FALSE );
		return;
	}
    
	if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
		send_to_char( "You can't take that.\n\r", ch );
		return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( "$d: you can't carry that many items.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }


    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	act( "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))
    {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

    if ( container != NULL )
    {
    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  get_trust(ch) < obj->level)
	{
	    send_to_char("You are not powerful enough to use it.\n\r",ch);
	    return;
	}

    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE) && obj->timer)
	    obj->timer = 0;	
	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	obj_from_obj( obj );
    }
    else
    {
	act( "You get $p.", ch, obj, container, TO_CHAR );
	act( "$n gets $p.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->gold += obj->value[0];
        if (IS_SET(ch->act,PLR_AUTOSPLIT))
        { /* AUTOSPLIT code */
    	  members = 0;
    	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	  {
            if ( is_same_group( gch, ch ) )
	      members++;
    	  }

	  if ( members > 1 && obj->value[0] > 1)
	  {
	    sprintf(buffer,"%d",obj->value[0]);
	    do_split(ch,buffer);	
	  }
        }
 
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
    }

    return;
}


void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;
    bool ontest=FALSE; /*TRUE on the object, FALSE in the object*/
    argument = one_argument( argument, arg1 ); /*Snarf this*/
    argument = one_argument( argument, arg2 ); /*Snarf that*/

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2); /*Snarf that*/
    if (!str_cmp(arg2,"off")){
       argument = one_argument(argument,arg2); /*Snarf that!*/
       ontest=TRUE; /*MUST use off with all*/
       }
    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }
	    if (!(IS_IMMORTAL(ch)) && (obj->onlist != NULL)){
		send_to_char("There's too much on that!\n\r", ch);
	       return;
	       }
	    if (obj->atlist != NULL){
	       send_to_char("Looks like you need to convince some people to move\n\r",ch);
	       return;
	    }
	    
/* nolongdesc bug fix - Eris 24 March 2000 */
REMOVE_BIT(obj->extra_flags, ITEM_NOLONG);
           	get_obj( ch, obj, NULL );
	}
	else
	{

	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if ( (!IS_IMMORTAL(ch)) && (obj->onlist != NULL)){
			send_to_char("There's too much on that!\n\r", ch);
			return;
		       }
		    if (obj->atlist != NULL){
		       send_to_char("Looks like you need to convince some people to move\n\r",ch);
		       return;
		    }
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
  else /*obj from/off obj*/
    {
     if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	/*Can't get x in ALL or x on ALL*/
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }
      if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) ){

	    obj = get_obj_list( ch, arg1, container->onlist );
	    if ( obj != NULL ) /*If that bugger was on the object*/
	    {
	    ontest=TRUE;
	    }
	    else
	     ontest=FALSE;
	    }
	/*if it's on the object, then we *shouldn't* need to check furniture flags*/
	else
	 ontest=FALSE; /*Get all*/
	if (ontest){
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->onlist );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that on the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	act( "You get $p off $P.", ch, obj, container, TO_CHAR );
	act( "$n gets $p off $P.", ch, obj, container, TO_ROOM );
	obj_off_obj(obj);
	obj_to_char(obj, ch);
	}
       }
     else { /* it's a container*/
	 switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	    {

		if (!can_loot(ch,container))
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
	    }
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    &&  !IS_IMMORTAL(ch))
		    {
			send_to_char("Don't be so greedy!\n\r",ch);
			return;
		    }
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
      }
    }

    return;
}


/*Furniture stuff.. Kat 04181999*/
void do_put( CHAR_DATA *ch, char *argument )
{
    /*Plase note, I can't sleep.. beware of bugs and silliness--Kat*/
    /*Put: put this in that put this on that put this that*/
    int MAX_ONHOLD = 3;   /*mutliply the original weight by this and you get how much you can put ON the object*/
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container; /*Does double duty as furniture.. they use the same idea*/
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool ontest; /*TRUE for put on, FALSE for put in*/
    char buf[MAX_STRING_LENGTH]; /*So we can sprintf format strings*/

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument( argument, arg1 ); /*grab this*/
    argument = one_argument( argument, arg2 );
     if (!str_cmp(arg2,"in")){    /*'put this in that'*/
	argument = one_argument(argument,arg2); /*grab that*/
	ontest = FALSE;
	}
     else if (!str_cmp(arg2,"on")){  /*put this on that*/
	argument = one_argument(argument,arg2); /*grab that*/
	ontest=TRUE;
	}
    else /*put this that*/
     ontest=FALSE; /*Default to put in*/

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	sprintf(buf, "Put what %s what?\n\r", ontest?"on":"in"); /*in or on*/
	send_to_char( buf, ch);
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	/*Can't put in ALL or on ALL*/
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }
    switch (ontest){
    case FALSE: /*Do ye olde normal put stuff*/
    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	if ( get_obj_weight( obj ) + get_obj_weight( container )
	     > container->value[0] )
	{
	    send_to_char( "It won't fit.\n\r", ch );
	    return;
	}

	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container,ITEM_TAKE))
        {
	    if (obj->timer)
	    {
		send_to_char( "Only permanent items may go in the pit.\n\r",ch);
		return;
	    }
	    else
            {
		obj->timer = number_range(100,200);
            }
        }
	obj_from_char( obj );
	obj_to_obj( obj, container );
	act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_obj_weight( container )
		 <= container->value[0] )
	    {
		if (container->pIndexData->vnum == OBJ_VNUM_PIT
		&&  !CAN_WEAR(obj, ITEM_TAKE) )
                {
		    if (obj->timer)
                    {
			continue;
                    }
		    else
                    {
			obj->timer = number_range(100,200);
                    }
                }
		obj_from_char( obj );
		obj_to_obj( obj, container );
		act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	    }
	   }
	 }
     /*****************END OF PUT IN************************************/
     break;

   case TRUE: /*Put on stuffies!*/
    /*Furniture flags are independent of item time.. so let's check them bits*/
    /*Rather fragging ugly here.. but it wouldn't work as an OR*/
    if ( (!IS_SET(container->furn_flags, FURN_LAY_ON)))
     {
	if ( (!IS_SET(container->furn_flags, FURN_SET_ON)) ) {
	send_to_char("You can't lay things on that!!\n\r", ch);
	return;
	}
	}

    /*Comment out the next if block if you want people to be able to hold items and put things on them*/
    /*Please note there is currently NO save code for objects with objects on them*/
    if (container->carried_by != NULL){ /*Someone is holding this!*/
	send_to_char("You can't put things on furniture you're holding!\n\r",ch);
	return;
       }
    /*End section about furniture in inventory*/
    /*If we aren't doing put all*/
    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "Put something on itself, Now that's funny.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}
	/*Not allowing you to put something 3 times the container's original weight on it*/
	if ( get_obj_weight( obj ) + get_obj_weight( container )
	     > container->pIndexData->weight*MAX_ONHOLD )
	{
	    send_to_char( "It won't fit.\n\r", ch );
	    return;
	}


	obj_from_char( obj );
	obj_on_obj( obj, container );
	act( "$n puts $p on $P.", ch, obj, container, TO_ROOM );
	act( "You put $p on $P.", ch, obj, container, TO_CHAR );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_obj_weight( container )
		 <= container->pIndexData->weight*MAX_ONHOLD )
	    {

		obj_from_char( obj );
		obj_on_obj( obj, container );
		act( "$n puts $p on $P.", ch, obj, container, TO_ROOM );
		act( "You put $p on $P.", ch, obj, container, TO_CHAR );
	    }
	}
    }

  }
   return;
}

void do_donate( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    ROOM_INDEX_DATA *donation,*find_location();
    ROOM_INDEX_DATA *was_in_room;
    char buf[200];
    char *room;
    bool found;

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument( argument, arg );
    room = "3300";
    donation = find_location( ch, room );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Donate what?\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'donate obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, donation );
	act( "$n donates $p.", ch, obj, NULL, TO_ROOM );
	act( "You donate $p.", ch, obj, NULL, TO_CHAR );
	was_in_room=ch->in_room;
	ch->in_room=donation;
	sprintf( buf, "A shimmering portal appears in midair, and through it falls\n\r$n's donation: $p");
	act( buf, ch, obj, NULL, TO_ROOM);
	ch->in_room=was_in_room;
    }
    else
    {
	/* 'donate all' or 'donate all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, donation );
		act( "$n donates $p.", ch, obj, NULL, TO_ROOM );
		act( "You donate $p.", ch, obj, NULL, TO_CHAR );
		was_in_room=ch->in_room;
		ch->in_room=donation;
		sprintf( buf, "A shimmering portal appears in midair, and through it falls\n\r$n's donation: $p");
		act( buf, ch, obj, NULL, TO_ROOM);
		ch->in_room=was_in_room;
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}

void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
	int amount;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
	     str_cmp( arg, "gold"  ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that many coins.\n\r", ch );
	    return;
	}

	ch->gold -= amount;

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_MONEY_ONE:
		amount += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_MONEY_SOME:
		amount += obj->value[0];
		extract_obj( obj );
		break;
	    }
	}

	obj_to_room( create_money( amount ), ch->in_room );
	act( "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	     str_cmp( arg2, "gold"  ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that much gold.\n\r", ch );
	    return;
	}

	ch->gold     -= amount;
	victim->gold += amount;
	sprintf(buf,"$n gives you %d gold.",amount);
	act( buf, ch, NULL, victim, TO_VICT    );
	act( "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );
	sprintf(buf,"You give $N %d gold.",amount);
	act( buf, ch, NULL, victim, TO_CHAR    );
	mprog_bribe_trigger( victim, ch, amount );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    mprog_give_trigger( victim, ch, obj );
    return;
}




void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != 0 )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    act( "You fill $p.", ch, obj, NULL, TO_CHAR );
    obj->value[2] = 0;
    obj->value[1] = obj->value[0];
    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument(argument,arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
        send_to_char("Pour what into what?\n\r",ch);
        return;
    }


    if ((out = get_obj_carry(ch,arg)) == NULL)
    {
        send_to_char("You don't have that item.\n\r",ch);
        return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
        send_to_char("That's not a drink container.\n\r",ch);
        return;
    }

    if (!str_cmp(argument,"out"))
    {
        if (out->value[1] == 0)
        {
            send_to_char("It's already empty.\n\r",ch);
            return;
        }

        out->value[1] = 0;
        out->value[3] = 0;
        sprintf(buf,"You invert $p, spilling %s all over the ground.",
                liq_table[out->value[2]].liq_name);
        act(buf,ch,out,NULL,TO_CHAR);

        sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
                liq_table[out->value[2]].liq_name);
        act(buf,ch,out,NULL,TO_ROOM);
        return;
    }

    if ((in = get_obj_here(ch,argument)) == NULL)
    {
        vch = get_char_room(ch,argument);

        if (vch == NULL)
        {
            send_to_char("Pour into what?\n\r",ch);
            return;
        }

        in = get_eq_char(vch,WEAR_HOLD);

        if (in == NULL)
        {
            send_to_char("They aren't holding anything.",ch);
            return;
        }
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
        send_to_char("You can only pour into other drink 
containers.\n\r",ch);
        return;
    }

    if (in == out)
    {
        send_to_char("You cannot change the laws of physics!\n\r",ch);
        return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
        send_to_char("They don't hold the same liquid.\n\r",ch);
        return;
    }

    if (out->value[1] == 0)
    {
        act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
        return;
    }

    if (in->value[1] >= in->value[0])    {
        act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
        return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];

    if (vch == NULL)
    {
        sprintf(buf,"You pour %s from $p into $P.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,out,in,TO_CHAR);
        sprintf(buf,"$n pours %s from $p into $P.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,out,in,TO_ROOM);
    }
    else
    {
        sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR);
        sprintf(buf,"$n pours you some %s.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_VICT);
        sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT);

    }
}


void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	break;

    case ITEM_FOUNTAIN: /*oh look.. fountains with different liquids *EG*/
	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	act( "$n drinks $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_ROOM );
	act( "You drink $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_CHAR );

	amount = number_range(3, 10);


	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );
	gain_condition( ch, COND_FULL,
	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );
	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	    send_to_char( "You feel drunk.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	    send_to_char( "You are full.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	    send_to_char( "You do not feel thirsty.\n\r", ch );
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	act( "$n drinks $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_ROOM );
	act( "You drink $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_CHAR );

	amount = number_range(3, 10);
	amount = UMIN(amount, obj->value[1]);

	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );
	gain_condition( ch, COND_FULL,
	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );
	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	    send_to_char( "You feel drunk.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	    send_to_char( "You are full.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	    send_to_char( "You do not feel thirsty.\n\r", ch );
	
	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned ! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );
	    af.type      = gsn_poison;
	    af.level	 = number_fuzzy(amount); 
	    af.duration  = 3 * amount;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}

	obj->value[1] -= amount;
	break;
    }

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
	{   
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char( "You are full.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.type      = gsn_poison;
	    af.level 	 = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	break;

    case ITEM_PILL:
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/

    if ( ch->level < obj->level )
    {
	sprintf( buf, "You must be level %d to use this object.\n\r",
	    obj->level );
	send_to_char( buf, ch );
	act( "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
	act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

	/*Start Spellsong Add for New Wear Slots*/
	
	/*ears*/
	if ( CAN_WEAR( obj, ITEM_WEAR_EAR ) )
    {
	if ( get_eq_char( ch, WEAR_EAR_L ) != NULL
	&&   get_eq_char( ch, WEAR_EAR_R ) != NULL
	&&   !remove_obj( ch, WEAR_EAR_L, fReplace )
	&&   !remove_obj( ch, WEAR_EAR_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_EAR_L ) == NULL )
	{
	    act( "$n wears $p in $s left ear.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p in your left ear.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_EAR_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_EAR_R ) == NULL )
	{
	    act( "$n wears $p in $s right ear.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p in your right ear.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_EAR_R );
	    return;
	}

	bug( "Wear_obj: no free ear.", 0 );
	send_to_char( "You already wear two ear items.\n\r", ch );
	return;
    }
	
	/*floating slot*/
	if ( CAN_WEAR( obj, ITEM_WEAR_FLOATING ) )
    {
	if ( !remove_obj( ch, WEAR_FLOATING, fReplace ) )
	    return;
	act( "$n casts $p in the air next to $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You cast $p in the air next to you.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FLOATING );
	return;
    }

	/*bicep slot*/
	if ( CAN_WEAR( obj, ITEM_WEAR_BICEP ) )
    {
	if ( get_eq_char( ch, WEAR_BICEP_L ) != NULL
	&&   get_eq_char( ch, WEAR_BICEP_R ) != NULL
	&&   !remove_obj( ch, WEAR_BICEP_L, fReplace )
	&&   !remove_obj( ch, WEAR_BICEP_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_BICEP_L ) == NULL )
	{
	    act( "$n wears $p around $s left bicep.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left bicep.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_BICEP_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_BICEP_R ) == NULL )
	{
	    act( "$n wears $p around $s right bicep.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right bicep.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_BICEP_R );
	    return;
	}

	bug( "Wear_obj: no free bicep.", 0 );
	send_to_char( "You already wear two bicep items.\n\r", ch );
	return;
    }

	/*face*/
	if ( CAN_WEAR( obj, ITEM_WEAR_FACE ) )
    {
	if ( !remove_obj( ch, WEAR_FACE, fReplace ) )
	    return;
	act( "$n wears $p over $s face.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p over your face.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FACE );
	return;
    }

	/*Ankles*/
	if ( CAN_WEAR( obj, ITEM_WEAR_ANKLE ) )
    {
	if ( get_eq_char( ch, WEAR_ANKLE_L ) != NULL
	&&   get_eq_char( ch, WEAR_ANKLE_R ) != NULL
	&&   !remove_obj( ch, WEAR_ANKLE_L, fReplace )
	&&   !remove_obj( ch, WEAR_ANKLE_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_ANKLE_L ) == NULL )
	{
	    act( "$n wears $p around $s left ankle.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left ankle.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_ANKLE_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_ANKLE_R ) == NULL )
	{
	    act( "$n wears $p around $s right ankle.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right ankle.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_ANKLE_R );
	    return;
	}

	bug( "Wear_obj: no free ankle.", 0 );
	send_to_char( "You already wear two ankle items.\n\r", ch );
	return;
    }
	
	/*shoulders*/
	if ( CAN_WEAR( obj, ITEM_WEAR_SHOULDERS ) )
    {
	if ( !remove_obj( ch, WEAR_SHOULDERS, fReplace ) )
	    return;
	act( "$n wears $p over $s shoulders.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p over your shoulders.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHOULDERS );
	return;
    }
	/*End Spellsong Add For New Wear Slots*/

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;
	OBJ_DATA *second_weapon;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);
	second_weapon = get_eq_char(ch,WEAR_SECOND_WIELD);
	if (((weapon != NULL) && (ch->size < SIZE_LARGE) 
	&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS)) 
	|| (weapon != NULL && (second_weapon != NULL
	|| get_eq_char(ch,WEAR_HOLD) != NULL)))
	{
	    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
	    return;
	}

	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn=0,skill;

	if ( !remove_obj(ch, WEAR_SECOND_WIELD, fReplace) )
	    return;

	if ( !IS_NPC(ch) 
	&& get_obj_weight( obj ) > str_app[get_curr_stat(ch,STAT_STR)].wield )
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

	if ((!IS_NPC(ch) && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  (( get_eq_char(ch,WEAR_SECOND_WIELD) !=NULL
 	|| get_eq_char(ch,WEAR_SHIELD) != NULL) 
 	|| get_eq_char(ch,WEAR_WIELD) != NULL))
 	|| (get_eq_char(ch,WEAR_WIELD) != NULL
 	&& (get_eq_char(ch,WEAR_SHIELD) != NULL
 	|| get_eq_char(ch,WEAR_SECOND_WIELD) !=NULL
 	|| get_eq_char(ch,WEAR_HOLD) != NULL))
 	|| (get_eq_char(ch,WEAR_SHIELD) != NULL
 	&& get_eq_char(ch,WEAR_HOLD) != NULL))
	{
	    send_to_char("You'd need another free hand to do that.\n\r",ch);
	    if (get_eq_char(ch,WEAR_WIELD) != NULL)
	       if ( !remove_obj(ch, WEAR_SECOND_WIELD, fReplace) )
	          return;
	    return;
	}

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	if (get_eq_char(ch,WEAR_WIELD) == NULL) {
	    equip_char( ch, obj, WEAR_WIELD );
            sn = get_weapon_sn(ch);
        }
	else if (get_eq_char(ch,WEAR_SECOND_WIELD) == NULL) {
	    equip_char( ch, obj, WEAR_SECOND_WIELD);
            sn = get_second_weapon_sn(ch);
        }

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100)
            act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)
            act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else
            act("You don't even know which is end is up on $p.",
                ch,obj,NULL,TO_CHAR);

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;
        if (get_eq_char(ch, WEAR_WIELD) != NULL 
        && (get_eq_char(ch, WEAR_SECOND_WIELD) != NULL
        || get_eq_char(ch, WEAR_SHIELD) != NULL))
        {
            send_to_char("You need one hand free to hold that.\n\r",ch);
                return;
        }
	act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE );
    }

    return;
}

void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *wield;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'rem obj' */
	if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
	{
	    send_to_char( "You aren't wearing that item.\n\r", ch );
	    return;
	}
	if (get_eq_char(ch, WEAR_WIELD) == obj
	&& get_eq_char(ch, WEAR_SECOND_WIELD) != NULL) {
            remove_obj( ch, obj->wear_loc, TRUE );
            wield=get_eq_char(ch, WEAR_SECOND_WIELD);
            wield->wear_loc=WEAR_WIELD;
            }
        else remove_obj( ch, obj->wear_loc, TRUE );
    }
    else
    {
	/* 'rem all' or 'rem all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc != WEAR_NONE )
	    {
		found = TRUE;
	        remove_obj( ch, obj->wear_loc, TRUE );
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not wearing anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not wearing any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}

void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int gold;
    
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

	buf[0] = '\0'; /*spellsong add*/
	buffer[0] = '\0'; /*spellsong add*/

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( "$n offers $mself to the gods, but they graciously decline.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "The gods appreciate your offer and may accept it later.\n\r", ch );
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
        {
	   send_to_char(
	     "The gods wouldn't like that.\n\r",ch);
	   return;
        }
    }


    if ( !CAN_WEAR(obj, ITEM_TAKE))
    {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
    }

    gold = UMAX(1,obj->level * 2);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	gold = UMIN(gold,obj->cost);

    if (gold == 1)
        send_to_char(
	    "The gods give you one gold coin for your sacrifice.\n\r", ch );
    else
    {
	sprintf(buf,"The gods give you %d gold coins for your sacrifice.\n\r",
		gold);
	send_to_char(buf,ch);
    }
    
    ch->gold += gold;
    
    if (IS_SET(ch->act,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	{
    	    if ( is_same_group( gch, ch ) )
            members++;
    	}

	if ( members > 1 && gold > 1)
	{
	    sprintf(buffer,"%d",gold);
	    do_split(ch,buffer);	
	}
    }

    act( "$n sacrifices $p to the gods.", ch, obj, NULL, TO_ROOM );
    wiznet("$N sends up $p as a burnt offering.", ch,obj,WIZ_SACCING,0,0);
    extract_obj( obj );
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    if (ch->level < obj->level)
    {
	send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
	return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }

    if ( ch->level < scroll->level)
    {
	send_to_char(
		"This scroll is too complex for you to comprehend.\n\r",ch);
	return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    {
	send_to_char("You mispronounce a syllable.\n\r",ch);
	check_improve(ch,gsn_scrolls,FALSE,2);
    }

    else
    {
    	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
	check_improve(ch,gsn_scrolls,TRUE,2);
    }

    extract_obj( scroll );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	if ( ch->level < staff->level 
	||   number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
 	{
	    act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
	    act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

	if ( victim != NULL ) /*make sure its a char first */
	{
	if(wand->value[3] < 0 && wand->value[3] >= MAX_SKILL)
	{
		char mesg[MAX_STRING_LENGTH];
		sprintf(mesg,"Bad sn on wand: vnum: %d sn: %d",
			 wand->value[3], wand->pIndexData->vnum);
		send_to_char("Something goes wrong.\n\r",ch);
		bug(mesg,0);
		return;
	}
	if(skill_table[wand->value[3]].target == TAR_CHAR_OFFENSIVE )
	{
          if ( !IS_NPC(victim) )
          {
            if ( !IS_SET(victim->act, PLR_KILLER) || !IS_SET(ch->act, PLR_KILLER) ) 
            {
             send_to_char( "You can only kill other player killers.\n\r", ch );
             return;
            }
          }
	}
	}

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

 	if (ch->level < wand->level 
	||  number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5) 
	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_CHAR);
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /*Spellsong modifications to steal:
	 *better make sure both the thief and victim if players are
	 *set to pkill. Why so you ask...Ask me and i will explain.*/
	
	if (!IS_NPC(ch) && !IS_NPC(victim)) /*are both players*/
	{
		if ( !IS_SET(ch->act, PLR_KILLER) )
		{
			send_to_char( "Only player killers can steal from other players!\n\r", ch );
			return;
		}
		else /*is a killer*/
		{
			
			if ( !IS_SET(victim->act, PLR_KILLER) ) /*victim not a killer*/
			{
				send_to_char( "You can only steal from other player killers!\n\r", ch );
				return;
			}
			/*end if*/
		}
		/*end if*/
	}
	/*end if*/
	/****************End Spellsong Add***************/
	
	if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (victim->position == POS_FIGHTING)
    {
	send_to_char("You'd better not -- you might get hit.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 );

    /* Eris add to allow easy theft from knocked out persons 8 July 2000*/
    if (victim->position != POS_STUNNED)
    {
    if ( ch->level + 5 < victim->level
    ||   victim->position == POS_FIGHTING
    || ( !IS_NPC(ch) && percent > (ch->pcdata->learned[gsn_steal] - 2) ) )
    {
	/*
	 * Failure.
	 */
	send_to_char( "Oops.\n\r", ch );
	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s is a lousy thief!", ch->name );
	   break;
        case 1 :
	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		    ch->name,(ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"%s tried to rob me!",ch->name );
	    break;
	case 3 :
	    sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	    break;
        }
	do_yell( victim, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
	        check_improve(ch,gsn_steal,FALSE,2);
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
                sprintf(buf,"$N tried to steal from %s.",victim->name);
                wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
		log_string( buf );
		if ( !IS_SET(ch->act, PLR_THIEF) )
		{
		    SET_BIT(ch->act, PLR_THIEF);
		    send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
		    save_char_obj( ch );
		}
	    }
	}

	return;
    }
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "gold"  ) )
    {
	int amount;
        int expgain;

	amount = victim->gold * number_range(1, 10) / 100;
	if ( amount <= 0 )
	{
	    send_to_char( "You couldn't get any gold.\n\r", ch );
	    return;
	}

	ch->gold     += amount;
	victim->gold -= amount;
        expgain = amount*33;
        gain_exp(ch,expgain);
/*	sprintf( buf, "Bingo!  You got %d gold coins.\n\r", amount ); */
        sprintf( buf, "Bingo!  You get %d gold coins and gained %d experience.\n\r", amount, expgain );
	send_to_char( buf, ch );
	check_improve(ch,gsn_steal,TRUE,2);
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   obj->level > ch->level )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    check_improve(ch,gsn_steal,TRUE,2);
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

	buf[0] = '\0'; /*spellsong add*/

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Undesirables.
     */
/*    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
    {
	do_say( keeper, "Killers are not welcome!" );
	sprintf( buf, "%s the KILLER is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }*/

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
    {
	do_say( keeper, "Thieves are not welcome!" );
	sprintf( buf, "%s the THIEF is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }

    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_say( keeper, "Sorry, I am closed. Come back later." );
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	do_say( keeper, "Sorry, I am closed. Come back tomorrow." );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "I don't trade with folks I can't see." );
	return NULL;
    }

    return keeper;
}



int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj->pIndexData == obj2->pIndexData )
            {
                cost = 0;
                break;
            }
	}
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
	cost = cost * obj->value[2] / obj->value[1];

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cost,roll;

	buf[0] = '\0'; /*spellsong add*/

    if ( argument[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_MOUNT_SHOP) )
    {
        do_buy_mount( ch, argument );
       return;
    }
    
    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg );
	ch->in_room = in_room;

	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	if ( ch->pet != NULL )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return;
	}

 	cost = 10 * pet->level * pet->level;

	if ( ch->gold < cost )
	{
	    send_to_char( "You can't afford this pet.\n\r", ch );
	    return;
	}

	if ( ch->level < pet->level )
	{
	    send_to_char( "You're not powerful enough to master this pet.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	
	}

	ch->gold		-= cost;
	pet			= create_mobile( pet->pIndexData );
	SET_BIT(ch->act, PLR_BOUGHT_PET);
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	return;
    }
/*    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	obj  = get_obj_carry( keeper, argument );
	cost = get_cost( keeper, obj, TRUE );

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you 'I don't sell that -- try 'list''.",
		keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if ( ch->gold < cost )
	{
	    act( "$n tells you 'You can't afford to buy $p'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
	if ( obj->level > ch->level )
	{
	    act( "$n tells you 'You can't use $p yet'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	}

	if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}


	act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
	act( "You buy $p.", ch, obj, NULL, TO_CHAR );
	ch->gold     -= cost;
	keeper->gold += cost;

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    obj = create_object( obj->pIndexData, obj->level );
	else
	    obj_from_char( obj );


	if (obj->timer > 0)
	    obj-> timer = 0;
	obj_to_char( obj, ch );
	if (cost < obj->cost)
	    obj->cost = cost;
	return;
    }
}
*/
/* Multiple object buy routine

  This code was written by Erwin Andreasen, 4u2@aarhues.dk
  Please mail me if you decide to use this code, or if you have any
  additions, bug reports, ideas or just comments.
  Please do not remove my name from the files.

*/
    else /* object purchase code begins HERE */
    {
        CHAR_DATA *keeper;
        OBJ_DATA *obj;
        int cost;
	char arg[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH]; /* 2nd argument */
        int item_count = 1;          /* default: buy only 1 item */

    argument = one_argument (argument, arg); /* get first argument */
    argument = one_argument (argument, arg2); /* get another argument, if any */

    if (arg2[0]) /* more than one argument specified? then arg2[0] <> 0 */
    {
        /* check if first of the 2 args really IS a number */

        if (!is_number(arg))
        {
            send_to_char ("Syntax for BUY is: BUY [number] <item>\n\r\"number\" is an optional number of items to buy.\n\r",ch);
            return;
        }

        item_count = atoi (arg); /* first argument is the optional count */
        strcpy (arg,arg2);       /* copy the item name to its right spot */
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL ) /* is there a shopkeeper here? */
            return;

/* find the pointer to the object */
        obj  = get_obj_carry( keeper, arg );
        cost = get_cost( keeper, obj, TRUE );

    if ( cost <= 0 || !can_see_obj( ch, obj ) ) /* cant buy what you cant see */
        {
            act( "$n tells you 'I don't sell that -- try 'list''.",keeper, NULL, ch, TO_VICT );
            ch->reply = keeper;
            return;
        }

/* check for valid positive numeric value entered */
    if (!(item_count > 0))
    {
            send_to_char ("Buy how many? Number must be positive!\n\r",ch);
            return;
    }

	/* haggle */
	roll = number_percent();
	if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
	{
	    cost -= obj->cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}

/* can the character afford it ? */
    if ( ch->gold < (cost * item_count) )
    {
        if (item_count == 1) /* he only wanted to buy one */
        {
            act( "$n tells you 'You can't afford to buy $p'.",keeper, obj, ch, TO_VICT );
        }
        else
        {
            char buf[MAX_STRING_LENGTH]; /* temp buffer */
            if ( (ch->gold / cost) > 0) /* how many CAN he afford? */
                sprintf (buf, "$n tells you 'You can only afford %ld of those!", (ch->gold / cost));
            else /* not even a single one! what a bum! */
                sprintf (buf, "$n tells you '%s? You must be kidding - you can't even afford a single one, let alone %d!'",capitalize(obj->short_descr), item_count);

            act(buf,keeper, obj, ch, TO_VICT );
            ch->reply = keeper; /* like the character really would reply to the shopkeeper... */
            return;
        }

        ch->reply = keeper; /* like the character really would reply to the shopkeeper... */
        return;
    }

/* Can the character use the item at all ? */
    if ( obj->level > ch->level )
    {
        act( "$n tells you 'You can't use $p yet'.",
            keeper, obj, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }
/* can the character carry more items? */
    if ( ch->carry_number + (get_obj_number(obj)*item_count) > can_carry_n( ch ) )
        {
            send_to_char( "You can't carry that many items.\n\r", ch );
            return;
        }

/* can the character carry more weight? */
    if ( ch->carry_weight + item_count*get_obj_weight(obj) > can_carry_w( ch ) )
        {
            send_to_char( "You can't carry that much weight.\n\r", ch );
            return;
        }

/* check for objects sold to the keeper */
    if ( (item_count > 1) && !IS_SET (obj->extra_flags,ITEM_INVENTORY))
    {
        act( "$n tells you 'Sorry - $p is something I have only one of'.",keeper, obj, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

/* change this to reflect multiple items bought */
    if (item_count == 1)
    {
        act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
        act( "You buy $p.", ch, obj, NULL, TO_CHAR );
    }
    else /* inform of multiple item purchase */
    {
        char buf[MAX_STRING_LENGTH]; /* temporary buffer */

/* "buys 5 * a piece of bread" seems to be the easiest and least gramatically incorerct solution. */
        sprintf (buf, "$n buys %d * $p.", item_count);
        act (buf, ch, obj, NULL, TO_ROOM); /* to char self */
        sprintf (buf, "You buy %d * $p.", item_count);
        act(buf, ch, obj, NULL, TO_CHAR ); /* to others */
    }

    ch->gold     -= cost*item_count;
    keeper->gold += cost*item_count;

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) ) /* 'permanent' item */
    {
        /* item_count of items */
        for ( ; item_count > 0; item_count--) /* create item_count objects */
        {
            obj = create_object( obj->pIndexData, obj->level );
            obj_to_char( obj, ch );
        }
    }
    else /* single item */
    {
        obj_from_char( obj );
        obj_to_char( obj, ch );
    }
    return;
    } /* else */
} /* do_buy */


void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)
    || IS_SET(ch->in_room->room_flags, ROOM_MOUNT_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
            if ( IS_SET(pet->act, ACT_PET) 
            || IS_SET(pet->act, ACT_MOUNT) )
	    {
		if ( !found )
		{
		    found = TRUE;
                   if (IS_SET(pet->act, ACT_PET))
                       send_to_char( "Pets for sale:\n\r", ch );
                   else if (IS_SET(pet->act, ACT_MOUNT))
                       send_to_char( "Mounts for sale:\n\r", ch );
		}
		sprintf( buf, "[%2d] %8d - %s\n\r",
		    pet->level,
		    10 * pet->level * pet->level,
		    pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
        {
            if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
                send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
            else
                send_to_char( "Sorry, we're out of mounts right now.\n\r",ch ); 
        }
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;
        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 
	    &&   ( arg[0] == '\0'  
 	       ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Lv Price] Item\n\r", ch );
		}

		sprintf( buf, "[%2d %5d] %s.\n\r",
		    obj->level, cost, obj->short_descr);
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "You can't buy anything here.\n\r", ch );
	return;
    }
}

/********************************************************************
new do_sell routine. Based off Embermud and Rom2.4. Idea consideration
goes to Brian Moore and Russ & Gab Tailor for ideas from the ROM4.0
Codebase. Any use of this is prohibited without prior consent.

email nfn15571@naples.net for consideration.
Author: Mike Riddle 16March2000 
AKA: SPELLSONG
********************************************************************/

void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;
	SHOP_DATA *pShop;
	
	/*add. const and vars needed*/
	const int MAX_SELL = 8; /*alter this to adjust how many item the keeper will buy*/
	OBJ_DATA *incObj;
	int incMod,itype;
	bool found;
	
	buf[0] = '\0'; /*spellsong add*/

	found = FALSE;
	incMod = 0; /*init incMod. Default 0*/
	
    one_argument( argument, arg ); /*get the argument*/

    if ( arg[0] == '\0' ) /*gotta enter something*/
    {
		send_to_char( "What are you trying to sell?\n\r", ch );
		return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL ) /*umm gots to be a keeper there...duh*/
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL ) /* character doesnt have that */
    {
		act( "$n tells you 'If you actually had that item, I just might buy it'.", /*lets make them a bit droll*/
	    keeper, NULL, ch, TO_VICT );
		ch->reply = keeper; /*what the hell use reply and tell the keeper he is a moron*/
		return;
    }

    if ( !can_drop_obj( ch, obj ) ) /*sorry bub, but your item is cursed*/
    {
		send_to_char( "You try to sell it but you can't let go of it.\n\r", ch );
		return;
    }

    if (!can_see_obj(keeper,obj)) /*well he can't see it - so lets return a little sass*/
    {
		act("$n tells you 'I cannot take your word that something is actually there'.",keeper,NULL,ch,TO_VICT);
		return;
    }
	
	pShop = keeper->pIndexData->pShop; /*initialize pShop*/

	/*check to ensure item type matches shoptype*/
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
			found = TRUE;
			break;
	    }
	}
	
	if (!found) /*well he doesnt buy that type - so lets return a little snob*/
    {
		act("$n tells you 'I don't deal in those items'.",keeper,NULL,ch,TO_VICT);
		return;
    }
	
	/*throw a check in here to allow multiple sells and to decrease the
	 * price for each item after the first - spellsong*/
    	
	if( obj->timer ) /*make single check here for rot items*/
    {   /*return some attitude*/
		act( "$n looks at $p and says 'What do you take me for'?", keeper, obj, ch, TO_VICT );
		return;
    }
	
	cost = obj->cost; /*get items cost*/

	/*allow multiple sells of same item. Alter MAX_SELL at beginning of
	 *function to alter this. - Spellsong*/
	for ( incObj = keeper->carrying; incObj != NULL; incObj = incObj->next_content )
	{	
		/*looping through items carried by the keeper to find a match*/
	    if (!strncmp(incObj->short_descr, obj->short_descr, strlen(incObj->short_descr))) /*ok keepers shors item being solds short*/
		{
			incMod += 1; /*counter for number of matching shorts*/
		}
	}
	/*works to here*/
	if (incMod > 0 )
	{
		if (incMod >= MAX_SELL) /*if he has reached his inventory limit too bad so sad*/ 
		{
			act( "$p is no longer needed by $n.", keeper, obj, ch, TO_VICT );
			return;
		}
		else
		{
			if (incMod > 0) /*better check for 0 :)*/
			{
				/*lets reduce this incrementally*/
				cost = cost - (cost * (.117 * incMod));
			}
		}/*end if*/

		if (cost < 1)
		{
			/*a little bit of snob*/
			act( "$n looks at $p and says 'This is worthless'.", keeper, obj, ch, TO_VICT );
			return;
		}
		/*end if*/
	}
	/*end if*/

	/*ok lets modify for the shops profit*/
	cost = cost * pShop->profit_sell / 100;
	
	if ( cost > keeper->gold )
    {
		/*poor shop keeper is broke*/
		act("$n tells you 'I'm afraid I don't have enough gold to buy $p.",
	    keeper,obj,ch,TO_VICT);
		return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    
	/* haggle */
    roll = number_percent();
    if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        /*cost += obj->cost / 2 * roll / 100;*/
		cost += cost / 2 * roll / 100; /*spellsong mod for new*/
        /*cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);*/
		cost = UMIN(cost,95 * cost / 100);
		cost = UMIN(cost,keeper->gold);
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    
	sprintf( buf, "You sell $p for %d gold piece%s.",
	cost, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    
	/*handle all that gold exchange stuff*/
	ch->gold     += cost;
    keeper->gold -= cost;
    
	/*just a check to make sure?*/
	if ( keeper->gold < 0 )
	{
		keeper->gold = 0;
	}

    /*Lets take that item and give it to the shopkeeper*/
	if ( obj->item_type == ITEM_TRASH )
    {
		extract_obj( obj );
    }
    else
    {
		obj_from_char( obj );
		obj->timer = number_range(50,100);/*adjust this range for how long*/ 
		obj_to_char( obj, keeper );		  /*the item stays in keepers inv */
    }/*end if*/
	
    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

	buf[0] = '\0'; /*spellsong add*/

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, "$n tells you 'I'll give you %d gold coins for $p'.", cost );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}


void do_junk( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;
    int gold;

	buf[0] = '\0'; /*spellsong add*/
    
    one_argument( argument, arg );
    gold = 0;
    if ( arg[0] == '\0' )
    {
	send_to_char(
	    "What do you wish to junk?\n\r", ch );
	return;
    }
    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        /* 'junk obj' */
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        if ( !can_drop_obj( ch, obj ) )
        {
            send_to_char( "You can't let go of it.\n\r", ch );
            return;
        }

    	if ( obj->item_type == ITEM_CORPSE_PC )
    	{
	  if (obj->contains)
            {
	     send_to_char(
	     "The gods wouldn't like that.\n\r",ch);
	     return;
            }
         }

    	gold = UMAX(1,obj->level * 2);

    	if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	gold = UMIN(gold,obj->cost);

    	if (gold == 1)
        send_to_char("The gods give you one gold coin for your sacrifice.\n\r", ch );
   	else
    	{
	sprintf(buf,"The gods give you %d gold coins for your sacrifice.\n\r",gold);
	send_to_char(buf,ch);
    	}
    
    	ch->gold += gold;
	act( "$n sacrifices $p to the gods.", ch, obj, NULL, TO_ROOM );
	extract_obj( obj );
 	return;
   }
else
   {
        /* 'donate all' or 'donate all.obj' */
        found = FALSE;
        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            &&   can_see_obj( ch, obj )
            &&   obj->wear_loc == WEAR_NONE
            &&   can_drop_obj( ch, obj ) )
            {
                found = TRUE;
        	if ( !can_drop_obj( ch, obj ) )
        	{
            	send_to_char( "You can't let go of it.\n\r", ch );
            	break;
        	}

    		if ( obj->item_type == ITEM_CORPSE_PC )
    		{
	 	  if (obj->contains)
            	  {
	     	   send_to_char(
	     	   "The gods wouldn't like that.\n\r",ch);
	     	   break;
            	  }
		}
	gold = UMAX(1,obj->level * 2);
		
    	if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	gold = UMIN(gold,obj->cost);

    	if (gold == 1)
        send_to_char("The gods give you one gold coin for your sacrifice.\n\r", ch );
    	else
    	{
	sprintf(buf,"The gods give you %d gold coins for your sacrifice.\n\r",gold);
	send_to_char(buf,ch);
    	}
    
    	ch->gold += gold;
	act( "$n sacrifices $p to the gods.", ch, obj, NULL, TO_ROOM );
    	extract_obj( obj );
       }
    }
        if ( !found )
        {
            if ( arg[3] == '\0' )
                act( "You are not carrying anything.",
                    ch, NULL, arg, TO_CHAR );
            else
                act( "You are not carrying any $T.",
                    ch, NULL, &arg[4], TO_CHAR );
        }
  }

    return;
}

/*
brew.c by Jason Huang (huangjac@netcom.com) 
*/
/* Original Code by Todd Lair.                                        */
/* Improvements and Modification by Jason Huang (huangjac@netcom.com).*/
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void do_brew ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn;

    if ( !IS_NPC( ch ) && ch->level < skill_table[gsn_brew].skill_level[ch->class] )
    {                                          
		send_to_char( "You do not know how to brew potions.\n\r", ch );
		return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
		send_to_char( "Brew what spell?\n\r", ch );
		return;
    }

    /* Do we have a vial to brew potions? */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
		if ( obj->pIndexData->vnum == OBJ_VNUM_EMPTY_VIAL && obj->wear_loc == WEAR_HOLD )
			break;
    }

    /* Interesting ... Most scrolls/potions in the mud have no hold
       flag; so, the problem with players running around making scrolls 
       with 3 heals or 3 gas breath from pre-existing scrolls has been 
       severely reduced. Still, I like the idea of 80% success rate for  
       first spell imprint, 25% for 2nd, and 10% for 3rd. I don't like the
       idea of a scroll with 3 ultrablast spells; although, I have limited
       its applicability when I reduced the spell->level to 1/3 and 1/4 of 
       ch->level for scrolls and potions respectively. --- JH */


    /* I will just then make two items, an empty vial and a parchment available
       in midgaard shops with holdable flags and -1 for each of the 3 spell
       slots. Need to update the midgaard.are files --- JH */

    if ( !obj )
    {
		send_to_char( "You are not holding a vial.\n\r", ch );
		return;
    }

    if ( ( sn = skill_lookup(arg) )  < 0)
    {
		send_to_char( "You don't know any spells by that name.\n\r", ch );
		return;
    }

    /* preventing potions of gas breath, acid blast, etc.; doesn't make sense
       when you quaff a gas breath potion, and then the mobs in the room are
       hurt. Those TAR_IGNORE spells are a mixed blessing. - JH */
  
    if ( (skill_table[sn].target != TAR_CHAR_DEFENSIVE) && 
         (skill_table[sn].target != TAR_CHAR_SELF)              ) 
    {
		send_to_char( "You cannot brew that spell.\n\r", ch );
		return;
    }

    act( "$n begins preparing a potion.", ch, obj, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_brew].beats );

    /* Check the skill percentage, fcn(wis,int,skill) */
    if ( !IS_NPC(ch) 
         && ( number_percent( ) > ch->pcdata->learned[gsn_brew] ||
              number_percent( ) > ((get_curr_stat(ch, STAT_INT)-13)*5 + 
                                   (get_curr_stat(ch, STAT_WIS)-13)*3) ))
    {
	act( "$p explodes violently!", ch, obj, NULL, TO_CHAR );
	act( "$p explodes violently!", ch, obj, NULL, TO_ROOM );
     act("A blast of acid erupts from $p, burning your skin!", ch, obj, NULL, TO_CHAR);
     act("A blast of acid erupts from $p, burning $n's skin!", ch, obj, NULL, TO_ROOM);
    new_damage( ch, ch, dice(UMIN(ch->level,45),10), skill_lookup("acid blast"),DAM_ACID,FALSE );

	extract_obj( obj );
	return;
    }

    /* took this outside of imprint codes, so I can make do_brew differs from
       do_scribe; basically, setting potion level and spell level --- JH */

    obj->level = ch->level/2;
    obj->value[0] = ch->level/4;
    spell_imprint(sn, ch->level, ch, obj);

}

void do_scribe ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn, dam;


    if ( !IS_NPC( ch )                                                  
	&& ch->level < skill_table[gsn_scribe].skill_level[ch->class] )
    {                                          
	send_to_char( "You do not know how to scribe scrolls.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Scribe what spell?\n\r", ch );
	return;
    }

    /* Do we have a parchment to scribe spells? */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == OBJ_VNUM_BLANK_SCROLL && obj->wear_loc == WEAR_HOLD )
	    break;
    }
    if ( !obj )
    {
	send_to_char( "You are not holding a parchment.\n\r", ch );
	return;
    }


    if ( ( sn = skill_lookup(arg) )  < 0)
    {
	send_to_char( "You don't know any spells by that name.\n\r", ch );
	return;
    }
    
    act( "$n begins writing a scroll.", ch, obj, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_scribe].beats );

    /* Check the skill percentage, fcn(int,wis,skill) */
    if ( !IS_NPC(ch) 
         && ( number_percent( ) > ch->pcdata->learned[gsn_brew] ||
              number_percent( ) > ((get_curr_stat(ch, STAT_INT)-13)*5 + 
                                   (get_curr_stat(ch, STAT_WIS)-13)*3) ))
    {
	act( "$p bursts in flames!", ch, obj, NULL, TO_CHAR );
	act( "$p bursts in flames!", ch, obj, NULL, TO_ROOM );
      act("$n is enveloped in flames!", ch, NULL, NULL, TO_ROOM);
      act("You are enveloped in flames!", ch, NULL, NULL, TO_CHAR);

	dam = dice(UMIN(ch->level,45),7);
    new_damage( ch, ch, dam, skill_lookup("fireball"), DAM_FIRE, FALSE );
	extract_obj( obj );
	return;
    }

    /* basically, making scrolls more potent than potions; also, scrolls
       are not limited in the choice of spells, i.e. scroll of enchant weapon
       has no analogs in potion forms --- JH */

    obj->level = ch->level*2/3;
    obj->value[0] = ch->level/3;
    spell_imprint(sn, ch->level, ch, obj); 

}

/*  edoc  */

AFFECT_DATA *find_sharpen_affect(AFFECT_DATA *ptr,sh_int loc)
{
    bool found=FALSE; /* not found yet */

    /* while we haven't found one and there is still affects left */
    while(!found&&ptr)
    {
        /* is this a sharpen affect ? */
        if(ptr->type!=gsn_sharpen||ptr->location!=loc)
        {
            /* no, so move to next affect */
            ptr=ptr->next;
        }
        else
        {
            /* is a sharpen affect, so we can bail the loop */
            found=TRUE;
        }
    }
    /* return the sharpen affect (or NULL) */
    return(ptr);
}

void remove_sharpness_affect(OBJ_DATA *obj,AFFECT_DATA *aff)
{
    AFFECT_DATA *data=obj->affected;

    /* if it's not the head of the list */
    if(data!=aff)
    {
        /* while it's not the next one in the list && there is stuff left */
        while(data->next&&data->next!=aff)
        {
            /* move to next affect */
            data=data->next;
        }
        /* if the next one */
        if(data->next==aff)
        {
            /* get rid of it */
            data->next=aff->next;
            free_affect(aff);
        }
    }
    else
    {
        /* reset the head of the objects affects */
        obj->affected=data->next;
        free_affect(aff);
    }
}

void do_sharpen(CHAR_DATA *ch,char *arg)
{
    bool ok_to_sharpen=TRUE;
    char object[MAX_INPUT_LENGTH];
    OBJ_DATA *obj=NULL;
    int sharpness,skill_roll,sign;
    AFFECT_DATA *aff_dam=NULL,*aff_hit=NULL;
    sh_int wear=WEAR_NONE;

    /* max +/- affect bonus */
    const int max_affect=4;

    /* do they know how to sharpen ? */
    if(get_skill(ch,gsn_sharpen)<2)
    {
        const char e_msg[] = {"You do not know how to sharpen!\n\r"};

        send_to_char(e_msg,ch);
        ok_to_sharpen=FALSE;
    }

    /* check mounted */
    if(ok_to_sharpen&&MOUNTED(ch))
    {
        const char e_msg[] = {"You can\'t sharpen while riding.\n\r"};

        send_to_char(e_msg,ch);
        ok_to_sharpen=FALSE;
    }
    /* check ridden */
    if(ok_to_sharpen&&RIDDEN(ch))
    {
        const char e_msg[] = {"You can\'t sharpen while being riding.\n\r"};

        send_to_char(e_msg,ch);
        ok_to_sharpen=FALSE;
    }
    if(ok_to_sharpen)
    {
        /* get arg */
        arg=one_argument(arg,object);
        /* not blank */
        if(*object)
        {
            /* attempt to get the object from the char */
            obj=get_obj_carry(ch,object);
            if(!obj)
            {
                const char e_msg[] = {"You don\'t seem to be carrying that!\n\r"};

                send_to_char(e_msg,ch);
                ok_to_sharpen=FALSE;
            }
        }
        else
        {
            const char e_msg[] = {"You must sharpen something!\n\r"};

            send_to_char(e_msg,ch);
            ok_to_sharpen=FALSE;
        }
    }
    /* lets try and sharpen it ! */
    if(ok_to_sharpen)
    {
        /* this object cannot be sharpend UNLESS it's a n edged weapon */
        ok_to_sharpen=FALSE;
        /* is the object valid */
        if(obj)
        {
            /* is it a weapon ? */
	    if(obj->item_type==ITEM_WEAPON)
            {
                /* check the i dunno what for sharpness */
                switch(obj->value[0])
                {
                    /* only "sharp" type weapons */
                    case WEAPON_SWORD:
                    case WEAPON_DAGGER:
                    case WEAPON_SPEAR:
                    case WEAPON_AXE:
                    case WEAPON_SHARP:
                    case WEAPON_SERRATED:
                    {
                        /*
                         * do nothing, just set the can sharpen flag
                         */
                        ok_to_sharpen=TRUE;
                        break;
                    }
                    default: /* not a sharp weapon */
                    {
                        const char e_msg[] = {"You cannot sharpen that!\n\r"};

                        send_to_char(e_msg,ch);
                        break;
                    }
                }
            }
            else /* not a weapon */
	    {
                const char e_msg[] = {"You can\'t sharpen that !\n\r"};
    
                send_to_char(e_msg,ch);
            }
        }
        else /* oopsie, this is a bummer ! */
        {
            const char e_msg[] = {"You fail to sharpen your weapon.\n\r"};
    
            send_to_char(e_msg,ch);
            bug("Sharpen: No object !",0);
        }
    }
    /* can this object be sharpend ? */
    if(ok_to_sharpen)
    { /* yes */
        /* if the object is wielded */
        if(obj==get_eq_char(ch,WEAR_WIELD))
        {
            /* save the wield type */
            wear=WEAR_WIELD;
        }
        /* if the object is second wielded */
        if(obj==get_eq_char(ch,WEAR_SECOND_WIELD))
        {
            /* save the wield type */
            wear=WEAR_SECOND_WIELD;
        }
        /* if it's wielded, unwield */
        if(wear!=WEAR_NONE)
        {
            unequip_char(ch,obj);
        }
        /* do the roll against the chars skill */
        skill_roll=get_skill(ch,gsn_sharpen)-number_percent();
        /* + or - damage ? */
        sign=(skill_roll<0)?-1:1;
        /* make sure we are within 0-100 */
        skill_roll=skill_roll<=100?skill_roll:100;
        /* this is setup for a +/- max affect evenly dist over 100 levels */
        /* calc the sharpness */
        sharpness=(abs(skill_roll)+(100/max_affect))/(100/max_affect);
        /* fix the sign */
        sharpness*=sign;
        /* if we need to sharpen / blunt */
        if(sharpness)
        {
            /* check affected list */
            aff_dam=find_sharpen_affect(obj->affected,APPLY_DAMROLL);
            aff_hit=find_sharpen_affect(obj->affected,APPLY_HITROLL);
            /* couldn't find an affect */
            if(!aff_hit&&!aff_dam)
            {
                /* make new affects */
                aff_hit=new_affect();
                aff_dam=new_affect();
                /* make sure we made both */
                if(aff_hit&&aff_dam)
                {
                    /* clear the data */
                    memset(aff_hit,0,sizeof(AFFECT_DATA));
                    memset(aff_dam,0,sizeof(AFFECT_DATA));
                    /* set location */
                    aff_hit->location=APPLY_HITROLL;
                    aff_dam->location=APPLY_DAMROLL;
                    /* stop the timer */
                    aff_hit->duration=-1;
                    aff_dam->duration=-1;
                    /* set the skill types */
                    aff_hit->type=aff_dam->type=gsn_sharpen;
                    /* set the affect level */
                    aff_hit->level=aff_dam->level=obj->level;
                    /* add them to the object */
                    aff_hit->next=aff_dam;
                    aff_dam->next=obj->affected;
                    obj->affected=aff_hit;
                }
                else
                {
                    /* nope out of memory */
                    const char e_msg[] = {"You fail to sharpen your weapon.\n\r"};

                    send_to_char(e_msg,ch);
                    bug("Sharpen: out of memory",0);
                    /* free the alloc'd affect */
                    if(aff_hit)
                    {
                        free_affect(aff_hit);
                    }
                    /* free the alloc'd affect */
                    if(aff_dam)
                    {
                        free_affect(aff_dam);
                    }
                    /* nullify the pointers */
                    aff_hit=aff_dam=NULL;
                }
            }
            else
            {
                /* sanity check there is another one... */
                if(!aff_dam||!aff_hit)
                {
                    /* not another affect, this is a bug */
                    const char e_msg[] = {"You fail to sharpen your weapon.\n\r"};

                    send_to_char(e_msg,ch);
                    bug("Sharpen: missing an affect",0);
                    aff_dam=aff_hit=0;
                }
            }
        }
        /* if we have valid ptrs */
        if(aff_hit&&aff_dam)
        {
            /* apply the sharpen result */
            aff_hit->modifier+=sharpness;
            /* make sure it's within limits */
            if(aff_hit->modifier>max_affect)
            {
                aff_hit->modifier=max_affect;
            }
            else
            {
                if(aff_hit->modifier<(0-max_affect))
                {
                    aff_hit->modifier=0-max_affect;
                }
            }
            /* apply the sharpen result */
            aff_dam->modifier+=sharpness;
            /* make sure it's within limits */
            if(aff_dam->modifier>max_affect)
            {
                aff_dam->modifier=max_affect;
            }
            else
            {
                if(aff_dam->modifier<(0-max_affect))
                {
                    aff_dam->modifier=0-max_affect;
                }
            }
            /* if the result is 0 */
            if(aff_hit->modifier==0||aff_hit->modifier==0)
            {
                /* then we can remove these modifiers */
                /* sanity check */
                if(aff_hit->modifier!=aff_dam->modifier)
                {
                    const char e_msg[] = {"SHARPEN: modifier mismatch!\n\r"};

                    bug(e_msg,0);
                }
                /* remove the affects */
                remove_sharpness_affect(obj,aff_hit);
                remove_sharpness_affect(obj,aff_dam);
               /* clean up */
               aff_hit=aff_dam=NULL;
            }
            /* see if the char improves the sharpen skill */
            if(sharpness>0)
            {
                const char e_msg[] = {"You sharpen your blade.\n\r"};
                
                send_to_char(e_msg,ch);
	        check_improve(ch,gsn_sharpen,TRUE,1);
            }
            else
            {
                const char e_msg[] = {"You dull your blade.\n\r"};
                
                send_to_char(e_msg,ch);
	        check_improve(ch,gsn_sharpen,FALSE,1);
            }
            /* check for max */
            if(aff_hit)
            {
                if(aff_hit->modifier>=max_affect)
                {
                    const char e_msg[] = {"Your blade is as sharp as you can "
                                          "make it.."};

                    send_to_char(e_msg,ch);
                }
            } 
            /* make player wait */
            WAIT_STATE(ch,skill_table[gsn_sharpen].beats);
        }
        /* if it was wielded, wield again */
        if(wear!=WEAR_NONE)
        {
            equip_char(ch,obj,wear);
        }
    }
}

void adjust_sharpness_affects(CHAR_DATA *ch,OBJ_DATA *obj)
{
    AFFECT_DATA *aff_dam,*aff_hit;
    sh_int wear=WEAR_NONE;
    sh_int mod;

    /* not a null ptr */
    if(ch&&obj)
    {
        /* check affected list */
        aff_dam=find_sharpen_affect(obj->affected,APPLY_DAMROLL);
        aff_hit=find_sharpen_affect(obj->affected,APPLY_HITROLL);
        /* if we found them both */
        if(aff_dam&&aff_hit)
        {
            /* if the object is wielded */
            if(obj==get_eq_char(ch,WEAR_WIELD))
            {
                /* save the wield type */
                wear=WEAR_WIELD;
            }
            /* if the object is second wielded */
            if(obj==get_eq_char(ch,WEAR_SECOND_WIELD))
            {
                /* save the wield type */
                wear=WEAR_SECOND_WIELD;
            }
            /* if wielded, unwield */
            if(wear!=WEAR_NONE)
            {
                /* unwield */
                unequip_char(ch,obj);
            }
            /* check the bonus */
            if(aff_hit->modifier!=aff_dam->modifier)
            {
                const char e_msg[] = {"Adj Sharp: dam != hit?\n\r"};

                aff_hit->modifier=aff_dam->modifier;
                bug(e_msg,0);
            }
            /* adjust the  bonus' */
            /* check the sign */
            mod=aff_hit->modifier>0?-1:1;
            /* adjust */
	    aff_hit->modifier+=mod;
            aff_dam->modifier+=mod;
            /* check for Z3R0 */
            if(!aff_dam->modifier)
            {
                const char e_msg[] = {"Your blade returns to normal.\n\r"};

                send_to_char(e_msg,ch);
                /* remove the affects */
                remove_sharpness_affect(obj,aff_hit);
                remove_sharpness_affect(obj,aff_dam);
                /* null-ify the ptrs */
                aff_hit=aff_dam=NULL;
            }
            /* if is was wielded */
            if(wear!=WEAR_NONE)
            {
                /* re-wield */
                equip_char(ch,obj,wear);
            }
        }
        else
        {
            /* if we only found one, it's a bug */
            if(aff_dam||aff_hit)
            {
                const char e_msg[] = {"Adj Sharp: Only one affect!\n\r"};

                bug(e_msg,0);
            }
        }
    }
    else
    { /* bug */

        if(!ch)
        {
            const char e_msg[] = {"Ajd Sharp: Null char object !\n\r"};

            bug(e_msg,0);
        }
    }
}

/* #@#@ edoc #@#@ */
