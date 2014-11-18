/*--------------------------------------------------------------------
 * knet_i2c.c - I²C Communication Layer
 *--------------------------------------------------------------------
 * $Id: knet_i2c.c,v 1.2 2007/04/05 08:12:27 flambercy Exp $
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Revision: 1.2 $
 * $Date: 2007/04/05 08:12:27 $
 *--------------------------------------------------------------------*/

/*! 
 * \file   knet_i2c.c I²C Communication Adaptor for K-Net             
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
#include "knet_i2c.h"
#include "i2ccom.h"

static int knet_i2c_exit( knet_bus_t * bus );

static struct knet_dev_s * knet_i2c_open( struct knet_bus_s * bus , 
					  knet_ext_t ext ,
					  knet_mod_t mod , 
					  const char * name ,
					  int argc , char * argv[] );

static int knet_i2c_transfer( knet_bus_t * bus ,
			      knet_dev_t * dev , 
			      const char * write_buf , 
			      unsigned int write_len ,
			      char * read_buf , 
			      unsigned int read_len );

static int knet_i2c_read( knet_bus_t * bus ,
			  knet_dev_t * dev ,
			  char * buf , unsigned int len );

static int knet_i2c_write( knet_bus_t * bus ,
			   knet_dev_t * dev ,
			   const char * buf , unsigned int len );

/*--------------------------------------------------------------------*/
/*!
 * This function is used internally.  
 */
static int knet_i2c_scan_callback( i2c_t * i2c , 
				   i2c_dev_t dev , 
				   void * context )
{
  knet_bus_t * bus = (knet_bus_t * ) context;

  pthread_mutex_lock( &bus->lock );
  knet_device_create( bus , 0 , dev , NULL ); 
  pthread_mutex_unlock( &bus->lock );
  return 0;
}


/*--------------------------------------------------------------------*/
/*!
 *  This function initializes the I²C bus. 
 * 
 * \param bus  KNet Bus Handle
 * \param argc number of elements in argv
 * \param argv string used as initialization parameters
 * \return An error code:
 *        - <0 on error - error code
 *        - >=0 on success - number of devices found on the bus
 */
int knet_i2c_init( knet_bus_t * bus , int argc , char * argv [] )
{
  int i, rc;
  i2c_t * i2c;
  char * device = NULL;
  int scan_bus = 0;

  bus->exit     = knet_i2c_exit;
  bus->open     = knet_i2c_open;
  bus->close    = NULL;
  bus->transfer = knet_i2c_transfer;
  bus->read     = knet_i2c_read;
  bus->write    = knet_i2c_write;

  i2c = KB_ALLOC( i2c_t , 1 );
  
  /* try to find a parameter for the I²C layer */
  for (i=1; i<argc; i++) {

    /* --i2c-device define the device to use for the I²C bus */
    if (!strcmp(argv[i],"--i2c-device")) {
      device = argv[i];
      i++;
    }
    else if (!strcmp(argv[i],"--i2c-scan")) {
      scan_bus = 1;
    }
  }

  if ((rc=i2c_open( i2c , device )) < 0) {
    kb_free( i2c );
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "knet_i2c_open" ,
		     KB_ERROR_OPENBUS ,
		     "I²C" );

  }
  bus->bus_info = i2c;

  /* bus scanning */
  if ( scan_bus > 0 ) {

    rc = i2c_scan( i2c , 
		   knet_i2c_scan_callback , 
		   bus );
    if ( rc < 0 ) {
      i2c_close( i2c );
      kb_free( i2c );
      return kb_error( __FILE__ ,
		       __LINE__ ,
		       "knet_i2c_open" ,
		       KB_ERROR_SCANBUS ,
		       "I²C" );
    } 
  }
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function releases all resources used by an I²C Bus
 *
 * \param bus KNet I²C bus handle
 * \return always 0
 *
 * \remark This function is NOT exported outside this module.
 */
static int knet_i2c_exit( knet_bus_t * bus )
{
  i2c_close( (i2c_t *) bus->bus_info );
  kb_free( bus->bus_info );
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function opens a given device on the I²C Bus
 *
 * \param bus    KNet I²C bus handle
 * \param ext    KNet Extension Id (don't care for I²C Bus)
 * \param mod    KNet Module Id (I²C Device Address).
 *
 * \return A Pointer to a KNet Device Descriptor or 
 *         NULL in case of error.
 *
 * \remark This function is NOT exported outside this module.
 */
static struct knet_dev_s * knet_i2c_open( struct knet_bus_s * bus , 
					  knet_ext_t  ext ,
					  knet_mod_t  mod , 
					  const char * name ,
					  int argc , char * argv[] )
{
  knet_dev_t * dev;
  int ok;
  i2c_dev_t adr;

  adr = mod;

  dev = knet_bus_find_device( bus , 0 , adr );
  if ( dev == NULL ) {

    /* 
     * create device on demand (it's faster than 
     * scanning the bus at the startup of the application 
     */
    if ( i2c_exists( (i2c_t *)bus->bus_info , adr ) ) {
      dev = knet_device_create( bus , 0 , adr , NULL ); 
    }
      
  }

  if ( dev == NULL ) {
    kb_error( __FILE__ ,
	      __LINE__ ,
	      "knet_i2c_open" ,
	      KB_ERROR_NODEVFOUND , 0 , adr );
    return NULL;
  }


   return dev;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function transfers data to and from an I²C bus.
 *
 * \param bus       KNet I²C bus handle
 * \param dev       KNet I²C device handle
 * \param write_buf Pointer to the buffer that contains data to be 
 *                  written 
 * \param write_len Number of bytes in the write buffer
 * \param read_buf  Pointer to the buffer that will receive data
 * \param read_len  Size of the read buffer in bytes
 *
 * \return A value:
 *         - <0 on error
 *         - >=0 on success
 *
 * \remark This function is NOT exported outside this module.
 */
static int knet_i2c_transfer( knet_bus_t * bus ,
			      knet_dev_t * dev , 
			      const char * write_buf , unsigned int write_len ,
			      char * read_buf , unsigned int read_len )
{
  return i2c_lltransfer( (i2c_t *)bus->bus_info ,
			 dev->mod_addr ,
			 write_buf , write_len ,
			 read_buf , read_len );
		  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function reads data from an I²C bus.
 *
 * \param bus KNet I²C bus handle
 * \param dev KNet I²C device handle
 * \param buf Pointer to the buffer that will receive the data
 * \param len Size of the buffer
 *
 * \return A value:
 *        - <0 on error
 *        - >=0 on success, number of bytes read
 * 
 * \remark This function is NOT exported outside this module.
 */
static int knet_i2c_read( knet_bus_t * bus ,
			  knet_dev_t * dev ,
			  char * buf , unsigned int len )
{
  return i2c_llread( (i2c_t *)bus->bus_info , 
		     dev->mod_addr ,
		     buf , len );
}


/*--------------------------------------------------------------------*/
/*! 
 * This function writes data to an I²C bus. 
 *
 * \param bus KNet I²C bus handle
 * \param dev KNet I²C device handle
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
static int knet_i2c_write( knet_bus_t * bus ,
			   knet_dev_t * dev ,
			   const char * buf , unsigned int len )
{
  return i2c_llwrite( (i2c_t *)bus->bus_info ,
		      dev->mod_addr ,
		      buf , len );
}
