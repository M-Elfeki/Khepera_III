#include <korebot/korebot.h>
static knet_dev_t * dsPic;
static knet_dev_t * mot1;
static knet_dev_t * mot2;


#define PULSE_TO_MM_FIRMWARE_S_3 0.04629 // for version <3.0
#define PULSE_TO_MM_FIRMWARE_BE_3 0.03068


#define MM_S_TO_SPEED_FIRMWARE_S_3 144.01 // for version <3.0
#define MM_S_TO_SPEED_FIRMWARE_BE_3 218.72


#define IR_BAR_LEN 15

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
	
	while(1)
	{
		sleep(2);
		
		lpos = kmot_GetMeasure(mot1, kMotRegPos);
		rpos = kmot_GetMeasure(mot2, kMotRegPos);
		fpos = 20.0*10.0/pulsestomm;
		
		printf("\nMoving forward %.1f cm in %.1f pulses with position control\
		(encoders: left %ld | right%ld)\n",20.0,fpos,lpos+(long)fpos,rpos+(long)fpos);

		kmot_SetPoint(mot1, kMotRegPosProfile, lpos+(long)fpos);
		kmot_SetPoint(mot2, kMotRegPosProfile, rpos+(long)fpos);

		printf("\n wait 10s \n");
		sleep(10);

		motspeed= (long)(-40.0*mmstospeed);
		kmot_SetPoint( mot1 , kMotRegSpeed , motspeed );
		kmot_SetPoint( mot2 , kMotRegSpeed , motspeed );
		printf("\nMoving backward %.1f cm at %.1f mm/s (internal speed %d) with\
		speed control\n",20.0,40.0,motspeed);

		sleep(5);

		kmot_SetPoint( mot1 , kMotRegSpeed , 0 );
		kmot_SetPoint( mot2 , kMotRegSpeed , 0 );

		sleep(5);

		if(kh3_proximity_ir((char *)Buffer, dsPic))
		{
			for (i=0;i<11;i++)
			{
			sensors[i]=(Buffer[i*2+1] | Buffer[i*2+2]<<8);
			n=(int)(sensors[i]/4096.0*IR_BAR_LEN);
			if (n==0)
			sprintf(bar[i],"|\33[%dC>|",IR_BAR_LEN-1);
			else
			if (n>=IR_BAR_LEN-1)
			sprintf(bar[i],"|>\33[%dC|",IR_BAR_LEN-1);
			else
			sprintf(bar[i],"|\33[%dC>\33[%dC|",IR_BAR_LEN-1-n,n);
			}
			
			printf("\n   near          far\nback left   : %4.4u\%s\nleft 90     : %4.4u  %s\nleft 45   : %4.4u %s\nfront left  : %4.4u\%s\nfront right : %4.4u    %s\nright 45: %4.4u %s\nright 90:%4.4u\%s\nback right : %4.4u     %s\nback    :%4.4u %s\nbottom right:  %4.4u\%s\nbottom left : %4.4u    %s\ntime stamp : %lu\r\n",
					 sensors[0], bar[0], sensors[1], bar[1], sensors[2], bar[2],
					 sensors[3], bar[3], sensors[4], bar[4], sensors[5], bar[5],
					 sensors[6], bar[6], sensors[7], bar[7], sensors[8], bar[8],
					 sensors[9], bar[9], sensors[10], bar[10],
					 ((Buffer[19] | Buffer[20]<<8) | (Buffer[21] | Buffer[22]<<8)<<16));
		}
		
		index=0;
		value=31;

		if(kh3_configure_os((char *)Buffer, index, value, dsPic))
			printf("\r\n US sensors configured.\r\n");
		else
			printf("\r\nconfigure OS error!\r\n");

		for (i=0;i<5;i++)
		{
			if(kh3_measure_us((char *)Buffer, i+1, dsPic))
			{
				usvalues[i] = (Buffer[0*8+3] | Buffer[0*8+4]<<8);
			}
			else
				printf("\r\ng, error...");
		}
		printf("US sensors : distance [cm]\nleft 90   : %3d\nleft 45: %3d \\nfront: %3d\nright 45 : %3d\nright 90 : %3d\n" ,
				usvalues[0],usvalues[1],usvalues[2],usvalues[3],usvalues[4]);
	}
}


