/*--------------------------------------------------------------------
 * knet.h - KoreBot Library - K-Net Abstraction Layer
 *--------------------------------------------------------------------
 * $Id: knet.h,v 1.5 2006/08/14 14:56:43 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Date: 2006/08/14 14:56:43 $
 * $Revision: 1.5 $
 *-------------------------------------------------------------------*/ 

#ifndef __knet__
#define __knet__

#ifdef __cplusplus
extern "C" {
#endif

/*! 
 * K-Net Busses Constants
 */
#define KNET_BUS_ANY    -1 /*! Any bus */
#define KNET_BUS_I2C     0 /*! I²C Bus */
#define KNET_BUS_SPI     1 /*! SPI Bus */
#define KNET_BUS_RS232   2 /*! RS-232 */
#define KNET_MAX_BUSSES  3 /*! Maximum number of busses */

/*!
 * List of existing knet bus names, as they should
 * appear in configuration files. Device class names
 * are defined in kb_config.h
 */
extern const char * knet_bus_names[];

/*-------------------------------------------------------------------- 
 * K-Net Order Constants
 */
#define KNET_ORDER_LITTLE    (0<<0) /*! Little Endian */
#define KNET_ORDER_BIG       (1<<0) /*! Big Endian */
#define KNET_ORDER_LSB_FIRST (0<<1) /*! LSB is transfered first */
#define KNET_ORDER_MSB_FIRST (1<<1) /*! MSB is transfered first */
#define KNET_ORDER_BIT_SWAP  (1<<2) /*! Reverse the bits order */
#define KNET_ORDER_REP_ADR   (1<<3) /*! Repeat Address for each byte */

/*--------------------------------------------------------------------
 * K-Net Type Definitions
 */
typedef unsigned int  knet_ext_t;       /*! Extension ID              */
typedef unsigned int  knet_mod_t;       /*! Module ID                 */
typedef unsigned char knet_reg_t;       /*! Register Number           */
typedef void *        knet_bus_info_t;  /*! Bus Private Data Pointer  */
typedef void *        knet_dev_info_t;  /*! Device Private Data Pointer */

/*--------------------------------------------------------------------*/
/*! K-Net Generic Bus Descriptor 
 */
struct knet_dev_s;
typedef struct knet_bus_s {
  
  /*! cleanup the private data in the bus structure [OPTIONAL] */
  int (*exit)       ( struct knet_bus_s * bus );
    
  /*! open a device on this bus [MANDATORY] */
  struct knet_dev_s *(*open) ( struct knet_bus_s * bus , 
			       knet_ext_t           ext ,
			       knet_mod_t           mod ,
			       const char         * name ,
			       int argc , char * argv[] );
  
  /*! close a device [OPTIONAL] */
  void (*close)      ( struct knet_bus_s * bus , 
		       struct knet_dev_s * device );
  
  /*! device transfer (write + read ) data [OPTIONAL] */
  int (*transfer)   ( struct knet_bus_s * bus ,
		      struct knet_dev_s * device ,
		      const char * write_buf , unsigned int write_len ,
		      char * read_buf , unsigned read_len );

  /*! device read data [MANDATORY] */
  int (*read)       ( struct knet_bus_s * bus ,
		      struct knet_dev_s * device ,  
		      char *       buf ,
		      unsigned int len );
  
  /*! device write data [MANDATORY] */
  int (*write)      ( struct knet_bus_s * bus ,
		      struct knet_dev_s * device , 
		      const char * buf ,
		      unsigned int len );

  /*! A pointer to the head to the device list for this bus */
  struct knet_dev_s * devices;

  /*! This lock is used to be thread safe when accessing a bus */
  pthread_mutex_t   lock;

  /*! This member is dedicated to the lower communication layer */
  knet_bus_info_t   bus_info;

  int               usage_counter;
}
knet_bus_t;

/*--------------------------------------------------------------------
 *! K-Net Generic Device Descriptor
 */
typedef struct knet_dev_s {

  /*! K-Net of this device */
  knet_bus_t     *bus;

  /*! Order Flags. See knet_set_order */
  unsigned long   order;
  
  /*! K-Net extension address */
  knet_ext_t      ext_addr;

  /*! K-Net module address */
  knet_mod_t      mod_addr;

  /*! K-Net device name */
  const char      *name;
  
  unsigned int    usage_counter;
  
  /*! This member is dedicated to the lower communication layer */
  knet_dev_info_t  info;
  
  /*! A pointer to the next device on the list */
  struct knet_dev_s *next; 
} 
knet_dev_t;



/*--------------------------------------------------------------------
 * Prototypes Declaration
 */
extern int knet_init( int argc , char * argv[] );

extern void knet_exit( void );

extern knet_dev_t * knet_bus_find_device( knet_bus_t * bus ,
					  knet_ext_t   ext_addr ,
					  knet_mod_t   mod_addr );

extern knet_dev_t *knet_bus_find_device_by_name( knet_bus_t * bus ,
						 const char * dev_name );

extern knet_dev_t * knet_open( const char * device ,
			       int preferred_bus , 
			       int argc , char * argv[] );

extern void knet_set_order( knet_dev_t * dev , unsigned long order );

extern void knet_close( knet_dev_t * dev );

extern knet_dev_t * knet_device_create( knet_bus_t * bus ,
					knet_ext_t   ext_addr ,
					knet_mod_t   mod_addr ,
	       				const char * name );

extern int knet_device_destroy( knet_dev_t * dev );

extern int knet_lltransfer( knet_dev_t * dev ,
			    const char * write_buf , 
			    unsigned int write_len ,
			    char * read_buf , 
			    unsigned int read_len );

extern int knet_llread( knet_dev_t * dev ,
			char *       buf ,
			unsigned int len );

extern int knet_llwrite( knet_dev_t * dev ,
			 const char * buf ,
			 unsigned int len );

extern int knet_convert( knet_dev_t * dev ,
			 char * buf ,
			 unsigned int len ,
			 int type );

extern int knet_read8( knet_dev_t * dev ,
		       knet_reg_t   reg ,
		       unsigned char * val );

extern int knet_read16( knet_dev_t * dev ,
			knet_reg_t   reg ,
			unsigned short * val );

extern int knet_read32( knet_dev_t * dev ,
			knet_reg_t   reg ,
			unsigned long * val );
extern int knet_readxBytes( knet_dev_t * dev ,
                 knet_reg_t reg ,
                 unsigned char * val, unsigned char rx_len );


extern int knet_write8( knet_dev_t * dev ,
			knet_reg_t reg ,
			unsigned char val );

extern int knet_write16( knet_dev_t * dev ,
			 knet_reg_t reg ,
			 unsigned short val );

extern int knet_write32( knet_dev_t * dev ,
			 knet_reg_t reg ,
			 unsigned long val );

/* Convenient function to read a single byte */
extern int knet_read( knet_dev_t * dev , knet_reg_t reg );

/* Convenient function to write a single byte */
#define knet_write( dev , reg , val ) knet_write8( dev , reg , val )

extern int knet_vprintf( knet_dev_t * dev , 
			 const char * format , 
			 va_list argptr );

extern int knet_printf( knet_dev_t * dev , 
			const char * format , ... );


extern int knet_read_string( knet_dev_t * dev , 
			     char * buffer , 
			     unsigned int size , 
			     unsigned char terminator );



#ifdef __cplusplus
}
#endif

#endif /* __knet__ */
