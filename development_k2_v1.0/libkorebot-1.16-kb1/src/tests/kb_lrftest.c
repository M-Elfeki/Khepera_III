/*! 
 * \file   kb_lrftest.c simple led range finder test executable             
 *
 * \brief 
 *         This module provides useful basic examples to interface with the
 *         LedRangeFinder library for korebot
 *
 * \author   Arnaud Maye (K-Team SA)
 *
 * \note     Copyright (C) 2005 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     
 */



#include <signal.h>
#include <korebot/korebot.h>

static int quitReq = 0;
static int lrfHandle = 0x1234;
static int gpioOKE = 0;
static int lrfOKE = 0;
static int snOKE = 0;
static int measCnt = 0;
static char *LRF_SN;
static char *KBA_SN;
static FILE *outfile;

unsigned short DataDist[121];

fd_set		fd_set1;	

/* various rs232 lrf protocol defines */
#define MAX_TIMEOUT_IN_MS	500
#define MAX_DATA_BYTES		246

/* various gpio related defines */
#define	PXA_INT0	8
#define	OUT     	0
        
#define device "/dev/tts/2"

/*--------------------------------------------------------------------*/
/*!
 * "clear" the console screen
 */
void clrscr(void)
{
  char a[80];
  printf("\033[2J");            /* erase the whole console */
  printf("\033[0;0f");          /* Move cursor to the top left */
}


void initTime( struct timeval *timestruct )
{
   gettimeofday(timestruct, NULL);
}  

unsigned long deltaTime( struct timeval *timeS1)
{
   struct timeval loc;
   
   gettimeofday(&loc, NULL);
   
   timersub(&loc, timeS1, &loc);
   timeradd(timeS1, &loc, timeS1);
   
   return loc.tv_usec;
}


int gpioinit( int argc, char * argv[], void * data)
{
   int rc;
   /* Initialise the GPIO library */
   if((rc = kb_init( argc , argv )) > -1 )
   {
      /* Init gpio module */
      kb_gpio_init();
      
      /* Set GPIO8 pin as gpio, ndlr: because this pin doesnt have
         an alternate function, it is not required... */
      kb_gpio_function(PXA_INT0,0);  
  
      /* Set GPIO8 (PXA_INT0) as output */
      kb_gpio_dir(PXA_INT0, OUT);
   }
   
   if(rc < 0)
      printf("unable to initialise the gpio library\r\n");
   else
   {
      printf("the gpio library has been initialized\r\n");
      gpioOKE = 1;
   }
   
}

int lrfon( int argc, char * argv[], void * data)
{
   if(gpioOKE)
   {
      printf("lrf is now powered...\r\n");
      kb_lrf_pwrOn();   
      /* Wait that 1.5 seconds before to query the lrf - 1s specified by datasheet */ 
      usleep(1500*1000);
   }
   else
   {
      printf("lrfon : error the gpio library need to be initialized, please use gpioinit\r\n");
   }
      
   
}

int lrfoff( int argc, char * argv[], void * data)
{
   if(gpioOKE)
   {
      kb_lrf_pwrOff();
      printf("lrf is now unpowered...\r\n"); 
   }
   else
   {
      printf("lrfoff : error the gpio library need to be initialized, please use gpioinit\r\n");
   }
   
}

int setsn( int argc, char *argv[], void * data)
{
   strcat(LRF_SN, argv[1]);
   strcat(KBA_SN, argv[2]);
   
   if(*LRF_SN)
   {
      printf("Led Range Finder pcb serial number is : %s\n", LRF_SN);
   }
   else
      printf("no LRF serial number has been set, please retry\n");
      
   if(*KBA_SN)
   {
      printf("Korebot adaptor pcb serial number is : %s\n", KBA_SN);
   }
   else
      printf("no KBA serial number has been set, please retry\n");   
   
   snOKE = 1;
}
 
int zeroCnt( int argc, char *argv[], void * data)
{
   printf("zeroing the measure counter\r\n");
   measCnt = 0;
   
}  

char *lrfheader( )
{
    char *locbuf, *header;
    
    printf("lrf_header : preparing the measure header..\n");
    
    locbuf = (char *)malloc(200);
    memset(locbuf, 0, 200);
    
    header = (char *)malloc(2000);
    memset(header, 0, 2000);
    
    strcat(header, "LED RANGE FINDER MEASUREMENT\n----------------------------\n\n");
    
    if(*LRF_SN)
    {
       printf("lrf_header : appending lrf sernum to the header..\n");
       strcat(header, "the led range finder pcb serial number is ");
       strcat(header, LRF_SN);
       strcat(header, "\n");
    }
    else
       printf("lrf_header : no lrf sernum to append to the header..\n");
    
    if(*KBA_SN)
    {
       printf("lrf_header : appending kba sernum to the header..\n");
       strcat(header, "the kore bot adaptor pcb serial number is ");
       strcat(header, KBA_SN);
       strcat(header, "\n\n");
    }
    else
       printf("lrf_header : no kba sernum to append to the header..\n");
    
   
    if(outfile)
    {
       printf("lrf_header : putting the measurement  header as file header..\n");
       fprintf(outfile, header);
    }
    else
       printf("lrf_header : error file not been set, please use the setfile command..\n");
    
    free(locbuf);
    free(header);

}    
   
int setfile( int argc, char * argv[], void * data)
{
    outfile = fopen(argv[1], "w");
    
    if(outfile)
    {
       printf("file %s has been openned with success\r\n", argv[1]);
    }
    else
       printf("unable to open the %s file\r\n", *argv[1]);
       
       
}


int lrfinit( int argc, char * argv[], void * data)
{
   lrfHandle = kb_lrf_Init(device);       
       
}

int lrfmeasure( int argc, char * argv[], void * data)
{
   int i, j, k;
   int max;
   char *measTbl, *measBuf;
   
   
   measTbl = (char*)malloc(20000); memset(measTbl, 0, 20000);
   measBuf = (char*)malloc(20000); memset(measBuf, 0, 20000);
   
   max = atoi(argv[1]);
   
   lrfHandle = kb_lrf_Init(device);
   
   lrfheader();
   
   printf("lrfmeasure : goin to proceeed with %d measure...\r\n", max); 
   
//   initTime( &time_ );
   
   for (k=0; k < max; k++)
   {
      printf("lrfmeasure : measure %d \n", k); 
         
      clrscr();
      
      if (kb_lrf_GetDistances(lrfHandle) >= 0)
      {
         measCnt++;
  //       sprintf(measBuf, "\nTime spent on measure %d is %dms\n", measCnt, deltaTime( &curtime )/1000);
         strcat(measTbl, measBuf);

         for (i=0, j = 0; i<121; i++, j++)
         {
               if( j == 8)
               {
                  j = 0;
                  strcat(measTbl, "\n");
                  printf("\n");
               }

               /* color non zero measure in red */
               sprintf (measBuf, "Dir %3.3d: ", i);
               printf ("Dir %3.3d: ", i);

               strcat(measTbl, measBuf);

               if(kb_lrf_DistanceData[i]) 
               {
                   printf ("\e[31m");

               }
               else
               { 
                   printf("\e[30m");
               }

               sprintf (measBuf, "%6.6d ", kb_lrf_DistanceData[i]);
               printf ("%6.6d ", kb_lrf_DistanceData[i]);


               printf("\e[30mmm ");

         }

         strcat(measTbl, "\n");

         if(outfile)
         {
            fprintf(outfile, measTbl);
         }
            
         memset(measTbl, 0, 20000);   
      }
      j++;
      
   }
   
   gpioinit(0, NULL, NULL);
   lrfoff(0, NULL, NULL);
   
   free(measTbl);
   free(measBuf);
   
}      
   




/*--------------------------------------------------------------------*/
/*! Quit the program.
 */
int quit( int argc , char * argv[] , void * data) 
{
  quitReq = 1;
}

int emergencystop( int argc , char * argv[] , void * data)
{
    gpioinit(0, NULL, NULL);
    
    lrfoff(0, NULL, NULL);
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
  { "emstop"          , 0,  0 , emergencystop },
  { "zeromeasure"     , 0 , 0 , zeroCnt },
  { "setserialnum"    , 2 , 2 , setsn },
  { "setfile"         , 1 , 1 , setfile },
  { "gpioinit"        , 0 , 0 , gpioinit },
  { "lrfinit"        , 0 , 0 , lrfinit },
  { "lrfon"           , 0 , 0 , lrfon },
  { "lrfoff"          , 0 , 0 , lrfoff },
  { "lrfmeasure"      , 0 , 1 , lrfmeasure },
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

/*--------------------------------------------------------------------*/
/*! Main program to process the command line. 
 *
 */
static char buf[1024];

int main( int argc , char * argv[] )
{
  int rc,ver;
  struct timeval starttime;
  struct timeval fintime;

  /* Get start time */
  gettimeofday(&starttime, NULL);

  /* allocate and zero some memory for the serial numbers */
  LRF_SN = (char *)malloc(200); KBA_SN = (char *)malloc(200);
  memset(LRF_SN, 0, 200); memset(KBA_SN, 0, 200);

  /* reset the screen */
  clrscr();
  
  /* Set the libkorebot debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  printf("Led Range Finder Test Program - A.Maye (K-Team) \r\n");
  
   /* parse commands */
  while (!quitReq) {
    printf("\n> ");

    if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) {
      buf[strlen(buf)-1] = '\0';
      kb_parse_command( buf , cmds , NULL);
    }
  }
  gettimeofday(&fintime, NULL);
  timersub(&fintime, &starttime, &fintime);
  printf("Total test time is %ds %dus\n", fintime.tv_sec , fintime.tv_usec);
  
  /* free the memory allocated for the hardware serial numbers */
  free(LRF_SN); free(KBA_SN);

  
}
