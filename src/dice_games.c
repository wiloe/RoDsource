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
 * If you choose to use this code, please retain my name in this file and  *
 * send me an email (dwa1844@rit.edu) saying you are using it. Suggestions *
 * for improvement are welcome.   -- Rahl (Daniel Anderson)                *
 *                                                                         *
 *                                                                         *
 *  ADAPTED AND RECODED TO REALMS OF DISCORDIA BY - SPELLSONG              *
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

void win( CHAR_DATA *ch, long amnt );
void lose( CHAR_DATA *ch, long amnt );

void win( CHAR_DATA *ch, long amnt )
{
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    ch->gold += amnt;
    sprintf( buf, "You win %ld gold!\n\r", amnt );
    send_to_char( buf, ch );
}

void lose( CHAR_DATA *ch, long amnt )
{
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    ch->gold -= amnt;
    sprintf( buf, "You lost %ld gold!\n\r", amnt );
    send_to_char( buf, ch );
}

void game_even_odd( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    long amount;
    int roll;
    CHAR_DATA *gamemaster;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    buf[0] = '\0';

    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: game even-odd <even|odd> <amount>\n\r", ch );
        return;
    }

    for ( gamemaster = ch->in_room->people; gamemaster != NULL; gamemaster = gamemaster->next_in_room )
    {
        if ( !IS_NPC( gamemaster ) )
            continue;
        if ( gamemaster->spec_fun == spec_lookup( "spec_gamemaster" ) )
            break;
    }

    if ( gamemaster == NULL || gamemaster->spec_fun != spec_lookup( "spec_gamemaster" ) )
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( gamemaster->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    if ( !is_number( arg2 ) )
    {
        send_to_char( "You must bet a number.\n\r", ch );
        return;
    }

    roll = dice( 2, 6 );

    amount = atol( arg2 );

	
	/*spellsong check added to limit bets to cut down on abuse*/
	if (amount > 100)
	{
		send_to_char( "Sorry maximum bet is 100 here!\n\r", ch );
        return;
	}
	
	/*spellsong check added so char cant bet more than they have*/
	if (amount > ch->gold)
	{
		send_to_char( "You can't bet more than you have silly!\n\r", ch );
        return;
	}

    if ( amount < 1 )
    {
        send_to_char( "Bet SOMETHING, will you?\n\r", ch );
        return;
    }

    sprintf( buf, "%s rolls the dice.\n\rThe roll is %d.\n\r", gamemaster->short_descr, roll );
    send_to_char( buf, ch );
    
    if ( !str_cmp( arg, "odd" ) )
    {
        if ( roll %2 != 0 )     /* you win! */
           	win( ch, amount );
        else
            lose( ch, amount );

		/*had to put in a random wait state*/
		WAIT_STATE( ch, number_range(1,5));
        return;
    }
    else if ( !str_cmp( arg, "even" ) )
    {
        if ( roll %2 == 0 )
            win( ch, amount );
        else
            lose( ch, amount );

		/*had to put in a random wait state*/
		WAIT_STATE( ch, number_range(1,5));
        return;
    }
    else
    {
        send_to_char( "Syntax: game even-odd <even|odd> <amount>\n\r", ch );
    }
    return;
}


void game_high_low( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    long amount;
    int roll;
    CHAR_DATA *gamemaster;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    buf[0] = '\0';

    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: game high-low <high|low> <amount>\n\r", ch );
        return;
    }

    for ( gamemaster = ch->in_room->people; gamemaster != NULL; gamemaster = gamemaster->next_in_room )
    {
        if ( !IS_NPC( gamemaster ) )
            continue;
        if ( gamemaster->spec_fun == spec_lookup( "spec_gamemaster" ) )
            break;
    }

    if ( gamemaster == NULL || gamemaster->spec_fun != spec_lookup( "spec_gamemaster" ) )
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( gamemaster->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    if ( !is_number( arg2 ) )
    {
        send_to_char( "You must bet a number.\n\r", ch );
        return;
    }

    roll = dice( 2, 6 ); /*roll a 7 and ya autolose should be enough to*/

    amount = atol( arg2 );

	/*spellsong check added to limit bets to cut down on abuse*/
	if (amount > 100)
	{
		send_to_char( "Sorry maximum bet is 100 here!\n\r", ch );
        return;
	}

	/*spellsong check added so char cant bet more than they have*/
	if (amount > ch->gold)
	{
		send_to_char( "You can't bet more than you have silly!\n\r", ch );
        return;
	}

    if ( amount < 1 )
    {
        send_to_char( "Bet SOMETHING, will you?\n\r", ch );
        return;
    }

    sprintf( buf, "%s rolls the dice.\n\rThe roll is %d.\n\r", gamemaster->short_descr, roll );
    send_to_char( buf, ch );
    
    if ( !str_cmp( arg, "high" ) )
    {
        if ( roll > 7 )     /* you win! */
            win( ch, amount );
        else
            lose( ch, amount );

		/*had to put in a random wait state*/
		WAIT_STATE( ch, number_range(1,5));
        return;
    }
    else if ( !str_cmp( arg, "low" ) )
    {
        if ( roll < 7 )
            win( ch, amount );
        else
            lose( ch, amount );
		
		/*had to put in a random wait state*/
		WAIT_STATE( ch, number_range(1,5));
        return;
    }
    else
    {
        send_to_char( "Syntax: game high-low <high|low> <amount>\n\r", ch );
    }
    return;
}


void game_higher_lower( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    long amount;
    int your_roll, his_roll;
    CHAR_DATA *gamemaster;

    argument = one_argument( argument, arg );

    buf[0] = '\0';

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: game higher-lower <amount>\n\r", ch );
        return;
    }

    for ( gamemaster = ch->in_room->people; gamemaster != NULL; gamemaster = gamemaster->next_in_room )
    {
        if ( !IS_NPC( gamemaster ) )
            continue;
        if ( gamemaster->spec_fun == spec_lookup( "spec_gamemaster" ) )
            break;
    }

    if ( gamemaster == NULL || gamemaster->spec_fun != spec_lookup( "spec_gamemaster" ) )
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( gamemaster->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    if ( !is_number( arg ) )
    {
        send_to_char( "You must bet a number.\n\r", ch );
        return;
    }

    your_roll = dice( 2, 6 ) - 1; /*load the dice a bit - SS*/
	
	if (your_roll < 1)
		your_roll = 0;

    his_roll = dice( 2, 6 );

    amount = atol( arg );

	/*spellsong check added to limit bets to cut down on abuse*/
	if (amount > 100)
	{
		send_to_char( "Sorry maximum bet is 100 here!\n\r", ch );
        return;
	}

	/*spellsong check added so char cant bet more than they have*/
	if (amount > ch->gold)
	{
		send_to_char( "You can't bet more than you have silly!\n\r", ch );
        return;
	}

    if ( amount < 1 )
    {
        send_to_char( "Bet SOMETHING, will you?\n\r", ch );
        return;
    }

    sprintf( buf, "%s rolls the dice and gets a %d.\n\rYour roll is %d.\n\r",                                gamemaster->short_descr, his_roll, your_roll );
    send_to_char( buf, ch );
    
    if ( your_roll > his_roll )     /* you win! */
        win( ch, amount );
    else
        lose( ch, amount );
    
	/*had to put in a random wait state*/
	WAIT_STATE( ch, number_range(1,5));
	return;
}

void do_game( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
    /*added in here by spellsong so they dont get a game listing
	*if the mobs with the spec isnt in the same room*/
	CHAR_DATA *gamemaster;
	
	for ( gamemaster = ch->in_room->people; gamemaster != NULL; gamemaster = gamemaster->next_in_room )
    {
        if ( !IS_NPC( gamemaster ) )
            continue;
        if ( gamemaster->spec_fun == spec_lookup( "spec_gamemaster" ) )
            break;
    }

	if ( gamemaster == NULL || gamemaster->spec_fun != spec_lookup( "spec_gamemaster" ) )
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }
	/*end spellsong add*/    
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "higher-lower" ) )
        game_higher_lower( ch, argument );
    else if ( !str_cmp( arg, "even-odd" ) )
        game_even_odd( ch, argument );
    else if ( !str_cmp( arg, "high-low" ) )
        game_high_low( ch, argument );
    else
    {
        send_to_char( "Current games are: higher-lower, even-odd, and high-low.\n\r", ch );
        return;
    }
}
