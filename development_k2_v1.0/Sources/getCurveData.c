#include <stdio.h>
#include <stdlib.h>
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
	char usvalues[5];
	long motspeed;
	float pulsestomm=PULSE_TO_MM_FIRMWARE_S_3 ; 
	float mmstospeed=MM_S_TO_SPEED_FIRMWARE_S_3 ; 
  	int status;
	
	
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
	
	char *fileName ="test.txt", ch, path[1035];
	int iterator =0, j=0, y=0;
    	FILE *f = fopen(fileName, "w");
        fprintf(f, "\n=================================================\n\n\n");
	motspeed= (long)(40.0*mmstospeed);
	kmot_SetPoint( mot1 , kMotRegSpeed , motspeed );
	kmot_SetPoint( mot2 , kMotRegSpeed , motspeed );

	while(iterator<24)
	{	
		FILE *fp = popen("iwlist eth0 scan", "r");
        	while (fgets(path, sizeof(path)-1, fp) != NULL)
	            fprintf(f, "%s", path);
        	fprintf(f, "\n=================================================\n\n\n");
	        iterator++;
        	pclose(fp);	
	}
	kmot_SetPoint( mot1 , kMotRegSpeed , 0 );
	kmot_SetPoint( mot2 , kMotRegSpeed , 0 );
}


