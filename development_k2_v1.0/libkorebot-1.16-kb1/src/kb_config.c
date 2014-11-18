/*--------------------------------------------------------------------
 * kb_config.c - KoreBot Library -  Configuration Handling
 *--------------------------------------------------------------------
 * $Id: kb_config.c,v 1.7 2005/01/31 07:34:28 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Revision: 1.7 $
 * $Date: 2005/01/31 07:34:28 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*--------------------------------------------------------------------*/
/*! 
 * \file   kb_config.c Configuration Handling
 *
 * \brief
 *         This module reads, parses and store the information 
 *         contained in  the configuration files.
 *
 * \author   Cédric Gaudin (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.
 * \todo     nothing.
 */


static int kb_is_valid_ientifier( const char * str );

static void kb_config_error( const char * file ,
			     unsigned int line ,
			     const char * func ,
			     unsigned int error , 
			     va_list argptr );

static int kb_parse_section( int argc , char * argv[] , void * data);
static int kb_parse_modbus( int argc , char * argv[] , void * data);
static int kb_parse_modbusaddr( int argc , char * argv[] , void * data);
static int kb_parse_alias( int argc , char * argv[] , void * data);
static int kb_parse_device( int argc , char * argv[] , void * data);
static int kb_parse_register( int argc , char * argv[] , void * data);

static struct kb_command_s kb_config_cmds[] = {
  { "section"   , 1 , 1 , kb_parse_section   } ,
  { "modulebus" , 1 , 1 , kb_parse_modbus    } ,
  { "modbusaddr", 1 , 1 , kb_parse_modbusaddr} ,
  { "alias"     , 1 , 1 , kb_parse_alias     } ,
  { "device"    , 3 , 3 , kb_parse_device    } ,
  { "register"  , 2 , 2 , kb_parse_register  } ,
  { NULL        , 0 , 0 , NULL               }
};

/*!
 * List of existing device class names, as they should
 * appear in configuration files. Knet bus names are
 * defined in knet.h.
 */
const char * device_class_names[] = { 
    "i2c" , "spi" , "module" , "rs232" , NULL 
  };

static kb_symbol_table_t kb_config_table;
static kb_symbol_t *     kb_sections = NULL;
static kb_symbol_t *     kb_config_scope = NULL;
static const char *      kb_config_file = 0;
static unsigned int      kb_config_line = 0;

/*--------------------------------------------------------------------*/
/*! 
 * This function initializes the configuration tables and variables
 *
 * \param argc Number of element in the array of arguments 
 * \param argv Array of string containing all arguments
 * \return an error code
 *        - <0 error code
 *        - 0 no error
 */
int kb_config_init( int argc , char * argv[] )
{
  DIR * dir;
  struct dirent *d;
  char *p;
  int rc = 0;
  unsigned int count = 0;
  char filename[PATH_MAX + 1];

  kb_create_symbol_table( &kb_config_table );

  /* read all configuration files */
  if ((dir = opendir(KB_CONFIG_DIR_INTERNAL)) == NULL ) {
    return kb_warning(KB_WARN_OPENDIR, KB_CONFIG_DIR_INTERNAL);
  }
  
  while ((d = readdir( dir)) != NULL ) {
    strcpy(filename, KB_CONFIG_DIR_INTERNAL);
    strcat(filename, d->d_name);
    p = d->d_name + strlen(d->d_name) - 4;
    if (!strncmp(p,".knc",4)) {
      count++;
      if ((rc=kb_parse_config_file(filename))<0) {
	break;
      }
    }
  }

  if(!count)
    kb_warning( KB_WARN_CONFIG_FILE, KB_CONFIG_DIR_INTERNAL );

  closedir( dir );

  /*atexit( kb_config_exit );*/
  
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function should be called on exit by 'kb_init' and free 
 * all memory  used in this module.
 *
 * \remark Never call this function !
 */
void kb_config_exit( void )
{
  kb_destroy_symbol_table(&kb_config_table);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function try to find a device configuration descriptor given 
 * its name.
 * 
 * \param name A string containing the name of the device to find.
 * \return A pointer to the device configuration descriptor or 
 *         NULL if no device is found.
 */
kb_device_config_t * kb_lookup_device( const char * name )
{
  kb_symbol_t * sym;
  kb_alias_config_t * alias;
  
  if ((sym = kb_lookup_symbol( &kb_config_table , name )) != NULL ) {
    
    if ( sym->type == KB_SYMBOL_TYPE_ALIAS ) {
      alias = (kb_alias_config_t *)sym->value;
      
      if ( alias->ptr == NULL ) {
	sym = kb_lookup_symbol( &kb_config_table , alias->name );
      }
      else {
	sym = alias->ptr;
      }

      if ( sym == NULL ) return NULL;
    }

    if ( sym->type == KB_SYMBOL_TYPE_DEVICE ) {
      return (kb_device_config_t *)sym->value;
    }
    
  }
  return NULL;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function try to find a register configuration descriptor 
 * given its name.
 *
 * \param name A string containing the name of the register to find.
 * \return A pointer to the register configuration descriptor or 
 *         NULL if no register is found.
 * 
 */
kb_register_config_t * kb_lookup_register( const char * name )
{
  kb_symbol_t * sym;
  kb_alias_config_t * alias;
 
  if ((sym = kb_lookup_symbol( &kb_config_table , name )) != NULL ) {
 
    if ( sym->type == KB_SYMBOL_TYPE_ALIAS ) {
      alias = (kb_alias_config_t *)sym->value;
      
      if ( alias->ptr == NULL ) {
	sym = kb_lookup_symbol( &kb_config_table , alias->name );
      }
      else {
	sym = alias->ptr;
      }

      if ( sym == NULL ) return NULL;
    }
     
    if ( sym->type == KB_SYMBOL_TYPE_REGISTER ) 
      return (kb_register_config_t *)sym->value;

  }  
  return NULL;
}

/*--------------------------------------------------------------------*/
/*!
 * This function checks if a string is a valid identifier. 
 * A valid identifier must begin with a letter of '_' and is followed 
 * by letters, digits or '_'. An identifier must be longer than 3 
 * characters.
 *
 * letter     ::= [A-Za-z_]
 * digit      ::= [0-9]
 *
 * identifier ::= letter ( letter | digit )*
 *  
 * \param str  Pointer to a string containing an identifier to check
 * \return An error code: 
 *         - <0 on error
 *         - 0 the identifier is valid
 *
 * \remark This function is NOT exported outside this module.
 */
static int kb_is_valid_identifier( const char * str ) 
{
  unsigned int pos;
  unsigned int len = strlen(str);
  
  int ok = 0;

  if ((isalpha(str[0]) || str[0]=='_') && len >= 3) {

    ok = 1;
    for (pos=1; pos<len; pos++) {
      if ( ! (isalnum(str[pos]) || str[pos]=='_') ) {
	ok = 0;
	break;
      }
    }
  }

  if ( !ok ) {
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "kb_is_valid_identifier" ,
		     KB_ERROR_INVALID ,
		     str );
  }

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * The function builds a symbol name using the current scope.
 *
 * \param scoped_name  Pointer to the buffer that will receive the 
 *                     scoped name. If a NULL value is specified,
 *                     the internal buffer is used. 
 * \param name         Pointer to the name to scope
 *
 * \return Pointer to the buffer
 *
 * \remark This function is not exported outside this module.
 */
static char * kb_build_scoped_name( char * scoped_name , 
				    const char * name )
{
  static char buffer[ KB_SYMBOL_NAME_SIZE ]; 

  if ( scoped_name == NULL )
    scoped_name = buffer;

  if ( kb_config_scope != NULL ) 
    sprintf( scoped_name , "%s:%s" , kb_config_scope->name , name  );
  else 
    strcpy( scoped_name , name );

  return scoped_name;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function is the error handler of the configuration file parser
 *
 * \param file   A string containing the name of the file where the 
 *               error has occurred.
 * \param line   Line number where the error has occurred.
 * \param func   A string containing the name of the function where
 *               the error has occurred.
 * \param error  the code of the error
 * \param argptr optional arguments
 *
 * \remark This function is NOT exported outside this module. 
 */
static void kb_config_error( const char * file ,
			     unsigned int line ,
			     const char * func ,
			     unsigned int error , 
			     va_list argptr )
{
  fprintf( stderr , "%s:%u " , 
	   kb_config_file , 
	   kb_config_line ); 
  
  kb_verror( file , line , func , error , argptr );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function parses a 'modulebus' command line in a configuration
 * file. This will set the module bus for the current section. The module 
 * bus is used to access all the devices defined as 'module'. 
 *
 * \param argc Number of arguments
 * \param argv An array of strings that represents the list of token
 * \return an error code:
 *   - <0 on error
 *   - 0  'section' already defined
 *   - 1  on success
 * 
 * \remark This function is NOT exported outside this module
 */
static int kb_parse_modbus( int argc , char * argv[] , void * data)
{
  int device_class;
  kb_section_config_t * section;

  /* Check if a section is currently configured */
  if ( !kb_config_scope || !(section = (kb_section_config_t *)kb_config_scope->value)) {
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "kb_parse_modbus" ,
		     KB_ERROR_NOSECTION ,
		     argv[0], argv[1] );
  }

  /* Check if the given bus is valid */
  device_class = kb_find_string( argv[1] , knet_bus_names );

  if ( device_class == -1 ) {
    return kb_error( __FILE__ ,
	             __LINE__ ,
	             "kb_parse_modbus" ,
	             KB_ERROR_UNKBUS ,
	             argv[1] );
  }

  /* The bus number is the bus names index minus one */
  section->module_bus = device_class - 1;

  return 1;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function parses a 'modbusaddr' command line in a configuration
 * file. This will set the address for module bus of the current section. The module 
 * bus is used to access all the devices defined as 'module'. 
 *
 * \param argc Number of arguments
 * \param argv An array of strings that represents the list of token
 * \return an error code:
 *   - <0 on error
 *   - 0  'section' already defined
 *   - 1  on success
 * 
 * \remark This function is NOT exported outside this module
 */
static int kb_parse_modbusaddr( int argc , char * argv[] , void * data)
{
  int device_class;
  kb_section_config_t * section;

  /* Check if a section is currently configured */
  if ( !kb_config_scope || !(section = (kb_section_config_t *)kb_config_scope->value)) {
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "kb_parse_modbus" ,
		     KB_ERROR_NOSECTION ,
		     argv[0], argv[1] );
  }

  /* Store the address */
  section->module_bus_addr = atoi(argv[1]);

  return 1;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function parses a 'section' command line in a configuration
 *
 * \param argc Number of arguments
 * \param argv An array of strings that represents the list of token
 * \return an error code:
 *   - <0 on error
 *   - 0  'section' already defined
 *   - 1  on success
 * 
 * \remark This function is NOT exported outside this module
 */
static int kb_parse_section( int argc , char * argv[] , void * data)
{
  kb_symbol_t * sym;
  kb_section_config_t * section;

  /* verify the identifier */
  if (kb_is_valid_identifier(argv[1])<0) 
    return 0;
  
  /* symbol already defined ? */
  if ( kb_lookup_symbol( &kb_config_table , argv[1] ) != NULL ) {
    
    kb_error( __FILE__ ,
	      __LINE__ ,
	      "kb_config_parse_section" ,
	      KB_ERROR_SYMDEF , argv[1] );
    return 0;
  }

  section = KB_ALLOC( kb_section_config_t , 1 );
  section->module_bus      = -1;
  section->module_bus_addr = -1;
  section->device_count    = 0;
  section->devices         = NULL;
  section->register_count  = 0;
  section->registers       = NULL;
  section->next            = NULL;

  sym = KB_ALLOC( kb_symbol_t , 1 );
  sym->alloc = 1;
  sym->value = (unsigned int) section;
  sym->type = KB_SYMBOL_TYPE_SECTION;
  sym->next = NULL;
  strcpy( sym->name , argv[1] );
  
  kb_add_symbol( &kb_config_table , sym );

  section->next = kb_sections;
  kb_sections = sym;

  kb_config_scope = sym;
  
  return 1;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function parses a 'alias' command line in a configuration
 *
 * \param argc Number of arguments
 * \param argv An array of strings that represents the list of token
 * \return an error code:
 *   - <0 on error
 *   - 0  'alias' already defined
 *   - 1  on success
 * 
 * \remark This function is NOT exported outside this module
 */
static int kb_parse_alias( int argc , char * argv[] , void * data)
{
  char sym_name[ KB_SYMBOL_NAME_SIZE ];
  char *alias_name;
  kb_symbol_t *sym , * aliasSym ;
  kb_alias_config_t * alias;
  kb_section_config_t * section;
 
  /* verify the identifier */
  if ( kb_is_valid_identifier( argv[1] ) < 0) 
    return 0;
 
  kb_build_scoped_name( sym_name , argv[1] );
  
  /* symbol already defined ? */
  if (kb_lookup_symbol( &kb_config_table , sym_name ) != NULL ) {
    kb_error( __FILE__ ,
	      __LINE__ ,
	      "kb_parse_alias" ,
	      KB_ERROR_SYMDEF ,
	      sym_name );
    return 0;
  }

  /* already have a different scope */
  if ( strchr( argv[2] , ':' ) == NULL ) {
    alias_name = kb_build_scoped_name( NULL , argv[2] );
  }
  else {
    alias_name = argv[2];
  }

  /* Check if a section is currently set */
  if ( !kb_config_scope || !(section = (kb_section_config_t *)kb_config_scope->value)) {
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "kb_parse_alias" ,
		     KB_ERROR_NOSECTION ,
		     "alias" , argv[1] );
  }
  
  /* try to find the aliases symbol */
  aliasSym = kb_lookup_symbol ( &kb_config_table , alias_name );
	
  alias      = KB_ALLOC( kb_alias_config_t , 1 );
  alias->ptr = aliasSym;
  strcpy(alias->name , alias_name );

  sym        = KB_ALLOC( kb_symbol_t , 1 );
  sym->type  = KB_SYMBOL_TYPE_ALIAS;
  sym->value = (unsigned int)alias;
  sym->alloc = 1;
  sym->next  = NULL;
  strcpy( sym->name , sym_name );

  alias->next = section->aliases;
  section->aliases = sym;
  section->alias_count++;

  kb_add_symbol( &kb_config_table , sym );
  
  return 1;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function parses a 'device' command line in a configuration
 *
 * \param argc Number of arguments
 * \param argv An array of strings that represents the list of token
 * \return an error code:
 *   - <0 on error
 *   - 0  'device' already defined
 *   - 1  on success
 * 
 * \remark This function is NOT exported outside this module
 */
static int kb_parse_device( int argc , char * argv[] , void * data)
{
  int device_class;
  long device_address;
  char * device_name;
  char * sym_name;
  kb_symbol_t * sym;
  kb_device_config_t * dev;
  
  /* Check if the given identifier is valid */
  if (kb_is_valid_identifier( argv[1] )<0)
    return 0;

  /* Translate the device class name to its index */
  device_class = kb_find_string( argv[2] , device_class_names );

  if ( device_class == -1 ) {
    kb_error( __FILE__ ,
	      __LINE__ ,
	      "kb_parse_device" ,
	      KB_ERROR_UNKDEVCLASS ,
	      argv[2] );
    return 0;
  }

  device_name    = NULL;
  device_address = 0;
  sym_name = kb_build_scoped_name( NULL , argv[1] );

  /* Check if the given symbol is already created */
  if ((sym=kb_lookup_symbol( &kb_config_table , sym_name )) != NULL ) {
    
    /* symbol is already defined for something else than a device */
    if (sym->type != KB_SYMBOL_TYPE_DEVICE ) {
      kb_error( __FILE__ ,
		__LINE__ ,
		"kb_parse_device" ,
		KB_ERROR_SYMDEF ,
		argv[1] );
      return 0;
    }
    /* check if device class is already used */
    dev = (kb_device_config_t *)sym->value;
    
    if ( dev->kclass[ device_class ].defined ) {
      kb_error( __FILE__ ,
		__LINE__ ,
		"kb_parse_device" ,
		KB_ERROR_DEVCLASSUSED ,
		argv[1] , argv[2] );
      return 0;
    }

  }
  else {
    /* Create a new device description */
    kb_section_config_t * section;
    
    if ( !kb_config_scope || !(section = (kb_section_config_t *)kb_config_scope->value)) {
      return kb_error( __FILE__ ,
		       __LINE__ ,
		       "kb_parse_device" ,
		       KB_ERROR_NOSECTION ,
		       "device" , argv[1] );
    }

    dev = KB_ALLOC( kb_device_config_t , 1 );

    dev->next = NULL;

    /* Create  a new symbol for the new device */
    sym = KB_ALLOC( kb_symbol_t , 1 );
    sym->type = KB_SYMBOL_TYPE_DEVICE;
    sym->value = (unsigned int) dev;
    sym->alloc = 1;
    sym->next = NULL;
    strcpy( sym->name , sym_name );

    dev->next        = section->devices;
    dev->section     = section;
    section->devices = sym;
    section->device_count++;

    kb_add_symbol( &kb_config_table , sym );
    
  }
  
  /* Fill in the new class description for the device */
  /* Generic class properties */
  dev->kclass[ device_class ].defined = 1;

  /* Class specific properties */
  switch(device_class)
  {
    case KB_DEVICE_CLASS_I2C:
    case KB_DEVICE_CLASS_MODULE:
    case KB_DEVICE_CLASS_SPI:
      /* Read the device address */
      if ((device_address = strtol(argv[3],NULL,0))<0) {
	kb_error( __FILE__ ,
	    __LINE__ ,
	    "kb_parse_device" ,
	    KB_ERROR_INVNUM , argv[3] );

	return 0;
      }    

      dev->kclass[ device_class ].address     = device_address;
      strncpy(dev->kclass[ device_class ].device_name, "/dev/null", KB_SYMBOL_NAME_SIZE);
      break;

    case KB_DEVICE_CLASS_RS232:
      dev->kclass[ device_class ].address     = device_address;
      device_name = argv[3];
      if ( device_name != NULL )
	strncpy(dev->kclass[ device_class ].device_name, device_name, KB_SYMBOL_NAME_SIZE);
      else {
	kb_error( __FILE__ ,
	    __LINE__ ,
	    "kb_parse_device" ,
	    KB_ERROR_INVNAME , argv[3] );

	return 0;
      }    
      
      break;
  }

  
#if 0
  if ( device_class == KB_DEVICE_CLASS_SPI ||
       device_class == KB_DEVICE_CLASS_MODULE ||
       device_class == KB_DEVICE_CLASS_I2C ) {

    if ((device_address = strtol(argv[3],NULL,0))<0) {
      kb_error( __FILE__ ,
		__LINE__ ,
		"kb_parse_device" ,
		KB_ERROR_INVNUM , argv[3] );
      return 0;
    }    
  }
  else {
    device_name    = argv[3];
  }

  
  if ((sym=kb_lookup_symbol( &kb_config_table , sym_name )) != NULL ) {
    
    /* symbol is already defined for something else than a device */
    if (sym->type != KB_SYMBOL_TYPE_DEVICE ) {
      kb_error( __FILE__ ,
		__LINE__ ,
		"kb_parse_device" ,
		KB_ERROR_SYMDEF ,
		argv[1] );
      return 0;
    }
      
    /* device class already used ? */
    dev = (kb_device_config_t *)sym->value;
    
    if ( dev->kclass[ device_class ].defined ) {
      kb_error( __FILE__ ,
		__LINE__ ,
		"kb_parse_device" ,
		KB_ERROR_DEVCLASSUSED ,
		argv[1] , argv[2] );
      return 0;
    }

    /* Add a new definition to the class list for this device */
    dev->kclass[ device_class ].defined = 1;
    dev->kclass[ device_class ].address = device_address;
    if ( device_name != NULL ) 
      strncpy( dev->kclass[ device_class ].device_name , 
	       device_name , KB_SYMBOL_NAME_SIZE );
  }
  else {

    /* Create a new device description */
    kb_section_config_t * section;
    
    if ( !kb_config_scope || !(section = (kb_section_config_t *)kb_config_scope->value)) {
      return kb_error( __FILE__ ,
		       __LINE__ ,
		       "kb_parse_device" ,
		       KB_ERROR_NOSECTION ,
		       "device" , argv[1] );
    }

    dev = KB_ALLOC( kb_device_config_t , 1 );
    
    dev->kclass[ device_class ].defined = 1;
    dev->kclass[ device_class ].address = device_address;
    if ( device_name != NULL )
      strncpy( dev->kclass[ device_class ].device_name ,
	       device_name , KB_SYMBOL_NAME_SIZE );
    dev->next = NULL;

    sym = KB_ALLOC( kb_symbol_t , 1 );
    sym->type = KB_SYMBOL_TYPE_DEVICE;
    sym->value = (unsigned int) dev;
    sym->alloc = 1;
    sym->next = NULL;
    strcpy( sym->name , sym_name );

    dev->next        = section->devices;
    dev->section     = section;
    section->devices = sym;
    section->device_count++;

    kb_add_symbol( &kb_config_table , sym );
  }
#endif
  return 1;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function parses a 'register' command line in a configuration
 *
 * \param argc Number of arguments
 * \param argv An array of strings that represents the list of token
 * \return an error code:
 *   - <0 on error
 *   - 0  'register' already defined
 *   - 1  on success
 * 
 * \remark This function is NOT exported outside this module
 */
static int kb_parse_register( int argc , char * argv[] , void * data)
{
  long value;
  char * sym_name;
  kb_symbol_t * sym;
  kb_register_config_t * reg;
  kb_section_config_t * section;

  /* verify the identifier */
  if ( kb_is_valid_identifier( argv[1] ) < 0 ) 
    return 0;
  
  if ((value = strtol( argv[3] , NULL , 0 )) < 0) {
    kb_error( __FILE__ , 
	      __LINE__ ,
	      "kb_parse_register" , 
	      KB_ERROR_INVNUM , argv[3] );
    return 0;
  }
  
  sym_name = kb_build_scoped_name( NULL , argv[1] );

  if (( sym = kb_lookup_symbol( &kb_config_table , sym_name )) != NULL ) {
    kb_error( __FILE__ ,
	      __LINE__ ,
	      "kb_parse_register" ,
	      KB_ERROR_SYMDEF , sym_name );
    return 0;
  }


  if ( !kb_config_scope || !(section = (kb_section_config_t *)kb_config_scope->value)) {
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "kb_parse_register" ,
		     KB_ERROR_NOSECTION ,
		     "register" , argv[1] );
  }
  
  reg = KB_ALLOC( kb_register_config_t , 1 );
  reg->value = value;
  reg->next = NULL;

  sym = KB_ALLOC(kb_symbol_t,1);
  sym->type = KB_SYMBOL_TYPE_REGISTER;
  sym->alloc = 1;
  strcpy( sym->name , sym_name );
  sym->value = (unsigned int)reg;
  sym->next = NULL;

  reg->next = section->registers;
  section->registers = sym;
  section->register_count++;

  kb_add_symbol( &kb_config_table , sym );
  
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function parses a configuration file, builds and stores 
 * configuration information in memory.
 *
 * \param file configuration file to parse
 * \return an error code:
 *   - <0 on error
 *   - 0  on success
 * 
 */
int kb_parse_config_file( const char * file )
{
  FILE * in;
  char * buf;
  int rc , len;
  
  /* open the configuration file */
  if ((in = fopen( file , "r")) == NULL ) {
    return kb_error( __FILE__ ,
		     __LINE__ ,
		     "kb_parse_config_file" ,
		     KB_ERROR_FILEOPEN ,
		     file );
  }

  buf = KB_ALLOC( char , 1024 );

  kb_set_error_handler( kb_config_error );

  kb_config_line  = 0;
  kb_config_file  = file;
  kb_config_scope = NULL;

  while(fgets(buf,1024, in) != NULL) {

    kb_config_line++;

    /* remove the line ending character */
    len = strlen( buf );
    if ( len > 0 )
      buf[len-1] = '\0';

    /* comment or empty line */
    if (buf[0]=='#' || buf[0]=='\0')
      continue;

    if ((rc = kb_parse_command( buf , kb_config_cmds ,NULL))<0) { 
      if ( rc != -KB_ERROR_UNKCMD )
	break;
    }
    rc = 0;
    
  }
  kb_set_error_handler( NULL );
  kb_free(buf);
  fclose(in);
  return rc;
} 

/*--------------------------------------------------------------------*/
/*! 
 * This function enumerates all sections.
 *
 * \param func    A Pointer to a function called for each section 
 *                found.
 * \param context A Pointer or a value passed as parameter to the 
 *                function 'func'
 * \return an error code:
 *          - <0 on error
 *          - >=0 the number of sections
 */
int kb_enum_section( int (*func)( const char * name ,
				  kb_section_config_t * section ,
				  void * context ) ,
		     void * context )
{
  int rc, count = 0;
  kb_symbol_t * sym = kb_sections;
  kb_section_config_t * section;

  if ( sym ) {
    while ( sym != NULL ) {
      section = (kb_section_config_t *)sym->value;

      if ((rc = func( sym->name , section , context ))<0)
	return rc;

      sym = section->next;
      count++;
    }
    return count;
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function enumerates all aliases of a given section.
 *
 * \param section_name A string containing the name of the section
 * \param func         A function called for each alias found.
 * \param context      A Pointer or a value passed as parameter to
 *                     function 'func'.
 * \return an error code:
 *    - <0 on error
 *    - >= number of aliases in a given section
 *
 */
int kb_enum_alias( const char * section_name ,
		   int (*func)( const char * name ,
				kb_alias_config_t * alias ,
				void * context ) ,
		   void * context )
{
  kb_symbol_t * sym;
  kb_section_config_t * section;
  kb_alias_config_t * alias;
  int rc;

  sym = kb_lookup_symbol( &kb_config_table , section_name );

  if ( sym != NULL ) {

    if ( sym->type == KB_SYMBOL_TYPE_SECTION ) {

      section = (kb_section_config_t *)sym->value;

      sym = section->aliases;
     
      while ( sym != NULL ) {

	alias = (kb_alias_config_t *)sym->value;
	
	if ((rc = func( sym->name , 
			alias , 
			context )) < 0 )
	  return rc;

	sym = alias->next; 
      } 
      return (int)section->alias_count;
    }
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function enumerates all devices of a given section.
 *
 * \param section_name A string containing the name of the section
 * \param func         A function called for each device found.
 * \param context      A Pointer or a value passed as parameter to
 *                     function 'func'.
 * \return an error code:
 *    - <0 on error
 *    - >= number of devices in a given section
 *
 */
int kb_enum_device( const char * section_name ,
		    int (*func)( const char * name ,
				 kb_device_config_t * device ,
				 void * context ) ,
		    void * context )
{
  kb_symbol_t * sym;
  kb_section_config_t * section;
  kb_device_config_t * device;
  int rc;

  sym = kb_lookup_symbol( &kb_config_table , section_name );

  if ( sym != NULL ) {

    if ( sym->type == KB_SYMBOL_TYPE_SECTION ) {

      section = (kb_section_config_t *)sym->value;

      sym = section->devices;
     
      while ( sym != NULL ) {

	device = (kb_device_config_t *)sym->value;
	
	if ((rc = func( sym->name , 
			device , 
			context )) < 0 )
	  return rc;

	sym = device->next; 
      } 
      return (int)section->device_count;
    }
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function enumerate all registers of a given section
 *
 * \param section_name A string containing the name of the section
 * \param func         A function called for each register found.
 * \param context      A Pointer or a value passed as parameter to
 *                     function 'func'.
 * \return an error code:
 *    - <0 on error
 *    - >= number of registers in a given section
 */
int kb_enum_register( const char * section_name ,
		      int (*func)( const char * name , 
				   kb_register_config_t * reg ,
				   void * context ) ,
		      void * context )
{
  kb_symbol_t * sym;
  kb_section_config_t * section;
  kb_register_config_t * reg;
  int rc;

  sym = kb_lookup_symbol( &kb_config_table , section_name );

  if ( sym != NULL ) {

    if ( sym->type == KB_SYMBOL_TYPE_SECTION ) {

      section = (kb_section_config_t *)sym->value;

      sym = section->registers;

      while ( sym != NULL ) {

	reg = (kb_register_config_t *)sym->value;

	if ((rc = func( sym->name , reg , context )) < 0 )
	  return rc;

	sym = reg->next;
      } 
      return (int)section->register_count;
    }
  }
  return 0;
}
