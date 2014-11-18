 #include <korebot/korebot.h>
 
 #define MAXBUFFERSIZE 100
 
 static int quitReq = 0;
 static char buf[1024];
 
 
 static knet_dev_t * dsPic;
 static knet_dev_t * mot1;
 static knet_dev_t * mot2;


int kb_kbhit(void)
 {
   int cnt = 0;
   int error;
   static struct termios Otty, Ntty;
 
 
   tcgetattr( 0, &Otty);
   Ntty = Otty;
 
   Ntty.c_iflag          = 0;       /* input mode                */
   Ntty.c_oflag          = 0;       /* output mode               */
   Ntty.c_lflag         &= ~ICANON; /* raw mode */
   Ntty.c_cc[VMIN]       = CMIN;    /* minimum time to wait      */
   Ntty.c_cc[VTIME]      = CTIME;   /* minimum characters to wait for */
 
   if (0 == (error = tcsetattr(0, TCSANOW, &Ntty))) {
     error += ioctl(0, FIONREAD, &cnt);
     error += tcsetattr(0, TCSANOW, &Otty);
   }
 
   return ( error == 0 ? cnt : -1 );
 }



void kb_clrscr(void)
 {
   printf("\033[2J");            /* erase the whole console */
   printf("\033[1;1f");          /* Move cursor to the top left */
 }
 
 
 /*--------------------------------------------------------------------*/
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
           kmot_ConfigurePID( hDev , kMotRegSpeed , 620, 3 , 10 );
           kmot_ConfigurePID( hDev,kMotRegPos,600,20,30);
           kmot_SetSpeedProfile(hDev,15000,30);
 
           return 1;
   }
   else
   {
           printf("initMot error, handle cannot be null\r\n");
           return 0;
   }
 }
                                                                                                         
 
 /*--------------------------------------------------------------------*/
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
 int proxIR( int argc, char * argv[], void * data)
 {
   char Buffer[MAXBUFFERSIZE];
   
   int sensors[11],i,n;
   
   
   char bar[9][20];
   
   while (!kb_kbhit())
   {
                 if(kh3_proximity_ir((char *)Buffer, dsPic))
                 {       
                         for (i=0;i<9;i++)
                         {
                                 sensors[i]=(Buffer[i*2+1] | Buffer[i*2+2]<<8);
                                 
                                 n=(int)(sensors[i]/4096.0*15);
                                 
                                 if (n==0)
                                         sprintf(bar[i],"|\33[%dC>|",14);
                                 else
                                         if (n>=14)
                                                 sprintf(bar[i],"|>\33[%dC|",14);
                                         else
                                                 sprintf(bar[i],"|\33[%dC>\33[%dC|",14-n,n);
                                 
                         }
         
                         
                         printf("\nPush RETURN to stop!\n                    near            far\nback left   : %4.4u  %s\nleft 90     : %4.4u  %s\nleft 45     : %4.4u  %s\nfront left  : %4.4u  %s\nfront right : %4.4u  %s\nright 45    : %4.4u  %s\nright 90    : %4.4u  %s\nback right  : %4.4u  %s\nback        : %4.4u  %s\ntime stamp  : %lu\r\n",
                           sensors[0],bar[0],  sensors[1],bar[1],
                          sensors[2],bar[2],  sensors[3],bar[3],
                          sensors[4],bar[4],  sensors[5],bar[5],
                          sensors[6],bar[6],  sensors[7],bar[7],
                          sensors[8], bar[8],
                          ((Buffer[19] | Buffer[20]<<8) | (Buffer[21] | Buffer[22]<<8)<<16));
                 }
                 else
                         printf("\r\nn, error...\r\n");
                         
                 usleep(200000); 
                 kb_clrscr();    
         }
 }
                 
 /*--------------------------------------------------------------------*/
 int ambIR( int argc, char * argv[], void * data)
 {
   char Buffer[MAXBUFFERSIZE];
   
   int sensors[11],i,n;
   
   
   char bar[9][20];
   
   while (!kb_kbhit())
   {
                 if(kh3_ambiant_ir((char *)Buffer, dsPic))
                         {       
                         for (i=0;i<9;i++)
                         {
                                 sensors[i]=(Buffer[i*2+1] | Buffer[i*2+2]<<8);
                                 
                                 n=(int)(sensors[i]/4096.0*15);
                                 
                                 if (n==0)
                                         sprintf(bar[i],"|\33[%dC>|",14);
                                 else
                                         if (n>=14)
                                                 sprintf(bar[i],"|>\33[%dC|",14);
                                         else
                                                 sprintf(bar[i],"|\33[%dC>\33[%dC|",14-n,n);
                                 
                         }
         
                         
                         printf("\nPush RETURN to stop!\n                    dark       bright\nback left   : %4.4u  %s\nleft 90     : %4.4u  %s\nleft 45     : %4.4u  %s\nfront left  : %4.4u  %s\nfront right : %4.4u  %s\nright 45    : %4.4u  %s\nright 90    : %4.4u  %s\nback right  : %4.4u  %s\nback        : %4.4u  %s\ntime stamp  : %lu\r\n",
                           sensors[0],bar[0],  sensors[1],bar[1],
                          sensors[2],bar[2],  sensors[3],bar[3],
                          sensors[4],bar[4],  sensors[5],bar[5],
                          sensors[6],bar[6],  sensors[7],bar[7],
                          sensors[8], bar[8],
                          ((Buffer[19] | Buffer[20]<<8) | (Buffer[21] | Buffer[22]<<8)<<16));
                 }
                 else
                         printf("\r\no, error...\r\n");
                        
                 usleep(200000); 
                 kb_clrscr();    
                         
   }
   
  
 }
 
 /*--------------------------------------------------------------------*/
 int voltageBAT( int argc, char * argv[], void * data)
 {
   char Buffer[MAXBUFFERSIZE];
   short argument;
 
   argument = atoi(argv[1]);
   if(kh3_battery_voltage((char *)Buffer, argument, dsPic)){
         switch(argument){
                 case 0:         /* Read Voltage */
                   printf("\r\n%c,%3.3u,%3.3u => battery voltage = %u.%uV\r\n",
                    Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
                    (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
                   break;
                 case 1:         /* Read Current */
                   printf("\r\n%c,",Buffer[0]);
                   if ((Buffer[2] & 0x80) != 0){
                         Buffer[2] = Buffer[2] & 0x7F;
                         printf("-%3.3u,%3.3u => battery current = -%u.%uA\r\n",
                            (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
                            (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
                   }
                   else{
                         printf("%3.3u,%3.3u => battery current = %u.%uA\r\n",
                          (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
                          (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
                   }
                   break;
                 case 2:         /* Read Average Current */
                   printf("\r\n,%c,",Buffer[0]);
                   if ((Buffer[2] & 0x80) != 0){
                         Buffer[2] = Buffer[2] & 0x7F;
                         printf("-%3.3u,%3.3u => battery average current = -%u.%uA\r\n",
                            (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
                            (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
                   }
                   else{
                         printf("%3.3u,%3.3u => battery average current = %u.%uA\r\n",
                          (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
                          (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
                   }                   break;
                 case 3:         /* Read absolute remaining capacity */
                   printf("\r\n%c,%3.3u => battery remaining capacity = %umAh\r\n",
                    Buffer[0], (Buffer[1] | Buffer[2]<<8), 
                    (Buffer[1] | Buffer[2]<<8) );
                   break;
                 case 4:         /* Read Temperature */
                   printf("\r\n%c,%3.3u,%3.3u => temperature = %u.%uC\r\n",
                    Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
                    (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
                   break;
                 case 5:         /* Read relative remaining capacity */
                   printf("\r\n%c,%3u, => battery remaining capacity = %u%%\r\n",
                    Buffer[0], (Buffer[1] | Buffer[2]<<8), 
                    (Buffer[1] | Buffer[2]<<8));
                   break;
 
 
                 default:
                   printf("argument error\r\n");
         }
   }
   else
           printf("\r\nv, error...\r\n");
 }
 
 /*--------------------------------------------------------------------*/
 int tstampRST( int argc, char * argv[], void * data)
 {
   char Buffer[MAXBUFFERSIZE];
 
   if(kh3_reset_tstamp((char *)Buffer, dsPic))
         printf("\r\n%c\r\n", Buffer[0]);
   else
         printf("\r\nz, error...\r\n");
 }
 
 /*--------------------------------------------------------------------*/
 int revisionOS( int argc, char * argv[], void * data)
 {
   char Buffer[MAXBUFFERSIZE];   /* buffer that handle the returned datas from kh3 */
   unsigned int ver ;
 
 
 
 
   if(kh3_revision((char *)Buffer, dsPic)){
         printf("\r\n%c,%4.4u,%4.4u => Version = %u, Revision = %u\r\n", 
            Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
            (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
    kmot_GetFWVersion( mot1 , &ver );
     printf("Right Motor Firmware v%u.%u\n" , 
         KMOT_VERSION(ver) , KMOT_REVISION(ver) );
 
    kmot_GetFWVersion( mot2 , &ver );
     printf("Left Motor Firmware v%u.%u\n" , 
         KMOT_VERSION(ver) , KMOT_REVISION(ver) );
 
 
   }
   else
         printf("\r\nb, error...\r\n");  
 }
 
 /*--------------------------------------------------------------------*/
 int configureOS( int argc, char * argv[], void * data)
 {
   char Buffer[MAXBUFFERSIZE];   /* buffer that handle the returned datas from kh3 */
   short index;                  /* variable that handles index */
   short value;                  /* variable that handle value */
 
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
 int measureUS( int argc, char * argv[], void * data)
 {
   char Buffer[MAXBUFFERSIZE];
   int i;
   short usnoise;        /* Noise on the given adc pin when no us is received */
   short echonbr;        /* Number of echo part of this us measure */
   short usvalue;        /* Variable that handle distances */
   short usampl;         /* Variable than nandle amplitudes */
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
                         usvalue = (Buffer[i*8+3] | Buffer[i*8+4]<<8);
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
 int getallus( int argc, char * argv[], void * data)
 {
   char Buffer[MAXBUFFERSIZE];
   int i;
   short usnoise;        /* Noise on the given adc pin when no us is received */
   short echonbr;        /* Number of echo part of this us measure */
   short usvalue;        /* Variable that handle distances */
   short usampl;         /* Variable than nandle amplitudes */
   short argument; 
  
   char usvalues[5];
         
         
         short  index=0,value=31; // all us sensors active
         
 
   if(kh3_configure_os((char *)Buffer, index, value, dsPic))
         printf("\r\n%c\r\n", Buffer[0]);
   else
                 printf("\r\nconfigure OS error!\r\n");
         
   while (!kb_kbhit())
   {
         
         for (i=0;i<5;i++)
         {
         
                         if(kh3_measure_us((char *)Buffer, i+1, dsPic))
                         {
                                 usvalues[i] = (Buffer[0*8+3] | Buffer[0*8+4]<<8);                                       
                         }
                         else
                                 printf("\r\ng, error...");
                 }
         
                 printf("Push RETURN to stop!\n\nUS sensor : distance [cm]\nleft 90   : %3d\nleft 45   : %3d\nfront     : %3d\nright 45  : %3d\nright 90  : %3d\n",usvalues[0],usvalues[1],usvalues[2],usvalues[3],usvalues[4]);
         
                 usleep(200000); 
                 kb_clrscr();    
         }       
                                 
 }
 
 /*--------------------------------------------------------------------*/
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
 #define BR_IRGAIN 30
 #define fwSpeed 20
 #define MIN_SPEED 10
 #define RotSpeedL 50
 #define RotSpeedR -50
  
 int braitenbergAvoidance( int argc , char * argv[] , void * data)
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
 
 
   char cmd[3] = { 2, 'N', 0};
 
   /* Get the current position values */
   prevpos_left = kmot_GetMeasure( mot1 , kMotRegPos );
   prevpos_right = kmot_GetMeasure( mot2 , kMotRegPos );
 
 
         printf("\nPush RETURN to stop!\n");
 
   while(!kb_kbhit())
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
 
 
     //printf("lspd = %d rspd = %d\r\n", left_speed, right_speed);                       
 
     usleep(20000); 
   }
   
   kmot_SetMode( mot1 , kMotModeStopMotor );
         kmot_SetMode( mot2 , kMotModeStopMotor );
   
 }
 
 
 /*--------------------------------------------------------------------*/
 int quit( int argc , char * argv[] , void * data)
 {
   quitReq = 1;
 }
 
 
 int help( int argc , char * argv[] , void * data);
 /*--------------------------------------------------------------------*/
 static kb_command_t cmds[] = {
  { "quit"            , 0 , 0 , quit } ,
  { "exit"            , 0 , 0 , quit } ,
  { "bye"             , 0 , 0 , quit } ,
  { "setcfg"          , 2 , 2 , configureOS },
  { "getrev"          , 0 , 0 , revisionOS },
  { "getbat"          , 1 , 1 , voltageBAT },
  { "rststamp"        , 0 , 0 , tstampRST },    
  { "getambir"        , 0 , 0 , ambIR },
  { "getproxir"       , 0 , 0 , proxIR },
  { "getus"           , 1 , 1 , measureUS },
  { "getallus"           , 0 , 0 , getallus },
  { "setmotspeed"     , 2 , 2 , motSpeed },
  { "setmotmove"      , 2 , 2 , motMove },
  { "motstop"         , 0 , 0 , motStop },
  { "braiten"         , 0 , 0 , braitenbergAvoidance},
  { "help"            , 0 , 0 , help } ,
  { NULL              , 0 , 0 , NULL }
 };
 
 /*--------------------------------------------------------------------*/
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
 int main( int arc, char *argv[])
 {
   char Buffer[MAXBUFFERSIZE];
   char i;
 
 
   printf("Khepera3 test program (C) K-Team S.A\r\n");
 
   if(!initKH3())
   {
     printf("Init OK...\r\n");
 
    kh3_revision((char *)Buffer, dsPic);
    printf("\r\n%c,%4.4u,%4.4u => Version = %u, Revision = %u\r\n",
            Buffer[0], (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8),
            (Buffer[1] | Buffer[2]<<8), (Buffer[3] | Buffer[4]<<8));
 
 
         
 
     while (!quitReq) 
     {
 
 
       printf("\n> ");
 
       if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) 
       {
                                 buf[strlen(buf)-1] = '\0';
                                 kb_parse_command( buf , cmds , NULL);
       }
     }
 
     printf("Exiting...\r\n");
     
     
     return 0;
         }
         else
           printf("Fatal error, unable to initialize\r\n");
           
          return -1; 
 }
 
 

