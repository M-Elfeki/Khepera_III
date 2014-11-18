/*--------------------------------------------------------------------
 * i2ccom.c - I²C Low-Level Communication Layer
 *--------------------------------------------------------------------
 * $Id: i2ccom.c,v 1.4 2006/03/07 17:39:57 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Date: 2006/03/07 17:39:57 $
 * $Revision: 1.4 $
 *--------------------------------------------------------------------
 * $Log: i2ccom.c,v $
 * Revision 1.4  2006/03/07 17:39:57  pbureau
 * Makefile update for clean x86 compiling
 * i2c compilation cleaned for 2.6 kernel
 *
 * Revision 1.3  2004/09/22 18:55:08  pbureau
 * *** empty log message ***
 *
 * Revision 1.1  2004/07/29 10:51:54  cgaudin
 * New libkorebot release 1.2
 *
 *--------------------------------------------------------------------*/

#include "i2ccom.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <errno.h>

//#include <linux/i2c.h>
//#include <linux/i2c-dev.h>
#include "i2c-dev.h"

/*! 
 * \file   i2ccom.c I²C Low-Level Communication Layer             
 *
 * \brief 
 *         This module is the low-level communication layer between   
 *         the KoreBot Library and the Linux Operating System
 *        
 * \author   Yves Piguet (Calerga Sarl)                               
 * \author   Pierre Bureau (K-Team SA)                                
 * \author   Cédric Gaudin (K-Team SA)                                
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

/*--------------------------------------------------------------------*/
/*! 
 * This function opens an I²C Bus. 
 *
 * \param i2c    Pointer to I²C Bus Descriptor. 
 *               This descriptor must be allocated by the caller.
 * \param device A string representing the device name. 
 *               A NULL should be used for default I²C Bus. 
 *               (e.g. /dev/i2c/0)
 * \return an error code
 *          - <0 standard error number. See errno.h
 *          - 0 if I²C Bus Device has been opened
 */   
int i2c_open( i2c_t *i2c , char const *device )
{
  /* no device selected */
  i2c->dev = -1;      
    
  /* set default I²C bus device */
  if (!device)
    device = "/dev/i2c-0";

  if ((i2c->fd = open( device , O_RDWR )) < 0 )
    return i2c->fd;
  
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This procedure closes an I²C Bus. 
 * 
 * \param i2c  I²C Bus Handle
 */
void i2c_close( i2c_t *i2c )
{
  if (i2c->fd>0) { 
    close(i2c->fd);
    i2c->fd = -1;
  }
}

/*---------------------------------------------------------------------*/
/*! 
 * This function selects an I²C Device on a given I²C Bus.
 *
 * \param i2c     I²C Bus Handle
 * \param address I²C Device Address. This value contains only 
 *                the I²C address bits (7-bit or 10-bit).   
 * \return an error code
 *         - <0 standard error code. See errno.h 
 *         - 0 on success
 *
 * \remarks This function is used internally and is never 
 *          exported outside this module. 
 */
static int i2c_priv_select_device( i2c_t * i2c , i2c_dev_t address )
{
  int rc;

  /* change device only if necessary */
  if (i2c->dev != address ) {

    if ((rc=ioctl( i2c->fd , I2C_SLAVE , address )) < 0)
      return rc;

    i2c->dev = address;
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function is the primitive to read data from a given device 
 * on a given I²C Bus.
 *
 * \param i2c I²C Bus Handle
 * \param dev I²C Device Address. This value contains only 
 *            the I²C address bits (7-bit or 10-bit).   
 * \param buf Pointer to the buffer that will receive the data bytes.
 * \param len Size of the buffer in bytes.
 * \return an error code:
 *         - <0 standard error number. See errno.h
 *         - >=0 number of bytes read
 */
int i2c_llread( i2c_t * i2c , 
		i2c_dev_t dev , 
		char * buf , 
		unsigned int len )
{
  int rc;

  i2c_priv_select_device( i2c , dev );

  rc = read( i2c->fd , buf , len );

  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function is the primitive to write data to a given device 
 * on a given I²C Bus.
 * 
 * \param i2c I²C Bus Handle
 * \param dev I²C Device Address. This value contains only 
 *            the I²C address bits (7-bit or 10-bit).  
 * \param buf Pointer to the buffer that will be written.
 * \param len Number of bytes to write
 * \return an error code:
 *         - <0 standard error number. See errno.h 
 *         - >=0 number of bytes written 
 */
int i2c_llwrite( i2c_t * i2c , 
		 i2c_dev_t dev , 
		 const char * buf , 
		 unsigned int len )
{
  int rc;
  int i;

  i2c_priv_select_device( i2c , dev );
  
  rc = write( i2c->fd , buf , len );
  
  return rc;
}


/*--------------------------------------------------------------------*/
/*! 
 *
 * This function is the primitive to write data to and then read 
 * data from a given device on a given I²C Bus.
 * 
 * \param i2c       I²C Bus Handle.
 * \param dev       I²C Device Address. This value contains only 
 *                  the I²C address bits (7-bit or 10-bit).   
 * \param write_buf Pointer to the buffer that contains the data 
 *                  to be written. 
 * \param write_len Number of bytes to write. The data are in 
 *                  the write buffer.  
 * \param read_buf  Pointer to the buffer that will receive the 
 *                  data. 
 * \param read_len  Size of the read buffer in bytes.
 * \return an error code:
 *         - <0  standard error code. See errno.h
 *         - >=0 on success 
 */
int i2c_lltransfer( i2c_t * i2c , i2c_dev_t dev , 
		    const char * write_buf , unsigned int write_len ,
		    char * read_buf , unsigned int read_len )
{
  struct i2c_msg             msgs[2];
  struct i2c_rdwr_ioctl_data msgset;
  int rc;  
	
  /* Write Message */
  msgs[0].addr  = dev;
  msgs[0].flags = 0;
  msgs[0].buf   = (char *)write_buf;
  msgs[0].len   = write_len;
      
  /* Read Message */
  msgs[1].addr  = dev;
  msgs[1].flags = I2C_M_RD;
  msgs[1].buf   = read_buf;
  msgs[1].len   = read_len;
  	
  msgset.msgs  = msgs;
  msgset.nmsgs = 2;
	
  rc = ioctl( i2c->fd , I2C_RDWR , &msgset ); 
 
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function verifies the existance of a given device on 
 * a given I²C Bus.
 * 
 * \param i2c  I²C Bus Handle.
 * \param dev  I²C Device Address. This value contains only 
 *             the I²C address bits (7-bit or 10-bit).   
 * 
 * \return A value indicating if a device is present or not:
 *         - 1 device present 
 *         - 0 no device present
 */
int i2c_exists( i2c_t * i2c , i2c_dev_t dev )
{
  int rc, adr, found=0;
  struct i2c_msg msg;
  struct i2c_rdwr_ioctl_data msgset;
 
  msg.addr  = dev;
  msg.flags = 0;
  msg.buf   = 0;
  msg.len   = 0;
    
  msgset.msgs  = &msg;
  msgset.nmsgs = 1;

  rc = ioctl( i2c->fd , I2C_RDWR , &msgset );
  
  /* answer from an I²C device */ 
  if ( rc == 1 ) return 1;
    
  return 0;
} 
	   
/*--------------------------------------------------------------------*/
/*! 
 * This function scans a given I²C Bus to find devices
 * 
 * \param i2c       I²C Bus Handle.
 * \param callback  A callback function called when a device is found.
 *                  If this callback function returns a value <0 the 
 *                  bus scan stops immedialety and the value is 
 *                  used as return value by i2c_scan.
 * \param context   A value or a pointer passed to the callback 
 *                  function.
 * \return an error code:
 *         - <0 standard error code (See errno.h) or a user error code.
 *         - >=0 Number of devices found on the I²C bus.
 */
int i2c_scan( i2c_t * i2c , 
	      int (*callback)( i2c_t * i2c , 
			       i2c_dev_t dev , 
			       void * context ) , 
	      void * context)
{
  int rc, adr, found=0;
  struct i2c_msg msg;
  struct i2c_rdwr_ioctl_data msgset;
 
  /* scan the whole I²C bus range */
  for (adr=2; adr<128; adr++) {
    msg.addr  = adr;
    msg.flags = 0;
    msg.buf   = 0;
    msg.len   = 0;
    
    msgset.msgs  = &msg;
    msgset.nmsgs = 1;

    rc = ioctl( i2c->fd , I2C_RDWR , &msgset );
    /* answer from a I²C device */ 
    if ( rc == 1 ) {
      found++;
      if ( callback != NULL ) {
	if ((rc=callback( i2c , adr , context ))<0)
	  return rc;
      }
    }
  }

  return found;
}

