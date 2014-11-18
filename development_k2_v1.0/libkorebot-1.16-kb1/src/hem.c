/*--------------------------------------------------------------------
 * hem.c - KoreBot Library - Hemisson Robot Support for KoreBot
 *--------------------------------------------------------------------
 * $Id: hem.c,v 1.3 2004/08/18 08:13:44 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Date: 2004/08/18 08:13:44 $
 * $Revision: 1.3 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*! 
 * \file   hem.c Hemisson Robot Support for KoreBot.             
 *
 * \brief 
 *         This module provides useful basic facilities to use a 
 *         Hemisson Robot from KoreBot.
 *
 * \author   Cédric Gaudin (K-Team SA)                               
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered. (not tested)                                         
 * \todo     nothing.
 */

/*--------------------------------------------------------------------*/
/*! This function returns the version of the HemiOS. 
 */
int hem_getOSVersion( knet_dev_t * dev , 
		      unsigned char *version , 
		      unsigned char *revision )
{
  char *p;
  char buf[16];
  int rc;

  if ((rc=knet_printf( dev , "B\r\n")) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'b' )
    return 0;

  p = &buf[2];

  if ( version != NULL ) {
    *version = atoi(p);
  }

  if ( revision != NULL ) {

    while (*p!='\0' && *p!='.' ) p++;
    if (*p == '.' ) {
      p++;
	*revision = atoi(p);
    }
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! This function changes the speed of the left & right motor of the
 *  Hemission robot.
 *
 * \param dev   K-Net Device Descriptor
 * \param left  Left Motor Speed.  (range [-9..9])
 * \param right Right Motor Speed. (range [-9..9])
 * \returns <=0 on error or >0 on sucess 
 *
 * \remark A speed of '0' stops the motor. 
 */
int hem_setSpeed( knet_dev_t * dev , int left , int right )
{
  char buf[32];
  int rc;

  if ( left > 9 ) left = 9;
  if ( left < -9 ) left = -9;
  if ( right > 9 ) right = 9;
  if ( right < -9 ) right = -9;

  if ((rc=knet_printf( dev , "D,%d,%d\r\n" , left , right )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'd' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*! This function changes the state of the continous beeper.
 */
int hem_beep( knet_dev_t * dev , unsigned int state )
{
  char buf[16];
  int rc;

  if ((rc = knet_printf( dev , "H,%u\r\n" , (state!=0?1:0))) < 0 )
    return rc;

  if ((rc = knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'h' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*!
 */
int hem_readSwitches( knet_dev_t * dev , unsigned int * state )
{
  char buf[32];
  int rc;
  unsigned int val = 0;

  if ((rc = knet_printf( dev , "I\r\n")) < 0 )
    return rc;

  if (( rc = knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'i' ) {
    return 0;
  }

  if ( buf[2] == '1' ) {
    val |= hem_Switch1;
  }

  if ( buf[4] == '1' ) {
    val |= hem_Switch2;
  }

  if ( buf[6] == '1' ) {
    val |= hem_Switch3;
  }

  if ( buf[8] == '1' ) {
    val |= hem_Switch4;
  }

  if ( state != NULL ) {
    *state = val;
  }

  return 1;
}

/*--------------------------------------------------------------------*/
/*! This function changes the LED state.
 *
 * \param dev    K-Net Device Descriptor
 * \param state  New Led State
 */
int hem_setLEDs( knet_dev_t * dev , unsigned int state )
{
  unsigned char buf[16];
  int rc;

  if ((rc=knet_printf( dev , "L,%u,%u,%u,%u\r\n" , 
		       ((state & hem_LedOnOff)?1:0) ,
		       ((state & hem_LedPgmExec)?1:0) ,
		       ((state & hem_LedFrontLeft)?1:0) ,
		       ((state & hem_LedFrontRight)?1:0) )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 ) 
    return rc;

  if ( buf[0] != 'l' )
    return 0;
  
  return 1;
} 

/*--------------------------------------------------------------------*/
/*! This function reads the proximity of an object using the 
 *  infra-red  sensors.
 * 
 * \param dev     K-Net Device Descriptor
 * \param sensor  An array of size hem_SensorMax containing the
 *                proximity intensity for each sensor (range [0..255]).
 *                The smaller is the value, the further is the object.
 *
 * \returns <= on error or >0 on success
 */
int hem_readProximitySensors( knet_dev_t * dev ,
			      unsigned char * sensors )
{
  unsigned char buf[64];
  char * p;
  int rc;
  int pos;

  if ((rc = knet_printf( dev , "N\r\n")) < 0 )
    return rc;

  if ((rc = knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 ) 
    return rc;

  if ( buf[0] != 'n' )
    return 0;

  pos = 0;

  for ( p = strtok(buf,",") ;  
	p != NULL && pos < hem_SensorMax ; 
	p = strtok(NULL,",")) {
  
    sensors[pos++] = atoi(p);
  }

  return 1;
}

/*--------------------------------------------------------------------*/
/*! This function reads the ambient light using the infra-red sensors.
 *
 * \param dev    K-Net Device Descriptor
 * \param sensor An array of size hem_SensorMax containing the 
 *               ambient light intensity for each sensor 
 *               (range [0..255]). The smaller is the value, the 
 *               more infra-red light is detected.
 *
 * \returns <=0 on error or >0 on success             
 */
int hem_readAmbientLightSensors( knet_dev_t * dev , 
				 unsigned char * sensors )
{
  unsigned char buf[64];
  char * p;
  int rc;
  int pos;

  if ((rc = knet_printf( dev , "O\r\n")) < 0 )
    return rc;
  
  if ((rc = knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 ) 
    return rc;

  if ( buf[0] != 'o' )
    return 0;

  pos = 0;

  for ( p = strtok(buf,",") ;  
	p != NULL && pos < hem_SensorMax ; 
	p = strtok(NULL,",")) {
  
    sensors[pos++] = atoi(p);
  }

  return 1; 
}

/*--------------------------------------------------------------------*/
/*! This function reads the last character received from the TV 
 *  remote.
 * 
 */
int hem_readTVRemote( knet_dev_t * dev ,
		      unsigned char * value )
{
  unsigned char buf[16];
  int rc;

  if ((rc = knet_printf( dev , "T\r\n")) < 0 )
    return rc;
  
  if ((rc = knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  if ( buf[0] != 't' )
    return 0;

  
  if ( value != NULL ){
    *value = atoi( &buf[2] );
  }

  return 1;
}

/*--------------------------------------------------------------------*/
/*! This function reads a given number of registers of an I²C 
 *  peripheral. 
 *
 * \param dev   K-Net device descriptor
 * \param slave I²C Slave address
 * \param reg   I²C Start register
 * \param data  I²C Data read
 * \param count Number of registers to read
 *
 * \returns The number of registers read on success or <=0 on error  
 */
int hem_readI2C( knet_dev_t * dev , 
		 unsigned char slave , 
		 unsigned char reg , 
		 unsigned char * data,
		 unsigned char count )
{
  unsigned char buf[ 130 ];
  int rc;
  unsigned int pos;

  if ( count > (sizeof(buf)-3)/2 ) { 
    count = (sizeof(buf)-3)/2;
  }
  
  if ((rc=knet_printf( dev , 
		       "R,%02X,%02X,%02X\r\n" , 
		       slave , reg , count )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , 
			    buf , sizeof(buf-1) , '\n' )) < 0)
    return rc;
  
  if ( buf[0] != 'r' )
    return 0;

  for (pos=0; pos<count; pos++) {
    data[pos] = strtoul( &buf[2+pos*2] , NULL , 16 );
  }

  return count;
}

/*--------------------------------------------------------------------*/
/*! This function writes a given number of registers to an I²C 
 * peripheral.
 */
int hem_writeI2C( knet_dev_t * dev ,
		  unsigned char slave , 
		  unsigned char reg , 
		  unsigned char *data,
		  unsigned char count )
{
  unsigned char buf[16];
  int rc;
  unsigned int pos;

  for (pos=0; pos<count; pos++) {
  
    if ((rc=knet_printf( dev ,
			 "W,%02X,%02X,%02X" , 
			 slave , 
			 (unsigned char)(reg+pos) ,
			 data[pos] )) < 0)
      return rc;

    if ((rc=knet_read_string( dev ,
			      buf , sizeof(buf)-1 , '\n' )) < 0 )
      return rc;
  
    if ( buf[0] != 'w' )
      return 0;
  }

  return count;
}

/*--------------------------------------------------------------------*/
/*! This function reads all input of the Hemisson Robot.
 *
 * \param dev                 K-Net Device Descriptor
 * \param motorLeft           Left motor state
 * \param motorRight          Right motor state
 * \param proximitySensors    An array of size hem_SensorMax containing 
 *                            the proximity intensity for each sensor 
 *                            (range [0..255]). The smaller is the 
 *                            value, the further is the object.
 * \param ambientLightSensors An array of size hem_SensorMax 
 *                            containing the ambient light intensity 
 *                            for each sensor
 * \param switches            4-bit switch value
 * \param tvRemoveData        TV remote last data received
 * \param version             HemiOS version number
 * \param revision            HemiOS revision number
 * 
 * \returns <=0 on error or 1 on success
 */
int hem_fastRead( knet_dev_t * dev ,
		  unsigned char *motorLeft , 
		  unsigned char *motorRight ,
		  unsigned char *proximitySensors ,
		  unsigned char *ambientLightSensors ,
		  unsigned char *switches ,
		  unsigned char *tvRemoteData ,
		  unsigned char *version ,
		  unsigned char *revision )
{
  unsigned char buf[64];
  int rc;
  unsigned int pos;

  if ((rc = knet_printf( dev , "&\r\n" )) < 0 )
    return rc;

  if ((rc=knet_llread( dev , buf , 25 )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != '&' )
    return 0;

  if ( motorLeft != NULL ) {
    *motorLeft = buf[1];
  }

  if ( motorRight != NULL ) {
    *motorRight = buf[2];
  }

  if ( proximitySensors != NULL ) {
    for (pos=0; pos<hem_SensorMax; pos++) {
      proximitySensors[pos] = buf[3+pos];
    }
  }

  if ( ambientLightSensors != NULL ) {
    for (pos=0; pos<hem_SensorMax; pos++) {
      ambientLightSensors[pos] = buf[11+pos];
    }
  }
  
  if ( switches != NULL ) {
    *switches = buf[19];
  }

  if ( tvRemoteData != NULL ) {
    *tvRemoteData = buf[20];
  }

  if ( version != NULL ) {
    *version = buf[21];
  }

  if ( revision != NULL ) {
    *revision = buf[22];
  }
  
  return 1;
}

/*--------------------------------------------------------------------*/
/*! This function changes the motor speed and the led state.
 *
 * \param dev        K-Net Device Descriptor
 * \param motorLeft  Left motor speed
 * \param motorRight Right motor speed
 * \param ledState   Led state
 * 
 * \returns <=0 on error or 1 on success
 */
int hem_fastWrite( knet_dev_t * dev , 
		   unsigned char motorLeft ,
		   unsigned char motorRight ,
		   unsigned char ledState )
{
  int rc;
  char buf[16];

  if ((rc = knet_printf( dev , "*%c%c%c\r\n" , 
			 motorLeft , 
			 motorRight , 
			 ledState )) < 0 )
    return rc;

  
  if ((rc = knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  if ( buf[0] != '*' )
    return 0;

  return 1;
}
