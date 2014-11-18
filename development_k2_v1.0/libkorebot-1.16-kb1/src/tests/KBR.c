/*--------------------------------------------------------------------
 * kmot_test.c - KoreBot Library - KoreMotor Test
 *--------------------------------------------------------------------
 * $Id: kmotLE_test.c,v 1.1 2006/10/27 08:41:10 flambercy Exp $
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Date: 2006/10/27 08:41:10 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#include <signal.h>
#include <korebot/korebot.h>

/*! 
 * \file   kmot_test.c KoreMotor utility program
 *
 * \brief 
 *         kmot_test provides a small utility program to communicate with the
 *         the KoreMotor and send commands to the motor controllers. Use the              
 *         help command to get a list of available controls.
 *         
 *
 * \author   Pierre Bureau (K-Team SA)
 * \author   Cédric Gaudin (K-Team SA)                               
 *
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

static int quitReq = 0;
static int stopReq = 0;
static int currentCommand = 0;

static FILE * logfile;
static knet_dev_t * motor;
static pthread_t log_task;

/*!
 * Make sure the program terminate properly on a ctrl-c
 */
static void ctrlc_handler( int sig ) 
{
  stopReq = 1;
}

/*--------------------------------------------------------------------*/
/*! Quit the program.
 */
int quit( int argc , char * argv[] , void * data) 
{
  quitReq = 1;
}

/*--------------------------------------------------------------------*/
/*! Stop the current motor (set mode to stop motor mode).
 */
int stop( int argc , char * argv[] , void * data) 
{
  kmot_SetMode( motor , kMotModeStopMotor );
}


/*--------------------------------------------------------------------*/
/*! Set a new open loop command for the current motor. The
 * controller must be properly initialized.
 * syntax: openloop <command>
 */
int right( int argc , char * argv[] , void * data)
{
  int pwm = atoi(argv[1]);
  int current = atoi(argv[2]);
  char i = 0;

  kmot_SetPoint( motor , kMotRegOpenLoop, pwm);
  i = 0;
  while(i < 10){
    while( kmot_GetMeasure( motor , kMotMesCurrent ) < current) 
      i++;
  }
  kmot_SetMode( motor , kMotModeStopMotor );
 
 return 0;
}


/*--------------------------------------------------------------------*/
/*! Set a new open loop command for the current motor. The
 * controller must be properly initialized.
 * syntax: openloop <command>
 */
int left( int argc , char * argv[] , void * data)
{
  int pwm = -atoi(argv[1]);
  int current = atoi(argv[2]);
  char i = 0;

  kmot_SetPoint( motor , kMotRegOpenLoop, pwm);
  i = 0;
  while(i < 10){
    while( kmot_GetMeasure( motor , kMotMesCurrent ) < current)
      i++;
  }
  kmot_SetMode( motor , kMotModeStopMotor );
 
  return 0;
}
int right3( int argc , char * argv[] , void * data)
{

  kmot_SetPoint( motor , kMotRegOpenLoop, 2048);
  usleep(50000);
  kmot_SetMode( motor , kMotModeStopMotor );

 return 0;
}


/*--------------------------------------------------------------------*/
/*! Set a new open loop command for the current motor. The
 * controller must be properly initialized.
 * syntax: openloop <command>
 */
int left3( int argc , char * argv[] , void * data)
{


  kmot_SetPoint( motor , kMotRegOpenLoop, -2048);
  usleep(50000);
  kmot_SetMode( motor , kMotModeStopMotor );

  return 0;
}

int right2( int argc , char * argv[] , void * data)
{

  kmot_SetPoint( motor , kMotRegOpenLoop, 2048);
  usleep(500000);
  kmot_SetMode( motor , kMotModeStopMotor );

 return 0;
}


/*--------------------------------------------------------------------*/
/*! Set a new open loop command for the current motor. The
 * controller must be properly initialized.
 * syntax: openloop <command>
 */
int left2( int argc , char * argv[] , void * data)
{
  

  kmot_SetPoint( motor , kMotRegOpenLoop, -2048);
  usleep(500000);
  kmot_SetMode( motor , kMotModeStopMotor );

  return 0;
}



/*--------------------------------------------------------------------*/
/*! Set a new open loop command for the current motor. The
 * controller must be properly initialized.
 * syntax: openloop <command>
 */
int test( int argc , char * argv[] , void * data)
{
  int loop = atoi(argv[1]);

  currentCommand = loop;
  kmot_SetPoint( motor , kMotRegOpenLoop, loop);
  return 0;
}

int help( int argc , char * argv[] , void * data);
/*--------------------------------------------------------------------*/
/*! The command table contains:
 * command name : min number of args : max number of args : the function to call
 */
static kb_command_t cmds[] = {
  { "quit"            , 0 , 0 , quit } ,
  { "1"		      , 2 , 2 , right } ,
  { "2"               , 2 , 2 , left } ,
  { "11"               , 0 , 0 , right2 } ,
  { "12"               , 0 , 0 , left2 } ,
  { "21"               , 0 , 0 , right3 } ,
  { "22"               , 0 , 0 , left3 } ,
  { "3"               , 0 , 0 , test } ,
  { "help"            , 0 , 0 ,  help } ,
  { NULL              , 0 , 0 , NULL }
};

/*--------------------------------------------------------------------*/
/*! Display a list of available commands.
 */
int help( int argc , char * argv[] , void * data) 
{
  kb_command_t * scan = cmds;
  while(scan->name != NULL)
  {
    printf("%s\r\n",scan->name);
    scan++;
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Main program to process the command line. 
 *
 */
static char buf[1024];

int main( int argc , char * argv[] )
{
  int rc;
  char * name;

  if ( argc < 2 ) {
    kb_msg("usage: kmot_test <motor_name>\n");
    kb_msg("motor_name: KoreMotorLE:PriMotor1  (0x1F)\n");
    kb_msg("            KoreMotorLE:PriMotor2  (0x20)\n");
    kb_msg("            KoreMotorLE:PriMotor3  (0x21)\n");
    kb_msg("            KoreMotorLE:PriMotor4  (0x22)\n");
    kb_msg("            KoreMotorLE:AltMotor1  (0x23)\n");
    kb_msg("            KoreMotorLE:AltMotor2  (0x24)\n");
    kb_msg("            KoreMotorLE:AltMotor3  (0x25)\n");
    kb_msg("            KoreMotorLE:AltMotor4  (0x26)\n");
    return 0;
  }

  name = argv[1];

  /* Set the libkorebot debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if ((rc = kb_init( argc , argv )) < 0 )
    return 1;

  signal( SIGINT , ctrlc_handler );

  motor = knet_open( name , KNET_BUS_I2C , 0 , NULL );

  if ( motor ) {
    unsigned int ver , rev;
    unsigned char status , error;
    int min, max;

    kmot_GetFWVersion( motor , &ver );

    printf("Motor %s Firmware v%u.%u\n" , name , 
	KMOT_VERSION(ver) , KMOT_REVISION(ver) );

  kmot_SetMode( motor , kMotModeIdle );
  kmot_SetSampleTime( motor , 1550);
  kmot_SetMargin( motor , 6 );
  kmot_SetOptions( motor , 0x0 , kMotSWOptWindup | kMotSWOptStopMotorBlk );

  kmot_ResetError( motor );
  kmot_SetBlockedTime( motor , 10 );
  kmot_SetLimits( motor , kMotRegCurrent , 0 , 500 );
  kmot_SetLimits( motor , kMotRegPos , -10000 , 10000 );

  /* PID  */
  kmot_ConfigurePID( motor , kMotRegSpeed , 768 , 0 , 18 );
  kmot_ConfigurePID(motor,kMotRegPos,70,50,10);

  kmot_SetSpeedProfile(motor,30,10);


    while (!quitReq) {
      printf("\n> ");

      if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) {
	buf[strlen(buf)-1] = '\0';
	kb_parse_command( buf , cmds , NULL);
      }
    }

    knet_close( motor );
  } else {
    printf("Cannot open KoreMotor device\r\n");
  }

  return 0;
}


extern void kmot_GetSpeedMultiplier( knet_dev_t * dev ,  unsigned short *mult );

extern void kmot_SetSpeedMultiplier( knet_dev_t * dev , int mode );

