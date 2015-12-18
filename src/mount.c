/***************************************************************************
 * 		Original Source Code by John Booth (EoD) 		   *
 * 	   Ported to Rom2.4b4 by Yago Diaz <yago@cerberus.uab.es>	   *
 *	       Bugfixes by Bogardan <jordi@cerberus.uab.es>		   *
 *		     (C) Last Modification February 1998		   *
 ***************************************************************************/

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
 **************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1996 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@efn.org)                                  *
*           Gabrielle Taylor                                               *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"


int mount_success ( CHAR_DATA *ch, CHAR_DATA *mount, int canattack)
{
    int percent;
    int success;

    percent = number_percent() + (ch->level < mount->level ? 
				    (mount->level - ch->level) * 3 : 
				    (mount->level - ch->level) * 2);

    if (!ch->fighting)
	percent -= 25;

    if (!IS_NPC(ch) && IS_DRUNK(ch))
    {
	percent += ch->pcdata->learned[gsn_riding] / 2;
	send_to_char("Due to your being under the influence, riding seems a bit harder...\n\r", ch);
    }

    success = percent - ch->pcdata->learned[gsn_riding];

    if( success <= 0 )
    {
	check_improve(ch, gsn_riding, TRUE, 1);
	return(1);
    }
    else
    {
	check_improve(ch, gsn_riding, FALSE, 1);

	if ( success >= 10 && MOUNTED(ch) == mount)
	{
	    act("You lose control and fall off of $N.", ch, NULL, mount, TO_CHAR);
	    act("$n loses control and falls off of $N.", ch, NULL, mount, TO_ROOM);
	    act("$n loses control and falls off of you.", ch, NULL, mount, TO_VICT);

	    ch->riding = FALSE;
	    mount->riding = FALSE;

	    if (ch->position>POS_STUNNED) 
			ch->position=POS_SITTING;

		send_to_char("Ouch! That kind of hurt!\n\r", ch);
	    ch->hit -= 5;
	    update_pos(ch);
	}

	if ( success >= 40 && canattack)
	{
	    act("$N doesn't like the way you've been treating $M.", ch, NULL, mount, TO_CHAR);
	    act("$N doesn't like the way $n has been treating $M.", ch, NULL, mount, TO_ROOM);
	    act("You don't like the way $n has been treating you.", ch, NULL, mount, TO_VICT);

	    act("$N snarls and attacks you!", ch, NULL, mount, TO_CHAR);
	    act("$N snarls and attacks $n!", ch, NULL, mount, TO_ROOM);
	    act("You snarl and attack $n!", ch, NULL, mount, TO_VICT);  

	    damage( mount, ch, number_range( 1, mount->level), gsn_kick, DAM_BASH );

	}
    }

    return(0);
}

void do_mount( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *mount;

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument(argument, arg);

    if (IS_NPC(ch))
	return;

    if (arg[0] == '\0' && ch->mount && ch->mount->in_room == ch->in_room)
    {
	mount = ch->mount;
    }
    else if (!(mount = get_char_room(ch, arg)))
    {
	send_to_char("Mount what?\n\r", ch);
	return;
    }
 
    if (!IS_NPC(ch) && !ch->pcdata->learned[gsn_riding])
    {
	send_to_char("You don't know how to ride!\n\r", ch);
	return;
    } 

    if (!IS_NPC(mount) || !IS_SET(mount->act, ACT_MOUNT)) 
    {
	sprintf( buf,"You can't ride that.\n\r"); 
	send_to_char(buf, ch); 
	return;
    }
  
    if (mount->level - 5 > ch->level)
    {
	send_to_char("That beast is too powerful for you to ride.", ch);
	return;
    }    

    if( (mount->mount) && (!mount->riding) && (mount->mount != ch))
    {
	sprintf(buf, "%s belongs to %s, not you.\n\r",
	    mount->short_descr, mount->mount->name);
	send_to_char(buf, ch);
	return;
    } 

    if (mount->position < POS_STANDING)
    {
	send_to_char("Your mount must be standing.\n\r", ch);
	return;
    }

    if (RIDDEN(mount))
    {
	send_to_char("This beast is already ridden.\n\r", ch);
	return;
    }
    else if (MOUNTED(ch))
    {
	send_to_char("You are already riding.\n\r", ch);
	return;
    }

    if( !mount_success(ch, mount, TRUE) )
    {
	send_to_char("You fail to mount the beast.\n\r", ch);  
	return; 
    }

    act("You hop on $N's back.", ch, NULL, mount, TO_CHAR);
    act("$n hops on $N's back.", ch, NULL, mount, TO_NOTVICT);
    act("$n hops on your back!", ch, NULL, mount, TO_VICT);
 
    ch->mount = mount;
    ch->riding = TRUE;
    mount->mount = ch;
    mount->riding = TRUE;

    affect_strip(ch, gsn_sneak);
    REMOVE_BIT(ch->affected_by, AFF_SNEAK);
    affect_strip(ch, gsn_hide);
    REMOVE_BIT(ch->affected_by, AFF_HIDE);
}

void do_dismount( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mount;

    if(MOUNTED(ch))
    {
	mount = MOUNTED(ch);

	act("You dismount from $N.", ch, NULL, mount, TO_CHAR);
	act("$n dismounts from $N.", ch, NULL, mount, TO_NOTVICT);
	act("$n dismounts from you.", ch, NULL, mount, TO_VICT);

	ch->riding = FALSE;
	mount->riding = FALSE;
    }
    else
    {
	send_to_char("You aren't mounted.\n\r", ch);
	return;
    }
}


void do_buy_mount( CHAR_DATA *ch, char *argument )
{
    int cost,roll;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char color[80], name[30], size[30];
    CHAR_DATA *mount;
    ROOM_INDEX_DATA *pRoomIndexNext;
    ROOM_INDEX_DATA *in_room;

	buf[0] = '\0'; /*spellsong add*/
    
    name[0] = '\0';
    size[0] = '\0';
    color[0] = '\0'; 

    if ( IS_NPC(ch) )
        return;

    if (!IS_NPC(ch) && !ch->pcdata->learned[gsn_riding])
    {
	send_to_char("How do you expect to buy a horse when you can't even ride?\n\r", ch);
        return;
    }

    argument = one_argument(argument,arg);

    pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

    if ( pRoomIndexNext == NULL )
    {
	bug( "Do_buy: bad mount shop at vnum %d.", ch->in_room->vnum );
        send_to_char( "I'm afraid the stalls where I keep my mounts don't exist.\n\r", ch );
        return;
    }

    in_room     = ch->in_room;
    ch->in_room = pRoomIndexNext;
    mount       = get_char_room( ch, arg );
    ch->in_room = in_room;

    if ( mount == NULL)
    {
	send_to_char( "Sorry, we don't sell any of those here.\n\r", ch );
	return;
    }

    if ( !IS_SET(mount->act, ACT_MOUNT) )
    {
	send_to_char( "Sorry, we don't sell any of those here.\n\r", ch );
	return;
    }
 
    if ( MOUNTED(ch) || ch->mount != NULL)
    {
        send_to_char("You already have a mount.\n\r",ch);
        return;
    }

    /* cost = 100 * ch->level; What a pile of shit! Eris removed 30 April 2000 */
    cost = 10 * mount->level * mount->level;
    
    if ( ch->gold < cost )
    {
	send_to_char( "You can't afford it.\n\r", ch );
	return;
    }
 
    roll = number_percent();

    if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
    {
        cost -= cost / 2 * roll / 100;
        sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
        send_to_char(buf,ch);
        check_improve(ch,gsn_haggle,TRUE,4);
    }

    ch->gold -= cost;
    mount = create_mobile( mount->pIndexData );
        
    mount->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
/*more shit - Eris 30 April 2000 */

/*    mount->max_hit  = mount->hit  = (ch->level * 4) - (25 + number_range(1,50));
    if( mount->max_hit < 25 ) 
        mount->max_hit = mount->hit = 25;
    mount->max_mana = mount->mana = 50;
    mount->max_move = mount->move = 200 + ch->level * 10;
    mount->level = ch->level / 1.25;
    mount->sex = ch->sex;
*/
    argument = one_argument( argument, arg );

    if ( arg[0] != '\0' )
        sprintf( buf, "%s %s", mount->name, arg );
    else
        sprintf( buf, "%s", mount->name );
    free_string( mount->name );
    mount->name = str_dup( buf );

   /* sprintf( buf, "a %s",  mount->name );
    free_string( mount->short_descr );
    mount->short_descr = str_dup( buf );*/

    sprintf( buf, "A %s `wis standing here.\n\r", mount-> name );
    free_string( mount->long_descr );
    mount->long_descr = str_dup( buf );

    sprintf( buf, "%sThe name %s is branded on its hind leg.\n\r",
             mount->description, ch->name );
    free_string( mount->description );
    mount->description = str_dup( buf );

    char_to_room( mount, ch->in_room );
    add_follower( mount, ch );

    act( "$n bought $N as a mount.", ch, NULL, mount, TO_ROOM );
    
    sprintf( buf, "You purchase a %s`w.\n\r", mount->name );
    send_to_char( buf, ch);
    
    do_mount( ch, name );

    return;
}


char *get_mount_owner(CHAR_DATA *pet)
{
    static char owner_name[20];
    char *temp;
    int len=0,i=0;

    if (!IS_NPC(pet))
	return NULL;

    if (pet->description)
	len=strlen(pet->description);
    else
	return NULL;

    temp=pet->description;

    while (*temp && i<15 && len<1024)
    {
	if (*temp==' ')
	    i++;		
	temp++;
	len++;
    }

    if (len==1024)
    {
	log_string("BUG: description overflow for get_mount_owner.");
	return NULL;
    }

    len=0;
    owner_name[0]=0;
    while (*temp && *temp!=' ' && len<16)
    {
	owner_name[len++]=*temp;
	temp++;
    }

    owner_name[len]=0;
    return(owner_name);
}
