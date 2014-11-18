/*--------------------------------------------------------------------
 * kb_time.h - KoreBot Library - Basic Time Function
 *--------------------------------------------------------------------
 * $Id: kb_time.h,v 1.1 2004/07/29 10:51:55 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Revision: 1.1 $
 * $Date: 2004/07/29 10:51:55 $
 *--------------------------------------------------------------------*/

#ifndef __kb_time__
#define __kb_time__

/*-------------------------------------------------------------------*/
/*! Time Stamp Definition
 */
typedef u_int32_t kb_time_t;

/*--------------------------------------------------------------------
 * kb_getTime() - Number of seconds and usec from the beginning of the 
 *                day 
 */
extern kb_time_t kb_getTime(void);

#endif /* __kb_time__ */
