/*--------------------------------------------------------------------
 * koreio.c - KoreBot Library - KoreIO Support
 *--------------------------------------------------------------------
 * $Id: koreio.h,v 1.3 2005/03/02 15:12:13 lambercy Exp $
 *--------------------------------------------------------------------	
 *        
 * $Author: lambercy $
 * $Date: 2005/03/02 15:12:13 $
 * $Revision: 1.3 $
 *         
 * $Author: lambercy $
 * $Date: 2005/03/02 15:12:13 $
 * $Revision: 1.3 $
 *--------------------------------------------------------------------*/


#ifndef __koreio__
#define __koreio__

#define KIO_FWVersion	0x00
#define KIO_IOReadBase	0x01
#define KIO_PWMRatio	0x01
#define KIO_ANReadBase  0x11
#define KIO_ANWriteBase 0x11
#define KIO_IOConfigIn	0x19
#define KIO_IOConfigOut	0x1A
#define KIO_IOConfigPwm	0x1B
#define KIO_IOClearBase	0x1C
#define KIO_IOSetBase	0x1D
#define KIO_IOChgBase	0x1E
#define KIO_IOChgLed	0x1C
#define KIO_FreqBase	0x1F
#define KIO_ClearCAN	0x21
#define KIO_PWClearBase	0x22
#define KIO_PWSetBase	0x23
#define KIO_PWChgBase   0x24
#define KIO_TimerBase	0x25
#define KIO_CANWriteBase 0x26
#define KIO_I2CWriteBase 0x32
#define KIO_I2CReadBase 0x33
#define KIO_I2CReturnRead 0x6C
#define KIO_I2CScan	0x37
#define KIO_I2CWriteAddr 0x38
#define KIO_I2C_ScanRead 0x70
#define KIO_I2CList     0x71
#define KIO_CANReadBase 0x59
#define KIO_Status	0xF2

extern void kio_GetFWVersion( knet_dev_t * dev , unsigned int *);
extern int kio_SetIO(knet_dev_t * dev, unsigned int io);
extern int kio_ClearIO(knet_dev_t * dev, unsigned int io);
extern int kio_ReadIO(knet_dev_t * dev, unsigned int io);
extern int kio_ChangeIO(knet_dev_t * dev, unsigned int io);
extern int kio_ReadAnalog(knet_dev_t * dev, unsigned int an, uint16_t *value, uint32_t *time);
extern int kio_ChangeLed(knet_dev_t * dev, unsigned state);
extern int kio_ConfigIO(knet_dev_t * dev, unsigned io, unsigned config);
extern int kio_ChangePWM_ratio(knet_dev_t * dev, unsigned int io, unsigned int ratio);
extern int kio_ChangePWM_freq(knet_dev_t * dev, uint16_t freq);
extern void kio_i2c_StartScan(knet_dev_t * dev);
extern void kio_i2c_StartRead(knet_dev_t *dev,char device, char reg, char n_read);
extern int kio_t2c_ReturnRead(knet_dev_t * dev, char n_read, uint32_t *values);
extern int kio_SetANValue(knet_dev_t * dev, unsigned int an, unsigned int value);
extern int kio_i2c_ListScan(knet_dev_t * dev, char * list);
extern int kio_i2c_Write(knet_dev_t * dev, char device, char reg, char txdata);
extern int kio_ReadCAN(knet_dev_t * dev, uint32_t *id, char *len, uint32_t *can_data1,uint32_t *can_data2 , char *can_status, uint32_t *time);
extern int kio_SendCAN(knet_dev_t * dev, uint32_t id, uint32_t can_data1, uint32_t can_data2, char len, char can_status); 

#endif /* __koreio__ */
