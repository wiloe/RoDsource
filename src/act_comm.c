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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "grant.h" /*spellsong for infflags check in channels*/

/* command procedures needed */
DECLARE_DO_FUN(do_quit	);


/*
 * Local functions.
 */
void	note_attach	args( ( CHAR_DATA *ch ) );
void	note_remove	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void	note_delete	args( ( NOTE_DATA *pnote ) );



bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( !str_cmp( ch->name, pnote->sender ) )
		return TRUE;

    if ( is_name( "all", pnote->to_list ) )
		return TRUE;

    if ( IS_HERO(ch) && is_name( "immortal", pnote->to_list ) )
		return TRUE;

    if ( is_name( ch->name, pnote->to_list ) )
		return TRUE;

    return FALSE;
}



void note_attach( CHAR_DATA *ch )
{
    NOTE_DATA *pnote;

    if ( ch->pnote != NULL )
		return;

    if ( note_free == NULL )
    {
		pnote	  = alloc_perm( sizeof(*ch->pnote) );
    }
    else
    {
		pnote	  = note_free;
		note_free = note_free->next;
    }

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->name );
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    ch->pnote		= pnote;
    return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    FILE *fp;
    NOTE_DATA *prev;
    char *to_list;

    /*
     * Build a new to_list.
     * Strip out this recipient.
     */
    to_new[0]	= '\0';
    to_list	= pnote->to_list;
    while ( *to_list != '\0' )
    {
		to_list	= one_argument( to_list, to_one );
		if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
		{
			strcat( to_new, " " );
			strcat( to_new, to_one );
		}
    }

    /*
     * Just a simple recipient removal?
     */
    if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' )
    {
		free_string( pnote->to_list );
		pnote->to_list = str_dup( to_new + 1 );
		return;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == note_list )
    {
		note_list = pnote->next;
    }
    else
    {
		for ( prev = note_list; prev != NULL; prev = prev->next )
		{
			if ( prev->next == pnote )
			break;
		}

		if ( prev == NULL )
		{
			bug( "Note_remove: pnote not found.", 0 );
			return;
		}

		prev->next = pnote->next;
    }

    free_string( pnote->text    );
    free_string( pnote->subject );
    free_string( pnote->to_list );
    free_string( pnote->date    );
    free_string( pnote->sender  );
    pnote->next	= note_free;
    note_free	= pnote;

    /*
     * Rewrite entire list.
     */
    fclose( fpReserve );
    if ( ( fp = fopen( NOTE_FILE, "w" ) ) == NULL )
    {
		perror( NOTE_FILE );
    }
    else
    {
		for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
		{
			fprintf( fp, "Sender  %s~\n", pnote->sender);
			fprintf( fp, "Date    %s~\n", pnote->date);
			fprintf( fp, "Stamp   %d\n",  (int) pnote->date_stamp);
			fprintf( fp, "To      %s~\n", pnote->to_list);
			fprintf( fp, "Subject %s~\n", pnote->subject);
			fprintf( fp, "Text\n%s~\n",   pnote->text);
		}
		fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/* used by imps to nuke a note for good */
void note_delete( NOTE_DATA *pnote )
{
    FILE *fp;
    NOTE_DATA *prev;
 
    /*
     * Remove note from linked list.
     */
    if ( pnote == note_list )
    {
        note_list = pnote->next;
    }
    else
    {
        for ( prev = note_list; prev != NULL; prev = prev->next )
        {
            if ( prev->next == pnote )
                break;
        }
 
        if ( prev == NULL )
        {
            bug( "Note_delete: pnote not found.", 0 );
            return;
        }
 
        prev->next = pnote->next;
    }
 
    free_string( pnote->text    );
    free_string( pnote->subject );
    free_string( pnote->to_list );
    free_string( pnote->date    );
    free_string( pnote->sender  );
    pnote->next = note_free;
    note_free   = pnote;
 
    /*
     * Rewrite entire list.
     */
    fclose( fpReserve );
    if ( ( fp = fopen( NOTE_FILE, "w" ) ) == NULL )
    {
        perror( NOTE_FILE );
    }
    else
    {
        for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
        {
            fprintf( fp, "Sender  %s~\n", pnote->sender);
            fprintf( fp, "Date    %s~\n", pnote->date);
            fprintf( fp, "Stamp   %d\n",  (int) pnote->date_stamp);
            fprintf( fp, "To      %s~\n", pnote->to_list);
            fprintf( fp, "Subject %s~\n", pnote->subject);
            fprintf( fp, "Text\n%s~\n",   pnote->text);
        }
        fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*  do_unread - Tells you how many unread notes you have.
 *  Added by Raptor
 */
void do_unread( CHAR_DATA *ch, char *argument )
{
	NOTE_DATA *pnote;
	int notes;
	char buf[MAX_INPUT_LENGTH];

	buf[0] = '\0'; /*spellsong add*/

	notes = 0;

        for ( pnote = note_list; pnote != NULL; pnote = pnote->next)
            if (is_note_to(ch,pnote) && str_cmp(ch->name,pnote->sender)
            &&  pnote->date_stamp > ch->last_note)
                notes++;

        if (notes == 1)
	{
            send_to_char("\n\rYou have one new note waiting.\n\r",ch);
	}
        else if (notes > 1)
        {
            sprintf(buf,"\n\rYou have %d new notes waiting.\n\r",notes);
            send_to_char(buf,ch);
        }
	else if (notes < 1)
	{
	    send_to_char("\n\rYou have no new notes.\n\r", ch );
	}

	return;
}

void do_note( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    NOTE_DATA *pnote;
    int vnum;
    int anum;

	buf[0] = '\0'; /*spellsong add*/

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if (arg[0] == '\0')
    {
		do_note(ch,"read");
		return;
    }

    if ( !str_prefix( arg, "list" ) )
    {
		vnum = 0;
		for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) )
			{
				sprintf( buf, "`K[`W%3d%s`K] `Y%s`w: `G%s\n\r`w",
					vnum, 
					(pnote->date_stamp > ch->last_note 
					 && str_cmp(pnote->sender,ch->name)) ? "N" : " ",
					 pnote->sender, pnote->subject );
				send_to_char( buf, ch );
				vnum++;
			}
		}
		return;
    }

    if ( !str_prefix( arg, "read" ) )
    {
		bool fAll;

		if ( !str_cmp( argument, "all" ) )
		{
			fAll = TRUE;
			anum = 0;
		}
		
		else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
		/* read next unread note */
		{
			vnum = 0;
			for ( pnote = note_list; pnote != NULL; pnote = pnote->next) 
			{
			if (is_note_to(ch,pnote) && str_cmp(ch->name,pnote->sender)
			&&  ch->last_note < pnote->date_stamp)
            		{
						sprintf( buf, "`K[`W%3d`K] `Y%s`w: `G%s\n\r`G%s\n\r`GTo: %s\n\r`w",
                    		vnum,
                    		pnote->sender,
                    		pnote->subject,
                    		pnote->date,
                    		pnote->to_list);
						send_to_char( buf, ch );
						page_to_char( pnote->text, ch );
						ch->last_note = UMAX(ch->last_note,pnote->date_stamp);
						return;
			}
			else if (is_note_to(ch,pnote))
				vnum++;
				}
			send_to_char("You have no unread notes.\n\r",ch);	
			return;
		}

		else if ( is_number( argument ) )
		{
			fAll = FALSE;
			anum = atoi( argument );
		}
		else
		{
			send_to_char( "Note read which number?\n\r", ch );
			return;
		}

		vnum = 0;
		for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
			{
			   sprintf( buf, "`K[`W%3d`K] `Y%s`w: `G%s\n\r`G%s\n\r`GTo: %s\n\r`w",
				vnum - 1,
				pnote->sender,
				pnote->subject,
				pnote->date,
				pnote->to_list
				);
			send_to_char( buf, ch );
			send_to_char( pnote->text, ch );
			ch->last_note = UMAX(ch->last_note,pnote->date_stamp);
			return;
			}
		}

		send_to_char( "No such note.\n\r", ch );
		return;
		}

		if ( !str_cmp( arg, "+" ) )
		{
			note_attach( ch );
			strcpy( buf, ch->pnote->text );
			if ( strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 255 )
			{
				send_to_char( "Note too long.\n\r", ch );
				return;
			}

			strcat( buf, argument );
			strcat( buf, "\n\r" );
			free_string( ch->pnote->text );
			ch->pnote->text = str_dup( buf );
			send_to_char( "Ok.\n\r", ch );
			return;
		}

    if ( !str_prefix( arg, "subject" ) )
    {
		note_attach( ch );
		free_string( ch->pnote->subject );
		ch->pnote->subject = str_dup( argument );
		send_to_char( "Ok.\n\r", ch );
		return;
    }

    if ( !str_prefix( arg, "to" ) )
    {
		note_attach( ch );
		free_string( ch->pnote->to_list );
		ch->pnote->to_list = str_dup( argument );
		send_to_char( "Ok.\n\r", ch );
		return;
    }

    if ( !str_prefix( arg, "clear" ) )
    {
		if ( ch->pnote != NULL )
		{
			free_string( ch->pnote->text );
			free_string( ch->pnote->subject );
			free_string( ch->pnote->to_list );
			free_string( ch->pnote->date );
			free_string( ch->pnote->sender );
			ch->pnote->next	= note_free;
			note_free		= ch->pnote;
			ch->pnote		= NULL;
		}

		send_to_char( "Ok.\n\r", ch );
		return;
    }

    if ( !str_prefix( arg, "show" ) )
    {
		if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	sprintf( buf, "%s: %s\n\r`wTo: %s\n\r`w",
	    ch->pnote->sender,
	    ch->pnote->subject,
	    ch->pnote->to_list
	    );
	send_to_char( buf, ch );
	send_to_char( ch->pnote->text, ch );
	return;
    }

    if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send"))
    {
	FILE *fp;
	char *strtime;

	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	if (!str_cmp(ch->pnote->to_list,""))
	{
	    send_to_char(
		"You need to provide a recipient (name, all, or immortal).\n\r",
		ch);
	    return;
	}

	if (!str_cmp(ch->pnote->subject,""))
	{
	    send_to_char("You need to provide a subject.\n\r",ch);
	    return;
	}

	ch->pnote->next			= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	ch->pnote->date			= str_dup( strtime );
	ch->pnote->date_stamp		= current_time;

	if ( note_list == NULL )
	{
	    note_list	= ch->pnote;
	}
	else
	{
	    for ( pnote = note_list; pnote->next != NULL; pnote = pnote->next )
		;
	    pnote->next	= ch->pnote;
	}
	pnote		= ch->pnote;
	ch->pnote	= NULL;

	fclose( fpReserve );
	if ( ( fp = fopen( NOTE_FILE, "a" ) ) == NULL )
	{
	    perror( NOTE_FILE );
	}
	else
	{
	    fprintf( fp, "Sender  %s~\n",	pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %d\n", (int) pnote->date_stamp);
	    fprintf( fp, "To	  %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n", pnote->text);
	    fclose( fp );
	}
	fpReserve = fopen( NULL_FILE, "r" );

	send_to_char( "Ok.\n\r", ch );
	
	for ( d = descriptor_list; d != NULL; d = d->next )
        {
        
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;
        
        if ( d->connected == CON_PLAYING && d->character != ch  )
        {
             act_new("`M$n has posted a note!`w",ch,argument,d->character,TO_VICT,POS_DEAD);
        }
        }
	return;
    }

    if ( !str_prefix( arg, "remove" ) )
    {
	if ( !is_number( argument ) )
	{
	    send_to_char( "Note remove which number?\n\r", ch );
	    return;
	}

	anum = atoi( argument );
	vnum = 0;
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && vnum++ == anum )
	    {
		note_remove( ch, pnote );
		send_to_char( "Ok.\n\r", ch );
		return;
	    }
	}

	send_to_char( "No such note.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "delete" ) && (get_trust(ch) >= MAX_LEVEL - 1
	|| !str_cmp( ch->name, "Fury")  || !str_cmp( ch->name,"Lutessa")))
    {
        if ( !is_number( argument ) )
        {
            send_to_char( "Note delete which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                note_delete( pnote );
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }
 
        send_to_char( "No such note.\n\r", ch );
        return;
    }

        if ( !str_prefix( arg, "edit" ) )
        {
            note_attach( ch );
            string_append( ch, &ch->pnote->text );
            return;
        }

    send_to_char( "Huh?  Type 'help note' for usage.\n\r", ch );
    return;
}


/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	    stop_fighting( ch, TRUE );
	    do_quit(ch,"");
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
    ch->pcdata->confirm_delete = TRUE;
}
	    
/*Channel look by Kyndig - fixed by spellsong*/

void do_channels( CHAR_DATA *ch, char *argument)
{
     
   printf_to_char(ch,
"            `GYour specific communications information:\r
  `WO`B------------------------------------------------------------`WO\r
  `B|       `GDEAF MODE: `W[%s                   `GOOC: `W[%s       `B|\r
  `B|     `GAFK MODE: `W[%s            `GQUIET MODE: `W[%s          `B|\r",
/*  `B|    `GDEAF MODE: `W[%s                `                        `B|\r
*/


   IS_SET(ch->comm,COMM_DEAF) ? "`RON `W]" : "`goff`W",
   IS_SET(ch->comm, COMM_NOOOC) ? "`goff`W]" : "`GON `W]",
   IS_SET(ch->act, PLR_AFK) ? "`RON `W]" : "`goff`W]",
   IS_SET(ch->comm, COMM_QUIET) ? "`RON `W]" : "`goff`W]"
/* 
   ,
   IS_SET(ch->comm, COMM_DEAF) ? "`RON `W]" : "`goff`W]"
*/
);
   if ( !IS_IMMORTAL(ch))
   {
        printf_to_char(ch,
			"\n\r  `WO`B------------------------------------------------------------`WO" );
   }

   if ( IS_IMMORTAL(ch))
   {
        printf_to_char(ch,
"\n\r  `WO`B----------------------`RIMMORTALS INFO`B------------------------`WO\r
\n\r  `WO`B-------------------`GIMMORTAL CHANN `W[%s`B---------------------`WO\r
  `WO`B---------------------`GWIZNET CHANN `W[%s`B---------------------`WO
\r",
       IS_SET(ch->comm, COMM_NOWIZ) ? "`goff`W]" : "`GON `W]",
       IS_SET(ch->wiznet, WIZ_ON) ? "`GON `W]" : "`goff`W]");
        
    }

	if(IS_SET ( ch->infflags, INF_IMMTALK )) 
	{
		printf_to_char(ch,
"  `WO`B----------------------`GADMIN CHANN `W[%s`B---------------------`WO
\n\r",
		IS_SET(ch->comm, COMM_NOIMP) ? "`goff`W]" : "`GON `W]");

	}

   if (ch->lines != PAGELEN)
   {
        if (ch->lines)
        {
            printf_to_char(ch,"*You display %d lines of scroll.\n\r",ch->lines);
        }
        else
            send_to_char("*Scroll buffering is off.\n\r",ch);
   }

   if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("*You cannot use tell.\n\r",ch);

   if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("*You cannot use channels.\n\r",ch);

   if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("*You cannot show emotions.\n\r",ch);

}


/* RT code to display channel status 

void do_channels( CHAR_DATA *ch, char *argument)
{
    send_to_char("`W   channel   status`w\n\r",ch);
    send_to_char("`K---------------------`w\n\r",ch);
 
    send_to_char("gossip         ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    send_to_char("auction        ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    send_to_char("music          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    send_to_char("Q/A            ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);

    if (IS_HERO(ch))
    {
      send_to_char("imm channel    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
	send_to_char("`GON`w\n\r",ch);
      else
	send_to_char("`ROFF`w\n\r",ch);

      send_to_char("Prayers        ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
	send_to_char("`GON`w\n\r",ch);
      else
	send_to_char("`ROFF`w\n\r",ch);

    }

    send_to_char("shouts         ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);


    send_to_char("quiet mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("`GON`w\n\r",ch);
    else
      send_to_char("`ROFF`w\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	char buf[100];
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot emote.\n\r",ch);

}
*/

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear shouts again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear shouts.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* do OOC installed by Kyndig */

void do_ooc( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

	buf[0] = '\0'; /*spellsong add*/

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOOOC))
      {
        send_to_char("OOC channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOOOC);
      }
      else
      {
        send_to_char("OOC channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOOOC);
      }
    }
    else  /*Argument !=\0*/
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }

        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
        }

         REMOVE_BIT(ch->comm,COMM_NOOOC);
       
 
       sprintf( buf, "`yYou ooc `Y'`y%s`Y'\n\r", argument );
       send_to_char( buf, ch );
       for ( d = descriptor_list; d != NULL; d = d->next )
       {

        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOOOC) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
      	     act_new("`y$n ooc's `Y'`y$t`Y'",
                    ch,argument,d->character,TO_VICT,POS_DEAD);
        }
    }
  } /*Moved from the wrong place*/
}

/* RT chat replaced with ROM gossip */
void do_gossip( CHAR_DATA *ch, char *argument )
{
	send_to_char("No gossip, use ooc.\n\r",ch);
}

/*Pray channel, added by Kat 04111999 updated 04121999
Will rename later: Akira
void do_praytoimm( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOPRAY) && IS_HERO(ch))
      {
	send_to_char("`rPrayer `Wchannel is now ON\n\r`w",ch);
	REMOVE_BIT(ch->comm,COMM_NOPRAY);
      }
      else if ((!IS_SET(ch->comm,COMM_NOPRAY)) && IS_HERO(ch)) 
      {
	send_to_char("`rPrayer `Wchannel is now OFF\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOPRAY);
      }
      else 
      {
       send_to_char("Whom would you like to pray to?\n\r",ch);
      }
      return;
    }
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}
    REMOVE_BIT(ch->comm,COMM_NOPRAY);

    act_new("`rYou pray to the immortals: '$t'`w",ch,argument,NULL,TO_CHAR,POS_DEAD);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     IS_HERO(d->character) &&
	     !IS_SET(d->character->comm,COMM_NOPRAY) )
	{
	    act_new("`r$n's prayer is heard by the immortals: $t`w",ch,argument,d->character,TO_VICT,POS_DEAD);
	}

    }

    return;
}*/

/*debugged by spellsong - Now performs a flag check instead
 *of a trust check*/
void do_immtalk( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
		send_to_char("`CI`cm`Cm`co`Cr`ct`Ca`cl`W channel is now ON\n\r`w",ch);
		REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
		send_to_char("`CI`cm`Cm`co`Cr`ct`Ca`cl `Wchannel is now OFF\n\r`w",ch);
		SET_BIT(ch->comm,COMM_NOWIZ);
      }
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);

    act_new("`B$n: `C$t`w",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && 
	     !IS_SET(d->character->comm,COMM_NOWIZ) )
	{
		/*if structure added and check - spellsong*/
	    if(IS_SET ( d->character->infflags, INF_IMMTALK )) 
		{
			act_new("`B$n: `C$t`w",ch,argument,d->character,TO_VICT,POS_DEAD);
		}
	}
    }

    return;
}

/*new channel added per nyobe by spellsong
  can only use channel if granted. Performs
  check in descriptor loop to see if you 
  actually have the command*/
void do_imptalk( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
	
    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOIMP))
      {
		send_to_char("`YA`yd`Ym`yi`Yn `Wchannel is now ON\n\r`w",ch);
		REMOVE_BIT(ch->comm,COMM_NOIMP);
      }
      else
      {
		send_to_char("`YA`yd`Ym`yi`Yn `Wchannel is now OFF\n\r`w",ch);
		SET_BIT(ch->comm,COMM_NOIMP);
      }
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOIMP);

    act_new("`Y$n: `y$t`w",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
		if ( d->connected == CON_PLAYING && 
	     !IS_SET(d->character->comm,COMM_NOIMP) )
		{
			/*flag check - spellsong*/
			if(IS_SET ( d->character->infflags, INF_IMPTALK ))
			{
				act_new("`Y$n: `y$t`w",ch,argument,d->character,TO_VICT,POS_DEAD);
			}
		}
    }

    return;
}

void do_say( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    act( "`G$n says '$T`G'`w", ch, NULL, argument, TO_ROOM );
    act( "`GYou say '$T`G'`w", ch, NULL, argument, TO_CHAR );
    mprog_speech_trigger( argument, ch );
    return;
}


void do_osay( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
        send_to_char( "Say what OOC?\n\r", ch );
        return;
    }

    act( "`G$n says ooc: '$T`G'`w", ch, NULL, argument, TO_ROOM );
    act( "`GYou say ooc: '$T`G'`w", ch, NULL, argument, TO_CHAR );
    return;
}


void do_sendinfo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
    
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOINFO))
      {
        send_to_char("`BInfo channel is now ON.\n\r`w",ch);
        REMOVE_BIT(ch->comm,COMM_NOINFO);
      }
      else
      {
        send_to_char("`BInfo channel is now OFF.\n\r`w",ch);
        SET_BIT(ch->comm,COMM_NOINFO);
      }
      return;
    }
    strcpy(buf, "`WINFO: `R");
    strcat(buf, argument);
    strcat(buf, "\n\r`w");
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     !IS_SET(victim->comm, COMM_DEAF) &&
	     !IS_SET(victim->comm, COMM_NOINFO) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
	    send_to_char(buf,d->character);
	}
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
  
    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    act( "`cYou tell $N '$t`c'`w", ch, argument, victim, TO_CHAR );
    act_new("`c$n tells you '$t`c'`w",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
        act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
        return;
    }

    act("`cYou tell $N '$t`c'`w",ch,argument,victim,TO_CHAR);
    act_new("`c$n tells you '$t`c'`w",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }


    act("`YYou yell '$t`Y'`w",ch,argument,NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
        &&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    act("`Y$n yells '$t`Y'`w",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You can't emote.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    act( "$n $T`w", ch, NULL, argument, TO_ROOM );
    act( "$n $T`w", ch, NULL, argument, TO_CHAR );
    return;
}

void do_info( CHAR_DATA *ch)
{
      if (IS_SET(ch->comm,COMM_NOINFO))
      {
        send_to_char("`BInfo channel is now ON.\n\r`w",ch);
        REMOVE_BIT(ch->comm,COMM_NOINFO);
      }
      else
      {
	send_to_char("`BInfo channel is now OFF.\n\r`w",ch);
	SET_BIT(ch->comm,COMM_NOINFO);
      }
      return;
}

/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "You sizzle with energy.",
	    "$n sizzles with energy.",
	    "You feel very holy.",
	    "$n looks very holy.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles."
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers."
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean."
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll."
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle."
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder."
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great gods give you a staff.",
	    "The great gods give $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him."
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}



void do_idea( CHAR_DATA *ch, char *argument )
{
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Idea logged. This is NOT an identify command.\n\r", ch );
    return;
}



void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}



void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
/* */
    send_to_char( "\n\r", ch );

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)) )
    {
        send_to_char ("Wait till you have sold/bought the item on auction.\n\r",ch);
        return;
    }

    /* Prevent recently brawling people from logging out -Rhys */
    if ( ( ch->pcdata->fight_timer > 0 ) && !IS_IMMORTAL(ch ))
    {
        send_to_char ( "You were recently in a fight. You must calm down first.\n\r", ch );
        return;
    }

    send_to_char(
	"Alas, all good things must come to an end.\n\r",ch);
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

    /*
     * After extract_char the ch is no longer valid!
     */
    if (ch->at_obj !=NULL){
      char_off_obj(ch);
	}
    save_char_obj( ch );
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    extern bool chaos;

    if ( IS_NPC(ch) )
	return;
if ( chaos )
  {
   send_to_char("Saving is not allowed during `rC`RH`YA`RO`rS.\n\r`w",ch);
   return;
  }

    save_char_obj( ch );
    send_to_char("Saving.\n\r", ch );
    /* WAIT_STATE(ch,5 * PULSE_VIOLENCE); */
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_HERO(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if (IS_NPC(ch))
       return;

    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

   if ( ch->mount
   && (ch->mount->in_room == ch->in_room || ch->mount->in_room==NULL) )
   {
      pet = ch->mount;
      do_dismount(ch, "");
      if (pet->in_room != NULL)
          act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
      else
          log_string("void nuke_pets: Extracting null pet");
       ch->mount = NULL;
       ch->riding = FALSE;            
      extract_char(pet, TRUE);
   }
   else if (ch->mount)
   {
      ch->mount->mount = NULL;
      ch->mount->riding = FALSE;
   }
   ch->mount = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

	buf[0] = '\0'; /*spellsong add*/

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;

        if (!strcmp(arg, "mount"))
        {
            if (!ch->mount)
            {
                send_to_char("Your don't have a mount.\n\r", ch);
                return;
            }

            if (ch->mount->in_room != ch->in_room)
            {
                send_to_char("Your mount isn't here!\n\r", ch);
                return;
            }
            else
            {
                victim = ch->mount;
            }
        }
        else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

       if ( victim->mount == ch )
       { 
           if ( !mount_success(ch, victim, FALSE) )
           {
               act("$N ignores your orders.", ch, NULL, victim, TO_CHAR);
                return;
           }
           else
           {
               sprintf(buf,"%s orders you to \'%s\'.", ch->name, argument);
               send_to_char(buf, victim);
               interpret( victim, argument );
               return;
            }                
       }
       else if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
       ||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
       {
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
       }
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
	send_to_char( "Ok.\n\r", ch );
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

	buf[0] = '\0'; /*spellsong add*/

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "`K[`W%s's group`K]`w\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"`K[`W%3d `G%s`K] `w%-16s `W%4d`K/`W%4d hp %4d`K/`W%4d mana %4d`K/`W%4d mv %5ld xp\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (IS_AFFECTED(victim,AFF_CHARM))
    {
        send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
    	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }
    if ( ch->level - victim->level < -6
    ||   ch->level - victim->level >  6 )
    {
	act( "$N cannot join $n's group.",     ch, NULL, victim, TO_NOTVICT );
	act( "You cannot join $n's group.",    ch, NULL, victim, TO_VICT    );
	act( "$N cannot join your group.",     ch, NULL, victim, TO_CHAR    );
	return;
    }

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

	buf[0] = '\0'; /*spellsong add*/

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }

    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }

    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sprintf( buf,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );
    send_to_char( buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

	buf[0] = '\0'; /*spellsong add*/

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "`C%s tells the group '%s`C'.\n\r`w", ch->name, argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    send_to_char( buf, gch );
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

void do_beep ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/

    if (IS_NPC(ch))
	return;

    argument = one_argument( argument, arg );

    if  ( arg[0] == '\0' )
    {
        send_to_char( "Beep who?\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
        send_to_char( "They are not here.\n\r", ch );
        return;
    }


    if ( IS_NPC(victim))
    {
        send_to_char( "They are not beepable.\n\r", ch );
        return;
    }

    sprintf( buf, "\aYou beep %s.\n\r", victim->name );
    send_to_char( buf, ch );

    sprintf( buf, "`R\a%s has beeped you.\n\r", ch->name );
    send_to_char( buf, victim );
    return;
}


void add_to_ignore_list(CHAR_DATA *ch,CHAR_DATA *dude)
{
    IGNORE_DATA *ign=ignore_free;

    if(ignore_free)
    {
        ignore_free=ignore_free->next;
    }
    else
    {
        ign=alloc_perm(sizeof(IGNORE_DATA));
    }
    if(ign)
    {
        memset(ign,0,sizeof(IGNORE_DATA));
        ign->name=alloc_mem(strlen(dude->name)+1);
        if(ign->name)
        {
            strcpy(ign->name,dude->name);
            ign->next=ch->ignore;
            ch->ignore=ign;
        }
        else
        {
            bug("add_to_ignore: out of mem",0);
            ign->next=ignore_free;
            ignore_free=ign;
        }
    }
    else
    {
        bug("add_to_ignore_list: out of memory",0);
    }
}

void remove_from_ignore_list(CHAR_DATA *ch,CHAR_DATA *dude)
{
    IGNORE_DATA *last=NULL,*check;
    bool found=FALSE;

    if(ch&&dude)
    {
        check=ch->ignore;
        while(!found&&check)
        {
            if(strcmp(check->name,dude->name))
            {
                last=check;
                check=check->next;
            }
            else
            {
                found=TRUE;
            }
        }
        if(found)
        {
            if(last)
            {
                last->next=check->next;
            }
            else
            {
                ch->ignore=check->next;
            }
            check->next=ignore_free;
            free_mem(check->name,strlen(check->name));
            check->name=NULL;
            ignore_free=check;
        }
        else
        {
            bug("remove_from_ignore_list: could not find dude!",0);
        }
    }
    else
    {
        bug("remove_from_ignore_list: char or dude is null",0);
    }
}

CHAR_DATA *should_ignore(CHAR_DATA *ch,CHAR_DATA *dude)
{
    IGNORE_DATA *ign=NULL;
    bool found=FALSE;

    if(ch)
    {
        ign=ch->ignore;
        while(!found&&ign)
        {
           if(strcmp(ign->name,dude->name))
            {
                ign=ign->next;
            }
            else
            {
                found=TRUE;
            }
        }
    }
    else
    {
        bug("should_ignore: NULL char!",0);
    }
    return(found?dude:NULL);
}

void release_ignores(CHAR_DATA *ch)
{
    IGNORE_DATA *ign;

    if(ch)
    {
        if(ch->ignore)
        {
            ign=ch->ignore;
            while(ign->next)
            {
                ign=ign->next;
            }
            ign->next=ignore_free;
            ignore_free=ch->ignore;
        }
    }
    else
    {
        bug("release_ignores: null char",0);
    }
}

void do_ignore(CHAR_DATA *ch,char *arguement)
{
    char arg[MAX_INPUT_LENGTH];
    char buff[MAX_STRING_LENGTH];
    CHAR_DATA *dude;
    IGNORE_DATA *ign;

    if(arguement)
    {
        one_argument(arguement,arg);
        if(*arg)
        {
            dude=get_char_world(ch,arg);
            if(dude)
            {
                if(should_ignore(ch,dude))
                {
                    remove_from_ignore_list(ch,dude);
                    sprintf(buff,"You will now hear %s.\n\r",dude->name);
                }
                else
                {
                    if(ch!=dude)
                    {
                        if(dude->level<LEVEL_IMMORTAL)
                        {
                            add_to_ignore_list(ch,dude);
                            sprintf(buff,"You are now ignoring %s.\n\r",
                                       dude->name);
                        }
                        else
                        {
                            sprintf(buff,"For your own safety, you "
                                         "cannot ignore %s.\n\r",dude->name);
                        }
                    }
                    else
                    {
                        sprintf(buff,"You can\'t ignore yourself!\n\r");
                    }
                }
                send_to_char(buff,ch);
            }
            else
            {
                sprintf(buff,"%s is not in Discordia.\n\r",arg);
                send_to_char(buff,ch);
            }
        }
        else
        {
            if(ch->ignore)
            {
                send_to_char("You are currently ignoring:\n\r",ch);
                ign=ch->ignore;
                while(ign)
                {
                    sprintf(buff,"\t%s\n\r",ign->name);
                    send_to_char(buff,ch);
                    ign=ign->next;
                }
            }
            else
            {
                send_to_char("You are not ignoring anybody\n\r",ch);
            }
        }
    }
    else
    {
        bug("do_ignore: no arg!",0);
    }
}

