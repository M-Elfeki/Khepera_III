/*! 
 * \file   khepera3_test.c  Khepera3 test application              
 *
 * \brief 
 *         This is an application that demonstrates the various khepera3 commands. 
 *         
 *        
 * \author   Arnaud Maye (K-Team SA)                               
 *
 * \note     Copyright (C) 2005 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#include <korebot/korebot.h>

#define MAXBUFFERSIZE 100

static int quitReq = 0;
static char buf[1024];


/*! handle to the various khepera3 devices (knet socket, i2c mode)
 */
static knet_dev_t * dsPic;
static knet_dev_t * mot1;
static knet_dev_t * mot2;


/*--------------------------------------------------------------------*/
/*! initMot initializes then configures the motor control
 * unit.
 *
 * \return A value :
 *      - 1 if success
 *      - 0 if any error
 *   
 */
int initMot(knet_dev_t *hDev)
{
  if(hDev)
  {
	  kmot_SetMode( hDev , kMotModeIdle );
	  kmot_SetSampleTime( hDev , 1550 );
	  kmot_SetMargin( hDev , 6 );
	  if(hDev == mot1)
	    kmot_SetOptions( hDev , 0x0 , kMotSWOptWindup | kMotSWOptStopMotorBlk | kMotSWOptDirectionInv);
	  else
	    kmot_SetOptions( hDev , 0x0 , kMotSWOptWindup | kMotSWOptStopMotorBlk );
	  kmot_ResetError( hDev );
	  kmot_SetBlockedTime( hDev , 10 );
	  kmot_SetLimits( hDev , kMotRegCurrent , 0 , 500 );
	  kmot_SetLimits( hDev , kMotRegPos , -10000 , 10000 );
	
	  /* PID  */
	  kmot_ConfigurePID( hDev , kMotRegSpeed , 400 , 0 , 10 );
	  kmot_ConfigurePID( hDev,kMotRegPos,620,3,10);
	  kmot_SetSpeedProfile(hDev,30,10);

	  return 1;
  }
  else
  {
	  printf("initMot error, handle cannot be null\r\n");
	  return 0;
  }
}
													

/*--------------------------------------------------------------------*/
/*! initKH3 initialize various things in the kh3 then
 * sequentialy open the various required handle to the three i2c devices 
 * on the khepera3 using knet_open from the knet.c libkorebot's modules.
 * Finaly, this function initializes then configures the motor control
 * unit.
 *
 * \return A value :
 *      - 1 if success
 *      - 0 if any error
 */
int initKH3( void )
{
  /* This is required */
  kh3_init(); 
  
  /* open various socket and store the handle in their respective pointers */
  dsPic = knet_open( "Khepera3:dsPic" , KNET_BUS_I2C , 0 , NULL );
  mot1  = knet_open( "Khepera3:mot1" , KNET_BUS_I2C , 0 , NULL );
  mot2  = knet_open( "Khepera3:mot2" , KNET_BUS_I2C , 0 , NULL );

  if(dsPic!=0)
  {
    if((mot1!=0)&&(mot2!=0))
    {
      initMot(mot1);
      initMot(mot2);
      return 0;
    }
    else
      return -1;
  }

  return -2;

} 


/*--------------------------------------------------------------------*/
/*! proxIR retrieves proximity ir measure using kb_khepera3.c library.
 */
int proxIR( int argc, char * argv[], void * data)
{
  char Buffer[MAXBUFFERSIZE];
  if(kh3_proximity_ir((char *)Buffer, dsPic))
  	printf("\r\n%c,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%lu\r\n",
	   Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
	   (Buffer[5] | Buffer[6]<<8), (Buffer[7] | Buffer[8]<<8),
	   (Buffer[9] | Buffer[10]<<8), (Buffer[11] | Buffer[12]<<8),
	   (Buffer[13] | Buffer[14]<<8), (Buffer[15] | Buffer[16]<<8),
	   (Buffer[17] | Buffer[18]<<8), 
	   ((Buffer[19] | Buffer[20]<<8) | (Buffer[21] | Buffer[22]<<8)<<16));
  else
  	printf("\r\nn, error...\r\n");
}
		
/*--------------------------------------------------------------------*/
/*! ambIR retrieves ambiant ir measure using kb_khepera3.c library.
 */
int ambIR( int argc, char * argv[], void * data)
{
  char Buffer[MAXBUFFERSIZE];
  if(kh3_ambiant_ir((char *)Buffer, dsPic))
   	printf("\r\n%c,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%lu\r\n",
	   Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
	   (Buffer[5] | Buffer[6]<<8), (Buffer[7] | Buffer[8]<<8),
	   (Buffer[9] | Buffer[10]<<8), (Buffer[11] | Buffer[12]<<8),
	   (Buffer[13] | Buffer[14]<<8), (Buffer[15] | Buffer[16]<<8),
	   (Buffer[17] | Buffer[18]<<8),
	   ((Buffer[19] | Buffer[20]<<8) | (Buffer[21] | Buffer[22]<<8)<<16));
  else
  	printf("\r\no, error...\r\n");
}

/*--------------------------------------------------------------------*/
/*! voltageBAT retrieves the battery voltage using kb_khepera3.c library.
 */
int voltageBAT( int argc, char * argv[], void * data)
{
  char Buffer[MAXBUFFERSIZE];
  short argument;

  argument = atoi(argv[1]);
  if(kh3_battery_voltage((char *)Buffer, argument, dsPic)){
	switch(argument){
		case 0:		/* Read Voltage */
		  printf("\r\n%c,%4.4u,%4.4u => battery voltage = %u.%uV\r\n",
	     	   Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
		   (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
		  break;
		case 1:		/* Read Current */
		  printf("\r\n%c,",Buffer[0]);
		  if ((Buffer[2] & 0x80) != 0){
			Buffer[2] = Buffer[2] & 0x7F;
			printf("-%4.4u,%4.4u => battery current = -%u.%uA\r\n",
		     	   (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
			   (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
		  }
		  else{
		  	printf("%4.4u,%4.4u => battery current = %u.%uA\r\n",
	     	 	 (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
		  	 (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
		  }
		  break;
		case 2:		/* Read Average Current */
		  printf("\r\n,%c,",Buffer[0]);
		  if ((Buffer[2] & 0x80) != 0){
			Buffer[2] = Buffer[2] & 0x7F;
			printf("-%4.4u,%4.4u => battery average current = -%u.%uA\r\n",
		     	   (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
			   (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
		  }
		  else{
		  	printf("%4.4u,%4.4u => battery average current = %u.%uA\r\n",
	     	 	 (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
		  	 (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
		  }
		  break;
		case 3:		/* Read Accumulated Current */
		  printf("\r\n%c,%4.4u,%4.4u => battery accumulared current = %u.%uAh\r\n",
	     	   Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
		   (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
		  break;
		case 4:		/* Read Temperature */
		  printf("\r\n%c,%4.4u,%4.4u => temperature = %u.%u°C\r\n",
	     	   Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
		   (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
		  break;

		default:
		  printf("argument error\r\n");
	}
  }
  else
	  printf("\r\nv, error...\r\n");
}

/*--------------------------------------------------------------------*/
/*! tstampRST resets the relative time stamp using kb_khepera3.c library.
 */
int tstampRST( int argc, char * argv[], void * data)
{
  char Buffer[MAXBUFFERSIZE];

  if(kh3_reset_tstamp((char *)Buffer, dsPic))
  	printf("\r\n%c\r\n", Buffer[0]);
  else
	printf("\r\nz, error...\r\n");
}

/*--------------------------------------------------------------------*/
/*! revisionOS retrieves the khepera3 os version using kb_khepera3.c library.
 */
int revisionOS( int argc, char * argv[], void * data)
{
  char Buffer[MAXBUFFERSIZE];	/* buffer that handle the returned datas from kh3 */

  if(kh3_revision((char *)Buffer, dsPic))
  	printf("\r\n%c,%4.4u,%4.4u => Version = %u, Revision = %u\r\n", 
	   Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
	   (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
  else
	printf("\r\nb, error...\r\n");  
}

/*--------------------------------------------------------------------*/
/*! configureOS configures various parameters inside the kh3 firmware
 *  using kb_khepera3.c library.
 *
 *  \param 1st first param (argv[1]) is the index pointing in configuration array.
 *  \param 2nd the second param (argv[2]) is the value to store where the index point at.
 *  
 */
int configureOS( int argc, char * argv[], void * data)
{
  char Buffer[MAXBUFFERSIZE];	/* buffer that handle the returned datas from kh3 */
  short index;			/* variable that handles index */
  short value;			/* variable that handle value */

  /* Retrive the arguments from the parameter */
  index = atoi(argv[1]);
  value = atoi(argv[2]);
  
  /* Configure */
  if(kh3_configure_os((char *)Buffer, index, value, dsPic))
  	printf("\r\n%c\r\n", Buffer[0]);
  else
	printf("\r\nc, error...\r\n");
}
  
/*--------------------------------------------------------------------*/
/*! measureUS retrieves ultrasonic measure from a given transceiver.
 *
 *  \param 1st first param (argv[1]) is the us number to read from (1 to 5).
 *  
 */
int measureUS( int argc, char * argv[], void * data)
{
  char Buffer[MAXBUFFERSIZE];
  int i;
  short usnoise;	/* Noise on the given adc pin when no us is received */
  short echonbr;	/* Number of echo part of this us measure */
  float usconst = 1.715;
  short usvalue;	/* Variable that handle distances */
  short usampl;		/* Variable than nandle amplitudes */
  short argument; 
 
  argument = atoi(argv[1]);
  if(kh3_measure_us((char *)Buffer, argument, dsPic))
  {
	/* Printout complete frame as received by the khepera3 */
  	printf("\r\n%c", Buffer[0]);
	for(i = 0; i < 22; i++)
		printf(",%4.4u", (Buffer[1+i*2] | Buffer[2+i*2]<<8));
	printf("\r\n");

	/* We guess the echo number ( how many echo has been received from a captor ) */
	echonbr = (Buffer[1] | Buffer[2]<<8);
	printf("echonbr = %d\r\n", echonbr);

	/* Loop as may time it is required */
	if(echonbr < 10)
	{
		for(i = 0; i < echonbr ; i++)
		{
			/* Get the distance measure from one echo */
			usvalue = (Buffer[i*8+3] | Buffer[i*8+4]<<8) * usconst;
			usampl  = (Buffer[i*8+5] | Buffer[i*8+6]<<8);
		
			/* Print out the result */
			printf("Echo %d : Amplitude = %d,  Distance = %dcm.\r\n", i+1, usampl, usvalue);
		}	
	}
	else
		printf("read error\r\n"); 
  }
  else
  	printf("\r\ng, error...");
}

/*--------------------------------------------------------------------*/
/*! motSpeed configures the motor controller speed in the engine control unit.
 *
 *  \param 1st first param (argv[1]) is the motor1 speed.
 *  \param 2nd second param (argv[2]) is the motor2 speed.
 *
 */
int motSpeed( int argc, char *argv[], void *data)
{
  if(mot1!=0 && mot2!=0)
  {
  	kmot_SetPoint( mot1 , kMotRegSpeed , atoi(argv[1]));
  	kmot_SetPoint( mot2 , kMotRegSpeed , atoi(argv[2]));
	return 0;
  }
  else
	return -1;
}

/*--------------------------------------------------------------------*/
/*! motMove configures the motor controller position in the engine control unit.
 *
 *  \param 1st first param (argv[1]) is the motor1 position.
 *  \param 2nd second param (argv[2]) is the motor2 position.
 *
 */
int motMove( int argc, char *argv[], void *data)
{
  if(mot1!=0 && mot2!=0)
  {
	kmot_SetPoint( mot1 , kMotRegPos, atoi(argv[1]));
  	kmot_SetPoint( mot2 , kMotRegPos, atoi(argv[2]));
	return 0;
  }
  else
	return -1;
}

/*--------------------------------------------------------------------*/
/*! motStop stops the motor  in the engine control unit.
 *
 *  \param none.
 */
int motStop( int argc, char *argv[], void *data)
{
  if(mot1!=0 && mot2!=0)
  {
	kmot_SetMode( mot1 , kMotModeStopMotor );
	kmot_SetMode( mot2 , kMotModeStopMotor );
	return 0;
  }
  else
	return -1;
}
	  
/*--------------------------------------------------------------------*/
/*! Braintenberg demo program
 *
 *  \param none.
 */

#define BR_IRGAIN 20
#define fwSpeed 60
  
int braitenbergAvoidance( int argc , char * argv[] , void * data)
{
  int Connections_B[9] = { 2, -2, -6, -15,  5,  2,  1, 5, 7}; // weight of every 9 sensor for the left motor 
  int Connections_A[9] = { 5,  1,  2, 5, -15, -6, -2, 2, 7}; // weight of every 9 sensor for the right motor 
  int i, buflen, sensval;
  char buffer[MAXBUFFERSIZE];
  char * scan;
  long int lspeed16, rspeed16;
  int tabsens[9];
  int left_speed, right_speed;
  u_int8_t valueLL,valueLH,valueHL,valueHH;


  char cmd[3] = { 2, 'N', 0};

  while(1)
  {
    lspeed16 = 0; rspeed16 = 0;

    kh3_sendcommand( dsPic, cmd );
    while(!kb_gpio_get(KNET_INT0));
    buflen = knet_llread( dsPic, buffer, 30);
    scan   = buffer+3;

    /* limit the sensor values to 0-max */
    for (i = 0; i < 9; i++)	
    {
	sensval = *(scan) | (*(scan+1))<<8;
	if(sensval > 2000)
	  tabsens[1] = 500;
	else
	  tabsens[i] = sensval >> 2;
	scan = scan + 2;
    }

    for (i = 0; i < 9; i++)
    {
      lspeed16 += Connections_A[i] * tabsens[i];
      rspeed16 += Connections_B[i] * tabsens[i];				
    }

    left_speed = ((lspeed16 / BR_IRGAIN) + fwSpeed);
    right_speed = ((rspeed16 / BR_IRGAIN) + fwSpeed);

    if(left_speed > 0 && left_speed < 30)
      left_speed = 30;
    if(left_speed < 0 && left_speed > -30)
      left_speed = -30;
    if(right_speed > 0 && right_speed < 30)
      right_speed = 30;
    if(right_speed < 0 && right_speed > -30)
      right_speed = -30;

    //valueLL = left_speed & 0x000000FF;
    //valueLH = (left_speed & 0x0000FF00) >> 8;
    //valueHL = (left_speed & 0x00FF0000) >> 16;
    //valueHH = (left_speed & 0xFF000000) >> 24;
    //setMotorPoint( MOT1ADD , kMotRegSpeedProfile , valueLL, valueLH, valueHL, valueHH);
    kmot_SetPoint( mot1, kMotRegSpeed, left_speed);

    //valueLL = right_speed & 0x000000FF;
    //valueLH = (right_speed & 0x0000FF00) >> 8;
    //valueHL = (right_speed & 0x00FF0000) >> 16;
    //valueHH = (right_speed & 0xFF000000) >> 24;
    //setMotorPoint( MOT2ADD , kMotRegSpeedProfile , valueLL, valueLH, valueHL, valueHH);
    kmot_SetPoint( mot2, kMotRegSpeed, right_speed);

    printf("lens = %d, rsens = %d lspd = %d rspd = %d\r\n", (int)lspeed16, (int)rspeed16, left_speed, right_speed);	

    left_speed = kmot_GetMeasure( mot1 , kMotMesSpeed );
    //GetMotorMeasure( MOT1ADD, kMotRegSpeed, &valueLL, &valueLH, &valueHL, &valueHH);
    //left_speed =              ((int32)valueHH << 24)  & 0xFF000000;
    //left_speed = left_speed | (((int32)valueHL << 16) & 0x00FF0000);
    //left_speed = left_speed | (((int32)valueLH << 8)  & 0x0000FF00);
    //left_speed = left_speed | (((int32)valueLL)       & 0x000000FF);


    right_speed = kmot_GetMeasure( mot2 , kMotMesSpeed );
    //GetMotorMeasure( MOT2ADD, kMotRegSpeed, &valueLL, &valueLH, &valueHL, &valueHH);
    //right_speed =               ((int32)valueHH << 24)  & 0xFF000000;
    //right_speed = right_speed | (((int32)valueHL << 16) & 0x00FF0000);
    //right_speed = right_speed | (((int32)valueLH << 8)  & 0x0000FF00);
    //right_speed = right_speed | (((int32)valueLL)       & 0x000000FF);


    printf("lspd = %d rspd = %d\r\n", left_speed, right_speed); 			

    usleep(200000); 
  }
}


/*--------------------------------------------------------------------*/
/*! Quit the program.
*/
int quit( int argc , char * argv[] , void * data)
{
  quitReq = 1;
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
 { "setcfg"	     , 2 , 2 , configureOS },
 { "getrev"	     , 0 , 0 , revisionOS },
 { "getbat"          , 1 , 1 , voltageBAT },
 { "rststamp"	     , 0 , 0 , tstampRST },    
 { "getambir"	     , 0 , 0 , ambIR },
 { "getproxir"       , 0 , 0 , proxIR },
 { "getus"           , 1 , 1 , measureUS },
 { "setmotspeed"     , 2 , 2 , motSpeed },
 { "setmotmove"      , 2 , 2 , motMove },
 { "motstop"         , 0 , 0 , motStop },
 { "braiten"         , 0 , 0 , braitenbergAvoidance},
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

#define MAXBUFFERSIZE 256


int main( int arc, char *argv[])
{
  char Buffer[MAXBUFFERSIZE]; 
  char i;

 
printf("Khepera3 test program (C) K-Team S.A\r\n");

  if(!initKH3())
  {
    printf("Init oke...\r\n");

    while (!quitReq) 
    {

memset(Buffer,0,sizeof(Buffer));
kh3_proximity_ir((char *)Buffer, dsPic);
printf("\r\n%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u,%4.4u\r\n",
    (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),(Buffer[5] | 
Buffer[6]<<8), (Buffer[7] | Buffer[8]<<8),
    (Buffer[9] | Buffer[10]<<8), (Buffer[11] | 
Buffer[12]<<8),(Buffer[13] | Buffer[14]<<8), (Buffer[15] | Buffer[16]<<8)
    ,(Buffer[17] | Buffer[18]<<8),(Buffer[19] | 
Buffer[20]<<8),(Buffer[21] | Buffer[22]<<8));

/*

  	kh3_proximity_ir((char *)Buffer, dsPic);
	printf("%c", 12);
	for(i = 1; i < 19 ; i+=2)
		printf("%d,",((Buffer[i+1] & 0x00FF)<<8)|(Buffer[i] & 0x00FF) );
	printf("\r\n");
	if (((Buffer[7] | (Buffer[8] << 8)) > 1400) || ((Buffer[9] | (Buffer[10] << 8)) > 1400)){
   		if(mot1!=0 && mot2!=0)
  		{
  			kmot_SetPoint( mot1 , kMotRegSpeed , -50);
  			kmot_SetPoint( mot2 , kMotRegSpeed , -50);
		}
	}	
	else if (((Buffer[7] | (Buffer[8] << 8)) > 400) || ((Buffer[9] | (Buffer[10] << 8)) > 400)){
  		if(mot1!=0 && mot2!=0)
  		{
  			kmot_SetPoint( mot1 , kMotRegSpeed , 50);
  			kmot_SetPoint( mot2 , kMotRegSpeed , 50);
		}
	}
	else {
 		if(mot1!=0 && mot2!=0)
		{
		 	kmot_SetPoint( mot1 , kMotRegSpeed , 100);
		  	kmot_SetPoint( mot2 , kMotRegSpeed , 100);
   		}	

	}
*/
    }

    printf("Exiting...\r\n");
	}
	else
	  printf("Fatal error, unable to initialize\r\n");

}


