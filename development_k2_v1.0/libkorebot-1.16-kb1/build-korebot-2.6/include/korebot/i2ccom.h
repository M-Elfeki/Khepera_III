/*--------------------------------------------------------------------
 * i2ccom.h - I²C Low-Level Communication Layer	
 *--------------------------------------------------------------------
 * $Id: i2ccom.h,v 1.1 2004/07/29 10:51:54 cgaudin Exp $
 *--------------------------------------------------------------------
 * Original author: Yves Piguet, 2004
 * $Author: cgaudin $
 * $Date: 2004/07/29 10:51:54 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

/*! \file i2ccom.h I²C Low-Level Communication Layer */

#ifndef __i2ccom__
#define __i2ccom__

/*-------------------------------------------------------------------*/
/*! I²C Device Address Definition
 */
typedef unsigned int  i2c_dev_t; /* device address */


/*--------------------------------------------------------------------*/
/*! I²C Bus Handle Definition
 */
typedef struct
{
  /*! File descriptor of the I²C bus device */ 
  int       fd;     
  /*! Address of the currently selected I²C device */
  i2c_dev_t dev; 
} 
i2c_t;


/*--------------------------------------------------------------------
 * Public Prototypes Declaration 
 */

extern int  i2c_open( i2c_t *i2c , const char * device );

extern void i2c_close( i2c_t *i2c );

extern int  i2c_llread( i2c_t *i2c , 
			i2c_dev_t dev , 
			char * buf , unsigned int len );

extern int  i2c_llwrite( i2c_t *i2c , 
			 i2c_dev_t dev , 
			 const char * buf , unsigned int len );

extern int  i2c_lltransfer( i2c_t * i2c , 
			    i2c_dev_t dev , 
			    const char * write_buf , 
			    unsigned int write_len , 
			    char * read_buf , 
			    unsigned int read_len );

extern int   i2c_exists( i2c_t * i2c , i2c_dev_t dev );

extern int   i2c_scan( i2c_t * i2c , 
		       int (*callback)( i2c_t * i2c , 
					i2c_dev_t dev , 
					void * context ) , 
		       void * context);

#endif /* __i2ccom__ */
