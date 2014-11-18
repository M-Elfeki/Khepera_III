/*--------------------------------------------------------------------
 * kmot.c - KoreBot Library - KoreMotor Support
 *--------------------------------------------------------------------
 * $Id: kmot.c,v 1.9 2007/04/05 08:12:27 flambercy Exp $
 *--------------------------------------------------------------------	
 * Author: Alexandre Colot & Pierre Bureau
 *         heavy modifications by Yves Piguet to abstract from the 
 *         I2C access layer
 * $Author: flambercy $
 * $Date: 2007/04/05 08:12:27 $
 * $Revision: 1.9 $
 *--------------------------------------------------------------------*
 * Modifications made by Yves Piguet (27 Feb 2004):
 *
 * - removed K*-style command handler (SpecsTable, Command_Table, 
 *      cmd_handler, test_command_length, SyntaxError
 * - removed led_state
 * - removed scan_and_configure (seems incomplete)
 * - removed main
 * - removed last bits of Kbios- and compiler-specific code 
 *      (#use, #fuse, #device, Fifo, #define OS_*)
 * - removed #include's I don't have (16F876A.h, registers.h, 
 *      fifo.h, i2c.h, rs232.h, knet.h)
 * - removed write_i2c and read_i2c
 * - removed "Special Function available only for KNET"
 * - defined types i2c_io_readbyte_t, i2c_io_writebyte_t, i2c_io_t
 * - defined macros READI2C(i2c, slave, addr) and WRITEI2C(i2c, 
 *      slave, addr, value)
 * - changed get_status, configure_pid, get_measure, set_position, 
 *      set_profile, set_limits, search_limits, set_options, set_point, 
 *      save_config
 * - removed  kmot_SearchLimits, kmot_GetStatus, kmot_ConfigurePID, 
 *      kmot_GetMeasure, kmot_SetCurrentLimits, kmot_SetPositionLimits;
 *	renamed K commands above accordingly and recycled 
 *      documentation comments
 * - changed kmot_ResetError, kmot_SetBlockedTime, kmot_SetSampleTime, 
 *      kmot_SetFilterOrder to use i2c_io_t
 * - removed kmot_I2cRead, kmot_I2cWrite (not used anymore)
 * - changed kmot_I2cRead32 and kmod_I2cRead16 to use i2c_io_t
 * - removed kmot_TestLink
 *
 * $Log: kmot.c,v $
 * Revision 1.9  2007/04/05 08:12:27  flambercy
 * modif knet_i2c.c
 *
 * Revision 1.8  2006/10/27 08:40:58  flambercy
 * Release 1.9
 * KmotLE-knc and KoreioLE.knc added
 * kmot.c modificated for new koremotor firmware
 *
 * Revision 1.7  2006/02/08 13:51:30  amaye
 * *** empty log message ***
 *
 * Revision 1.6  2006/01/23 15:03:34  flambercy
 * *** empty log message ***
 *
 * Revision 1.5  2005/10/25 12:44:56  pbureau
 * New function to read kmot options
 *
 * Revision 1.4  2004/11/12 14:35:23  pbureau
 * Minor corrections
 *
 * Revision 1.3  2004/09/02 14:04:00  cgaudin
 * Minor corrections.
 *
 * Revision 1.2  2004/08/16 20:05:22  cgaudin
 * Corrected bugs in kmot.c
 *
 * Revision 1.1  2004/07/29 10:51:55  cgaudin
 * New libkorebot release 1.2
 *
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*! 
 * \file   kmot.c Support for the KoreMotor board.             
 *
 * \brief 
 *         This module provides useful basic facilities to use a 
 *         KoreMotor.
 *
 * \author   Alexandre Colot (K-Team SA)
 * \author   Pierre Bureau (K-Team SA)
 * \author   Yves Piguet (Calerga)
 * \author   Cédric Gaudin (K-Team SA)                               
 *
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

/*--------------------------------------------------------------------*/
/*! 
 * This function get the firmware version and revision number.
 *
 * \param dev      K-Net Device Descriptor to the corresponding motor
 * \param version  A pointer to a variable that will receive the 
 *                 firmware version number.
 */
void kmot_GetFWVersion( knet_dev_t * dev , 
			unsigned int * version )
{
  unsigned char val;

  knet_read8( dev , MOT_FWVersion, &val );
  *version  = val;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function gets the status flags and error flags. 
 *
 * \param dev    K-Net Device Descriptor
 * \param status Status Flags
 * \param error  Error Flags
 *
 *  Error Flags:
 *
 *  - bit 0 : Sample time too small
 *  - bit 1 : Watchdog timer overflow
 *  - bit 2 : Brown-out
 *  - bit 3 : Software stoped motor (if softstop enabled)
 *  - bit 4 : Motor blocked (if motorblock enabled)
 *  - bit 5 : Position out of range
 *  - bit 6 : Speed out of range
 *  - bit 7 : Torque out of range
 *
 *  Status flags:
 *
 *  - bit 0 : Movement detected
 *  - bit 1 : Direction 0=negative 1=positive
 *  - bit 2 : On setpoint
 *  - bit 3 : Near setpoint
 *  - bit 4 : Command saturated
 *  - bit 5 : Antireset windup active
 *  - bit 6 : Software current control active
 *  - bit 7 : Softstop active
 */ 
void kmot_GetStatus( knet_dev_t * dev , 
		     unsigned char * status , 
		     unsigned char * error )
{
  if ( status != NULL )
    knet_read8( dev , MOT_StatusFlags , status );
  
  if ( error != NULL )
    knet_read8( dev , MOT_ErrorFlags , error );
}



/*--------------------------------------------------------------------*/
/*!
 * This function gets the pid speed multiplier.
 */
void kmot_GetSpeedMultiplier( knet_dev_t * dev ,
                     unsigned short *mult )
{
  unsigned char lsb, msb;
  unsigned char *plsb, *pmsb;
	

  plsb = &lsb;
  pmsb = &msb;

  if ( mult != NULL )
  {
	knet_read8( dev , MOT_SpeedMultL , plsb );
  	knet_read8( dev , MOT_SpeedMultH , pmsb );
	
	*mult = (msb<<8) | lsb;

        printf("kmot_GetSpeedMultiplier : mult = %x, lsb = %x, msb = %x\r\n",
        *mult, lsb, msb);

  }
}
   


/*--------------------------------------------------------------------*/
/*! 
 * This function gets the software options and hardware options. 
 *
 * \param dev    K-Net Device Descriptor
 * \param software Software Options
 * \param hardware Hardware Options
 *
 * Software Options:
 *
 *  - bit 0 : Use alternate algorithm PID derivation.
 *            The derivate part is calculated using the
 *            process variable rather than the error
 *  - bit 1 : Activate the anti reset windup routine
 *  - bit 2 : Stop the motor if the min position is reached
 *  - bit 3 : Stop the motor if the max position is reached
 *  - bit 4 : Generate an error when position is out of
 *            limits, in this case the error must be re-
 *            seted before any further commands can be
 *            executed.
 *  - bit 5 : Stop the motor if the blocked condition is met
 *  - bit 6 : Activate software current limitation (Not implemented)
 *  - bit 7 : Invert the motor direction
 *
 *  Hardware Options:
 *
 *  - bit 0 : Startup mode (0 = idle mode, 1 = control mode)
 *  - bit 1 : Use analog input for setpoint (Not Implemented)
 *  - bit 2 : Not Implemented
 *  - bit 3 : Resolution for the encoder (0 = 100%, 1 = 25%)
 *  - bit 4 : Invert the internal current measurement
 *  - bit 5 : Not Implemented
 *  - bit 6 : Not Implemented
 *  - bit 7 : Not Implemented
 */ 
void kmot_GetOptions( knet_dev_t * dev , 
		     unsigned char * software, 
		     unsigned char * hardware)
{
  if ( hardware != NULL )
    knet_read8( dev , MOT_HWOptions, hardware);
  
  if ( software != NULL )
    knet_read8( dev , MOT_SWOptions, software);
}

/*--------------------------------------------------------------------*/
/*! 
 * This function configures the PID controller using a given 
 * regulation mode. Please refer to the kMotRegType Definition
 *
 * \param dev     K-Net Device Descriptor 
 * \param regtype Type of regulation
 * \param Kp      PID Proportional Constant 
 * \param Kd      PID Differential Constant
 * \param Ki      PID Integral Constant
 * 
 */
void kmot_ConfigurePID( knet_dev_t * dev , 
			int regtype , 
			int16_t Kp , int16_t Kd , int16_t Ki )
{
  unsigned char regs[3];

  switch(regtype) {
    
    /* Position */
  case kMotRegPos:	
    regs[0] = MOT_KpPosL;
    regs[1] = MOT_KdPosL;
    regs[2] = MOT_KiPosL;
    break;
  
  case kMotRegPosProfile:   /* PosProfile   */
  case kMotRegSpeed:        /* Speed        */
  case kMotRegSpeedProfile: /* SpeedProfile */

    regs[0] = MOT_KpSpeedL;
    regs[1] = MOT_KdSpeedL;
    regs[2] = MOT_KiSpeedL;    
    break;
	
  case kMotRegTorque: /* Current */
    regs[0] = MOT_KpTorqueL;
    regs[1] = MOT_KdTorqueL;
    regs[2] = MOT_KiTorqueL;
    break;
  }

  knet_set_order( dev , KMOT_ORDER_MASK );
  
  knet_write16( dev , regs[0] , Kp );
  knet_write16( dev , regs[1] , Kd );
  knet_write16( dev , regs[2] , Ki );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function selects a source for the target point. Usually the 
 * setpoint is given by the application with knet commnands, on some 
 * cases, an internal signal generator can provide a variable setpoint.
 *                       
 * \param dev       K-Net Device Descriptor
 * \param regtype   Type of regluation. See kMotRegType
 * \param wavetype  Target Point Source. See kMotPointSource
 * \param period    Period [0..255]
 * \param amplitude Amplitude [0..255]
 * \param offset    Offset [-128..127]
 */
void kmot_SetPointSource( knet_dev_t * dev , 
			  int regtype , 
			  int wavetype ,
			  int period , 
			  int amplitude , 
			  int offset )
{ 
  knet_write8( dev , MOT_SetPointSource , wavetype );
  knet_write8( dev , MOT_IntGenPeriod , period );
  knet_write8( dev , MOT_IntGenAmplitude , amplitude );
  knet_write8( dev , MOT_IntGenOffset , offset );
  knet_write8( dev , MOT_Mode , 1 );
  knet_write8( dev , MOT_ControlTyp , regtype );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function read a measure from a given motor.
 * 
 * \param dev     K-Net Device Descriptor 
 * \param regtype Type of regulation
 * \return        the measured position or torque depending on 
 *                the type of regulation		
 */
long kmot_GetMeasure( knet_dev_t * dev , int regtype )
{
  short s;
  long m = 0; int rc;

  knet_set_order( dev , KMOT_ORDER_MASK );
  
  switch(regtype) {
  
  default:
  case kMotRegOpenLoop: /* OpenLoop */ 
    m = 0; 
    break;

  case kMotRegPos:        /* Position */
  case kMotRegPosProfile: /* PosProfile */
    rc=knet_read32( dev , MOT_PositionLL , &m );
    break;

#ifdef NEW_KMOT_FIRMWARE
  case kMotRegSpeed:	 /* Speed */
  case kMotRegSpeedProfile:
    rc=knet_read32( dev , MOT_SpeedLL , &m );
    break;
#else
  case kMotRegSpeed:	 /* Speed */
  case kMotRegSpeedProfile:
    knet_read16( dev , MOT_SpeedLL , &s );
    m = (long)s;
    break;
#endif
  case kMotRegTorque:	  
    knet_read16( dev , MOT_TorqueL , &s );
    m = (long)s;
    break;
  }
  
  return m;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function sets the position counter to the given value 
 * for a given motor.
 *
 * \param dev      K-Net Device Descriptor corresponding to the motor
 * \param position New position value 
 * 
 */
void kmot_SetPosition( knet_dev_t * dev , long position )
{
  knet_set_order( dev , KMOT_ORDER_MASK );
  knet_write32( dev , MOT_PositionLL , position );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function sets the profile for position regulation.
 * 
 * The trapezoidal speed profile is calculated using the acceleration 
 * until the maxspeed is reached. -acceleration is used to decelerate 
 * until target position is reached.
 *
 * \param dev          K-Net Device Descriptor corresponding to the motor
 * \param maxspeed     Maximal speed to reach
 * \param acceleration Acceleration
 */
void kmot_SetSpeedProfile( knet_dev_t * dev , 
			   int maxspeed , 
			   int acceleration )
{
  knet_set_order( dev , KMOT_ORDER_MASK );
  knet_write16( dev , MOT_MaxSpeedL , maxspeed );
  knet_write8( dev , MOT_Acceleration , acceleration );
}

/*--------------------------------------------------------------------*/
/*!
 * This function sets the minimum speed for the tapezoidal profile
 *
 * The trapezoidal speed profile start with this minimal value to avoid a too low speed value
 * that the PID won't be able to regulate.
 *
 * \param dev          K-Net Device Descriptor corresponding to the motor
 * \param minspeed     Minimal speed to start the trapeze 
 */
void kmot_SetMinSpeed( knet_dev_t * dev ,
                           int minspeed  )
{
  knet_set_order( dev , KMOT_ORDER_MASK );
  knet_write16( dev , MOT_MinSpeedL , minspeed );
}



/*--------------------------------------------------------------------*/
/*! 
 * This function sets the limits for the given regulation. 
 * 
 * The exact behavior when a measure reaches a limit depends 
 * on the controller options. 
 * 
 * \param dev         K-Net Device Descriptor corresponding to the motor
 * \param regtype     Type of regulation 
 * \param softStopMin Minimal Software Stop Value
 * \param softStopMax Maximal Software Stop Value
 *
 */ 
void kmot_SetLimits( knet_dev_t *dev , 
		     int regtype , 
		     long softStopMin , long softStopMax )
{
  unsigned short v;

  knet_set_order( dev ,  KMOT_ORDER_MASK );

  switch(regtype) {
  
  case kMotRegPos:     
  case kMotRegPosProfile:
    knet_write32( dev , MOT_SoftStopMinLL , softStopMin );
    knet_write32( dev , MOT_SoftStopMaxLL , softStopMax );
    break;

  case kMotRegTorque:
    /* Only MOT_SWCurrentLimitL is used */
    v = (unsigned short)softStopMax;
    knet_write16( dev , MOT_SWCurrentLimitL , v );
    break;

  default:   
    break;
  }
}

/*--------------------------------------------------------------------*/
/*! 
 * This function searches the system mechanical limits. 
 *
 * The blocked time can adjust the test sensitivity and the current 
 * limit used for blocked detection is the one set with 
 * kmot_SetLimits. The given speed is used to perform the test. 
 * The min and max positions are retruned with the minpos and maxpos 
 * parameters. The speed regulator must be properly configured before 
 * calling the search limit routine. This function is blocking and 
 * will not return until the search cycle is completed or until the 
 * number of given retry has been reached.
 *
 * \param dev         K-Net Device Descriptor corresponding to the motor
 * \param blockedTime  
 * \param setPoint
 * \param minpos      Minimal mechanical position
 * \param maxpos      Maximal mechanical position
 * \param timeout     Timeout value in milli-seconds
 * \return A value:
 *            - -1 on timeout
 *            - 0 on success
 */
int kmot_SearchLimits( knet_dev_t * dev , 
		       int8_t blockedTime , 
		       int32_t setPoint , 
		       int32_t * minpos , 
		       int32_t * maxpos , 
		       unsigned int timeout )
{
  kb_time_t t;
  int ok;

  knet_set_order( dev , KMOT_ORDER_MASK );

  knet_write8( dev ,  MOT_BlockedTime , blockedTime );
  knet_write8( dev ,  MOT_ControlTyp , 0 );   // Control in open loop for version KM 6.3 and after

  knet_write32( dev , MOT_SetPointLL , setPoint );
  
  knet_write8( dev , MOT_Mode , kMotModeSearchLimit );
  
  t = kb_getTime() + timeout;

  ok = 0;
  do {
    if ( knet_read( dev , MOT_Mode ) == 0 ) {
      ok = 1;
      break;
    }
    usleep( 10000 );
  }
  while ( kb_getTime() < t ); 
  
  if (!ok) return -1; /* TIMEOUT */
  
  knet_read32( dev , MOT_SoftStopMinLL , (unsigned long *)minpos );
  knet_read32( dev , MOT_SoftStopMaxLL , (unsigned long *)maxpos );

  return 0; /* OK */
}

/*--------------------------------------------------------------------*/
/*! 
 * This function sets the options for the given controller. Each SW 
 * and HW option parameters is a OR field of option flags.
 *
 * \param dev       K-Net Device Descriptor to the corresponding motor
 * \param hwOptions Hardware Option Flags
 * \param swOptions Software Option Flags
 *
 * Software Option Flags:
 *
 * - kMotSWOptSepD         : use alternate algorithm PID derivation
 * - kMotSWOptWindup       : activate the anti reset windup routine
 * - kMotSWOptSoftStopMin  : stop the motor if the min position 
 *                           is reached
 * - kMotSWOptSoftStopMax  : stop the motor if the max position 
 *                           is reached
 * - kMotSwOptStopErr      : generate an error when position is out 
 *                           of limits in this case the error must be 
 *                           rested before any further commands can be 
 *                           executed
 * - kMotSWOptStopMotorBlk : stop the motor if the blocked condition 
 *                           is met (No movement, and current over 
 *                           the limit for the blocked time period).
 * - kMotSWOptCurrentCtrl  : activate software current limitation 
 *                           (Not implemented)
 * - kMotSWOptDirectionInv : invert the motor direction
 *
 * Hardware Option Flags:
 *
 * - kMotHWOptIdle        : Idle Startup Mode 
 * - kMotHWOptNormal      : Normal Control Startup Mode 
 * - kMotHWOptAnSetPtInEn : use analog input for setpoint 
 *                          (Not Implemented)
 * - kMotHWOptLed	  : Not Implemented
 * - kMotHWOptEncRes4x    : 4x 100% resolution for the encoder
 * - kMotHWOptEncRes1x    : 1x  25% resolution for the encoder
 * - kMotHWOptTorqueInv   : invert the internal current measurement	 
 * - kMotHWOptDriverOpt1  : Not Implemented
 * - kMotHWOptDriverOpt2  : Not Implemented
 * - kMotHWOptDriverOpt3  : Not Implemented
 */
void kmot_SetOptions( knet_dev_t *dev , int hwOptions , int swOptions )
{
  knet_write8( dev ,  MOT_HWOptions , hwOptions );
  knet_write8( dev ,  MOT_SWOptions , swOptions );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function sets a given target point for a given motor.
 *
 * \param dev      K-Net Device Descriptor corresponding to the motor
 * \param regtype  Type of regulation
 * \param setPoint
 */
void kmot_SetPoint( knet_dev_t * dev , int regtype , long setPoint )
{
  /*int chgmode = 0;*/

  knet_set_order( dev , KMOT_ORDER_MASK );

  if ( knet_read( dev , MOT_Mode ) != kMotModeNormal ) {
    knet_write8( dev , MOT_Mode , kMotModeNormal );
  }

  /*
    if( knet_read( dev , MOT_ControlTyp ) != regtype ) {
    knet_write8( dev ,  MOT_Mode , kMotModeStopMotor );
    chgmode = 1;
    } 
    else {
    if( knet_read( dev ,  MOT_Mode ) != kMotModeNormal )
    knet_write8( dev ,  MOT_Mode , kMotModeNormal );
    }
  */

  knet_write8( dev ,  MOT_ControlTyp , regtype );
  knet_write32( dev , MOT_SetPointLL , setPoint );
  knet_write8( dev ,  MOT_SetPointSource , kMotPtSrcExtI2C );
  
  /*  
      if( chgmode ) {
      knet_write8( dev , MOT_Mode , kMotModeNormal );
      }
  */
}

/*--------------------------------------------------------------------*/
/*! 
 * This function saves the current configuration for the given motor 
 * to the controller EEPROM. This EEPROM keeps the configuration even 
 * if the controller is switch off. 
 *
 * \param dev  K-Net Device Descriptor corresponding to the motor 
 */
void kmot_SaveConfig( knet_dev_t * dev ) 
{
  knet_write8( dev , MOT_Mode , 0x55 );
  knet_write8( dev , MOT_Mode , 0xAA );
  knet_write8( dev , MOT_Mode , kMotModeSaveE2PROM );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function resets the error register for the given motor, all 
 * controls are disabled while any error is active.
 *
 * \param dev K-Net Device Descriptor corresponding to the motor
 */
void kmot_ResetError( knet_dev_t * dev )
{
  knet_write8( dev , MOT_ErrorFlags , 0 );
}

/*--------------------------------------------------------------------*/
/*! 
 * kmot_SetBlockedTime - 
 *
 * \param dev K-Net Device Descriptor corresponding to the motor
 * \param time 
 */
void kmot_SetBlockedTime( knet_dev_t * dev , int time )
{
  knet_write8( dev , MOT_BlockedTime , time );
}

/*--------------------------------------------------------------------*/
/*! 
 * kmot_SetSampleTime -  
 *
 * \param dev K-Net Device Descriptor corresponding to the motor
 * \param sample
 */
void kmot_SetSampleTime( knet_dev_t * dev , int sample ) 
{
  knet_set_order( dev , KMOT_ORDER_MASK );
  knet_write8( dev , MOT_SampleTimeH , (sample & 0xFF00) >> 8 );
  knet_write8( dev , MOT_SampleTimeL , (sample & 0x00FF));
}

/*--------------------------------------------------------------------*/
/*! 
 * kmot_SetFilterOrder -  
 *
 * \param dev KNet Device Descriptor corresponding to the motor
 e \param order
  didastelmotor_getVersion( 1  , &version );
 */
void kmot_SetFilterOrder( knet_dev_t * dev , int order ) 
{
  knet_write8( dev , MOT_Filter, order );
}

/*--------------------------------------------------------------------*/
/*! 
 * kmot_SetMode -  
 *
 * \param dev KNet Device Descriptor corresponding to the motor
 * \param mode
 */
void kmot_SetMode( knet_dev_t * dev , int mode )
{
  knet_write8( dev , MOT_Mode , mode );
}


/*--------------------------------------------------------------------*/
/*!
 * kmot_SetPrescale -
 *
 * \param dev KNet Device Descriptor corresponding to the motor
 * \param prescale
 */
void kmot_SetPrescale( knet_dev_t * dev , int mode )
{
  knet_write8( dev , MOT_Prescale , mode );
}


/*--------------------------------------------------------------------*/
/*!
 * kmot_SetSpeedMultiplier -
 *
 * \param dev KNet Device Descriptor corresponding to the motor
 * \param prescale
 */
void kmot_SetSpeedMultiplier( knet_dev_t * dev , int mode )
{
  short value;
  char lsb, msb;

  value = mode;

  lsb = value & 0xFF;
  msb = (value >> 8) & 0xFF;

  printf("kmot_SetSpeedMultiplier : mode = %x, value = %x, lsb = %x, msb = %x\r\n",
	mode, value, lsb, msb);
 
  knet_write8( dev , MOT_SpeedMultL , lsb );
  knet_write8( dev , MOT_SpeedMultH , msb );

}





/*--------------------------------------------------------------------*/
/*!
 * kmot_SetVelocityPrescale -
 *
 * \param dev KNet Device Descriptor corresponding to the motor
 * \param prescale
 */

void kmot_SetVelocityPrescale( knet_dev_t * dev, int mode )
{
  knet_write8( dev , MOT_VelocityPrescaler , mode );
}

/*--------------------------------------------------------------------*/
/*! 
 * kmot_SetMargin -  
 *
 * \param dev KNet Device Descriptor corresponding to the motor
 * \param margin
 */
void kmot_SetMargin( knet_dev_t *dev , int margin )
{
  knet_write8( dev , MOT_NearTargetMargin , margin );
}
