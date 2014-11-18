/*--------------------------------------------------------------------
 * pcf8574.h - KoreBot Library - Philips PCF8574 Support
 *--------------------------------------------------------------------
 * $Id: pcf8574.h,v 1.1 2004/09/07 17:54:07 cgaudin Exp $
 *--------------------------------------------------------------------	
 * $Author: cgaudin $
 * $Date: 2004/09/07 17:54:07 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#ifndef __pcf8574__
#define __pcf8574__

/*--------------------------------------------------------------------
 * Public Prototypes Declaration
 */
extern int pcf8574_Read( knet_dev_t * dev ,
			 unsigned char *value ,
			 unsigned char mask );

extern int pcf8574_Write( knet_dev_t * dev ,
			  unsigned char value );


#endif /* __pcf8574__ */

