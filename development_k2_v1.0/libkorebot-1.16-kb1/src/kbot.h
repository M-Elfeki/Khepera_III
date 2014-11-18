/*--------------------------------------------------------------------
 * kbot.c - KoreBot Library - KoreBot Support
 *--------------------------------------------------------------------
 * $Id: kbot.h,v 1.2 2004/09/07 17:54:07 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Date: 2004/09/07 17:54:07 $
 * $Revision: 1.2 $
 *-------------------------------------------------------------------
 * $Log: kbot.h,v $
 * Revision 1.2  2004/09/07 17:54:07  cgaudin
 * Added support for PCF8574 for Philips, MUST BE TESTED !
 *
 * Revision 1.1  2004/07/29 10:51:55  cgaudin
 * New libkorebot release 1.2
 *
 *-------------------------------------------------------------------*/ 

#ifndef __kbot__
#define __kbot__

/*--------------------------------------------------------------------
 * KoreBot Prototype Function Declarations 
 */
extern int kbot_SetProcVCore( unsigned int voltage );

extern int kbot_SetLeds( unsigned char value );

#endif /* __kbot__ */
