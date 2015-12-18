/* Healer code written for Merc 2.0 muds by Alander 
   direct questions or comments to rtaylor@cie-2.uoregon.edu
   any use of this code must include this header */

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

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        /* display price list */
	act("$N says 'I offer the following spells:'",ch,NULL,mob,TO_CHAR);
	send_to_char("  patch:     patch wounds       50 gold\n\r",ch);
	send_to_char("  bind:      bind wounds        80 gold\n\r",ch);
	send_to_char("  mend:      mend wounds       125 gold\n\r",ch);
	send_to_char("  repair:    repair wounds     250 gold\n\r",ch);
	send_to_char("  vision:    restore vision    100 gold\n\r",ch);
	send_to_char("  disease:   cure disease       75 gold\n\r",ch);
	send_to_char("  alleviate: alleviate poison  125 gold\n\r",ch); 
	send_to_char("  uncurse:   remove curse	     250 gold\n\r",ch);
	send_to_char("  energy:    restore movement   25 gold\n\r",ch);
	send_to_char("  spell:     restore mana	     100 gold\n\r",ch);
	send_to_char(" Type heal <type> to be healed.\n\r",ch);
	return;
    }

    switch (arg[0])
    {
	case 'p' :
	    spell = spell_patch_wounds;
	    sn    = skill_lookup("patch wounds");
	    words = "judicandus dies";
	    cost  = 50;
	    break;

	case 'b' :
	    spell = spell_bind_wounds;
	    sn    = skill_lookup("bind wounds");
	    words = "judicandus gzfuajg";
	    cost  = 80;
	    break;

	case 'm' :
	    spell = spell_mend_wounds;
	    sn    = skill_lookup("mend wounds");
	    words = "judicandus qfuhuqar";
	    cost  = 125;
	    break;

	case 'r' :
	    spell = spell_repair_wounds;
	    sn = skill_lookup("repair wounds");
	    words = "pzar";
	    cost  = 250;
	    break;

	case 'v' :
	    spell = spell_cure_blindness;
	    sn    = skill_lookup("cure blindness");
      	    words = "judicandus noselacri";		
            cost  = 100;
	    break;

    	case 'd' :
	    spell = spell_cure_disease;
	    sn    = skill_lookup("cure disease");
	    words = "judicandus eugzagz";
	    cost = 75;
	    break;

	case 'a' :
	    spell = spell_cure_poison;
	    sn    = skill_lookup("cure poison");
	    words = "judicandus sausabru";
	    cost  = 125;
	    break;
	
	case 'u' :
	    spell = spell_remove_curse; 
	    sn    = skill_lookup("remove curse");
	    words = "candussido judifgz";
	    cost  = 250;
	    break;

	case 'e' :
	    spell =  spell_refresh;
	    sn    = skill_lookup("refresh");
	    words = "candusima"; 
	    cost  = 25;
	    break;

	case 's' :
	    spell = NULL;
	    sn = -1;
	    words = "energizer";
	    cost = 100;
	    break;

	default :
	    act("$N says 'Type 'heal' for a list of spells.'",
	        ch,NULL,mob,TO_CHAR);
	    return;
    }

    if (cost > ch->gold)
    {
	act("$N says 'You do not have enough gold for my services.'",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    ch->gold -= cost;
    mob->gold += cost;
    act("$n utters the words '$T'.",mob,NULL,words,TO_ROOM);
  
    if (spell == NULL)  /* restore mana trap...kinda hackish */
    {
	ch->mana += dice(2,8) + mob->level / 2;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	send_to_char("A warm glow passes through you.\n\r",ch);
	return;
     }

     if (sn == -1)
	return;
    
     spell(sn,mob->level,mob,ch);
}
