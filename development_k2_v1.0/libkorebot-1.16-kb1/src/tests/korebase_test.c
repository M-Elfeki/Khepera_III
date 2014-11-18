/*--------------------------------------------------------------------
 * gpio_test.c - KoreBot Library - GPIO Test
 *--------------------------------------------------------------------
 * $Id: korebase_test.c,v 1.1 2006/08/18 12:31:15 flambercy Exp $
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Date: 2006/08/18 12:31:15 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/
#include <signal.h>
#include <korebot/korebot.h>

static int quitReq = 0;

/*--------------------------------------------------------------------*/
/*!
 * Make sure the program terminate properly on a ctrl-c
 */
static void ctrlc_handler( int sig ) 
{
  quitReq = 1;
}

/*--------------------------------------------------------------------*/
/*! Quit the program.
 */
int quit( int argc , char * argv[] , void * data) 
{
  quitReq = 1;
}





  
/*--------------------------------------------------------------------*/
/*! Configure the given digital IO 
 *  The first argument is the IO number
 *  The second argument is:
 *    0 to set as input
 *    1 to set as output
 */
int configio(int argc, char * argv[], void * data)
{
	/* Set gpio to IO mode */
	kb_gpio_function(atoi(argv[1]),0);

  	/* Set gpio to output */
	if(atoi(argv[2]))
  		kb_gpio_dir(atoi(argv[1]),0);

  	/* Set gpio to input */
	else
  		kb_gpio_dir(atoi(argv[1]),1);		
}

/*--------------------------------------------------------------------*/
/*! Reset the given digital output value
 */
int resetio( int argc, char * argv[], void * data)
{
   kb_gpio_clear(atoi(argv[1]));

  }




/*--------------------------------------------------------------------*/
/*! Set the given digital output value
 */
int setio( int argc, char * argv[], void * data)
{

  kb_gpio_set(atoi(argv[1]));

}

/*--------------------------------------------------------------------*/
/*! Read the given digital input value
 */
int readio( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_gpio_get(atoi(argv[1]));

  if(rc < 0)
    printf("wrong io or not in input mode\r\n");
  else
    printf("read io %d: %d\r\n",atoi(argv[1]),rc);

}

/*--------------------------------------------------------------------*/
/*! Activate the given pwm 
 */
int pwm_on( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_activate(atoi(argv[1]));

  if(rc != 0)
    printf("wrong pwm\r\n");
  else;
    

}

/*--------------------------------------------------------------------*/
/*! Desactivate the given pwm 
 */
int pwm_off( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_desactivate(atoi(argv[1]));

  if(rc != 0)
    printf("wrong pwm\r\n");
  else;
    

}

/*--------------------------------------------------------------------*/
/*! Activate the given pwm 
 */
int pwm_ratio( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_duty(atoi(argv[1]), 0 , atoi(argv[2]));

  if(rc != 0)
    printf("wrong pwm\r\n");
  else;
    

}

/*--------------------------------------------------------------------*/
/*! Activate the given pwm 
 */
int pwm_freq( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_period(atoi(argv[1]), atoi(argv[2]));

  if(rc != 0)
    printf("wrong pwm\r\n");
  else;
   
}

/*--------------------------------------------------------------------*/
/*! Activate the given pwm 
 */
int pwm_config( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_config(atoi(argv[1]), atoi(argv[2]) , atoi(argv[3]));

  if(rc != 0)
    printf("wrong pwm\r\n");
  else;
    

}



int help( int argc , char * argv[] , void * data);
/*--------------------------------------------------------------------*/
/*! The command table contains:
 * command name : min number of args : max number of args : the function to call
 */
static kb_command_t cmds[] = {
  { "quit"            , 0 , 0 , quit } ,
  { "exit"            , 0 , 0 , quit } ,
  { "bye"             , 0 , 0 , quit } ,
  { "readio"          , 1 , 1 , readio },
  { "setio"           , 1 , 1 , setio },
  { "cleario"         , 1 , 1 , resetio },
  { "configio"        , 2 , 2 , configio },
  { "pwm_on"        , 1 , 1 , pwm_on },
  { "pwm_off"        , 1 , 1 , pwm_off },
  { "pwm_ratio"        , 2 , 2 , pwm_ratio},
  { "pwm_freq"        , 2 , 2 , pwm_freq },
  { "pwm_config"        , 3 , 3 , pwm_config },
  { "help"            , 0 , 0 , help } ,
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
  int rc,veri,io;

  /* Set the libkorebot debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  signal( SIGINT , ctrlc_handler );

  printf("K-Team KoreBase Test Program\r\n");
  
  /* Initialize the GPIO and the PWM modules */
  kb_pwm_init();
  
  io = 58;
  while(io < 74)
  {
	/* Set gpio to IO mode */
	kb_gpio_function(io,0);

  	/* Set gpio to output */
  	kb_gpio_dir(io,0);
	io++;
  }
  /* Activate the first pwm and configure it */	
  kb_pwm_activate(0);
  kb_pwm_duty(0, 0 , 20);
  kb_pwm_period(0, 100);
 
  /* Activate the second pwm and configure it */	
  kb_pwm_activate(1);
  kb_pwm_duty(1, 0 , 180);
  kb_pwm_period(1, 240);


  if(rc != 0)
    printf("wrong pwm\r\n");
  else;
    


  
  /* parse commands */
  while (!quitReq) {
    printf("\n> ");
	  

  /* blink the output */
  io = 58;
  while(io < 74)
  {
 	kb_gpio_set(io);
	io+=2;
  }	
  io = 59;
  while(io < 74)
  {
 	kb_gpio_clear(io);
	io+=2;
  } 

  /* clear the current page */
      usleep(20000);
  /* blink the output */
  io = 58;
  while(io < 74)
  {
 	kb_gpio_clear(io);
	io+=2;
  }	
  io = 59;
  while(io < 74)
  {
 	kb_gpio_set(io);
	io+=2;
  } 

  /* clear the current page */
      usleep(20000);

 } 

}
