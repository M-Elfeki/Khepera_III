/***********************************************************************
 * K-Team hex parser functions
 * P.Bureau - 09.2005
 */
#include "kt_hexparser.h"
#include <string.h>

size_t strlcpy(char *dst, const char *src, size_t siz)
{
  size_t n;
  size_t slen = strlen(src);
  if (siz) {
    if ((n = MIN(slen, siz - 1)))
      memcpy(dst, src, n);
    dst[n] = '\0';
  }
  return(slen);
}

size_t strlcat(char *dst, const char *src, size_t siz)
{
  size_t dlen = strlen(dst);
  /* Make sure siz is sane */
  if (dlen < siz - 1)
    return(dlen + strlcpy(dst + dlen,
	  src, siz - dlen));
  else
    return(dlen + strlen(src));
}


int axtoi(char *hexStg) {
  int n = 0;         // position in string
  int m = 0;         // position in digit[] to shift
  int count;         // loop index
  int intValue = 0;  // integer value of hex string
  int digit[5];      // hold values to convert
  while (n < 4) {
    if (hexStg[n]=='\0')
      break;
    if (hexStg[n] > 0x29 && hexStg[n] < 0x40 ) //if 0 to 9
      digit[n] = hexStg[n] & 0x0f;            //convert to int
    else if (hexStg[n] >='a' && hexStg[n] <= 'f') //if a to f
      digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
    else if (hexStg[n] >='A' && hexStg[n] <= 'F') //if A to F
      digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
    else break;
    n++;
  }
  count = n;
  m = n - 1;
  n = 0;
  while(n < count) {
    // digit[n] is value of hex digit at position n
    //      // (m << 2) is the number of positions to shift
    //           // OR the bits into return value
    intValue = intValue | (digit[n] << (m << 2));
    m--;   // adjust the position to set
    n++;   // next digit to process
  }
  return (intValue);
}

int hexp_start(FILE * progfile)
{
  /* Rewind stream */
  rewind(progfile);
	
  return 0;
}

int hexp_next_line(FILE * progfile, int* code, int *datacount, int* address, char * data)
{
  char linebuffer[hexp_linelength];
  char subbuffer[hexp_linelength];
  char dataline[hexp_linelength];
  int  i;

  /* Read next line */
  if(!fgets(linebuffer, hexp_linelength, progfile))
    return -1;

  if(linebuffer[0] == ':')
  {
    strlcpy(subbuffer,linebuffer+7,3);
    *code = axtoi(subbuffer);
    strlcpy(subbuffer,linebuffer+1,3);
    *datacount = axtoi(subbuffer);
    strlcpy(subbuffer,linebuffer+3,5);
    *address = axtoi(subbuffer) & 65535;

    strlcpy(dataline, linebuffer+9, ((*datacount)*2)+1);

    /* Fill in data table */
    for(i=0; i<(*datacount); i++)
    {
      strlcpy(subbuffer,dataline+(i*2),3);
      data[i] = axtoi(subbuffer);
    }

    return 1;
  }
  else
    return 2;
}

int hexp_parse_hex(FILE* progfile)
{
  char linebuffer[hexp_linelength];
  char subbuffer[hexp_linelength];
  char databyte;
  int addrcode;
  int datacount;
  int address;
  long checksum;
  int i;
  long lineaddrhigh;
  long outaddr;

  char indata[256];
  char dataline[hexp_linelength];

  char endparse = 1;

  /* Rewind stream */
  rewind(progfile);

  while(endparse && fgets(linebuffer, hexp_linelength, progfile))
  {
    for(i=0; i<256; i++)
      indata[i] = 255;

    if(linebuffer[0] == ':')
    {
	strlcpy(subbuffer,linebuffer+7,3);
	addrcode = axtoi(subbuffer);
	strlcpy(subbuffer,linebuffer+1,3);
	datacount = axtoi(subbuffer);
	strlcpy(subbuffer,linebuffer+3,5);
	address = axtoi(subbuffer) & 65535;

	strlcpy(dataline, linebuffer+9, (datacount*2)+1);

	switch(addrcode)
	{
	  case 0:
	    for(i=0; i<datacount; i++)
	    {
	      strlcpy(subbuffer,dataline+(i*2),3);
	      indata[i] = axtoi(subbuffer);
	    }

	    if(lineaddrhigh < 32 && lineaddrhigh > -1)
	    {
	      outaddr = (lineaddrhigh * 65536) + address;
	      printf("adcode: %x, datac: %x, addr prog: %lx data: %s\r\n", addrcode, datacount, outaddr, dataline);
	      //for(i=1; i<datacount; i++)
		
	    }
	    if(lineaddrhigh == 0x1F0)
	    {
	      printf("adcode: %x, datac: %x, addr prog: %lx data: %s\r\n", addrcode, datacount, outaddr, dataline);
	    }
	    
	    break;
	    
	  case 4:
	    strlcpy(subbuffer, linebuffer+9, 5); 
	    lineaddrhigh = axtoi(subbuffer) & 65535;
	    printf("adcode: %x, datac: %x, addr high: %lx data: %s\r\n", addrcode, datacount, lineaddrhigh, dataline);
	    break;

	  case 1:
	    endparse = 0; 
	    break;
	}

    }
  }
}

int hexp_valid_hex(FILE* progfile)
{
  char linebuffer[hexp_linelength];
  char subbuffer[hexp_linelength];
  char databyte;
  int addrcode;
  int datacount;
  int address;
  long checksum;
  int i;

  /* Check if the file is empty */
  if(fgetc(progfile) == EOF)
    return -1;

  /* Rewind stream */
  rewind(progfile);

  /* Validate file */
  while(fgets(linebuffer, hexp_linelength, progfile))
  {
    databyte = linebuffer[0];
    
    switch(databyte)
    {
      case ':':
	strlcpy(subbuffer,linebuffer+7,3);
	addrcode = axtoi(subbuffer);
	strlcpy(subbuffer,linebuffer+1,3);
	datacount = axtoi(subbuffer);
	strlcpy(subbuffer,linebuffer+3,5);
	address = axtoi(subbuffer) & 65535;
	
	checksum = 0;
	for(i=0; i<datacount+5; i++)
	{
	  strlcpy(subbuffer, linebuffer+(2*i)+2-1, 3);
	  checksum += axtoi(subbuffer);
	}

	if((checksum & 255) != 0)
	{
	  rewind(progfile);
	  return -3;
	}
	break;

      case ' ':
      case '\t':
      case '\r':
      case '\n':
	break;

      default:
	rewind(progfile);
	return -4;
	break;
    }

    if( addrcode == 1)
      break;
  }

  if(!feof(progfile))
    printf("Warning: end of file not reached.\r\n");
  
  /* Rewind stream */
  rewind(progfile);

  return 0;
}
