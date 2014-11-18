/*-------------------------------------------------------------------------------
 * Project	 :	command line i2c test utility
 * File		 :	i2ctest.c
 * Author	 :	Yves Piguet, 2004
 */

#include <stdlib.h>
#include <stdio.h>
#include "i2ccom.h"

int i2c_read8( i2c_t * i2c , 
	       i2c_dev_t dev , 
	       unsigned char reg , 
	       unsigned char *val )
{
  

  return i2c_lltransfer( i2c , dev , &reg , 1 , val , 1 );
}

int i2c_read16( i2c_t * i2c ,
		i2c_dev_t dev ,
		unsigned char reg ,
		unsigned short *val )
{
  return i2c_lltransfer( i2c , dev , &reg , 1 , 
			 (unsigned char *)val , 2 );
}

int i2c_read32( i2c_t * i2c ,
		i2c_dev_t dev ,
		unsigned char reg ,
		unsigned long *val )
{
  return i2c_lltransfer( i2c , dev , &reg , 1 , 
			 (unsigned char *)val , 4 );
}


int i2c_write8( i2c_t * i2c ,
		i2c_dev_t dev ,
		unsigned char reg ,
		unsigned char val )
{
  unsigned char buf[2];

  buf[0] = reg;
  buf[1] = val;

  return i2c_llwrite( i2c , dev , buf , 2 );
}

int i2c_write16( i2c_t * i2c ,
		 i2c_dev_t dev ,
		 unsigned char reg ,
		 unsigned short val )
{
  unsigned char buf[3];

  buf[0] = reg;
  buf[1] = val;
  buf[2] = (val>>8);

  return i2c_llwrite( i2c , dev , buf , 3 );
}

int i2c_write32( i2c_t * i2c ,
		 i2c_dev_t dev ,
		 unsigned char reg ,
		 unsigned long val )
{
  unsigned char buf[5];

  buf[0] = reg;
  buf[1] = val;
  buf[2] = (val>>8);
  buf[3] = (val>>16);
  buf[4] = (val>>24);

  return i2c_llwrite( i2c , dev , buf , 5 );
}

int main(int argc, char **argv)
{
  char const *devpath = NULL;
  int addr = 0, reg = 0, value, size = 8;
  int write = 0;
  unsigned char val8;
  unsigned short val16;
  i2c_t i2c;
  int status;
	
  /* decode arguments */
  for (; argc > 1; argc--, argv++) {
    if (!strcmp(argv[1], "--version")) {
      printf("i2c %s by Yves Piguet\n", __DATE__);
      exit(0);
    }
    else if (!strcmp(argv[1], "--help")) {
      printf(
	     "Usage: i2c options...\n"
	     "--dev path     device path\n"
	     "--reg r        register\n"
	     "--size n       word size (8=default or 16)\n"
	     "--slave addr   slave device address\n"
	     "--val v        value to write\n"
	     "--version      display version and quit\n"
	     );
      exit(0);
    }
    else if (!strcmp(argv[1], "--dev")) {
      argv++;
      argc--;
      devpath = argv[1];
    }
    else if (!strcmp(argv[1], "--size"))
      {
	argv++;
	argc--;
	size = atoi(argv[1]);
	if (size != 8 && size != 16)
	  {
	    fprintf(stderr, "Bad size\n");
	    exit(1);
	  }
      }
    else if (!strcmp(argv[1], "--slave"))
      {
	argv++;
	argc--;
	    addr = atoi(argv[1]) >> 1;
      }
    else if (!strcmp(argv[1], "--reg"))
      {
	argv++;
	argc--;
	reg = atoi(argv[1]);
      }
    else if (!strcmp(argv[1], "--value"))
      {
	argv++;
	  argc--;
	  value = atoi(argv[1]);
	  write = 1;
      }
    else
      {
	fprintf(stderr, "Unrecognized option %s (type i2c --help for usage)\n",
		argv[1]);
	exit(1);
      }
  }
  

  fprintf(stderr, "Opening i2c device %s\n", devpath);
  
  if (i2c_open(&i2c, devpath) < 0) {
    perror("open");
    exit(1);
  }

  fprintf(stderr, "Access i2c address 0x%x and register 0x%x\n",addr,reg);
  
  if (write)
    if (size == 16)
      status = i2c_write16(&i2c, addr, reg, value );
    else
      status = i2c_write8(&i2c, addr, reg, value);
  else
    if (size == 16)
      {
	status = i2c_read16(&i2c, addr, reg, &val16 );
	if (status == 0)
	  printf("%d\n", val16);
      }
    else
      {
	status = i2c_read8(&i2c, addr, reg, &val8);
	if ( status >= 0)
	  printf("%d\n", val8);
      }
  
  if (status < 0)
    fprintf(stderr, "Cannot access i2c register\n");
  
  i2c_close(&i2c);
  return status < 0 ? 1 : 0;
}
