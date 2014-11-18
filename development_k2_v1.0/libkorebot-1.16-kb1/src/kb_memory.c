/*--------------------------------------------------------------------
 * kb_memory.c - KoreBot Library - Memory Stubs	
 *--------------------------------------------------------------------
 * $Id: kb_memory.c,v 1.3 2005/01/26 08:45:28 amaye Exp $
 *--------------------------------------------------------------------
 * $Author: amaye $
 * $Date: 2005/01/26 08:45:28 $
 * $Revision: 1.3 $
 *--------------------------------------------------------------------*/ 

#include "korebot.h"

/*--------------------------------------------------------------------*/
/*! 
 * This function allocates safely memory. Produces an fatal error 
 * when no more memory is available.
 *
 * \param  size  size in bytes
 * \return pointer to memory allocated 
 */
void * kb_alloc( unsigned long size )
{
  void * blk = malloc(size);

  if (blk == NULL) {
    KB_FATAL( "kb_alloc" , KB_ERROR_NOMEM );
  }
  return blk;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function reallocates safely memory. Produces an fatal error
 * when no more memory is available
 *
 * \param ptr   Pointer to the block of memory to resize
 * \param size  New size of the memory block
 * \return a pointer on the new memory block  
 */
void * kb_realloc( void * ptr , unsigned long size )
{
  void * blk = realloc( ptr , size );

  if (blk == NULL) {
    KB_FATAL( "kb_alloc" , KB_ERROR_NOMEM );
  }
  return blk;
}

