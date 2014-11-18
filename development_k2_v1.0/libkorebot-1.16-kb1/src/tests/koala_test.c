/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau_local $
 * $Date: 2004/11/01 13:59:16 $
 * $Revision: 1.2 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/tests/koala_test.c,v 1.2 2004/11/01 13:59:16 pbureau_local Exp $
 */

/*--------------------------------------------------------------------*/
/*! 
 * \file   koala_test.c koala test program 
 *
 * \brief
 * 	  test program to check the communication between the korebot and the koala.
 *
 * \author   Pierre Bureau (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 */



#include <korebot/korebot.h>

int main( int argc , char * argv[] )
{
  knet_dev_t *koala;
  int rc;
  unsigned char buf[16];
  unsigned char a,b,c,d;

  kb_set_debug_level( 2 );

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  koala = knet_open( "Koala:Robot", KNET_BUS_ANY, 0 , NULL );
  if(!koala)
    printf("Open failed\r\n");
  
  koa_getOSVersion(koala,&a,&b,&c,&d);
  printf("OS revision %d.%d  protocol %d.%d\r\n",a,b,c,d);

  koa_setSpeed(koala,10,10);
  sleep(1);
  koa_setSpeed(koala,-10,-10);
  sleep(1);

  koa_setSpeed(koala,0,0);

  kb_config_exit();
  
  return 0;
}

