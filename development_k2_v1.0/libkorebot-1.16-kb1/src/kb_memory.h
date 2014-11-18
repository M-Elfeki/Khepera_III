/*--------------------------------------------------------------------
 * kb_memory.h - KoreBot Library - Memory Stubs	
 *--------------------------------------------------------------------
 * $Id: kb_memory.h,v 1.4 2005/01/26 08:45:28 amaye Exp $
 *--------------------------------------------------------------------
 * $Author: amaye $
 * $Date: 2005/01/26 08:45:28 $
 * $Revision: 1.4 $
 *--------------------------------------------------------------------*/ 

#ifndef __kb_memory__
#define __kb_memory__

/*--------------------------------------------------------------------
 * Macros
 */

/*! Memory Allocator Helpers */ 
#define KB_ALLOC( type , count ) (type *)kb_alloc(sizeof(type)*(count))
#define KB_REALLOC( ptr, type , count ) (type *)kb_realloc((void*)ptr, sizeof(type)*(count))
#define kb_free(ptr)             free(ptr)

/*--------------------------------------------------------------------
 * Public Prototype Declarations
 */
extern void * kb_alloc( unsigned long size );
extern void * kb_realloc( void * ptr , unsigned long size );

#endif /* __kb_memory__ */

