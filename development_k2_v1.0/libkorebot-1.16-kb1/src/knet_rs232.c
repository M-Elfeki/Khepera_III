/*--------------------------------------------------------------------
 * knet_rs232.c - RS-232 Communication Layer
 *--------------------------------------------------------------------
 * $Id: knet_rs232.c,v 1.2 2004/09/22 08:29:15 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Revision: 1.2 $
 * $Date: 2004/09/22 08:29:15 $
 *--------------------------------------------------------------------*/

/*! 
 * \file   knet_rs232.c RS-232 Communication Adaptor for K-Net             
 *
 * \brief 
 *         This module is the low-level communication layer between   
 *         the KoreBot Library and the Linux Operating System
 *        
 * \author   Cédric Gaudin (K-Team SA)                               
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#include "korebot.h"
#include "knet_rs232.h"

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termio.h>

static struct knet_dev_s * knet_rs232_open( struct knet_bus_s * bus , 
					    knet_ext_t ext ,
					    knet_mod_t mod , 
					    const char * dev_name ,
					    int argc , char * argv[] );

static void knet_rs232_close( knet_bus_t * bus ,
			      knet_dev_t * dev );

static int knet_rs232_read( knet_bus_t * bus ,
			    knet_dev_t * dev ,
			    char * buf , 
			    unsigned int len );

static int knet_rs232_write( knet_bus_t * bus ,
			     knet_dev_t * dev ,
			     const char * buf , 
			     unsigned int len );

/*--------------------------------------------------------------------*/
/*!
 *  This function initializes the RS-232 layer. 
 * 
 * \param bus  K-Net Bus Descriptor
 * \param argc number of elements in argv
 * \param argv string used as initialization parameters
 * \return An error code:
 *        - <0 on error - error code
 *        - >=0 on success - number of devices found on the bus
 */
int knet_rs232_init( knet_bus_t * bus , int argc , char * argv [] )
{
  bus->exit     = NULL;
  bus->open     = knet_rs232_open;
  bus->close    = knet_rs232_close;
  bus->transfer = NULL;
  bus->read     = knet_rs232_read;
  bus->write    = knet_rs232_write;
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function opens a given device on the RS-232 Bus
 *
 * \param bus    K-Net RS-232 bus descriptor
 * \param ext    K-Net Extension Id (not used)
 * \param mod    K-Net Module Id (not used)
 * \param name   K-Net device name 
 *
 * \return A Pointer to a KNet Device Descriptor or 
 *         NULL in case of error.
 *
 * \remark This function is NOT exported outside this module.
 */
static struct knet_dev_s * knet_rs232_open( struct knet_bus_s * bus , 
					    knet_ext_t  ext ,
					    knet_mod_t  mod , 
					    const char * name ,
					    int argc , char * argv[] )
{
  knet_dev_t * dev;
  knet_rs232_t * rs232;
  int fd;
 
  dev = knet_bus_find_device_by_name( bus , name );
  
  if ( dev == NULL ) {

    /* 
     * create device on demand (it's faster than 
     * scanning the bus at the startup of the application 
     */
    if ( !access( name , (F_OK|R_OK|W_OK)) ) {
      dev = knet_device_create( bus , 0 , 0 , name );
    }

      
  }

  if ( dev == NULL ) {
    kb_error( __FILE__ ,
	      __LINE__ ,
	      "knet_rs232_open" ,
	      KB_ERROR_NODEVFOUNDSTR , name );
    return NULL;
  }

  if ((fd=open(name,O_RDWR))<0) {
    kb_error( __FILE__ ,
	      __LINE__ ,
	      "knet_rs232_open" ,
	      KB_ERROR_FILEOPEN ,
	      name );
    return NULL;
  }

  rs232 = KB_ALLOC(knet_rs232_t,1);

  rs232->fd = fd;

  tcgetattr(fd , &rs232->tios);
  rs232->tios.c_cflag = ( CS8 | CLOCAL | CREAD | B115200 );
  rs232->tios.c_iflag = IGNPAR;
  rs232->tios.c_oflag = 0;
  rs232->tios.c_lflag = 0;
  rs232->tios.c_cc[VMIN] = 0;
  rs232->tios.c_cc[VTIME] = 0;
  tcsetattr( fd , TCSANOW , &rs232->tios );
  
  dev->info = (knet_dev_info_t*)rs232;
  
  return (dev);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function closes an RS-232 device.
 *
 * \param bus       K-Net RS-232 bus descriptor
 * \param dev       K-Net RS-232 device descriptor
 *
 * \remark This function is NOT exported outside this module.
 */
static void knet_rs232_close( knet_bus_t * bus ,
			      knet_dev_t * dev )
{
  knet_rs232_t * rs232 = dev->info;

  if ( rs232 != NULL ) {

    if ( rs232->fd != -1 ) 
      close(rs232->fd );
   
    kb_free( rs232 );
    dev->info = NULL;
  }
}

/*--------------------------------------------------------------------*/
/*! 
 * This function reads data from an RS-232 device.
 *
 * \param bus K-Net RS-232 bus descriptor
 * \param dev K-Net RS-232 device descriptor
 * \param buf Pointer to the buffer that will receive the data
 * \param len Size of the buffer
 *
 * \return A value:
 *        - <0 on error
 *        - >=0 on success, number of bytes read
 * 
 * \remark This function is NOT exported outside this module.
 */
static int knet_rs232_read( knet_bus_t * bus ,
			    knet_dev_t * dev ,
			    char * buf , unsigned int len )
{
  knet_rs232_t * rs232 = dev->info;

  if ( rs232 != NULL && rs232->fd != -1 )
    return read( rs232->fd , buf , len );

  return -1;
}


/*--------------------------------------------------------------------*/
/*! 
 * This function writes data to an RS-232 device. 
 *
 * \param bus K-Net RS-232 bus descriptor
 * \param dev K-Net RS-232 device descriptor
 * \param buf Pointer to the buffer that contains the data to be 
 *            written
 * \param len Number of the bytes in the buffer
 *
 * \return A value:
 *        - <0 on error
 *        - >=0 on success, number of byte written
 *
 * \remark This function is NOT exported outside this module.
 */
static int knet_rs232_write( knet_bus_t * bus ,
			   knet_dev_t * dev ,
			   const char * buf , unsigned int len )
{
  knet_rs232_t * rs232 = dev->info;
  
  if ( rs232 != NULL && rs232->fd != -1 )
    return write( rs232->fd , buf , len );

  return -1;
}
