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
	int sensors[11],i,n;

	float pulsestomm=PULSE_TO_MM_FIRMWARE_S_3 ;
	float mmstospeed=MM_S_TO_SPEED_FIRMWARE_S_3 ;
	long motspeed = (long)(40.0*mmstospeed);

	char fileName[] = "test.txt", searchStr[13]= "Signal level=", searchStr2[6]="ESSID:", measurement[4]
	, essid[12], path[1035], ch;
	char measurements [200][4], essids[200][12];
	int j, y, iterator=0;

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

    while(iterator<23)
	{
	    FILE *f = fopen(fileName, "w");
        FILE *fp = popen("iwlist eth0 scan", "r");
        while (fgets(path, sizeof(path)-1, fp) != NULL)
		    fprintf(f, "%s", path);
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
            }
        }
        pclose(fp);
        fp = fopen(fileName,"r");
        while(!feof(fp))
		{
            for(j=0; j<6; j++)
		    {
                ch = fgetc(fp);
                if(ch != searchStr2[j])
                    break;
            }
            if(j == 6)
            {
                for(y =0;y<5;y++)
                    feof(fp);
                essid[0] = fgetc(fp);feof(fp);
                essid[1] = fgetc(fp);feof(fp);
                essid[2] = fgetc(fp);feof(fp);
                essid[3] = fgetc(fp);feof(fp);
                essid[4] = fgetc(fp);feof(fp);
                essid[5] = fgetc(fp);feof(fp);
                essid[6] = fgetc(fp);feof(fp);
                essid[7] = fgetc(fp);feof(fp);
                essid[8] = fgetc(fp);feof(fp);
                essid[9] = fgetc(fp);feof(fp);
                essid[10] = fgetc(fp);feof(fp);
                essid[11]='\0';
            }
        }
        measurements[iterator][0] = measurement[0];
        measurements[iterator][1]= measurement[1];
        measurements[iterator][2] = measurement[2];
        measurements[iterator][3] = measurement[3];
        essids[iterator][0]= essid[0];essids[iterator][1]= essid[1];essids[iterator][2]= essid[2];
        essids[iterator][3]= essid[3];essids[iterator][4]= essid[4];essids[iterator][5]= essid[5];
        essids[iterator][6]= essid[6];essids[iterator][7]= essid[7];essids[iterator][8]= essid[8];
        essids[iterator][9]= essid[9];essids[iterator][10]= essid[10];
		iterator++;
	}
	FILE *f = fopen(fileName, "w");
	for(j =0;j<100;j++)
	{
	    fprintf(f, "%s is %s\n", essids[j], measurements[j]);
        fprintf(f, "==============================\n\n\n");
	}
    fclose(f);
}

