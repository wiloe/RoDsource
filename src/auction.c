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
#include <ctype.h>
#include "merc.h"


/*
 * Erwins Auction System by Kyndig
 */

/* modified by spellsong to replace commands and messages to the more commonly known bid*/

/*
DECLARE_SPELL_FUN(      spell_identify  );

*/
void talk_auction(char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

	buf[0] = '\0'; /*spellsong add*/

    sprintf (buf,"AUCTION: %s", argument);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING))
            act (buf, original, NULL, NULL, TO_CHAR);

    }
}


/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/

int advatoi (const char *s)
/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.

*/

{

/* the pointer to buffer stuff is not really necessary, but originally I
   modified the buffer, so I had to make a copy of it. What the hell, it 
   works:) (read: it seems to work:)
*/

  char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
  char *stringptr = string; /* a pointer to the buffer so we can move around */
  char tempstring[2];       /* a small temp buffer to pass to atoi*/
  int number = 0;           /* number to be returned */
  int multiplier = 0;       /* multiplier used to get the extra digits right */


  strcpy (string,s);        /* working copy */

  while ( isdigit (*stringptr)) /* as long as the current character is a digit */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      number = (number * 10) + atoi (tempstring); /* add to current number */
      stringptr++;                                /* advance */
  }

  switch (UPPER(*stringptr)) {
      case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
      case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
      case '\0' : break;
      default   : return 0; /* not k nor m nor NUL - return 0! */
  }

  while ( isdigit (*stringptr) && (multiplier > 1)) /* if any digits follow k/m, add those too
*/
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      multiplier = multiplier / 10;  /* the further we get to right, the less are the digit
'worth' */
      number = number + (atoi (tempstring) * multiplier);
      stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other
than NUL */
    return 0; /* If a digit is found, it means the multiplier is 1 - i.e. extra
                 digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


  return (number);
}


int parsebet (const int currentbet, const char *argument)
{

  int newbet = 0;                /* a variable to temporarily hold the new bet */
  char string[MAX_INPUT_LENGTH]; /* a buffer to modify the bet string */
  char *stringptr = string;      /* a pointer we can move around */
  char buf2[MAX_STRING_LENGTH];

  buf2[0] = '\0'; /*spellsong add*/

  strcpy (string,argument);      /* make a work copy of argument */


  if (*stringptr)               /* check for an empty string */
  {

    if (isdigit (*stringptr)) /* first char is a digit assume e.g. 433k */
      newbet = advatoi (stringptr); /* parse and set newbet to that value */

    else
      if (*stringptr == '+') /* add ?? percent */
      {
        if (strlen (stringptr) == 1) /* only + specified, assume default */
          newbet = (currentbet * 110) / 100; /* default: add 25% */ /*Changed by spellsong to 10%*/
        else
          newbet = (currentbet * (100 + atoi (++stringptr))) / 100; /* cut off the first char
*/
      }
      else
        {
        sprintf (buf2,"considering: * x \n\r");
        if ((*stringptr == '*') || (*stringptr == 'x')) /* multiply */
        {
          if (strlen (stringptr) == 1) /* only x specified, assume default */
            newbet = currentbet * 2 ; /* default: twice */
          else /* user specified a number */
            newbet = currentbet * atoi (++stringptr); /* cut off the first char */
        }
        }
  }

  return newbet;        /* return the calculated bet */
}


void auction_update (void)
{
    char buf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/

    if (auction->item != NULL)
        if (--auction->pulse <= 0) /* decrease pulse */
        {
            auction->pulse = PULSE_AUCTION;
            switch (++auction->going) /* increase the going state */
            {
            case 1 : /* going once */
            case 2 : /* going twice */
            if (auction->bet > 0) /*oops typo mike*/
                sprintf (buf, "%s: going %s for %d.", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"), auction->bet);
            else
                sprintf (buf, "%s: going %s (no bid received yet).", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"));
            talk_auction (buf);
            break;

            case 3 : /* SOLD! */

            if (auction->bet > 0)
            {
                sprintf (buf, "%s sold to %s for %d.",
                    auction->item->short_descr,
                    IS_NPC(auction->buyer) ? auction->buyer->short_descr :
                    auction->buyer->name,
                    auction->bet);
                talk_auction(buf);
                obj_to_char (auction->item,auction->buyer);
                act ("The auctioneer appears before you in a puff of smoke and hands you $p.",
                     auction->buyer,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n, and hands $m $p",
                     auction->buyer,auction->item,NULL,TO_ROOM);

                auction->seller->gold += auction->bet; /* give him the money */

                auction->item = NULL; /* reset item */

            }
            else /* not sold */
            {
                sprintf (buf, "No bids received for %s - object has been removed.",
                               auction->item->short_descr);
                talk_auction(buf);
                act ("The auctioneer appears before you to return $p to you.",
                      auction->seller,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n to return $p to $m.",
                      auction->seller,auction->item,NULL,TO_ROOM);
                obj_to_char (auction->item,auction->seller);
                auction->item = NULL; /* clear auction */
            } /* else */

            } /* switch */
        } /* if */
} /* func */


void do_auction (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char bufc[MAX_STRING_LENGTH];
    char betbuf[MAX_STRING_LENGTH];

	buf[0] = '\0'; /*spellsong add*/
	bufc[0] = '\0'; /*spellsong add*/
	betbuf[0] = '\0'; /*spellsong add*/

    argument = one_argument (argument, arg1);

        if (IS_NPC(ch))    /* NPC extracted can't auction! */
                return;

/*    if (IS_SET(ch->comm,COMM_NOAUCTION))
        {
         if (!str_cmp(arg1,"on") )
         {
          send_to_char("Auction channel is now ON.\n\r",ch);
          REMOVE_BIT(ch->comm,COMM_NOAUCTION);
          return;
         }
         else
         {
          send_to_char("Your auction channel is OFF.\n\r",ch);
          send_to_char("You must first turn the auction channel ON.\n\r",ch);
          return;
         }
        }
*/
    if (arg1[0] == '\0') 
    {
        if (auction->item != NULL)
        {
            /* show item data here */
            if (auction->bet > 0)
                sprintf (buf, "Current bid on this item is %d gold.\n\r",auction->bet);
            else
                sprintf (buf, "No bids on this item have been received.\n\r");
            send_to_char (buf,ch);
           /* spell_identify(0,0,ch, auction->item);
            */return;
        }
        else
        {
            sprintf(bufc,"Auction WHAT?\n\r");
            send_to_char (bufc,ch);
            return;
        }
    }

/*    if (!str_cmp(arg1,"off") )
        {
         send_to_char("Auction channel is now OFF.\n\r",ch);
         SET_BIT(ch->comm,COMM_NOAUCTION);
         return;
        }
*/

    if (IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
    {
      if (auction->item == NULL)
      {
        send_to_char ("There is no auction going on you can stop.\n\r",ch);
        return;
      }
      else /* stop the auction */
      {
        sprintf(buf,"Sale of %s has been stopped by the Gods. Item removed.",
                        auction->item->short_descr);
        talk_auction(buf);
        obj_to_char(auction->item, auction->seller);
        auction->item = NULL;
        if (auction->buyer != NULL) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char ("Your money has been returned.\n\r",auction->buyer);
        }
        return;
      }
    }

    if  (!str_cmp(arg1,"bid") ) 
    {
        if (auction->item != NULL)
        {
            int newbet;

        if ( ch == auction->seller )
        {
           send_to_char("You cannot bid on your own selling equipment...:)\n\r",ch);
            return;
        }
            /* make - perhaps - a bet now */
            if (argument[0] == '\0')
            {
                send_to_char ("Bid how much?\n\r",ch);
                return;
            }

            newbet = parsebet (auction->bet, argument);
            sprintf (betbuf,"Bet: %d\n\r",newbet);

            if (newbet < (auction->bet + 1))
            {
                send_to_char ("You must at least bid 1 gold over the current bid.\n\r",ch);
                return;
            }

            if (newbet > ch->gold)
            {
                send_to_char ("You don't have that much money!\n\r",ch);
                return;
            }

            /* the actual bet is OK! */

            /* return the gold to the last buyer, if one exists */
            if (auction->buyer != NULL)
                auction->buyer->gold += auction->bet;

            ch->gold -= newbet; /* substract the gold - important :) */
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION; /* start the auction over again */

            sprintf (buf,"A bid of %d gold has been received on %s.\n\r",newbet,auction->item->short_descr);
            talk_auction (buf);
            return;


        }
        else
        {
            send_to_char ("There isn't anything being auctioned right now.\n\r",ch);
            return;
        }

    /* finally... */
    }
    obj = get_obj_carry (ch, arg1 ); /* does char have the item ? */ 

    if (obj == NULL)
    {
        send_to_char ("You aren't carrying that.\n\r",ch);
        return;
    }

    if (auction->item == NULL)
    switch (obj->item_type)
    {

    default:
        act("$CYou cannot auction $Ts.$c",ch, NULL, item_type_name(obj),TO_CHAR);
        return;

    case ITEM_WEAPON:
    case ITEM_ARMOR:
    case ITEM_STAFF:
    case ITEM_WAND:
    case ITEM_SCROLL:
        obj_from_char (obj);
        auction->item = obj;
        auction->bet = 0;       /* obj->cost / 100 */
        auction->buyer = NULL;
        auction->seller = ch;
        auction->pulse = PULSE_AUCTION;
        auction->going = 0;

        sprintf(buf, "A new item has been received: %s.", obj->short_descr);
        talk_auction(buf);

        return;

    } /* switch */
    else
    {
        act ("Try again later - $p is being auctioned right now!",ch,auction->item,NULL,TO_CHAR);
        return;
    }
}



