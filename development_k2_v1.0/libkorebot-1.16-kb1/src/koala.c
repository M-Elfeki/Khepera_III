/*--------------------------------------------------------------------
 * koala.c - KoreBot Library - Koala Robot Support for KoreBot
 *--------------------------------------------------------------------
 * $Id: koala.c,v 1.2 2006/04/08 15:54:26 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Date: 2006/04/08 15:54:26 $
 * $Revision: 1.2 $
 *--------------------------------------------------------------------*/

#include "korebot.h"


/*! 
 * \file   koala.c Koala Robot Support for KoreBot.             
 *
 * \brief 
 *         This module provides useful basic facilities to use a 
 *         Hemisson Robot from KoreBot.
 *
 * \author   P. Bureau (K-Team SA)                               
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.
 * \todo     nothing.
 */

char * koa_get_nextValue(char * current, char * value)
{
  while(*current != ',' && *current != '\n')
  {
    *value = *current;
    value++;
    current++;
  }

  if(*current == '\n')
    return NULL;
  else
  {
    *value = '\0';
    return current+1;
  }
}

/*--------------------------------------------------------------------*/
/*! This function returns the version of the Koala OS and the
 * protocol revision . 
 * \param dev   K-Net Device Descriptor
 */
int koa_getOSVersion( knet_dev_t * dev , 
		      unsigned char *version , 
		      unsigned char *revision,
   		      unsigned char *pversion,
   		      unsigned char *prevision)
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

  if ( pversion != NULL ) {

    while (*p!='\0' && *p!=',' ) p++;
    if (*p == ',' ) {
      p++;
	*pversion = atoi(p);
    }
  }

  if ( prevision != NULL ) {

    while (*p!='\0' && *p!='.' ) p++;
    if (*p == '.' ) {
      p++;
	*prevision = atoi(p);
    }
  }

  return 0;
}

/*--------------------------------------------------------------------*/
/*! Configure the position PID controller. On robot reset the values are set 
 * to default of Kp to 400, Ki to 4, Kd to 400.
 *
 * \param dev   K-Net Device Descriptor
 * \param Kp Proportional Gain
 * \param Ki Integral Gain
 * \param Kd Derivative Gain
 *
 * \returns <=0 on error or >0 on sucess 
 */
int koa_setPidPosition(knet_dev_t * dev, int Kp, int Ki, int Kd)
{
  char buf[32];
  int rc;

  if ((rc=knet_printf( dev , "F,%d,%d,%d\r\n" , Kp, Ki, Kd)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'f' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*! Set the 32 bit position counter of the two motors. The unit is the pulse,
 * that corresponds to 0,045 mm.
 *
 * \param dev   K-Net Device Descriptor
 * \param left_pos Value to set left position counter
 * \param right_pos Value to set right position counter
 *
 * \returns <=0 on error or >0 on sucess 
 */
int koa_setPositionCounter(knet_dev_t * dev, long pos_left, long pos_right)
{
  char buf[32];
  int rc;

  if ((rc=knet_printf( dev , "G,%d,%d\r\n" , pos_left, pos_right)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'g' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*! Configure the speed PID controller. On robot reset the values are set 
 * to default of Kp to 1000, Ki to 800, Kd to 100.
 *
 * \param dev   K-Net Device Descriptor
 * \param Kp Proportional Gain
 * \param Ki Integral Gain
 * \param Kd Derivative Gain
 *
 * \returns <=0 on error or >0 on sucess 
 */
int koa_setPidSpeed(knet_dev_t * dev, int Kp, int Ki, int Kd)
{
  char buf[32];
  int rc;

  if ((rc=knet_printf( dev , "A,%d,%d,%d\r\n" , Kp, Ki, Kd)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'a' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*! Indicate to the wheel position controller an absolute position to be
 * reached. The motion control perform the movement using the three control
 * phases of a trapezoidal speed shape: an acceleration, a constant speed and a
 * deceleration period. These phases are performed according to the parameters
 * selected for the trapezoidal speed controller (command J). The maximum
 * distance that can be given by this command is (2**23)-2 pulses that
 * correspond to 318m. The unit is the pulse that corresponds to 0.045mm. The
 * movement is done immediately after the command is sent. In the case another
 * command is under execution (speed or position control) the last command
 * replaces the precedent one. Any replacement transition follows acceleration
 * and maximal speed constraints.
 *
 * \param dev   K-Net Device Descriptor
 * \param pos_left Target position for the left motor
 * \param pos_right Target position for the right motor
 * 
 * \returns <=0 on error or >0 on sucess 
 */
int koa_setPosition(knet_dev_t * dev, long pos_left, long pos_right)
{
  char buf[32];
  int rc;

  if ((rc=knet_printf( dev , "C,%d,%d\r\n" , pos_left, pos_right)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'c' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*! Read the instantaneous speed of the two motors. The unit is the pulse/10 ms
 * that corresponds to 4.5 millimetres per second.
 *
 * \param dev   K-Net Device Descriptor
 * \param speedtable Array of 2 integer to store the speed readings
 * 
 * \returns <=0 on error or >0 on sucess 
 *
 * \remark speedtable[0] is left motor speed
 * \remark speedtable[1] is right motor speed
 */
int koa_readSpeed(knet_dev_t * dev, int * speedtable)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "E\r\n" )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'e' )
    return 0;

  scan = buf+2;
  while(scan) {
    scan       = koa_get_nextValue(scan,val);
    *speedtable= atoi(val);
    speedtable++;
  }
  
  return 1;
}

/*--------------------------------------------------------------------*/
/*! Read the 32 bit position counter of the two motors. The unit is the pulse. 
 * 
 * \param dev   K-Net Device Descriptor
 * \param postable Array of 2 integer to store the position readings
 * 
 * \returns <=0 on error or >0 on sucess 
 *
 * \remark postable[0] is left motor speed
 * \remark postable[1] is right motor speed
 */
int koa_readPosition(knet_dev_t * dev, int * postable)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "H\r\n" )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'h' )
    return 0;

  scan = buf+2;
  while(scan) {
    scan       = koa_get_nextValue(scan,val);
    *postable = atoi(val);
    postable++;
  }
  
  return 1;
}

/*--------------------------------------------------------------------*/
/*! Read the status of the general input lines.
 * 
 * \param dev   K-Net Device Descriptor
 * \param channel The channel number to read
 * 
 * \returns <0 on error or the input value on sucess 
 */
int koa_readInput(knet_dev_t * dev, int  channel)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "Y,%d\r\n", channel)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'y' )
    return -1;

  scan = buf+2;
  scan = koa_get_nextValue(scan,val);
  return atoi(val);
}

/*--------------------------------------------------------------------*/
/*! Read the 10 bit value corresponding to the channel_number analog input. The
 * value 1024 corresponds to an analog value of 4,09 Volts.
 *
 * \param dev   K-Net Device Descriptor
 * \param channel The channel number to read
 * 
 * \returns <0 on error or the A/D value on sucess 
 */
int koa_readAD(knet_dev_t * dev, int  channel)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "I,%d\r\n", channel)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'i' )
    return -1;

  scan = buf+2;
  scan = koa_get_nextValue(scan,val);
  return atoi(val);
}

/*--------------------------------------------------------------------*/
/*! Set the Speed and the acceleration for the trapezoidal speed shape of the
 * position controller. The max_speed parameter indicates the maximal speed
 * reached during the displacement. The unit for the speed is the pulse/10ms
 * that corresponds to 4.5 mm/s. The unit for the acceleration is the
 * ((pulse/256)/10 ms)/10 ms, that correspond to 1.758 mm/s2. At the reset,
 * these parameters are set to standard values: max_speed to 20, acc to 64.
 *
 * \param dev   K-Net Device Descriptor
 * \param max_speed_left Maximum speed for the left motor
 * \param acc_left Acceleration for the left motor
 * \param max_speed_right Maximum speed for the right motor
 * \param acc_right Acceleration for the right motor
 * 
 * \returns <=0 on error or >0 on sucess 
 */
int koa_setProfile( knet_dev_t * dev, int max_speed_left, int acc_left, int max_speed_right, int acc_right)
{
  char buf[32];
  int rc;

  if ((rc=knet_printf( dev ,"J,%d,%d,%d,%d\r\n",max_speed_left,acc_left,max_speed_right,acc_right)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'j' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*! Read the status of the motion controller. The status is given by three
 * numbers for every motor: T (target), M (mode) and E (error). T=0 means that
 * the robot is still on movement. T=1 means that the robot is on the target
 * position. M=0 means that the current displacement is controlled in the
 * position mode. M=1 means that the current displacement is controlled in the
 * speed mode. E indicates controller position or speed error.
 *
 * \param dev   K-Net Device Descriptor
 * \param statustable Array of 6 short unsigned to store the status flags
 * \returns <=0 on error or >0 on sucess 
 */
int koa_readStatus(knet_dev_t * dev, int * statustable)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "K\r\n" )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'k' )
    return 0;

  scan = buf+2;
  while(scan) {
    scan       = koa_get_nextValue(scan,val);
    *statustable = atoi(val);
    statustable++;
  }
  
  return 1;
}

/*--------------------------------------------------------------------*/
/*! Perform an action on one of the two LEDs of the robot. Possible actions
 * are: 0: turn OFF, 1: turn ON, 2: change status. The LED number 0 is the
 * lateral one, the LED number 1 is the frontal one.
 *
 * \param dev   K-Net Device Descriptor
 * \param led The led number to change
 * \param action to be done
 * \returns <=0 on error or >0 on sucess 
 */
int koa_changeLed( knet_dev_t * dev , short int led, short int action )
{
  char buf[32];
  int rc;

  if ((rc=knet_printf( dev , "L,%d,%d\r\n" , led , action)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'l' )
    return 0;

  return 1;
}


/*--------------------------------------------------------------------*/
/*! Read the value measured on one of the management sensors. Available sensors
 * are: 0:BATTERY VOLTAGE (measure unit: 20 mV). 1: GENERAL CONSUMPTION CURRENT
 * (measure unit: 8 mA). 2: AMBIENT TEMPERATURE (measure unit: 0.1° C). 3: LEFT
 * MOTOR CURRENT (measure unit: 4 mA). 4: RIGHT MOTOR CURRENT (measure unit: 4
 * mA). 5: BATTERY TEMPERATURE (measure unit: 0.1° C).
 *
 * \param dev   K-Net Device Descriptor
 * \param sensor The sensor number to read
 * 
 * \returns <0 on error or the sensor value on sucess 
 */
int koa_readSensor(knet_dev_t * dev, int  sensor)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "I,%d\r\n", sensor)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'i' )
    return -1;

  scan = buf+2;
  scan = koa_get_nextValue(scan,val);
  return atoi(val);
}

/*--------------------------------------------------------------------*/
/*! This function changes the speed of the left & right motor of the
 *  Koala robot. Set the speed of the two motors. The unit is the pulse/10 ms
 *  that corres-ponds to 4.5 millimetres per second.
 *
 * \param dev   K-Net Device Descriptor
 * \param left  Left Motor Speed.  (range [-127..127])
 * \param right Right Motor Speed. (range [-127..127])
 * \returns <=0 on error or >0 on sucess 
 *
 * \remark A speed of '0' stops the motor. 
 */
int koa_setSpeed( knet_dev_t * dev , short int left , short int right )
{
  char buf[32];
  int rc;

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
/*!
 * Read the charge level of the battery. The number returned is the charge
 * expressed in mAh.
 *
 * \param dev   K-Net Device Descriptor
 * 
 * \returns <0 on error or the charge level on sucess 
 */
int koa_readBattery(knet_dev_t * dev)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "S\r\n")) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 's' )
    return -1;

  scan = buf+2;
  scan = koa_get_nextValue(scan,val);
  return atoi(val);
}

/*--------------------------------------------------------------------*/
/*!
 * Read the koala ambient liht sensor value. The 16 values are returned
 * within the senstable, which must be allocated by the caller.
 * 
 * \param dev   K-Net Device Descriptor
 * \param senstable An array of 16 integer to store the sensor values
 *
 * \returns <=0 on error or >0 on sucess 
 */
int koa_readAmbient(knet_dev_t * dev, int * senstable)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "O\r\n" )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'o' )
    return 0;

  scan = buf+2;
  while(scan) {
    scan       = koa_get_nextValue(scan,val);
    *senstable = atoi(val);
    senstable++;
  }
  
  return 1;
}

/*--------------------------------------------------------------------*/
/*!
 * Set the desired PWM amplitude (see  Motors and motor control  on page 9 for
 * more details) on the two motors. The minimum PWM ratio is 0 (0%). The
 * maximal forward ratio (100%) correspond to a value of 255. The maximal
 * backwards ratio (100%) correspond to a value of -255.
 *
 * \param dev   K-Net Device Descriptor
 * \param left  Left Motor PWM.  (range [-255..255])
 * \param right Right Motor PWM. (range [-255..255])
 * \returns <=0 on error or >0 on sucess 
 */
int koa_setPWM( knet_dev_t * dev , int left , int right )
{
  char buf[32];
  int rc;

  if ((rc=knet_printf( dev , "P,%d,%d\r\n" , left , right )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'p' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*! 
 * Perform an action on one of the general output lines of the robot. Possible
 * actions are: 0: turn OFF, 1: turn ON, 2: change status. Outputs 0 to 7 are
 * the open drain outputs. The ON status correspond to the transistor
 * closed.Outputs 8 to 11 are the digital ones.
 * 
 * \param dev   K-Net Device Descriptor
 * \param output output number to change
 * \param action action to perform
 * 
 * \returns <=0 on error or >0 on sucess 
 */
int koa_setOutput(knet_dev_t * dev, int output, int action)
{
  char buf[32];
  int rc;

  if ((rc=knet_printf( dev , "Q,%d,%d\r\n" , output, action)) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'q' )
    return 0;

  return 1;
}

/*--------------------------------------------------------------------*/
/*!
 * Read the koala proximity sensor value. The 16 values are returned
 * within the senstable, which must be allocated by the caller.
 * 
 * \param dev   K-Net Device Descriptor
 * \param senstable An array of 16 integer to store the sensor values
 *
 * \returns <=0 on error or >0 on sucess 
 */
int koa_readProximity(knet_dev_t * dev, int * senstable)
{
  char buf[256];
  char val[8];
  char * scan;
  int rc;

  if ((rc=knet_printf( dev , "N\r\n" )) < 0 )
    return rc;

  if ((rc=knet_read_string( dev , buf , sizeof(buf)-1 , '\n' )) < 0 )
    return rc;

  /* bad acknowledge */
  if ( buf[0] != 'n' )
    return 0;

  scan = buf+2;
  while(scan) {
    scan       = koa_get_nextValue(scan,val);
    *senstable = atoi(val);
    senstable++;
  }
  
  return 1;
}
