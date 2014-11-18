/*--------------------------------------------------------------------
 * kb_error.h - KoreBot Library - Error Handling	
 *--------------------------------------------------------------------
 * $Id: kb_error.h,v 1.13 2006/08/14 14:56:43 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Date: 2006/08/14 14:56:43 $
 * $Revision: 1.13 $
 *--------------------------------------------------------------------*/ 

#ifndef __kb_error__
#define __kb_error__

#ifdef __cplusplus
extern "C" {
#endif


#define KB_ERROR(f,e,...)   kb_error( __FILE__ , __LINE__ , f , e , ##__VA_ARGS__ )

#define KB_FATAL(f,e,...)   kb_fatal( __FILE__ , __LINE__ , f , e , ##__VA_ARGS__ )

#define KB_DEBUG(f,e,m,...) kb_debug( __FILE__ , __LINE__ , f , e , m , ##__VA_ARGS__ )

/*--------------------------------------------------------------------
 * Exit Code Constants
 */
#define KB_EXIT_ON_ERROR     200 /*! Exit on fatal error */

/*--------------------------------------------------------------------
 * Error Constants
 */
#define KB_ERROR_NOMEM          0
#define KB_ERROR_TOOMANYARGS    1
#define KB_ERROR_OUTOFRANGE     2
#define KB_ERROR_INVAL          3
#define KB_ERROR_UNKCMD         4
#define KB_ERROR_INVALID        5
#define KB_ERROR_SYMDEF         6
#define KB_ERROR_UNKDEVCLASS    7
#define KB_ERROR_DEVCLASSUSED   8
#define KB_ERROR_INVNUM         9
#define KB_ERROR_FILEOPEN      10
#define KB_ERROR_NOBUSFOUND    11
#define KB_ERROR_INVORDER      12
#define KB_ERROR_EXCLACCESS    13
#define KB_ERROR_DEVICEINUSE   14
#define KB_ERROR_UNLINKDEV     15
#define KB_ERROR_OPENBUS       16
#define KB_ERROR_SCANBUS       17
#define KB_ERROR_NODEVFOUND    18
#define KB_ERROR_OPENDIR       19
#define KB_ERROR_NODEVFOUNDSTR 20
#define KB_ERROR_SYMNOTDEF     21
#define KB_ERROR_NOSECTION     22
#define KB_ERROR_CMDARGMIN     23
#define KB_ERROR_CMDARGMAX     24
#define KB_ERROR_UNKBUS        25
#define KB_ERROR_NOBUSADDR     26
#define KB_ERROR_NOMODBUS      27
#define KB_ERROR_INVNAME       28  
#define KB_ERROR_NOINIT        29  
#define KB_ERROR_DUPLEX        30  
#define KB_ERROR_FRAGMENT      31  
#define KB_ERROR_FRAGEXP       32  
#define KB_ERROR_IPTR          33  
#define KB_ERROR_OPTR          34  
#define KB_ERROR_PTHREAD       35  
#define KB_ERROR_BADSOUND      36  
#define KB_ERROR_MMAP          37
#define KB_ERROR_OPENLRF       38
#define KB_ERROR_LRFDATA2BIG   39    
#define KB_ERROR_LRFNOSTX      40
#define KB_ERROR_LRFNOETX      41 
#define KB_ERROR_LRFWRONGSTX   42
#define KB_ERROR_LRFWRONGETX   43
#define KB_ERROR_LRFNODATA     44 
#define KB_ERROR_LRFWRONGCRC   45 
#define KB_ERROR_LRFSENDDATA   46
#define KB_ERROR_LRFNOCERT     47
#define KB_ERROR_LRFCMDTX      48
#define KB_ERROR_LRFDISTRX     49
#define KB_ERROR_LRFDATARX     50
#define KB_ERROR_KH3SZFMTERR   51
#define KB_ERROR_KH3FRMSZERR   52
#define KB_ERROR_KH3FRMSNDERR  53
#define KB_ERROR_KH3KBINIT     54
 /*--------------------------------------------------------------------
 * Warning Constants
 */
#define KB_WARN_CONFIG_FILE 0
#define KB_WARN_OPENDIR     1
#define KB_WARN_CONNECT     2
#define KB_WARN_SOUNDIO     3
#define KB_WARN_SOUNDSAMPLE 4

/*--------------------------------------------------------------------
 * Helper Macros
 */

/*--------------------------------------------------------------------
 * Public Type Declarations 
 */

/*! Error Handle Type Definition */
typedef void (*kb_error_handler_t)( const char * ,
				    unsigned int ,
				    const char * ,
				    unsigned int ,
				    va_list );

/*! Warning Handle Type Definition */
typedef void (*kb_warning_handler_t)( unsigned int , va_list );
/*--------------------------------------------------------------------
 * Public Prototye Declarations
 */

extern void kb_set_debug_level( unsigned int level );

extern void kb_set_error_handler( kb_error_handler_t handler );

extern void kb_vmsg( const char * msg , va_list argptr );

extern void kb_msg( const char * msg , ... );

extern int kb_vdebug( const char * file ,
		      unsigned int line ,
		      const char * func ,
		      unsigned int level ,
		      const char * msg , 
		      va_list argptr );
     
extern int kb_debug( const char * file ,
		     unsigned int line ,
		     const char * func ,
		     unsigned int level ,
		     const char * msg , 
		     ... );

extern int  kb_verror( const char * file ,
		       unsigned int line ,
		       const char * func ,
		       unsigned int error , va_list argptr );

extern int  kb_error( const char * file ,
		      unsigned int line ,
		      const char * func ,
		      unsigned int error , ... );

extern int  kb_vwarning( unsigned int error , va_list argptr );

extern int  kb_warning( unsigned int error , ... );

extern void kb_fatal( const char * file ,
		      unsigned int line ,
		      const char * func ,
		      unsigned int error ,
		      ... );

extern const char * kb_errmsg [];
extern const char * kb_warnmsg [];

extern unsigned int kb_debug_level;

#ifdef __cplusplus
}
#endif

#endif /* __kb_error__ */
