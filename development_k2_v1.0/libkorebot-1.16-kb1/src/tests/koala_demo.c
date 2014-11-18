/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2005/07/03 07:38:15 $
 * $Revision: 1.7 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/tests/koala_demo.c,v 1.7 2005/07/03 07:38:15 pbureau Exp $
 */

/*--------------------------------------------------------------------*/
/*! 
 * \file   koala_demo.c koala demo program (server side)
 *
 * \brief
 * 	   Demo program to remote control the koala robot from 
 * 	   a wireless ethernet link, using a koremotor to control 2 pantilt
 * 	   cameras, and a koresound to play sound effects.
 *
 * \author   Pierre Bureau (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 */


#include <korebot/korebot.h>

#define RCVBUFSIZE 32   /* Size of receive buffer */

knet_dev_t *koala;
int right = 0;
int left  = 0;
int left_limit_p  = 100;
int left_limit_n  = -100;
int right_limit_p = 100;
int right_limit_n = -100;

knet_dev_t *motor0, *motor1, *motor2, *motor3;
int32_t minpos0,maxpos0,minpos1,maxpos1;
int32_t minpos2,maxpos2,minpos3,maxpos3;
  
/* Sound files list */
const char * soundlist[] = {
  "sound1.wav",
  "sound2.wav",
  "sound3.wav",
  "sound4.wav",
  "sound5.wav"
};
const unsigned int soundlist_size = sizeof(soundlist)/sizeof(char*);

/* Sound queue */
typedef struct koasound_s {
  int sound;
  struct koasound_s * next;
}koasound_t;

koasound_t * lastsound    = NULL;
koasound_t * currentsound = NULL;
pthread_mutex_t soundlock;

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
  kmot_SearchLimits(motor0, 1, 3, &minpos0, &maxpos0,100000);
  printf("motor0: min:%ld max:%ld\n\r",minpos0, maxpos0);
  kmot_SearchLimits(motor1, 5, 3, &minpos1, &maxpos1,100000);
  printf("motor1: min:%ld max:%ld\n\r",minpos1, maxpos1);
  kmot_SearchLimits(motor2, 1, 3, &minpos2, &maxpos2,100000);
  printf("motor2: min:%ld max:%ld\n\r",minpos2, maxpos2);
  kmot_SearchLimits(motor3, 5, 3, &minpos3, &maxpos3,100000);
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
#if 0
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

int koa_net_stop(int argc, char * argv[])
{
  koa_setSpeed(koala,0,0);

  return 0;
}

int koa_net_setspeed(int argc, char * argv[])
{
  int sensor_table[16];
  int sum;
  int newright,newleft;
  int i;

  right = atoi(argv[1]);
  left  = atoi(argv[2]);

  newright = atoi(argv[1]);
  newleft  = atoi(argv[2]);

  if(newleft > 0)
  {
    /* Compute the left positive limit */
    if(left_limit_p<newleft)
      newleft = left_limit_p;
  }
  else
    if(newleft < 0)
    {
      /* Compute the left negative limit */
      if(left_limit_n>newleft)
	newleft = left_limit_n;
    }

  if(newright > 0)
  {
    /* Compute the right positive limit */
    if(right_limit_p<newright)
      newright = right_limit_p;
  }
  else
    if(newright < 0)
    {
      /* Compute the right negative limit */
      if(right_limit_n>newright)
	newright = right_limit_n;
    }

  koa_setSpeed(koala,newright,newleft);

  for(i=0;i<argc;i++)
    printf("%s ",argv[i]);
  printf("(%d,%d)->(%d,%d)\r\n",right,left,newright,newleft);

  return 0;
}

int net_exit(int argc, char * argv[])
{
  exit(0);
}

/****************************************************************/
/*! Playing sound using the KoreMedia - generate a sound event
 */
int koa_net_playsound(int argc, char * argv[])
{
  unsigned sound = atoi(argv[1]);
  koasound_t * newsound;

  if(sound >= soundlist_size)
    return 1;
    
  newsound = kb_alloc(sizeof(koasound_t));
  newsound->sound = sound;
  newsound->next  = NULL;
  
  /* update the sound queue */
  pthread_mutex_lock(&soundlock);
  if(lastsound != NULL)
    lastsound->next = newsound;
  else
    currentsound = newsound;
  lastsound = newsound;
  pthread_mutex_unlock(&soundlock);

  printf("Addsound %d\r\n", sound);

  return 0;
}

/****************************************************************/
/*! Koala Sound playing task
 */
#define soundbufsize 2048
void * koa_sound_task(void * arg)
{
   WAV wav;
   int rc;
   snd_t snd;
   short buf[soundbufsize];
   int nextsound;
   koasound_t * previous;

   /* Init sound library */
   kb_snd_init(&snd , "/dev/sound/dsp", "/dev/sound/mixer");

   if ( kb_snd_open(&snd ) < 0 ) {
     printf("cannot open sound devices\r\n");
     return NULL;
   }

   kb_wav_init( &wav );
   
   /* Playing sounds from the queue */
   for(;;)
     if(currentsound != NULL)
     {
       nextsound = currentsound->sound;
       printf("Playsound %d\r\n", nextsound);

       if ((rc = kb_wav_open( &wav , soundlist[nextsound], 1 )) == -1 ) {
	 printf("cannot open %s\r\n",soundlist[nextsound]);
	 continue;
       }

       if ( wav.fmt.bit_per_sample != 16  && wav.fmt.nb_channel != 2 ) {
	 printf("wrong bit_per_sample or nb_channel in wav file\r\n");
	 continue;
       }

       /* Set Sample Rate */
       if ( kb_snd_setSampleRate( &snd, wav.fmt.sample_rate ) == -1 ) {
	 printf("unable to set sample rate\r\n");
	 continue;
       }

       /* Sound playing loop */
       for (;;) {
	 rc = kb_wav_read_data( &wav , buf , soundbufsize);
	 if ( rc <= 0 ) {
	   if ( rc < 0 )
	     printf( "i/o error in reading file '%s' !" , soundlist[nextsound]);
	   break;
	 }

	 do {
	   rc = kb_snd_play( &snd , buf , soundbufsize);
	   if ( rc < 0 ) {
	     if ( errno != EAGAIN ) {
	       printf( "error in playing sample errno=%d", errno );
	       break;
	     }
	     continue;
	   }
	 }
	 while (0);
       }

       kb_wav_close( &wav );

       /* update the sound list */
       pthread_mutex_lock(&soundlock);
       previous = currentsound;
       currentsound = currentsound->next; 
       if(currentsound == NULL)
	 lastsound = NULL;
       pthread_mutex_unlock(&soundlock);

       if(previous != NULL)
	 kb_free(previous);
     }
}

/****************************************************************/
/*! Koala Sensors reading and speed limiting task
 */
void * koa_sensor_task(void * arg)
{
  int sensor_table[16];
  int sum;
  int newright,newleft;

  for(;;)
  {
    newright = right;
    newleft  = left;

    koa_readProximity(koala,sensor_table);

    if(newright > 0) 
    {
      /* Compute the right positive limit */
      sum = sensor_table[0]  + 2*sensor_table[8]  + 3*sensor_table[9] +
	sensor_table[12] + 2*sensor_table[11] + 3*sensor_table[10];
      sum = (sum / 12) - 50;
      if(sum<0)
	sum = 0;
      sum = (sum*100) / 250;
      if(sum>100)
	sum = 100;
      sum = 100 - sum;
      right_limit_p = sum;
      if(sum<newright)
	newright = sum;
    }
    else
    {
      /* Compute the right negative limit */
      sum = sensor_table[13] + 2*sensor_table[14] + sensor_table[15];
      sum = (sum / 4) - 50;
      if(sum<0)
	sum = 0;
      sum = (sum*100) / 250;
      if(sum>100)
	sum = 100;
      sum = -(100 - sum);
      right_limit_n = sum;
      if(sum>newright)
	newright = sum;
    } 
    
    if(newleft > 0)
    {
      /* Compute the left positive limit */
      sum = sensor_table[8] + 2*sensor_table[0] + 3*sensor_table[1] +
	sensor_table[4] + 2*sensor_table[3] + 3*sensor_table[2];
      sum = (sum / 12) - 50;
      if(sum<0)
	sum = 0;
      sum = (sum*100) / 250;
      if(sum>100)
	sum = 100;
      sum = 100 - sum;
      left_limit_p = sum;
      if(sum<newleft)
	newleft = sum;
    }
    else
    {
      /* Compute the left negative limit */
      sum = sensor_table[5] + 2*sensor_table[6] + sensor_table[7];
      sum = (sum / 4) - 50;
      if(sum<0)
	sum = 0;
      sum = (sum*100) / 250;
      if(sum>100)
	sum = 100;
      sum = -(100 - sum);
      left_limit_n = sum;
      if(sum>newleft)
	newleft = sum;
    } 
    koa_setSpeed(koala,newright,newleft);
  }
}

/****************************************************************/
/*! Pan-tilt motor initialization
 */
void InitMotor(knet_dev_t * mot)
{
    kmot_SetMode(mot,0);
    kmot_SetSampleTime(mot,6);
    kmot_SetMargin(mot,50);
    kmot_SetOptions(mot,0,kMotSWOptWindup|kMotSWOptStopMotorBlk);     
    kmot_ResetError(mot);

    kmot_ConfigurePID(mot,kMotRegSpeed,500,250,30);
    kmot_ConfigurePID(mot,kMotRegPos,70,50,5);
    kmot_SetSpeedProfile(mot,20,100);

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

  printf("K-Team Koala Demo Program - Server side\r\n");

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

  /* Open a device for the koala robot */
  koala = knet_open( "Koala:Robot", KNET_BUS_ANY, 0 , NULL );
  if(!koala)
  {
    printf("Open Koala device failed\r\n");
    return 1;
  }
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

  
  /* Koala Comm test */
  koa_getOSVersion(koala,&a,&b,&c,&d);
  printf("OS revision %d.%d  protocol %d.%d\r\n",a,b,c,d);
  koa_setSpeed(koala,0,0);
  /* KoreMotor Comm test */
  kmot_GetFWVersion( motor1, &rc);
  printf("Motor 1 Firmware v%u.%u\n" , KMOT_VERSION(rc) , KMOT_REVISION(rc));

  /* Intialize motor controller */
  InitMotor(motor0);
  InitMotor(motor1);
  InitMotor(motor2);
  InitMotor(motor3);
  kmot_SearchLimits(motor0, 1, 3, &minpos0, &maxpos0,100000);
  printf("motor0: min:%ld max:%ld\n\r",minpos0, maxpos0);
  kmot_SearchLimits(motor1, 5, 3, &minpos1, &maxpos1,100000);
  printf("motor1: min:%ld max:%ld\n\r",minpos1, maxpos1);
  kmot_SearchLimits(motor2, 1, 3, &minpos2, &maxpos2,100000);
  printf("motor2: min:%ld max:%ld\n\r",minpos2, maxpos2);
  kmot_SearchLimits(motor3, 5, 3, &minpos3, &maxpos3,100000);
  printf("motor3: min:%ld max:%ld\n\r",minpos3, maxpos3);
  kmot_SetBlockedTime(motor0,5);
  kmot_SetBlockedTime(motor2,5);

  /* Network setup - Create a ksock server */
  ksock_server_open(&server, port);
  
  ksock_add_command("setspeed",2,2,koa_net_setspeed);
  ksock_add_command("stop",0,0,koa_net_stop);
  ksock_add_command("movecamera",2,2,koa_net_camera);
  ksock_add_command("movecamera2",2,2,koa_net_camera2);
  ksock_add_command("moveboth",2,2,koa_net_bothcam);
  ksock_add_command("playsound",1,1,koa_net_playsound);
  ksock_add_command("initcamera",0,0,koa_net_init);
  list_command();

  /* Task creation */
  rc = pthread_create(&sensor_task, NULL, koa_sensor_task, NULL);
  if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }
  rc = pthread_create(&sound_task, NULL, koa_sound_task, NULL);
  if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

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

