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

#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <korebot/korebot.h>
#include <string.h>
#include <math.h>

#define MAX_SENSOR_NUMBER 16
#define RANGE (1024 / 2)

#define TIME_STEP 64
#define RESET_BUT 65 

/* Activate the debug mode */
#undef DEBUG

/* Global Handle for the Koala */
knet_dev_t * BTComm;
knet_dev_t *koala;
int fd;
static int quitReq = 0;
#define BAUDRATE B115200

/*
 * You should declare here DeviceTag global variables for storing pointers
 * robot devices. You should use the static keyword when declaring these
 * variables.
 */
static int sensors[MAX_SENSOR_NUMBER];

/*static float matrix[16][2] = 
   { {17, -1}, {8, -2}, {4, -3}, {14, -2}, {0, 0}, {0, 0}, {0, 0},
   {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {-3, 13}, {-3, 5},
   {-2, 10}, {-1, 15} };*/
static float matrix[16][2] =
   { {-1, 15}, {-1, 5}, {-3, 4}, {-7, 17}, {0, 0}, {0, 0}, {0, 0},
   {-10, -10}, {17, -1}, {5, -1}, {4,-3 }, {17, -7}, {0, 0}, {0, 0},
   {0, 0}, {-10, -10} };



static int sensor_number;
static int range;
static int phase;
static int position;
static int goal;
static int oldEncoder[2];
static float X,Y,Theta;
static int Start;


typedef struct {

  pthread_t       thread;
  pthread_mutex_t cmdWaitLock;
  pthread_cond_t  cmdWait;
  int             busy;
  int             done;
  int             state;
}
koala_t;
/* declaration of the structure */
koala_t BTkoala_struct ;

/*
 * This is the reset function called once upon initialization of the robot.  
 */
static void init(void) 
{

  int i, j;


  range = RANGE;
  sensor_number = 16;
#ifdef DEBUG
  printf("The koala robot is reset, it uses 16 sensors.\n\r");
#endif
  /* Stop at start */
  phase = 2;
  /* the koala is on the goal but not in the correct direction */
  position = 0;
  goal = 0;
  X=0; Y=0; Theta=0.0;
  oldEncoder[0]=0;oldEncoder[1]=0;
  koa_setPositionCounter(koala, 0, 0);
  //differential_wheels_set_encoders(0, 0);
  Start=1;
  
  return;
}



int cmd_received = 0;

/* Breitenberg management task */
void * thread(void * arg)
{
  koala_t *BT_koala = (koala_t *)arg;

  int i, j, button;
  float speed[2];
  int leftspeed, rightspeed;
  unsigned short sensors_value[MAX_SENSOR_NUMBER];
  float encoder_temp[2];
  int encoder[2];   // encoders in mmm
  int dSl, dSr;
  char sensors_val[6], pos_val[8];
  long left,right;

for(;;)
{

/*
 * This is the main control loop function, it is called repeatedly by the KoreBot
 */

  /*
   * read encoders and calculate odometry, convert the value in mm                     
   */
  koa_readPosition(koala, encoder);
  encoder[0] = encoder[0] * 0.045;
  encoder[1] = encoder[1] * 0.045;
#ifdef DEBUG 
  printf("encoder = %d, %d\n\r", encoder[0], encoder[1]);
#endif

  // Calculate distances for both wheels
  dSl=encoder[0]-oldEncoder[0];					
  dSr=encoder[1]-oldEncoder[1];
  //dSm=(dSr+dSl)/2;
  // Integrate robot position
  X+= (dSr+dSl)*cos(Theta)/2;
  Y+= (dSr+dSl)*sin(Theta)/2;
  Theta+=(dSr-dSl) /320.0;  //Theta  (Drobot=2785mm) but 320mm is the best for the koala 151
//  Theta+=(dSr-dSl) /300.0;  //Theta  (Drobot=2785mm) use for koala 104

  //Theta=set2range(Theta);
  Theta=fmod(Theta,2*M_PI);
  if (Theta > M_PI)
    Theta=Theta-2*M_PI;
  if (Theta < -M_PI)
    Theta=Theta+2*M_PI;

  //Keep track of previous wheel positions
  oldEncoder[0]=encoder[0];
  oldEncoder[1]=encoder[1];
  
  /* 
   * Read the promity sensors
   */
  koa_readProximity(koala, sensors);
  /* Check if the Reset button is pushed (0 = pushed)*/
  if( kb_gpio_get(RESET_BUT))
  {
    /*
     * Braitenberg obstacle avoidance 
    */
    if (phase==0) { 
      position = 0;		// Reset the position flag
      /*
       * The Braitenberg algorithm is really simple, it simply computes the
       * speed of each wheel by summing the value of each sensor multiplied by
       * its corresponding weight. That is why each sensor must have a weight 
       * for each wheel.
       */
      for (i = 0; i < 2; i++) {
        speed[i] = 20;

        for (j = 0; j < sensor_number; j++) {

          /* 
          * We need to recenter the value of the sensor to be able to get
          * negative values too. This will allow the wheels to go 
          * backward too.
          */
          speed[i] += matrix[j][i] *
              (1 - ((float) sensors[j] / range));
        }
      }
    
      if ((sensors[0] + sensors[1] + sensors[2]+ sensors[3] +
          sensors[8] + sensors[9] + sensors[10] + sensors[11] )>3500) { // if obstacle all around
        speed[0] = 0;
        speed[1] = 0;
#ifdef DEBUG
        printf("STOP phase. sens:%d,%d,%d,%d,%d,%d\n",
          sensors[0],sensors[1],sensors[2],
          sensors[8],sensors[9],sensors[10]);
        printf("position: %d,%d,%d\n", (int)X, (int)Y, (int)((Theta/3.14)*180));
#endif
	phase=1; // Turn around
        if (X<200 && X>-200 && Y<200 && Y>-200) { // close to start point
#ifdef DEBUG
          write(fd,"Back at START\n", 14);
#endif
          Start=1;
          // need to recalibrate a bit the odometry!!!
          //if (X>5) X-=5;
          //if (X<-5) X+=5;
          X=X*0.5;
          //if (Y>5) Y-=5;
          //if (Y<-5) Y+=5;
          Y=Y*0.5;
          //if (Theta>0.1) Theta-=0.1;
          //if (Theta<-0.1) Theta+=0.1;
          Theta=Theta*0.5;
          /* Stop at goal if asked */
	  if(goal)
 	  {
	     phase = 2;  // Stop
 	     goal = 0;
	     position = 1; 
   	  }
        }
      }
    }
    /*
     * Turn 180 Deg
    */ 
    else if (phase == 1) {  // to be changed to turn 180 deg
      position = 0;  	// Reset the position flag
      if (Start) {
        speed[0] = 40;speed[1] = -40; // clockwise
      }   
      else {
        speed[0] = -40;speed[1] = 40; // anticlockwise
      }
      /* Turn until there no obstacle   */
      if ((sensors[0] + sensors[1] + sensors[2] +
        sensors[8] + sensors[9] + sensors[10])<1400)
      {
	phase = 0;
	Start = 0;
	speed[0] = 0;
    	speed[1] = 0;
      }
    }
    /*
     * use for stop the Koala
    */
    else if (phase==2) {
      Start=0;
      speed[0] = 0;speed[1] = 0;
    }

    /* 
     * Wait 2sec after a reset 
    */
    else if(phase<63)
    {
      phase++;
      speed[0] = 0;
      speed[1] = 0;
    }
    /*
     * Restart after a reset 
    */
    else if(phase >= 63) 
      phase = 0;
  }
  /* 
   * Button pushed, then reset the application 
  */
  else
  {
    speed[0] = 0;
    speed[1] = 0; 
    goal = 0;
    position = 0;
    phase = 30; 
    X=0; Y=0; Theta=0.0;
    oldEncoder[0]=0;oldEncoder[1]=0;
    koa_setPositionCounter(koala, 0, 0);
    Start=1;
  }
  /* Set the motor speeds */
  leftspeed = speed[0];
  rightspeed = speed[1];
#ifdef DEBUG
  printf("left %d, right %d\n\r", leftspeed, rightspeed);

  write(fd,"\n\rPos: ",6);
  sprintf(pos_val, ",%6d", (int)X );
  write(fd,pos_val,7);
  sprintf(pos_val, ",%6d", (int)Y );
  write(fd,pos_val,7);
  
  sprintf(pos_val, ",%6d", (int)(Theta*180/M_PI) );
  write(fd,pos_val,7);

  write(fd,"\n\r",2);
#endif

  koa_setSpeed(koala, leftspeed, rightspeed);


//  return TIME_STEP; /* this is the time step value, in milliseconds. */

  usleep(64000);  // 64ms
  }

}
 



/*--------------------------------------------------------------------*/
/*! Right Arm create
 */
int thread_create( koala_t *koala )
{
  int i;

  pthread_mutex_init( &koala->cmdWaitLock , NULL );
  pthread_cond_init( &koala->cmdWait , NULL );

  koala->busy = 0;
  koala->done = 0;
  koala->state = 0;

  if (pthread_create( &koala->thread , NULL , thread , koala ))
    return -1;

  return 0;
}




/*--------------------------------------------------------------------*/
/*! Quit the program.
 */
int quit( int argc , char * argv[] , void * data)
{
  write(fd,"q\n\r", 3);
  koa_setSpeed(koala,0,0);
  quitReq = 1;
  return 0;
}


/*--------------------------------------------------------------------*/
/*! Start the program.
 */
int start( int argc , char * argv[] , void * data)
{
  phase = 0;
  goal = 0;
  write(fd,"r\n\r", 3);
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Stop the Koala.
 */
int stop( int argc , char * argv[] , void * data)
{
  phase = 2;
  write(fd,"s\n\r", 3);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! Reset the program.
 */
int reset( int argc , char * argv[] , void * data)
{
  /* Wait some time before started */
  phase = 50;
  X=0; Y=0; Theta=0.0;
  oldEncoder[0]=0;oldEncoder[1]=0;
  koa_setPositionCounter(koala, 0, 0);
  Start=1;
  /* the koala is on the goal but not in the correct direction */
  position = 0;
  goal = 0;
  write(fd,"i\n\r", 3);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! Go to the goal and stop.
 */
int gotogoal( int argc , char * argv[] , void * data)
{
  phase = 0;
  goal = 1;
  write(fd,"g\n\r", 3);
  /* if in goal position, then turn and do a turn */
  if(position) 
  {
    phase = 1;
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Return if the Koala is on the goal or not (1 yes, 0 no).
 */
int askposition( int argc , char * argv[] , void * data)
{
  write(fd,"p,", 2);
  if(position) 
    write(fd,"1\r\n",3);
  else 
    write(fd,"0\r\n",3);
  return 0;
}

 
void handle_kill(int arg)
{
  fprintf(stderr, "arg... killed\r\n");

  koa_setSpeed(koala,0,0);

  exit(1);
}



static kb_command_t cmds[] = {
  { "R"  		, 0 , 0 , start },
  { "Q"		        , 0 , 0 , quit },
  { "S"              	, 0 , 0 , stop },
  { "I" 	        , 0 , 0 , reset },
  { "G"			, 0 , 0 , gotogoal },
  { "P"			, 0 , 0 , askposition },
  { NULL                , 0 , 0 , NULL }
};


/*--------------------------------------------------------------------*/
/*! Display a list of available commands.
 */
int help( int argc , char * argv[] , void * data)
{
  kb_command_t * scan = cmds;
  while(scan->name != NULL)
  {
    write(fd,"TEST\n\r", 6);
    //knet_printf(BTComm, "%s\r\n",scan->name);
    scan++;
  }
  return 0;
}



int main( int argc , char * argv[] )
{
  static char buf[1024];
  char command[40];
  int rc;
  unsigned char a,b,c,d;
  int len, n;
  int res;
  struct termios oldtio,newtio;


  /* Set signal handler */
  signal(SIGINT, handle_kill);

  /* Global libkorebot init */
  if(kb_init( argc , argv ) < 0 ){
    perror("global libkorebot init Problem \n\r");
    return -1;
  }
 
 
  /* Set the libkorebot debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  printf("Koala GoAutomation Program\r\n");

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


  /* Configure BT serial port */
  fd = open("/dev/tts/2", O_RDWR | O_NOCTTY | O_NDELAY);  
  if ( fd < 0 )
    {
      printf( "Unable to open /dev/tts/2 !" );
      return -1;
    }

  fcntl(fd, F_SETFL, 0);
  tcgetattr(fd,&oldtio); /* sauvegarde de la configuration courante */
  bzero(&newtio, sizeof(newtio)); /* on initialise la structure à zéro */

/* 
  BAUDRATE: Affecte la vitesse. vous pouvez également utiliser cfsetispeed
            et cfsetospeed.
  CRTSCTS : contrôle de flux matériel (uniquement utilisé si le câble a
            les lignes nécessaires. Voir la section 7 du Serial-HOWTO).
  CS8     : 8n1 (8 bits,sans parité, 1 bit d'arrêt)
  CLOCAL  : connexion locale, pas de contrôle par le modem
  CREAD   : permet la réception des caractères
*/
 newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
 
/*
  IGNPAR  : ignore les octets ayant une erreur de parité.
  ICRNL   : transforme CR en NL (sinon un CR de l'autre côté de la ligne
            ne terminera pas l'entrée).
  sinon, utiliser l'entrée sans traitement (device en mode raw).
*/
 newtio.c_iflag = IGNPAR | ICRNL;
 
/*
 Sortie sans traitement (raw).
*/
 newtio.c_oflag = 0;
 
/*
  ICANON  : active l'entrée en mode canonique
  désactive toute fonctionnalité d'echo, et n'envoit pas de signal au
  programme appelant.
*/
 newtio.c_lflag = ICANON;
 
/* 
  initialise les caractères de contrôle.
  les valeurs par défaut peuvent être trouvées dans
  /usr/include/termios.h, et sont données dans les commentaires.
  Elles sont inutiles ici.
*/
 newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
 newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
 newtio.c_cc[VERASE]   = 0;     /* del */
 newtio.c_cc[VKILL]    = 0;     /* @ */
 newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
 newtio.c_cc[VTIME]    = 0;     /* compteur inter-caractère non utilisé */
 newtio.c_cc[VMIN]     = 1;     /* read bloquant jusqu'à l'arrivée d'1 caractère */
 newtio.c_cc[VSWTC]    = 0;     /* '\0' */
 newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
 newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
 newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
 newtio.c_cc[VEOL]     = 0;     /* '\0' */
 newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
 newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
 newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
 newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
 newtio.c_cc[VEOL2]    = 0;     /* '\0' */

/* 
  à présent, on vide la ligne du modem, et on active la configuration
  pour le port
*/
 tcflush(fd, TCIFLUSH);
 tcsetattr(fd,TCSANOW,&newtio);

  /* Init gpio module */
  kb_gpio_init();

  /* Set gpio 65 to IO mode */
  kb_gpio_function(RESET_BUT ,0);

  /* Set gpio 65 to input */
  kb_gpio_dir( RESET_BUT ,IN);

 /*
  * Reset the odometry of the Koala
  */
 init();

 /* thread creation */
 thread_create(&BTkoala_struct);

 while(!quitReq){
      res = read(fd,buf,255);
      buf[res-1]='\0';
//      write(fd, buf,res);
//      write(fd, "\n\r",2);
      if (kb_parse_command(buf,cmds,NULL)!=0){
        write(fd,"ERROR\n\r", 7);
      }

    }
 /* restaure les anciens paramètres du port */
 tcsetattr(fd,TCSANOW,&oldtio);
 close(fd);


}
