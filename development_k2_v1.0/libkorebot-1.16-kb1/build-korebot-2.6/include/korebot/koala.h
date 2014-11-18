/*--------------------------------------------------------------------
 * koala.c - KoreBot Library - Koala Robot Support for KoreBot 
 *--------------------------------------------------------------------
 * $Id: koala.h,v 1.1 2004/09/22 08:29:15 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Date: 2004/09/22 08:29:15 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#ifndef __koa__
#define __koa__


/*--------------------------------------------------------------------*/
/* Public Function Prototypes
 */
int koa_getOSVersion( knet_dev_t * dev , 
		      unsigned char *version , 
		      unsigned char *revision,
   		      unsigned char *pversion,
   		      unsigned char *prevision);
#endif /*__koa__*/
