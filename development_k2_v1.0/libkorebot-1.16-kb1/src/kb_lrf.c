/*--------------------------------------------------------------------
 * lrf.c - KoreBot Library - LedRangeFinder Support
 *--------------------------------------------------------------------
 * $Id: kb_lrf.c,v 1.5 2005/07/03 07:37:55 pbureau Exp $
 *--------------------------------------------------------------------	
 * $Author: pbureau $
 * $Date: 2005/07/03 07:37:55 $
 * $Revision: 1.5 $
 *--------------------------------------------------------------------*/
#include "kb_lrf.h"
#include "kb_gpio.h"
#include "korebot.h"

/*! 
 * \file   kb_lrf.c Support for KoreBot Led Range Finder.             
 *
 * \brief 
 *         This module provides useful basic facilities to use the
 *         KoreBot with the LedRangeFinder card
 *
 * \author   Arnaud Maye (K-Team SA)
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     
 */
 

#define DISTANCE_READINGS_TO_AVERAGE	3

#define MAX_TIMEOUT_IN_MS	500
#define MAX_DATA_BYTES		246

#define device "/dev/tts/2"
/* various defines */
#define	PXA_INT0	8
#define	OUT	0

/* Table for ((int) (ceil(4.0 * ((float) i) / 3.0)))  replacement */
unsigned short calcTable[] = {
   0x0000, 0x0002, 0x0003, 0x0004, 0x0006, 0x0007, 0x0008, 0x000A, 0x000B, 0x000C, 
   0x000E, 0x000F, 0x0010, 0x0012, 0x0013, 0x0014, 0x0016, 0x0017, 0x0018, 0x001A, 
   0x001B, 0x001C, 0x001E, 0x001F, 0x0020, 0x0022, 0x0023, 0x0024, 0x0026, 0x0027, 
   0x0028, 0x002A, 0x002B, 0x002C, 0x002E, 0x002F, 0x0030, 0x0032, 0x0033, 0x0034, 
   0x0036, 0x0037, 0x0038, 0x003A, 0x003B, 0x003C, 0x003E, 0x003F, 0x0040, 0x0042, 
   0x0043, 0x0044, 0x0046, 0x0047, 0x0048, 0x004A, 0x004B, 0x004C, 0x004E, 0x004F, 
   0x0050, 0x0052, 0x0053, 0x0054, 0x0056, 0x0057, 0x0058, 0x005A, 0x005B, 0x005C, 
   0x005E, 0x005F, 0x0060, 0x0062, 0x0063, 0x0064, 0x0066, 0x0067, 0x0068, 0x006A, 
   0x006B, 0x006C, 0x006E, 0x006F, 0x0070, 0x0072, 0x0073, 0x0074, 0x0076, 0x0077, 
   0x0078, 0x007A, 0x007B, 0x007C, 0x007E, 0x007F, 0x0080, 0x0082, 0x0083, 0x0084, 
   0x0086, 0x0087, 0x0088, 0x008A, 0x008B, 0x008C, 0x008E, 0x008F, 0x0090, 0x0092, 
   0x0093, 0x0094, 0x0096, 0x0097, 0x0098, 0x009A, 0x009B, 0x009C, 0x009E, 0x009F, 
   0x00A0, 0x00A2, 0x00A3, 0x00A4, 0x00A6, 0x00A7, 0x00A8, 0x00AA, 0x00AB, 0x00AC, 
   0x00AE, 0x00AF, 0x00B0, 0x00B2, 0x00B3, 0x00B4, 0x00B6, 0x00B7, 0x00B8, 0x00BA, 
   0x00BB, 0x00BC, 0x00BE, 0x00BF, 0x00C0, 0x00C2, 0x00C3, 0x00C4, 0x00C6, 0x00C7, 
   0x00C8, 0x00CA, 0x00CB, 0x00CC, 0x00CE, 0x00CF, 0x00D0, 0x00D2, 0x00D3, 0x00D4, 
   0x00D6, 0x00D7, 0x00D8, 0x00DA, 0x00DB, 0x00DC, 0x00DE, 0x00DF, 0x00E0, 0x00E2, 
   0x00E3, 0x00E4, 0x00E6, 0x00E7, 0x00E8, 0x00EA, 0x00EB, 0x00EC, 0x00EE, 0x00EF, 
   0x00F0, 0x00F2, 0x00F3, 0x00F4, 0x00F6, 0x00F7, 0x00F8, 0x00FA, 0x00FB, 0x00FC, 
   0x00FE, 0x00FF, 0x0100, 0x0102, 0x0103, 0x0104, 0x0106, 0x0107, 0x0108, 0x010A, 
   0x010B, 0x010C, 0x010E, 0x010F, 0x0110, 0x0112, 0x0113, 0x0114, 0x0116, 0x0117, 
   0x0118, 0x011A, 0x011B, 0x011C, 0x011E, 0x011F, 0x0120, 0x0122, 0x0123, 0x0124, 
   0x0126, 0x0127, 0x0128, 0x012A, 0x012B, 0x012C, 0x012E, 0x012F, 0x0130, 0x0132, 
   0x0133, 0x0134, 0x0136, 0x0137, 0x0138, 0x013A, 0x013B, 0x013C, 0x013E, 0x013F, 
   0x0140, 0x0142, 0x0143, 0x0144, 0x0146, 0x0147, 0x0148, 0x014A, 0x014B, 0x014C, 
   0x014E, 0x014F, 0x0150, 0x0152, 0x0153 };
   
 /* *****************************************************************************************
   ** LRF rs232 protocol definitions                                                      **
   *****************************************************************************************/
#define STX 2
#define ETX 3
   

#define LRF_PORT "/dev/tts/2"


unsigned char kb_lrf_Data[MAX_DATA_BYTES];		// buffer for data from LRF (ser port reply)

unsigned short kb_lrf_DistanceData[121];

unsigned short kb_lrf_DistanceDataSensor[121];	// Current Data from LRF
unsigned long kb_lrf_DistanceDataSum[121];		// Summed Data from LRF
int kb_lrf_DistanceGoodCounter[121];		// Counter for good readings

unsigned short CertifiedCode;

#ifdef kt_debug_lowlevel

/* ***************************************************************************************** */
/* ** Debug function                                                                      ** */
/* ***************************************************************************************** */
char *Bin2Asc(char *str, int bin, int sz)
{
   int i, j;
     
   for( j = (sz - 1); j > -1; j-- )
   {
      str[j] = (bin & 1) + 0x30;
      bin = bin >> 1;
   }
   
   return str;   

}

char *Array2Raw(char *str, int sz)
{
   int i, j;
   char raw[sz+1];
   
      
   for( i = 0, j = (sz-1); i < sz; i++, j-- )
   {
      raw[i] = (str[j] == 0x30) ? 0x31 : 0x30;
   }
   
   str[0] = 0x31; /* add the "stop" bit */
   
   for( i = 0; i < sz-1; i++)
   {
      str[i+1] = raw[i];
   }
   
   return str;   
}   

char *makeStream(char *outStream, char *inMsg, int inSz)
{
   int i, j;
   char *buf;
   
   
   buf = (char *)malloc(190477);
   memset(buf, 0, 190477);
   
   *outStream = 0;
   
   for( i = 0; i < inSz; i++ )
   {
      Bin2Asc(buf, inMsg[i], 8);
      Array2Raw(buf, 8);
      
      strcat(outStream, buf);
      memset(buf, 0, 190477);
   }
     
   free(buf);
   
   return outStream;
}

#endif

/*****************************************************************************/
/*!
 * kb_lrf_OpenComPort opens the serial port which is LRF card port ( /dev/tts/2 ). 
 *
 * \param none
 * \return 	- < 0 : Any error code 
 * 		- > 0 : Handle to the openned port
 */
int kb_lrf_OpenComPort(void)
{
  int     LRF_DeviceHandle;
  struct termios tty;

  if ((LRF_DeviceHandle = open(LRF_PORT, O_RDWR)) < 0)
  {
  
    KB_ERROR("kb_lrf_OpenComPort",KB_ERROR_OPENLRF, LRF_PORT);
    return(-1);
  }

  /* set serial port parameters */
  tcgetattr(LRF_DeviceHandle, &tty);
  cfsetospeed(&tty, (speed_t) B57600);
  cfsetispeed(&tty, (speed_t) B57600);
  tty.c_cflag = (tty.c_cflag & ~(CSIZE)) | CS7 | CSTOPB | CLOCAL | CREAD ;
  tty.c_iflag = IGNBRK;
  tty.c_lflag = 0; 
  tty.c_oflag = 0;
  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 5;
  tcsetattr(LRF_DeviceHandle, TCSANOW, &tty);

  return(LRF_DeviceHandle);
}


/*****************************************************************************/
/*!
 * kb_lrf_FlushSerPort "erases" the serial port buffer. 
 *
 * \param LRF_DeviceHandle	Handle to the port which is returned by kb_lrf_OpenComPort
 * \return none	
 */
void kb_lrf_FlushSerPort(int LRF_DeviceHandle)
{
  fd_set		fd_set1;	
  struct timeval	tv;
  char InputDummy;

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fd_set1);
  FD_SET(LRF_DeviceHandle, &fd_set1);
  
  while (select(FD_SETSIZE, &fd_set1, NULL, NULL, &tv) > 0)
  {
    (void) read(LRF_DeviceHandle, &InputDummy, 1);
    FD_ZERO(&fd_set1);
    FD_SET(LRF_DeviceHandle, &fd_set1);
  }
}

/*****************************************************************************/
/*!
 * kb_lrf_lsb_to_msb reverts Lsb to Msb on a byte 
 *
 * \param lsb	byte to reverse bits on
 * \return inverted byte	
 */
#define HKO_POLY        0x1021 /**< crc-ccitt mask */
unsigned char kb_lrf_lsb_to_msb(unsigned char lsb)
{
  unsigned char msb=0;
  unsigned char i, tmp;

  for(i=0; i<8; i++)
  {
    if (i<4)
      tmp = (lsb & (1<<i)) << (((3-i)*2)+1);
    else
      tmp = (lsb & (1<<i)) >> (((i-4)*2)+1);

    msb |= tmp;
  }
  return msb;
}

/*****************************************************************************/
/*!
 * kb_lrf_GenerateCRC does generating CCIT CRC for a given byte array and for a given array lenght 
 *
 * \param buf	Pointer to the array that need to be CRCed
 * \param count	Size of the array passed as buf
 * \return crc	CCIT crc computed from our input buffer	
 */
unsigned short kb_lrf_GenerateCRC(unsigned char *buf, int count)
{
    unsigned short ch, i, crc, j, v, xor_flag;
    unsigned short crc_msb;

    crc = 0x0000;
    for(i=0; i< count; i++)
    {
        ch = buf[i];
  
        v = 0x01;

        for (j=0; j<8; j++)
        {
            if (crc & 0x8000)
            {
                xor_flag= 1;
            }
            else
            {
                xor_flag= 0;
            }
            crc = crc << 1;

            if (ch & v)
            {
                crc= crc + 1;
            }

            if (xor_flag)
            {
                crc = crc ^ HKO_POLY;
            }

            v = v << 1;
        }
    }

    for (i=0; i<16; i++)
    {
        if (crc & 0x8000)
        {
            xor_flag= 1;
        }
        else
        {
            xor_flag= 0;
        }
        crc = crc << 1;

        if (xor_flag)
        {
            crc = crc ^ HKO_POLY;
        }
    }
 
    crc_msb = kb_lrf_lsb_to_msb((unsigned char) (crc >> 8));
    crc_msb = crc_msb << 8;
    crc_msb = crc_msb | kb_lrf_lsb_to_msb((unsigned char) (crc & 0x00FF));

    return crc_msb;
}

/*****************************************************************************/
/*!
 * kb_lrf_GetDataFromLRF does receiving some datas from the LRF sensor
 * data is stored in the global 121 bytes arrays
 *
 * datasheet is saying there is 121 measures which all are 16 bits wide
 * each measure is the distance in millimeters from the sensor and measure 0
 * means it is at -18 degrees ( inverse clockwise ) and mesure 121 means
 * it is at +198 degrees ( inverse clockwise )
 *
 * \param LRF_DeviceHandle	Handle to the lrf port which as been openned
 				by kb_lrf_openComPort or kb_lrf_init
 * \param TotalBytes	Number of bytes that should be received 
 * \return 	- < 0 : Any error code 
 * 		- > 0 : Sucess
 */
int kb_lrf_GetDataFromLRF(int LRF_DeviceHandle, int TotalBytes)
{

  fd_set		fd_set0;	
  struct timeval	tv;

  int i, j;
  int d0, d1, d2, d3;

  int BytesRead;
  int TotalBytesRaw;
  int TotalBytesRead;
  int TotalBytesMissing;

  char DummyInputChar;
  char DataRaw[2*MAX_DATA_BYTES];

  unsigned short CalculatedCRC;
  unsigned short ReceivedCRC;

  /* Calculate the raw data size from a given real data size 
   * Replacement for the ugly 
   * 	 TotalBytesRaw =((int) (ceil(4.0 * ((float) TotalBytes) / 3.0)))
   *
   *	 Note : ceil() were causing issues in KoreBot environment so
   *	        because data won't be bigger than 246 here is a table
   *		based replacement... 
   */

  TotalBytesRaw = calcTable[TotalBytes];

  /* Add room for STX + ETX bytes */
  TotalBytesRaw += 2;
  

  /* Start receive loop ( loop rquired because maximum rs232 stream size per read
   * is 255 which is beyond the maximum data size we will receive... 
   */
  TotalBytesRead = 0;
  while(TotalBytesRead < TotalBytesRaw)
  {
    tv.tv_sec = 0;
    tv.tv_usec = MAX_TIMEOUT_IN_MS*1000;
    FD_ZERO(&fd_set0);
    FD_SET(LRF_DeviceHandle, &fd_set0);

    if(select(FD_SETSIZE, &fd_set0, NULL, NULL, &tv) <= 0)
    {
      /* Kt_debug stuff here time out with no data */
      return(-3);
    }

    TotalBytesMissing = TotalBytesRaw - TotalBytesRead;
    BytesRead = read(LRF_DeviceHandle, DataRaw+TotalBytesRead, TotalBytesMissing);
    TotalBytesRead += BytesRead;
  }

  /* check that transmitted data have a correct lenght */
  if(TotalBytesRead != TotalBytesRaw)
  {
	  /* Kt_debug stuff here wrong size */
      return(-4);
  }

  /* check STX is at its correct place */
  if(DataRaw[0] != STX)
  {
      KB_ERROR("GetDataFromLRF",KB_ERROR_LRFNOSTX);
      return(-5);
  }

  /* check ETX is at its correct place */
  if(DataRaw[TotalBytesRead-1] != ETX)
  {
      KB_ERROR("kb_lrf_GetDataFromLRF",KB_ERROR_LRFNOETX); 
      return(-6);
  }

  
  /* Initialise index variables where i is real data index
   * and j is raw data index
   * 
   * j is initialised with 1 because RawData[0] is in fact STX byte
   */
  i=0;
  j=1;

  while (i<TotalBytes+1) /* +1 because first byte is STX not that there is a
			    bytes we dun even use is obviously latest byte
			    which us ETX... */
  {
    /* Works from 4 bytes of data covert back from ascii to int
	   while removing 0x20... */
    d0 = DataRaw[j  ]-0x20;			
    d1 = DataRaw[j+1]-0x20;
    d2 = DataRaw[j+2]-0x20;
    d3 = DataRaw[j+3]-0x20;

    /* retrieve the real values back from the uencoded64 values */
    kb_lrf_Data[i  ] = ((d0 & 0x3F)<<2) | ((d1 & 0x30)>>4);	
    kb_lrf_Data[i+1] = ((d1 & 0x0F)<<4) | ((d2 & 0x3C)>>2);	
    kb_lrf_Data[i+2] = ((d2 & 0x03)<<6) | ((d3 & 0x3F)   );

    /* change indexes to point next group of data 
       because rawdata (j) 4 bytes and realdata (i) is 3 bytes */
    i += 3;				
    j += 4;
  }

  /* Compute the CRC from the message we received from the sensor
     be attentive that 2 last bytes of the message is in fact CRC
     of the real message which has been appended to message */
  CalculatedCRC = kb_lrf_GenerateCRC(kb_lrf_Data, TotalBytes-2);

  /* "extract" the crc value from the message */
  ReceivedCRC = ((unsigned short) (kb_lrf_Data[TotalBytes-2]<<8) | (kb_lrf_Data[TotalBytes-1]));

  /* ensure both crcs ( calculated and received ) are equal */
  if (CalculatedCRC != ReceivedCRC)
  {
    KB_DEBUG( "kb_lrf_GetDataFromLRF", 0, "CRC_CALCULATED = 0x%4.4X, CRC_RECEIVED = 0x%4.4X", 
              CalculatedCRC, ReceivedCRC );
    return(-7);
  }


  return(1);
}



/*****************************************************************************/
/*!
 * kb_lrf_SendCommand does sending a command to the LRF, checkout the link before as well
 *
 * \param LRF_DeviceHandle	Handle to the lrf port which as been openned
 				by kb_lrf_openComPort or kb_lrf_init
 * \param LRF_Command	Buffer containing the command to send 
 * \param LRF_CommandLength	Lenght of the command to send
 * \return 	- < 0 : Any error code 
 * 		- > 0 : Sucess
 */
int kb_lrf_SendCommand(int LRF_DeviceHandle, char *LRF_Command, int LRF_CommandLength)
{
  fd_set		fd_set1;		/* set of file descriptors to wait for */
  struct timeval	tv;		/* specifies the time to wait */

  int i, j;
  int d0, d1, d2;
  int BytesToSend, RawBytesToSend, BytesWritten, ceilInt;

  char Data[MAX_DATA_BYTES];
  char DataRaw[2*MAX_DATA_BYTES];
  
  unsigned short CalculatedCRC;
  float nbrBytes, ceilVal;

  BytesToSend = LRF_CommandLength;

  if (BytesToSend+2 > MAX_DATA_BYTES)
  {
     KB_ERROR("kb_lrf_SendCommand",KB_ERROR_LRFDATA2BIG, BytesToSend+2);
     return(-1);
  }


  /*  Empty Serial Port Input */
  kb_lrf_FlushSerPort(LRF_DeviceHandle);

  /*  Calculate CRC and append to data */
  for (i=0; i<BytesToSend; i++)
  {
    Data[i] = LRF_Command[i];
  }
  CalculatedCRC = kb_lrf_GenerateCRC(LRF_Command, BytesToSend);
  Data[i]   = (CalculatedCRC & 0xFF00)>>8;
  Data[i+1] = (CalculatedCRC & 0x00FF);
  BytesToSend += 2;

  /* Convert Data to RAW sending data */
  i=0;									/* pointer to real data */
  j=0;									/* pointer to raw data */

  while (i<BytesToSend)
  {
    d0 = Data[i  ];							/* take 3 bytes data */
    d1 = Data[i+1];
    d2 = Data[i+2];

    DataRaw[1+j  ] = 0x20 +                       ((d0 & 0xFC) >> 2) ;
    DataRaw[1+j+1] = 0x20 + (((d0 & 0x03) << 4) | ((d1 & 0xF0) >> 4));
    DataRaw[1+j+2] = 0x20 + (((d1 & 0x0F) << 2) | ((d2 & 0xC0) >> 6));
    DataRaw[1+j+3] = 0x20 +  ((d2 & 0x3F)     )                      ;

    i += 3;								/* increase pointer in raw data */
    j += 4;
  }
   
  nbrBytes = (float) BytesToSend;
  nbrBytes /= 3.0;
  nbrBytes *= 4.0;
  
  KB_DEBUG( "kb_lrf_SendCommand", 0, "nbrBytes=%f", nbrBytes );
  
  /* Calculate the raw data size from a given real data size Replacement for the ugly 
      RawBytesToSend =((int) (ceil(4.0 * ((float) BytesToSend) / 3.0)))

      Note : ceil() were causing issues in KoreBot environment so
             because data won't be bigger than 246 here is a table
             based replacement... */

  RawBytesToSend = calcTable[BytesToSend];
  
  //RawBytesToSend = ((int) (ceil(4.0*(((float) BytesToSend)/3.0))));
  
  DataRaw[0]=0x02;			// STX
  DataRaw[RawBytesToSend+1]=0x03;	// ETX
  RawBytesToSend += 2;

  /*  Wait for free serial port and send data off */
  tv.tv_sec = 0;
  tv.tv_usec = 1000*MAX_TIMEOUT_IN_MS;
  FD_ZERO(&fd_set1);
  FD_SET(LRF_DeviceHandle, &fd_set1);
  if (select(FD_SETSIZE, NULL, &fd_set1, NULL, &tv) <= 0) 
  {
     KB_ERROR("kb_lrf_SendCommand",KB_ERROR_LRFNODATA); 
     return(-2);
  }

  BytesWritten = write(LRF_DeviceHandle, DataRaw, RawBytesToSend);

  if (BytesWritten != RawBytesToSend)
  {
     KB_ERROR("kb_lrf_SendCommand", KB_ERROR_LRFSENDDATA);  
     return(-3);
  }

  KB_DEBUG( "kb_lrf_SendCommand", 0, "bytes sent=%d", RawBytesToSend );
  
  return(0);
}


/*****************************************************************************/
/*!
 * kb_lrf_AcquireCertifiedCode does receive certification code from LRF
 *
 * \param LRF_DeviceHandle	Handle to the lrf port which as been openned
 *				by kb_lrf_openComPort or kb_lrf_init
 * \return 	- < 0 : Any error code 
 * 		- > 0 : Sucess
 */
int kb_lrf_AcquireCertifiedCode(int LRF_DeviceHandle)
{
  char LRF_CMD[6];		/* at most 5 chars + \0 */

  /*  Link Certified Code Acquisition */
  sprintf (LRF_CMD, "%c%c", 0xA0, 0x69);
  KB_DEBUG( "kb_lrf_AcquireCertifiedCode", 0, "command=%c%c", 0xA0, 0x69 );

  kb_lrf_SendCommand(LRF_DeviceHandle, LRF_CMD, 2);

  if (kb_lrf_GetDataFromLRF(LRF_DeviceHandle, 12) < 0)
  {
     KB_ERROR("kb_lrf_AcquireCertifiedCode", KB_ERROR_LRFNOCERT);
     return(-1);
  }
  if ((kb_lrf_Data[0] != 0xA0) || (kb_lrf_Data[1] != 0x69))
  {
     KB_ERROR("kb_lrf_AcquireCertifiedCode", KB_ERROR_LRFNODATA);
     return(-2);
  }

  CertifiedCode = kb_lrf_GenerateCRC(kb_lrf_Data+2, 8);

  return(0);
}

/*****************************************************************************/
/*!
 * kb_lrf_CertifyLink does checking that link is still valid, caution,
 * this function need to be called regulary, below 3 seconds
 *
 * \param LRF_DeviceHandle	Handle to the lrf port which as been openned
 *				by kb_lrf_openComPort or kb_lrf_init
 * \return 	- < 0 : Any error code 
 * 		- > 0 : Sucess
 */
int kb_lrf_CertifyLink(int LRF_DeviceHandle)
{
  char LRF_CMD[6];		// at most 5 chars + \0

  sprintf (LRF_CMD, "%c%c%c%c%c", 0xA0, 0x5A, 0x01,
			((CertifiedCode & 0xFF00)>>8), (CertifiedCode & 0x00FF));
  KB_DEBUG( "kb_lrf_CertifyLink", 0, "command=%c%c%c%c%c", 0xA0, 0x5A, 0x01, 
  			((CertifiedCode & 0xFF00)>>8), (CertifiedCode & 0x00FF));

  kb_lrf_SendCommand(LRF_DeviceHandle, LRF_CMD, 5);

  if (kb_lrf_GetDataFromLRF(LRF_DeviceHandle, 5) < 0)
  {
     KB_ERROR("kb_lrf_CertifyLink", KB_ERROR_LRFNOCERT); 
     return(-1);
  }
  if ((kb_lrf_Data[0] != 0xA0) || (kb_lrf_Data[1] != 0x5a) || (kb_lrf_Data[2] != 0x01))
  {
     KB_ERROR("kb_lrf_SendCommand",KB_ERROR_LRFNODATA); 
     return(-2);
  }

  return(0);
}


/*****************************************************************************/
/*!
 * kb_lrf_pwrOn does enabling 12 => 24V converter thus powering LRF module
 */
void kb_lrf_pwrOn(void)
{
    kb_gpio_set(PXA_INT0);
}
 
/*****************************************************************************/
/*!
 * kb_lrf_pwrOff does enabling 12 => 24V converter thus powering LRF module
 */
void kb_lrf_pwrOff(void)
{
    kb_gpio_clear(PXA_INT0);
}  

/*****************************************************************************/
/*!
 * kb_lrf_Init does openning port, turn power on the LRF then certify the link,
 *
 * \return 	- < 0 : Any error code 
 * 		- > 0 : Handle to the device
 */
int kb_lrf_Init(char *LRF_DeviceName)
{
  int LRF_DeviceHandle;
  int rc;

  /* Open port to LRF device    */
  LRF_DeviceHandle = kb_lrf_OpenComPort();
  if (LRF_DeviceHandle < 0)
  {
     KB_ERROR("kb_lrf_OpenComPort",KB_ERROR_OPENLRF, LRF_DeviceName);
     return(-1);
  }
  
  /* Initialise the GPIO library */
  if((rc = kb_init( 0 , NULL )) < 0 )
    return 1;
    
  /* Init gpio module */
  kb_gpio_init();
    
  /* Set GPIO8 pin as gpio, ndlr: because this pin doesnt have
     an alternate function, it is not required... */
  kb_gpio_function(PXA_INT0,0);  
  
  /* Set GPIO8 (PXA_INT0) as output */
  kb_gpio_dir(PXA_INT0, OUT);

  /* turn "on" the led range finder module */
  kb_lrf_pwrOn();
  
  /* Wait that 1.5 seconds before to query the lrf - 1s specified by datasheet */ 
  usleep(1500*1000);

  /*  Initialize LRF */
  kb_lrf_AcquireCertifiedCode(LRF_DeviceHandle);
  kb_lrf_CertifyLink(LRF_DeviceHandle);
  
  /* Some more timer that is required */
  sleep(0.5);

  return(LRF_DeviceHandle);
}


/*****************************************************************************/
/*!
 * kb_lrf_Close does powering off the LRF, the close the port device
 *
 * \param LRF_DeviceHandle	Handle to the lrf port which as been openned
 *				by kb_lrf_openComPort or kb_lrf_init
 */
void kb_lrf_Close(int LRF_DeviceHandle)
{

  kb_lrf_pwrOff();
  close(LRF_DeviceHandle);
}


/*****************************************************************************/
/*!
 * kb_lrf_FetchDistanceFromLRF is a medium level function that does whole mechanism
 * send the command, then receive back the data from LRF,
 *
 * \param LRF_DeviceHandle	Handle to the lrf port which as been openned
 *				by kb_lrf_openComPort or kb_lrf_init
 * \return 	- < 0 : Any error code 
 * 		- > 0 : Sucess
 */
int kb_lrf_FetchDistanceFromLRF(int LRF_DeviceHandle)
{
  char LRF_CMD[2];
  int i;

  /*  Request Distance reading */
  LRF_CMD[0]=0xA2;
  LRF_CMD[1]=0x69;
  if (kb_lrf_SendCommand(LRF_DeviceHandle, LRF_CMD, 2) < 0)
  {
     KB_ERROR("kb_lrf_FetchDistanceFromLRF",KB_ERROR_LRFCMDTX); 
     return(-1);
  }

  /*  Retrieve Distance data */
  if (kb_lrf_GetDataFromLRF(LRF_DeviceHandle, 246) < 0)	 /* 253?? */
  {
     KB_ERROR("kb_lrf_FetchDistanceFromLRF",KB_ERROR_LRFDISTRX); 
     return(-2);
  }

  /* Simple Check for data integrity */
  if ((kb_lrf_Data[0] != 0xA2) || (kb_lrf_Data[1] != 0x69))
  {
     KB_ERROR("kb_lrf_FetchDistanceFromLRF",KB_ERROR_LRFDATARX); 
     return(-3);
  }

  /* Transform bytes into distance data */
  for (i=0; i<121; i++)
  {
    kb_lrf_DistanceDataSensor[i] = (kb_lrf_Data[(2*i)+3] << 8) +
    			         kb_lrf_Data[(2*i)+2];
  }

  return(0);
}


/*****************************************************************************/
/*!
 * kb_lrf_GetDistances is a high level function that does whole mechanism
 * certify the link, get some distances, averages then receive back the data from LRF,
 *
 * \param LRF_DeviceHandle	Handle to the lrf port which as been openned
 *				by kb_lrf_openComPort or kb_lrf_init
 * \return 	- < 0 : Any error code 
 * 		-  0 : Sucess
 */
int kb_lrf_GetDistances(int LRF_DeviceHandle)
{
  int i, j;

  /* Certify Link */
  if (kb_lrf_CertifyLink(LRF_DeviceHandle) < 0)
  {
    kb_lrf_AcquireCertifiedCode(LRF_DeviceHandle);		// get new certificate

    if (kb_lrf_CertifyLink(LRF_DeviceHandle) < 0)			// still problems?
    {
       KB_ERROR("kb_lrf_GetDistances", KB_ERROR_LRFNOCERT);
       return (-1);
    }
  }

  /* Clear averaging memory */
  for (j=0; j<121; j++)
  {
    kb_lrf_DistanceDataSum[j]     = 0;
    kb_lrf_DistanceGoodCounter[j] = 0;
  }

  /* Get several readings to average */
  for (i=0; i<DISTANCE_READINGS_TO_AVERAGE; i++)
  {
    (void) kb_lrf_FetchDistanceFromLRF(LRF_DeviceHandle);
    for (j=0; j<121; j++)
    {
      if (kb_lrf_DistanceDataSensor[j] < 0xF000)		/* otherwise error! */
      {
        kb_lrf_DistanceDataSum[j] += kb_lrf_DistanceDataSensor[j];	/* add 'good' values */
        kb_lrf_DistanceGoodCounter[j]++;				/* increase counter for good values */
      }
    }
  }

  /* averaging */
  for (j=0; j<121; j++)
  {
    if (kb_lrf_DistanceGoodCounter[j] > 0)
    {
      kb_lrf_DistanceData[j] = kb_lrf_DistanceDataSum[j]/kb_lrf_DistanceGoodCounter[j];
    } else {
      kb_lrf_DistanceData[j] = 0;
    }
  }

  return(0);
}

