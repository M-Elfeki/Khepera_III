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
	short index, value;
	char usvalues[5];
	long motspeed;
	float pulsestomm=PULSE_TO_MM_FIRMWARE_S_3 ; 
	float mmstospeed=MM_S_TO_SPEED_FIRMWARE_S_3 ; 
	char fileName[] = "test.txt", searchStr[13]= "Signal level=", measurement[4], path[1035], ch;
	int  j, c, d, k, y, h, counter, sum, results[30];
	double average =0;
	FILE *f = fopen(fileName, "w");
	FILE *fp;
	
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
	
	
	long iterator=0;
			
	motspeed= (long)(40.0*mmstospeed);
	while(iterator<76)
	{	
		
		kmot_SetPoint( mot1 , kMotRegSpeed , motspeed );
		kmot_SetPoint( mot2 , kMotRegSpeed , motspeed );
		sleep(2);
		kmot_SetPoint( mot1 , kMotRegSpeed , 0 );
		kmot_SetPoint( mot2 , kMotRegSpeed , 0 );
		for(h =0;h<30;h++)
  		{
		        fp = popen("iwlist eth0 scan", "r");
		        while (fgets(path, sizeof(path)-1, fp) != NULL)
		            fprintf(f, "%s", path);
		}
		fclose(f);
		pclose(fp);

		fp = fopen(fileName,"r");
		while(!feof(fp))
		{
		        for(j=0; j<13; j++)
		        {
		            ch = fgetc(fp);
		            if(ch != searchStr[j])
                		break;
		        }
		        if(j == 13)
        		{
		            for(y =0;y<12;y++)
                		feof(fp);
		            measurement[0] = fgetc(fp);
		            feof(fp);
		            measurement[1] = fgetc(fp);
		            feof(fp);
		            measurement[2] = fgetc(fp);
		            feof(fp);
		            measurement[3]='\0';
		            sscanf(measurement, "%d", &results[counter]);
		            counter++;
        		}
	    	}
		for(k = 0; k<30;k++)
		        sum+=results[k];
		average = sum/30;
		printf("\n\n%f00\n\n", average);
		sleep(7);	
	}
}


