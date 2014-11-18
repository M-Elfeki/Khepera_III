/*--------------------------------------------------------------------
 * pcf8574.c - Philips PCF8574 Remote 8-bit I/O Expander Support 	
 *--------------------------------------------------------------------
 * $Id: pcf8574.c,v 1.1 2004/09/07 17:54:07 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Date: 2004/09/07 17:54:07 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*! 
 * \file   pcf8574.c Philips PCF8574 Remote 8-bit I/O Expander Support
 *
 * \brief
 *         This module provides all the primitives to 
 *         read and write data to/from a PCF8574 Remote 
 *         8-bit I/O Expander. 
 *
 * \author   Cédric Gaudin (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.
 * \todo     nothing.
 */

/*--------------------------------------------------------------------*/
/*!
 * This function reads 8-bit from the Remote I/O Expander
 * 
 * \param dev    K-Net Device Descriptor
 * \param value  The value read.
 *
 * \returns A value :
 *                    - <0 on error
 *                    - >=0 on success
 */
int pcf8574_Read( knet_dev_t * dev , 
		  unsigned char *value , 
		  unsigned char mask )
{
  int rc=0;
 
  if ( value != NULL ) {

    if ((rc=knet_lltransfer( dev , 
			     &mask , 1 ,
			     value , 1 )) < 0 )
      return rc;
  }

  return rc;
}

/*--------------------------------------------------------------------*/
/*!
 * This function writes 8-bit from the Remote I/O Expander
 * 
 * \param dev    K-Net Device Descriptor
 * \param value  The value to write.
 *
 * \returns A value :
 *                    - <0 on error
 *                    - >=0 on success
 */
int pcf8574_Write( knet_dev_t * dev , unsigned char value )
{
  int rc;
  
  rc=knet_llwrite( dev , &value , 1 );
  
  return rc;
}
