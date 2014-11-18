/*--------------------------------------------------------------------
 * knet.c - KoreBot Library - K-Net Abstraction Layer
 *--------------------------------------------------------------------
 * $Id: knet.c,v 1.13 2007/04/05 08:12:27 flambercy Exp $
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Date: 2007/04/05 08:12:27 $
 * $Revision: 1.13 $
 *-------------------------------------------------------------------- 
 * $Log: knet.c,v $
 * Revision 1.13  2007/04/05 08:12:27  flambercy
 * modif knet_i2c.c
 *
 * Revision 1.12  2006/02/08 09:53:18  flambercy
 * *** empty log message ***
 *
 * Revision 1.11  2006/01/12 15:28:49  pbureau
 * corrected makefile in src/utils
 *
 * Revision 1.10  2005/11/16 13:15:03  amaye
 * Various fixes:
 * - Some errors definition has been added in the kb_error module.
 * - The functions knet_Getcommand + knet_Sendcommand has been moved out of the knet module.
 * - The functions knet_getcommand and knet_sendcommand are renamed to kh3_getcommand and so on.
 * - The kb_khepera3.h has been included within korebot.h
 *
 * Revision 1.9  2005/11/03 17:15:50  amaye
 * knet_SendCommand + knet_GetCommand for a frame based i2c communication
 *
 * Revision 1.8  2005/04/28 16:33:44  pbureau
 * kb_sound update
 *
 * Revision 1.7  2004/09/22 08:29:15  pbureau
 * Added Koala support, lot of additions
 *
 * Revision 1.6  2004/09/05 14:05:14  pbureau
 * updated src/tests with new test programs
 *
 * Revision 1.5  2004/09/02 14:41:57  pbureau
 * solved conflict
 *
 * Revision 1.4  2004/09/02 14:38:33  pbureau
 * Added kb_socket.c
 *
 * Revision 1.3  2004/08/18 07:39:08  cgaudin
 * Compilation problems solved.
 *
 * Revision 1.2  2004/08/16 22:04:15  cgaudin
 * Added Hemisson Robot support for KoreBot, need to be tested.
 *
 * Revision 1.1  2004/07/29 10:51:55  cgaudin
 * New libkorebot release 1.2
 *
 *--------------------------------------------------------------------*/

/*! 
 * \file   knet.c K-Net Abstraction Layer              
 *
 * \brief 
 *         This module is an abstraction layer for communication with 
 *         different busses and devices. It offers simple interface 
 *         to other module or to the user.
 *        
 * \author   Cédric Gaudin (K-Team SA)                               
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#include "korebot.h"
#include "knet_i2c.h"
#include "knet_rs232.h"

#include "bit_conv_tab.c"

/*! K-Net Internal Buffer */
static unsigned int knet_buffer_size = 0;
static char        *knet_buffer = NULL;
/*! Lock to protec the internal buffer */
pthread_mutex_t   knet_buffer_lock;

/*! Number of registered busses  */
static unsigned int knet_bus_count=0;

/*! K-Net Bus Descriptors */
static knet_bus_t   knet_busses[ KNET_MAX_BUSSES ];

/*!
 * List of existing knet bus names, as they should
 * appear in configuration files. Device class names
 * are defined in kb_config.h
 */
const char * knet_bus_names[] = { 
    "none" , "i2c" , "spi" , "rs232" , NULL 
  };

/*--------------------------------------------------------------------*/
/*! 
 * This function initializes the K-Net layer. This function should be
 * called once at startup.
 *
 * \param argc Number of arguments
 * \param argv An array of strings containing a list of arguments
 *
 * \return An error code:
 *       - <0 on error
 *       - 0  on success
 */
int knet_init( int argc , char * argv[] )
{
  int rc;
  unsigned int r;
  knet_bus_t * bus;

  if ( knet_bus_count > 0 ) {
    /* WARNING: mutiple call to knet_bus_init */
    return;
  }

  for (r=0; r<KNET_MAX_BUSSES; r++) 
    knet_busses[r].usage_counter = 0;
 
#ifdef KNET_BUS_I2C
  bus = &knet_busses[ KNET_BUS_I2C ];
  pthread_mutex_init( &bus->lock , 0 );

  if ( (rc=knet_i2c_init( bus , argc , argv )) < 0 )
    return rc;
 
  bus->usage_counter    = 1;
  knet_bus_count++;
#endif /* KNET_BUS_I2C */

#ifdef KNET_BUS_SPI
  /* not implemented yet */
#endif /* KNET_BUS_SPI */

#ifdef KNET_BUS_RS232
  bus = &knet_busses[ KNET_BUS_RS232 ];
  pthread_mutex_init( &bus->lock , 0 );
  
  if ((rc=knet_rs232_init( bus , argc , argv )) < 0 )
    return rc;
    
  bus->usage_counter = 1;
  knet_bus_count++;
#endif /* KNET_BUS_RS232 */

  /*atexit( knet_exit );*/

  knet_buffer_size = 1024;
  knet_buffer = kb_alloc( knet_buffer_size );
  pthread_mutex_init( &knet_buffer_lock, 0 );

  return knet_bus_count;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function releases all resources used by the K-Net layer
 *
 * \remark Never call this function !
 */
void knet_exit( void ) 
{
  unsigned int r;
  knet_bus_t * bus;
  knet_dev_t * dev;

  for (r=0; r<KNET_MAX_BUSSES; r++) {
    bus = &knet_busses[ r ];
    if ( bus->usage_counter ) {
      /* destroy all devices */
      while ( bus->devices != NULL ) {
	knet_device_destroy( bus->devices );
      }
      /* close the bus */
      if ( bus->exit != NULL ) bus->exit( bus );
      knet_bus_count--;
    }
  }

  kb_free ( knet_buffer );
  knet_buffer = NULL;
  knet_buffer_size = 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function finds a K-Net device descriptor given its K-Net 
 * Extension and K-Net Module IDs.
 *
 * \param bus      K-Net Bus Handle
 * \param ext_addr K-Net Extension Id
 * \param mod_addr K-Net Module Id
 *
 * \return A pointer to a K-Net device descriptor or NULL if no 
 *         device found.
 */
knet_dev_t *knet_bus_find_device( knet_bus_t * bus ,
				  knet_ext_t ext_addr ,
				  knet_mod_t mod_addr ) 
{
  knet_dev_t * dev;

  dev = bus->devices;

  while ( dev != NULL ) {
    if ( dev->ext_addr == ext_addr &&
	 dev->mod_addr == mod_addr ) 
      return dev;
    dev = dev->next;
  }
  return NULL;
}				  

/*--------------------------------------------------------------------*/
/*! 
 * This function finds a K-Net device descriptor given its 
 * Device Name.
 *
 * \param bus      K-Net Bus Descriptor
 * \param dev_name Device Name
 *
 * \return A pointer to a K-Net device descriptor or NULL if no 
 *         device found.
 */
knet_dev_t *knet_bus_find_device_by_name( knet_bus_t * bus ,
					  const char * dev_name )
{
  knet_dev_t * dev;

  dev = bus->devices;

  while ( dev != NULL ) {
    if ( !strcmp( dev->name , dev_name )) 
      return dev;
    dev = dev->next;
  }
  return NULL;
}				  


/*--------------------------------------------------------------------*/
/*! 
 * This function opens a K-Net device given its name.
 * 
 * \param  device        K-Net Device name to open
 * \param  preferred_bus Preferred Bus.  
 *
 * Preferred bus value:
 *        - KNET_BUS_ANY tells to choose any bus for
 *          communicating with the given device. 
 *        
 *        - KNET_BUS_I2C tells to choose an I²C bus for 
 *          communicating with the given device.
 * 
 *        - KNET_BUS_SPI tells to choose an SPI bus for
 *          communicating with the given device.
 *
 *        - KNET_BUS_RS232 tells to choose a rs232 port for
 *          communicating with the given device.
 *
 * \return A pointer to a K-Net device descriptor or 
 *         NULL if no device is found
 *
 * \remark This function is thread safe.
 */   
knet_dev_t * knet_open( const char * device , 
			int preferred_bus , 
			int argc , char * argv[] )
{
  knet_bus_t * bus = NULL;
  knet_dev_t * dev;
  //char *name;
  knet_mod_t mod_adr;
  knet_ext_t ext_adr;
  kb_device_config_t  * mod;
  kb_section_config_t * sec;
  //char * mod_name , *ext_name;
  char * dev_name;

#if 0
  name = KB_ALLOC( char , strlen(device)+1 );

  strcpy( name , device );

  ext_name = name;
  mod_name = strchr( name , '%' );
  if ( mod_name != NULL ) {
    *mod_name = '\0';
    mod_name++;
  }
  
  if ((ext = kb_lookup_device( ext_name )) == NULL) {
  /  kb_error( __FILE__ ,
	      __LINE__ ,
	      "knet_open" ,
	      KB_ERROR_NODEVFOUNDSTR , ext_name );
    kb_free(name);
    return NULL;
  }

  if ( mod_name == NULL ) {
    mod = ext;
    ext = NULL;
  }
  else {
    if ((mod = kb_lookup_device( mod_name )) == NULL) {
      kb_error( __FILE__ ,
	        __LINE__ ,
	        "knet_open" ,
	        KB_ERROR_NODEVFOUNDSTR ,  mod_name );
      kb_free( name );
      return NULL;
    }
  }

  kb_free(name);
#endif

  if ((mod = kb_lookup_device( device )) == NULL) {
    KB_ERROR( "knet_open" , KB_ERROR_NODEVFOUNDSTR , device );
    return NULL;
  }

  sec = mod->section;
  bus = NULL; 
    
  /* Check first if the device should be accessed as a module */
  if((mod->kclass[ KB_DEVICE_CLASS_MODULE ].defined && preferred_bus == KNET_BUS_ANY) ||
     (sec->module_bus != KNET_BUS_ANY && preferred_bus == sec->module_bus          ))
  {
    if(sec->module_bus == KNET_BUS_ANY)	{
      KB_ERROR("knet_open",KB_ERROR_NOMODBUS);
      return NULL;
    }
    
    /* Store the module bus address from the section */
    mod_adr = sec->module_bus_addr;
    if(mod_adr < 0) {
      KB_ERROR("knet_open",KB_ERROR_NOBUSADDR);
      return NULL;
    }

#ifdef KNET_BUS_SPI
    if(sec->module_bus == KNET_BUS_SPI)
    {
      bus      = &knet_busses[ KNET_BUS_SPI ];
      ext_adr  = mod->kclass[ KB_DEVICE_CLASS_MODULE].address;
    }
#endif
    
#ifdef KNET_BUS_I2C
    if(sec->module_bus == KNET_BUS_I2C)
    {
      bus      = &knet_busses[ KNET_BUS_I2C ];
      mod_adr  = mod->kclass[ KB_DEVICE_CLASS_MODULE].address;
    }
#endif
    
#ifdef KNET_BUS_RS232
    if(sec->module_bus == KNET_BUS_RS232)
    {
      bus     = &knet_busses[ KNET_BUS_RS232 ];
      mod_adr = mod->kclass[ KB_DEVICE_CLASS_MODULE].address;
    }
#endif
  }
  else
  {
    /* Scan all other available bus to access the device */
#ifdef KNET_BUS_SPI
    if ( (( bus == NULL ) || ( preferred_bus == KNET_BUS_SPI )) 
	  && mod->kclass[ KB_DEVICE_CLASS_SPI ].defined ) {
      bus      = &knet_busses[ KNET_BUS_SPI ];
      mod_adr  = mod->kclass[ KB_DEVICE_CLASS_SPI ].address;
      ext_adr  = 0;
      dev_name = NULL;
    }
#endif /* KNET_BUS_SPI */

#ifdef KNET_BUS_I2C
    if ( (( bus == NULL ) || ( preferred_bus == KNET_BUS_I2C ))
	&& mod->kclass[ KB_DEVICE_CLASS_I2C ].defined ) {
      bus = &knet_busses[ KNET_BUS_I2C ];
      mod_adr  = mod->kclass[ KB_DEVICE_CLASS_I2C ].address;
      ext_adr  = 0;
      dev_name = NULL;
    }
#endif /* KNET_BUS_I2C */

#ifdef KNET_BUS_RS232
    if ( (( bus == NULL ) || ( preferred_bus == KNET_BUS_RS232 ))
	&& mod->kclass[ KB_DEVICE_CLASS_RS232 ].defined ) {
      bus = &knet_busses[ KNET_BUS_RS232 ];
      mod_adr = mod->kclass[ KB_DEVICE_CLASS_RS232 ].address;
      ext_adr = 0;
      dev_name = mod->kclass[ KB_DEVICE_CLASS_RS232 ].device_name;
    }
#endif /* KNET_BUS_RS232 */
  }

  /* no bus found */ 
  if ( (bus == NULL) || (bus->usage_counter==0) ) {
    kb_error( __FILE__ ,
	      __LINE__ ,
	      "knet_open" ,
	      KB_ERROR_NOBUSFOUND , device );
    return NULL;
  }

  /* FIXME: should we keep the sleep */
  usleep(50000);
  /* open the device */
  pthread_mutex_lock( &bus->lock );
  dev = bus->open( bus , 
		   ext_adr , mod_adr , dev_name , 
		   argc , argv );
  pthread_mutex_unlock ( &bus->lock );

  usleep(50000);
  return dev;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function closes a K-Net Device 
 * 
 * \param  dev  K-Net device descriptor to close
 *
 * \remark This function is thread safe. 
 */   
void knet_close( knet_dev_t * dev )
{
  knet_bus_t * bus = dev->bus;

  pthread_mutex_lock( &bus->lock );
  if ( dev->usage_counter > 0 ) {
    if ( bus->close != NULL ) bus->close( bus , dev );
    dev->usage_counter--;
  }
  pthread_mutex_unlock( &bus->lock );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function changes the order flags. These flags are used by
 * knet_{read|write}{8|16|32} functions to choose the desired 
 * encoding before and after reading from or writing to a K-Net 
 * device.  
 *
 * \param dev   K-Net device descriptor 
 * \param order New order flags.
 *
 * Order Flags:
 *
 *    - KNET_ORDER_LITTLE tells that the target device use little 
 *      endian encoding. In little endian, the least significant 
 *      BYTE is at the lowest address value.
 *
 *    - KNET_ORDER_BIG tells that the target device use big endian
 *      encoding. In big endian, the most significant BYTE is at 
 *      the lowest address value. 
 *
 *    - KNET_ORDER_LSB_FIRST tells that the lowest address is the 
 *      first sent. This flag takes effect only if 
 *      KNET_ORDER_REP_ADR is set.
 *      
 *    - KNET_ORDER_MSB_FIRST tells that the highest address is the
 *      first sent. This flag takes effect only if K
 *      NET_ORDER_REP_ADR is set.
 *
 *    - KNET_ORDER_BIT_SWAP tells to reverse the bit order.
 *    - KNET_ORDER_REP_ADR tells to use multiple atomic 8-bit 
 *      reads or writes in order to read or write a 16-bit or 
 *      32-bit.
 *
 * \remark This function is thread safe.
 */
void knet_set_order( knet_dev_t * dev , unsigned long order )
{
  knet_bus_t * bus = dev->bus;

  pthread_mutex_lock( &bus->lock );
  dev->order = order;
  pthread_mutex_unlock( &bus->lock);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function creates a new K-Net device descriptor  
 * 
 * \param bus      Pointer to a given KNet Bus Descriptor 
 * \param ext_addr K-Net Device Extension Id    
 * \param mod_addr K-Net Device Module Id 
 * \param name     K-Net Device Name 
 *
 * \return A pointer to a K-Net device descriptor
 *
 * \remark This function is thread safe.
 */
knet_dev_t * knet_device_create( knet_bus_t * bus , 
				 knet_ext_t   ext_addr ,
				 knet_mod_t   mod_addr ,
				 const char * name )
{
  knet_dev_t * dev = KB_ALLOC( knet_dev_t , 1 );


  dev->bus      = bus;
  dev->order    = 0;
  dev->next     = NULL;
  dev->ext_addr = ext_addr;
  dev->mod_addr = mod_addr;
  dev->usage_counter    = 0;


  dev->next     = bus->devices;
  bus->devices  = dev;


  return dev;
}

/*--------------------------------------------------------------------*/
/*!
 * This function destroys a K-Net device descriptor
 * 
 * \param dev      Pointer to K-Net device descriptor
 *
 * \return An error code:
 *                 - <0 on error
 *                 - 0 on success
 *
 * \remark This function is thread safe. 
 */
int knet_device_destroy( knet_dev_t * dev )
{
  knet_bus_t * bus  = dev->bus;
  knet_dev_t * prev = bus->devices;
  int ok;

  if ( dev->usage_counter ) {
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "knet_device_destroy" ,
		     KB_ERROR_DEVICEINUSE );
  }
  
  pthread_mutex_lock( &bus->lock );

  ok = 0;

  if ( prev == dev ) {
    bus->devices = dev->next;
    dev->next    = NULL;
    ok = 1;
  }
  else {
    while( prev != NULL ) {
      if ( prev->next == dev ) {
	prev->next = dev->next;
	dev->next = NULL;
	ok = 1;
	break;
      }
      prev = prev->next;
    } 
  }

  pthread_mutex_unlock( &bus->lock );

  if ( !ok ) {
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "knet_device_destroy" ,
		     KB_ERROR_UNLINKDEV );
  }
  
  kb_free( dev );
  return 0;
}

/*--------------------------------------------------------------------*/
/*!
 * This function transfers data to and from a given device.
 * 
 * \param dev         Pointer to a given K-Net Device Descriptor
 * \param write_buf   Pointer to the buffer that contains the data 
 *                    to be written
 * \param write_len   Length in bytes of the data in the write buffer
 * \param read_buf    Pointer to the buffer that will receive the 
 *                    data
 * \param read_len    Size of the read buffer in bytes
 *
 * \return An error code:
 *           - <0 on error
 *           - >=0 on success
 *
 * \remark This function is thread safe. 
 */
int knet_lltransfer( knet_dev_t * dev ,
		     const char * write_buf , unsigned int write_len ,
		     char * read_buf , unsigned int read_len )
{
  knet_bus_t * bus;
  int rc;

  bus = dev->bus;

  pthread_mutex_lock( &bus->lock );
  
  if ( bus->transfer == NULL ) {
    rc = bus->write( bus , dev , write_buf , write_len );
    if ( rc >= 0 ) 
      rc = bus->read( bus , dev , read_buf , read_len );
  }
  else 
    rc = bus->transfer( bus , dev , write_buf , write_len , read_buf , read_len );
  
  pthread_mutex_unlock( &bus->lock );

  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function reads data from a given device
 *
 * \param dev   K-Net device descriptor 
 * \param buf   Pointer to the buffer that will receive the data
 * \param len   Size of the buffer
 *
 * \return A value:
 *        - <0 on error
 *        - >=0 number of bytes read 
 *
 * \remark This function is thread safe.
 */

int knet_llread( knet_dev_t * dev , 
		 char *buf , 
		 unsigned int len )
{
  knet_bus_t * bus;
  int rc;
  
  bus = dev->bus;

  pthread_mutex_lock(&bus->lock);
  rc = bus->read( bus , dev , buf , len );
  pthread_mutex_unlock(&bus->lock);
  
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function writes data into a given device.
 *
 * \param dev  K-Net device descriptor
 * \param buf  Pointer to the buffer that contains the data to be 
 *             written
 * \param len  Length in bytes of the data in the buffer
 *
 * \return A value:
 *        - <0 on error
 *        - >=0 number of bytes written 
 *
 * \remark This function is thread safe.
 */
int knet_llwrite( knet_dev_t *  dev ,
		  const char  * buf ,
		  unsigned int  len )
{
  knet_bus_t * bus;
  int rc;
  
  bus = dev->bus;

  pthread_mutex_lock(&bus->lock);
  rc = bus->write( bus , dev , buf , len );
  pthread_mutex_unlock(&bus->lock);
  
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function converts a data buffer into the encoding of a given 
 * K-Net device.
 * 
 * \param dev    K-Net device descriptor 
 * \param buf    Pointer to the buffer that contains the data to 
 *               convert
 * \param len    Number of data bytes in the buffer
 * \param type   Size of the primitive type in the buffer:
 *                -  1   8-bit
 *                -  2  16-bit
 *                -  4  32-bit
 *             
 * \return An error code
 *         - <0 on error
 *         - 0 on success  
 */
int knet_convert( knet_dev_t * dev , char * buf , unsigned int len , int type ) 
{
  unsigned int pos;
  unsigned char tmp;

  if ( type != 1 && type != 2 && type != 4 ) 
    return -1;
   
  if ( dev->order & KNET_ORDER_BIT_SWAP ) {
    for (pos=0; pos<len; pos++) {
      buf[pos] = bit_conv_tab[buf[pos]];
    }
  }
    
  if ( type > 1 && 
       (dev->order & KNET_ORDER_BIG) ) {
    
    if ((len%type)) return -1;

    for (pos=0; pos<(len/type); pos++) {
      if ( type == 2 ) {
	tmp = buf[2*pos];
	buf[2*pos] = buf[2*pos+1];
	buf[2*pos+1] = tmp;
      }
      else {
	tmp = buf[4*pos];
	buf[4*pos] = buf[4*pos+3];
	buf[4*pos+3] = tmp;
	tmp = buf[4*pos+1];
	buf[4*pos+1] = buf[4*pos+2];
	buf[4*pos+2] = tmp;
      }
    }
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function reads a 8-bit value of data from a K-Net device.
 *
 * \param dev   K-Net device descriptor
 * \param reg   K-Net register address
 * \param val   Pointer to a variable that will receive the value
 *
 * \return An error code:
 *         - <0 on error
 *         - >=0 on success 
 *
 * \remark This function is thread safe.
 */
int knet_read8( knet_dev_t * dev , 
		knet_reg_t reg , 
		unsigned char * val )
{
  int rc;

  if ( dev->order & KNET_ORDER_BIT_SWAP ) 
    reg = bit_conv_tab[reg];
   
  rc = knet_lltransfer( dev , &reg , 1 , val , 1 );

  if ( dev->order & KNET_ORDER_BIT_SWAP ) 
    (*val) = bit_conv_tab[(*val)];

  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function reads a 16-bit value from a given K-Net device
 *
 * \param dev   K-Net device descriptor
 * \param reg   K-Net register address
 * \param val   A pointer to a variable that will receive the 
 *              16-bit value
 *
 * \return A value:
 *       - <0 on error
 *       - >= on success
 *
 * \remark This function is thread safe.
 */
int knet_read16( knet_dev_t * dev , 
		 knet_reg_t reg , 
		 unsigned short * val )
{
  int rc;
  unsigned char * p = (unsigned char *) val;
  unsigned char t , treg[2];
  

  /* Bit Swap needed */
  if ( dev->order & KNET_ORDER_BIT_SWAP ) {
    treg[0] = bit_conv_tab[reg];
    treg[1] = bit_conv_tab[reg+1];
  }
  else {
    treg[0] = reg;
    treg[1] = reg+1;
  }

  /* Repeat the address at each transfer */
  if ( dev->order & KNET_ORDER_REP_ADR ) {
    unsigned char * p = (unsigned char *)val;

    /* Read MSB first */
    if ( dev->order & KNET_ORDER_MSB_FIRST ) {
     
      if ((rc=knet_lltransfer( dev , &treg[1] , 1 , (char *)&p[1] , 1 )) < 0 )
	return rc;

      if ((rc=knet_lltransfer( dev , &treg[0] , 1 , (char *)&p[0] , 1 )) < 0 )
	return rc;
    }
    /* Read LSB first */
    else {

      if ((rc=knet_lltransfer( dev , &treg[0] , 1 , (char *)&p[0] , 1 )) < 0 )
	return rc;
      
      if ((rc=knet_lltransfer( dev , &treg[1] , 1 , (char *)&p[1] , 1 )) < 0 )
	return rc;
    }
  }
  else {
    rc = knet_lltransfer( dev , &treg[0] , 1 , (char *)val , 2 );
    if ( rc < 0 )
      return rc;
  }

  /* need to convert result from big endian */
  if ( dev->order & KNET_ORDER_BIG ) {
    t = p[0];
    p[0] = p[1];
    p[1] = t;
  }

  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function reads a 32-bit value from a given K-Net device.
 *
 * \param dev   K-Net device descriptor
 * \param reg   K-Net register address
 * \param val   A pointer to a 32-bit value that will receive 
 *              the value
 * 
 * \remark This function is thread safe.
 */
int knet_read32( knet_dev_t * dev , 
		 knet_reg_t reg , 
		 unsigned long * val )
{
  int rc;
  unsigned char * p = (unsigned char *) val;
  unsigned char t , treg[4];
  
  /* Bit Swap needed */
  if ( dev->order & KNET_ORDER_BIT_SWAP ) {
    treg[0] = bit_conv_tab[reg];
    treg[1] = bit_conv_tab[reg+1];
    treg[2] = bit_conv_tab[reg+2];
    treg[3] = bit_conv_tab[reg+3];
  }
  else {
    treg[0] = reg;
    treg[1] = reg+1;
    treg[2] = reg+2;
    treg[3] = reg+3;
  }

  /* Repeat the address at each transfer */
  if ( dev->order & KNET_ORDER_REP_ADR ) {
    unsigned char * p = (unsigned char *)val;

    /* Read MSB first */
    if ( dev->order & KNET_ORDER_MSB_FIRST ) {

      if ((rc=knet_lltransfer( dev , &treg[3] , 1 , &p[3] , 1 )) < 0 )
	return rc;

      if ((rc=knet_lltransfer( dev , &treg[2] , 1 , &p[2] , 1 )) < 0 )
	return rc;

      if ((rc=knet_lltransfer( dev , &treg[1] , 1 , &p[1] , 1 )) < 0 )
	return rc;

      if ((rc=knet_lltransfer( dev , &treg[0] , 1 , &p[0] , 1 )) < 0 )
	return rc;

    }
    /* Read LSB first */
    else {
      
      if ((rc=knet_lltransfer( dev , &treg[0] , 1 , &p[0] , 1 )) < 0 )
	return rc;

      if ((rc=knet_lltransfer( dev , &treg[1] , 1 , &p[1] , 1 )) < 0 )
	return rc;

      if ((rc=knet_lltransfer( dev , &treg[2] , 1 , &p[2] , 1 )) < 0 )
	return rc;

      if ((rc=knet_lltransfer( dev , &treg[3] , 1 , &p[3] , 1 )) < 0 )
	return rc;
    }
  }
  else {
    rc = knet_lltransfer( dev , &treg[0] , 1 , (char *)val , 4 );
    if ( rc < 0 )
      return rc;
  }

  /* need to convert result from big endian */
  if ( dev->order & KNET_ORDER_BIG ) {
    t = p[0];
    p[0] = p[3];
    p[3] = t;
    t = p[1];
    p[1] = p[2];
    p[2] = t;
  }
}

/*--------------------------------------------------------------------*/
/*!
 * This function reads a x Bytes value from a given K-Net device.
 *
 * \param dev   K-Net device descriptor
 * \param reg   K-Net register address
 * \param val   A pointer to a table value that will receive
 *              the value
 * \param rx_len the number of Bytes to be read
 *
 * \remark This function is thread safe.
 */
int knet_readxBytes( knet_dev_t * dev ,
                 knet_reg_t reg ,
                 unsigned char * val, unsigned char rx_len )
{
  int rc;

  /* Bit Swap needed */
  if ( dev->order & KNET_ORDER_BIT_SWAP ) {
    reg = bit_conv_tab[reg];
  }
  rc = knet_lltransfer( dev , &reg , 1 , val , rx_len );
  if(rc < 0)
    return rc;



}

/*--------------------------------------------------------------------*/
/*! 
 * This function writes an 8-bit value to a given K-Net device.
 * 
 * \param dev K-Net device descriptor 
 * \param reg K-Net register address
 * \param val an 8-bit value
 *
 * \return A value:
 *       - <0 on error
 *       - >=0 on success
 *
 * \remark This function is thread safe.
 */
int knet_write8( knet_dev_t * dev ,
		 knet_reg_t reg ,
		 unsigned char val )
{
  unsigned char buf[2];

  if ( dev->order & KNET_ORDER_BIT_SWAP ) {
    buf[0] = bit_conv_tab[reg];
    buf[1] = bit_conv_tab[val];
  } 
  else {
    buf[0] = reg;
    buf[1] = val;
  }
     
  return knet_llwrite( dev , buf , 2 );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function writes an 16-bit value to a given K-Net device.
 *
 * \param dev   K-Net device descriptor
 * \param reg   K-Net register address 
 * \param val   a 16-bit value
 *
 * \return A value:
 *         - <0 on error
 *         - >=0 on success
 * 
 * \remark This function is thread safe.
 */
int knet_write16( knet_dev_t * dev ,
		  knet_reg_t reg ,
		  unsigned short val )
{
  unsigned char buf[3];
  unsigned char *p = (unsigned char *)&val;
  unsigned char t , treg[2];
  int rc;

  if ( dev->order & KNET_ORDER_BIT_SWAP ) {
    treg[0] = bit_conv_tab[reg];
    treg[1] = bit_conv_tab[reg+1];
  }
  else {
    treg[0] = reg;
    treg[1] = reg+1;
  }

  if ( dev->order & KNET_ORDER_BIG ) {
    t = p[0];
    p[0] = p[1];
    p[1] = t;
  }
  
  if ( dev->order & KNET_ORDER_REP_ADR ) {
    

    if ( dev->order & KNET_ORDER_MSB_FIRST ) {
      
      buf[0] = treg[1];
      buf[1] = p[1];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
    
      buf[0] = treg[0];
      buf[1] = p[0];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
      
    }
    else {
      
      buf[0] = treg[0];
      buf[1] = p[0];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
      
      buf[0] = treg[1];
      buf[1] = p[1];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
    
    }
  }
  else {
    rc=knet_llwrite( dev , buf , 3 );
  }
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function writes a 32-bit value to a given K-Net device.
 *
 * \param dev   K-Net device descriptor
 * \param reg   K-Net register address 
 * \param val   a 32-bit value
 *
 * \return A value:
 *           - <0 on error
 *           - >=0 on success
 *
 * \remark This function is thread safe.
 */
int knet_write32( knet_dev_t * dev ,
		  knet_reg_t reg ,
		  unsigned long val )
{
  unsigned char buf[5];
  unsigned char *p = (unsigned char *)&val;
  unsigned char t , treg[4];
  int rc;

  if ( dev->order & KNET_ORDER_BIT_SWAP ) {
    treg[0] = bit_conv_tab[reg];
    treg[1] = bit_conv_tab[reg+1];
    treg[2] = bit_conv_tab[reg+2];
    treg[3] = bit_conv_tab[reg+3];  
  }
  else {
    treg[0] = reg;
    treg[1] = reg+1;
    treg[2] = reg+2;
    treg[3] = reg+3;
  }

  if ( dev->order & KNET_ORDER_BIG ) {
    t = p[0];
    p[0] = p[3];
    p[3] = t;
    t = p[1];
    p[1] = p[2];
    p[2] = t;
  }
  
  if ( dev->order & KNET_ORDER_REP_ADR ) {
    
    if ( dev->order & KNET_ORDER_MSB_FIRST ) {
      
      buf[0] = treg[3];
      buf[1] = p[3];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;

      buf[0] = treg[2];
      buf[1] = p[2];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;

      buf[0] = treg[1];
      buf[1] = p[1];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
    
      buf[0] = treg[0];
      buf[1] = p[0];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
      
    }
    else {
      
      buf[0] = treg[0];
      buf[1] = p[0];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
      
      buf[0] = treg[1];
      buf[1] = p[1];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
      
      buf[0] = treg[2];
      buf[1] = p[2];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
      buf[0] = treg[3];
      buf[1] = p[3];

      if ((rc=knet_llwrite( dev , buf , 2 )) < 0 )
	return rc;
    }
  }
  else {
    rc=knet_llwrite( dev , buf , 3 );
  }
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function reads an 8-bit value from a given K-Net device 
 * and return this value. 
 *
 * \param dev  K-Net device descriptor
 * \param reg  K-Net register address
 * \return A value:
 *         - <0 on error
 *         - >=0 the value read
 *
 * \remark This function is thread safe.
 */
int knet_read( knet_dev_t * dev , knet_reg_t reg )
{
  unsigned char val;
  int rc;
  
  if ((rc = knet_read8( dev , reg , &val )) < 0)
    return rc;

  return val;
}

/*--------------------------------------------------------------------*/
/*! This function writes into a device using a vprintf-like formatting
 *  function.
 *
 * \param dev    K-Net device descriptor
 * \param format printf-like formatted string.
 * \param argptr arguments list 
 *
 * \return <0 on error and >=0 number of characters written.
 */

int knet_vprintf( knet_dev_t * dev , 
		  const char * format , 
		  va_list argptr )
{
  int rc;
  
  pthread_mutex_lock(&knet_buffer_lock);
  for (;;) {
    rc = vsnprintf( knet_buffer , knet_buffer_size , format , argptr );
   
    if ( knet_buffer_size < rc ) {
      /* buffer too small => enlarge it */
      while ( knet_buffer_size < rc ) {
	knet_buffer_size *= 2;
      }
      knet_buffer = kb_realloc( knet_buffer , knet_buffer_size );
    }
    else
      break;
  }
  pthread_mutex_unlock(&knet_buffer_lock);

  return knet_llwrite( dev , knet_buffer , rc );
}

/*--------------------------------------------------------------------*/
/*! This function writes into a device using a printf-like formatting
 *  function.
 *
 * \param dev    K-Net device descriptor
 * \param format printf-like formatted string.
 * 
 * \return <0 on error and >=0 number of characters written.
 */

int knet_printf( knet_dev_t * dev , const char * format , ... )
{
  va_list argptr;
  int rc;

  pthread_mutex_lock(&knet_buffer_lock);
  va_start( argptr , format );
  for (;;) {
    rc = vsnprintf( knet_buffer , knet_buffer_size , format , argptr );
   
    if ( knet_buffer_size < rc ) {
      /* buffer too small => enlarge it */
      while ( knet_buffer_size < rc ) {
	knet_buffer_size *= 2;
      }
      knet_buffer = kb_realloc( knet_buffer , knet_buffer_size );
    }
    else
      break;
  }
  va_end( argptr );
  pthread_mutex_unlock(&knet_buffer_lock);

  return knet_llwrite( dev , knet_buffer , rc );
}

/*--------------------------------------------------------------------*/
/*! Read a character string from the given knet device.
 * If the terminator character is not read, maximum size characters are
 * read. The terminator is not removed from the string. 
 */

int knet_read_string( knet_dev_t * dev , 
		      char * buffer , 
		      unsigned int size , 
		      unsigned char terminator )
{
  unsigned int pos=0;
  unsigned char ch;
  int rc;

  for (;;) {
    if ((rc = knet_llread( dev , &ch , 1 )) < 0 )
      return rc;
    if ( rc > 0 ) {
      buffer[pos++] = ch;
      buffer[pos]   = '\0';

      if ( pos == size || ch == terminator  ) 
	return pos;
    }
  }
}
