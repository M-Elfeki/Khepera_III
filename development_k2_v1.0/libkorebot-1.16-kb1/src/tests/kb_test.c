/*--------------------------------------------------------------------
 * kb_test.c - Config Test
 *--------------------------------------------------------------------
 * $Id: kb_test.c,v 1.1 2004/07/29 10:57:20 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Date: 2004/07/29 10:57:20 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#include <korebot/korebot.h>

/* main */

int main( int argc , char * argv[] )
{
  int rc;
  kb_device_config_t * dc;

  kb_set_debug_level(1);

  if ((rc = kb_config_init( argc , argv )) < 0 ) 
    return 1;
  
  dc = kb_lookup_device( argv[1] );
  if ( dc != NULL ){
    kb_msg( "OK\n");
  }
  else {

    kb_msg("ER\n");
  }
  kb_config_exit();
	
  return 0;
}
