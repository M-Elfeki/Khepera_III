/*! 
 * \file   khepera3.c Khepera3 layer              
 *
 * \brief 
 *         This module is layer for communication with various devices 
 *         which are part of the khepera3 robot. It offers simple interface 
 *         to the user.
 *        
 * \author   Arnaud Maye (K-Team SA)                               
 *
 * \note     Copyright (C) 2005 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#include "korebot.h"

//#define KH3_UNDER_16

/*!
 * handles to the various khepera3 devices
 */
static knet_dev_t *dspic;
static knet_dev_t *mot1;
static knet_dev_t *mot2;


/*! kh3_init initializes some things like the GPIO40 pin.
 * This function needs to be called BEFORE any other functions.
 * 
 * \param none
 *
 * \return A value:
 *       - <0 on error 
 *       - 0 on success 
 *
 */
int kh3_init( void )
{
 int rc;

 /* First of all this function initializes the korebot library */
 
 if((rc = kb_init( 0 , NULL )) < 0 )
 {
 	/* Unable to initialize the korebot library */
	KB_ERROR("kb_kh3_init",KB_ERROR_KH3KBINIT);
	return KH3_ERROR_KBINIT;
 }
 
 /* Then the function Configures the KNET_INT0 pin as it should be configured */
 
 /* Init gpio module */
 kb_gpio_init();

 /* Set KNET_INT0 pin as a gpio */
 kb_gpio_function(KNET_INT0,0);

 /* Set KNET_INT0 pin as an input */
 kb_gpio_dir(KNET_INT0, IN);
		 
} 





/*!
 * kh3_getcommand gets a command frame from a given khepera3 device.
 * 
 * Function flow:
 * - a) :  retrieve the first byte which is the frame size from the device
 * - b) :  retrieve the required bytes
 *  
 * Normally an end user don't want to use these function as they are
 * assumed as "low level functions".
 * 
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 * \param out is a pointer to a buffer where the command frame will be stored on.
 *
 * \return A value:
 *       - <0 on error (KH3_ERROR_FRMSZERR, KH3_ERROR_SZFMTERR)
 *       - >=0 on success (returns should be the size of frame) 
 *
 * \remark This function requires that kb_kh3_init has been called
 */
int kh3_getcommand( knet_dev_t *hDev, unsigned char *out )
{
 char sizeMsg;
 int rc;

 if( knet_llread( hDev, &sizeMsg, 1 ) == 1 ) 
 {
 	rc = knet_llread( hDev, out, sizeMsg );
	if(rc == sizeMsg)
		return rc;
	else
	{
		KB_ERROR("knet_getCommand", KB_ERROR_KH3FRMSZERR, rc, sizeMsg);
		return KH3_ERROR_FRMSZ; 
	}
 }
 else 
 {
 	 KB_ERROR("knet_getCommand", KB_ERROR_KH3SZFMTERR);
	 return KH3_ERROR_SZFMT;
 }
}


/*!
 * kh3_sendcommand sets a command frame to a given khepera3 device.
 *
 * Normally and end user don't want to use these function as they are
 * assumed as "low level functions".
 *   
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 * \param in is a pointer to a buffer where the command frame to be sent is stored on.
 *
 * \return A value:
 *       - <0 on error (KH3_ERROR_FRMSNDERR)
 *       - >=0 on success (returns should be the size of frame)
 *
 * \remark This function requires that kb_kh3_init has been called
 */
int kh3_sendcommand( knet_dev_t *hDev, unsigned char *in )
{
 char sizeMsg;

 /* first byte in the frame is the complete frame size */
 sizeMsg = in[0];
	
 if( knet_llwrite( hDev, in, sizeMsg) == sizeMsg) 
 	return sizeMsg;
 else
 {
 	KB_ERROR("knet_sendCommand", KB_ERROR_KH3FRMSNDERR);
	return KH3_ERROR_FRMSND;
 }
}



/*!
 * kh3_proximity_ir retrieves an instant IR measure.
 *
 * \param outbuf is a buffer where the data will be stored on.
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 *
 *
 * \return NULL or a pointer to the IR measure
 */
int kh3_proximity_ir(char *outbuf, knet_dev_t *hDev)
{

  int rc , i;
 
  /* Frame format : { Size, Command, Terminator } 
   * where the command can be more than 1 byte */
  char cmd[3] = { 2, 'N', 0};

  if(hDev)
  {
 
 	kh3_sendcommand( hDev , cmd );
#ifdef KH3_UNDER_16
  	/* delay to ensure the correct reading of KNET_INT0 pin */	
	usleep(K3_CMD_DELAY);
	while(!kb_gpio_get(KNET_INT0));
#endif
	rc = kh3_getcommand( hDev, outbuf );
	
	return rc;
	
  }
 
  return 0;
 
}

/*!
 * kh3_ambiant_ir retrieves an instant IR measure.
 *
 * \param outbuf is a buffer where the data will be stored on.
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 *
 * \return NULL or a pointer to the IR measure.
 *
 */
int kh3_ambiant_ir(char *outbuf, knet_dev_t *hDev)
{
  int rc;

  /* Frame format : { Size, Command, Terminator }
   * where the command can be more than 1 byte */
  char cmd[3] = { 2, 'O', 0};

  if(hDev)
  {
        kh3_sendcommand( hDev , cmd );
#ifdef KH3_UNDER_16 
 	/* delay to ensure the correct reading of KNET_INT0 pin */	
	usleep(K3_CMD_DELAY);

	while(!kb_gpio_get(KNET_INT0));
#endif
	rc = kh3_getcommand( hDev, outbuf );

	return rc;
  }

  return 0;

}

/*!
 * kh3_battery_Voltage retrieves the actual battery voltage.
 *
 * \param outbuf is a buffer where the data will be stored on.
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 *
 * \return NULL or a pointer to the battery voltage.
 *
 */
int kh3_battery_voltage(char *outbuf, unsigned char argument, knet_dev_t *hDev)
{
  int rc;

  /* Frame format : { Size, Command, Terminator }
   * where the command can be more than 1 byte */
  char cmd[4] = { 3, 'V', 0, 0};
  cmd[2] = argument;

  if(hDev)
  {
	kh3_sendcommand( hDev , cmd );
#ifdef KH3_UNDER_16
  	/* delay to ensure the correct reading of KNET_INT0 pin */	
	usleep(K3_CMD_DELAY);
	while(!kb_gpio_get(KNET_INT0));
#endif	

	rc = kh3_getcommand( hDev, outbuf );
	return rc;
  }

  return 0;

}

/*!
 * kh3_reset_tstamp resets the absolute timestamp.
 *
 * \param outbuf is a buffer where the data will be stored on.
 *  in this case the only answer to expect is z, which is a ack.
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 * 
 *
 * \return NULL or a pointer to the ack.
 *
 */
int kh3_reset_tstamp(char *outbuf, knet_dev_t *hDev)
{
  int rc;

  /* Frame format : { Size, Command, Terminator }
   * where the command can be more than 1 byte */
  char cmd[3] = { 2, 'Z', 0};

  if(hDev)
  {
	kh3_sendcommand( hDev , cmd );
#ifdef KH3_UNDER_16
  	/* delay to ensure the correct reading of KNET_INT0 pin */	
	usleep(K3_CMD_DELAY);

	while(!kb_gpio_get(KNET_INT0));
#endif
	rc = kh3_getcommand( hDev, outbuf );

	return rc;
  }

  return 0;

}

/*!
 * kh3_revision retrieves the current OS version/revision
 *
 * \param outbuf is a buffer where the data will be stored on.
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 *
 * \return NULL or a pointer to the revision data
 */
int kh3_revision(char *outbuf, knet_dev_t *hDev)
{
  int rc;

  /* Frame format : { Size, Command, Terminator }
   * where the command can be more than 1 byte */
  char cmd[3] = { 2, 'B', 0};

  if(hDev)
  {
	kh3_sendcommand( hDev , cmd );
#ifdef KH3_UNDER_16
  	/* delay to ensure the correct reading of KNET_INT0 pin */	
	usleep(K3_CMD_DELAY);
	
	while(!kb_gpio_get(KNET_INT0));
#endif
	rc = kh3_getcommand( hDev, outbuf );

	return rc;
  }
  return 0;

}

/*!
 * kh3_configure configures the current firmware operation mode.
 * a configuration array is used by the khepera3 to decide its mode
 * of operation.
 *
 * \param outbuf is a buffer where the data will be stored on.
 * \param index is the index pointing one of the configuration word in the config array.
 * \param value is the value to store in the configuration array.
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 * 
 * Indexes:
 *
 * - 0 : Value that tells on which us transciever to work from
 *       This is a bit field where bit0 means trx 1 and so on
 *       So to work on the two most left transceiver one should
 *       Set 0b0000000000000011 => 3.
 *       Note: default value is 4 to work from only the front us trx.
 * - 1 : Value that decide what is the maximum echo number
 *       Note: default value is 3.
 * - 2 : Value that tell how many pulse can be seen without the echo
 *       is detected. This is required because the korebot may generate
 *       some noise.
 *       A received echo means many 40kHz pulse so this is why we can
 *       filter noise that way.
 *       Note: default value is 5.
 *                 
 * \return NULL or a pointer to the output
 *
 */
int kh3_configure_os(char *outbuf, unsigned char index, unsigned char value, knet_dev_t *hDev)
{
  int rc;

  /* Frame format : { Size, Command, arg1, arg2, Terminator }
   * where the command can be more than 1 byte */
  char cmd[6] = { 5, 'C', 0, 0, 0, 0};

  cmd[2] = index;
  cmd[4] = value;

  if(hDev)
  {
	kh3_sendcommand( hDev , cmd );
#ifdef KH3_UNDER_16
      	/* delay to ensure the correct reading of KNET_INT0 pin */	
	usleep(K3_CMD_DELAY);

	while(!kb_gpio_get(KNET_INT0));
#endif
	rc = kh3_getcommand( hDev, outbuf );
        return rc;
  }
  return 0;

}

/*!
 * kh3_measure_us retrieves measure from a given US transmitter. 
 *
 * \param outbuf is a buffer where the data will be stored on.
 * \param usnbr is a number of the us trx to read from ( 1 to 5 ).
 * \param hDev is a handle to an openned knet socket (Khepera3:dsPic).
 * 
 * \return NULL or a pointer to the ultrasonic measure
 */
int kh3_measure_us(char *outbuf, unsigned char usnbr, knet_dev_t *hDev)
{
  int rc;

  /* Frame format : { Size, Command, arg1, arg2, Terminator }
   * where the command can be more than 1 byte */
  char cmd[4] = { 3, 'G', 0, 0};

  cmd[2] = usnbr;
  if(hDev)
  {
 	kh3_sendcommand( hDev , cmd );
#ifdef KH3_UNDER_16
	/* delay to ensure the correct reading of KNET_INT0 pin */	
	usleep(K3_CMD_DELAY);

	while(!kb_gpio_get(KNET_INT0));
#endif
	rc = kh3_getcommand( hDev, outbuf );
	
	return rc;
  }
  return 0;

}
