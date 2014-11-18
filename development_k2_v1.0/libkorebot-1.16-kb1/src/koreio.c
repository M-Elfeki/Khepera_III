/*--------------------------------------------------------------------
 * koreio.c - KoreBot Library - KoreIO Support
 *--------------------------------------------------------------------
 * $Id: koreio.c,v 1.5 2007/04/05 08:12:27 flambercy Exp $
 *--------------------------------------------------------------------	
 * 
 * $Author: flambercy $
 * $date: 2005/03/02 16:11:35 $
 * $Revision: 1.5 $
 *         
 * $Author: flambercy $
 * $Date: 2007/04/05 08:12:27 $
 * $Revision: 1.5 $
 *--------------------------------------------------------------------*/

#include "korebot.h"

/*! 
 * \file   koreio.c Support for the KoreIO board.             
 *
 * \brief 
 *         This module provides useful API to use a 
 *         KoreIO from a KoreBot program.
 *
 * \author   Pierre Bureau (K-Team SA)
 *           Frédéric Lambercy (K-Team SA)
 * 
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

/*--------------------------------------------------------------------*/
/*! 
 * This function get the firmware version and revision number.
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param version  A pointer to a variable that will receive the 
 *                 firmware version number.
 */
void kio_GetFWVersion( knet_dev_t * dev , 
			unsigned int * version )
{
  unsigned char val;

  knet_read8( dev , KIO_FWVersion, &val );
  *version  = val;
}


/*--------------------------------------------------------------------*/
/*! 
 * Read an Analog input level. The 10 bit value, as well as the corresponding
 * timestamp are returned. The timestamp is given is mS since the last reset of
 * the timer.
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param an	   The analog input to read
 * \param value	   Pointer to store the 10bit value from the A/D
 * \param time     Pointer to store the 32bit value of the timestamp
 *
 * \return 
 * 	- 0 if succesful
 * 	- -1 if the given Analog is invalid
 * 	- -2 in case of knet transfer failure
 */
int kio_ReadAnalog(knet_dev_t * dev, unsigned int an, uint16_t *value, uint32_t *time)
{
  char addr;
  char val;
  char buf[6];
  char * ptr;
  char i;
  
  if(an > 11)
    return -1;

  addr = KIO_ANReadBase + (an * 6);

  i = 0;
  while(i < 6)
  {
  	if(knet_read8(dev , addr, &buf[i]) < 0)
	  	return -2;
	i++;
	addr++;
  }

  ptr = (char*)value;
  ptr[0] = buf[1];
  ptr[1] = buf[0];

  ptr = (char*)time;
  ptr[0] = buf[5];
  ptr[1] = buf[4];
  ptr[2] = buf[3];
  ptr[3] = buf[2];

  return 0;
}




/*--------------------------------------------------------------------*/
/*!
 * Read the Can message and erase it. Return the number of other 
 * Can messages available.
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param id       Pointer to store the 32bit value of the identifier
 * \param len      Pointer to store the Data length of the message
 * \param can_data1  Pointer to store the 4 Bytes high of Data
 * \param can_data2  Pointer to store the 4 Bytes low of Data
 * \param status   Pointer to store the Status bits of the message
 * \param time     Pointer to store the 4 Bytes of the message timing
 * 
 * \return 
 * 	- n_mess   The number of available messages if succesful
 * 	- -1 if the buffer is empty
 * 	- -2 in case of knet transfer failure
 */
int kio_ReadCAN(knet_dev_t * dev, uint32_t *id, char *len, uint32_t *can_data1, uint32_t *can_data2, char *can_status, uint32_t *time)
{
  char n_mess;
  char addr;
  char val;
  char buf[19];
  char * ptr;
  int i;

  addr = KIO_CANReadBase;

  if (knet_lltransfer( dev, &addr, 1, buf, 19) < 0)
     return -2;
  if(buf[0] == 0)
    return -1;
  
  
  n_mess = buf[0] - 1;

  ptr = (char*)id;
  ptr[0] = buf[1];
  ptr[1] = buf[2];
  ptr[2] = buf[3];
  ptr[3] = buf[4];

  ptr = (char*)len;
  ptr[0] = buf[5];
 
  ptr = (char*)can_data1; 
  ptr[0] = buf[9];
  ptr[1] = buf[8];
  ptr[2] = buf[7];
  ptr[3] = buf[6];

  ptr = (char*)can_data2;
  ptr[0] = buf[13];
  ptr[1] = buf[12];
  ptr[2] = buf[11];
  ptr[3] = buf[10];

  ptr = (char*)can_status;
  ptr[0] = buf[14];

  ptr = (char*)time;
  ptr[0] = buf[15];
  ptr[1] = buf[16];
  ptr[2] = buf[17];
  ptr[3] = buf[18];

  addr = KIO_ClearCAN;  
  knet_write8(dev, addr, 0);

  return n_mess; 

}

/*--------------------------------------------------------------------*/
/*!
 * Send a CAN message on the CAN bus
 *
 * \param dev	   K-Net Device Descriptor to the KoreIO
 * \param id	   Can identifier on 32 Bits
 * \param can_data1  4 Data high
 * \param can_data2  4 Data low
 * \param len	   Number of Bytes to send on the Bus
 * \param status   Set the CAN control bits 
 *
 * \return
 * 	- 0 if succesful
 * 	- 1 if fail
 */

int kio_SendCAN(knet_dev_t * dev, uint32_t id, uint32_t can_data1, uint32_t can_data2, char len, char can_status)
{
  char addr;
  char val;
  char temp;
  
  if (len > 8)
    len = 8;

  addr = KIO_CANWriteBase;
  
  temp = id >> 24;
  knet_write8(dev, addr, temp);
  temp = id >> 16;
  knet_write8(dev, addr, temp);
  temp = id >> 8;
  knet_write8(dev, addr, temp);
  temp = id;
  knet_write8(dev, addr, temp);  

  addr++;

  temp = can_data1 >> 24;
  knet_write8(dev, addr, temp);
  temp = can_data1 >> 16;
  knet_write8(dev, addr, temp);
  temp = can_data1 >> 8;
  knet_write8(dev, addr, temp);
  temp = can_data1;
  knet_write8(dev, addr, temp);

  temp = can_data2 >> 24;
  knet_write8(dev, addr, temp);
  temp = can_data2 >> 16;
  knet_write8(dev, addr, temp);
  temp = can_data2 >> 8;
  knet_write8(dev, addr, temp);
  temp = can_data2;
  knet_write8(dev, addr, temp);

  addr+= 2;

  
  knet_write8(dev, addr, can_status);
  addr--;

  knet_write8(dev, addr, len);

  addr = KIO_Status;
  knet_read8(dev,addr,&val);

  return val;
  
 
}


/*--------------------------------------------------------------------*/
/*!
 * Change a analog output value
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param an 	   the output to set (0-7)
 * \param value	   the analog value (0-255)
 *
 * \return
 * 	- 0 if succesful
 * 	- -1 if the given output is invalid
 */

int kio_SetANValue(knet_dev_t * dev, unsigned int an, unsigned int value)
{
  char addr;

  if(an > 7)
    return -1;

  addr = KIO_ANWriteBase + an;

  knet_write8(dev, addr, value);

  return 0;
}
  
 
/*--------------------------------------------------------------------*/
/*! 
 * Configure a digital IO to be used as an input, output or PWM ouput
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param io       the io to set (0-15)
 * \param config   the mode to set (0: input)
 *                                 (1: ouput)
 *                                 (2: pwm)
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given io is invalid
 *       - -2 if the given config is invalid
 */

int kio_ConfigIO(knet_dev_t * dev, unsigned io, unsigned config)
{
  if(io > 15)
    return -1;

  switch(config)
  {
    case 0 :
      knet_write8(dev , KIO_IOConfigIn, io);
      break;
    case 1 :
      knet_write8(dev , KIO_IOConfigOut, io);
      break;
    case 2 :
      knet_write8(dev , KIO_IOConfigPwm, io);
      break;
    default:
      return -2;
      break;
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Read a digital IO state 
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param io       the io to read (0-15)
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given io is invalid
 *       - -2 if the given io is not in input mode
 */
int kio_ReadIO(knet_dev_t * dev, unsigned int io)
{
  char val;
 
  if(io > 15)
    return -1;

  knet_read8(dev , KIO_IOReadBase + io, &val);

  if(val == 2)
    return -2;
  else
    return val;
}

/*--------------------------------------------------------------------*/
/*! 
 * Set a digital IO state to 1
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param io       the io to set (0-15)
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given io is invalid
 *       - -2 if the given io is not in output mode
 */
int kio_SetIO(knet_dev_t * dev, unsigned int io)
{
  if(io > 15)
    return -1;

  knet_write8(dev , KIO_IOSetBase, io);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Change the state of the LED
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param state    The LED state is 0 to switch it off 
 *                                  1 to switch it on
 *                                  2 to changed it
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given state is invalid
 */
int kio_ChangeLed(knet_dev_t * dev, unsigned state)
{
  char addr;
  
  if(state > 2)
    return -1;

  addr = KIO_IOChgLed + state;
  knet_write8(dev , addr, 16);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Change the state of a digital IO 
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param io       the io to reset (0-15)
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given io is invalid
 *       - -2 if the given io is not in output mode
 */
int kio_ChangeIO(knet_dev_t * dev, unsigned int io)
{
  if(io > 15)
    return -1;

  knet_write8(dev , KIO_IOChgBase, io);

  return 0;
}
/*--------------------------------------------------------------------*/
/*! 
 * Set a digital IO state to 0
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param io       the io to reset (0-15)
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given io is invalid
 *       - -2 if the given io is not in output mode
 */
int kio_ClearIO(knet_dev_t * dev, unsigned int io)
{
  if(io > 15)
    return -1;

  knet_write8(dev , KIO_IOClearBase, io);

  return 0;
}

/*-------------------------------------------------------------------*/
/*!
 * Change PWM channel ratio
 *
 * \param dev      K-Net Device Descriptor to the KoreIo
 * \param io 	   the io to modify PWM ratio 
 * \parama ratio   the ratio value
 *
 * \return 
 * 	- 0 if succesful
 * 	- -1 if the given io is invalid 
 */
int kio_ChangePWM_ratio(knet_dev_t * dev, unsigned int io, unsigned int ratio)
{
  char addr;
  
  if (io > 15)
    return -1;
  
  addr = KIO_PWMRatio + io;
  knet_write8(dev, addr,  ratio);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Change PWM frequency
 *
 * \param dev	   K-Net Device Descriptor to the KoreIo
 * \param freq	   Frequency of the PWM
 *
 * \return	
 * 	- 0 if succesful
 * 	- -1 if fail
 */
int kio_ChangePWM_freq(knet_dev_t * dev, uint16_t freq)
{
  char addr;
  char temp;

  addr = KIO_FreqBase;
	
  temp = freq;
  knet_write8(dev, addr, temp);

  addr++;
  temp = freq >> 8;
  knet_write8(dev, addr, temp);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Change the state of a power output 
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param io       the power output to set (0-6)
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given pw is invalid
 */
int kio_ChangePW(knet_dev_t * dev, unsigned int io)
{
  if(io > 6)
    return -1;

  knet_write8(dev , KIO_PWChgBase, io);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Set a power output state to 1
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param io       the power output to set (0-6)
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given pw is invalid
 */
int kio_SetPW(knet_dev_t * dev, unsigned int io)
{
  if(io > 6)
    return -1;

  knet_write8(dev , KIO_PWSetBase, io);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Set a power output state to 0
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param io       the power output to reset (0-6)
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given pw is invalid
 */
int kio_ClearPW(knet_dev_t * dev, unsigned int io)
{
  if(io > 6)
    return -1;

  knet_write8(dev , KIO_PWClearBase, io);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Handle the koreio timer for analog measurements timestamp.
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param action   The action to perform with the timer
 *                     0: Reset the timer
 *                     1: Stop the timer
 *                     2: Launch the timer
 * 
 * \return 
 * 	 - 0 if succesful
 *       - -1 if the given action is invalid
 */
int kio_Timer(knet_dev_t * dev, unsigned action)
{
  if(action > 2)
    return -1;

  knet_write8(dev , KIO_TimerBase, action);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * Start an address scan on the secondary I2C bus.
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 */
void kio_i2c_StartScan(knet_dev_t * dev)
{
  knet_write8(dev, KIO_I2CScan, 1);
}

/*--------------------------------------------------------------------*/
/*! 
 * Return the list of answering devices from a scan on the secondary I2C bus.
 * The scan must be started using kio_i2c_StartScan.
 *
 * \param dev      K-Net Device Descriptor to the KoreIO
 * \param list     A pointer to an array of characters to store the list. 
 *                 The array must be large enough to store all the addesses (up
 *                 to 128bytes).
 *
 * \return the number of answering devices                
 *      - -1 if the given pointer is null
 *      - -2 in case of knet transfer failure
 */
int kio_i2c_ListScan(knet_dev_t * dev, char * list)
{
  unsigned char val;
  char addr;
  int len;

  if(!list)
    return -1;
  

  knet_read8( dev , KIO_I2C_ScanRead, &val );
  len  = val;
  addr = KIO_I2CList;

  if (knet_lltransfer( dev, &addr, 1, list, len) < 0)
    return -2;
  
  return val;
}

/*----------------------------------------------------------------------*/
/*! 
 * Start an register read on the secondary I2C bus.
 *
 * \param dev     K-Net Device Descriptor to the KoreIO
 * \param device  Device address 
 * \param reg	  Device register address
 * \param n_read  Number of read (max 4)
 *
 */
void kio_i2c_StartRead(knet_dev_t * dev, char device, char reg, char n_read)
{
  char addr;

  addr = KIO_I2CReadBase +1;
  knet_write8(dev, addr, device);

  if(n_read > 4)
    n_read = 4;
  addr++;
  knet_write8(dev, addr, n_read);

  knet_write8(dev, KIO_I2CReadBase, reg);
  
}

/*----------------------------------------------------------------------*/
/*! 
 * return the values of the previous I2C Read bus.
 *
 * \param dev     K-Net Device Descriptor to the KoreIO
 * \param n_read  Number of returned values (max 4)
 * \param values  Pointer to store the returned values
 *
 * \return
 * 	- 0 if succesful
 * 	- -1 if fail
 *
 */
int kio_i2c_ReturnRead(knet_dev_t * dev, char n_read, uint32_t *values)
{ 
    char addr;
    char buf[4];
    char i;
    char *ptr;

  if (n_read > 4)
    n_read = 4;
  
  addr = KIO_I2CReturnRead;
  if (knet_lltransfer( dev, &addr, 1, buf, n_read) < 0)
    return -2;

  ptr = (char*)values;

  i = 0;
  while(i < n_read)
  {
    ptr[i] = buf[i];
    i++;
  }
  return 0;
} 

/*--------------------------------------------------------------------------*/
/*!
 * Write on the secondary I2C bus.
 *
 * \param dev     K-Net Device Descriptor to the KoreIO
 * \param device  Device address 
 * \param reg     Device register address
 * \param txdata  Data to write on device
 *
 * \return
 * 	- 0 if succesful
 * 	- -1 if fail
 */
int kio_i2c_Write(knet_dev_t * dev, char device, char reg, char txdata)
{
	
  char addr;

  addr = KIO_I2CWriteAddr;
  
  knet_write8(dev, addr, device);

  addr++;
  knet_write8(dev, addr, reg);

  addr = KIO_I2CWriteBase;
  knet_write8(dev, addr, txdata);

}
