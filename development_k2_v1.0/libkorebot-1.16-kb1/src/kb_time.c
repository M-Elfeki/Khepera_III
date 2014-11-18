/*--------------------------------------------------------------------
 * kb_time.c - KoreBot Library - Basic Time Function
 *--------------------------------------------------------------------
 * $Id: kb_time.c,v 1.1 2004/07/29 10:51:55 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Revision: 1.1 $
 * $Date: 2004/07/29 10:51:55 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*! 
 * \file   kb_time.c Basic Time Function             
 *
 * \brief 
 *         This module provides a basic time function.
 *        
 * \author   Cédric Gaudin (K-Team SA)                                                           
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */


/*--------------------------------------------------------------------*/
/*! Reference time in milliseconds (initialized at the 
 *  application startup) 
 */
static u_int32_t ref_time = 0;

/*--------------------------------------------------------------------*/
/*! 
 * This function gives a timestamp with a resolution of one 
 * millisecond that warp to zero 0 in about 49 days.
 *
 * \return the number of milliseconds since the start of the application 
 */
kb_time_t kb_getTime(void)
{
  unsigned long val;
  struct timeval tv;

  gettimeofday( &tv , NULL );

  /* set the reference time */
  if ( ref_time == 0 ) {
    ref_time = (( tv.tv_sec * 1000 ) + ( tv.tv_usec / 1000 ));
  }

  val  = ( tv.tv_sec * 1000 ) + ( tv.tv_usec / 1000 );
  val -= ref_time;
 

  return val;
}

