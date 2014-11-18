/*--------------------------------------------------------------------
 * korebot.h - KoreBot Library	
 *--------------------------------------------------------------------
 * $Id: korebot.h,v 1.12 2006/10/27 08:40:58 flambercy Exp $
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Date: 2006/10/27 08:40:58 $
 * $Revision: 1.12 $
 *--------------------------------------------------------------------*/ 

#ifndef __korebot__
#define __korebot__

#define KB_VERSION  1
#define KB_REVISION 15

/*!
 * Define if KoreMotor is programmed with firmware 6.0 or newer
 */
#define NEW_KMOT_FIRMWARE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <termio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <getopt.h>

#include "kb_init.h"
#include "kb_error.h"
#include "kb_memory.h"
#include "kb_time.h"
#include "kb_cmdparser.h"
#include "kb_symbol.h"
#include "kb_config.h"
#include "kb_socket.h"
#include "kb_sound.h"
#include "kb_wav.h"
#include "kb_gpio.h"
#include "kb_pwm.h"
#include "kb_lrf.h"
#include "kb_khepera3.h"
#include "kb_gripper.h"

#include "knet.h"

#include "kbot.h"
#include "kmot.h"
#include "hem.h"
#include "koala.h"
#include "koreio.h"

#include "ltc1663.h"

#endif /* __korebot__ */
