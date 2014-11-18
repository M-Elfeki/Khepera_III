#include <korebot/korebot.h>

static knet_dev_t * Arm;
static knet_dev_t * Gripper;
static knet_dev_t * dsPic;
static knet_dev_t * mot1;
static knet_dev_t * mot2;


#define PULSE_TO_MM_FIRMWARE_S_3 0.04629 // for version <3.0
#define PULSE_TO_MM_FIRMWARE_BE_3 0.03068

#define MM_S_TO_SPEED_FIRMWARE_S_3 144.01 // for version <3.0
#define MM_S_TO_SPEED_FIRMWARE_BE_3 218.72

#define IR_BAR_LEN 15

#define	ARM_SEARCH_LIMIT   0x41

int kgripper_init( void )
 {
  int rc;
   
  if((rc = kb_init( 0 , NULL )) < 0 )
  {
         /* Unable to initialize the korebot library */
         KB_ERROR("kb_kh3_init",KB_ERROR_KH3KBINIT);
         return KH3_ERROR_KBINIT;
  }
 }

void kgripper_Arm_Set_Search_Limit( knet_dev_t * dev, unsigned char Search_Limit )
 {
   if(Search_Limit > 1)
     Search_Limit = 1;
   knet_write8( dev , ARM_SEARCH_LIMIT , Search_Limit );
 
 }

int main()
{
	float fpos;
	long lpos,rpos;
	char Buffer[100],bar[11][IR_BAR_LEN+5];
	int sensors[11],i,n,revision,version;
	short index, value;
	char usvalues[5];
	long motspeed;
	float pulsestomm=PULSE_TO_MM_FIRMWARE_S_3 ; 
	float mmstospeed=MM_S_TO_SPEED_FIRMWARE_S_3 ; 
	
	kh3_init();

	dsPic = knet_open( "Khepera3:dsPic", KNET_BUS_I2C , 0 , NULL );
	mot1 = knet_open( "Khepera3:mot1", KNET_BUS_I2C , 0 , NULL );
	mot2 = knet_open( "Khepera3:mot2", KNET_BUS_I2C , 0 , NULL );
	
	
	kmot_SetMode( mot1 , kMotModeIdle );
	kmot_SetSampleTime( mot1 , 1550 );
	kmot_SetMargin( mot1 , 6 );
	kmot_SetOptions( mot1 , 0x0 , kMotSWOptWindup | kMotSWOptStopMotorBlk
	| kMotSWOptDirectionInv );
	kmot_ResetError( mot1 );
	kmot_SetBlockedTime( mot1 , 10);
	kmot_ConfigurePID( mot1 , kMotRegSpeed , 620 , 3 , 10 );
	kmot_ConfigurePID( mot1 ,kMotRegPos,600,20,30);
	kmot_SetSpeedProfile(mot1 ,15000,30);

	
	kmot_SetMode( mot2 , kMotModeIdle );
	kmot_SetSampleTime( mot2 , 1550 );
	kmot_SetMargin( mot2 , 6 );
	kmot_SetOptions( mot2 , 0x0 , kMotSWOptWindup | kMotSWOptStopMotorBlk );
	kmot_ResetError( mot2 );
	kmot_SetBlockedTime( mot2 , 10);
	kmot_ConfigurePID( mot2 , kMotRegSpeed , 620 , 3 , 10 );
	kmot_ConfigurePID( mot2 ,kMotRegPos,600,20,30);
	kmot_SetSpeedProfile(mot2 ,15000,30);

	

	kgripper_init();

	Arm = knet_open( "Kgripper:Arm" , KNET_BUS_I2C , 0 , NULL );
	Gripper = knet_open( "Kgripper:Gripper" , KNET_BUS_I2C , 0 , NULL );
	

	if(kh3_revision((char *)Buffer, dsPic))
	{
		version=(Buffer[1] | Buffer[2]<<8);
		revision=(Buffer[3] | Buffer[4]<<8);
		printf("\r\n%c,%4.4u,%4.4u => Version = %u, Revision = %u\r\n",
		Buffer[0],version ,revision,version ,revision);
		
		if (version <3)
		{
			pulsestomm=PULSE_TO_MM_FIRMWARE_S_3;
			mmstospeed=MM_S_TO_SPEED_FIRMWARE_S_3;
		}
		else
		{
			pulsestomm=PULSE_TO_MM_FIRMWARE_BE_3;
			mmstospeed=MM_S_TO_SPEED_FIRMWARE_BE_3;
		}
	}
	
	if(kh3_battery_voltage((char *)Buffer, 0, dsPic))
	{
		printf("\r\n%c,%3.3u,%3.3u => battery voltage = %u.%uV\r\n",
		Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
		(Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
	}
	

			motspeed= (long)(40.0*mmstospeed);

	while(1)
	{	
		index=0;
		value=31;

		kmot_SetPoint( mot1 , kMotRegSpeed , motspeed );
		kmot_SetPoint( mot2 , kMotRegSpeed , motspeed );		


		for (i=0;i<5;i++)
		{
			if(kh3_measure_us((char *)Buffer, i+1, dsPic))
			{
				usvalues[i] = (Buffer[0*8+3] | Buffer[0*8+4]<<8);
			}
		}

		if((usvalues[1]<10&&usvalues[1]!=0)||(usvalues[2]<20&&usvalues[2]!=0)||(usvalues[3]<10&&usvalues[3]!=0))
		{
			kmot_SetPoint( mot1 , kMotRegSpeed , 0 );
			kmot_SetPoint( mot2 , kMotRegSpeed , 0 );
			kgripper_Arm_Set_Search_Limit( Arm, 1);	
		}
	}
}


