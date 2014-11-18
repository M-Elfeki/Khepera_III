/*--------------------------------------------------------------------
 * kb_symbol.h - KoreBot Library - Symbol Management
 *--------------------------------------------------------------------
 * $Id: kb_symbol.h,v 1.3 2004/09/22 08:29:14 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Revision: 1.3 $
 * $Date: 2004/09/22 08:29:14 $
 *--------------------------------------------------------------------*/

#ifndef __kb_symbol__
#define __kb_symbol__

/*--------------------------------------------------------------------
 * Symbol Constants Definition
 */
#define KB_SYMBOL_TYPE_NONE       -1 /* Entry not used */
#define KB_SYMBOL_TYPE_SECTION     0 /* Section  */
#define KB_SYMBOL_TYPE_ALIAS       1 /* Alias    */
#define KB_SYMBOL_TYPE_DEVICE      2 /* Device   */
#define KB_SYMBOL_TYPE_REGISTER    3 /* Register */
#define KB_SYMBOL_NAME_SIZE      128 /* Maximal length of a symbol name */
#define KB_SYMBOL_TABLE_SIZE     141

/*--------------------------------------------------------------------
 * Symbol Type Definition
 */
typedef struct kb_symbol_s 
{
  /* Symbol Name */
  char   name[ KB_SYMBOL_NAME_SIZE ];
  
  /* Symbol Type */
  int    type;

  int    alloc;

  /*!
   *  Symbol Value: can be used to store a single integer value
   *  or usually a pointer to a struct describing the 
   *  corresponding object
   */
  unsigned int value;

  /* Next Symbol in the list */
  struct kb_symbol_s * next;
}
kb_symbol_t;

/*--------------------------------------------------------------------
 * Symbol Table Definition
 */
typedef struct kb_table_s
{
  pthread_mutex_t lock;
  kb_symbol_t     *symbols[ KB_SYMBOL_TABLE_SIZE ];
  unsigned int    count;
}
kb_symbol_table_t;

/*--------------------------------------------------------------------
 * Public Prototype Declarations 
 */

extern kb_symbol_table_t * kb_create_symbol_table( kb_symbol_table_t * table ); 

extern void kb_destroy_symbol_table( kb_symbol_table_t * table );

extern unsigned long kb_hash( const char * name );

extern int kb_add_symbol( kb_symbol_table_t * table , 
			  kb_symbol_t * knew );

extern kb_symbol_t * kb_lookup_symbol( kb_symbol_table_t * table ,
				       const char * name );

extern void kb_dump_symbol_table( kb_symbol_table_t * table );

extern const char * kb_symbol_type_names[];

#endif /* __kb_symbol__ */
