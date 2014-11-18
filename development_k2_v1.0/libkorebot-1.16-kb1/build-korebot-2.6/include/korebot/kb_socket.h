/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2005/01/31 07:34:28 $
 * $Revision: 1.4 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/kb_socket.h,v 1.4 2005/01/31 07:34:28 pbureau Exp $
 */

#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define MAXPENDING 5    /* Maximum outstanding connection requests */

/******************************************************************/
/*!
 * KoreBot library socket interface provides an high level server socket interface.
 */

/*!
 * Korebot socket struct
 */
typedef struct ksock_s {
    struct sockaddr_in serv_addr; /* Local address */
    int    serv_socket;           /* Socket descriptor for server */
} ksock_t;

/* Prototypes */
extern int ksock_server_open(ksock_t * server, unsigned short port);
extern int ksock_next_connection(ksock_t * server);
extern int ksock_exec_command(char * cmd);
extern int ksock_exec_command_pending(int clntSock, char * cmd);
extern int ksock_send_command(int socket, char * cmd, ...);
extern int ksock_connect(char * servIP, unsigned short servPort);
