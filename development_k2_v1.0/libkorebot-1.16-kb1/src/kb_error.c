/*--------------------------------------------------------------------
 * kb_error.h - KoreBot Library - Error Handling	
 *--------------------------------------------------------------------
 * $Id: kb_error.c,v 1.14 2005/11/16 13:15:03 amaye Exp $
 *--------------------------------------------------------------------
 * $Author: amaye $
 * $Date: 2005/11/16 13:15:03 $
 * $Revision: 1.14 $
 *--------------------------------------------------------------------*/ 

#include "korebot.h"

/*! 
 * \file   kb_error.c Error Handling
 *
 * \brief 
 *         This module provides basic error and message handling.
 *        
 * \author   Cédric Gaudin (K-Team SA)                                
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

/*--------------------------------------------------------------------*/
/*! Error Messages   
 * Error codes are defined in kb_error.h and are always 
 * returned as negative integers. 
 */
const char *kb_errmsg[] = {
  "no enough memory" ,
  "too many argument" , 
  "value %u %s out of range [ %u ; %u ]" , 
  "invalid value" ,
  "unknown command '%s'" ,
  "invalid identifier '%s'" ,
  "symbol '%s' already defined" ,
  "unknown device class '%s'" ,
  "device '%s' of class '%s' already defined" ,
  "invalid number '%s'" , 
  "unable to open file '%s'" ,
  "no bus found for device '%s'" ,
  "invalid order flags setting" ,
  "unable to gain exclusive access" ,
  "device in use" ,
  "unable to unlink device from bus devices" ,
  "unable to open '%s' bus" ,
  "unable to scan '%s' bus" ,
  "no device found with (%u,%u)" ,
  "unable to open directory '%s'" ,
  "no device found with (%s)" ,
  "symbol '%s' not defined" ,
  "no section defined ! %s '%s' must be defined in a section" ,
  "command '%s' requires a minimum of %d arguments" ,
  "command '%s' accepts a maximum of %d arguments" ,
  "unknow bus name '%s'",
  "no module bus address defined",
  "no module bus specified",
  "invalid device name '%s'",
  "module '%s' not initialized",
  "Unable to enable DUPLEX",
  "Unable to set fragment",
  "Bad FragExponent (%d!=%d)",
  "Unable to get IPTR",
  "Unable to get OPTR",
  "Cannot create pthread (%s)",
  "Bad sound file format (%s)",
  "Unable to map memory at 0x%lx",
  "Cannot open serial connexion to the LRF card as %s",
  "Too much data was about to be received, please update constant in sources : Nbr %d",
  "Timeout waiting for STX byte",
  "Timeout waiting for ETX byte",
  "Wrong STX has been received : 0x%2.2X",
  "Wrong ETX has been received : 0x%2.2X",
  "Timeout waiting for Data",
  "The CRC computed doesn't match expected!",
  "Problem sending data to the LRF",
  "Problem receiving certification code from LRF",
  "Error while sending command to LRF",
  "Error while receiving distances from LRF",
  "Error while receiving datas from LRF",
  "Error while receiving command from Khepera3 - Unexpected frame size format",
  "Error while receiving command from Khepera3 - Frame size Real(%d)!=Expected(%d)",	  
  "Error while sending command to Khepera3",
  "Unable to initialize the korebot library",
  NULL
};

/*--------------------------------------------------------------------*/
/*! Warning Messages   
 * Warning codes are defined in kb_error.h and are always 
 * returned as positive intergers.
 */
const char *kb_warnmsg[] = {
  "no configuration file available in %s" ,
  "unable to open directory '%s'" ,
  "Handling client %s",
  "i/o error in reading file '%s' !",
  "error in playing sample errno=%d",
  NULL
};

static kb_error_handler_t kb_user_error_handler = NULL;
static kb_warning_handler_t kb_user_warning_handler = NULL;
unsigned int              kb_debug_level = 0;
unsigned int              kb_debug_mask  = 0;

/*--------------------------------------------------------------------*/
/*! 
 * This function changes the debug mask 
 *
 * \param level New debug mask 
 */
void kb_set_debug_mask( unsigned int mask)
{
  kb_debug_mask = mask;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function changes the debug level
 *
 * \param level New debug level
 */
void kb_set_debug_level( unsigned int level)
{
  kb_debug_level = level;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function sets a user error handler
 *
 * \param handler Pointer to the user error handler  
 */
void kb_set_error_handler( kb_error_handler_t handler )
{
  kb_user_error_handler = handler;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displays a message on standard output
 *
 * \param msg    message
 * \param argptr list of arguments  
 */
void kb_vmsg( const char * msg , va_list argptr )
{
  vfprintf( stdout , msg , argptr );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displays a message on standard output
 *
 * \param msg message
 * \param ... list of arguments
 */
void kb_msg( const char * msg , ... )
{
  va_list argptr;

  va_start( argptr , msg );
  kb_vmsg( msg , argptr );
  va_end( argptr );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displays a debug message on standard output.
 *
 * \param file caller's source file (usually __FILE__)
 * \param line caller's source line (usually __LINE__)
 * \param func caller's name
 * \param level debug level
 * \param msg debug message
 * \param argptr list of arguments 
 * \return -error
 * 
 * \remark You can call this function from your user error handler.
 */
int kb_vdebug( const char * file ,
	       unsigned int line ,
	       const char * func ,
	       unsigned int mask ,
	       const char * msg ,
	       va_list argptr )
{
  if ( (mask & kb_debug_mask ) != 0  ) {

    fprintf( stdout , "Debug> %s:%u:%s " , file , line , func );
    vfprintf( stdout , msg , argptr );
    fputc( '\n' , stdout );
  }
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displays a debug message on standard output.
 *
 * \param file caller's source file (usually __FILE__)
 * \param line caller's source line (usually __LINE__)
 * \param func caller's name
 * \param mask debug mask 
 * \param msg debug message
 * \param ... list of arguments 
 * \return -error
 *
 * \remark You can call this function from your user error handler.
 */
int kb_debug( const char * file ,
	      unsigned int line ,
	      const char * func ,
	      unsigned int mask ,
	      const char * msg , 
	      ... )
{
  va_list argptr;

  va_start( argptr , msg );
  kb_vdebug( file , line , func , mask , msg , argptr );
  va_end( argptr );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displays an error message on standard error.
 * 
 *
 * \param file caller's source file (usually __FILE__)
 * \param line caller's source line (usually __LINE__)
 * \param func caller's name
 * \param error number of the error
 * \param argptr list of arguments 
 * \return -error
 *
 * \remark You can call this function from your user error handler.
 */
int kb_verror( const char * file ,
	       unsigned int line ,
	       const char * func  ,
	       unsigned int error , 
	       va_list argptr )
{
  static int in_error = 0;
 
  /* call user error handler ? */ 
  if ( kb_user_error_handler != NULL && in_error == 0 ) {

    in_error = 1;
    (*kb_user_error_handler)( file , line , func , error , argptr );
    in_error = 0;

  } 
  else {
    if ( kb_debug_level != 0 ) { 
      fprintf( stderr , "Error: %s:%u:%s " , file , line , func );
    }

    vfprintf( stderr , kb_errmsg[error] , argptr );
    fputc( '\n' , stderr );
  }

  return -error;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displays an error message on standard error
 *
 * \param file caller's source file (usually __FILE__)
 * \param line caller's source line (usually __LINE__)
 * \param func caller's name
 * \param error number of the error
 * \param ... list of arguments 
 * \return -error
 */
int kb_error( const char * file ,
	      unsigned int line ,
	      const char * func ,
	      unsigned int error , ... )
{
  int rc;
  va_list argptr;
 
  va_start( argptr , error );
  rc = kb_verror( file , line , func , error , argptr );
  va_end( argptr );
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displays a warning message on standard error.
 *
 * \param error number of the error
 * \param argptr list of arguments 
 * \return -error
 *
 * \remark You can call this function from your user error handler.
 */
int kb_vwarning( unsigned int error , va_list argptr )
{
  static int in_error = 0;
  
  if ( kb_user_warning_handler != NULL && !in_error ) {

    in_error = 1;
    (*kb_user_warning_handler)( error , argptr );
    in_error = 0;

  } 
  else {
    if ( kb_debug_level > 1 ) { 
      fprintf( stderr , "Warning: " );
    }

    vfprintf( stderr , kb_warnmsg[error] , argptr );
    fputc( '\n' , stderr );
  }

  return error;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displays a warning message on standard error
 *
 * \param error number of the error
 * \param ... list of arguments 
 * \return -error
 */
int kb_warning(unsigned int error , ... )
{
  int rc;
  va_list argptr;
 
  va_start( argptr , error );
  rc = kb_vwarning( error , argptr );
  va_end( argptr );
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function displaya an error message on standard error and 
 * exit the application.
 *
 * \param file caller's source file (usually __FILE__)
 * \param line caller's source line (usually __LINE__)
 * \param func caller's name
 * \param error number of the error
 * \param ... list of arguments 
 */
void kb_fatal( const char * file ,
	       unsigned int line ,
	       const char * func ,
	       unsigned int error , ... )
{
  va_list argptr;

  va_start( argptr , error );
  kb_verror( file , line , func , error , argptr );
  va_end( argptr );
  
  exit( KB_EXIT_ON_ERROR );
}

