/*--------------------------------------------------------------------
 * ltc1663.h - KoreBot Library - Linear LTC1663 Support
 *--------------------------------------------------------------------
 * $Id: ltc1663.h,v 1.1 2004/07/29 10:51:56 cgaudin Exp $
 *--------------------------------------------------------------------	
 * $Author: cgaudin $
 * $Date: 2004/07/29 10:51:56 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#ifndef __ltc1663__
#define __ltc1663__

/*! LTC1663 have different I²C address [0x20..0x27] */
#define LTC1663_ADDR(x)   (0x20|(x&7))

/*! Selects the type of update */
#define LTC1663_SY        (1<<0)  /* SY */

/*! Enable or disable the power-down mode */
#define LTC1663_SD        (1<<1) 

/*! 
 * Selects the power supply or the internal
 * bandgap (+2.5V) as reference 
 */
#define LTC1663_BG        (1<<2)  /* BG */

/*--------------------------------------------------------------------
 * Public Prototypes Declaration
 */
extern int ltc1663_SendCommand( const char * device ,
				unsigned char command , 
				unsigned int value );


#endif /* __ltc1663__ */

