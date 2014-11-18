/*--------------------------------------------------------------------
 * knet_rs232.h - RS-232 Communication Layer
 *--------------------------------------------------------------------
 * $Id: knet_rs232.h,v 1.1 2004/07/29 10:51:56 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Revision: 1.1 $
 * $Date: 2004/07/29 10:51:56 $
 *--------------------------------------------------------------------*/

#ifndef __knet_rs232__
#define __knet_rs232__

typedef struct knet_rs232_s {
  
  /*! RS-232 device file descriptor */
  int fd;

  /*! */
  struct termios tios;

}
knet_rs232_t;


extern int knet_rs232_init( knet_bus_t * bus , int argc , char * argv [] );


#endif /* __knet_rs232__ */
