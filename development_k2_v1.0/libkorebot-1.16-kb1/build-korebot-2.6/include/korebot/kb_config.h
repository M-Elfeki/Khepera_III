/*--------------------------------------------------------------------
 * kb_config.h - KoreBot Library - Configuration Handling
 *--------------------------------------------------------------------
 * $Id: kb_config.h,v 1.3 2004/09/22 08:29:14 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Revision: 1.3 $
 * $Date: 2004/09/22 08:29:14 $
 *--------------------------------------------------------------------*/

#ifndef __kb_config__
#define __kb_config__

/*!
 * Define the location for libkorebot configuration files. Can be modified
 * in the top level Makefile editing KB_CONFIG_DIRECTORY.
 */
#define KB_CONFIG_DIR_INTERNAL KB_CONFIG_DIRECTORY "/"

/*--------------------------------------------------------------------
 * Device Class Definition
 */

/*! Any device class */
#define KB_DEVICE_CLASS_ANY      -1

/*! I²C Device */
#define KB_DEVICE_CLASS_I2C       0 

/*! SPI Device */
#define KB_DEVICE_CLASS_SPI       1

/*! Module */
#define KB_DEVICE_CLASS_MODULE    2

/*! RS-232 Device */
#define KB_DEVICE_CLASS_RS232     3

/*! Maximum number of device classes */
#define KB_DEVICE_MAX_CLASS       4 

/*--------------------------------------------------------------------*/
/*! Section Config Definition 
 */
typedef struct kb_section_config_s 
{
  /*! Module bus */
  int module_bus;

  /*! Module bus address */
  int module_bus_addr;

  /*! Number of devices */
  unsigned int  device_count;

  /*! Pointer to the head to the device list */
  kb_symbol_t * devices;

  /*! Number of registers */
  unsigned int  register_count;

  /*! Pointer to the head to the register list */
  kb_symbol_t * registers;

  /*! Number of aliases */
  unsigned int  alias_count;

  /*! Pointer to the head to the alias list */
  kb_symbol_t * aliases;

  /*! Pointer to the next section */
  kb_symbol_t * next;
}
kb_section_config_t;

/*--------------------------------------------------------------------*/
/*! Device Config Definition 
 */
typedef struct kb_device_config_s 
{
  /*! Device class */
  struct 
  {
    int   defined;
    int   address;
    char  device_name[ KB_SYMBOL_NAME_SIZE ];
  }
  kclass[ KB_DEVICE_MAX_CLASS ];

  /*! Pointer to the next device */
  kb_symbol_t * next;

  /*! Pointer to the corresponding section */
  kb_section_config_t * section;
}
kb_device_config_t;

/*--------------------------------------------------------------------*/
/*! Register Configuration Descriptor Definition 
 */
typedef struct kb_register_config_s 
{
  /*! Register value */
  int value;
  
  /*! Pointer to the next register */
  kb_symbol_t * next;
}
kb_register_config_t;

/*--------------------------------------------------------------------*/
/*! Alias Configuration Descriptor Definition
 */
typedef struct kb_alias_config_s
{
  /*! Name to the aliased symbol */
  char  name[ KB_SYMBOL_NAME_SIZE ];

  /*! Pointer to the aliased symbol */
  kb_symbol_t * ptr;

  /*! Pointer to the next alias */
  kb_symbol_t * next;
}
kb_alias_config_t;


/*--------------------------------------------------------------------
 * Prototype Declaration
 */
extern int           kb_config_init( int argc , char * argv[] );

extern void          kb_config_exit( void );

extern kb_device_config_t *   kb_lookup_device( const char * name );

extern kb_register_config_t * kb_lookup_register( const char * name );

extern int           kb_parse_config_file( const char * file );

extern int           kb_enum_section( int (*func)( const char * name , 
						   kb_section_config_t * section ,
						   void * context ) ,
				      void * context );

extern int          kb_enum_alias( const char * section ,
				   int (*func)( const char * name ,
						kb_alias_config_t * alias ,
						void * context ) ,
				   void * context );

extern int           kb_enum_device( const char * section ,
				     int (*func)( const char * name , 
						  kb_device_config_t * device , 
						  void * context ) ,
				     void * context );

extern int           kb_enum_register( const char * section ,
				       int (*func)( const char * name ,
						    kb_register_config_t * reg ,
						    void * context ) ,
				       void * context );


#endif /* __kb_config__ */
