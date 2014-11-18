/*! 
 * \file   kh3server.c  Khepera3 server application              
 *
 * \brief 
 *         This is application waits for Khepera 3 commands A-Z received from serial port
 *				 usage: kh3server [serial_port (optional)]	 
 *         
 *        
 * \author   Julien Tharin (K-Team SA)                               
 *
 * \note     Copyright (C) 2012 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#include <korebot/korebot.h>
#include <signal.h>
#include <limits.h>

#define MAXBUFFERSIZE 128

//#define DEBUG

// serial parameters
#define BAUDRATE B115200
#define VAL_BAUDRATE 115200
#define SERIAL_PORT "/dev/ttyS1" // Bluetooth
// parameters for serial transmission
enum {
  Timeout = 800, // ms
  LineLength = 16 + 64 + 1 + 1 + 1,
};

#define ERROR_CMD_CHAR '$'


#define MAX_ARG 4
#define ARG_LENGTH 10
typedef  char string[ARG_LENGTH];

static int porthandle=-1;

static int quitReq = 0;
static char buf[1024];


/*! handle to the various khepera3 devices (knet socket, i2c mode)
 */
static knet_dev_t * dsPic;
static knet_dev_t * mot1;
static knet_dev_t * mot2;


int process_command (); // function declaration

/*--------------------------------------------------------------------*/
/*!
 * Intercept the ctrl-c
 *
 * \param sig signal
 *
 * \return none
 */
static void ctrlc_handler( int sig ) 
{
  quitReq = 1;

}


/*--------------------------------------------------------------------*/
/*! Kbhit helper function
 *
 * \return A value :
 *      - 1 Character pending 
 *      - 0 no character
 *			- -1 error
 */
int kbhit(void)
{
	struct timeval tv;
	fd_set read_fd;  /* Do not wait at all, not even a microsecond */
  
  tv.tv_sec=0;
  tv.tv_usec=0;  /* Must be done first to initialize read_fd */
  FD_ZERO(&read_fd);  /* Makes select() ask if input is ready:   *
                         0 is the file descriptor for stdin      */
  FD_SET(0,&read_fd);  /* The first parameter is the number of the *
                          largest file descriptor to check + 1. */
  if(select(1, &read_fd, NULL, /*No writes*/ NULL, /*No exceptions*/
&tv) == -1)
    return -1; /* An error occured */

  /* read_fd now holds a bit map of files that are   *
     readable. We test the entry for the standard    *
     input (file 0). */
  if(FD_ISSET(0,&read_fd))    /* Character pending on stdin */
    return 1; 
    
    
  /* no characters were pending */
  
  return 0;

} 

/*--------------------------------------------------------------------*/
/*! clear screen
 *
 * \return none
 *   
 */
void clear_screen()
{
	printf ("\33[2J" );
}

/*--------------------------------------------------------------------*/
/*! connect to serial port
 *
 * \param device device name
 * \param baudrate baudrate
 * \return >=0 : Ok; -1 : cannot open serial port
 *
*/ 
static int com_connect(const char* device, speed_t baudrate) {

	struct termios options;
	int HComm=-1; // handle to the port where the sensor is connected 

	if ((HComm=open(device,O_RDWR | O_NOCTTY | O_NDELAY))<0) {
		printf("ERROR: can't open serial port %s\n",device);
		return -1;
	}

	fcntl(HComm, F_SETFL, 0); // blocking input

	/* get the current options */
	tcgetattr(HComm, &options);

	/* set raw input, 0 second timeout */
	options.c_cflag     |= (CLOCAL | CREAD);
	// options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	
	options.c_lflag = ICANON;

	
	options.c_oflag     &= ~OPOST;
	options.c_cc[VMIN]  = 1;//0; // wait 1 char
	options.c_cc[VTIME] = 0; //Timeout/100; // timeout in 1/10 of second

	// setbaudrate
	cfsetispeed(&options, baudrate);
	cfsetospeed(&options, baudrate);

	/* set the options */
	tcsetattr(HComm, TCSANOW, &options);

  return HComm;
}

/*--------------------------------------------------------------------*/

/*!
 * write to serial port
 *
 * \param data data to be sent to the port
 * \param size size of the data
 *
 * \return number of written char
*/
static int com_send(const char* data, int size) {
  int n;

   tcflush (porthandle, TCIFLUSH); // clear the read/write buffer
   n=write( porthandle , data , size );

  return n;
}


/*--------------------------------------------------------------------*/
/*!
 * receive data from serial port
 *
 * \param data data to be sent to the port
 * \param max_size max size of the data variable
 * \param timeout timeout in [ms]
 *
 * \return number of data read
*/
static int com_recv(char* data, int max_size, int timeout) {

  int filled = 0;
  int readable_size = 0;
 	struct termios options;
 
 // fcntl(porthandle, F_SETFL, FNDELAY); // read will return immediately if no data

  memset (data, 0,max_size);

   do {
   // DWORD dwErrors;
   // COMSTAT ComStat;
   // ClearCommError(HComm, &dwErrors, &ComStat);

    ioctl(porthandle, FIONREAD, &readable_size); // get number of bytes available to be read

    int read_n = (readable_size > max_size) ? max_size : readable_size;

    int n;
    
    n=read( porthandle , &data[filled] ,read_n );
    filled += n;
    readable_size -= n;

    if (filled >= max_size) {
      return filled;
    }
  } while (readable_size > 0);

  // tcflush (HComm, TCIFLUSH); // clear the read buffer
 

  if (timeout > 0) {

	 /* get the current options */
	//	tcgetattr(porthandle, &options);
		options.c_cc[VTIME] = timeout/100; // timeout in tenth of second

		/* set the options */
	//	tcsetattr(porthandle, TCSANOW, &options);


	//	fcntl(porthandle, F_SETFL, 0); // blocking with timeout

		int n;
		while (1) {

			n=read(porthandle, &data[filled], 1);
			if (n < 1) {

				tcflush (porthandle, TCIFLUSH); // clear the read buffer

				return filled;
			}
			filled += n;
			if (filled >= max_size) {
				return filled;
			}
		
		}
  }
}

/*--------------------------------------------------------------------*/
/*
/*!
 * Read data (Reply) until the termination
 *
 *  \param buffer read data
 *
 *  \return return the number of the count of the read data
*/
static int readLine(char *buffer) {

  int i;
  for (i = 0; i < LineLength -1; ++i) {
    char recv_ch;
    int n = read(porthandle, &recv_ch, 1);// com_recv(&recv_ch, 1, Timeout);
    if (n <= 0) {
      if (i == 0) {
				return -1;		// timeout
      }
      break;
    }
    if ((recv_ch == '\r') || (recv_ch == '\n')) {
      break;
    }
    buffer[i] = recv_ch;
  }
  buffer[i] = '\0';

	
	
  return i;
}

/*--------------------------------------------------------------------*/
/*! initMot initializes then configures the motor control unit.
 *
 * \param hDev device handle
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
	  kmot_ConfigurePID( hDev , kMotRegSpeed , 620 , 3 , 10 ); // kp kd ki
	  kmot_ConfigurePID( hDev,kMotRegPos,600,20,30);
	  kmot_SetSpeedProfile(hDev,15000,10);

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
 *      - 0 if success
 *      - <0 if any error
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
/*! Init mot or reset motor controller
 *
 * \param narg number of argument
 * \param larg argument array
 *
 * \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int InitMot(int narg,string *larg)
{
	int rc;
	char Buffer[MAXBUFFERSIZE];

  if(dsPic)
  {
    if((mot1!=0)&&(mot2!=0))
    {
      initMot(mot1);
      initMot(mot2);
      sprintf(Buffer,"m\r\n");
			com_send(Buffer,strlen(Buffer));
      return 0;
    }
  }
	
	return -1;
}	


/*--------------------------------------------------------------------*/
/*! proxIR retrieves proximity ir measure using kb_khepera3.c library.
 *
 * \param narg number of argument
 * \param larg argument array
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int ReadProxSensors(int narg, string *larg)
{
  char Buffer[MAXBUFFERSIZE];
  char BufferOut[MAXBUFFERSIZE];
  int sensor,i;
  

	sprintf(BufferOut,"n");

	if(kh3_proximity_ir((char *)Buffer, dsPic))
	{	
		for (i=0;i<11;i++)
		{
			sensor=(Buffer[i*2+1] | Buffer[i*2+2]<<8);
			sprintf(BufferOut+strlen(BufferOut),",%d",sensor);
			
		}																		
		sprintf(BufferOut+strlen(BufferOut),",%lu\r\n",((Buffer[23] | Buffer[24]<<8 ) |  ( Buffer[25] | Buffer[26]<<8)<<16) );
		com_send(BufferOut, strlen(BufferOut));
		
		
		#ifdef DEBUG
			printf("\nReadProxSensors : %s\n",BufferOut);
		#endif	
		return 0;	
	}

	return -1;			
}
		
/*--------------------------------------------------------------------*/
/*! ambIR retrieves ambiant ir measure using kb_khepera3.c library.
 *
 * \param narg number of argument
 * \param larg argument array
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int ReadAmbSensors(int narg, string *larg)
{
  char Buffer[MAXBUFFERSIZE];
  char BufferOut[MAXBUFFERSIZE];
  int sensor,i;
  

	sprintf(BufferOut,"o");

	if(kh3_ambiant_ir((char *)Buffer, dsPic))
	{	
		for (i=0;i<11;i++)
		{
			sensor=(Buffer[i*2+1] | Buffer[i*2+2]<<8);

			sprintf(BufferOut+strlen(BufferOut),",%d",sensor);
				
		}
	
		sprintf(BufferOut+strlen(BufferOut),",%lu\r\n",((Buffer[23] | Buffer[24]<<8 ) |  ( Buffer[25] | Buffer[26]<<8)<<16));
		com_send(BufferOut, strlen(BufferOut));
		#ifdef DEBUG
			printf("\nReadAmbSensors : %s\n",BufferOut);
		#endif	
		return 0;	
	}

	return -1;	
}

/*--------------------------------------------------------------------*/
/*! batStatus retrieves the battery status using kb_khepera3.c library.
 *
 * \param narg number of argument
 * \param larg state to inquire
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int batStatus(int narg, string *larg)
{
  char Buffer[MAXBUFFERSIZE];
  char BufferOut[MAXBUFFERSIZE];
  short argument;

	if (narg != 1)
	{
		sprintf(Buffer,"%c\r\n",ERROR_CMD_CHAR);
		com_send(Buffer, strlen(Buffer));
		return -1;
	}

  argument = atoi(larg[0]);
  if(kh3_battery_voltage((char *)Buffer, argument, dsPic)){
		switch(argument){
			case 0:		/* Read Voltage */
				 sprintf(BufferOut,"v,%d,%d\r\n",(Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
				break;
			case 1:		/* Read Current */
				if ((Buffer[2] & 0x80) != 0){
					Buffer[2] = Buffer[2] & 0x7F;
					sprintf(BufferOut,"v,-%d,%d\r\n",(Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
				}
				else{
					sprintf(BufferOut,"v,%d,%d\r\n",(Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
				}
				break;
			case 2:		/* Read Average Current */
				if ((Buffer[2] & 0x80) != 0){
					Buffer[2] = Buffer[2] & 0x7F;
			
					sprintf(BufferOut,"v,-%d,%d\r\n",(Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
				}
				else{
					sprintf(BufferOut,"v,%d,%d\r\n",(Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
				}
				break;
			case 3:		/* Read absolute remaining capacity */
				sprintf(BufferOut,"v,%3.3u\r\n",(Buffer[1] | Buffer[2]<<8));
				break;
			case 4:		/* Read Temperature */
				sprintf(BufferOut,"v,%d,%d\r\n",(Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
				break;
			case 5:		/* Read relative remaining capacity */
				 sprintf(BufferOut,"v,%d\r\n",(Buffer[1] | Buffer[2]<<8));
				break;


			default:
				sprintf(BufferOut,"%c\r\n",ERROR_CMD_CHAR);
				
				
		} // switch
		com_send(BufferOut,strlen(BufferOut));
		return 0;
  }
  
  return -1;
  	
	 // printf("\r\nv, error...\r\n");
}

/*--------------------------------------------------------------------*/
/*! tstampRST resets the relative time stamp using kb_khepera3.c library.
 *
 * \param narg number of argument
 * \param larg array of argument
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int tstampRST(int narg, string *larg)
{
  char Buffer[MAXBUFFERSIZE];

  if(kh3_reset_tstamp((char *)Buffer, dsPic))
	{
		sprintf(Buffer,"z\r\n");
		com_send(Buffer,strlen(Buffer));
	
		return 0;
	}
	
	return -1;
}

/*--------------------------------------------------------------------*/
/*! revisionOS retrieves the khepera3 os version using kb_khepera3.c library.
 *
 * \param narg number of argument
 * \param larg array of argument
 *
 * \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int revisionOS(int narg, string *larg)
{
  char Buffer[MAXBUFFERSIZE];	/* buffer that handle the returned datas from kh3 */
  unsigned int ver ;


  if(kh3_revision((char *)Buffer, dsPic)){
  
  	sprintf(Buffer,"b,%d,%d\r\n",(Buffer[1] | Buffer[2]<<8),(Buffer[3] | Buffer[4]<<8));
  	com_send(Buffer, strlen(Buffer));
   
		return 0;
  }
  
  return -1;
}

/*--------------------------------------------------------------------*/
/*! configureOS configures various parameters inside the kh3 firmware
 *  using kb_khepera3.c library.
 *
 *	\param argc number of argument	
 *  \param argv first param (argv[1]) is the index pointing in configuration array.
 *              2nd the second param (argv[2]) is the value to store where the index point at.
 *  
 */
int configureOS( int argc, string * argv)
{
  char Buffer[MAXBUFFERSIZE];	/* buffer that handle the returned datas from kh3 */
  short index;			/* variable that handles index */
  short value;			/* variable that handle value */

  /* Retrieve the arguments from the parameter */
  index = atoi(argv[0]);
  
  argv[1][0]=' '; // remove d (16 bits) indicator
  value = atoi(argv[1]);
  
  /* Configure */
  if(kh3_configure_os((char *)Buffer, index, value, dsPic))
  {
  	sprintf(Buffer,"c\r\n");
  	com_send(Buffer, strlen(Buffer));
  	return 0;
  }
  //	printf("\r\n%c\r\n", Buffer[0]);
  //else
	//printf("\r\nc, error...\r\n");
	return -1;
}
  
/*--------------------------------------------------------------------*/
/*! measureUS retrieves ultrasonic measure from a given transceiver.
 *
 *	\param argc number of argument 	
 *  \param argv first param (argv[0]) is the us number to read from (1 to 5).
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 *  
 */
int GetUS( int argc, string * argv)
{
  char Buffer[MAXBUFFERSIZE],buf[MAXBUFFERSIZE];
  
  int i;
 

  if(kh3_measure_us((char *)Buffer, atoi(argv[0]), dsPic))
  {
		sprintf(buf,"g,%d",(Buffer[1] | Buffer[2]<<8));
		
		for(i=0;i<5;i++)
		{
			sprintf(buf+strlen(buf),",%d,%d,%ld",(Buffer[3+8*i] | Buffer[4+8*i]<<8),(Buffer[5+8*i] | Buffer[6+8*i]<<8),(long int)(Buffer[7+8*i] | Buffer[8+8*i]<<8 | Buffer[9+8*i]<<16 | Buffer[10+8*i]<<24));
		}
		
		sprintf(buf+strlen(buf),",%d\r\n",(Buffer[43] | Buffer[44]<<8));
		com_send(buf, strlen(buf));
		
		return 0;
	} 
	
	return -1;
	
}


/*--------------------------------------------------------------------*/
/*! SetSpeed configures the motor controller speed in the engine control unit.
 *
 *	\param argc number of argument	
 *  \param argv first param (argv[0]) is the motor1 speed.
 *   						2nd second param (argv[1]) is the motor2 speed.
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int SetSpeed( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
  if(mot1!=0 && mot2!=0)
  {
  	// remove 32 bit indicator
  	argv[0][0]=' ';
  	argv[1][0]=' ';
  	
  	
  	#ifdef DEBUG
  		printf("\nSetSpeed : argv[0] %s %d argv[1] %s %d",argv[0],atoi(argv[0]),argv[1],atoi(argv[1]));
  	#endif	
  	
  	kmot_SetPoint( mot1 , kMotRegSpeed , atol(argv[0]));
  	kmot_SetPoint( mot2 , kMotRegSpeed , atol(argv[1]));
  	
  	sprintf(Buffer,"d\r\n");
  	com_send(Buffer, strlen(Buffer));
  	
	return 0;
  }

	return -1;
}

/*--------------------------------------------------------------------*/
/*! SetSpeedOpenLoop configures the motor controller speed in the engine control unit.
 *
 *	\param argc number of argument	
 *  \param argv first param (argv[0]) is the motor1 speed.
 *   						2nd second param (argv[1]) is the motor2 speed.
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int SetSpeedOpenLoop( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
  if(mot1!=0 && mot2!=0)
  {
  	// remove 32 bit indicator
  	argv[0][0]=' ';
  	argv[1][0]=' ';
  	
  	kmot_SetPoint( mot1 , kMotRegOpenLoop  , atol(argv[0]));
  	kmot_SetPoint( mot2 , kMotRegOpenLoop  , atol(argv[1]));
  	
  	sprintf(Buffer,"l\r\n");
  	com_send(Buffer, strlen(Buffer));
  	
	return 0;
  }


	return -1;
}


/*--------------------------------------------------------------------*/
/*! Getspeed
 *
 *	\param argc number of argument	
 *  \param argv array of argument
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int GetSpeed( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
	int left,right; 

  if(mot1!=0 && mot2!=0)
  {
  	
  	
  	left = kmot_GetMeasure( mot1 , kMotMesSpeed );
    right = kmot_GetMeasure( mot2 , kMotMesSpeed );
    
   	#ifdef DEBUG
   		printf("\nGetSpeed : left %d right %d",left,right);
   	#endif
  	
  	
  	sprintf(Buffer,"e,%d,%d\r\n",left,right);
  	com_send(Buffer, strlen(Buffer));
  	
	return 0;
  }


	return -1;
}

/*--------------------------------------------------------------------*/
/*! SetTargetProfile configures the motor controller position in the engine control unit using the PID and speed/accel profile.
 *
 *	\param argc number of argument
 *  \param argv first param (argv[0]) is the motor1 position.
 *  						second param (argv[1]) is the motor2 position.
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int SetTargetProfile( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
  if(mot1!=0 && mot2!=0)
  {
  	// remove 32 bit indicator
  	argv[0][0]=' ';
  	argv[1][0]=' ';
		kmot_SetPoint( mot1 , kMotRegPosProfile, atol(argv[0]));
  	kmot_SetPoint( mot2 , kMotRegPosProfile, atol(argv[1]));
  	
  	sprintf(Buffer,"f\r\n");
  	com_send(Buffer, strlen(Buffer));
	return 0;
  }


	return -1;
}

/*--------------------------------------------------------------------*/
/*! SetTargetPosition move without the acceleration and decceleration.
 *
 *	\param argc number of argument
 *  \param argv first param (argv[0]) is the motor1 position.
 *  						second param (argv[1]) is the motor2 position.
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int SetTargetPosition( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
  if(mot1!=0 && mot2!=0)
  {
  	// remove 32 bit indicator
  	argv[0][0]=' ';
  	argv[1][0]=' ';
  	
  	#ifdef DEBUG
  		printf("\nSetPosition : argv[0] %s %d argv[1] %s %d",argv[0],atoi(argv[0]),argv[1],atoi(argv[1]));
  	#endif
  	
		kmot_SetPoint( mot1 , kMotRegPos , atol(argv[0]));
  	kmot_SetPoint( mot2 , kMotRegPos, atol(argv[1]));
  	
  	sprintf(Buffer,"p\r\n");
  	com_send(Buffer, strlen(Buffer));
	return 0;
  }

	return -1;
}

/*--------------------------------------------------------------------*/
/*! SetEncPosition set encoder position
 *
 *	\param argc number of argument
 *  \param argv first param (argv[0]) is the motor1 position.
 *  						second param (argv[1]) is the motor2 position.
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int SetEncPosition( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
	
  if(mot1!=0 && mot2!=0)
  {
  	// remove 32 bit indicator
  	argv[0][0]=' ';
  	argv[1][0]=' ';
  	
  	#ifdef DEBUG
  		printf("\nSetEncPosition : argv[0] str:%s nb:%ld | argv[1] str:%s nb:%ld",argv[0],atol(argv[0]),argv[1],atol(argv[1]));
  	#endif
  	
		kmot_SetPosition( mot1 , atol(argv[0]));
  	kmot_SetPosition( mot2 , atol(argv[1]));
  	
  	sprintf(Buffer,"i\r\n");
  	com_send(Buffer, strlen(Buffer));
	return 0;
  }

	return -1;
}

/*--------------------------------------------------------------------*/
/*! GetMotPos get the motor position.
 *	
 *	\param argc number of argument
 *  \param argv first param (argv[0]) is the motor1 position.
 *  						second param (argv[1]) is the motor2 position.
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int ReadPos( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
	int left,right; 
	

  if(mot1!=0 && mot2!=0)
  {
  	
  	
  	left = kmot_GetMeasure( mot1 , kMotMesPos );
    right = kmot_GetMeasure( mot2 , kMotMesPos );
  	
  	
  	#ifdef DEBUG
  		printf("\nReadPos : left %d right %d",left,right);
  	#endif	
  	
  	sprintf(Buffer,"r,%d,%d\r\n",left,right);
  	com_send(Buffer, strlen(Buffer));
  	
		return 0;
  }

	return -1;
}


/*--------------------------------------------------------------------*/
/*! Configure PID
 *	
 *	\param argc number of argument
 *  \param argv first param (argv[0]) is 0 speed control or 1 position control
 *  						second param (argv[1]) is the Kp parameter
 *  						third param (argv[2]) is the Ki parameter
 *  						forth param (argv[3]) is the Kd parameter
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int ConfigPID( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
  if(mot1!=0 && mot2!=0)
  {
  	// remove 16 bit indicator
  	argv[0][0]=' ';
  	argv[1][0]=' ';
  	argv[2][0]=' ';
  	argv[3][0]=' ';
  	
  	if (atoi(argv[0]))
  	{
  		// position control
  		kmot_ConfigurePID( mot1,kMotRegPos,atoi(argv[1]),atoi(argv[3]),atoi(argv[2])); // kp kd ki
  		kmot_ConfigurePID( mot2,kMotRegPos,atoi(argv[1]),atoi(argv[3]),atoi(argv[2])); // kp kd ki
  	}
  	else
  	{
  		// speed control
  		kmot_ConfigurePID( mot1,kMotRegSpeed,atoi(argv[1]),atoi(argv[3]),atoi(argv[2])); // kp kd ki
  		kmot_ConfigurePID( mot2,kMotRegSpeed,atoi(argv[1]),atoi(argv[3]),atoi(argv[2])); // kp kd ki
  	}
  	
 	
  	sprintf(Buffer,"h\r\n");
  	com_send(Buffer, strlen(Buffer));
	return 0;
  }

	return -1;	
}

/*--------------------------------------------------------------------*/
/*! configure speed and acceleration profiles
 *
 *	\param argc number of argument	
 *  \param argv array of argument
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int ConfigSpeedProfile( int argc, string *argv)
{
	char Buffer[MAXBUFFERSIZE];
  if(mot1!=0 && mot2!=0)
  {
  	// remove 32 bit indicator
  	argv[0][0]=' ';
  	argv[1][0]=' ';
  	kmot_SetSpeedProfile(mot1,atoi(argv[0]),atoi(argv[1]));
  	kmot_SetSpeedProfile(mot2,atoi(argv[0]),atoi(argv[1]));

  	
  	sprintf(Buffer,"j\r\n");
  	com_send(Buffer, strlen(Buffer));
	return 0;
  }

	return -1;
}  

/*--------------------------------------------------------------------*/
/*! set led
 *
 *	\param argc number of argument	
 *  \param argv first param (argv[0]) is led index
 *              second param (argv[1]) is led state to set
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int SetLED(int argc,string * argv)
{
	int rc;
	char buf[MAXBUFFERSIZE];
  /* Frame format : { Size, Command, Terminator }
   * where the command can be more than 1 byte */
  char cmd[5] = { 4, 'K', 0, 0, 0};
  cmd[2] = (char)atoi(argv[0]);
  cmd[3] = (char)atoi(argv[1]);



  if(dsPic)
  {
		kh3_sendcommand( dsPic , cmd );
		rc = kh3_getcommand( dsPic, buf );
		

		sprintf(buf,"k\r\n");
		com_send(buf, strlen(buf));
		
		#ifdef DEBUG
  		printf("\nSetLED : number %d state %d",cmd[2],cmd[3]);
  	#endif
		
		return rc;
  }
	
	return 0;
}	

	  

/*--------------------------------------------------------------------*/
/*! Braintenberg mode
 *
 *	\param argc number of argument	
 *  \param argv first param (argv[0]) is avoidance sensor 0 = IR, 1 = US (unimplemented)
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
 */

#define BR_IRGAIN 30
#define fwSpeed 20
#define MIN_SPEED 10
#define RotSpeedL 50
#define RotSpeedR -50
 
int braitenberg( int argc , string * argv)
{
  int Connections_B[9] = { 2, -2, -4, -12,  5,  2,  2, 2, 4}; // weight of every 9 sensor for the left motor 
  int Connections_A[9] = { 2,  2,  2, 5, -12, -4, -2, 2, 4}; // weight of every 9 sensor for the right motor 
  int i, buflen, sensval;
  char buffer[MAXBUFFERSIZE];
  char * scan;
  long int lspeed16, rspeed16;
  int tabsens[9];
  int left_speed, right_speed;
  unsigned int immobility = 0;
  unsigned int prevpos_left, pos_left, prevpos_right,  pos_right;
  u_int8_t valueLL,valueLH,valueHL,valueHH;
  
  int readable_size=0;
	int out=0;
	char Buffer[MAXBUFFERSIZE];

	
	if (argc != 1)
	{
		sprintf(buf,"%c\r\n",ERROR_CMD_CHAR);
		com_send(buf, strlen(buf));
		return -1;
	}
	
	if (atoi(argv[0]) == 2 )
	{
		sprintf(buf,"a\r\n");
		com_send(buf, strlen(buf));
		return 1;
	} else
	{
		if ((atoi(argv[0]) == 0) || (atoi(argv[0]) == 1) )
		{
			// continue with below
		}
		else
		{
			sprintf(buf,"%c\r\n",ERROR_CMD_CHAR);
			com_send(buf, strlen(buf));
			return -1;
		}
	}
	
	sprintf(buf,"a\r\n");
	com_send(buf, strlen(buf));
	
  /* Get the current position values */
  prevpos_left = kmot_GetMeasure( mot1 , kMotRegPos );
  prevpos_right = kmot_GetMeasure( mot2 , kMotRegPos );


	printf("\nBraitenberg mode; enter any serial command to stop\n");

	fcntl(porthandle, F_SETFL, FNDELAY); // read will return immediately if no data

  do
  {
    lspeed16 = 0; rspeed16 = 0;

#if 0
    kh3_sendcommand( dsPic, cmd );
    while(!kb_gpio_get(KNET_INT0));

    buflen = knet_llread( dsPic, buffer, 30);
#endif

    kh3_proximity_ir((char *)buffer, dsPic);
    
    
    scan   = buffer+3; // why not 1???

    /* limit the sensor values to 0-max */
    for (i = 0; i < 9; i++)	
    {
			sensval = *(scan) | (*(scan+1))<<8;
			if(sensval > 1000)
				tabsens[i] = 450;
			else if (sensval < 100)
				tabsens[i] = 0;
			else
				tabsens[i] = (sensval - 100) >> 1;
			scan = scan + 2;
    }

    for (i = 0; i < 9; i++)
    {
      lspeed16 += Connections_A[i] * tabsens[i];
      rspeed16 += Connections_B[i] * tabsens[i];				
    }

    left_speed = ((lspeed16 / BR_IRGAIN) + fwSpeed);
    right_speed = ((rspeed16 / BR_IRGAIN) + fwSpeed);

    if(left_speed > 0 && left_speed < MIN_SPEED)
      left_speed = MIN_SPEED;
    if(left_speed < 0 && left_speed > -MIN_SPEED)
      left_speed = -MIN_SPEED;
    if(right_speed > 0 && right_speed < MIN_SPEED)
      right_speed = MIN_SPEED;
    if(right_speed < 0 && right_speed > -MIN_SPEED)
      right_speed = -MIN_SPEED;

/* define Shift_speed for version under 1.4*/
#ifndef  SHIFT_SPEED
     left_speed *= 256;
     right_speed *= 256;
#endif


    kmot_SetPoint( mot1, kMotRegSpeed, left_speed);
    kmot_SetPoint( mot2, kMotRegSpeed, right_speed);

    //printf("lens = %d, rsens = %d lspd = %d rspd = %d\r\n", (int)lspeed16, (int)rspeed16, left_speed, right_speed);	

    left_speed = kmot_GetMeasure( mot1 , kMotMesSpeed );
    right_speed = kmot_GetMeasure( mot2 , kMotMesSpeed );

    /* Get the new position of the wheel to compare with previous values */
    pos_left = kmot_GetMeasure( mot1 , kMotRegPos );
    pos_right = kmot_GetMeasure( mot2 , kMotRegPos );


    if((pos_left < (prevpos_left + 700)) && (pos_left > (prevpos_left -700)) && (pos_right < (prevpos_right + 700)) && (pos_right > (prevpos_right -700)))
    {
    	
    	//printf("==> Immobility\n");
    	
      if(++immobility > 5)
      {
         left_speed = RotSpeedL;
         right_speed = RotSpeedR;
#ifndef  SHIFT_SPEED
         left_speed *= 256;
         right_speed *= 256;
#endif
         kmot_SetPoint( mot1, kMotRegSpeed, left_speed);
         kmot_SetPoint( mot2, kMotRegSpeed, right_speed);

				 do{
						usleep(10);
						kh3_proximity_ir((char *)buffer, dsPic);
				 }while (((buffer[7] | (buffer[8]<<8) ) >250) || ((buffer[9] | (buffer[10]<<8)) >250));

         immobility = 0;
         prevpos_left = pos_left;
         prevpos_right = pos_right;
      }
    }
    else
    {
       immobility = 0;
       prevpos_left = pos_left;
       prevpos_right = pos_right;
    }

    usleep(20000); 
  
  	
  	ioctl(porthandle, FIONREAD, &readable_size); // get number of bytes available to be read
  	
  	if (readable_size>0)
		{	
    	out=process_command(); // warning: if braitenberg 0 or 1, nested loop
   	} // if (readable_size>0)
  
  } while(!out);
  
  
  kmot_SetMode( mot1 , kMotModeStopMotor );
	kmot_SetMode( mot2 , kMotModeStopMotor );
  
  tcflush (porthandle, TCIFLUSH); // clear the read buffer
  fcntl(porthandle, F_SETFL, 0); // blocking input
  
  printf("Braitenberg mode exit\n");
  
  return 0;
}


#define RS232_EOA     	','
#define EOL_TEST(data) (data == '\r' || data == '\n' || data == '\0')

/*--------------------------------------------------------------------*/
/*! getArgs separate argument , return arguments number and save in larg every argument 
 *
 *	\param buf arguments string	
 *  \param larg array of argument
 *
 *  \return arguments number
*/ 
int getArgs(char *buf,string *larg)
{
	int narg=0;
	
	char delim[2]= {RS232_EOA , '\0'};
	char * pch;
	

		
	pch = strtok (buf,delim);
	while (pch != NULL)
	{
	  
	  strcpy(larg[narg],pch);
	  pch = strtok (NULL, delim);
	  
	  #ifdef DEBUG
		  printf(", | arg nb %d arg val: %s",narg,larg[narg]);
		#endif  

	  narg++;
	  
	  if (narg == MAX_ARG)
			break;
	}


	return narg;
}


/*--------------------------------------------------------------------*/
/*!  BinaryRead for GUI
 *
 *	\param narg number of argument	
 *  \param larg array of argument
 *
 *  \return A value :
 *      - 0 if success
 *      - <0 if any error
*/
int	BinaryRead(int narg,string *larg) {
	char Buffer[MAXBUFFERSIZE],buf[MAXBUFFERSIZE];
	int i, data;
		
		// Write the proximity sensor value in the buffer

		Buffer[0]='x';
		
		kh3_proximity_ir((char *)buf, dsPic);
		for (i=0;i<22;i++)
		{	
				Buffer[i+1]=buf[i+1];
		}	
		
		// Write the ambient light measurement 
	
		kh3_ambiant_ir((char *)buf, dsPic);
		for (i=0;i<22;i++)
		{
			Buffer[i+23]=buf[i+1];
		}		
		
				
		// Get the motor Speed and write it in the buffer
		
		data = kmot_GetMeasure( mot1 , kMotMesSpeed );		
    Buffer[45]=(u_int8_t )(data & 0x00FF);
    Buffer[46]=(u_int8_t )((data>>8) & 0x00FF);
    Buffer[47]=(u_int8_t )((data>>16) & 0x00FF);
    Buffer[48]=(u_int8_t )((data>>24) & 0x00FF);
    
    data = kmot_GetMeasure( mot2 , kMotMesSpeed );
    Buffer[49]=(u_int8_t )(data & 0x00FF);
    Buffer[50]=(u_int8_t )((data>>8) & 0x00FF);
    Buffer[51]=(u_int8_t )((data>>16) & 0x00FF);
    Buffer[52]=(u_int8_t )((data>>24) & 0x00FF);

 
		
		// Get the motor Position and write it in the buffer
		
		data = kmot_GetMeasure( mot1 , kMotRegPos );
 		Buffer[53]=(u_int8_t )(data & 0x00FF);
    Buffer[54]=(u_int8_t )((data>>8) & 0x00FF);
    Buffer[55]=(u_int8_t )((data>>16) & 0x00FF);
    Buffer[56]=(u_int8_t )((data>>24) & 0x00FF);
    data = kmot_GetMeasure( mot2 , kMotRegPos );
    Buffer[57]=(u_int8_t )(data & 0x00FF);
    Buffer[58]=(u_int8_t )((data>>8) & 0x00FF);
		Buffer[59]=(u_int8_t )((data>>16) & 0x00FF);
    Buffer[60]=(u_int8_t )((data>>24) & 0x00FF);
    
		//timestamp from last call
		

		Buffer[61]=buf[23];
    Buffer[62]=buf[24];
    Buffer[63]=buf[25];
    Buffer[64]=buf[26];
    
    Buffer[65]='\n';
    Buffer[66]='\r';
    Buffer[67]='\0';
		
		com_send(Buffer, 67);
		
		#ifdef DEBUG
			printf("\nBinaryRead (hexa): ");
			for (i=0; i<67; i++)
				printf(" %02x",Buffer[i]);
		#endif	
						
		
	return 0;	
}		

/*--------------------------------------------------------------------*/
/*! process command
 *
 * \return A value : braitenberg end mode
 *    - 0 if success
 *    - <0 if any error
*/
int process_command ()
{
	char Buffer[MAXBUFFERSIZE];
 	int narg;
  
  int out=0;
	
 	string larg[MAX_ARG];
	char *bptr;


	// receive and interpret commands (wait)
  if ( readLine(Buffer) >0 ) 
  {
  	#ifdef DEBUG
  		printf("%c length %d |%s|",Buffer[0],strlen(Buffer),Buffer);
  	#endif
  	
  	if (strlen(Buffer)>2)
  	{
  		/* Process all the args */
			bptr = Buffer + 2;
			
			narg=getArgs(bptr,larg);
			
		}
  	
		switch(Buffer[0])
		{
			case 'A':
				out=braitenberg(narg,larg);
				break;
			case 'B':
				revisionOS(narg,larg);
				break;
			case 'C': 
				configureOS(narg,larg);	
				break;
			case 'D': 
				SetSpeed(narg,larg);	
				break;
			case 'E': 
				GetSpeed(narg,larg);	
				break;
			case 'F': 
				SetTargetProfile(narg,larg);	
				break;
			case 'G': 
				GetUS(narg,larg);	
				break;
			case 'H': 
				ConfigPID(narg,larg);	
				break;
			case 'I': 
				SetEncPosition(narg,larg);	
				break;
			case 'J': 
				ConfigSpeedProfile(narg,larg);	
				break;
			case 'K': 
				SetLED(narg,larg);	
				break;
			case 'L': 
				SetSpeedOpenLoop(narg,larg);	
				break;
			case 'M': 
				InitMot(narg,larg);	
				break;					
			case 'N': 
				ReadProxSensors(narg,larg);	
				break;
			case 'O': 
				ReadAmbSensors(narg,larg);	
				break;
			case 'P': 
				SetTargetPosition(narg,larg);	
				break;
			case 'R': 
				ReadPos(narg,larg);	
				break;							
			case 'V':
				batStatus(narg,larg);
				break;
			case 'X':
				BinaryRead(narg,larg);
				break;
			case 'Z':
				tstampRST(narg,larg);
				break;
			default:
				sprintf(Buffer,"%c\r\n",ERROR_CMD_CHAR);
				com_send(Buffer, strlen(Buffer));
				
		}
  }
      
	return out;
}


/*****************************************************************************/
/**** Main program ***********************************************************/
int main( int argc, char *argv[])
{
  char Buffer[MAXBUFFERSIZE];
 
 	
	
  printf("Khepera3 server program (C) K-Team S.A\r\n");

	


  if(!initKH3())
  {
    printf("Init ok...\r\n");

   kh3_revision((char *)Buffer, dsPic);
   printf("\r\n%c,%4.4u,%4.4u => Version = %u, Revision = %u\r\n",
           Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
           (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));


		if (argc == 2)
		{
			strcpy(Buffer,argv[1]); // use serial port given by parameter
		}
		else
		{
			strcpy(Buffer,SERIAL_PORT); // copy default
		}

		// open serial port
		if ((porthandle=com_connect(Buffer,BAUDRATE))<0)
		{
			printf("\nError: Serial port %s could not be open!\n",Buffer);
			return -1;
		}

		printf("\nParsing commands from serial port %s, baudrate %ld.\nPush CTRL-C for quitting!\n",Buffer,VAL_BAUDRATE);

		// loop
 		while (!quitReq) 
    {


			#ifdef DEBUG
      	printf("\n> ");
      #endif
			process_command();  

			
    }

    printf("Exiting...\r\n");
	}
	else
	  printf("Fatal error, unable to initialize\r\n");
	
	// close serial port	
	if (porthandle>=0)
		close(porthandle);
		  
	return 0;  
}


