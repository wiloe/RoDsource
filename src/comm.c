#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h> /*Copyover, SEDIT, printf_to_char  by Kyndig*/

#include "merc.h"
#include <math.h>

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_outfit	);
DECLARE_DO_FUN(do_afk		);

extern CHAR_DATA *should_ignore(CHAR_DATA *,CHAR_DATA *);
extern void release_ignores(CHAR_DATA *);

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	defined(linux)
/*
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
*/
int	close		args( ( int fd ) );
//int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
//int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
//int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
/*spellsong comment*/
/*int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );*/
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
/*spellsong replacement for above*/
#if !defined(__SVR4)
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );

#if defined(SYSV)
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;	/* Free list for descriptors	*/
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
bool		    chaos;		/* Game in CHAOS!		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
bool 		    rolled=FALSE;
int 		    stat1[5],stat2[5],stat3[5],stat4[5],stat5[5];
bool fCopyOver;
int port;
int control;

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d, bool color ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length, bool color) );
/* Needs to be global because of do_copyover */
int port, control;
#endif


/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d, bool color ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
char 	*do_color		args( (char *plaintext, bool color) );
char 	*doparseprompt		args( (CHAR_DATA *ch) );


int figure_difference(int points)
{
    if (points >= 28)
    	return ((int)pow((double)points,(double)1.2));
    if (points <28) 
        return (26+points);
    return(0);
}

int main( int argc, char **argv )
{
    struct timeval now_time;

   fCopyOver = FALSE;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
		perror( NULL_FILE );
		log_string( "NULL_FILE BUG: in main in comm.c" );
		exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 7000;
     fCopyOver = FALSE;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    log_string( "PORT ERROR: in main in comm.c" );
		exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    log_string( "INVALID PORT: in main in comm.c" );
		exit( 1 );
	}
        /* Are we recovering from a copyover? */
        if (argv[2] && argv[2][0])
        {
                fCopyOver = TRUE;
                control = atoi(argv[3]);
        }
        else
                fCopyOver = FALSE;
    }

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "EmberMud is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
   if (!fCopyOver)
     control = init_socket( port );
           
    boot_db();
    sprintf( log_buf, "EmberMUD is ready to rock on port %d.", port );
    log_string( log_buf );
    game_loop_unix( control );
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}

#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
		perror( "Init_socket: socket" );
		log_string( "SOCKET ERROR: in Init_socket in comm.c" );
		exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
		perror( "Init_socket: SO_REUSEADDR" );
		log_string( "SO_REUSEADDR: in Init_socket in comm.c" );
		close(fd);
		exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
		struct	linger	ld;

		ld.l_onoff  = 1;
		ld.l_linger = 1000;

		if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
		(char *) &ld, sizeof(ld) ) < 0 )
		{
			perror( "Init_socket: SO_DONTLINGER" );
			log_string( "SO_DONTLINGER: in Init_socket in comm.c" );
			close(fd);
			exit( 1 );
		}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
		perror("Init socket: bind" );
		log_string( "SOCKET BIND: in Init_socket in comm.c" );
		close(fd);
		exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
		perror("Init socket: listen");
		log_string( "SOCKET LISTEN: in Init_socket in comm.c" );
		close(fd);
		exit(1);
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;
    bool color;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.connected	= CON_ANSI;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= descriptor_list;
    dcon.showstr_head	= NULL;
    dcon.showstr_point	= NULL;
    dcon.pEdit		= NULL;			/* OLC */
    dcon.pString	= NULL;			/* OLC */
    dcon.editor		= 0;			/* OLC */
    descriptor_list	= &dcon;

    /* Main loop */

close_socket( d );
	

    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/* WHAT THE FUCK! THIS WAS HERE SANS THE COMMENT INDEICATORS - SS
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d, color ) )
		{
		    /*if ( d->character != NULL)*/ /*spellsong replace with below*/
			if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }
	    read_from_buffer( d, color );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	        /* OLC */
	        if ( d->showstr_point )
	            show_string( d, d->incomm );
	        else
	        if ( d->pString )
	            string_add( d->character, d->incomm );
	        else
	            switch ( d->connected )
	            {
	                case CON_PLAYING:
						if ( !run_olc_editor( d ) )
						substitute_alias( d, d->incomm );
						break;
	                default:
						nanny( d, d->incomm );
						break;
	            }

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon, color ) )
		{
		    /*if ( dcon.character != NULL && d->character->level > 1)*/
			/*above replaced with below - spellsong*/
			if ( dcon.character != NULL && d->connected == CON_PLAYING)
				save_char_obj( d->character );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }
*/
    return;
}
#endif



#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;
    bool color;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    log_string( "Game_loop_UNIX: select: poll" );
		exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    new_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		/*if ( d->character && d->character->level > 1)*/
		/*above replaced with below - spellsong*/
		if ( d->character && d->connected == CON_PLAYING)
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
/* THIS WHOLE PORTION WAS A FUCKING MESS. IF IT PROVES
 * TO BE BUGGY IN THE LEAST COPY ORIGINAL BACK IN FROM OLD BACK UP
 * SPELLSONE
 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
			if ( d->character != NULL ) 
			{
				d->character->timer = 0;
				if ( IS_SET(d->character->act, PLR_COLOR))
					color=TRUE;
				else
					color=FALSE;

			} 
			else
				color=FALSE;
			
			if ( !read_from_descriptor( d, color ) )
			{
				FD_CLR( d->descriptor, &out_set );
				/*if ( d->character != NULL && d->character->level > 1)*/
				/*above replaced with below - spellsong*/
				if ( d->character != NULL && d->connected == CON_PLAYING)
					save_char_obj( d->character );
				d->outtop	= 0;
				close_socket( d );
				continue;
			}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }
	    
	    read_from_buffer( d, FALSE );
	    if ( d->incomm[0] != '\0' )
	    {
			d->fcommand	= TRUE;
			stop_idling( d->character );

			/* OLC */
			if ( d->showstr_point )
				show_string( d, d->incomm );
			else 
			{
				if ( d->pString )
					string_add( d->character, d->incomm );
				else
				{
					switch ( d->connected )
					{
						case CON_PLAYING:
							if ( !run_olc_editor( d ) )
								substitute_alias( d, d->incomm );
							break;
						default:
							nanny( d, d->incomm );
							break;
					}
				}
			}
			d->incomm[0]	= '\0';
	    }
	} /*end for*/

/*END SPELLSONG UNFUCK*/


	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
			if ( !process_output( d, TRUE ) )
			{
				/*if ( d->character != NULL && d->character->level > 1)*/
				/*spellsong switch above with below*/
				if ( d->character != NULL && d->connected == CON_PLAYING)
					save_char_obj( d->character );
				d->outtop	= 0;
				close_socket( d );
			}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
			struct timeval stall_time;

			stall_time.tv_usec = usecDelta;
			stall_time.tv_sec  = secDelta;
			if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
			{
				perror( "Game_loop: select: stall" );
				exit( 1 );
			}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void new_descriptor( int control )
{
    static DESCRIPTOR_DATA d_zero;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;
    bool color;

	buf[0] = '\0'; /*spellsong add*/

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
		perror( "New_descriptor: accept" );
		return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
		perror( "New_descriptor: fcntl: FNDELAY" );
		return;
    }

    /*
     * Cons a new descriptor.||CONVERT TO 2.4 later Spellsong
     */
    if ( descriptor_free == NULL )
    {
		dnew		= alloc_perm( sizeof(*dnew) );
    }
    else
    {
		dnew		= descriptor_free;
		descriptor_free	= descriptor_free->next;
    }

    *dnew		= d_zero;
    dnew->descriptor	= desc;
    dnew->connected	= CON_ANSI;
    dnew->showstr_head	= NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->pEdit		= NULL;			/* OLC */
    dnew->pString	= NULL;			/* OLC */
    dnew->editor	= 0;			/* OLC */
    dnew->outbuf	= alloc_mem( dnew->outsize );

	/* Akira's test */
	dnew->character = NULL;

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	/*boy someone was pissed at comeone to hardcode a permban - Spellsong*/
	/*if (from && (!str_cmp(from->h_name,"ursula.uoregon.edu")
		 ||  !str_cmp(from->h_name,"monet.ucdavis.edu")))
	    dnew->host = str_dup("white.nextwork.rose-hulman.edu");
	else*/
	    dnew->host = str_dup( from ? from->h_name : buf );
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
	 *
	 * SPELLSONG - CONVERT to 2.4 later
	 */
    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
		if ( !str_suffix( pban->name, dnew->host ) )
		{
			color=FALSE;
			write_to_descriptor( desc,
			"Your site has been banned from this Mud.\n\r", 0 , color);
			close( desc );
			free_string( dnew->host );
	/*	    free_string( dnew->outbuf ); 
			free_string( dnew->inbuf );  */
			free_mem( dnew->outbuf, dnew->outsize );
			dnew->next		= descriptor_free;
			descriptor_free	= dnew;
			return;
		}
    }

    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    write_to_buffer( dnew, "\n\rDo you want ANSI color? [Y/n] ", 0 ); 

    return;
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
		DESCRIPTOR_DATA *d;

		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if ( d->snoop_by == dclose )
			d->snoop_by = NULL;
		}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
		sprintf( log_buf, "Closing link to %s.", ch->name );
		log_string( log_buf );

		/*spellsong pet crash fix*/
		if ( ch->pet && ch->pet->in_room == NULL )
		{
			char_to_room( ch->pet, get_room_index(ROOM_VNUM_LIMBO) );
			extract_char( ch->pet, TRUE );
		}

		/*spellsong mount crash fix*/
		if ( ch->mount && ch->mount->in_room == NULL )
		{
			char_to_room( ch->mount, get_room_index(ROOM_VNUM_LIMBO) );
			extract_char( ch->mount, TRUE );
		}
		
		/* cut down on wiznet spam when rebooting */
		if ( dclose->connected == CON_PLAYING && !merc_down ) /*added merc down check - spellsong*/
		{
			act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
				wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
			ch->desc = NULL;
                        release_ignores(ch);
		}
		else
		{
			/*free_char( dclose->character );*/
			/*spellsong replaced above with below*/
			free_char( dclose->original ? dclose->original : dclose->character );

		}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_string( dclose->host ); /*convert this to 2.4 - spellsong*/
    /* RT socket leak fix -- I hope */
    free_mem(dclose->outbuf,dclose->outsize);
/*    free_string(dclose->showstr_head); */
    dclose->next	= descriptor_free;
    descriptor_free	= dclose;
#if defined(MSDOS) || defined(macintosh)
    log_string( "Socket Closed - exit" )
	exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d, bool color )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
		sprintf( log_buf, "%s input overflow!", d->host );
		log_string( log_buf );
		write_to_descriptor( d->descriptor,
			"\n\r*** PUT A LID ON IT!!! ***\n\r", 0 , color);
		return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
		int c;
		c = getc( stdin );
		if ( c == '\0' || c == EOF )
			break;
		putc( c, stdout );
		if ( c == '\r' )
			putc( '\n', stdout );
		d->inbuf[iStart++] = c;
		if ( iStart > sizeof(d->inbuf) - 10 )
			break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d, bool color )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 , color);

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
/*
    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 25 )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
                wiznet("Spam spam spam $N spam spam spam spam spam!",
                       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
                if (d->incomm[0] == '!')
                    wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
                        get_trust(d->character));
                else
                    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
                        get_trust(d->character));
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 , color);
		strcpy( d->incomm, "quit" );
	    }
	}
    }
*/

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;
    bool color=TRUE;

    /*
     * Bust a prompt.
     */
    if ( (d->character != NULL) && IS_SET(d->character->act,PLR_COLOR))
    {
        color=TRUE;
    }
    else
    {
        color=FALSE;
    }

    if ( !merc_down )
    {
	if ( d->showstr_point )
	    write_to_buffer( d, "`W[Hit Return to continue]\n\r`w", 0 );
	else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
	    write_to_buffer( d, "> ", 2 );
        else if ( fPrompt && d->connected == CON_PLAYING && !merc_down )/*added merc down check - spellsong*/
        {
			CHAR_DATA *ch;

			ch = d->character;
            if (IS_SET(ch->act,PLR_COLOR))
				color=TRUE;
			else
				color=FALSE;
        /* battle prompt 
          if ((victim = ch->fighting) != NULL)
          {
            int percent;
            char wound[100];
	    char buf[MAX_STRING_LENGTH];
 
            if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
            else
                percent = -1;
 
            if (percent >= 100)
                sprintf(wound,"is in excellent condition.");
            else if (percent >= 90)
                sprintf(wound,"has a few scratches.");
            else if (percent >= 75)
                sprintf(wound,"has some small wounds and bruises.");
            else if (percent >= 50)
                sprintf(wound,"has quite a few wounds.");
            else if (percent >= 30)
                sprintf(wound,"has some big nasty wounds and scratches.");
            else if (percent >= 15)
                sprintf(wound,"looks pretty hurt.");
            else if (percent >= 0)
                sprintf(wound,"is in awful condition.");
            else
                sprintf(wound,"is bleeding to death.");
 
            sprintf(buf,"%s %s \n\r", 
	            IS_NPC(victim) ? victim->short_descr : victim->name,wound);
	    buf[0] = UPPER(buf[0]);
            write_to_buffer( d, buf, 0);
        } */


		ch = d->original ? d->original : d->character;
		if (!IS_SET(ch->comm, COMM_COMPACT) )
			write_to_buffer( d, "\n\r", 2 );

		if ( IS_SET(ch->comm, COMM_PROMPT) )
		{
			char buf[200];
			
			buf[0] = '\0'; /*spellsong*/

			ch = d->character;
			if (!IS_NPC(ch)) 
				sprintf( buf, "%s", doparseprompt(ch));
			else 
				sprintf( buf, "<H%d/%d M%d/%d V%d/%d>", ch->hit, ch->max_hit,
    				ch->mana, ch->max_mana, ch->move, ch->max_move);
			
			write_to_buffer( d, buf, 0 );
		}

		if (IS_SET(ch->comm,COMM_TELNET_GA))
			write_to_buffer(d,go_ahead_str,0);
      }   //end brackets added to avoid warning - Lotex - 2/27/00
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop , color) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}



/*
 * Append onto an output buffer.
 */
 void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
		char *outbuf;

		/*old stuff*/
        /*if (d->outsize > 32000)
		{
			bug("Buffer overflow. Closing.\n\r",0);
			close_socket(d);
			return;
 		}*/
		/*spellsong rehack - if ya want to know why...ask*/
		if ( ( 2 * d->outsize) > 63000 )
		{
			bug("Write_to_buffer:  outsize too large:  %d", d->outsize);
			return;
		}

		outbuf      = alloc_mem( 2 * d->outsize );
		strncpy( outbuf, d->outbuf, d->outtop );
		free_mem( d->outbuf, d->outsize );
		d->outbuf   = outbuf;
		d->outsize *= 2;
    }

    /*
     * Copy.
     */
	/*strcpy( d->outbuf + d->outtop, txt);*/
	/*above replace with below - spellsong - bugged recheck*/
/*      strcpy( d->outbuf + d->outtop, txt);*/
	strncpy( d->outbuf + d->outtop, txt, length);
    d->outtop += length;
	d->outbuf[d->outtop]='\0';
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 *
 * SPELLSONG RE-LOOK LATER!!!!
 */
bool write_to_descriptor( int desc, char *txt, int length , bool color)
{
    int iStart;
    int nWrite;
    int nBlock;
/*!!!!   Lines added by ZAK to prepare for color stuff*/
    char blah[MAX_STRING_LENGTH*2];

    strncpy (blah, do_color(txt, color), sizeof(blah)-1);
    strcat (blah, "\0");
      
#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

  /*  if ( length <= 0 )  */
	length = strlen(blah);
    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, sizeof(blah) );
	if ( ( nWrite = write( desc, blah + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 
/*
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
*/


    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 *
 * SPELLSONG PERUSE LATER AND FIX!!!
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass,race,i,notes;
    NOTE_DATA *pnote;
    bool fOld;
    DESCRIPTOR_DATA *d_old;
    extern char *help_greeting;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if ( IS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_HERO(ch)) 
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	    else if (chaos && !IS_HERO(ch))
	    {
		write_to_buffer( d, "The game is in CHAOS!\n\r", 0 );
		close_socket( d );
		return;
	   }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
 	    if (newlock)
	    {
                write_to_buffer( d, "The game is newlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	    sprintf( buf, "NAME POLICY:\n\r"
 "Choosing a name is one of the most important parts of this game...\n\r"
 "Make sure to pick a name appropriate to the character you are going\n\r"
 "to role play, and be sure that it suits a medieval theme.\n\r"
 "If the name you select is not acceptable, you will be asked to choose\n\r"
 "another one. Unacceptable names may be DENIED at any time.\n\r\n\r");
 write_to_buffer( d, buf, 0 );
 sprintf( buf, "ACCEPTABLE:\n\r"
 "1)  Proper names, medieval or modern. i.e  (Takasi, Mitra)\n\r"
 "2)  Created names with a medieval or fictional sound.  (Priatros, Sansoles)\n\r\n\r"
 "UNACCEPTABLE:\n\r"
 "1)  Vulgar or sexually suggestive names.  (No examples needed)\n\r"
 "2)  Well known names from literature or mythology.  (Chronos, Zeus)\n\r"
 "3)  Nouns which are names of animals.  (Horse, Duck)\n\r"
 "4)  Strings of random characters.  (Asdfg, Qwdldkf)\n\r"
 "5)  Titles which imply rank or privelege.   (Lord, Sir,...)\n\r"
 "6)  Scientific phrases, words. (Proof, Logic, Electrics...)\n\r"
 "7)  Object names. (Necklace,Staff,Sword, etc)\n\r"
 "By accepting this name, you are saying that  your name complies with these guidelines.\n\r");
	    write_to_buffer( d, buf, 0 ); 
 sprintf( buf, "Are you sure you want this name? (Y/N)?" );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}

	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}
 
	if ( ch->pcdata->pwd[0] == 0)
	{
	    write_to_buffer( d, "Warning! Null password!\n\r",0 );
	    write_to_buffer( d, "Please report old password with bug.\n\r",0);
	    write_to_buffer( d, 
		"Type 'password null <new password>' to fix.\n\r",0);
	}

	write_to_buffer( d, echo_on_str, 0 );

	if ( check_playing( d, ch->name ) )
	    return;
		    
	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	log_string( log_buf );
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

        if ( ch->desc->ansi ) {
            SET_BIT( ch->act, PLR_COLOR );
        } else {
            REMOVE_BIT( ch->act, PLR_COLOR );
        }

	if ( IS_HERO(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
 	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;

/* RT code for breaking link */
 
    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
            for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name, d_old->original ? 
		  d_old->original->name : d_old->character->name))
		    continue;

		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->name,TRUE))
	    	return;
	    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	  write_to_buffer( d,"\n\rI see you are new to this realm. Give me a secret password so\n\r",0);
	  write_to_buffer( d,"I will know you when you tread this path again.\n\r\n\r" ,0);
	 sprintf( buf, "Give me a password for %s: %s",
	   ch->name, echo_off_str );

	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Y or N? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}
	
	sprintf ( buf, "You pass through the Gates of Chaos!\n\r\n\r"
"You sense someone watching you very closely.\n\r"
"You hear another voice say to you:\n\r"
"\"Greetings traveler. I am an acoloyte of Nyobe, the goddess of fate.\n\r"
"I am here to guide you into this land.\"\n\r\n\r"
"At this point a shimmering light is visible to you. It appears to take upon a\n\r"
"vaguely humanoid shape without any discernable features.\n\r\n\r"
"You hear the voice again:\n\r"
"\"This is your spirit. We now must choose its form.\"\n\r");
write_to_buffer (d, buf, 0);
	write_to_buffer( d, echo_on_str, 0 );
	/* write_to_buffer(d,"The following races exist in this land:\n\r  ",0); */

	sprintf( buf, "The following races exist in this land:\n\r"
"Human         The standard race, potentially good at any task.\n\r"
"Dwarf         Tempermental, short demihumans. Strong and sturdy.\n\r" 
"Elf           Lithe magical woodland creatures. Intelligent and quick.\n\r"
"Haitorin      A honorable race of feline-like people. Strong and fast.\n\r"
"Tervadi       Bird-like demihumans. Can fly. Intelligent and wise.\n\r"
"Lacerti       Lizard race. Gain and lose power based on the Sun. Versatile.\n\r"
"Nymph         All-female race of forestfolk. Quick, smart and magickal.\n\r"
"Spiritfolk    A strange race. Not fully part of the plane of the living.\n\r"    
"(Type help racename for a more detailed explanation. eg HELP HUMAN)\n\r\n\r");

/* Commented out from above
"Shapeshifter  Rare beings able to change form. Feared and hated by society.\n\r\n\r"
*/

write_to_buffer( d, buf, 0);

	/* for ( race = 1; race_table[race].name != NULL; race++ )
	{
	    if (!race_table[race].pc_race)
		break;
	    write_to_buffer(d,race_table[race].name,0);
	    write_to_buffer(d," ",1);
	} */ 

	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d,"What form do you choose? ",0);
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
	one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_help(ch,"race help");
	    else
		do_help(ch,argument);
            write_to_buffer(d,
		"What form do you choose? ",0);
	    break;
  	}

	race = race_lookup(argument);

	if (race == 0 || !race_table[race].pc_race || race == 9) // Eris shapeshifter hack
	{
	    write_to_buffer(d,"That is not a valid race.\n\r",0);
            write_to_buffer(d,"The following races exist in this land:\n\r  ",0);
	    for ( race = 1; race_table[race].name != NULL && race !=9; race++ ) // Eris shapeshifter hack 
            {
            	if (!race_table[race].pc_race)
                    break;
            	write_to_buffer(d,race_table[race].name,0);
            	write_to_buffer(d," ",1);
            }
            write_to_buffer(d,"\n\r",0);
            write_to_buffer(d,
		"What form do you choose? (help for more information) ",0);
	    break;
	}

        ch->race = race;
	/* initialize stats */
	for (i = 0; i < MAX_STATS; i++)
	    ch->perm_stat[i] = pc_race_table[race].stats[i];
	ch->affected_by = ch->affected_by|race_table[race].aff;
	ch->imm_flags	= ch->imm_flags|race_table[race].imm;
	ch->res_flags	= ch->res_flags|race_table[race].res;
	ch->vuln_flags	= ch->vuln_flags|race_table[race].vuln;
	ch->form	= race_table[race].form;
	ch->parts	= race_table[race].parts;

	/* add skills */
	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[race].skills[i] == NULL)
	 	break;
	    group_add(ch,pc_race_table[race].skills[i],FALSE);
	}
	
	/*
	 * Specific races get racial skills set to 100% - Eris 8 April 2000
	 */
	if (ch->race == 2)
	{
 	  ch->pcdata->learned[gsn_phase]=100;
	}
	else if (ch->race == 3 || ch->race == 5)
	{
	  ch->pcdata->learned[gsn_sneak]=100;
	  ch->pcdata->learned[gsn_hide]=100;
	} 
	else if (ch->race == 6)
	{
	  ch->pcdata->learned[skill_lookup("vision")]=100;
	}
	else if (ch->race == 8)
	{
  	  ch->pcdata->learned[gsn_captivate]=100;
	}

	/* add cost */
	ch->pcdata->points = pc_race_table[race].points;
	ch->size = pc_race_table[race].size;
        write_to_buffer( d, "What is your sex (M/F)? ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;

/* Nymph sex check added by Eris 28 Feb 2000 */

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
        case 'm': case 'M': if (ch->race == 8) 
                            {
                              ch->sex = SEX_FEMALE;
                              ch->pcdata->true_sex = SEX_FEMALE;
                              write_to_buffer( d, "\n\rAll Nymphs are Female. Setting sex to Female!\n\r ", 0);
                            } 
			    else
			    {
			      ch->sex = SEX_MALE;    
			      ch->pcdata->true_sex = SEX_MALE;
			    }			  
  			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}
	write_to_buffer(d, "Building character... Press Return to continue", 0);
	d->connected = CON_GET_STATS;
	break;
	
	/*
	 * CON_GET_STATS Hacked to set all stats of race to be the same always.
	 * Eris 25 Jan 2000
         * Further modified to ditch stat display altogether.
         * Eris 20 March 2000	
	 */

 case CON_GET_STATS:
	 {

			ch->perm_stat[0]=14+pc_race_table[ch->race].stats[0];
			ch->perm_stat[1]=14+pc_race_table[ch->race].stats[1];
			ch->perm_stat[2]=14+pc_race_table[ch->race].stats[2];
			ch->perm_stat[3]=14+pc_race_table[ch->race].stats[3];
			ch->perm_stat[4]=14+pc_race_table[ch->race].stats[4];
			
			/* 
			 * Shapeshifter specific class check
			 * Eris 26 March 2000
			 */

			if ( ch->race==9)
			  { 
/*	 write_to_buffer(d, "\n\rBuilding Shapeshifter character... Press Return to continue\n\r",0);
			    d->connected = CON_GET_NEW_CLASS;
			    break;
*/
			  }
			else
			  {
			   sprintf(buf,
"\n\rYou feel a blast of air and your body suddenly seems more solid.\n\r\n\r"
"The soft voice returns: \n\r\n\r"
"\"In this land every creature is destined to perform certain services in order to survive.\n\r"
"There are a variety of services avaiable. Which one feels most natural to you?\"\n\r\n\r"
"The following classes exist in the land:\n\r\n\r"
"Sorcerer      Mage. Access to all kinds of different spells.\n\r"
"Mystic        Blessed by the Gods. Able to heal. Psychic powers.\n\r"
"Thief         Masters of stealth and cunning. Able to steal. Good at combat.\n\r"
"Warrior       A master of arms and tactics. Strong and efficient killers.\n\r"
"Templar       Holy crusaders. Warriors with a healing touch.\n\r\n\r"
"(type help classname for more information. eg HELP THIEF)\n\r"
"Choose your occupation:");
			    write_to_buffer( d, buf, 0);
			    
                            /* strcpy( buf, "Select a class [" );
			    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
			      {
				if ( iClass > 0 )
				strcat( buf, " " );
				strcat( buf, class_table[iClass].name );
			      }
			    strcat( buf, "]: " );
			    write_to_buffer( d, buf, 0 ); */

			    d->connected = CON_GET_NEW_CLASS;
			    break;
			  }
	}
        
	 /* OLD VERSION - Eris 20 March 2000
    case CON_GET_STATS:
	if (rolled==TRUE) switch(argument[0]) {
		case '0' : 
		case '1' :
		case '2' :
		case '3' :
		case '4' :
			ch->perm_stat[0]=stat1[atoi(argument)];
			ch->perm_stat[1]=stat2[atoi(argument)];
			ch->perm_stat[2]=stat3[atoi(argument)];
			ch->perm_stat[3]=stat4[atoi(argument)];
			ch->perm_stat[4]=stat5[atoi(argument)];

			strcpy( buf, "Select a class [" );
			for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
			{
	    		if ( iClass > 0 )
				strcat( buf, " " );
			    strcat( buf, class_table[iClass].name );
			}
			strcat( buf, "]: " );
			write_to_buffer( d, buf, 0 );
			d->connected = CON_GET_NEW_CLASS;
			break;
		default:
		    	write_to_buffer( d, "                       0    1    2    3    4\n\r", 0);
			write_to_buffer( d, "     Strength     :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat1[x]=14+pc_race_table[ch->race].stats[0];
				sprintf(buf, "   %2d", stat1[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Intelligence :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat2[x]=14+pc_race_table[ch->race].stats[1];
				sprintf(buf, "   %2d", stat2[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Wisdom       :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat3[x]=14+pc_race_table[ch->race].stats[2];
				sprintf(buf, "   %2d", stat3[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Dexterity    :", 0);
			for (x = 0 ; x < 5 ; x++) {
			  stat4[x]=14+pc_race_table[ch->race].stats[3];
				sprintf(buf, "   %2d", stat4[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Constitution :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat5[x]=14+pc_race_table[ch->race].stats[4];
				sprintf(buf, "   %2d", stat5[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r\n\r     Press enter to roll again, else enter number of column: ", 0);
			rolled=TRUE;
			break;
	} else {
		    	write_to_buffer( d, "                       0    1    2    3    4\n\r", 0);
			write_to_buffer( d, "     Strength     :",0);
			for (x = 0 ; x < 5 ; x++) {
				stat1[x]=14+pc_race_table[ch->race].stats[0];
				sprintf(buf, "   %2d", stat1[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Intelligence :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat2[x]=14+pc_race_table[ch->race].stats[1];
				sprintf(buf, "   %2d", stat2[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Wisdom       :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat3[x]=14+pc_race_table[ch->race].stats[2];
				sprintf(buf, "   %2d", stat3[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Dexterity    :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat4[x]=14+pc_race_table[ch->race].stats[3];
				sprintf(buf, "   %2d", stat4[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r     Constitution :", 0);
			for (x = 0 ; x < 5 ; x++) {
				stat5[x]=14+pc_race_table[ch->race].stats[4];
				sprintf(buf, "   %2d", stat5[x]);
				write_to_buffer( d, buf, 0);
				}
			write_to_buffer( d, "\n\r\n\r     Press enter to roll again, else enter number of column: ", 0);
			rolled=TRUE;
	}
	break;
	 */
	 /* END OLD VERSION OF CON_GET_STATS - Eris 20 March 2000 */

    case CON_GET_NEW_CLASS:
        
      /*
       * Shapeshifter class set to 7 (shapeshifter) 
       * Eris 26 March 2000
       */

if (!strcmp(arg,"help"))
        {
            argument = one_argument(argument,arg);
            if (argument[0] == '\0')
                do_help(ch,"class help");
            else
                do_help(ch,argument);
            write_to_buffer(d,
                "What class do you choose? ",0);
            break;
        }

        if ( ch->race==9)
	  { 
iClass = class_lookup("shapeshifter");
ch->class = iClass;
write_to_buffer(d, "\n\rSetting up shapeshifter class. Please press Return.\n\r", 0);
	  }
	else
	  {
	    iClass = class_lookup(argument); 

	   /*  if ( iClass == -1 || iClass == 6 ) Eris - until alchemist/ shapeshifter is in */
	      if (iClass == -1 || iClass == 4 || iClass == 6) // i.e. nothing, or alch, or shapeshifter 
	      {
		write_to_buffer( d,
		"That's not a class.\n\rWhat IS your class? ", 0 );
		return;
	      }

	    ch->class = iClass;
	  }

	/* 
	 * Stuff to change stats based on characters class
	 * Eris 26 March 2000
	 * Like most of this stuff, completely specific to Realms of Discordia
	 * I should really have modified const.c and merc.h and added another
	 * table for these modifications but this is simpler to test.
	 * If they were set more than once, then I'd bother.
	 */
	switch (ch->class)
	  {
	    /* Sorcerer */
	  case 0 : 
	    ch->perm_stat[0]+=-2;
	    ch->perm_stat[1]+=+2;
	    break;
	    /* Mystic */
	  case 1 : 	    
	    ch->perm_stat[0]+=-1;
	    ch->perm_stat[2]+=+2;
	    break;
	    /* Thief */
	  case 2 : 
	    ch->perm_stat[3]+=+2;
	    break;
	    /* Warrior */
	  case 3 : 
	    ch->perm_stat[0]+=+2;
	    ch->perm_stat[1]+=-2;
	    ch->perm_stat[2]+=-2;
	    ch->perm_stat[4]+=+1;
	    break;	    
	    /* Alchemist */
	  case 4 : 
	    ch->perm_stat[0]+=-1;
	    ch->perm_stat[1]+=+1;
	    break;	    
	    /* Templar */
	  case 5 : 
	    ch->perm_stat[0]+=+1;
	    ch->perm_stat[4]+=+1;
	    break;	    	    
	  default : break;
	  }
	/* END OF CLASS SPECIFIC STAT MODIFICATIONS - Eris 26 March 2000 */


	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
        wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	write_to_buffer( d, "\n\r", 2 );
	/*if (ch->race == 1) write_to_buffer( d, "Your class is evil by nature.\n\r",0);
	else {*/
        write_to_buffer( d, "Your mind fills with knowledge.\n\r",0);
	write_to_buffer( d, "You feel as if you know things that you didn't know before.\n\r\n\r",0);
	write_to_buffer( d, "You may be good, neutral, or evil.\n\r",0);
	write_to_buffer( d, "Which alignment (G/N/E)? ",0);
	/*}*/
	d->connected = CON_GET_ALIGNMENT;
	break;

case CON_GET_ALIGNMENT:
	/*The following making first race automatically evil -spellsong*/
	/*if (ch->race == 1) ch->alignment = -750;
	else switch( argument[0])*/
	switch( argument[0]) /*BUG FIX HERE - SPELLSONG*/
	{
	    case 'g' : case 'G' : ch->alignment = 750;  break;
	    case 'n' : case 'N' : ch->alignment = 0;	break;
	    case 'e' : case 'E' : ch->alignment = -750; break;
	    default:
		write_to_buffer(d,"That's not a valid alignment.\n\r",0);
		write_to_buffer(d,"Which alignment (G/N/E)? ",0);
		return;
	}

	write_to_buffer(d,"\n\r",0);
        group_add(ch,"rom basics",FALSE);
        // group_add(ch,class_table[ch->class].base_group,FALSE);
        group_add(ch,class_table[ch->class].base_group,TRUE);//spellsong
        ch->pcdata->learned[gsn_recall] = 50;

	/*
	 * More junk added by Eris - 26 March 2000
	 */
	
	sprintf(buf,
"A different voice whistles. You feel as if you are being poked and\n\r" 
"prodded, yet there is nothing there.\n\r\n\r" 
"You hear the voice speak:\n\r\n\r"
"\"Well, this one is about ready!\"\n\r");
	write_to_buffer(d, buf, 0);
write_to_buffer(d, "Press Return when you are ready to choose your starting skills and spells.\n\r\n\r"
,0);	
	/* write_to_buffer(d, "Customize this character?<y/n>", 0); */

	/* write_to_buffer(d,"Do you wish to customize this character?\n\r",0);
	write_to_buffer(d,"Customization takes time, but allows a wider range of skills and abilities.\n\r",0);
	write_to_buffer(d,"Customize (Y/N)? ",0); */
	d->connected = CON_DEFAULT_CHOICE;
	break;

case CON_DEFAULT_CHOICE:
	write_to_buffer(d,"\n\r",2);
        /* switch ( argument[0] )
        {
        case 'y': case 'Y': */
	    group_add(ch,class_table[ch->class].default_group,TRUE); // Eris - forces them to have defaults
	    ch->gen_data = alloc_perm(sizeof(*ch->gen_data) );
	    ch->gen_data->points_chosen = ch->pcdata->points;
	    do_help(ch,"group header");
	    list_group_costs(ch);
	    write_to_buffer(d,"You already have the following skills:\n\r",0);
	    do_skills(ch,"");
	    do_help(ch,"menu choice");
	    d->connected = CON_GEN_GROUPS;
	/*    break;
        case 'n': case 'N': 
	    group_add(ch,class_table[ch->class].default_group,TRUE);
            write_to_buffer( d, "\n\r", 2 );
            do_help( ch, "motd" );
            d->connected = CON_READ_MOTD;
            break;
        default:
            write_to_buffer( d, "Please answer (Y/N)? ", 0 );
            return;
        } */
	break;

    case CON_GEN_GROUPS:
	send_to_char("\n\r",ch);
       	if (!str_cmp(argument,"done"))
       	{
	    if (ch->gen_data->points_chosen < MIN_SKILL_POINTS)
	    {
	    send_to_char("You have to take a certain minimum number of creation points. The current minimum is 40.\n\r",ch);
	    send_to_char("These points reflect what you did before choosing a life of adventure. Please pick another skilll:\n\r", ch);
	    return;
	    }	  

	    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	    send_to_char(buf,ch);
	    sprintf(buf,"Experience modifier: %d (Percent of difference from the norm)\n\r",
	            figure_difference(ch->gen_data->points_chosen));
	    /*if (ch->pcdata->points < 40)
		ch->train = (40 - ch->pcdata->points + 1) / 2;*/
	    send_to_char(buf,ch);
            write_to_buffer( d, "\n\r", 2 );
            do_help( ch, "motd" );
            d->connected = CON_READ_MOTD;
            break;
        }

        if (!parse_gen_groups(ch,argument))
        send_to_char(
        "Choices are: list,learned,premise,add,drop,info,help, and done.\n\r"
        ,ch);

        do_help(ch,"menu choice");
        break;

    case CON_READ_IMOTD:
	write_to_buffer(d,"\n\r",2);
        do_help( ch, "motd" );
        d->connected = CON_READ_MOTD;
	break;

    case CON_ANSI:
        if ( argument[0] == '\0' || UPPER( argument[0] ) == 'Y' ) { 
            d->ansi = TRUE; 
            send_to_desc( "\n\r`RANSI color enabled!`w\n\r", d );
            d->connected = CON_GET_NAME;

       	    if ( help_greeting[0] == '.' ) { 
         	send_to_desc( help_greeting + 1, d ); 
       	    } else {
           	send_to_desc( help_greeting, d );
       	    }       
 	    break;
	}

        if ( UPPER( argument[0] ) == 'N' ) {
            d->ansi = FALSE;
            send_to_desc( "\n\rANSI color disabled!\n\r", d );
            d->connected = CON_GET_NAME;

            if ( help_greeting[0] == '.' ) {
                send_to_desc( help_greeting + 1, d );
            } else {
                send_to_desc( help_greeting, d );
            }

            break;
        } else {
            send_to_desc( "Do you want ANSI color? [Y/n] ", d );
            break;
        }
	break;


    case CON_READ_MOTD:
	write_to_buffer( d, 
    "\n\rWelcome to Realms of Discordia.\n\r",
	    0 );
	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;
	reset_char(ch);

	/* train set to 6 from 1 , prac set to 6 from 5 - Eris 16 April 2000 */
	/* note: This only works due to act_move.c cost=3 instead of 1 in do_train */

	if ( ch->level == 0 )
	{

	    ch->level	= 1;
	    ch->exp	= 0;
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->move	= ch->max_move;
	    ch->train	 = 6;
	    ch->practice = 6;
	    ch->at_obj  =NULL;
	    ch->next_at =NULL;


	    sprintf( buf, "the %s",
		title_table [ch->class] [ch->level]
		[ch->sex == SEX_FEMALE ? 1 : 0] );
	    set_title( ch, buf );

	    do_outfit(ch,"");
	    obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);

 	    ch->pcdata->learned[get_weapon_sn(ch)]= 40;

            if ( ch->desc->ansi ) {
		SET_BIT( ch->act, PLR_COLOR );
            } else {
                REMOVE_BIT( ch->act, PLR_COLOR );
            }

	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	    send_to_char("\n\r",ch);
	    save_char_obj( ch );
	    do_help(ch,"NEWBIE INFO");
	    send_to_char("\n\r",ch);
	}
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
        if (ch->mount != NULL)
        {
            char_to_room(ch->mount,ch->in_room);
            act("$n has entered the game.",ch->mount,NULL,NULL,TO_ROOM);
            add_follower(ch->mount, ch);
            do_mount(ch, ch->mount->name);
        }

        wiznet("$N has left real life behind.",ch,NULL,
                WIZ_LOGINS,WIZ_SITES,get_trust(ch));

	do_look( ch, "auto" );

/*	if (ch->gold > 250000 && !IS_IMMORTAL(ch))
	{
	    sprintf(buf,"You are taxed %d gold to pay for the Mayor's bar.\n\r",
		(ch->gold - 250000) / 2);
	    send_to_char(buf,ch);
	    ch->gold -= (ch->gold - 250000) / 2;
	}*/
	
	if (ch->pet != NULL)
	{
	    char_to_room(ch->pet,ch->in_room);
	    act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
	}

	if (IS_SET(ch->act,PLR_AFK))
	do_afk(ch,NULL);

        ch->pcdata->chaos_score = 0;
	/* check notes */
	notes = 0;

	for ( pnote = note_list; pnote != NULL; pnote = pnote->next)
	    if (is_note_to(ch,pnote) && str_cmp(ch->name,pnote->sender)
	    &&  pnote->date_stamp > ch->last_note)
	     	notes++;
	
	if (notes == 1)
	    send_to_char("\n\rYou have one new note waiting.\n\r",ch);

	else if (notes > 1)
	{
	    sprintf(buf,"\n\rYou have %d new notes waiting.\n\r",notes);
	    send_to_char(buf,ch);
	}

	break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 *
 * SPELLSONG CHECK LATER
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.
     */
    if ( is_exact_name( name, "all auto immortal self someone something the you fuck shit" )
       || is_name(name, "fuck") || is_name(name, "shit") || is_name(name, "imm")
       || is_name(name, "pussy") || is_name(name, "cunt") || is_name(name, "dick")
       || is_name(name, "blow") || is_name(name, "suck") || is_name(name, "cock") )
        return FALSE;
	
    if (str_cmp(capitalize(name),"Alander") && (!str_prefix("aland",name)
    || !str_suffix("alander",name)))
	return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_exact_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char( "Reconnecting.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
                wiznet("$N groks the fullness of $S link.",
                    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?", 0 );
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}

/*STOP POINT SPELLSONG CONTINUE DEBUG FROM HERE!!!*/

/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

void send_to_desc( char *txt, DESCRIPTOR_DATA *d ) {
    if ( txt != NULL && d != NULL ) {
        if ( d->ansi ) {
            write_to_descriptor( d->descriptor, txt, strlen(txt), 
                TRUE ); 
        } else {
            write_to_descriptor( d->descriptor, txt, strlen(txt), 
                FALSE );
        }       
    }
        
    return; 
}

/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
	return;
	
#if defined(macintosh)
	send_to_char(txt,ch);
#else
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
#endif
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_string(d->showstr_head);
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
        	    {
            		free_string(d->showstr_head);
            		d->showstr_head = 0;
        	    }
        	    d->showstr_point  = 0;
    		}
	    }
	    return;
	}
    }
    return;
}
	

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type)
{
    /* to be compatible with older code */
    act_new(format,ch,arg1,arg2,type,POS_RESTING);
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MAX_STRING_LENGTH*2];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
		return;

    to = ch->in_room->people; /**/
    
	if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

		if (vch->in_room == NULL)
			return;

		to = vch->in_room->people;
    }
 
    for ( ; to != NULL; to = to->next_in_room ) /**/
    {
        if ( to->desc == NULL || to->position < min_pos )
            continue;
 
        if ( type == TO_CHAR && to != ch )
            continue;

        if ( type == TO_CHAR_FIGHT && to != ch )
            continue;

        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;

	if ( type == TO_ROOM && to == ch )
            continue;

	if ( type == TO_ROOM_FIGHT && to == ch )
            continue;

	if ( type == TO_ROOM_MOVE && to == ch )
            continue;

        if ( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;

        if(should_ignore(to,ch)
         &&(type!=TO_CHAR_FIGHT)
         &&(type!=TO_CHAR_MOVE)
         &&(type!=TO_ROOM_FIGHT)
         &&(type!=TO_ROOM_MOVE)
         &&(type!=TO_NOTVICT)
         &&(type!=TO_VICT))
            continue;
 
        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
		continue;
	    }
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
			{
				switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                        break;
                /* Thx alex for 't' idea */
		case 't': if ( (i = (char *) arg1) ==NULL) i="Buggy";   break;
		case 'T': if ((i = (char *) arg2) ==NULL) i="Buggy";	break;
		case 'n': i = PERS( ch,  to  );                         break;
		case 'N': i = PERS( vch, to  );                         break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;

		case 'p':
		    if (obj1 != NULL)
			{
				i = can_see_obj( to, obj1 )
					? obj1->short_descr /* char can see the target */
					: "something";      /* char is blind           */
			}
		    else
			{
				 bug("act_new code with no obj1", 0);
				 i="Something";
		    }
		    break;

		case 'P':
			if (obj2 != NULL)
			{
				i = can_see_obj( to, obj2 )
					? obj2->short_descr
					: "something";
			}
		    else
			{
				bug("act_new code with no obj2", 0);
				i="Something";
		    }
		    break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
				i = "door";
		    }
		    else
		    {
				one_argument( (char *) arg2, fname );
				i = fname;
		    }
		    break;
		}
	    }

	    ++str;
	    while ( ( *point = *i ) != '\0' )
			++point, ++i;
	}

	*point++ = '`';
	*point++ = 'w';
	*point++ = '\n';
	*point++ = '\r';
	buf[0]   = UPPER(buf[0]);
	    if (to->desc)
	       write_to_buffer( to->desc, buf, point - buf );
	    if (MOBtrigger)
	       mprog_act_trigger( buf, to, ch, obj1, vch );

    }
    MOBtrigger = TRUE;
    return;
}

void act_chan( const char *format, CHAR_DATA *ch, const void *arg1, 
	       const void *arg2, int type, long bit )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
    const char min_pos = POS_RESTING;  //type cast to avoid warnings
 					// Lotex 2/27/00
    char buf[MAX_STRING_LENGTH*2];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
    DESCRIPTOR_DATA *d;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

	if (vch->in_room == NULL)
	    return;

	to = vch->in_room->people;
    }

    if( IS_SET(ch->comm,bit ) )
    {
	send_to_char( "Channel now on.\n\r", ch );
        REMOVE_BIT( ch->comm, bit );
    }
 
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        /* Send this to their original form, not what they are/might be changed into */	
        to = d->original ? d->original : d->character;
        if ( d->connected == CON_PLAYING &&
            !IS_SET(to->comm,bit ) &&
            !IS_SET(to->comm,COMM_QUIET) )
        {
/* old loop */
/*    for ( ; to != NULL; to = to->next_in_room ) */
          if ( to->desc == NULL || to->position < min_pos )
              continue;
 
          if ( type == TO_CHAR && to != ch )
              continue;
          if ( type == TO_VICT && ( to != vch || to == ch ) )
              continue;
          if ( type == TO_WORLD && to == ch )
              continue; 
          if ( type == TO_ROOM && ( to == ch || to->in_room != ch->in_room ) )
 	      continue;
          if ( type == TO_NOTVICT && ( ( to == ch || to == vch ) || to->in_room != ch->in_room ) )
              continue;
          if ( type == TO_WORLD_NOTVICT && (to == ch || to == vch) )
              continue;
 
          point   = buf;
          str     = format;
          while ( *str != '\0' )
          {
            if ( *str != '$' )
            {
                *point++ = *str++;
		continue;
	    }
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                                break;
                /* Thx alex for 't' idea */
		case 't': if ( (i = (char *) arg1) ==NULL) i = "Buggy"; break;
		case 'T': if ( (i = (char *) arg2) ==NULL) i = "Buggy"; break;
		case 'n': i = PERS( ch,  to  );                         break;
		case 'N': i = PERS( vch, to  );                         break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;

		case 'p':
		   if (obj1 != NULL)
                   {
		     i = can_see_obj( to, obj1 )
		         ? obj1->short_descr /* char can see the target */
		         : "something";      /* char is blind           */
	           }
		   else 
                   {
		     bug("act_new code with no obj1", 0);
		     i="Something";
		   }
		   break;

		case 'P':
		   if (obj2 != NULL)
                   {
		      i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
	           }
		   else 
                   {
		     bug("act_new code with no obj2", 0);
		     i="Something";
		   }
		   break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }

	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

	*point++ = '`';
	*point++ = 'w';
	*point++ = '\n';
	*point++ = '\r';
	buf[0]   = UPPER(buf[0]);
	if (to->desc)
	   write_to_buffer( to->desc, buf, point - buf );
	if (MOBtrigger)
	   mprog_act_trigger( buf, to, ch, obj1, vch );
        }
    }
    MOBtrigger = TRUE;
    return;
}



/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

char *do_color(char *plaintext, bool color)
{
	static char color_text[MAX_STRING_LENGTH*2];
	char *ct_point;
	
	bzero(color_text, sizeof(color_text));
	ct_point=color_text;
	while ( *plaintext != '\0' ) {
		if ( *plaintext != '`' ) {
			*ct_point = *plaintext;
			ct_point++;
			plaintext++;
			continue;
		}
		plaintext++;
		if (!color)
		  switch(*plaintext) {
		  	case 'k':
	  		case 'K':
		  	case 'r':
		  	case 'R':
		  	case 'b':
	  		case 'B':
		  	case 'c':
	  		case 'C':
		  	case 'Y':
		  	case 'y':
	  		case 'm':
		  	case 'M':
		  	case 'w':
	  		case 'W':
		  	case 'g':
		  	case 'G':
		  		plaintext++;
	  			break;
		  	default:
				strcat(color_text, "`");
				ct_point++;
				break;
		  }
		else
		  switch(*plaintext) {
			case 'k':
				strcat(color_text, "[0;30m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'K':
				strcat(color_text, "[1;30m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'r':
				strcat(color_text, "[0;31m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'R':
				strcat(color_text, "[1;31m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'g':
				strcat(color_text, "[0;32m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'G':
				strcat(color_text, "[1;32m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'y':
				strcat(color_text, "[0;33m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'Y':
				strcat(color_text, "[1;33m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'b':
				strcat(color_text, "[0;34m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'B':
				strcat(color_text, "[1;34m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'm':
				strcat(color_text, "[0;35m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'M':
				strcat(color_text, "[1;35m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'c':
				strcat(color_text, "[0;36m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'C':
				strcat(color_text, "[1;36m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'w':
				strcat(color_text, "[0;37m");
				ct_point+=7;
				plaintext++; 
				break;
			case 'W':
				strcat(color_text, "[1;37m");
				ct_point+=7;
				plaintext++; 
				break;
			default:
				strcat(color_text, "`");
				ct_point++;
				break;
		}
	}
	strcat(color_text, "[0m\0");
	return(color_text);
}

char *figurestate(int current, int max)
{
    static char status[40];
    
    bzero(status, sizeof(status));
    if (current >= (max/3*2)) sprintf(status, "`W%d`w", current);
    else if (current >= max/3) sprintf(status, "`Y%d`w", current);
    else sprintf(status, "`R%d`w", current);
    return (status);
}

char *damstatus(CHAR_DATA *ch)
{
    int percent;
    static char wound[40];
 
    bzero(wound, sizeof(wound));
    if (ch->max_hit > 0)
        percent = ch->hit * 100 / ch->max_hit;
    else
        percent = -1;
    if (percent >= 100)
        sprintf(wound,"excellent condition");
    else if (percent >= 90)
        sprintf(wound,"few scratches");
    else if (percent >= 75)
        sprintf(wound,"small wounds");
    else if (percent >= 50)
        sprintf(wound,"quite a few wounds");
    else if (percent >= 30)
        sprintf(wound,"nasty wounds");
    else if (percent >= 15)
        sprintf(wound,"pretty hurt");
    else if (percent >= 0)
        sprintf(wound,"awful condition");
    else
        sprintf(wound,"bleeding to death");
    return (wound);
}

char *doparseprompt(CHAR_DATA *ch)
{
	CHAR_DATA *tank,*victim;
	static char finished_prompt[240];
	char workstr[100];
	char *fp_point;
	char *orig_prompt;
/* next 6 lines added by namsherf 05201999 for exits in prompt */
	char buf2[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
        bool found;
        const char *dir_name[] = {"N", "E", "S", "W", "U", "D"};
        int door;
        char doors[MAX_INPUT_LENGTH];

	buf2[0] = '\0'; /*spellsong add*/

	bzero(finished_prompt, sizeof(finished_prompt));
	orig_prompt=ch->pcdata->prompt;
	fp_point=finished_prompt;
	while(*orig_prompt != '\0') {
		if (*orig_prompt != '%') {
			*fp_point = *orig_prompt;
			orig_prompt++;
			fp_point++;
			continue;
		}
		orig_prompt++;
		switch(*orig_prompt) {
	    		case 'h':  	sprintf(workstr, "%d", ch->hit);
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'H':  	sprintf(workstr, "%d", ch->max_hit);
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'm':  	sprintf(workstr, "%d", ch->mana);
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'M':  	sprintf(workstr, "%d", ch->max_mana);
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'v':	sprintf(workstr, "%d", ch->move);
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'V':	sprintf(workstr, "%d", ch->max_move);
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'r':	strcat(finished_prompt, "\n\r");
					fp_point++;
					orig_prompt++;
					break;
			case 'i':	sprintf(workstr, "%s", figurestate(ch->hit, ch->max_hit));
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'n':	sprintf(workstr, "%s", figurestate(ch->mana, ch->max_mana));
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'w':	sprintf(workstr, "%s", figurestate(ch->move, ch->max_move));
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
	    				orig_prompt++;
					break;
			case 'l':  	if ((tank=ch->fighting) != NULL) 
					    if ((tank=tank->fighting) != NULL) {
	 		 		        sprintf(workstr, "%s", damstatus(tank));
		    				strcat(finished_prompt, workstr);
		    				fp_point+=strlen(workstr);
					    }
    					orig_prompt++;
					break;
			case 'e':	if ((victim=ch->fighting) != NULL) {
					    sprintf(workstr, "%s", damstatus(victim));
		    				strcat(finished_prompt, workstr);
		    				fp_point+=strlen(workstr);
					}
    					orig_prompt++;
					break;
			case 's':	sprintf(workstr, "%s", damstatus(ch));
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
    					orig_prompt++;
					break;
			case 't':	sprintf(workstr,"%d:%d%s%s",(time_info.hour % 12 == 0)
						? 12 : time_info.hour %12,time_info.minute, time_info.minute==0?"0":"",time_info.hour >=
						12 ? "pm" : "am");
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
    					orig_prompt++;
					break;
			case 'T':	sprintf(workstr,"%d:%d%s",time_info.hour,time_info.minute,time_info.minute==0?"0":" ");
	    				strcat(finished_prompt, workstr);
	    				fp_point+=strlen(workstr);
    					orig_prompt++;
					break;
                        case '#':       if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
                                        {
					 sprintf(workstr, "%d", ch->in_room->vnum);
                                         strcat(finished_prompt, workstr);
                                         fp_point+=strlen(workstr);
                                         orig_prompt++;
                                         break;
					}
					else
                                        {
                                         sprintf(workstr, "IMMORTAL ONLY" );
                                         strcat(finished_prompt, workstr);
                                         fp_point+=strlen(workstr);
                                         orig_prompt++;
					}
					 break;
                        case 'A':       if (IS_SET(ch->act,PLR_AFK))
					{
					 sprintf(workstr, "`W(AFK)");
                                         strcat(finished_prompt, workstr);
                                         fp_point+=strlen(workstr);
					}
					if (IS_SET(ch->act,PLR_KILLER))
					{
					 sprintf(workstr, "`R(KILLER)");
                                         strcat(finished_prompt, workstr);
                                         fp_point+=strlen(workstr);
					}
					if (IS_SET(ch->act,PLR_THIEF))
					{
					 sprintf(workstr, "`K(THIEF)");
                                         strcat(finished_prompt, workstr);
                                         fp_point+=strlen(workstr);
					}
					if( IS_IMMORTAL( ch ) && IS_SET(ch->act, PLR_WIZINVIS))
					{
					 sprintf(workstr, "`B(Wizi:%d)",ch->invis_level);
					 strcat(finished_prompt, workstr);
                                         fp_point+=strlen(workstr);
					}
                                        orig_prompt++;
                                        break;
                        case 'x':       sprintf(workstr, "%ld", ch->exp);
                                        strcat(finished_prompt, workstr);
                                        fp_point+=strlen(workstr);
                                        orig_prompt++;
                                        break;
                        case 'X':       sprintf(workstr, "%ld", exp_per_level(ch,ch->pcdata->points)-ch->exp);
                                        strcat(finished_prompt, workstr);
                                        fp_point+=strlen(workstr);
                                        orig_prompt++;
                                        break;
                        /* Added by namsherf 05201999 for Exits in the prompt */
                      case 'E':       
                                        found = FALSE;
                                        doors[0] = '\0';
                                        for (door = 0; door < 6; door++)
                                        {
                                            if ((pexit = ch->in_room->exit[door]) != NULL
                                                && pexit->u1.to_room != NULL
                                                && (can_see_room(ch, pexit->u1.to_room)
                                                    || (IS_AFFECTED(ch, AFF_INFRARED)
                                                        && !IS_AFFECTED(ch, AFF_BLIND)))
                                                && !IS_SET(pexit->exit_info, EX_CLOSED))
                                            {
                                                found = TRUE;
                                                strcat(doors, dir_name[door]);
                                            }
                                        }
                                        if (!found)
                                            strcat(buf2, "none");
                                        else
                                        {
                                            sprintf(buf2, "%s", doors);
                                            strcat(finished_prompt, buf2);
                                        }
                                        fp_point+=strlen(buf2);
                                        orig_prompt++;
                                        break;

                        /* Added by namsherf 05201999 for Room Name in the prompt */
                      case 'R':
                                        strcat(finished_prompt, ch->in_room->name);
                                        fp_point+=strlen(ch->in_room->name);
                                        orig_prompt++;
                                        break;

			default:	strcat(finished_prompt, "%");
					fp_point++;
					break;			
		}
	}
	return(finished_prompt);
}

void printf_to_char(CHAR_DATA *ch, char *fmt, ... )
{
        char param[ MAX_STRING_LENGTH*4 ];
        {
            va_list args;
            va_start (args, fmt);
            vsprintf (param, fmt, args);
            va_end (args);
        }
        if ( param[0] && ch->desc )
             write_to_buffer( ch->desc, param, strlen(param) );
           
        return;          
}

