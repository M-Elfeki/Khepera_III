#include <korebot/korebot.h>
#define PULSE_TO_MM_FIRMWARE_S_3 0.04629
#define PULSE_TO_MM_FIRMWARE_BE_3 0.03068
#define MM_S_TO_SPEED_FIRMWARE_S_3 144.01
#define MM_S_TO_SPEED_FIRMWARE_BE_3 218.72
#define IR_BAR_LEN 15
static knet_dev_t * dsPic;
static knet_dev_t * mot1;
static knet_dev_t * mot2;

int main()
{
	char Buffer[100],bar[11][IR_BAR_LEN+5];
	int sensors[11],i,n, j=0, y=0, k=0;
	float pulsestomm=PULSE_TO_MM_FIRMWARE_S_3 ;
	float mmstospeed=MM_S_TO_SPEED_FIRMWARE_S_3 ;
	long motspeed = (long)(40.0*mmstospeed),iterator =0;
	char *fileName ="results.txt", ch, path[1035];
    	FILE *f = fopen(fileName, "w");
        fprintf(f, "\n=================================================\n\n\n");

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

	while(iterator<5)
	{
        	kmot_SetPoint( mot1 , kMotRegSpeed , motspeed );
	        kmot_SetPoint( mot2 , kMotRegSpeed , motspeed );
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
			if(sensors[9]>3000&& sensors[10]<3000)
			{
			    kmot_SetPoint( mot1 , kMotRegSpeed , -motspeed );
			}
			else if(sensors[10]>3000&& sensors[9]<3000)
			{
			    kmot_SetPoint( mot2 , kMotRegSpeed , -motspeed );
			}
			else if(sensors[10]<3000&&sensors[9]<3000)
			{
				if(iterator!=4)
				{
					kmot_SetPoint( mot1 , kMotRegSpeed , 0 );
					kmot_SetPoint( mot2 , kMotRegSpeed , 0 );
					FILE *fp = popen("iwlist eth0 scan", "r");
					while (fgets(path, sizeof(path)-1, fp) != NULL)
						fprintf(f, "%s", path); 
					fprintf(f, "\n=================================================\n\n\n");
					pclose(fp);
				}
				kmot_SetPoint( mot1 , kMotRegSpeed , motspeed );
			        kmot_SetPoint( mot2 , kMotRegSpeed , motspeed );
				sleep(1);				
				iterator++;
			}
		}
		
	}
	kmot_SetPoint( mot1 , kMotRegSpeed , 0 );
	kmot_SetPoint( mot2 , kMotRegSpeed , 0 );
}

