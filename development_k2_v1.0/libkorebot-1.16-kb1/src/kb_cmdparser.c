/*--------------------------------------------------------------------
 * kb_cmdparser.c - KoreBot Library - Command Parser
 *--------------------------------------------------------------------
 * $Id: kb_cmdparser.c,v 1.7 2005/01/31 07:34:28 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Revision: 1.7 $
 * $Date: 2005/01/31 07:34:28 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*! 
 * \file   kb_cmdparser.c Command Parser
 *
 * \brief
 *         This module provides basic command parsing facilities.
 *
 * \author   Cédric Gaudin (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.
 * \todo     nothing.
 */


/*! Maximal number of arguments */
#define ARGV_SIZE  128

/*--------------------------------------------------------------------*/
/*! 
 * return the given argument from the given command string.
 * \param cmd the command string
 * \param arg the argument to find
 * \return the argument or NULL
 */
char * kb_get_arg(char * cmd, unsigned arg)
{
  char * argv[ ARGV_SIZE ];
  char * t, *cpy;
  int argc;

  if(argc >= ARGV_SIZE)
    return NULL;

  cpy = strdup(cmd);

  /* tokenize */
  argc=0;
  for (t=strtok(cpy," \t");t!=NULL;t=strtok(NULL," \t")) {
    if ( argc < ARGV_SIZE ) {
      argv[argc] = t;
      argc++;
    }
    else {
      kb_free(cpy);
      //return KB_ERROR( "kb_parse_command" , KB_ERROR_TOOMANYARGS );	  
      return NULL;
    }
  }
  kb_free(cpy);
  return argv[arg];
}

/*--------------------------------------------------------------------*/
/*! 
 * This function parses a given command line and executes commands 
 * found in a given command descriptor table.
 *
 * \param line  Pointer to the command line.
 * \param cmds  Pointer to an array of command descriptor
 * \param data  Pointer to additional data that the application may
 *              need to pass to the command
 * \return an error code:
 *    - <0 on error
 *    - 0 on success
 *
 * \remark All comparaison are case insensitive.
 */
int kb_parse_command( char * line , kb_command_t * cmds, void * data) 
{
  int argc, ok, pos, rc;
  char * argv[ ARGV_SIZE ];
  char * t;

  if ( line == NULL || cmds == NULL ) {
    return KB_ERROR( "kb_parse_command" ,
		     KB_ERROR_INVAL );
  }

  /* tokenize */
  argc=0;
  for (t=strtok(line," \t");t!=NULL;t=strtok(NULL," \t")) {
    
    if ( argc < ARGV_SIZE ) {
      argv[argc] = t;
      argc++;
    }
    else {
      return KB_ERROR( "kb_parse_command" ,
		       KB_ERROR_TOOMANYARGS );	  
    }
    
  }
    
  /* parse */
  if ( argc > 0 ) {
    
    ok = 0;
    for (pos=0; cmds[pos].name != NULL; pos++) {
      
      if (!strcasecmp(cmds[pos].name,argv[0])) {

	/* Check the min number of args */
	if ( cmds[pos].minParam != -1 && 
	     argc -1 < cmds[pos].minParam ) {
	  return kb_error( __FILE__ ,
			   __LINE__ ,
			   "kb_parse_command" ,
			   KB_ERROR_CMDARGMIN ,
			   cmds[pos].name,
			   cmds[pos].minParam);
	}

	/* check for maximal number of arguments */
	if ( cmds[pos].maxParam != -1 &&
	     argc -1 > cmds[pos].maxParam ) {
	  return kb_error( __FILE__ ,
			   __LINE__ ,
			   "kb_parse_command" ,
			   KB_ERROR_CMDARGMAX, 
			   cmds[pos].name,
			   cmds[pos].maxParam);
	}

	/* Call the command function */
	if (cmds[pos].parse && (rc = cmds[pos].parse( argc , argv , data)) < 0 )
	  return rc;

	ok=1;
	break;
      }
    }
    
    if (ok==0) {
      return KB_ERROR( "kb_parse_command" ,
		       KB_ERROR_UNKCMD ,
		       argv[0] );
    }
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*!
 * This function finds a command in an array of command_s
 * 
 * \param cmd_name Pointer to the command name.
 * \param cmd_list Pointer to the array of commands.
 * \return A value:
 *   - >= 0 index of the command in the array
 *   - KB_ERROR_UNKCMD if the command is unknown        
 *
 * \remark The comparaison is case insensitive.
 */
int kb_find_command( const char * cmd_name, const kb_command_t * cmd_list)
{
  int index;
 
  for (index=0; cmd_list[index].name != NULL ; index++ ) {
    if (!strcasecmp( cmd_name, cmd_list[index].name ))
      return index;
  }

  KB_ERROR("kb_find_command", KB_ERROR_UNKCMD, cmd_name);
  return -KB_ERROR_UNKCMD;
}

/*--------------------------------------------------------------------*/
/*!
 * This function finds a string in an array of strings
 * 
 * \param str  Pointer to the string to search in the array.
 * \param list Pointer to the array of strings.
 * \return A value:
 *   - >0 index of the string in the array
 *   - -1 no matching string found
 *
 * \remark The comparaison is case insensitive.
 */
int kb_find_string( const char * str , const char **list )
{
  int index;
 
  for (index=0; list[index] != NULL ; index++ ) {
    if (!strcasecmp( str , list[index] ))
      return index;
  }
  return -1;
}

