/*! 
 * \file   koala_lrf_demo.c Led Range Finder Demo
 *
 * \brief  Obstacle avoidance program for the Koala robot, using the
 *         Led Range Finder
 *
 * \author   P.Bureau (K-Team SA)
 *
 * \note     Copyright (C) 2005 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     
 */

#include <signal.h>
#include <korebot/korebot.h>

#define device "/dev/tts/2"

#define MAX_DIST 2000
#define RCVBUFSIZE 32   /* Size of receive buffer */

/* Global Handle for the LRF */
int lrf;
knet_dev_t *koala;

int cmd_received = 0;

/* LRF management task */
void * koa_lrf_task(void * arg)
{
  long left,right;
  unsigned workdata[121];
  int potential[121];
  int i;
  enum {suspended, active} state = active;

  struct timeval lastcommand, now, diff;
  struct timezone here;

  /* Get the handle for the LRF */
  lrf = kb_lrf_Init(device);

  /* Fill in the potential table */
  for(i=0; i<10; i++)
    potential[i] = -1;
  
  for(i=10; i<30; i++)
    potential[i] = -7;

  for(i=30; i<40; i++)
    potential[i] = -15;
 
  for(i=40; i<50; i++)
    potential[i] = -22;
 
  for(i=50; i<60; i++)
    potential[i] = -27;
 
  for(i=60; i<70; i++)
    potential[i] = 9;

  for(i=70; i<80; i++)
    potential[i] = 7;

  for(i=80; i<90; i++)
    potential[i] = 6;

  for(i=90; i<110; i++)
    potential[i] = 3;

  for(i=110; i<121; i++)
    potential[i] = 1;
  
#if 1
  for(;;)
  {
    switch(state)
    {
      case active:
	if(cmd_received)
	{
	  gettimeofday(&lastcommand,&here);
	  state = suspended;
	}
	else
	  if(kb_lrf_GetDistances(lrf) >= 0)
	  {    
	    for(i=0; i<121; i++)
	      if(kb_lrf_DistanceData[i])
		workdata[i] = MAX_DIST/kb_lrf_DistanceData[i];
	      else
		workdata[i] = 0;

	    for(i=0, right=0, left=0; i<121; i++)
	    {
	      right += (workdata[i] * potential[i]);
	      left  += (workdata[i] * potential[120-i]);
	    }

	    printf("done ok d:%ld g:%ld\r\n",right,left);

	    right = (right / 10) + 100;
	    left  = (left  / 10) + 100;

	    printf("droite:%ld gauche:%ld\r\n",right,left);
	    koa_setSpeed(koala,right,left);
	  }

	break;
	
      case suspended:
	gettimeofday(&now,&here);
	timersub(&now,&lastcommand,&diff);

	if(diff.tv_sec > 3)
	{
	  state        = active;
	  cmd_received = 0;
	}
	  
	break;
    }
  }
#endif

}
  
void handle_kill(int arg)
{
  fprintf(stderr, "arg... killed\r\n");

  koa_setSpeed(koala,0,0);
  /* shutdown the lrf */
  kb_lrf_Close(lrf);

  exit(1);
}

int koa_net_stop(int argc, char * argv[])
{
  koa_setSpeed(koala,0,0);

  return 0;
}

int koa_net_setspeed(int argc, char * argv[])
{
  int right = 0;
  int left  = 0;
  int i;

  right = atoi(argv[1]);
  left  = atoi(argv[2]);

  koa_setSpeed(koala, right, left);

  for(i=0;i<argc;i++)
    printf("%s ",argv[i]);
  printf("(%d,%d)\r\n",right,left);

  return 0;
}

int main( int argc , char * argv[] )
{
  int rc;
  unsigned char a,b,c,d;
  int clntSocket;
  ksock_t server;
  char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
  int port;                           /* The server port to open */

  pthread_t lrf_task;

  /* Set signal handler */
  signal(SIGINT, handle_kill);

  if(argc > 1)
    port = atoi(argv[1]);
  else
    port = 344;
  
  /* Set the libkorebot debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  printf("Koala Demo Program\r\n");

  /* Korebot socket module init */
  if((rc = ksock_init('\n')) < 0 )
    return 1;
  /* Open a device for the koala robot */
  koala = knet_open( "Koala:Robot", KNET_BUS_ANY, 0 , NULL );
  if(!koala)
  {
    printf("Open Koala device failed\r\n");
    return 1;
  }

  /* Koala Comm test */
  koa_getOSVersion(koala,&a,&b,&c,&d);
  printf("OS revision %d.%d  protocol %d.%d\r\n",a,b,c,d);
  koa_setSpeed(koala,0,0);

  /* Network setup - Create a ksock server */
  ksock_server_open(&server, port);
  
  ksock_add_command("setspeed",2,2,koa_net_setspeed);
  ksock_add_command("stop",0,0,koa_net_stop);
  list_command();

  /* Task creation */
  rc = pthread_create(&lrf_task, NULL, koa_lrf_task, NULL);
  if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

  /* Network connection handling */
  clntSocket = ksock_next_connection(&server);

  for(;;)
  {
#if 1
    rc = ksock_get_command(clntSocket, echoBuffer, RCVBUFSIZE);

    if(rc>0)
    {
      cmd_received = 1;
      ksock_exec_command(echoBuffer);
      /* Echo message back to client */
      if (send(clntSocket, echoBuffer, rc, 0) != rc)
	DieWithError("send() failed");
    } 
    else 
      switch(rc)
      {
	case 0 :
	  break;
	case -3 :
	  printf("Client disconnected\r\n");
	  clntSocket = ksock_next_connection(&server);
	  break;
	default :
	  printf("Socket error: %d\r\n",rc);
	  clntSocket = ksock_next_connection(&server);
	  break;
      }
#endif
    
  }
  close(clntSocket);    /* Close client socket */
}
