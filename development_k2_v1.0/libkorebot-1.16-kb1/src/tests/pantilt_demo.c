/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2005/01/20 08:42:05 $
 * $Revision: 1.1 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/tests/pantilt_demo.c,v 1.1 2005/01/20 08:42:05 pbureau Exp $
 */

/*--------------------------------------------------------------------*/
/*! 
 * \file   pantilt_demo.c pantilt demo program (server side)
 *
 * \brief
 * 	   Demo program to remote control 2 pantilt camera from 
 * 	   a wireless ethernet link, using a koremotor to control 2 pantilt
 *
 * \author   Pierre Bureau (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 */


#include <korebot/korebot.h>

#define RCVBUFSIZE 32   /* Size of receive buffer */


knet_dev_t *motor0, *motor1, *motor2, *motor3;
int32_t minpos0,maxpos0,minpos1,maxpos1;
int32_t minpos2,maxpos2,minpos3,maxpos3;
  
/****************************************************************/
/*! Koala Camera init task
 *
 */
void * koa_init_task(void * arg)
{
  kmot_ResetError(motor0);
  kmot_ResetError(motor1);
  kmot_ResetError(motor2);
  kmot_ResetError(motor3);
  kmot_SearchLimits(motor0, 10, 3, &minpos0, &maxpos0,100000);
  printf("motor0: min:%ld max:%ld\n\r",minpos0, maxpos0);
  kmot_SearchLimits(motor1, 10, 3, &minpos1, &maxpos1,100000);
  printf("motor1: min:%ld max:%ld\n\r",minpos1, maxpos1);
  kmot_SearchLimits(motor2, 10, 3, &minpos2, &maxpos2,100000);
  printf("motor2: min:%ld max:%ld\n\r",minpos2, maxpos2);
  kmot_SearchLimits(motor3, 10, 3, &minpos3, &maxpos3,100000);
  printf("motor3: min:%ld max:%ld\n\r",minpos3, maxpos3);
  kmot_SetBlockedTime(motor0,5);
  kmot_SetBlockedTime(motor2,5);

  return NULL;
}


/****************************************************************/
/*! Koala Network command callbacks
 */
int koa_net_init(int argc, char * argv[])
{
  pthread_t init_task;
  int rc;

  rc = pthread_create(&init_task, NULL, koa_init_task, NULL);
  if (rc){
    KB_FATAL("koa_net_init", KB_ERROR_PTHREAD, "koa_init_task");
  }
}

int koa_net_camera(int argc, char * argv[])
{
  int i;
  int pos0,pos1;

  pos0 = atoi(argv[1]);
  if(pos0 < 5)
    pos0 = 6;
  if(pos0 > 95)
    pos0 = 95;
    
  pos1 = atoi(argv[2]);
  if(pos1 < 5)
    pos1 = 6;
  if(pos1 > 95)
    pos1 = 95;
    
  pos0 = (pos0 * minpos0)/100;
  pos1 = (pos1 * minpos1)/100;
#if 1
  kmot_SetPoint(motor0, kMotRegPos, pos0);
  kmot_SetPoint(motor1, kMotRegPos, pos1);
#else
  kmot_SetPoint(motor0, kMotRegPosProfile, pos0);
  kmot_SetPoint(motor1, kMotRegPosProfile, pos1);
#endif
  for(i=0;i<argc;i++)
    printf("%s ",argv[i]);
  printf("(%d,%d)\r\n",pos0,pos1);
}

int koa_net_camera2(int argc, char * argv[])
{
  int i;
  int pos0,pos1;

  pos0 = atoi(argv[1]);
  if(pos0 < 5)
    pos0 = 6;
  if(pos0 > 95)
    pos0 = 95;
    
  pos1 = atoi(argv[2]);
  if(pos1 < 5)
    pos1 = 6;
  if(pos1 > 95)
    pos1 = 95;
    
  pos0 = (pos0 * minpos2)/100;
  pos1 = (pos1 * minpos3)/100;
  kmot_SetPoint(motor2, kMotRegPos, pos0);
  kmot_SetPoint(motor3, kMotRegPos, pos1);
  for(i=0;i<argc;i++)
    printf("%s ",argv[i]);
  printf("(%d,%d)\r\n",pos0,pos1);
}

int koa_net_bothcam(int argc, char * argv[])
{
  int i;
  int pos0,pos1;

  pos0 = atoi(argv[1]);
  if(pos0 < 5)
    pos0 = 6;
  if(pos0 > 95)
    pos0 = 95;
    
  pos1 = atoi(argv[2]);
  if(pos1 < 5)
    pos1 = 6;
  if(pos1 > 95)
    pos1 = 95;
    
  kmot_SetPoint(motor0, kMotRegPos, (pos0 * minpos0)/100);
  kmot_SetPoint(motor1, kMotRegPos, (pos1 * minpos1)/100);
  kmot_SetPoint(motor2, kMotRegPos, (pos0 * minpos2)/100);
  kmot_SetPoint(motor3, kMotRegPos, (pos1 * minpos3)/100);
  for(i=0;i<argc;i++)
    printf("%s ",argv[i]);
  printf("(%d,%d)\r\n",pos0,pos1);
}

int net_exit(int argc, char * argv[])
{
  exit(0);
}

/****************************************************************/
/*! Pan-tilt motor initialization
 */
void InitMotor(knet_dev_t * mot)
{
    kmot_SetMode(mot,0);
    kmot_SetSampleTime(mot,1550);
    kmot_SetMargin(mot,20);
    kmot_SetOptions(mot,0,kMotSWOptWindup|kMotSWOptStopMotorBlk);     
    kmot_ResetError(mot);

    kmot_ConfigurePID(mot,kMotRegSpeed,1500,0,300);
    kmot_ConfigurePID(mot,kMotRegPos,70,50,5);
    kmot_SetSpeedProfile(mot,30,10);

    kmot_SetBlockedTime(mot,5);
    kmot_SetLimits(mot,kMotRegCurrent,0,50);
    kmot_SetLimits(mot,kMotRegPos,-10000,10000);
}

/****************************************************************/
int main( int argc , char * argv[] )
{
  ksock_t server;
  int rc;
  unsigned char buf[16];
  unsigned char a,b,c,d;
  int clntSocket;
  char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
  int recvMsgSize;                    /* Size of received message */
  int port;                           /* The server port to open */
  
  pthread_t sensor_task;
  pthread_t sound_task;

  if(argc > 1)
    port = atoi(argv[1]);
  else
    port = 344;

  /* Various initialization */
  /* Set the libkorebot debug level - Highly recommended for development. */
  kb_set_debug_level( 2 );

  /* Global libkorebot init */
  if((rc = kb_init( argc , argv )) < 0 )
    return 1;
  
  /* Korebot socket module init */
  if((rc = ksock_init('\n')) < 0 )
    return 1;

  /* Open the motor devices */
  motor1 = knet_open( "KoreMotor:PriMotor1", KNET_BUS_ANY, 0 , NULL );
  if(!motor1)
  {
    printf("Cannot open motor 0\r\n");
    return 1;
  }
  motor0 = knet_open( "KoreMotor:PriMotor2", KNET_BUS_ANY, 0 , NULL );
  if(!motor0)
  {
    printf("Cannot open motor 1\r\n");
    return 1;
  }
  motor3 = knet_open( "KoreMotor:PriMotor3", KNET_BUS_ANY, 0 , NULL );
  if(!motor3)
  {
    printf("Cannot open motor 2\r\n");
    return 1;
  }
  motor2 = knet_open( "KoreMotor:PriMotor4", KNET_BUS_ANY, 0 , NULL );
  if(!motor2)
  {
    printf("Cannot open motor 3\r\n");
    return 1;
  }

  
  /* KoreMotor Comm test */
  kmot_GetFWVersion( motor1, &rc);
  printf("Motor 1 Firmware v%u.%u\n" , KMOT_VERSION(rc) , KMOT_REVISION(rc));

  /* Intialize motor controller */
  InitMotor(motor0);
  InitMotor(motor1);
  InitMotor(motor2);
  InitMotor(motor3);
  kmot_SearchLimits(motor0, 10, 3, &minpos0, &maxpos0,100000);
  printf("motor0: min:%ld max:%ld\n\r",minpos0, maxpos0);
  kmot_SearchLimits(motor1, 10, 3, &minpos1, &maxpos1,100000);
  printf("motor1: min:%ld max:%ld\n\r",minpos1, maxpos1);
  kmot_SearchLimits(motor2, 10, 3, &minpos2, &maxpos2,100000);
  printf("motor2: min:%ld max:%ld\n\r",minpos2, maxpos2);
  kmot_SearchLimits(motor3, 10, 3, &minpos3, &maxpos3,100000);
  printf("motor3: min:%ld max:%ld\n\r",minpos3, maxpos3);
  kmot_SetBlockedTime(motor0,5);
  kmot_SetBlockedTime(motor2,5);

  /* Network setup - Create a ksock server */
  ksock_server_open(&server, port);
  
  ksock_add_command("movecamera",2,2,koa_net_camera);
  ksock_add_command("movecamera2",2,2,koa_net_camera2);
  ksock_add_command("moveboth",2,2,koa_net_bothcam);
  ksock_add_command("initcamera",0,0,koa_net_init);
  list_command();

  /* Network connection handling */
  clntSocket = ksock_next_connection(&server);
  for(;;)
  {
    rc = ksock_get_command(clntSocket, echoBuffer, RCVBUFSIZE);

    if(rc>0)
    {
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
  }

  close(clntSocket);    /* Close client socket */

  return 0;
}

