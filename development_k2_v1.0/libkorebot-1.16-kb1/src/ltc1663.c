/*--------------------------------------------------------------------
 * ltc1663.c - Linear LTC1663 DAC Support 	
 *--------------------------------------------------------------------
 * $Id: ltc1663.c,v 1.1 2004/07/29 10:51:56 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Date: 2004/07/29 10:51:56 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*! 
 * \file   ltc1663.c Linear LTC1663 DAC Support
 *
 * \brief
 *         This module provides the primitve to send command to an 
 *         LTC1663 DAC device connected on an I²C Bus.
 *
 * \author   Cédric Gaudin (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.
 * \todo     nothing.
 */

/*--------------------------------------------------------------------*/
/*!
 * This function send a command to change the output voltage 
 * of the LTC1663 DAC. The I²C address used should be one 
 * in the range [0x20..0x27] (depending on device) and NOT 
 * the SYNC address (0x7E).
 *  
 * \param device  A String containing the K-Net name of the 
 *                LTC1663 device.
 * \param command Command Word (see LTC1663 documentation)
 *
 * Command Word Format:
 * - bit 0 SY 
 *    - 1 Allows update on Acknowledge of SYNC Address only
 *    - 0 Update on Stop condition only (Power-On Default) 
 *    .
 * - bit 1 SD
 *    - 1 Puts the device in power-down mode
 *    - 0 Puts the device in standard operating mode (Power-On Default)  
 *    .
 * - bit 2 BG
 *    - 1 Selects the internal badgap reference
 *    - 0 Selects the supply as the reference (Power-On Default)
 *    .
 * - bits 3-7 Not used
 * .
 *
 * \param value  10-bit unsigned value [0..1023]
 * \return an error code:
 *       - <0 error code
 *       - 0  cannot open the K-Net device
 *       - 1  DAC voltage has been changed successfully 
 *
 */
int ltc1663_SendCommand( const char * device ,
			 unsigned char command , 
			 unsigned int value )
{
  knet_dev_t * dev;
  int rc;

  if ((dev = knet_open( device , 
			KNET_BUS_ANY , 0 , NULL )) == NULL )
    return 0;
  
  knet_set_order( dev , KNET_ORDER_LITTLE );

  if ((rc=knet_write16( dev , command , value & ((1<<10)-1) )) < 0 )
    return rc;

  knet_close( dev );

  return 1;
}

		
