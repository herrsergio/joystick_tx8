/* msocket.cpp: ************************************************************

   My Socket Handler functions library implementation.
   
   28/April/2003: Hiram Galicia. Version 2.0

   NOTES:

 - Based on gsNetPoll library
 - Object Oriented Programming

************************************************************************* */

/* standard c libraries */

   #include <unistd.h>
   #include <string.h>
   #include <stdlib.h>
   #include <stdio.h>

/* standard c libraries for sockets management */

   #include <netinet/in.h>
   #include <sys/socket.h>
   #include <sys/types.h>
   #include <sys/fcntl.h>
   #include <arpa/inet.h>
   #include <netdb.h>

/* user libraries */
 
   #include "msocket.h"

/* msocketInitSocket: ******************************************************

   msocketInitSocket returns MSOCKET_OK (i.e. 0) or the errorcode.

   hostname   : mezcal.fi-b.unam.mx, tx8.fi-b.unam.mx, www.yahoo.com, 132.248.59.97, ...
   port       : 2010, 3234, 8230, ...
   mode       : RX_SOCKET, TX_SOCKET

   call example:

   msocketData msd;

   msd.msocketInitSocket( "mezcal.fi-b.unam.mx",5000,TX_SOCKET )
   
************************************************************************* */

int msocketData::msocketInitSocket( char *hostname,int port,int Mode )
{
   struct hostent *hp;	/* host address structure */
   int arg;		/* fcntl argument         */

/* check the socket mode: RX_SOCKET,TX_SOCKET */

   if( Mode<RX_SOCKET || Mode>TX_SOCKET ) return( MSOCKET_INVALID_MODE );

   mode = Mode;		/* set the socket mode */

/* socket creation request */ 

   if( (sock = socket( AF_INET,SOCK_DGRAM,0 )) < 0 ) {
      return( MSOCKET_CAN_NOT_CREATE );
   }

   if( mode==RX_SOCKET ) {

   /* Request that the socket be non-blocking. Exact behavior depends
      on flavor of Unix; OSF is sloppy and gives EAGAIN; I expect others
      will return 0 bytes read, no error */

      arg = fcntl( sock,F_GETFL,arg );
            fcntl( sock,F_SETFL,arg|FNDELAY );
   }

/* initialize the socket's address structure */

   bzero( (char *)&s.sin,sizeof(s.sin) );
    
   s.sin.sin_family = AF_INET;	/* address family */

/* get the host address */
    
   if( mode==RX_SOCKET ) {
      s.sin.sin_addr.s_addr = INADDR_ANY;		 /* it doesn't matter who sends the data */ 
   } else {
      if( (hp = gethostbyname( hostname )) == NULL ) {			/* convert hostname to IP*/
         return( MSOCKET_IPADDR_NOT_FOUND ); 
      }
      bcopy( hp->h_addr,&(s.sin.sin_addr.s_addr),hp->h_length );	/* copy IP address */
   }
   s.sin.sin_port = htons( port );					/* address port */

/* bind the socket to the address (only on RX_SOCKET mode)*/
    
   if( mode==RX_SOCKET ) {
   
      if( bind( sock,&s.sa,sizeof(s.sa) ) < 0 ) {
         return( MSOCKET_IMPOSSIBLE_TO_BIND );
      }
   }

   return( MSOCKET_OK ); /* succesfull socket creation */ 
}

void msocketData::msocketKillSocket( void )
{
   close( sock );
}

int msocketData::msocketRxData( char *buffer,int buffersize )
{
   int n;
   
   if( mode != RX_SOCKET ) return( MSOCKET_RX_ATTEMPT_IN_TX_SOCK );
    
   n = read( sock,buffer,buffersize );

   return( n );
}

int msocketData::msocketTxData( char *buffer,int buffersize )
{
   if( mode != TX_SOCKET ) return( MSOCKET_TX_ATTEMPT_IN_RX_SOCK );

   if( sendto( sock,buffer,buffersize,0,&(s.sa),sizeof(s.sa)) < 0 )
      return( MSOCKET_CAN_NOT_TX );

   return( MSOCKET_OK );
}

int msocketGetIpAdd( char *hostname,char *ipaddress,char *realhost )
{
   struct hostent *he;

   if( (he=gethostbyname( hostname )) == NULL ) return( MSOCKET_IPADDR_NOT_FOUND_GIA );

   sprintf( ipaddress,"%s",inet_ntoa( *((struct in_addr *)he->h_addr) ) );
   sprintf( realhost,he->h_name );

   return( MSOCKET_OK );
}

char *msocketGetError( int errorcode )
{
   switch( errorcode ) {
      case MSOCKET_OK:
         return( "msocket: Socket Ok: there is no problem." );
      case MSOCKET_INVALID_MODE:
         return( "msocket: msocketInitSocket: invalid mode opening socket." );
      case MSOCKET_CAN_NOT_CREATE:
         return( "msocket: msocketInitSocket: can not create the socket." );
      case MSOCKET_IPADDR_NOT_FOUND:
         return( "msocket: msocketInitSocket: can not found the ipaddress of that hostname." );
      case MSOCKET_IMPOSSIBLE_TO_BIND:
         return( "msocket: msocketInitSocket: can not bind name to datagram socket." );
      case MSOCKET_RX_ATTEMPT_IN_TX_SOCK:
         return( "msocket: msocketRxData: attempting to rx in a tx socket." );
      case MSOCKET_TX_ATTEMPT_IN_RX_SOCK:
         return( "msocket: msocketTxData: attempting to tx in a rx socket." );
      case MSOCKET_CAN_NOT_TX:
         return( "msocket: msocketTxData: can not send datagram." );
      case MSOCKET_IPADDR_NOT_FOUND_GIA:
         return( "msocket: msocketGetIpAdd: can not found the ipaddress of that hostname." );
      default:
         return( "msocket: unknown error." );
   }
}

/* end of file: msocket.cpp ********************************************* */

