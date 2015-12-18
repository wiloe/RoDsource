/* db_new.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include "merc.h"
#include "db.h"


/*
 * MOBprogram locals
*/

/*log_string( "Load_resets: bad command" );*/

int 		mprog_name_to_type	args ( ( char* name ) );
MPROG_DATA *	mprog_file_read 	args ( ( char* f, MPROG_DATA* mprg, 
						MOB_INDEX_DATA *pMobIndex ) );
void		load_mobprogs           args ( ( FILE* fp ) );
void   		mprog_read_programs     args ( ( FILE* fp,
					        MOB_INDEX_DATA *pMobIndex ) );

/*
 * Snarf a mob section.  new style
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
 
    if ( !area_last )   /* OLC */
    {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
		log_string( "db2 - load mobiles: no area seen yet" );
        exit( 1 );
    }
    for ( ; ; )
    {
        sh_int vnum;
        char letter,temp;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
			log_string( "db2 - load mobiles: # not found" );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
			log_string( "db2 - load mobiles: vnum duplicated" );
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pMobIndex                       = alloc_perm( sizeof(*pMobIndex) );
        pMobIndex->vnum                 = vnum;
        pMobIndex->area                 = area_last;               /* OLC */
	pMobIndex->new_format		= TRUE;
	newmobs++;
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );
	pMobIndex->race		 	= race_lookup(fread_string( fp ));
 
        pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
 
        pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC
					| race_table[pMobIndex->race].act;
        pMobIndex->affected_by          = fread_flag( fp )
					| race_table[pMobIndex->race].aff;
        pMobIndex->pShop                = NULL;
        pMobIndex->alignment            = fread_number( fp );
        letter                          = fread_letter( fp );

        pMobIndex->level                = fread_number( fp );
        pMobIndex->hitroll              = fread_number( fp );  

	/* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
        /* 'd'          */                fread_letter( fp ); 
        pMobIndex->hit[DICE_TYPE]   	= fread_number( fp );
        /* '+'          */                fread_letter( fp );   
        pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

 	/* read mana dice */
	pMobIndex->mana[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_BONUS]	= fread_number( fp );

	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
	pMobIndex->dam_type		= fread_number( fp );

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_BASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_SLASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number( fp ) * 10;

	/* read flags and add in data from the race table */
	pMobIndex->off_flags		= fread_flag( fp ) 
					| race_table[pMobIndex->race].off;
	pMobIndex->imm_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].imm;
	pMobIndex->res_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].res;
	pMobIndex->vuln_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].vuln;
	/* vital statistics */
	pMobIndex->start_pos		= fread_number( fp );
	pMobIndex->default_pos		= fread_number( fp );

	/* Akira's fix-o-matic */
	if(pMobIndex->start_pos == POS_FIGHTING)
		pMobIndex->start_pos = POS_STANDING;
	if(pMobIndex->default_pos == POS_FIGHTING)
		pMobIndex->default_pos = POS_STANDING;

	pMobIndex->sex			= fread_number( fp );
	pMobIndex->gold			= fread_number( fp );
	
	pMobIndex->form			= fread_flag( fp )
					| race_table[pMobIndex->race].form;
	pMobIndex->parts		= fread_flag( fp )
					| race_table[pMobIndex->race].parts;
	/* size */
	temp				= fread_letter( fp );
	switch (temp)
	{
	    case ('T') :		pMobIndex->size = SIZE_TINY;	break;
	    case ('S') :		pMobIndex->size = SIZE_SMALL;	break;
	    case ('M') :		pMobIndex->size = SIZE_MEDIUM;	break;
	    case ('L') :		pMobIndex->size = SIZE_LARGE; 	break;
	    case ('H') :		pMobIndex->size = SIZE_HUGE;	break;
	    case ('G') :		pMobIndex->size = SIZE_GIANT;	break;
	    default:			pMobIndex->size = SIZE_MEDIUM; break;
	}
	pMobIndex->material		= material_lookup(fread_word( fp ));
 
 	pMobIndex->clan=letter-64;
 	if (letter == 'S') pMobIndex->clan=0;
	        letter = fread_letter( fp );
	                if ( letter == '>' )
	                        {
	                         ungetc( letter, fp );
	                         mprog_read_programs( fp, pMobIndex );
	                         }
	                else ungetc( letter,fp );
	                                                             
        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
        top_mob_index++;
        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
        assign_area_vnum( vnum );                                  /* OLC */
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}

/*
 * Snarf an obj section. new style
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
 
    if ( !area_last )   /* OLC */
    {
        bug( "Load_objects: no #AREA seen yet.", 0 );
		log_string( "db2 - load objects: no area seen yet" );
        exit( 1 );
    }
    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
            log_string( "db2 - load objects: #not found" );
			exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            log_string( "db2 - load object: vnum duplicated" );
			exit( 1 );
        }
        fBootDb = TRUE;
 
        pObjIndex                       = alloc_perm( sizeof(*pObjIndex) );

	/* For those location strings */
	pObjIndex->location_string=NULL;

        pObjIndex->vnum                 = vnum;
        pObjIndex->area                 = area_last;            /* OLC */
        pObjIndex->new_format           = TRUE;
	pObjIndex->reset_num		= 0;
	newobjs++;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
        pObjIndex->material		= material_lookup(fread_string( fp ));
 
        pObjIndex->item_type            = fread_number( fp );
        pObjIndex->extra_flags          = fread_flag( fp );
        pObjIndex->wear_flags           = fread_flag( fp );
	pObjIndex->furn_flags		= fread_flag( fp );
	pObjIndex->value[0]             = fread_flag( fp );
	pObjIndex->value[1]             = fread_flag( fp );
        pObjIndex->value[2]             = fread_flag( fp );
        pObjIndex->value[3]             = fread_flag( fp );
	pObjIndex->value[4]		= fread_flag( fp );
	pObjIndex->level		= fread_number( fp );
        pObjIndex->weight               = fread_number( fp );
        pObjIndex->cost                 = fread_number( fp ); 

        /* condition */
        letter 				= fread_letter( fp );
	switch (letter)
 	{
	    case ('P') :		pObjIndex->condition = 100; break;
	    case ('G') :		pObjIndex->condition =  90; break;
	    case ('A') :		pObjIndex->condition =  75; break;
	    case ('W') :		pObjIndex->condition =  50; break;
	    case ('D') :		pObjIndex->condition =  25; break;
	    case ('B') :		pObjIndex->condition =  10; break;
	    case ('R') :		pObjIndex->condition =   0; break;
	    default:			pObjIndex->condition = 100; break;
	}
 
        for ( ; ; )
        {
            char letter;
 
            letter = fread_letter( fp );
 
            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm( sizeof(*paf) );
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number( fp );
                paf->modifier           = fread_number( fp );
                paf->bitvector          = 0;
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;
            }
 
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;
 
                ed                      = alloc_perm( sizeof(*ed) );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pObjIndex->extra_descr;
                pObjIndex->extra_descr  = ed;
                top_ed++;
            }

	    else if( letter == 'L')
		pObjIndex->location_string = fread_string( fp );
 
            else
            {
                ungetc( letter, fp );
                break;
            }
        }

        /*
         * Translate spell "slot numbers" to internal "skill numbers."
         */
        switch ( pObjIndex->item_type )
        {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
            pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
            pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
            break;
 
        case ITEM_STAFF:
        case ITEM_WAND:
            pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
            break;
        }
 
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
        assign_area_vnum( vnum );                                   /* OLC */
    }
 
    return;
}

/*****************************************************************************
 Name:	        convert_objects
 Purpose:	Converts all old format objects to new format
 Called by:	boot_db (db.c).
 Note:          Loops over all resets to find the level of the mob
                loaded before the object to determine the level of
                the object.
		It might be better to update the levels in load_resets().
		This function is not pretty.. Sorry about that :)
 Author:        Hugin
 ****************************************************************************/
void convert_objects( void )
{
    int vnum;
    AREA_DATA  *pArea;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;

    if ( newobjs == top_obj_index ) return; /* all objects in new format */

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
	{
	    if ( !( pRoom = get_room_index( vnum ) ) ) continue;

	    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	    {
		switch ( pReset->command )
		{
		case 'M':
		    if ( !( pMob = get_mob_index( pReset->arg1 ) ) )
			bug( "Convert_objects: 'M': bad vnum %d.", pReset->arg1 );
		    break;

		case 'O':
		    if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
		    {
			bug( "Convert_objects: 'O': bad vnum %d.", pReset->arg1 );
			break;
		    }

		    if ( pObj->new_format )
			continue;

		    if ( !pMob )
		    {
			bug( "Convert_objects: 'O': No mob reset yet.", 0 );
			break;
		    }

		    pObj->level = pObj->level < 1 ? pMob->level - 2
			: UMIN(pObj->level, pMob->level - 2);
		    break;

		case 'P':
		    {
			OBJ_INDEX_DATA *pObj, *pObjTo;

			if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
			{
			    bug( "Convert_objects: 'P': bad vnum %d.", pReset->arg1 );
			    break;
			}

			if ( pObj->new_format )
			    continue;

			if ( !( pObjTo = get_obj_index( pReset->arg3 ) ) )
			{
			    bug( "Convert_objects: 'P': bad vnum %d.", pReset->arg3 );
			    break;
			}

			pObj->level = pObj->level < 1 ? pObjTo->level
			    : UMIN(pObj->level, pObjTo->level);
		    }
		    break;

		case 'G':
		case 'E':
		    if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
		    {
			bug( "Convert_objects: 'E' or 'G': bad vnum %d.", pReset->arg1 );
			break;
		    }

		    if ( !pMob )
		    {
			bug( "Convert_objects: 'E' or 'G': null mob for vnum %d.",
			     pReset->arg1 );
			break;
		    }

		    if ( pObj->new_format )
			continue;

		    if ( pMob->pShop )
		    {
			switch ( pObj->item_type )
			{
			default:
			    pObj->level = UMAX(0, pObj->level);
			    break;
			case ITEM_PILL:
			case ITEM_POTION:
			    pObj->level = UMAX(5, pObj->level);
			    break;
			case ITEM_SCROLL:
			case ITEM_ARMOR:
			case ITEM_WEAPON:
			    pObj->level = UMAX(10, pObj->level);
			    break;
			case ITEM_WAND:
			case ITEM_TREASURE:
			    pObj->level = UMAX(15, pObj->level);
			    break;
			case ITEM_STAFF:
			    pObj->level = UMAX(20, pObj->level);
			    break;
			}
		    }
		    else
			pObj->level = pObj->level < 1 ? pMob->level
			    : UMIN( pObj->level, pMob->level );
		    break;
		} /* switch ( pReset->command ) */
	    }
	}
    }

    /* do the conversion: */

    for ( pArea = area_first; pArea ; pArea = pArea->next )
	for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
	    if ( (pObj = get_obj_index( vnum )) )
 		if ( !pObj->new_format )
		    convert_object( pObj );

    return;
}



/*****************************************************************************
 Name:		convert_object
 Purpose:	Converts an old_format obj to new_format
 Called by:	convert_objects (db2.c).
 Note:          Dug out of create_obj (db.c)
 Author:        Hugin
 ****************************************************************************/
void convert_object( OBJ_INDEX_DATA *pObjIndex )
{
    int level;
    int number, type;  /* for dice-conversion */

    if ( !pObjIndex || pObjIndex->new_format ) return;

    level = pObjIndex->level;

    pObjIndex->level    = UMAX( 0, pObjIndex->level ); /* just to be sure */
    pObjIndex->cost     = 10*level;

    switch ( pObjIndex->item_type )
    {
        default:
            bug( "Obj_convert: vnum %d bad type.", pObjIndex->item_type );
            break;

        case ITEM_LIGHT:
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_SCROLL:
	    break;

        case ITEM_WAND:
        case ITEM_STAFF:
            pObjIndex->value[2] = pObjIndex->value[1];
	    break;

        case ITEM_WEAPON:

	    /*
	     * The conversion below is based on the values generated
	     * in one_hit() (fight.c).  Since I don't want a lvl 50 
	     * weapon to do 15d3 damage, the min value will be below
	     * the one in one_hit, and to make up for it, I've made 
	     * the max value higher.
	     * (I don't want 15d2 because this will hardly ever roll
	     * 15 or 30, it will only roll damage close to 23.
	     * I can't do 4d8+11, because one_hit there is no dice-
	     * bounus value to set...)
	     *
	     * The conversion below gives:

	     level:   dice      min      max      mean
	       1:     1d8      1( 2)    8( 7)     5( 5)
	       2:     2d5      2( 3)   10( 8)     6( 6)
	       3:     2d5      2( 3)   10( 8)     6( 6)
	       5:     2d6      2( 3)   12(10)     7( 7)
	      10:     4d5      4( 5)   20(14)    12(10)
	      20:     5d5      5( 7)   25(21)    15(14)
	      30:     5d7      5(10)   35(29)    20(20)
	      50:     5d11     5(15)   55(44)    30(30)

	     */

	    number = UMIN(level/4 + 1, 5);
	    type   = (level + 7)/number;

            pObjIndex->value[1] = number;
            pObjIndex->value[2] = type;
	    break;

        case ITEM_ARMOR:
            pObjIndex->value[0] = level / 5 + 3;
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[0];
	    break;

        case ITEM_POTION:
        case ITEM_PILL:
            break;

        case ITEM_MONEY:
	    pObjIndex->value[0] = pObjIndex->cost;
	    break;
    }

    pObjIndex->new_format = TRUE;
    ++newobjs;

    return;
}




/*****************************************************************************
 Name:		convert_mobile
 Purpose:	Converts an old_format mob into new_format
 Called by:	load_old_mob (db.c).
 Note:          Dug out of create_mobile (db.c)
 Author:        Hugin
 ****************************************************************************/
void convert_mobile( MOB_INDEX_DATA *pMobIndex )
{
    int i;
    int type, number, bonus;
    int level;

    if ( !pMobIndex || pMobIndex->new_format ) return;

    level = pMobIndex->level;

    pMobIndex->act              |= ACT_WARRIOR;

    /*
     * Calculate hit dice.  Gives close to the hitpoints
     * of old format mobs created with create_mobile()  (db.c)
     * A high number of dice makes for less variance in mobiles
     * hitpoints.
     * (might be a good idea to reduce the max number of dice)
     *
     * The conversion below gives:

       level:     dice         min         max        diff       mean
         1:       1d2+6       7(  7)     8(   8)     1(   1)     8(   8)
	 2:       1d3+15     16( 15)    18(  18)     2(   3)    17(  17)
	 3:       1d6+24     25( 24)    30(  30)     5(   6)    27(  27)
	 5:      1d17+42     43( 42)    59(  59)    16(  17)    51(  51)
	10:      3d22+96     99( 95)   162( 162)    63(  67)   131(    )
	15:     5d30+161    166(159)   311( 311)   145( 150)   239(    )
	30:    10d61+416    426(419)  1026(1026)   600( 607)   726(    )
	50:    10d169+920   930(923)  2610(2610)  1680(1688)  1770(    )

	The values in parenthesis give the values generated in create_mobile.
        Diff = max - min.  Mean is the arithmetic mean.
	(hmm.. must be some roundoff error in my calculations.. smurfette got
	 1d6+23 hp at level 3 ? -- anyway.. the values above should be
	 approximately right..)
     */
    type   = level*level*27/40;
    number = UMIN(type/40 + 1, 10); /* how do they get 11 ??? */
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, level*(8 + level)*.9 - number*type);

    pMobIndex->hit[DICE_NUMBER]    = number;
    pMobIndex->hit[DICE_TYPE]      = type;
    pMobIndex->hit[DICE_BONUS]     = bonus;

    pMobIndex->mana[DICE_NUMBER]   = level;
    pMobIndex->mana[DICE_TYPE]     = 10;
    pMobIndex->mana[DICE_BONUS]    = 100;

    /*
     * Calculate dam dice.  Gives close to the damage
     * of old format mobs in damage()  (fight.c)
     */
    type   = level*7/4;
    number = UMIN(type/8 + 1, 5);
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, level*9/4 - number*type);

    pMobIndex->damage[DICE_NUMBER] = number;
    pMobIndex->damage[DICE_TYPE]   = type;
    pMobIndex->damage[DICE_BONUS]  = bonus;

    switch ( number_range( 1, 3 ) )
    {
        case (1): pMobIndex->dam_type =  3;       break;  /* slash  */
        case (2): pMobIndex->dam_type =  7;       break;  /* pound  */
        case (3): pMobIndex->dam_type = 11;       break;  /* pierce */
    }

    for (i = 0; i < 3; i++)
        pMobIndex->ac[i]         = interpolate( level, 100, -100);
    pMobIndex->ac[3]             = interpolate( level, 100, 0);    /* exotic */

    pMobIndex->gold             /= 100;
    pMobIndex->size              = SIZE_MEDIUM;
    pMobIndex->material          = 0;

    pMobIndex->new_format        = TRUE;
    ++newmobs;

    return;
}

