/*--------------------------------------------------------------------
 * kb_cmdparser.h - KoreBot Library - Command Parser
 *--------------------------------------------------------------------
 * $Id: kb_cmdparser.h,v 1.4 2005/01/31 07:34:28 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Revision: 1.4 $
 * $Date: 2005/01/31 07:34:28 $
 *--------------------------------------------------------------------*/

#ifndef __kb_cmdparser__
#define __kb_cmdparser__

/*--------------------------------------------------------------------*/
/*! Command Definition
 */
typedef struct kb_command_s 
{
  /*! Command Name */
  char * name;

  /*! Minimum number of parameters */
  int    minParam;

  /*! Maximal number of parameters */
  int    maxParam;

  /*! Function able to parse the argument of this command */
  int    (*parse)( int argc , char *argv[] , void *data);
}
kb_command_t;

/*--------------------------------------------------------------------
 * Public Prototype Declarations
 */
extern int kb_parse_command( char * line , kb_command_t * cmds , void * data); 
extern char* kb_get_arg(char *,unsigned);
extern int kb_find_string( const char * str , const char * list[] );
extern int kb_find_command( const char * cmd_name, const kb_command_t * cmd_list);

#endif /* __kb_cmdparser__ */

