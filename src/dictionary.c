/******************************************************************
********************************************************************
**      |        \ /         _-                                   **
**  ---------     /  /      |         More code by Akira          **
**      |        / \/       |         OLC Spellcheck, y2000       **
**    --|--- /  /  /\       |                                     **
**   /  |   X     / _\      |___      Use only with permission    **
**  |   |  / \     /            \                                 **
**   \__|_/       |              |    "If you don't like it, then **
**      |          \___/    \___/      you can code it yourself!" **
********************************************************************
 ******************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/* Change to your liking */
#define OBJS_PER_CHECK 20
#define MOBS_PER_CHECK 15
#define ROOMS_PER_CHECK 10

#define SP_STATE_AREA_OBJ '1'
#define SP_STATE_AREA_MOB '2'
#define SP_STATE_AREA_ROOM '3'
#define SP_STATE_AREA_FIRST SP_STATE_AREA_OBJ
#define SP_STATE_AREA_FINAL SP_STATE_AREA_ROOM

typedef struct spellcheck_type SP_TYPE;
typedef struct queue_type Q_DATA;
typedef struct str_list STR_LIST;

struct str_list
{
	char *str;
	STR_LIST *next;
};

struct spellcheck_type
{
	CHAR_DATA *ch;
	char what;
	sh_int where;
	AREA_DATA *a;
};

struct queue_type
{
	Q_DATA *next;
	CHAR_DATA *ch;
	AREA_DATA *a;
};

struct spellcheck_type *current=NULL;
struct queue_type *firstqueue=NULL;
struct queue_type *lastqueue=NULL;

#define DICTIONARY_FILE "../area/dictionary.txt"
long NUM_DICTIONARY_WORDS;/*Originally 63247*/

char **dictionary_words;
STR_LIST *added_words=NULL; /*Always kept in sorted order*/

/* Local functions */
void spellcheck_str(CHAR_DATA *ch, char *str, const char *where);
bool word_in_dict(const char *str);
void spellcheck_obj(CHAR_DATA *ch, sh_int vnum);
void spellcheck_mob(CHAR_DATA *ch, sh_int vnum);
void spellcheck_room(CHAR_DATA *ch, sh_int vnum);
void spellcheck_extradesc(CHAR_DATA *ch, EXTRA_DESCR_DATA *ed, const char *where);

void check_queue();
char * parse(char *argument, char *arg_first);

void fwrite_dictionary();

void do_addword(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	
	argument=one_argument(argument,arg);
	
	if(arg[0]=='\0')
	{
		send_to_char("Syntax:\n\r\taddword <single word>\n\r\taddword save\n\r",ch);
		send_to_char("\n\rIf a mistake is made, please note Akira.\n\r",ch);
		return;
	}
	if(!str_cmp(arg,"save"))
	{
		fwrite_dictionary();
		send_to_char("Saved.\n\r",ch);
		return;
	}
	
	if(word_in_dict(arg))
	{
		send_to_char("That word is already in the dictionary.\n\r",ch);
		return;
	}
	
	{
		STR_LIST *data;
		STR_LIST *trans=NULL;
		data = malloc(sizeof(data));
		data->str=str_dup(arg);
		if(added_words==NULL)
		{
			added_words=data;
			added_words->next=NULL;
			return;
		}
		else if(strcmp(data->str,added_words->str)<0)
		{
			data->next=added_words;
			added_words=data;
			return;
		}
		for(trans=added_words;trans->next;trans=trans->next)
			if(strcmp(data->str,trans->next->str)<0)//data<trans->next
			{
				data->next=trans->next->next;
				trans->next=data;
				return;
			}
		if(strcmp(data->str,trans->str)<0)
			log_string("Assertion failed! addword algorithm flawed!");
		trans->next=data;
		data->next=NULL;
		return;
	}
}

void do_spellcheck(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	sh_int vnum=0;
	
	argument=one_argument(argument,arg);
	
	if(arg[0]=='\0')
	{
		send_to_char("Syntax:\n\r",ch);
		send_to_char("\tspellcheck obj <vnum>\n\r",ch);
		send_to_char("\tspellcheck mob <vnum>\n\r",ch);
		send_to_char("\tspellcheck room\n\r",ch);
		send_to_char("\tspellcheck area\n\r",ch);
		send_to_char("\n\rNote: Spellchecking an area might be delayed.\n\r",ch);
		return;
	}

	if(!str_cmp(arg,"area"))
	{
		Q_DATA *q;
		q=malloc(sizeof q);
		q->ch=ch;
		q->a=ch->in_room->area;
		q->next=NULL;
		if(!firstqueue)
		{
			firstqueue=q;
			lastqueue=q;
		}
		else
			lastqueue->next=q;
		if(current)
			send_to_char("Spellcheck request queued, please wait.\n\r",ch);

		return;		
	}
	
	if(!str_cmp(arg,"room"))
	{
		spellcheck_room(ch,ch->in_room->vnum);
		return;
	}

	if(argument[0]=='\0' || !is_number(argument))
	{
		do_spellcheck(ch,"");
		return;
	}

	vnum=atoi(argument);

	if(!str_cmp(arg,"obj"))
	{
		spellcheck_obj(ch,vnum);
		return;
	}
	
	if(!str_cmp(arg,"mob"))
	{
		spellcheck_mob(ch,vnum);
		return;
	}
	
	do_spellcheck(ch,"");
}


bool word_in_dict(const char *str)
{
	long i;
	STR_LIST *trans;
	for(i=0;i<NUM_DICTIONARY_WORDS;i++)
	{
		if(!strcmp(str,dictionary_words[i]))
			return TRUE;
	}
	for(trans=added_words;trans;trans=trans->next)
	{
		if(!strcmp(str,trans->str))
			return TRUE;
	}	
	return FALSE;
}

void spellcheck_str(CHAR_DATA *ch, char *str, const char *where)
{
	if(!str)
		return;
	while(str[0]!='\0')
	{
		char word[MAX_STRING_LENGTH];
		str=parse(str,word);
		if(word[0]=='\0')
			break;
		if(!word_in_dict(word))
		{
			char mesg[MAX_STRING_LENGTH];
			sprintf(mesg,"Questionable word- %s: %s\n\r",where,word);
			send_to_char(mesg,ch);
		}
	}
}

/* Called at init, aborts if failure */
void load_dictionary()
{
	FILE *fp;
	long i;

	fp = fopen(DICTIONARY_FILE, "r");
	if(!fp)
	{
		log_string("Dictionary file not found. Aborting");
		exit(1);
	}
	
	NUM_DICTIONARY_WORDS=fread_number(fp);
	if(NUM_DICTIONARY_WORDS<=0)
	{
		log_string("Impossible value for num dictionary words");
		exit(1);
	}
	dictionary_words=malloc(NUM_DICTIONARY_WORDS*sizeof(char *));
	
	for(i=0;i<NUM_DICTIONARY_WORDS;i++)
	{
		char *word;
		if(feof(fp))
		{
			log_string("NUM_DICTIONARY_WORDS incorrect, dictionary load failed");
			exit(1);
		}
		word=fread_word(fp);
		dictionary_words[i]=str_dup(word);
	}

	fclose(fp);
}


void spellcheck_update()
{
	int i;
	int limit=1;
	check_queue();
	if(!current)
		return;
	for(i=0;i<limit;i++)
	{
		switch(current->what)
		{
			case SP_STATE_AREA_OBJ:
				limit=OBJS_PER_CHECK;
				spellcheck_obj(current->ch,current->where);
				break;
			case SP_STATE_AREA_MOB:
				limit=MOBS_PER_CHECK;
				spellcheck_mob(current->ch,current->where);
				break;
			case SP_STATE_AREA_ROOM:
				limit=ROOMS_PER_CHECK;
				spellcheck_room(current->ch,current->where);
		}

		current->where++;

		if(current->where > current->a->uvnum)
		{
			if(current->what==SP_STATE_AREA_FINAL)
			{
				send_to_char("Spellcheck complete.\n\r",current->ch);
				current->ch=NULL;
				current->a=NULL;
				free(current);
				current=NULL;
				break;
			}
			else
			{
				current->what++;
				current->where=current->a->lvnum;
			}
		}

	}
}

void spellcheck_extradesc(CHAR_DATA *ch, EXTRA_DESCR_DATA *ed, const char *where)
{
	char buf[MAX_STRING_LENGTH];
	while(ed)
	{
		sprintf(buf,"extra desc on %s, keyword: %s",where,ed->keyword);
		spellcheck_str(ch,ed->description,buf);
		ed=ed->next;
	}
}

void spellcheck_obj(CHAR_DATA *ch, sh_int vnum)
{
	OBJ_INDEX_DATA *iObj;
	char buf[MAX_STRING_LENGTH];
	iObj=get_obj_index(vnum);
	if(!iObj)
		return;

	sprintf(buf,"keyword of obj #%d",vnum);	
	spellcheck_str(ch,iObj->name,buf);
	sprintf(buf,"short of obj #%d",vnum);	
	spellcheck_str(ch,iObj->short_descr,buf);
	sprintf(buf,"description of obj #%d",vnum);	
	spellcheck_str(ch,iObj->description,buf);
	sprintf(buf,"location_string of obj #%d",vnum);	
	spellcheck_str(ch,iObj->location_string,buf);
	sprintf(buf,"obj #%d",vnum);	
	if(iObj->extra_descr)
		spellcheck_extradesc(ch,iObj->extra_descr,buf);
}

void spellcheck_mob(CHAR_DATA *ch, sh_int vnum)
{
	MOB_INDEX_DATA *iMob;
	char buf[MAX_STRING_LENGTH];
	iMob=get_mob_index(vnum);
	if(!iMob)
		return;

	sprintf(buf,"name of mob #%d",vnum);	
	spellcheck_str(ch,iMob->player_name,buf);
	sprintf(buf,"short of mob #%d",vnum);	
	spellcheck_str(ch,iMob->short_descr,buf);
	sprintf(buf,"long of mob #%d",vnum);	
	spellcheck_str(ch,iMob->long_descr,buf);
	sprintf(buf,"description of mob #%d",vnum);	
	spellcheck_str(ch,iMob->description,buf);
}

void spellcheck_room(CHAR_DATA *ch, sh_int vnum)
{
	ROOM_INDEX_DATA *iRoom;
	char buf[MAX_STRING_LENGTH];
	sh_int i=0;
	iRoom=get_room_index(vnum);
	if(!iRoom)
		return;
	sprintf(buf,"title of room #%d",vnum);
	spellcheck_str(ch,iRoom->name,buf);
	sprintf(buf,"desc of room #%d",vnum);
	spellcheck_str(ch,iRoom->description,buf);
	for(i=0;i<6;i++)
	{
		if(iRoom->exit[i])
		{
			sprintf(buf,"keyword of %s exit of room #%d",dir_name[i],vnum);
			spellcheck_str(ch,iRoom->exit[i]->keyword,buf);
			sprintf(buf,"description of %s exit of room #%d",dir_name[i],vnum);
			spellcheck_str(ch,iRoom->exit[i]->description,buf);
		}
	}
}

char * parse(char *argument, char *arg_first)
{
	while(!isalpha(*argument))
	{
		if(*argument=='`' && *(argument+1)!='\0')
		{
			argument+=2;
			continue;
		}
		if(*argument=='\0')
		{
			*arg_first='\0';
			return argument;
		}
		argument++;
	}
    while ( *argument != '\0' )
    {
	if(*argument=='`' && *(argument+1)!='\0')
	{
		argument+=2;
		continue;
	}
    	if(*argument=='\'')
    	{
    		if(*(argument+1)=='s' && ((*(argument+1)!='\0') && !isalpha(*(argument+2))))
    		{
    			argument+=2;/*Take out possessive, keep contractions*/
				if ( !isalpha(*argument))
    		    {
    		        argument++;
  	        	  break;
  	      		}
  	      	}
    	}
        else if ( !isalpha(*argument))
        {
            argument++;
            break;
        }
        *arg_first = LOWER(*argument);
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    return argument;
}

void check_queue()
{
	Q_DATA *q=firstqueue;
	if(q==NULL || current)
		return;
	current=malloc(sizeof current);
	current->ch=q->ch;
	current->what=SP_STATE_AREA_FIRST;
	current->a=q->a;
	current->where=current->a->lvnum;
	firstqueue=firstqueue->next;
	q->next=NULL;
	q->ch=NULL;
	q->a=NULL;
	free(q);
}

void fwrite_dictionary()
{
	FILE *fp;
	STR_LIST *trans;
	long current;

	trans=NULL;

	fp=fopen(DICTIONARY_FILE,"w");
	if(!fp)
	{
		log_string("Error writing to dictionary, failed.");
		return;
	}
	
	current=0;
	for(trans=added_words;trans;trans=trans->next)
		current++;
	
	fprintf(fp,"%ld\n",NUM_DICTIONARY_WORDS+current);
	
	current=0;
	trans=added_words;
	while(trans || current< NUM_DICTIONARY_WORDS)
	{
		if(trans && current<NUM_DICTIONARY_WORDS)
		{
			int dif;
			if(!(dif=strcmp(trans->str,dictionary_words[current])))
			{
				log_string("Added same word in dictionary..somehow.");
				fprintf(fp,"%s\n",dictionary_words[current]);
				current++;
				trans=trans->next;
			}
			else if(dif>0) /*list > array*/
			{
				fprintf(fp,"%s\n",dictionary_words[current]);
				current++;
			}
			else
			{
				fprintf(fp,"%s\n",trans->str);
				trans=trans->next;
			}
		}
		else if(trans)
		{
			fprintf(fp,"%s\n",trans->str);
			trans=trans->next;
		}
		else
		{
			fprintf(fp,"%s\n",dictionary_words[current]);
			current++;
		}
	}
	fclose(fp);
}
