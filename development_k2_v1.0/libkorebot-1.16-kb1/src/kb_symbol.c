/*--------------------------------------------------------------------
 * kb_symbol.c - KoreBot Library - Symbol Management
 *--------------------------------------------------------------------
 * $Id: kb_symbol.c,v 1.4 2004/09/22 08:29:14 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Revision: 1.4 $
 * $Date: 2004/09/22 08:29:14 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*! 
 * \file   kb_symbol.c Symbol Table Management
 *
 * \brief 
 *         This module provides primitive to handle symbol table.
 *
 * \author   Cédric Gaudin (K-Team SA)
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 *
 * \bug      none discovered
 *
 * \todo     nothing.
 */

/*--------------------------------------------------------------------*/
/*! Symbol Type Names
 */ 
const char *kb_symbol_type_names[] = {
  "section" , "alias" , "device" , "register" , NULL
};

/*--------------------------------------------------------------------*/
/*! 
 * This function initializes a given table of symbol.
 *
 * \param table A pointer to a symbol table or NULL.
 * \return A pointer to the initialized symbol table
 */
kb_symbol_table_t * kb_create_symbol_table( kb_symbol_table_t * table )
{
  unsigned int index;

  if ( table == NULL ) 
    table = KB_ALLOC(kb_symbol_table_t,1);
 
  (void)pthread_mutex_init( &table->lock , 0 );
  table->count = 0;

  for (index=0; index<KB_SYMBOL_TABLE_SIZE; index++) {
    table->symbols[index] = NULL;
  }
  
  return table;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function destroys all elements in a given symbol table. 
 *
 * \param table A pointer to the given symbol table
 *
 * \remark table is never freed.
 */
void kb_destroy_symbol_table( kb_symbol_table_t * table )
{
  unsigned int index;
  kb_symbol_t * sym , *next;

  (void)pthread_mutex_destroy( &table->lock );
  table->count = 0;
 
  for (index=0; index<KB_SYMBOL_TABLE_SIZE; index++) {
  
    sym = table->symbols[index];
    
    while ( sym != NULL ) {
      //printf("toto\n");
      //printf("sym=%08X\n" , sym );
      //printf("sym->name=%s\n"  , sym->name );
      //printf("sym->type=%u\n" , sym->type );
      //printf("sym->alloc=%u\n" , sym->alloc );
      //printf("sym->value=%u\n" , sym->value );
      //printf("sym->next=%08X\n" , sym->next );

      next = sym->next;
      if ( sym->alloc != 0 ) {
	kb_free( (void *)sym->value );
      }
      kb_free( sym );
      sym = next;
    }
  }
}

/*--------------------------------------------------------------------*/
/*! 
 * This function calculates the hash value of a key. 
 *
 * \param name  A string containing the key to hash
 * \return a hash value
 *
 * \remark The hash function is case insensitive.
 */
unsigned long kb_hash( const char * name )
{
  unsigned long h = 0;

  while ( *name != '\0' ) {
    h = ( h << 2 ) + ( tolower(*name) );
    name++;
  }
  return h;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function adds a given symbol into a given table.
 * 
 * \param table  A symbol table
 * \param new    A new symbol
 * \return A value:
 *        - 0 symbol already defined
 *        - 1 new symbol added  
 */
int kb_add_symbol( kb_symbol_table_t * table , kb_symbol_t * knew ) 
{
  unsigned int index;
  kb_symbol_t * sym;

  knew->next = NULL;

  index = ( kb_hash( knew->name ) % KB_SYMBOL_TABLE_SIZE );

  sym = table->symbols[index];

  while ( sym != NULL ) {
    if ( !strcasecmp( sym->name , knew->name ) ) 
      return 0;
    sym = sym->next;
  }

  pthread_mutex_lock( &table->lock );
  knew->next = table->symbols[index];
  
  table->symbols[index] = knew;
  table->count++;
  pthread_mutex_unlock( &table->lock );

  return 1;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function find a symbol into a given table.
 *
 * \param table A symbol table
 * \param name  A string containing the name of the symbol to find.
 * 
 * \return A Pointer to the symbol found or 
 *         NULL if the symbol is not defined. 
 */
kb_symbol_t * kb_lookup_symbol( kb_symbol_table_t * table , 
				const char * name )
{
  unsigned int index;
  kb_symbol_t * sym;

  index = ( kb_hash(name) % KB_SYMBOL_TABLE_SIZE );
  
  sym = table->symbols[index];

  while ( sym != NULL ) {

    if ( !strcasecmp( sym->name , name ) ) 
      return sym;
   
    sym = sym->next;
  }
  
  return sym;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function dumps the symbol table
 *
 * \param table A symbol table
 */
void kb_dump_symbol_table( kb_symbol_table_t * table )
{
  unsigned int index;
  kb_symbol_t * sym;
  
  kb_msg("------------------------------------------------------------------\n");
  kb_msg(" Name                             Type                            \n");
  kb_msg("------------------------------------------------------------------\n");
  
  for (index=0; index<KB_SYMBOL_TABLE_SIZE; index++) {

    sym = table->symbols[index];
    while (sym != NULL) {
      kb_msg( " %-32s %-32s\n" , 
	      sym->name , 
	      kb_symbol_type_names[sym->type] );
      
      sym = sym->next;
    }
  }

  kb_msg("==================================================================\n");
}

