#include <korebot/korebot.h>
#include <stdlib.h>
#include <stdio.h>
static knet_dev_t * mot1;
static knet_dev_t * mot2;
static knet_dev_t * dsPic;

#define MM_S_TO_SPEED_FIRMWARE_S_3 144.01 // for version <3.0
#define MM_S_TO_SPEED_FIRMWARE_BE_3 218.72

#define PULSE_TO_MM_FIRMWARE_S_3 0.04629 // for version <3.0
#define PULSE_TO_MM_FIRMWARE_BE_3 0.03068

int main()
{
	float fpos;
	long lpos,rpos, motspeed;
	char Buffer[100];
	char buffer[3];
	int index =0, i, n,revision,version;
	float pulsestomm=PULSE_TO_MM_FIRMWARE_S_3 ; 
	float mmstospeed=MM_S_TO_SPEED_FIRMWARE_S_3 ;

	kh3_init();

	dsPic = knet_open( "Khepera3:dsPic", KNET_BUS_I2C , 0 , NULL );
	mot1 = knet_open( "Khepera3:mot1", KNET_BUS_I2C , 0 , NULL );
	mot2 = knet_open( "Khepera3:mot2", KNET_BUS_I2C , 0 , NULL );

	if(kh3_revision((char *)Buffer, dsPic))
	{
		version=(Buffer[1] | Buffer[2]<<8);
		revision=(Buffer[3] | Buffer[4]<<8);
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
	
	while(index<5)
	{
		index++;
		if(kh3_battery_voltage((char *)Buffer, 0, dsPic))
		{
			buffer[0] = (Buffer[1] | Buffer[2]<<8);
			buffer[1] = '.';
			buffer[2] = (Buffer[3] | Buffer[4]<<8);
			printf("Buffer is %u.%u\n", buffer[0], buffer[1]);
			double temp = strtod(buffer, NULL);
			printf("double is %f\n", temp);
		}
	}
}
