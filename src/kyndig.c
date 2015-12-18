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

/* procedures and constants needed for buffering */
BUFFER *buf_free;


/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
    16,32,64,128,256,1024,2048,4096,8192,16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
    int i;

    for (i = 0; i < MAX_BUF_LIST; i++)
        if (buf_size[i] >= val)
        {
            return buf_size[i];
        }

    return -1;
}

/* used to alloc a new buf */
BUFFER *new_buf()
{
    BUFFER *buffer;

    if (buf_free == NULL) 
        buffer = alloc_perm(sizeof(*buffer));
    else
    {
        buffer = buf_free;
        buf_free = buf_free->next;
    }

    buffer->next        = NULL;
    buffer->state       = BUFFER_SAFE;
    buffer->size        = get_size(BASE_BUF);

    buffer->string      = alloc_mem(buffer->size);
    buffer->string[0]   = '\0';
    VALIDATE(buffer);

    return buffer;
}

void free_buf(BUFFER *buffer)
{
    if (!IS_VALID(buffer))
        return;

    free_mem(buffer->string,buffer->size);
    buffer->string = NULL;
    buffer->size   = 0;
    buffer->state  = BUFFER_FREED;
    INVALIDATE(buffer);

    buffer->next  = buf_free;
    buf_free      = buffer;
}

bool add_buf(BUFFER *buffer, char *string)
{
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
        return FALSE;

    len = strlen(buffer->string) + strlen(string) + 1;

    while (len >= buffer->size) /* increase the buffer size */
    {
        buffer->size    = get_size(buffer->size + 1);
        {
            if (buffer->size == -1) /* overflow */
            {
                buffer->size = oldsize;
                buffer->state = BUFFER_OVERFLOW;
                bug("buffer overflow past size %d",buffer->size);
                return FALSE;
            }
        }
    }

    if (buffer->size != oldsize)
    {
        buffer->string  = alloc_mem(buffer->size);

        strcpy(buffer->string,oldstr);
        free_mem(oldstr,oldsize);
    }

    strcat(buffer->string,string);
    return TRUE;
}

char *buf_string(BUFFER *buffer)
{
    return buffer->string;
}

