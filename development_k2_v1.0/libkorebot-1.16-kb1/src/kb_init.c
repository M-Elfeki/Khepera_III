/*--------------------------------------------------------------------
 * kb_init.c - KoreBot Library -  Initialization
 *--------------------------------------------------------------------
 * $Id: kb_init.c,v 1.2 2004/09/22 08:29:14 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Revision: 1.2 $
 * $Date: 2004/09/22 08:29:14 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

static int kb_init_done = 0;

struct option opts[] = {
  { "kb-debug" ,   1 , 0 , 'd' } ,
  { "kb-version" , 1 , 0 , 'v' } ,
  { NULL      ,    0 , 0 , 0    }
};

/*--------------------------------------------------------------------*/
/*! This function initializes the KoreBot Library
 *
 */
int kb_init( int argc , char * argv[] )
{
  int opt, rc;

  if ( kb_init_done == 0 ) {

    for (;;) {
      opt = getopt_long( argc , argv , "" , opts , NULL );
      if ( opt == -1 )
	break;
      
      switch( opt ) {
	/* --kb-debug <level> */
      case 'd':
	kb_set_debug_level(atoi(optarg));
	break;

	/* --kb-version */
      case 'v':
	kb_msg( "libkorebot v%u.%u [%s]" , 
		KB_VERSION , KB_REVISION , __DATE__ );
	break;

      default:
	break;
      }
    }
      
    if ((rc = kb_config_init( argc, argv )) < 0 )
      return rc;
    
    if ((rc = knet_init( argc , argv )) <  0)
      return rc;
    
    atexit( kb_exit );
    
    kb_init_done = 1;
  }

  return 1;
}

/*--------------------------------------------------------------------*/
/*! This function is called automatically on exit and call the 'exit'
 *  of all sub layer. 
 *
 * \remark This function is called automatically at the terminaison 
 *         of the application.
 */
void kb_exit(void)
{
  if ( kb_init_done ) {
    knet_exit();
    kb_config_exit();
    kb_init_done = 0;
  }
}
