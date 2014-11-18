/*--------------------------------------------------------------------
 * kbot.c - KoreBot Library - KoreBot Support
 *--------------------------------------------------------------------
 * $Id: kbot.c,v 1.3 2004/09/22 19:22:21 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Date: 2004/09/22 19:22:21 $
 * $Revision: 1.3 $
 *-------------------------------------------------------------------
 * $Log: kbot.c,v $
 * Revision 1.3  2004/09/22 19:22:21  pbureau
 * Minor compilation fixes, kb_debug_mask & kb_debug_level inconsistency
 *
 * Revision 1.2  2004/09/07 17:54:07  cgaudin
 * Added support for PCF8574 for Philips, MUST BE TESTED !
 *
 * Revision 1.1  2004/07/29 10:51:55  cgaudin
 * New libkorebot release 1.2
 *
 *-------------------------------------------------------------------*/ 

/*! \file kbot.c This module contains all support for the KoreBot 
 * board 
 */


/*! 
 * \file   kbot.c Support for the KoreBot board             
 *
 * \brief 
 *         This module contains all support for the KoreBot board
 *        
 * \author   Cédric Gaudin (K-Team SA)                               
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */
#include "korebot.h"

/*--------------------------------------------------------------------*/
/*! 
 * This function changes the VCore voltage of the processor.
 *                   
 * \param voltage required \f$V_{core}\f$ voltage in [mV] 
 *        (range 600-1400)
 * \returns An error code:
 *                   - <0 on error
 *                   - 0 on success
 */

int kbot_SetProcVCore( unsigned int voltage ) 
{
  int vdac;
  
  if ( voltage < 600 || voltage > 1400 ) {
    return KB_ERROR( "kb_SetProcVCode" , 
		     KB_ERROR_OUTOFRANGE ,
		     voltage , "[mV]" , 600 , 1400 );
  }

  /*! 
   * Formula used to calculate the Vdac voltage:
   *
   * \f$V_{dac} = V_{fb} * (1 + \frac{R1}{R2} + \frac{R1}{R3}) - 
   * \frac{R1}{R2} * V_{core}\f$
   * 
   * \f$V_{fb} = 700[mV]\f$, \f$R1 = 4.7[k\Omega]\f$, 
   * \f$R2 = 1.5[k\Omega]\f$, \f$R3 = 2.2[k\Omega]\f$  
   */
  vdac = 4389 - (47 * (int)voltage) / 15;
  
  return ltc1663_SendCommand( "KoreBot:XScaleVCoreDAC" , 
		  	      LTC1663_BG , 
			      (unsigned int ) ((vdac * 10) / 33) );  
}

/*--------------------------------------------------------------------*/
/*! This function changes the leds connected to I²C on the KoreSDK.
 */
int kbot_SetLeds( unsigned char value )
{
  knet_dev_t * dev;
  int rc=0;

  if (( dev = knet_open( "KoreSDK:Leds" , 
			 KNET_BUS_ANY , 0 , NULL )) == NULL )
    return rc;

  //if (( rc = pcf8574( dev , value )) < 0 )
  //  return rc;

  knet_close( dev );

  return rc;
}
