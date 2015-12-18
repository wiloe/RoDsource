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
#include "grant.h"
#include "interp.h"

/*
 * Locals
 */
char *flag_string               args ( ( const struct flag_type *flag_table, 
                                         int bits ) );
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );

/* command procedures needed */
DECLARE_DO_FUN(do_sever         );
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_force		);


bool     check_parse_name        args( ( char *name ) );  // For rename.
/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );

/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if (ch->level > 5 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
    }

    if ( ( obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    { 
        obj = create_object( get_obj_index(class_table[ch->class].weapon),0);
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_WIELD );
    }

    send_to_char("You have been equipped by the gods.\n\r",ch);
}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel whom?", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have restored your channel priviliges.\n\r", 
		      victim );
        send_to_char( "NOCHANNELS removed.\n\r", ch );
        sprintf(buf,"$N restores channels to %s",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have revoked your channel priviliges.\n\r", 
		       victim );
        send_to_char( "NOCHANNELS set.\n\r", ch );
        sprintf(buf,"$N revokes %s's channels.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0'; /*spellsong add*/

	if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r`w",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );
	strcat(ch->pcdata->bamfin, "`w");

        sprintf(buf,"Your poofin is now %s\n\r`w",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    buf[0] = '\0'; /*spellsong add*/
	
	if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r`w",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
        strcat(ch->pcdata->bamfout, "`w");
 
        sprintf(buf,"Your poofout is now %s\n\r`w",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    do_quit( victim, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
   send_to_char( "Descriptor not found!\n\r", ch );
   return;
}

void do_new_discon( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   int sock_num;
   
   one_argument( argument, arg );
   if ( arg[0] == '\0' )
     {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
     }
   
   if ( !is_number(arg) )
     {
	send_to_char("Argument must be numeric, use sockets to find number.\n\r", ch);
	return;
     }
   sock_num = atoi(arg);
   
   for ( d = descriptor_list; d != NULL; d = d->next )
     {
	if ( d->descriptor == sock_num )
	  {
	     close_socket( d );
	     send_to_char( "Ok.\n\r", ch );
	     return;
	  }
     }
   
   send_to_char( "Descriptor not on list, use sockets.\n\r", ch );
   return;
}


void do_pardon( CHAR_DATA *ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
	if ( IS_SET(victim->act, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Killer flag removed.\n\r", ch );
	    send_to_char( "You are no longer a KILLER.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "thief" ) )
    {
	if ( IS_SET(victim->act, PLR_THIEF) )
	{
	    REMOVE_BIT( victim->act, PLR_THIEF );
	    send_to_char( "Thief flag removed.\n\r", ch );
	    send_to_char( "You are no longer a THIEF.\n\r", victim );
	}
	return;
    }

    send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( "`W", d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r`w",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (get_trust(d->character) >= get_trust(ch))
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r`w",   d->character );
	}
    }

    return;
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r`w",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r`w",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( location ) && get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    if (victim->at_obj !=NULL)
     char_off_obj(victim);
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );

    if( MOUNTED(victim) )
    {
        char_from_room( MOUNTED(victim) );
        char_to_room( MOUNTED(victim), location );
        send_to_char("Your rider is being transferred, and so are you.\n\r", MOUNTED(victim));
    }

    if (ch->pet != NULL)
    {
       char_from_room (ch->pet);
       char_to_room (ch->pet, location);
    }

    if( MOUNTED(ch) )
    {
        char_from_room( MOUNTED(ch) );
        char_to_room( MOUNTED(ch), location );
        send_to_char("Your rider is a god, and did a goto. You went along for the ride.\n\r",MOUNTED(ch));
    }   

    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) && get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}

void do_repop( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    int vnum;
    char buf[200];
    bool found_area=FALSE;
    
	buf[0] = '\0'; /*spellsong add*/
    
	if ( argument[0] == 0 ) {
        reset_area(ch->in_room->area);
        send_to_char("Area repop!\n\r", ch);
    }
    if ( is_number(argument) ) {
    	vnum=atoi(argument);
        for (pArea = area_first; pArea; pArea = pArea->next )
        {
            if (pArea->vnum == vnum) {
                sprintf(buf, "%s has been repoped!\n\r", &pArea->name[8]);
                send_to_char(buf, ch);
                found_area=TRUE;
                reset_area(pArea);
            }
        }
        if (!found_area) send_to_char("No such area!\n\r",ch);
    }
    if ( !strcmp(argument, "all") ) {
    	for (pArea = area_first; pArea; pArea = pArea->next )
    	{
    	    reset_area(pArea);
    	}
    	send_to_char("World repop!\n\r", ch);
    }
    return;
}


void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) && get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_look( ch, "auto" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }
   
   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

   



void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    buf[0] = '\0'; /*spellsong add*/
	
	one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private( location ) && 
	 get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s.'\n\rArea: '%s'.\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d.  Sector: %d.  Light: %d.\n\r",
	location->vnum,
	location->sector_type,
	location->light );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %d.\n\rDescription:\n\r%s",
	location->room_flags,
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    sprintf( buf, "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_type_name(obj), obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf(buf, "Furniture bits: %s\n\r", flag_string(furniture_flags, obj->furn_flags));
    send_to_char(buf, ch);
    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "someone",
	(int)obj->wear_loc );    //type cast added - Lotex (2/27/00)
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )
    {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf( buf, "Level %d spells of:", obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[1]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[2]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

		send_to_char( ".\n\r", ch );
	break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf( buf, "Has %d(%d) charges of level %d",
	    	obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );
      
	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    send_to_char( ".\n\r", ch );
	break;
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC) 	: send_to_char("exotic\n\r",ch);	break;
	    	case(WEAPON_SWORD)  	: send_to_char("sword\n\r",ch);	break;	
	    	case(WEAPON_DAGGER) 	: send_to_char("dagger\n\r",ch);	break;
	    	case(WEAPON_SPEAR)	: send_to_char("spear/staff\n\r",ch);	break;
	    	case(WEAPON_MACE) 	: send_to_char("mace/club\n\r",ch);	break;
	   	case(WEAPON_AXE)	: send_to_char("axe\n\r",ch);		break;
	    	case(WEAPON_FLAIL)	: send_to_char("flail\n\r",ch);	break;
	    	case(WEAPON_WHIP)	: send_to_char("whip\n\r",ch);		break;
	    	case(WEAPON_POLEARM)	: send_to_char("polearm\n\r",ch);	break;
	    	default			: send_to_char("unknown\n\r",ch);	break;
 	    }
	    if (obj->pIndexData->new_format)
	    	sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
	    	sprintf( buf, "Damage is %d to %d (average %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: %s\n\r",weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	break;

    	case ITEM_ARMOR:
	    sprintf( buf, 
	    	"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r", 
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    buf[0] = '\0'; /*spellsong add*/

	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: %s.\n\r",
	victim->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Race: %s  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "neutral",
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }

    sprintf( buf, 
   	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(ch) ? 0 : victim->practice );
    send_to_char( buf, ch );
	
    sprintf( buf,
	"Lv: %d  Class: %s  Align: %d  Gold: %ld  Exp: %ld\n\r",
	victim->level,       
	IS_NPC(victim) ? "mobile" : class_table[victim->class].name,            
	victim->alignment,
	victim->gold,         victim->exp );
    send_to_char( buf, ch );

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, "Hit: %d  Dam: %d  Saves: %d  Position: %d  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	victim->position,    victim->wimpy );
    send_to_char( buf, ch );

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].name);
	send_to_char(buf,ch);
    }
    sprintf( buf, "Fighting: %s\n\r",
	victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d  Full: %d  Drunk: %d\n\rRacial Ability Uses: %d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->racial_abil );
	send_to_char( buf, ch );


    }

    sprintf( buf, "Carry number: %d  Carry weight: %d\n\r",
	victim->carry_number, victim->carry_weight );
    send_to_char( buf, ch );


    if (!IS_NPC(victim))
    {
    	sprintf( buf, 
	    "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	    get_age(victim), 
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
    	send_to_char( buf, ch );
    }
     if (!IS_NPC(victim))
     {
       sprintf( buf, "Recall: %d\n\r", victim->pcdata->recall_room->vnum);
       send_to_char( buf, ch );
     }
            
    sprintf(buf, "Clan: %s\n\r", IS_NPC(victim) ? vis_clan(victim->pIndexData->clan)
                : vis_clan(victim->pcdata->clan));
    send_to_char(buf,ch);

    sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);
    
    if (victim->comm)
    {
    	sprintf(buf,"Comm: %s\n\r",comm_bit_name(victim->comm));
    	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
    	sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: %s\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist: %s\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    sprintf(buf, "Form: %s\n\rParts: %s\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    sprintf( buf, "Master: %s  Leader: %s  Pet: %s  Horse: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)",
        victim->mount       ? victim->mount->name    : "(none)");
    send_to_char( buf, ch );

   if ( !IS_NPC(victim))
     {
	sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );	/* OLC */
	send_to_char( buf, ch );					/* OLC */
     }
   
    sprintf( buf, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
	send_to_char( "Mobile has special procedure.\n\r", ch );

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( paf->bitvector ),
	    paf->level
	    );
	send_to_char( buf, ch );
    }

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }
    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    buf[0] = '\0'; /*spellsong add*/
	
	one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

	buf[0] = '\0'; /*spellsong add*/

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );

    return;
}

/*Owhere, ported from rom 2.4 */

void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    buf[0] = '\0'; /*spellsong add*/
	found = FALSE;
    number = 0;
    max_found = 200;

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }

     buffer[0] = '\0';
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name )
	||   ch->level < obj->level)
	    continue;

	found = TRUE;
	number++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
	    sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
		number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
	else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
	    sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
		number, obj->short_descr,in_obj->in_room->name,
		in_obj->in_room->vnum);
	else
	    sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);

	buf[0] = UPPER(buf[0]);
	strcat(buffer,buf);

	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
       if (ch->lines)
	page_to_char(buffer,ch);
       else
	send_to_char(buffer,ch);
}




void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool found;

	buf[0] = '\0'; /*spellsong add*/

    if ( argument[0] == '\0' )
    {
	send_to_char( "Mwhere whom?\n\r", ch );
	return;
    }

    found = FALSE;
    buffer[0] = '\0';
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( IS_NPC(victim)
	&&   victim->in_room != NULL
	&&   is_name( argument, victim->name ) )
	{
	    found = TRUE;
	    sprintf( buf, "[%5d] %-28s [%5d] %s\n\r",
		victim->pIndexData->vnum,
		victim->short_descr,
		victim->in_room->vnum,
		victim->in_room->name );
	    strcat(buffer,buf);
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
	if (ch->lines)
	    page_to_char(buffer,ch);
	else
	    send_to_char(buffer,ch);

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    extern bool chaos;

	buf[0] = '\0'; /*spellsong add*/

    if (chaos)
    {
     send_to_char("Please remove `rC`RH`YA`RO`rS`w before rebooting.\n\r", ch);
     return;
    }

    if (!IS_SET(ch->act,PLR_WIZINVIS))
    {
    	sprintf( buf, "Reboot by %s.", ch->name );
    	do_echo( ch, buf );
    }
    do_force ( ch, "all save");
    do_save (ch, "");
    do_asave (ch, "changed");
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
    	close_socket(d);
    }
    
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    extern bool chaos;

	buf[0] = '\0'; /*spellsong add*/
    
	if (chaos)
    {
     send_to_char("Please remove `rC`RH`YA`RO`rS`w before shutting down.\n\r", ch);
     return;
    }

    if (!IS_SET(ch->act,PLR_WIZINVIS))
    {
    	sprintf( buf, "Shutdown by %s.", ch->name );
    	append_file( ch, SHUTDOWN_FILE, buf );
    }
    strcat( buf, "\n\r" );
    do_echo( ch, buf );
    do_force ( ch, "all save");
    do_save (ch, "");
    do_asave (ch, "changed");
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	close_socket(d);
    }
    return;
}



void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
        wiznet("$N stops being such a snoop.",
                ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf,"$N starts snooping on %s",
        (IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));


    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    /*make sure the victim has the right imm commands*/
    victim->secflags=ch->secflags;
    victim->infflags=ch->secflags;
    victim->funflags=ch->secflags;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->secflags=0;
    ch->infflags=0;
    ch->funflags=0;
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
        wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
        char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
        sprintf(buf,"$N clones %s.",clone->short_descr);
        wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

	act( "`mYou are surrounded by purple flames as $n purges the room.`w", ch, NULL, NULL, TO_ROOM);
	send_to_char( "`mYou are surrounded in all consuming flames.\n\r`w", ch );
	return;
    }

    if ( (obj = get_obj_list( ch, arg, ch->in_room->contents ) ) != NULL)
    {
	extract_obj( obj );
	act( "You disintegrate $p.", ch, obj, NULL, TO_CHAR);
	act( "$n disintegrates $p.", ch, obj, NULL, TO_ROOM);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or hell.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);
	act( "You disintegrate $N.", ch, 0,victim, TO_CHAR);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( "You disintegrate $N.", ch, 0,victim, TO_CHAR);
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 1 to 100.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
	temp_prac = victim->practice;
	victim->level    = 1;
	victim->exp      = 0;
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level( victim );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	send_to_char( "You raise a level!!  ", victim );
	victim->level += 1;
	advance_level( victim );
    }
    victim->exp   = 0;
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 100.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);
            
            vch->hit 	= vch->max_hit;
            vch->mana	= vch->max_mana;
            vch->move	= vch->max_move;
            update_pos( vch);
            act("$n has restored you.",ch,NULL,vch,TO_VICT);
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));

        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            affect_strip(victim,gsn_plague);
            affect_strip(victim,gsn_poison);
            affect_strip(victim,gsn_blindness);
            affect_strip(victim,gsn_sleep);
            affect_strip(victim,gsn_curse);
            
            victim->hit 	= victim->max_hit;
            victim->mana	= victim->max_mana;
            victim->move	= victim->max_move;
            update_pos( victim);
	    if (victim->in_room != NULL)
                act("$n has restored you.",ch,NULL,victim,TO_VICT);
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
        IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
        sprintf(buf,"$N thaws %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
        sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
        sprintf(buf,"$N restores emotes to %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
        sprintf(buf,"$N revokes %s's emotes.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
        sprintf(buf,"$N restores shouts to %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
        sprintf(buf,"$N revokes %s's shouts.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
        sprintf(buf,"$N restores tells to %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
        sprintf(buf,"$N revokes %s's tells.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



BAN_DATA *		ban_free;
BAN_DATA *		ban_list;

void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

	buf[0] = '\0'; /*spellsong add*/
    
	if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	strcpy( buf, "Banned sites:\n\r" );
	for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
	    strcat( buf, pban->name );
	    strcat( buf, "\n\r" );
	}
	send_to_char( buf, ch );
	return;
    }

    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->name ) )
	{
	    send_to_char( "That site is already banned!\n\r", ch );
	    return;
	}
    }

    if ( ban_free == NULL )
    {
	pban		= alloc_perm( sizeof(*pban) );
    }
    else
    {
	pban		= ban_free;
	ban_free	= ban_free->next;
    }

    pban->name	= str_dup( arg );
    pban->next	= ban_list;
    ban_list	= pban;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if ( !str_cmp( arg, curr->name ) )
	{
	    if ( prev == NULL )
		ban_list   = ban_list->next;
	    else
		prev->next = curr->next;

	    free_string( curr->name );
	    curr->next	= ban_free;
	    ban_free	= curr;
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock ) {
        wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    } else {
        wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if ( newlock ) {
        wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    } else {
        wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
   char arg1 [MAX_INPUT_LENGTH];
   char arg2 [MAX_INPUT_LENGTH];
   char arg3 [MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *Clan;
   CLAN_DATA *Clan2;
   ROOM_INDEX_DATA *location;
   int value;
   char *tmpmem[MAX_CLAN_MEMBERS];
   int tmpcount, tmpcount2=0;
   
   buf[0] = '\0'; /*spellsong add*/
   
   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   strcpy( arg3, argument );
   
    if (( arg1[0] == '\0') || (arg2[0] == '\0') || (arg3[0] == '\0'))
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    str int wis dex con sex class level\n\r",	ch );
	send_to_char( "    race gold hp mana move practice align\n\r",	ch );
	send_to_char( "    train thirst drunk full security recall\n\r",ch );
	send_to_char( "    clan nokilling\n\r",ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity ranges is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
        	strcpy( buf, "Possible classes are: " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[class].name );
        	}
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->class = class;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL )
	{
	    send_to_char( "Level range is 0 to 100.\n\r", ch );
	    return;
	}
	victim->level = value;
	return;
    }
    if ( !str_prefix( arg2, "clan" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on mobiles (yet).\n\r", ch );
	    return;
	}

	
       if ( !str_prefix( arg3, "none" ))
	 {
	    if (victim->pcdata->clan <= 0)
	      {
		 sprintf(buf,"%s is not a member of a clan!\n\r",victim->name);
		 send_to_char(buf,ch);
		 return;
	      }
	    sprintf(buf,"%d",victim->pcdata->clan);
	    Clan=find_clan(buf);
	    for (tmpcount=0 ;tmpcount < Clan->num_members ; tmpcount++)
	      {
		 if (Clan->members[tmpcount]!=victim->name)
		   {
		      tmpmem[tmpcount2]=Clan->members[tmpcount];
		      tmpcount2++;
		   }
	      }
	    Clan->num_members = Clan->num_members-1;
	    for (tmpcount=0 ; tmpcount < Clan->num_members ; tmpcount++)
	      {
		 Clan->members[tmpcount] = tmpmem[tmpcount];
	      }
	    Clan->members[Clan->num_members+1]=NULL;
	    sprintf(buf,"%s is no longer a member of a clan.\n\r",victim->name);
	    send_to_char(buf, ch);
	    victim->pcdata->clan = 0;
	    do_asave(ch, "clans");
	    return;
	 }
       Clan=find_clan(arg3);
       if (Clan==NULL) 
	 {
	    send_to_char( "No such clan.\n\r", ch);
	    return;
	 }
       else 
	 {
	    sprintf(buf,"%d",victim->pcdata->clan);
	    Clan2=find_clan(buf);
	    if (Clan2!=NULL)
	      {
		 for (tmpcount=0 ; tmpcount < Clan2->num_members ; tmpcount++)
		   {
		      if (Clan2->members[tmpcount]!=victim->name)
			{
			   tmpmem[tmpcount2]=Clan2->members[tmpcount];
			   tmpcount2++;
			}
		   }
		 sprintf(buf,"%s is no longer a member of clan %s.\n\r",victim->name,Clan2->visible);
		 send_to_char( buf, ch);
		 Clan2->num_members = Clan2->num_members-1;
	      }
	    Clan->members[Clan->num_members]=str_dup(victim->name);
	    Clan->num_members = Clan->num_members+1;
	    sprintf(buf,"%s is now a member of clan %s.\n\r",victim->name,Clan->visible);
	    send_to_char( buf, ch);
	    victim->pcdata->clan = Clan->number;
	    do_asave(ch, "clans");
	    return;
	 }  
       return;
    }
   
   if ( !str_prefix( arg2, "gold" ) )
     {
	victim->gold = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 50 )
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }

    if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if ( !str_cmp( arg2, "recall" ) )	/* Thexder */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
    	if ( ( location = find_location( ch, arg3 ) ) == NULL )
    	{
            send_to_char( "That is an invalid location.\n\r", ch );
            return;
    	}
    	else 
    	{
	    victim->pcdata->recall_room = location;
	    send_to_char( "Recall set.\n\r", ch);
    	    return;
    	}
    }

    if ( !str_cmp( arg2, "nokilling" ) )   /* Eris 27 Sept 2000 */
    {
	if (IS_NPC( victim ) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
	victim->pcdata->nokilling = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
            string_append( ch, &victim->description );
            return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}
    }
    
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}
    	
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    if ( extra_descr_free == NULL )
	    {
	        ed			= alloc_perm( sizeof(*ed) );
	    }
	    else
	    {
	    	ed			= extra_descr_free;
	    	extra_descr_free	= ed->next;
	    }

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}



void do_sockets( CHAR_DATA *ch, char *argument )
{
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;

    count	= 0;
    buf[0]	= '\0';
	buf2[0] = '\0'; /*spellsong add*/
    
	one_argument(argument,arg);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character != NULL && can_see( ch, d->character ) 
	&& (arg[0] == '\0' || is_name(arg,d->character->name)
			   || (d->original && is_name(arg,d->original->name))))
	{
	    count++;
	    sprintf( buf + strlen(buf), "[%3d %2d] %-15s %-30s   Idle: %d ticks\n\r",
		d->descriptor,
		d->connected,
		d->original  ? d->original->name  :
		d->character ? d->character->name : "(none)",
		d->host,
		d->character->timer
		);
	}
    }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat(buf,buf2);
    page_to_char( buf, ch );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
	    &&	 vch->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
            &&   vch->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) 
    /* take the default path */

      if ( IS_SET(ch->act, PLR_WIZINVIS) )
      {
	  REMOVE_BIT(ch->act, PLR_WIZINVIS);
	  ch->invis_level = 0;
	  act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
	  SET_BIT(ch->act, PLR_WIZINVIS);
	  ch->invis_level = get_trust(ch);
	  act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  ch->reply = NULL;
          SET_BIT(ch->act, PLR_WIZINVIS);
          ch->invis_level = level;
          act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }

    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}
void do_chaos( CHAR_DATA *ch, char *argument )
{
    extern bool chaos;

if (!chaos)
   {
    do_force ( ch, "all save");
    do_save (ch, "");
   }

    chaos = !chaos;

    if ( chaos )
	{
	send_to_char( "Chaos now set.\n\r", ch );
	do_sendinfo(ch , "`rC`RH`YA`RO`rS`R has begun!");
	}
    else
	{
	send_to_char( "Chaos cancelled.\n\r", ch );
	do_sendinfo(ch, "`rC`RH`YA`RO`rS`R has been cancelled.");
	}

    return;
}

void do_rlist(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH]; 
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA     *pRoomIndex;
    AREA_DATA           *pArea;
    BUFFER              *buf1;
    bool found;
    int vnum;
    int avnum;
    int  col = 0;

	buf[0] = '\0'; /*spellsong add*/

    one_argument( argument, arg );
    buf1=new_buf();
    found   = FALSE;

    if (arg[0]=='\0')
    {
       pArea = ch->in_room->area;

       for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
         {
           if ( ( pRoomIndex = get_room_index( vnum ) ) )
             {
               found = TRUE;
               sprintf( buf, "[%5d] %-17.16s`w",
               pRoomIndex->vnum, capitalize( pRoomIndex->name ) );
               add_buf( buf1, buf );
               if ( ++col % 3 == 0 )
               add_buf( buf1, "\n\r" );
             }
         }

      if ( !found )
      {
         send_to_char( "No rooms found in this area.\n\r", ch);
         return;
      }

     if ( col % 3 != 0 )
        add_buf( buf1, "\n\r" );
       
      page_to_char( buf_string(buf1), ch );
      free_buf(buf1);
      return;
    }

 else if ( is_number(argument) ) 

    {
      avnum=atoi(argument);
        for (pArea = area_first; pArea; pArea = pArea->next )
          {
             if (pArea->vnum == avnum) 
                {
                   found=TRUE;
                   break;
                }
          }

      if (!found) 
        {
           send_to_char("No such area!\n\r",ch);
           return;
        }

      found=FALSE;

      for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
         {
           if ( ( pRoomIndex = get_room_index( vnum ) ) )
             {
               found = TRUE;
               sprintf( buf, "[%5d] %-17.16s`w",
               pRoomIndex->vnum, capitalize( pRoomIndex->name ) );
               add_buf( buf1, buf );
               if ( ++col % 3 == 0 )
               add_buf( buf1, "\n\r" );
             }
         }

     if ( col % 3 != 0 )
        add_buf( buf1, "\n\r" );

      page_to_char( buf_string(buf1), ch );
      free_buf(buf1);
      return;


   }
}


void do_test ( CHAR_DATA *ch, char *argument )
{
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument( argument, arg1 );
   strcpy(arg2, argument);

   if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
     {
	send_to_char("That player either doesn't exist, or is not currently logged in.\n\r", ch);
	send_to_char("You may currently only test players who are online.\n\r", ch);
	return;
     }
   if ( IS_NPC (victim) )
     {
	send_to_char("Mobiles don't need to be tested to join a clan, silly!\n\r", ch);
	return;
     }
   if (clan_accept(victim, arg2) == 0)
     {
	send_to_char("That player would be accepted by that clan.\n\r", ch);
	return;
     }
   if (clan_accept(victim, arg2) == 1)
     {
	send_to_char("That clan doesn't exist (clan_accept failure #1).\n\r", ch);
     }
   if (clan_accept(victim, arg2) == 2)
     {
	send_to_char("That clan has reached max members! (clan_accept failure #2).\n\r", ch);
     }
   if (clan_accept(victim, arg2) == 3)
     {
	send_to_char("That player isn't high enough level. (clan_accept failure #3).\n\r", ch);
     }
   if (clan_accept(victim, arg2) == 4)
     {
	send_to_char("That clan requires PK and that player is not. (clan_accept failure #4).\n\r", ch);
     }
   if (clan_accept(victim, arg2) == 5)
     {
	send_to_char("That player's class isn't accepted in that clan. (clan_accept failure #5).\n\r", ch);
     }
   if (clan_accept(victim, arg2) == 6)
     {
	send_to_char("That player's race isn't accepted in that clan. (clan_accept failure #6).\n\r", ch);
     }
   if (clan_accept(victim, arg2) == 7)
     {
	send_to_char("That player is already a member of that clan!\n\r", ch);
     }
   return;
}

void do_raffect(CHAR_DATA *ch, char *argument)
{
   RAFFECT_DATA *raf;
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   int  value;
   
   buf[0]='\0';
   smash_tilde(argument);
   argument = one_argument( argument, arg1);
   argument = one_argument( argument, arg2);
   argument = one_argument( argument, arg3);
   strcpy(arg4, argument);
   
   value = is_number( arg4 ) ? atoi( arg4 ) : -1;
   
   if (arg1[0] == '\0' )
     {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  raffect create <type>\n\r",ch);
	send_to_char("  raffect show <num>\n\r",ch);
	send_to_char("  raffect list\n\r",ch);
	send_to_char("  raffect kill <num>\n\r",ch);
	send_to_char("  raffect set <num> <field> <value>\n\r",ch);
	send_to_char("    Fields: type, damname, roommsg, victmsg,\n\r",ch);
	send_to_char("            timer, v0, v1, v2\n\r",ch);
	send_to_char("Type 'help raffect' for a list of room affect types.\n\r",ch);
	return;
     }
   if (!str_cmp( arg1, "create") )
     {
	if ((raf = new_raffect ( arg2 )) == NULL)
	  {
	     send_to_char("Type 'help raffect' for a list of room affect types.\n\r",ch);
	     return;
	  }
	/* write some function to do this for me */
	return;
     }
   /* end */
   do_raffect(ch, "");
   return;
}

	
	
	
	
void do_sever( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

	buf[0] = '\0'; /*spellsong add*/
    
	one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
       send_to_char("Who are you going to rip in half?\n\r",ch);
       return; 
    }
    
    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) 
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if(victim == ch)
    {
        send_to_char("You reach your neck fine, but cant' get your ankles.\n\r",ch);
        return;
    }

    if(IS_AFFECTED(victim,AFF_SEVERED))
    {
        send_to_char("They have no legs to rip off.\n\r",ch);
        return;
    }

    SET_BIT(victim->affected_by,AFF_SEVERED);
    if(!IS_NPC(victim))
    act("`R$n picks you up and rips you in half! Oh no! What a mess!",ch,NULL,victim,TO_VICT);
    act("`R$n picks up $N and rips $S legs off!",ch,NULL,victim,TO_NOTVICT);
    send_to_char("You rip them in half!\n\r",ch);

    obj = create_object( get_obj_index(OBJ_VNUM_LEGS ), 0 );

    obj->timer = 5;

   
if(IS_NPC(victim))
    sprintf(buf,"A pair of %s's legs are here, twitching.",victim->short_descr);
else
    sprintf(buf,"A pair of %s's legs are here, twitching.",victim->name);       
    free_string(obj->description);
    obj->description = str_dup( buf );


if(IS_NPC(victim))
    sprintf(buf,"%s's bloody, severed legs",victim->short_descr);
else
    sprintf(buf,"%s's bloody, severed legs",victim->name);    
    free_string(obj->short_descr);
    obj->short_descr = str_dup( buf );

    obj_to_char(obj,ch);

}	
	
	
void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
	if ( argument[0] == '\0' )
    {
        if (IS_SET(ch->wiznet,WIZ_ON))
        {
            send_to_char("Signing off of Wiznet.\n\r",ch);
            REMOVE_BIT(ch->wiznet,WIZ_ON);
        }
        else
        {
            send_to_char("Welcome to Wiznet!\n\r",ch);
            SET_BIT(ch->wiznet,WIZ_ON);
        }
        return;
    }

    if (!str_prefix(argument,"on"))
    {
        send_to_char("Welcome to Wiznet!\n\r",ch);
        SET_BIT(ch->wiznet,WIZ_ON);
        return;
    }

    if (!str_prefix(argument,"off"))
    {
        send_to_char("Signing off of Wiznet.\n\r",ch);
        REMOVE_BIT(ch->wiznet,WIZ_ON);
        return;
    }

    /* show wiznet status */
    if (!str_prefix(argument,"status"))
    {
        buf[0] = '\0';

        if (!IS_SET(ch->wiznet,WIZ_ON))
            strcat(buf,"off ");

        for (flag = 0; wiznet_table[flag].name != NULL; flag++)
            if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
            {
                strcat(buf,wiznet_table[flag].name);
                strcat(buf," ");
            }

        strcat(buf,"\n\r");

        send_to_char("Wiznet status:\n\r",ch);
        send_to_char(buf,ch);
        return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
    {
        buf[0] = '\0';

        for (flag = 0; wiznet_table[flag].name != NULL; flag++)
        {
            if (wiznet_table[flag].level <= get_trust(ch))
            {
                strcat(buf,wiznet_table[flag].name);
                strcat(buf," ");
            }
        }

        strcat(buf,"\n\r");

        send_to_char("Wiznet options available to you are:\n\r",ch);
        send_to_char(buf,ch);
        return;
    }

    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
        send_to_char("No such option.\n\r",ch);
        return;
    }

    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
        sprintf(buf,"You will no longer see %s on wiznet.\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
    }
    else
    {
        sprintf(buf,"You will now see %s on wiznet.\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        SET_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
    }

}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
            long flag, long flag_skip, int min_level)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH*2];

	buf[0] = '\0'; /*spellsong add*/
    
	for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING
        &&  IS_IMMORTAL(d->character)
        &&  IS_SET(d->character->wiznet,WIZ_ON)
        &&  (!flag || IS_SET(d->character->wiznet,flag))
        &&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
        &&  get_trust(d->character) >= min_level
        &&  d->character != ch)
        {
	    sprintf( buf, "`g%s`w", string );
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
		send_to_char("`g--> `w",d->character);
	    act_new(buf,d->character,obj,ch,TO_CHAR,POS_DEAD);
	}
    }

    return;
}

/*From: takeda@mathlab.sunysb.edu

Snippets of a Immortal commanded
called avator which let an immortal to level down to a certain level by
type level blah  you can use your character to test stuff and also your
character can also die :)
i am also commenting this so you newbies coders can understand it and can
help learn how to code faster :)
got lazy on the commenting at the end...wonder if i teacher is on this
list if so i'm dead :p.
  */
/* Written by takeda (takeda@mathlab.sunysb.edu) */
void do_avatar( CHAR_DATA *ch, char *argument ) /* Procedure Avatar */
{  /* Declaration */
    char buf[MAX_STRING_LENGTH];	/* buf */
    char arg1[MAX_INPUT_LENGTH];	/* arg1 */
    OBJ_DATA *obj_next;			/* obj data which is a pointer */
     OBJ_DATA *obj;			/* obj */
    int level;				/* level */
    int iLevel;				/* ilevel */

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument( argument, arg1 );
/* Check statements */
   if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char( "Syntax: avatar <level>.\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

	if ( ( level = atoi( arg1 ) ) < 1 || level > MAX_LEVEL )
	{
       sprintf(buf, "Level must be 1 to %d.\n\r", MAX_LEVEL );
	send_to_char( buf, ch );
      return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
        sprintf(buf, "Your Trust is %d.\n\r",ch->trust);
        send_to_char(buf,ch);
	return;
    }
/* Your trust stays so you will have all immortal command */
        /* SO their trust stays there */
	if(ch->trust == 0) {
        ch->trust = ch->level;
	}
/* Level gains*/
   if ( level <= ch->level )
    {
       int temp_prac;

        send_to_char( "Lowering a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r",ch );
	temp_prac = ch->practice;
	ch->level    = 1;
	ch->exp      = exp_per_level(ch,ch->pcdata->points);
	ch->max_hit  = 20;
	ch->max_mana = 100;
	ch->max_move = 100;
	ch->practice = 0;
	ch->hit      = ch->max_hit;
	ch->mana     = ch->max_mana;
	ch->move     = ch->max_move;
	advance_level( ch );
	ch->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", ch);
    }

    for ( iLevel = ch->level ; iLevel < level; iLevel++ )
    {
	ch->level += 1;
	advance_level( ch );
    }
    sprintf(buf,"You are now level %d.\n\r",ch->level);
    send_to_char(buf,ch);
    ch->exp   = 1000*ch->level;
    /*exp_per_level(ch,ch->pcdata->points)
		  * UMAX( 1, ch->level );*/
    ch->practice=100;
    ch->train=100;
/* Forces the person to remove all the eq....  so level restriction still apply */
   if(ch->level < LEVEL_HERO) {
  for (obj = ch->carrying; obj; obj = obj_next)
    {
   obj_next = obj->next_content;

    if (obj->wear_loc != WEAR_NONE && can_see_obj (ch, obj))
    remove_obj (ch, obj->wear_loc, TRUE);

    }
}
    save_char_obj(ch); /* save character */
    return;
}

/*******DO_GRANT and DO_REVOKE***************/
/*Ported from shadowlight, code by Nir Soffer [Scorpios]
 *Ported by Kat, for Realms of Discordia
 *grants and revokes imm commands
 *Please don't use packs, as they are not worked out for this mud yet
 *
 * ***bug fixes by Spellsong***
 */

void do_grant( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found = FALSE;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: grant <char> <command>\n\r",ch);
	return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't playing.\n\r", ch );
	return;
    }
    if (! str_prefix ( arg2, "all"    )	)
    {
	for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	{
		if ( victim->level >= cmd_table[cmd].level )
		{
			switch ( cmd_table[cmd].group )
			{
				case GROUP_SEC:
				SET_BIT (victim->secflags , cmd_table[cmd].flag );
				break;
				case GROUP_FUN:
				SET_BIT (victim->funflags , cmd_table[cmd].flag );
				break;
				case GROUP_INF:
				SET_BIT (victim->infflags , cmd_table[cmd].flag );
			}
		}
	}
	send_to_char("Ok. \n\r", ch );
	return;
    }
    if (! str_prefix ( arg2, "builder" ) )
    {
	SET_BIT( victim->secflags , PACK_BUILDER_SEC);
	SET_BIT( victim->funflags , PACK_BUILDER_FUN);
	SET_BIT( victim->infflags , PACK_BUILDER_INF);
	send_to_char( "Ok. \n\r", ch );
	return;
    }

    if (! str_prefix ( arg2, "normal" ) )
    {
	SET_BIT( victim->secflags , PACK_NORMAL_SEC );
	SET_BIT( victim->funflags , PACK_NORMAL_FUN );
	SET_BIT( victim->infflags , PACK_NORMAL_INF );
	send_to_char( "Ok. \n\r", ch );
	return;
    }
    if (! str_prefix ( arg2, "punitive" ) )
    {
	SET_BIT( victim->secflags , PACK_PUNITIVE_SEC );
	SET_BIT( victim->funflags , PACK_PUNITIVE_FUN );
	/*SET_BIT( victim->infflags , PACK_PUNITIVE_FUN );*/ /*bug (prolly a cut and paste typo)*/
	SET_BIT( victim->infflags , PACK_PUNITIVE_INF ); /*Spellsong fix*/

	send_to_char("Ok. \n\r", ch);
	return;
    }
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( arg2[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( arg2 , cmd_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }
    if (! found )
    {
	send_to_char("Not a valid command. \n\r", ch );
	return;
    }
    if ( cmd_table[cmd].group == GROUP_SEC )
		SET_BIT( victim->secflags , cmd_table[cmd].flag );
    if ( cmd_table[cmd].group == GROUP_INF )
		SET_BIT( victim->infflags , cmd_table[cmd].flag );
    if ( cmd_table[cmd].group == GROUP_FUN )
		SET_BIT( victim->funflags , cmd_table[cmd].flag );
    send_to_char ("Ok. \n\r", ch ) ;
    return;

}

void do_revoke( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found = FALSE;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: revoke <char> <command>\n\r",ch);
	return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't playing.\n\r", ch );
	return;
    }
    if (! str_prefix ( arg2, "normal" ) )
    {
	REMOVE_BIT( victim->secflags , PACK_NORMAL_SEC );
	REMOVE_BIT( victim->funflags , PACK_NORMAL_FUN );
	REMOVE_BIT( victim->infflags , PACK_NORMAL_INF );
	send_to_char( "Ok. \n\r", ch );
	return;
    }
    if (! str_prefix ( arg2, "punitive" ) )
    {
	REMOVE_BIT( victim->secflags , PACK_PUNITIVE_SEC );
	REMOVE_BIT( victim->funflags , PACK_PUNITIVE_FUN );
	/*REMOVE_BIT( victim->infflags , PACK_PUNITIVE_FUN );*/ /*bug (prolly a cut and paste typo)*/
	REMOVE_BIT( victim->infflags , PACK_PUNITIVE_INF ); /*Spellsong fix*/
	
	send_to_char("Ok. \n\r", ch);
	return;
    }
     for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( arg2[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( arg2 , cmd_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }
    if (! found )
    {
	send_to_char("Not a valid command. \n\r", ch );
	return;
    }
    if (!( ( cmd_table[cmd].group == GROUP_SEC &&
	IS_SET( cmd_table[cmd].flag , ch->secflags ) )
	||
	( cmd_table[cmd].group == GROUP_FUN &&
	IS_SET ( cmd_table[cmd].flag , ch->funflags ) )
	||
	( cmd_table[cmd].group == GROUP_INF &&
	IS_SET ( cmd_table[cmd].flag , ch->infflags ) ) ) )
	{
		send_to_char("Character doesn't have the command. \n\r" , ch);
		return;
	}

    if ( cmd_table[cmd].group == GROUP_SEC )
		REMOVE_BIT ( victim->secflags , cmd_table[cmd].flag );
    if ( cmd_table[cmd].group == GROUP_INF )
		REMOVE_BIT ( victim->infflags , cmd_table[cmd].flag );
    if ( cmd_table[cmd].group == GROUP_FUN )
		REMOVE_BIT ( victim->funflags , cmd_table[cmd].flag );
    send_to_char ("Ok. \n\r", ch ) ;
    return;

}

/* do_award function coded by Rindar (Ron Cole) and Raven (Laurie Zenner).  */
/*Adapted to this mud by Spellsong*/
void do_award( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    char       arg3 [ MAX_INPUT_LENGTH ];
    int      value;
    int      level;
    int      levelvi;
    char     *maxed;
    long     maxlong  = 2147483647;

	buf[0] = '\0'; /*spellsong add*/
    
	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    
    /* have to initialize maxed or will get warnings when compiling */
    maxed = strdup( "maxed" );

    if ( arg1[0] == '\0' || arg3[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: award <char> <amount> <type>.\n\r", ch );
        send_to_char( "Valid types: alignment, experience, gold, object.\n\r", ch );
        send_to_char( "NOTE: Substitute the object's VNUM for the amount.\n\r", ch );
        return;
    }
    
    if ( strncmp( arg3, "alignment",  strlen( arg3 ) )
    &&   strncmp( arg3, "experience", strlen( arg3 ) )
    &&   strncmp( arg3, "gold",       strlen( arg3 ) )
    &&   strncmp( arg3, "object",     strlen( arg3 ) ) )
    {
        send_to_char( "Valid types: alignment, experience, gold, object.\n\r", ch );
        send_to_char( "NOTE: Substitute the object's VNUM for the amount.\n\r", ch );
        return;
    }

    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
      send_to_char("That person is not currently on-line.\n\r", ch);
      return;
    }
       
    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    level = get_trust(ch);
    levelvi = get_trust(victim);

    if ( ( level <= levelvi ) && ( ch != victim ) )
    {
        send_to_char( "You can only award those of lesser trust level.", ch );
        return;
    }


    value = atoi( arg2 );
    
    if ( value == 0 )
    {
       send_to_char( "The value must not be equal to 0.\n\r", ch );
       return;
    }

    if ( arg3[0] == 'a' ) /* alignment */
    {
       if ( value < -2000 || value > 2000 )
       {
           send_to_char( "You can only alter someone's alignment by -2000 to 2000 points.\n\r", ch );
           return;
       }
    
       if ((victim->alignment+value) > 1000)
       {
          value = (1000 - victim->alignment);
          victim->alignment = 1000;
          maxed = strdup( "high" );
       }

       else if ((victim->alignment+value) < -1000)
       {
          value = (-1000 - victim->alignment);
          victim->alignment = -1000;
          maxed = strdup( "low" );
       }
       else
          victim->alignment += value;

       if ( value == 0 )
       {
          sprintf( buf, "%s's alignment is already as %s as it can be.\n\r", victim->name, maxed );
          send_to_char(buf, ch);
          return;
       }
       else
       {
          sprintf( buf,"You alter %s's alignment by %d points.\n\r",victim->name, value);
          send_to_char(buf, ch);
       
          if ( value > 0 )
          { 
             sprintf( buf,"Your soul feels lighter and more virtuous!");
             send_to_char( buf, victim );
             return;
          }
          else
          {
             sprintf( buf,"You shudder deeply as your soul darkens.");
             send_to_char( buf, victim );
             return;
          }
       }
    }

    if ( arg3[0] == 'e' )  /* experience */
    {
       /* Cannot use the 'gain_exp' function since it won't
          give experience if the player is in the arena and it
          takes con away if they go below minimum experience
          (which could make them self-destruct).   That's just
          too mean to do during an 'award', since it might happen
          by mistake. */
          
    
       if ( victim->level >= LEVEL_HERO )
       {
          sprintf( buf, "%s cannot receive experience bonuses", victim->name );
          send_to_char( buf, ch );
          return;
       }
       
       if ( value < -1000000 || value > 1000000 )
       {
           send_to_char( "You can only award between -1000000 and 1000000 experience.\n\r", ch );
           return;
       }

       if ( victim->exp < 0 && value < 0 && victim->exp < ((-1*maxlong) - value) )
       {
          value   = (-1*maxlong) - victim->exp;
          victim->exp = (-1*maxlong);
          maxed   = strdup( "minumum" );
       }
       else if ( victim->exp > 0 && value > 0 && victim->exp > (maxlong - value) )
       {
          value   = maxlong - victim->exp;
          victim->exp = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->exp += value;

       if ( value == 0 )
       {
          sprintf( buf, "%s already has the %s experience points possible.\n\r", victim->name, maxed );
          send_to_char( buf, ch );
          return;
       }
       else
       {
          sprintf( buf,"You award %s %d experience points.\n\r",victim->name, value);
          send_to_char(buf, ch);
       
          if ( value > 0 )
          { 
             sprintf( buf,"You receive %d experience points for your efforts!\n\r", value );
             send_to_char( buf, victim );
             return;
          }
          else
          {
             sprintf( buf,"You are drained of %d experience points!\n\r", value );
             send_to_char( buf, victim );
             return;
          }
       }
    }
    
    if (arg3[0] == 'g')  /* gold */
    {
       if ( value < -9999999 || value > 9999999 )
       {
           send_to_char( "You can only award between -9999999 and 9999999 gold.\n\r", ch );
           return;
       }

       if ( value < 0 && victim->gold < value )
       {
          value    = -1*victim->gold;
          victim->gold = 0;
          maxed    = strdup( "minumum" );
       }
       else if ( value > 0 && victim->gold > (maxlong - value) )
       {
          value   = maxlong - victim->exp;
          victim->exp = maxlong;
          maxed   = strdup( "maximum" );
       }
       else
          victim->gold += value;
          
       if ( value == 0 )
       {
          sprintf( buf, "%s already has the %s amount of gold allowed.\n\r", victim->name, maxed );
          send_to_char( buf, ch );
          return;
       }
       else
       {
          sprintf( buf,"You award %s %d gold coins.\n\r",victim->name, value);
          send_to_char(buf, ch);
       
          if ( value > 0 )
          { 
             sprintf( buf,"Your coin pouch grows heavier! You gain %d gold coins!\n\r", value );
             send_to_char( buf, victim );
             return;
          }
          else
          {
             sprintf( buf,"Your coin pouch grows lighter! You lose %d gold coins!\n\r", value );
             send_to_char( buf, victim );
             return;
          }
       }
    }

    if ( arg3[0] == 'o' )   /* objects */
    {
       OBJ_INDEX_DATA *pObjIndex;
       int level = get_trust(ch);
       OBJ_DATA *obj;

       if ( ( pObjIndex = get_obj_index( atoi( arg2 ) ) ) == NULL )
       {
           send_to_char( "There is no object with that vnum.\n\r", ch );
           return;
       }

       obj = create_object( pObjIndex, level );

       if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
       {
          sprintf( buf,"Alas, %s is carrying too many items to receive that.\n\r",victim->name);
          send_to_char( buf, ch );
          extract_obj( obj );
          return;
       }

       if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
       {
          sprintf( buf,"Alas, that is too heavy for %s to carry.\n\r",victim->name);
          send_to_char( buf, ch );
          extract_obj( obj );
          return;
       }
       
       obj_to_char( obj, victim );
       sprintf( buf,"You award %s item %d.\n\r",victim->name, value);
       send_to_char(buf, ch);
       
       sprintf( buf,"Your backpack seems heavier!\n\r");
       send_to_char( buf, victim );
       return;
    }
    
    return;
}  

// Allows an immortal to rename a player.
// Divinos, May 8th.

void do_rename(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA	*victim;
	FILE		*new_name_f;
	char		arg1[MAX_STRING_LENGTH],
			arg2[MAX_STRING_LENGTH], original[MAX_STRING_LENGTH],
			path[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
	bool		good;

	arg1[0] = '\0';
	arg2[0] = '\0';

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);



	if ((arg1[0] =='\0') || (arg2[0]=='\0'))
	{
		send_to_char("Usage: rename <target> <newname>\n\r", ch);
		return;
	}

	if( (victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("Target is not in game.\n\r", ch);
		return;
	}

	if(ch->level <= victim->level)
	{
		send_to_char("You may not rename your equals or superiors.\n\r", ch);
	}

	if (IS_NPC(victim))
	{
		send_to_char("Target must be a player character.\n\r", ch);
		return;
	}

	good = check_parse_name(arg2);
	
	if(good)
	{
		fclose(fpReserve);   // I'm not sure why this needs to be done.
		sprintf(path, "%s%s", PLAYER_DIR,capitalize(arg2));

		if ((new_name_f = fopen(path, "r"))!= NULL) 
                {
                	good = FALSE;
                	fclose(new_name_f);
                } // end if name is already taken.

		fpReserve = fopen( NULL_FILE, "r" );
	}

	if (good)
	{
		sprintf(original, victim->name);

		// Inform involved parties.
		sprintf(buf, "You have renamed %s, %s.\n\r", original, capitalize(arg2));
		send_to_char(buf,ch);
		sprintf(buf, "%s renamed %s, %s.", ch->name, original, capitalize(arg2));
		log_string( buf );
		wiznet(buf, NULL, NULL, 0, 0, 0);
		sprintf(buf, "You have been renamed, %s.\n\r", arg2);
		send_to_char(buf, victim);

		// Should save the new pfile.
		free_string(victim->name);
		victim->name = str_dup(capitalize(arg2)); 
		save_char_obj(victim);

		// Should delete the old pfile.
		sprintf(path, "%s%s", PLAYER_DIR, capitalize(original));
                unlink(path);
		return;
	}
        else
	{
                send_to_char("That name is invalid.\n\r", ch);
		sprintf(buf, "%s tried to give %s an invalid name.", ch->name, victim->name);
		log_string(buf);
		wiznet(buf, NULL, NULL, 0, 0, 0);
                return;
	}
}


void repeat(CHAR_DATA *ch, char *argument)
{
	char command[MAX_INPUT_LENGTH];
	int start;
	int stop;
	
	if(argument[0]=='\0')
	{

		send_to_char("Syntax: repeat '<command>' @ '<command with number>' ... # <start> <stop>\n\r",ch);
		send_to_char("\n\rUse %d  for the changing number.\n\r",ch);
		send_to_char("Chain commands together with \n\r",ch);
		send_to_char("Please be *extreamly* careful.\n\r",ch);
		send_to_char("Example of use:\n\r",ch);
		send_to_char("repeat 'say I'm about to talk now' 'say Just counting for fun: %d' # 1 5\n\r",ch);
		send_to_char("Here you count to 5 saying before you talk.\n\r",ch);
		send_to_char("You *will* lag the mud. Be careful. --Akira\n\r",ch);
		return;
	}
}
