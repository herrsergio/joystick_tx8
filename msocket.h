/* msocket.h: *************************************************

   My Socket Handler functions library definition.
   
   28/April/2003: Hiram Galicia. Version 2.0

   Notes:

 - Based on gsNetPoll library
 - Object Oriented Programming

************************************************************ */

#ifndef _MSOCKET_H_

   #define _MSOCKET_H_

   /* standard c libraries for sockets management */

      #include <netinet/in.h>
      #include <sys/socket.h>
      #include <netdb.h>

   /* normal socket opening modes */

      enum {
      
         RX_SOCKET, /* reception   socket mode */
         TX_SOCKET  /* transmision socket mode */
      };

   /* error table */
	  
      enum {
      
         MSOCKET_OK			= 0,

      /* errors generated while opening socket */
   
         MSOCKET_INVALID_MODE		= -2,
 	 MSOCKET_CAN_NOT_CREATE		= -3,
	 MSOCKET_IPADDR_NOT_FOUND	= -4,
	 MSOCKET_IMPOSSIBLE_TO_BIND	= -5,
  
      /* errors generated while receiving */

         MSOCKET_RX_ATTEMPT_IN_TX_SOCK	= -6,

      /* errors generated while transmiting */
   
         MSOCKET_TX_ATTEMPT_IN_RX_SOCK	= -7,
	 MSOCKET_CAN_NOT_TX		= -8,

      /* errors generated while getting ip address */
   
	 MSOCKET_IPADDR_NOT_FOUND_GIA	= -9

      };

   /* msocketData class definition */

      class msocketData {

      public:

      /* public methods */

         int   msocketInitSocket( char *hostname,int port,int Mode );
         void  msocketKillSocket( void );

         int   msocketRxData    ( char *buffer,int buffersize );
         int   msocketTxData    ( char *buffer,int buffersize );

      private:

      /* private attributes */

         int    mode;			/* socket mode: RX_SOCKET,TX_SOCKET */
         int    sock;			/* file descriptor to manage socket */

	 union {

            struct sockaddr_in sin;	/* struct to retain the socket address */
	    struct sockaddr    sa;	/* struct to retain the socket address */
	   
	 } s;
      
      };

   /* functions definition */

      int   msocketGetIpAdd( char *hostname,char *ipaddress,char *realhost );
   
      char *msocketGetError( int errorcode );

#endif /* _MSOCKET_H_ */

/* end of file: msocket.h ********************************* */

