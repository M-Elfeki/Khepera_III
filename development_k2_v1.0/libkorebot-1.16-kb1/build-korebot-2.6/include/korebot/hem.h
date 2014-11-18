/*--------------------------------------------------------------------
 * hem.c - KoreBot Library - Hemission Robot Support for KoreBot 
 *--------------------------------------------------------------------
 * $Id: hem.h,v 1.3 2004/08/18 08:13:45 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Date: 2004/08/18 08:13:45 $
 * $Revision: 1.3 $
 *--------------------------------------------------------------------*/

#ifndef __hem__
#define __hem__

/*--------------------------------------------------------------------*/
/*! Hemission Switch Bitmasks Definition
 */
#define hem_Switch1           (1<<0)
#define hem_Switch2           (1<<1)
#define hem_Switch3           (1<<2)
#define hem_Switch4           (1<<3)

/*--------------------------------------------------------------------*/
/*! Hemission Led Bitmasks Definition
 */
#define hem_LedOnOff          (1<<0) /*! Led On/Off      */
#define hem_LedPgmExec        (1<<1) /*! Led Pgm/Exec    */
#define hem_LedFrontLeft      (1<<2) /*! Led Front Left  */
#define hem_LedFrontRight     (1<<3) /*! Led Front Right */

/*--------------------------------------------------------------------*/
/*! Hemission Infra-Red Sensor Position Definition
 */
#define hem_SensorFront       0 /*! Front                     */
#define hem_SensorFrontRight  1 /*! Front Right               */
#define hem_SensorFrontLeft   2 /*! Front Left                */
#define hem_SensorRight       3 /*! Right                     */
#define hem_SensorLeft        4 /*! Left                      */
#define hem_SensorRear        5 /*! Rear                      */
#define hem_SensorGroundRight 6 /*! Ground Right              */
#define hem_SensorGroundLeft  7 /*! Ground Left               */
#define hem_SensorMax         8 /*! Maximum number of sensors */


/*--------------------------------------------------------------------*/
/*! This function opens a K-Net device  to communicate with the 
 *  Hemisson robot using KoreBot.
 *
 * \returns A Pointer to a K-Net Device Descriptor or NULL.
 */ 
#define hem_open( )           knet_open( "KoreBot:Hemission" )

/*--------------------------------------------------------------------*/
/*! This function closes the K-Net device used to communicate with the
 *  Hemisson robot. 
 */
#define hem_close( dev )      knet_close( dev)

/*--------------------------------------------------------------------*/
/* Public Function Prototypes
 */

extern int hem_getVersion( knet_dev_t * dev );

extern int hem_setSpeed( knet_dev_t * dev , int left , int right );

extern int hem_beep( knet_dev_t * dev , unsigned int state );

extern int hem_readSwitches( knet_dev_t * dev , unsigned int * state );

extern int hem_setLEDs( knet_dev_t * dev , unsigned int state );

extern int hem_readProximitySensors( knet_dev_t * dev , 
				     unsigned char * sensors );

extern int hem_readAmbientLightSensors( knet_dev_t * dev , 
					unsigned char * sensors );

extern int hem_readTVRemote( knet_dev_t * dev , unsigned char * value );

extern int hem_readI2C( knet_dev_t * dev ,
			unsigned char slave ,
			unsigned char reg ,
			unsigned char * data ,
			unsigned char count );

extern int hem_writeI2C( knet_dev_t * dev ,
			 unsigned char slave ,
			 unsigned char reg ,
			 unsigned char * data ,
			 unsigned char count );

extern int hem_fastRead( knet_dev_t * dev ,
			 unsigned char * motorLeft ,
			 unsigned char * motorRight ,
			 unsigned char * proximitySensors ,
			 unsigned char * ambientLightSensors ,
			 unsigned char * switches ,
			 unsigned char * tvRemoteData ,
			 unsigned char * version ,
			 unsigned char * revision );

extern int hem_fastWrite( knet_dev_t * dev ,
			  unsigned char motorLeft ,
			  unsigned char motorRight ,
			  unsigned char ledState );

#endif /* __hem__ */
