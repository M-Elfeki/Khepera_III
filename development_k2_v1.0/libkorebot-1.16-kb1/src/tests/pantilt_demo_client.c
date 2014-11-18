/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2005/01/20 08:42:05 $
 * $Revision: 1.1 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/tests/pantilt_demo_client.c,v 1.1 2005/01/20 08:42:05 pbureau Exp $
 */

/*--------------------------------------------------------------------*/
/*! 
 * \file   pantilt_demo_client.c pantilt demo program (client side)
 *
 * \brief
 * 	   Demo program to remote control 2 pantilt camera from 
 * 	   a wireless ethernet link. The client reads events from a joystick
 * 	   and send network command to the remote KoreBot.
 *
 * \author   Pierre Bureau (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 */


#include <korebot/korebot.h>
#include <linux/joystick.h>
#include <signal.h>


int sendflag   = 0;
int timeflag   = 0;
int actionflag = 0;
int sock;                        /* Socket descriptor */

#define ACT_KOA  0
#define ACT_CAM1 1
#define ACT_CAM2 2
#define ACT_BOTH 3

void handle_kill(int arg)
{
  fprintf(stderr, "arg... killed\r\n");
  shutdown(sock,2);

  close(sock);

  exit(1);
}

void handle_joyaxis(int number, int value, char * echoBuffer)
{
  static int campos0=0,campos1=0,campos2=0,campos3=0;

  switch(actionflag)
  {
      /* Moving camera 1 */
    case ACT_CAM1 :
      if(number == 1) {
	campos0 = ((value*50)/32767) + 50;
	//sprintf(echoBuffer,"movecamera %d %d\n",campos1,campos0);
	ksock_send_command(sock,"movecamera %d %d",campos1, campos0);
	//sendflag = 1;
	timeflag = 0;
      }

      if(number == 0) {
	campos1 = 50 - ((value*50)/32767);
	//sprintf(echoBuffer,"movecamera %d %d\n",campos1,campos0);
	ksock_send_command(sock,"movecamera %d %d",campos1, campos0);
	//sendflag = 1;
	timeflag = 0;
      }
      break;

      /* Moving camera 2 */
    case ACT_CAM2 :
      if(number == 1) {
	campos0 = ((value*50)/32767) + 50;
	//sprintf(echoBuffer,"movecamera2 %d %d\n",campos1,campos0);
	ksock_send_command(sock,"movecamera2 %d %d",campos1, campos0);
	//sendflag = 1;
	timeflag = 0;
      }

      if(number == 0) {
	campos1 = 50 - ((value*50)/32767);
	//sprintf(echoBuffer,"movecamera2 %d %d\n",campos1,campos0);
	ksock_send_command(sock,"movecamera2 %d %d",campos1, campos0);
	//sendflag = 1;
	timeflag = 0;
      }
      break;

      /* Moving both camera */
    case ACT_BOTH :
      if(number == 1) {
	campos0 = ((value*50)/32767) + 50;
	sprintf(echoBuffer,"moveboth %d %d\n",campos1,campos0);
	sendflag = 1;
	timeflag = 0;
      }

      if(number == 0) {
	campos1 = 50 - ((value*50)/32767);
	sprintf(echoBuffer,"moveboth %d %d\n",campos1,campos0);
	sendflag = 1;
	timeflag = 0;
      }
      break;
  }
}

void handle_joybutton(int number, int value, char * echoBuffer)
{
  switch(number)
  {
    case 2 :
      if(value)
	actionflag = ACT_BOTH;
      else
	if(actionflag == ACT_BOTH)
	{
	  //sprintf(echoBuffer,"moveboth %d %d\n",50,50);
	  //sendflag = 1;
	  actionflag = ACT_KOA;
	}
      break;
    case 3 :
      if(value)
	actionflag = ACT_CAM2;
      else
	if(actionflag == ACT_CAM2)
	{
	  sprintf(echoBuffer,"movecamera2 %d %d\n",50,50);
	  sendflag = 1;
	  timeflag = 1;
	  actionflag = ACT_KOA;
	}
      break;
    case 4 :
      if(value)
	actionflag = ACT_CAM1;
      else
	if(actionflag == ACT_CAM1)
	{
	  sprintf(echoBuffer,"movecamera %d %d\n",50,50);
	  sendflag = 1;
	  timeflag = 1;
	  actionflag = ACT_KOA;
	}
      break;
    case 10 :
      /* init camera */
      if(value)
	ksock_send_command(sock,"initcamera");
      break;
  } 
}

#define RCVBUFSIZE 32   /* Size of receive buffer */

int main(int argc, char *argv[])
{
    struct timeval lastcommand, now, diff;
    struct timezone here;

    char *servIP;                    /* Server IP address (dotted quad) */
    unsigned short servPort;         /* Echo server port */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() 
                                        and total bytes read */
    int fd;
    int readevent;
    struct js_event e;

    /* Set signal handler */
    signal(SIGINT, handle_kill);

    /* Set the starting time */
    gettimeofday(&lastcommand,&here);
    
    if (argc < 3)     /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Port>\n", argv[0]);
       exit(1);
    }

    /* Open the Joystick device */
    fd = open ("/dev/input/js0", O_RDONLY|O_NONBLOCK);
    if(fd < 0)
    {
      printf("Cannot open joystick(/dev/input/js0)\r\n");
      exit(1);
    }

    servIP = argv[1];             /* First arg: server IP address (dotted quad) */

    servPort = atoi(argv[2]); /* Use given port, if any */

    ksock_init('\n',2048);
	
    sock = ksock_connect(servIP, servPort);
    if(sock > 0)
      printf("Connected to %s:%d\r\n",servIP,servPort);

    /* Start reading event from the joystick */
    readevent = read (fd, &e, sizeof(struct js_event));

    while (readevent >=0 || sendflag || errno == EAGAIN)
    {
      if(readevent >= 0)
      {
	/* A new event is available */
	e.type &= ~JS_EVENT_INIT;
	switch (e.type)
	{

	  /* Handle joystick button event */
	  case JS_EVENT_BUTTON:
	    //printf("Button %d at pos %d\n", e.number, e.value);
	    handle_joybutton(e.number,e.value,echoBuffer);
	    break;

	  /* Handle joystick axis event */
	  case JS_EVENT_AXIS:
	    //printf("Axis %d at pos %d\n", e.number, e.value);
	    handle_joyaxis(e.number,e.value,echoBuffer);
	    break;
	}
      }

      /* Send the command to the server if required */
      if(sendflag) {
	/* check timing between to commands */
	gettimeofday(&now,&here);
	timersub(&now,&lastcommand,&diff);
	//printf("elapsed: %ld.%.6ld\r\n",diff.tv_sec,diff.tv_usec);

	if(diff.tv_usec > 100000 || !timeflag)
	{
	  lastcommand.tv_sec  = now.tv_sec;
	  lastcommand.tv_usec = now.tv_usec;

	  printf("%s\n",echoBuffer);
	  /* Send the command to the robot */
	  echoStringLen = strlen(echoBuffer);
	  if (send(sock, echoBuffer, echoStringLen, 0) != echoStringLen)
	    DieWithError("send() sent a different number of bytes than expected");
	  /* Receive the same string back from the server */
	  totalBytesRcvd = 0;
	  printf("Received: ");                /* Setup to print the echoed string */
	  while (totalBytesRcvd < echoStringLen)
	  {
	    /* Receive up to the buffer size (minus 1 to leave space for
	       a null terminator) bytes from the sender */
	    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
	      DieWithError("recv() failed or connection closed prematurely");
	    totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
	    echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
	    printf(echoBuffer);            /* Print the echo buffer */
	  }

	  printf("\n");    /* Print a final linefeed */

	  sendflag = 0;
	}
      }

      /* Read the next event */
      readevent = read (fd, &e, sizeof(struct js_event));
    }

    close(fd);
    close(sock);
    exit(0);
}
