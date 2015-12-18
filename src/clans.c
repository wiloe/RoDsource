/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
 *  File: clans.c							   *
 *        Clan code copyright (C) 1995 by Kevin Williams, and James 	   *
 *	  Ledbetter (zak@woodtech.com and thexder@woodtech.com)		   *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "merc.h"

/***************************************************************************
 *  Name:	find_clan						   *
 *  Purpose:	Accepts a string argument, searches the clans in memory    *
 *		for a clan->name matching the argument.	 Returns CLAN_DATA *
 *		for the clan if it exists, or NULL if it doesn't.	   *
 *  Called By:  do_mset (act_wiz.c), add_clan (clans.c),                   *
 *              do_show (act_info.c)		                           *
 ***************************************************************************/

CLAN_DATA *find_clan( char *name )
{
   CLAN_DATA *pClan;
   sh_int clannum;
   
   pClan = clan_first;
   while (pClan != NULL)
     {
	if (is_number(name))
	  {
	     clannum = atoi(name);
	     if (pClan->number == clannum ) return pClan;
	  }
	if (strcmp(pClan->name, name)==0) return pClan;
	pClan=pClan->next;
     }
   return pClan;
}

/***************************************************************************
 * Name:        clan_accept                                                *
 * Purpose:     Check CHAR_DATA arg to see if they can join clan arg2.     *
 * Called By:   ?                                                          *
 *                                                                         *
 * Returns:                                                                *
 *           0 : Yes                                                       *
 *           1 : Clan doesn't exist                                        *
 *           2 : Clan has reached max_members                              *
 *           3 : Player doesn't meet min_level req.                        *
 *           4 : Player must be PK.                                        *
 *           5 : Player doesn't meet class reqs.                           *
 *           6 : Player doesn't meet race reqs.                            *
 *           7 : Player is already a member of Clan.                       *
 ***************************************************************************/

sh_int clan_accept( CHAR_DATA *ch, char *clan )
{
   CLAN_DATA *pClan;
   bool      flag;
   sh_int    iClass;
   sh_int    iRace;
   sh_int    iMembers;
   
   flag = TRUE;
   pClan = find_clan(clan);
   if (pClan == NULL)
     {
	return 1;
     }
   if ((pClan->num_members+1) > pClan->max_members)
     {
	return 2;
     }
   if (pClan->min_level > ch->level)
     {
	return 3;
     }
   if ((pClan->req_flags == PLR_KILLER) && (!IS_SET(ch->act, PLR_KILLER)))
     {
	return 4;
     }
   for ( iClass=0 ; iClass < MAX_CLASS ; iClass++ )
     {
	if ((pClan->classes[iClass] == 0) && (ch->class == iClass))
	  {
	     flag = FALSE;
	  }
     }
   if (!flag)
     {
	return 5;
     }
   
   flag = TRUE;
   for ( iRace=0 ; iRace < MAX_PC_RACE ; iRace++ )
     {
	if ((pClan->races[iRace] == 0) && (ch->race == iRace))
	  {
	     flag = FALSE;
	  }
     }
   if (!flag)
     {
	return 6;
     }
   flag = TRUE;
   for ( iMembers=0 ; iMembers < pClan->num_members ; iMembers++ )
     {
	if (pClan->members[iMembers] == ch->name)
	  {
	     flag = FALSE;
	  }
     }
   if (!flag)
     {
	return 7;
     }
   return 0;
}

/***************************************************************************
 *  Name:	add_clan						   *
 *  Purpose:	Sticks CLAN_DATA argument on the end of the Clans stack.   *
 *  Called By:	?							   *
 ***************************************************************************/

void add_clan( CLAN_DATA *Clan )
{
   CLAN_DATA *pClan;
   char	  buf[MAX_STRING_LENGTH];
   
   buf[0]='\0';
   if (top_clan==0)
     {
	clan_first = Clan;
	pClan = Clan;
	pClan->next = NULL;
	clan_last = pClan;
     	top_clan++;
	return;
     }
   else
     {
       pClan = clan_last;
	pClan->next = Clan;
	clan_last = pClan->next;
	clan_last->next=NULL;
	top_clan++;
	return;
     }
}

/***************************************************************************
 *  Name:	new_clan						   *
 *  Purpose:    Creates a new clan with default values.			   *
 *  Called By:	?							   *
 ***************************************************************************/

void new_clan(char *clan_name, CHAR_DATA *ch)
{
   CLAN_DATA *tempClan;
   int	  iClass;
   int	  iRace;
   
   tempClan = alloc_perm( sizeof(*tempClan));
   if (top_clan==0)
     tempClan->number=1;
   else
     tempClan->number=(clan_last->number)+1;
   tempClan->name = str_dup(clan_name);
   tempClan->visible = str_dup(clan_name);
   tempClan->leader="Emberly";
   tempClan->god = str_dup(ch->name);
   tempClan->max_members=10;
   tempClan->min_level=90;
   tempClan->num_members=0;
   tempClan->req_flags=0;
   for (iClass=0;iClass<MAX_CLASS;iClass++)
     tempClan->classes[iClass]=1;
   for (iRace=0;iRace<MAX_PC_RACE;iRace++)
     tempClan->races[iRace]=1;
   tempClan->auto_accept=1;
   tempClan->next=NULL;
   add_clan(tempClan);
}

/***************************************************************************
 *  Name:	save_clans						   *
 *  Purpose:	Save #CLANS section of the clans.are file.		   *
 *  Called By:	do_asave (olc_save.c).					   *
 ***************************************************************************/

void save_clans()		       
{
   FILE *clanfile;
   CLAN_DATA *Clan;
   int iClass, iRace, iMembers;
   
   clanfile = fopen( "clans.are", "w" );
   Clan=clan_first;
   fprintf( clanfile, "#CLANS\n" );
   while (Clan != NULL)
     {
        fprintf( clanfile, "%d\n", Clan->number);
        fprintf( clanfile, "%s~\n%s~\n", Clan->name, Clan->visible);
        fprintf( clanfile, "%s~\n%s~\n", Clan->leader, Clan->god);
	fprintf( clanfile, "%d %d %d ", Clan->max_members, Clan->min_level, Clan->num_members);
	fprintf( clanfile, "%ld ", Clan->req_flags);
	fprintf( clanfile, "%ld %ld ", Clan->cost_gold, Clan->cost_xp);
	for ( iClass=0;iClass<MAX_CLASS;iClass++)
	 fprintf( clanfile, "%d ", Clan->classes[iClass]);
	fprintf( clanfile, "\n");
	for ( iRace=0;iRace<MAX_PC_RACE;iRace++)
	 fprintf( clanfile, "%d ", Clan->races[iRace]);
	fprintf( clanfile, "\n");
	fprintf( clanfile, "%d\n", Clan->auto_accept);
	for ( iMembers=0;iMembers<Clan->num_members;iMembers++)
	 fprintf( clanfile, "%s~\n", Clan->members[iMembers]);
	fprintf( clanfile, "\n");
	Clan=Clan->next;
    }
    fprintf( clanfile, "999\n");
    fprintf( clanfile, "#$\n");
    fclose(clanfile);
    
    return;
}	     

/***************************************************************************
 *  Name:	do_clan 						   *
 *  Purpose:	Create, delete, and modify a clan.		           *
 *  Called By:	interp.c.       					   *
 ***************************************************************************/

void do_clan(CHAR_DATA *ch, char *argument)
{
   char arg1 [MAX_INPUT_LENGTH];
   char arg2 [MAX_INPUT_LENGTH];
   char arg3 [MAX_INPUT_LENGTH];
   char arg4 [MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char tempfile[MAX_STRING_LENGTH];
   CLAN_DATA *Clan;
   int value;
   
   
   tempfile[0] = '\0';
   buf[0] = '\0';
   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   strcpy( arg4, argument );
   
   /*
    * Snarf the value (which need not be numeric).
    */
   value = is_number( arg4 ) ? atoi( arg4 ) : -1;
   
   if ( arg1[0] == '\0')
     { 
	send_to_char("Syntax:\n\r",ch);
	send_to_char("   clan create <name>\n\r",ch);
	send_to_char("   clan delete <name>\n\r",ch);
	send_to_char("   clan set <name> <field> <value>\n\r",ch);
	send_to_char("   Fields: name, visible, leader, god \n\r",ch);
	send_to_char("           max, level, flags, race, class\n\r",ch);
	send_to_char("		 accept\n\r",ch);
	return;
     }
   
   if (!str_cmp( arg1, "create"))
     {
	if (arg2[0] == '\0')
	  {
	     send_to_char("You must provide a clan name.\n\r",ch);
	     return;
	  }
        if (find_clan(arg2) != NULL)
	  {
	     send_to_char("That clan already exists.\n\r",ch);
	   return;
	  }
	if (top_clan == MAX_CLAN)
	  {
	     send_to_char("Cannot create clan, max clans has been reached.\n\r", ch);
	     return;
	  }
	sprintf(buf, "Creating clan %s.\n\r", arg2);
	send_to_char(buf, ch);
        new_clan(arg2, ch);
	return;
     }
   else if (!str_cmp( arg1, "set"))
   {
      if (arg2[0] == '\0')
	{
	   send_to_char("You must provide a clan name.\n\r",ch);
	   return;
	}
      if ((Clan = find_clan(arg2) ) == NULL)
	{
	   send_to_char("That clan does not exist.\n\r",ch);
	   return;
	}
      if (arg3[0] == '\0')
	{
	   send_to_char("You must selecet a field to edit.\n\r",ch);
	   send_to_char("   Fields: name, visible, leader, god \n\r",ch);
	   send_to_char("           max, level, flags, race, class\n\r",ch);
	   send_to_char("		 accept\n\r",ch);
	   return;
	}
      
    /* Set something */
      
      if (!str_cmp(arg3, "name") )
	{
	   if (arg4[0] == '\0' )
	     {
		send_to_char("You must supply an argument.\n\r", ch);
		return;
	     }
	   
	   if (is_number(arg4) )
	     {
		send_to_char("Clan name must not be numeric.\n\r", ch);
		return;
	     }
	   sprintf(buf, "Clan %s will now be known as %s.\n\r", Clan->name, arg4);
	   send_to_char(buf,ch);
	   Clan->name = str_dup( arg4 );
	   return;
	}
      if (!str_cmp(arg3, "visible" ) )
	   {
	      if (arg4[0] == '\0' )
	     {
		send_to_char("You must supply an argument.\n\r", ch);
		return;
	     }
	      sprintf(buf, "Clan %s will now be shown as %s.\n\r", Clan->name, arg4);
	      Clan->visible = str_dup(arg4);
	      send_to_char(buf, ch);
	      return;
	   }
      if (!str_cmp(arg3, "leader" ) )
	{
	   FILE *fp;
	   
	   if (arg4[0] == '\0' )
	     {
		send_to_char("You must supply an argument.\n\r", ch);
		return;
	     }
	   sprintf(tempfile, "%s%s", PLAYER_DIR, capitalize(arg4) );
	   if ( (fp = fopen( tempfile, "r" ) ) != NULL )
	     {
		sprintf(buf, "%s is now the the leader of %s.\n\r", capitalize(arg4), Clan->visible);
		Clan->leader = str_dup(arg4);
		send_to_char(buf, ch);
		fclose(fp);
		return;
	     }
	   send_to_char("That character is not a player on this mud.\n\r", ch);
	   fclose(fp);
	   return;
	}
      if (!str_cmp( arg3, "god") )
	{
	   FILE *fp;
	   
	   if (arg4[0] == '\0' )
	     {
		send_to_char("You must supply an argument.\n\r", ch);
		return;
	     }
	   sprintf(tempfile, "%s%s", GOD_DIR, capitalize(arg4) );
	   if ( (fp = fopen( tempfile, "r" ) ) != NULL )
	     {
		sprintf( buf, "%s is now the god sponsor for %s.\n\r", capitalize(arg4), Clan->visible);
		Clan->god = str_dup(arg4);
		send_to_char(buf, ch);
		fclose(fp);
		return;
	     }
	   send_to_char("That player is not a god on this mud.\n\r", ch);
	   fclose(fp);
	   return;
	}
      if (!str_cmp( arg3, "max") )
	{
	   if (arg4[0] == '\0' )
	     {
		send_to_char("You must supply an argument.\n\r", ch);
		return;
	     }
	   if ( !is_number(arg4) )
	     {
		send_to_char("Max clan members must be a numeric value.\n\r", ch);
		return;
	     }
	   if ( value < 1 || value > MAX_CLAN_MEMBERS )
	     {
		sprintf(buf, "Max clan members must be between 1 and %d.\n\r", MAX_CLAN_MEMBERS);
		send_to_char( buf, ch);
		return;
	     }
	   sprintf(buf, "%s can now have %d members.\n\r", Clan->visible, value);
	   Clan->max_members = value;
	   send_to_char(buf, ch);
	   return;
	}
      if (!str_cmp( arg3, "level" ) )
	   {
	      if (arg4[0] == '\0' )
		{
		   send_to_char("You must supply an argument.\n\r", ch);
		   return;
		}
	      if ( !is_number(arg4) )
		{
		   send_to_char("Minimum level for clan must be a numeric value.\n\r", ch);
		   return;
		}
	      if (value < 1 || value > MAX_LEVEL + 2 )
		{
		   sprintf(buf, "Min level for clan must be between 1 and %d.\n\r", MAX_LEVEL+2);
		   send_to_char(buf, ch);
		   return;
		}
	      sprintf(buf, "The min level for %s is now %d.\n\r", Clan->visible, value);
	      Clan->min_level = value;
	      send_to_char(buf, ch);
	      return;
	   }
      if (!str_cmp( arg3, "reqs" ) )
	{
	   if (arg4[0] == '\0' )
	     {
		send_to_char("Flags:\n\r", ch);
		send_to_char("      PLR_KILLER\n\r", ch);
		return;
	     }
	   if ( str_cmp( arg4, "PLR_KILLER") )
	     {
		send_to_char("Flags:\n\r", ch);
		send_to_char("      PLR_KILLER\n\r", ch);
		return;
	     }
	   if (Clan->req_flags == PLR_KILLER) 
	     {
		sprintf( buf, "PK is no longer required by %s.\n\r", Clan->visible);
		send_to_char(buf, ch);
		Clan->req_flags = 0;
		return;
	     }
	   sprintf( buf, "PK is now required by %s.\n\r", Clan->visible);
	   send_to_char(buf, ch);
	   Clan->req_flags = PLR_KILLER;
	   return;
	}
      if (!str_cmp( arg3, "race") )
	{
	   int race;
	   
	   if (arg4[0] == '\0' )
	     {
		send_to_char("You must supply an argument.\n\r", ch);
		return;
	     }
	   race = race_lookup(arg4);
	   if ( race == 0  || !race_table[race].pc_race)
	     {
		send_to_char("That is not a valid player race.\n\r", ch);
		return;
	     }
	   if (Clan->races[race] == 1 )
	     {
		sprintf(buf, "%s are no longer allowed in %s.\n\r", pc_race_table[race].name, Clan->visible);
		Clan->races[race] = 0;
		send_to_char(buf, ch);
		return;
	     }
	   else
	     {
		sprintf(buf, "%s are now allowed in %s.\n\r", pc_race_table[race].name, Clan->visible);
		Clan->races[race] = 1;
		send_to_char(buf, ch);
		return;
	     }
	   bug("Error while setting clan race.", 0);
	   return;
	}
      if (!str_cmp( arg3, "class") )
	{
	   int class;
	   
	   if (arg4[0] == '\0' )
	     {
		send_to_char("You must supply an argument.\n\r", ch);
		return;
	     }
	   class = class_lookup(arg4);
	   if ( class == 0)
	     {
		send_to_char("That is not a valid class.\n\r", ch);
		return;
	     }
	   if (Clan->classes[class] == 1)
	     {
		sprintf(buf, "%ss are no longer allowed in %s.\n\r", class_table[class].name, Clan->visible);
		Clan->classes[class] = 0;
		send_to_char(buf, ch);
		return;
	     }
	   else
	     {
		sprintf(buf, "%ss are now allowed in %s.\n\r", class_table[class].name, Clan->visible);
		Clan->classes[class] = 1;
		send_to_char(buf, ch);
		return;
	     }
	   bug("error while setting clan class.", 0);
	   return;
	}
      if ( !str_cmp( arg3, "accept") )
	{
	   if (Clan->auto_accept == 0 )
	     {
		sprintf( buf, "%s will now auto-accept members.\n\r", Clan->visible);
		Clan->auto_accept = 1;
		send_to_char(buf, ch);
		return;
	     }
	   sprintf( buf, "%s will no longer auto-accept members.\n\r", Clan->visible);
	   Clan->auto_accept = 0;
	   send_to_char(buf, ch);
	   return;
	}
   }
/*
 * Generate Usage Message   
 */
   do_clan(ch, "");
   return;
}

bool is_clan_leader(char *name, char *Clan)
{
   CLAN_DATA    *pClan;
   
   pClan = find_clan(Clan);
   if (!str_cmp(name, pClan->leader) )
     {
	return TRUE;
     }
   else
     return FALSE;
}

char * vis_clan(int clan)
{
    CLAN_DATA * pClan;
    
    for ( pClan=clan_first; pClan != NULL ; pClan=pClan->next)
        if ( pClan->number == clan)
    	    return( pClan->visible );
    return( "" );
}

char * clan_lookup(int clan)
{
   CLAN_DATA *pClan;
   
   for ( pClan=clan_first; pClan !=NULL; pClan=pClan->next)
     {
	if ( pClan->number == clan)
	  return( pClan->name );
     }
   return( "" );
}
