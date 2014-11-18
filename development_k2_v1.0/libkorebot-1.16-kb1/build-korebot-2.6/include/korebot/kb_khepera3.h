/*--------------------------------------------------------------------
 * kb_khepera3.h - KoreBot Library - Khepera3 functions
 *--------------------------------------------------------------------
 * $Id: kb_khepera3.h,v 1.3 2006/08/24 12:33:46 flambercy Exp $
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Date: 2006/08/24 12:33:46 $
 * $Revision: 1.3 $
 *-------------------------------------------------------------------*/ 

#ifndef __khepera3__
#define __khepera3__

#include "knet.h"

/*!
 *  Khepera3 <-> Korebot gpios pin definitions
 */
#define KNET_INT0        40      /* KNET_INT0 is on GPIO40 */
#define IN               1
#define OUT		 0


/*! 
 *  Khepera3 Error codes
 */ 
#define KH3_ERROR_SUCCESS	0		/* No errors */
#define KH3_ERROR_KBINIT	-1		/* Unable to initialize the korebot library */
#define KH3_ERROR_FRMSZ   	-2		/* Frame received doesnt have the expected size */
#define KH3_ERROR_SZFMT   	-3		/* Frame size received should be 1 byte wide, wrong format */
#define KH3_ERROR_FRMSND  	-4		/* Unable to send a command frame to the khepera3 */

/*!
 * Khepera3 command delay
 */
#define K3_CMD_DELAY	    300


/*!--------------------------------------------------------------------
 * Prototypes Declaration
 */
/* Need to be called before any other functions */
extern int kh3_init( void );

/* "Low level" function to communicate with the khepera3 via i2c */
extern int kh3_getcommand( knet_dev_t * , unsigned char *);
extern int kh3_sendcommand( knet_dev_t *, unsigned char *);

/* "High level" function that let user to simply retrieves various informations from the robot */
extern int kh3_proximity_ir(char *, knet_dev_t *);
extern int kh3_ambiant_ir(char *, knet_dev_t *);
extern int kh3_battery_voltage(char *, unsigned char, knet_dev_t *);
extern int kh3_reset_tstamp(char *, knet_dev_t *);
extern int kh3_revision(char *, knet_dev_t *);
extern int kh3_configure_os(char *, unsigned char, unsigned char, knet_dev_t *);
extern int kh3_measure_us(char *, unsigned char, knet_dev_t *);	
	


#endif /* __khepera3__ */
