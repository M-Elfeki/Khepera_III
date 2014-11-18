/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2005/10/25 12:44:56 $
 * $Revision: 1.7 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/kb_socket.c,v 1.7 2005/10/25 12:44:56 pbureau Exp $
 */

#include "korebot.h"

static char ksock_command_terminator = '\n';
static char * ksock_buf_snd          = NULL;
static unsigned ksock_buf_len        = 0;

static struct kb_command_s * ksock_cmd_list = NULL;
unsigned                     ksock_cmd_n;

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

/*!
 * Initialize the socket library
 * \param terminator The termination characters for network commands
 * \param buffer_len length of the buffer to send commands
 */
int ksock_init(char terminator, unsigned buffer_len)
{
  ksock_command_terminator = terminator;
  
  ksock_cmd_n = 0;

  ksock_cmd_list = KB_ALLOC(struct kb_command_s, 1);
  ksock_cmd_list->name     = NULL;
  ksock_cmd_list->minParam = 0;
  ksock_cmd_list->maxParam = 0;
  ksock_cmd_list->parse    = NULL;
 
  /* Init the command buffer */
  ksock_buf_snd = KB_ALLOC(char, buffer_len);
  ksock_buf_len = buffer_len;

  return 0;
}


/*!
 * Open a server socket
 */
int ksock_server_open(ksock_t * server, unsigned short port)
{
    /* Create socket for incoming connections */
    if ((server->serv_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&(server->serv_addr), 0, sizeof(server->serv_addr)); /* Zero out structure */
    server->serv_addr.sin_family = AF_INET;                     /* Internet address family */
    server->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);      /* Any incoming interface */
    server->serv_addr.sin_port = htons(port);                   /* Local port */

    /* Bind to the local address */
    if (bind(server->serv_socket,
		(struct sockaddr *) &(server->serv_addr),sizeof(server->serv_addr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(server->serv_socket, MAXPENDING) < 0)
        DieWithError("listen() failed");

    return 0;
}


/*!
 * Get the next connection to the given server
 */
int ksock_next_connection(ksock_t * server)
{
  struct sockaddr_in echoClntAddr; /* Client address */
  unsigned int clntLen;            /* Length of client address data structure */
  int clntSock;                    /* Socket descriptor for client */
      
  /* Set the size of the in-out parameter */
  clntLen = sizeof(echoClntAddr);

  /* Wait for a client to connect */
  if ((clntSock = accept(server->serv_socket, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
    DieWithError("accept() failed");

  /* Mark the socket as non blocking */
  fcntl(clntSock, F_SETFL, O_NONBLOCK);

  /* clntSock is connected to a client! */
  kb_warning(KB_WARN_CONNECT, inet_ntoa(echoClntAddr.sin_addr));

  return clntSock;
}

/*!
 * Add a network command to the command list. The number of argument does include
 * the name of the command itself. That means a command without args should have
 * 1 as both min_param and max_param.
 *
 * \return
 *   - KB_ERROR_NOINIT if the socket module is not initialize
 *   - 0 if successful
 */
int ksock_add_command(const char * name, int min_param, 
                      int max_param, int (*function)(int argc, char *argv[], void * data))
{
  if(!ksock_cmd_list) {
    KB_ERROR("ksock_exec_command", KB_ERROR_NOINIT, "kb_socket");
    return -KB_ERROR_NOINIT;
  }

  /* Store the new command in the list, replacing the last */
  ksock_cmd_list[ksock_cmd_n].name = KB_ALLOC(char, strlen(name));
  strcpy(ksock_cmd_list[ksock_cmd_n].name, name);
  ksock_cmd_list[ksock_cmd_n].minParam = min_param;
  ksock_cmd_list[ksock_cmd_n].maxParam = max_param;
  ksock_cmd_list[ksock_cmd_n].parse    = function;

  ksock_cmd_n++;

  /* Terminate the list */
  ksock_cmd_list = KB_REALLOC(ksock_cmd_list, struct kb_command_s, ksock_cmd_n + 1);
  ksock_cmd_list[ksock_cmd_n].name     = NULL;
  ksock_cmd_list[ksock_cmd_n].minParam = 0;
  ksock_cmd_list[ksock_cmd_n].maxParam = 0;
  ksock_cmd_list[ksock_cmd_n].parse    = NULL;

  return 0;
}

/*!
 * Remove a network command from the command list
 * 
 *
 *
 */
int ksock_remove_command(char * name)
{
  int index;

  if(!ksock_cmd_list) {
    KB_ERROR("ksock_exec_command", KB_ERROR_NOINIT, "kb_socket");
    return -KB_ERROR_NOINIT;
  }

  index = kb_find_command(name, ksock_cmd_list);
  if(index < 0)
    return index;

  for(; index<ksock_cmd_n-1; index++) {
    memcpy(&(ksock_cmd_list[index]), &ksock_cmd_list[index+1], sizeof(kb_command_t));
  }

  ksock_cmd_list[index].name     = NULL;
  ksock_cmd_list[index].minParam = 0;
  ksock_cmd_list[index].maxParam = 0;
  ksock_cmd_list[index].parse    = NULL;

  return 0;
}

void list_command()
{
  kb_command_t * scan = ksock_cmd_list;
  
  while(scan->name != NULL) {
    printf("%s: %d,%d\r\n", scan->name, scan->minParam, scan->maxParam);
    scan++;
  }
}

/*!
 * Send a network command through the given socket
 * \param socket the receiving socket
 * \param cmd the command line
 */
int ksock_send_command(int socket, char * cmd, ...)
{
  va_list ap;
  int buflen;
  
  /* get va_list */
  va_start(ap, cmd);
  
  /* Build the command string */
  vsnprintf(ksock_buf_snd, ksock_buf_len-1, cmd, ap);
  buflen                  = strlen(ksock_buf_snd);
  ksock_buf_snd[buflen]   = ksock_command_terminator;
  //ksock_buf_snd[buflen+1] = '\0';
  buflen++;
  
  /* Send the command to the robot */
  if (send(socket, ksock_buf_snd, buflen, 0) != buflen)
    DieWithError("send() sent a different number of bytes than expected");

  /* end va_list */
  va_end(ap);
}

/*!
 * Send a network answer through the given socket this function
 * should be called from a command function to send the answer 
 * back after a ksock_exec_command_pending.
 * \param socket the receiving socket
 * \param cmd the command line
 */
int ksock_send_answer(int * socketStorage, char * cmd)
{
  int buflen;
  
  /* Add the command terminator */
  strncpy(ksock_buf_snd,cmd,ksock_buf_len-1);
  buflen                  = strlen(ksock_buf_snd);
  ksock_buf_snd[buflen]   = ksock_command_terminator;
  buflen++;
  
  /* Send the command to the robot */
  if (send(*socketStorage, ksock_buf_snd, buflen, 0) != buflen)
    DieWithError("send() sent a different number of bytes than expected");

  kb_free(socketStorage);
}

/*!
 * Send a network command through the given socket and wait for an acknowledge
 * from the server. If the acknowledge is not received before the given timeout,
 * the function returns with an error code.
 * \param socket the receiving socket
 * \param cmd the command line
 */

/*!
 * Execute a network command that require an answer
 * 
 * The application should  send the corresponding answer 
 * using ksock_send_answer.
 *
 * Such network commands must use the following syntax:
 * The fisrt word is the command name.
 * The second word is the request id (unsigned long integer).
 * The following words are the command parameters.
 *
 * \param cmd the command line
 * \param clntSock the connection id
 * \return
 *    - the return value of the command if succesful
 *    - KB_ERROR_INVAL if the command is invalid
 *    - KB_ERROR_TOOMANYARGS if number of args is invalid
 *    - KB_ERROR_UNKCMD if the command is unknown
 */
int ksock_exec_command_pending(int clntSock, char * cmd)
{
  int * socketStorage;

  if(!ksock_cmd_list) {
    KB_ERROR("ksock_exec_command", KB_ERROR_NOINIT, "kb_socket");
    return -KB_ERROR_NOINIT;
  }

  socketStorage  = KB_ALLOC(int,1);
  *socketStorage = clntSock;
  
  return kb_parse_command( cmd, ksock_cmd_list,(void*)socketStorage);
}

/*!
 * Execute a network command 
 * 
 * \param cmd the command line
 * \return
 *    - the return value of the command if succesful
 *    - KB_ERROR_INVAL if the command is invalid
 *    - KB_ERROR_TOOMANYARGS if number of args is invalid
 *    - KB_ERROR_UNKCMD if the command is unknown
 */
int ksock_exec_command(char * cmd)
{
  if(!ksock_cmd_list) {
    KB_ERROR("ksock_exec_command", KB_ERROR_NOINIT, "kb_socket");
    return -KB_ERROR_NOINIT;
  }
    
  return kb_parse_command( cmd, ksock_cmd_list,NULL);
}

/*!
 * Get a command from the given connection. 
 *
 * \param clntSocket The connection id
 * \param cmdBuffer  The receive buffer allocated by the caller
 * \param bufSize    The size of the receive buffer
 * \return 
 *   - The received command length
 *   - 0  if the socket is non-blocking and no data is available 
 *   - -1 if a full command is not available
 *   - -2 if a coherency problem is detected
 *   - -3 if the given client socket has been disconnected
 */ 
int ksock_get_command(int clntSocket, char * cmdBuffer, unsigned bufSize)
{
  int recvMsgSize;
  char * term;
  unsigned cmdLen;

  /* Receive available data from client */
  if ((recvMsgSize = recv(clntSocket, cmdBuffer, bufSize-1, MSG_PEEK)) < 0)
  {
    if(errno == EAGAIN) {
      cmdBuffer[0] = '\0';
      return 0;
    } 
    else
      DieWithError("recv() failed");
  }

  /* check if something was available */
  if(!recvMsgSize) {
    cmdBuffer[0] = '\0';
    return -3;
  }

  /* Terminate the string */
  cmdBuffer[recvMsgSize] = '\0';

  /* Check if a full command is received */
  term = strchr(cmdBuffer, ksock_command_terminator);
  if( term == NULL) {
    cmdBuffer[0] = '\0';
    return -1;
  }

  /* Get the length of the full command */
  *term  = '\0';
  cmdLen = strlen(cmdBuffer) + 1;

  /* Remove the full command from the socket */
  if ((recvMsgSize = recv(clntSocket, cmdBuffer, cmdLen, 0)) < 0)
    DieWithError("recv() failed");
  /* Coherency Test */
  if(recvMsgSize != cmdLen) {
    cmdBuffer[0] = '\0';
    return -2;
  }

  /* Terminate the string, replacing the command terminator */
  cmdBuffer[recvMsgSize - 1] = '\0';

  /* return the comamnd length */
  return recvMsgSize;
}

/*!
 * Try to connect to the given server
 *
 * \param clntSocket The connection id
 * \param cmdBuffer  The receive buffer allocated by the caller
 * \param bufSize    The size of the receive buffer
 * \return 
 *   - the socket descriptor
 */ 
int ksock_connect(char * servIP, unsigned short servPort)
{
  struct sockaddr_in servAddr; /* Echo server address */
  int sock;

  /* Create a reliable, stream socket using TCP */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

  /* Construct the server address structure */
  memset(&servAddr, 0, sizeof(servAddr));         /* Zero out structure */
  servAddr.sin_family      = AF_INET;             /* Internet address family */
  servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
  servAddr.sin_port        = htons(servPort);     /* Server port */

  /* Establish the connection to the echo server */
  if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    DieWithError("connect() failed");

  return sock;
}

int ksock_server_close(ksock_t * server)
{

}
