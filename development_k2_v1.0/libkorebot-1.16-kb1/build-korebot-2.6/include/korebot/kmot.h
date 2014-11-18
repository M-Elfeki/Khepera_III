/*--------------------------------------------------------------------
 * kmot.h - KoreBot Library - KoreMotor Support 
 *--------------------------------------------------------------------
 * $Id: kmot.h,v 1.8 2007/04/05 08:12:27 flambercy Exp $
 *--------------------------------------------------------------------
 * Author: Alexandre Colot & Pierre Bureau
 *         heavy modifications by Yves Piguet to abstract from the 
 *         I2C access layer
 * $Author: flambercy $
 * $Date: 2007/04/05 08:12:27 $
 * $Revision: 1.8 $
 *--------------------------------------------------------------------*/

#ifndef __kmot__
#define __kmot__

#ifdef __cplusplus
extern "C" {
#endif

#define KMOT_VERSION(ver)   ((ver>>4)&0xf)
#define KMOT_REVISION(ver)  (ver&0xf)

/*--------------------------------------------------------------------
 *!  KoreMotor Order Mask (See knet_set_order)
 */
#define KMOT_ORDER_MASK ( KNET_ORDER_LITTLE | \
                         KNET_ORDER_REP_ADR | KNET_ORDER_MSB_FIRST )

/*--------------------------------------------------------------------
 *! KoreMotor Registers Definition 
 */
#define  MOT_FWVersion          0x00 /*! Firmware Revision Version */ 

#define  MOT_Mode		0x28
#define  MOT_SetPointSource	0x29
#define  MOT_HWOptions		0x2A
#define  MOT_SWOptions		0x2B
#define  MOT_ControlTyp		0x2C
#define  MOT_ErrorFlags		0x2D
#define  MOT_StatusFlags	0x2E

#define  MOT_SetPointLL		0x2F
#define  MOT_SetPointLH		0x30
#define  MOT_SetPointHL		0x31
#define  MOT_SetPointHH		0x32

#define  MOT_Filter		0x33

#define  MOT_PositionLL		0x34
#define  MOT_PositionLH		0x35
#define  MOT_PositionHL		0x36
#define  MOT_PositionHH		0x37

#define  MOT_SpeedLL		0x38
#define  MOT_SpeedHL		0x39

#ifdef NEW_KMOT_FIRMWARE
#define  MOT_SpeedLH		0x3A
#define  MOT_SpeedHH		0x3B

#define  MOT_TorqueL		0x5A
#define  MOT_TorqueH		0x5B

#else
#define  MOT_TorqueL		0x3A
#define  MOT_TorqueH		0x3B
#endif

#define  MOT_TorqueBiasL	0x3C
#define  MOT_TorqueBiasH	0x3D

#define  MOT_KpSpeedL		0x3E
#define  MOT_KpSpeedH		0x3F

#define  MOT_KdSpeedL		0x40
#define  MOT_KdSpeedH		0x41

#define  MOT_KiSpeedL		0x42
#define  MOT_KiSpeedH		0x43

#define  MOT_SampleTimeL	0x44
#define  MOT_SampleTimeH	0x45

#define  MOT_BlockedTime	0x46
#define  MOT_IntGenPeriod	0x47
#define  MOT_IntGenAmplitude	0x48
#define  MOT_IntGenOffset	0x49

#define  MOT_SoftStopMinLL	0x4A
#define  MOT_SoftStopMinLH	0x4B
#define  MOT_SoftStopMinHL	0x4C
#define  MOT_SoftStopMinHH	0x4D

#define  MOT_SoftStopMaxLL	0x4E
#define  MOT_SoftStopMaxLH	0x4F
#define  MOT_SoftStopMaxHL	0x50
#define  MOT_SoftStopMaxHH	0x51

#define  MOT_Acceleration	0x52 /*! Acceleration for trapezoidal */ 
				     /*! speed profile (0..255)       */
#define  MOT_Unused		0x53
#define  MOT_StaticFriction	0x54 /*! Static Friction of the       */
                                     /*! system (0..255)              */
#define  MOT_HWCurrentLimit	0x55 /*! Generate PWM for current     */
                                     /*! limiting with RC-filter      */
                                     /*! [U = 5V / 255 * CurrentLimit]*/
#define  MOT_SWCurrentLimitL	0x56 /*! Soft. current limit (0..127) */
#define	 MOT_SWCurrentLimitH	0x57 /*! Temporary register usable in */
				     /*! the Interrup Service Routine */
#define	 MOT_MinSampleTimeH	0x59 /*! Time used to pass one cycle  */
#define  MOT_MinSampleTimeL	0x58

#define  MOT_PID_OUTH		0x5D
#define  MOT_PID_OUTL		0x5C

#define  MOT_MinSpeedL          0x5E
#define  MOT_MinSpeedH          0x5F

#define  MOT_NearTargetMargin	0x60

#define  MOT_KpPosL		0x61
#define  MOT_KpPosH		0x62

#define  MOT_KdPosL		0x63
#define  MOT_KdPosH		0x64

#define  MOT_KiPosL		0x65
#define  MOT_KiPosH		0x66

#define  MOT_KpTorqueL		0x67
#define  MOT_KpTorqueH		0x68

#define  MOT_KdTorqueL		0x69
#define  MOT_KdTorqueH		0x6A

#define  MOT_KiTorqueL		0x6B
#define  MOT_KiTorqueH		0x6C

#define  MOT_MaxSpeedL		0x6D
#define  MOT_MaxSpeedH		0x6E

#define  MOT_Prescale           0x70

#define  MOT_SpeedMultL		0x71
#define  MOT_SpeedMultH 	0x72


#define  MOT_VelocityPrescaler	0x73


/*--------------------------------------------------------------------
 *! KoreMotor Modes 
 */
enum kMotMode
{
  kMotModeIdle        = 0 , /*! Idle                        */
  kMotModeNormal      = 1 , /*! Normal Control Mode         */
  kMotModeStopMotor   = 2 , /*! Stop Motor                  */
  kMotModeSleep       = 3 , /*! Sleep                       */
  kMotModeReset       = 4 , /*! Reset                       */
  kMotModeSaveE2PROM  = 5 , /*! Save Config Param in E2PROM */
  kMotModeSearchLimit = 6   /*! Search Limits               */
};
    
/*--------------------------------------------------------------------
 *! KoreMotor Regulation Types 
 */
enum kMotRegType 
{
  kMotRegOpenLoop     = 0 , /*! Open Loop        */
  kMotRegPos          = 1 , /*! Position         */
  kMotRegPosProfile   = 2 , /*! Position Profile */
  kMotRegSpeed        = 3 , /*! Speed            */
  kMotRegSpeedProfile = 4 , /*! Speed Profile    */
  kMotRegCurrent      = 5 , /*! Current          */
  kMotRegTorque       = 5 , /*! Torque           */
  kMotRegZeroFriction = 6   /*! not implemented  */
};

/*--------------------------------------------------------------------
 *! KoreMotor Measurement Types
 */
enum kMotMesureType 
{
  kMotMesPos     = kMotRegPos ,     /*! Position */
  kMotMesSpeed   = kMotRegSpeed ,   /*! Speed    */
  kMotMesCurrent = kMotRegCurrent , /*! Current  */
  kMotMesTorque  = kMotRegTorque    /*! Torque   */
};

/*--------------------------------------------------------------------
 *! KoreMotor Target Point Sources 
 */
enum kMotPointSource
{
  kMotPtSrcExtI2C    = 0 , /*! External I²C                         */
  kMotPtSrcExtAnalog = 1 , /*! External Analog - not implemented    */
  kMotPtSrcSquare    = 2 , /*! Internal Square Wave Generator       */
  kMotPtSrcTriangle  = 3 , /*! Internal Triangle Generator          */
  kMotPtSrcSinus     = 4   /*! Internal Sinus Gen. - not implemented*/
};

/*--------------------------------------------------------------------
 *! KoreMotor Hardware Options
 */
enum kMotHWOptions
{
  kMotHWOptIdle        = (0<<0) , /*! Idle Mode             */
  kMotHWOptNormal      = (1<<0) , /*! Normal Control Mode   */
  kMotHWOptAnSetPtInEn = (1<<1) , /*! Analog SetPoint Input */
  kMotHWOptLed         = (1<<2) , /*! Led (0=Dis,1=En)      */
  kMotHWOptEncRes4x    = (0<<3) , /*! Encoder 4x Resolution */
  kMotHWOptEncRes1x    = (1<<3) , /*! Encoder 1x Resolution */
  kMotHWOptTorqueInv   = (1<<4) , /*! Torque Inversion      */
  kMotHWOptDriverOpt1  = (1<<5) , /*! Driver Option 1       */
  kMotHWOptDriverOpt2  = (1<<6) , /*! Driver Option 2       */
  kMotHWOptDriverOpt3  = (1<<7)   /*! Driver Option 3       */
};

/*--------------------------------------------------------------------
 *! KoreMotor Software Options
 */
enum kMotSWOptions
{
  kMotSWOptSepD                 = (1<<0) , /*! Seperate D              */
  kMotSWOptWindup               = (1<<1) , /*! Antireset Windup        */
  kMotSWOptSoftStopMin          = (1<<2) , /*! SoftStop MIN            */
  kMotSWOptSoftStopMax          = (1<<3) , /*! SoftStop MAX            */
  kMotSWOptSoftStopErr          = (1<<4) , /*! Error on SoftStop       */
  kMotSWOptStopMotorBlk         = (1<<5) , /*! Stop Motor when blocked */
  kMotSWOptCurrentCtrl          = (1<<6) , /*! Current Control by Soft.*/
  kMotSWOptDirectionInv         = (1<<7)   /*! Direction Inversion     */  
};

/*--------------------------------------------------------------------
 *! KoreMotor Error Flags
 */
enum kMotErrorFlags
{
  kMotErrorSampleTimeTooSmall = (1<<0) , /*! Sample Time Too Small  */
  kMotErrorWDTOverflow        = (1<<1) , /*! WatchDog Timer Overflow*/
  kMotErrorBrownOut           = (1<<2) , /*! Brown-Out              */
  kMotErrorSoftStopMotor      = (1<<3) , /*! Software stopped Motor */
  kMotErrorMotorBlocked       = (1<<4) , /*! Motor Blocked          */
  kMotErrorPosOutOfRange      = (1<<5) , /*! Position Out Of Range  */
  kMotErrorSpeedOutOfRange    = (1<<6) , /*! Speed Out Of Range     */
  kMotErrorTorqueOutOfRange   = (1<<7)   /*! Torque Out Of Range    */
};

/*--------------------------------------------------------------------
 *! KoreMotor Status Flags
 */
enum kMotStatusFlags
{
  kMotStatusMoveDet         = (1<<0) , /*! Movement detected             */
  kMotStatusDir             = (1<<1) , /*! Direction (0:Neg, 1:Pos)      */
  kMotStatusOnSetPt         = (1<<2) , /*! On Set Point                  */
  kMotStatusNearSetPt       = (1<<3) , /*! Near Set Point                */
  kMotStatusCmdSat          = (1<<4) , /*! Command Saturated             */
  kMotStatusWindup          = (1<<5) , /*! Antireset Windup              */
  kMotStatusSoftCurCtrl     = (1<<6) , /*! Sofware Current Control Active*/
  kMotStatusSoftStop        = (1<<7)   /*! Software Stop Active          */
};

/*--------------------------------------------------------------------
 * KoreMotor Public Function Prototype Declarations
 */

extern void kmot_GetFWVersion( knet_dev_t * dev , 
			       unsigned int * version );

extern void kmot_GetStatus( knet_dev_t * dev , 
			    unsigned char * status , 
			    unsigned char * error );

extern void kmot_GetOptions( knet_dev_t * dev , 
		     unsigned char * software, 
		     unsigned char * hardware);

extern void kmot_ConfigurePID( knet_dev_t * dev , 
			       int regtype ,
			       int16_t Kp , 
			       int16_t Kd , 
			       int16_t Ki );

extern void kmot_SetPointSource( knet_dev_t * dev , 
				 int regtype , 
				 int wavetype ,
				 int period , 
				 int amplitude , 
				 int offset );

extern long kmot_GetMeasure( knet_dev_t * dev , 
			     int regtype );

extern void kmot_SetPosition( knet_dev_t * dev , 
			      long position );

extern void kmot_SetSpeedProfile( knet_dev_t * dev , 
				  int maxspeed , 
				  int acceleration );

extern void kmot_SetLimits( knet_dev_t * dev , 
			    int regtype , 
			    long softStopMin , 
			    long softStopMax );

extern int kmot_SearchLimits( knet_dev_t * dev , 
			      int8_t blockedTime , 
			      int32_t setPoint , 
			      int32_t * minpos ,
			      int32_t * maxpos ,
			      unsigned int timeout );

extern void kmot_SetOptions( knet_dev_t * dev , 
			     int hwOptions , 
			     int swOptions );

extern void kmot_SetPoint( knet_dev_t * dev , 
			   int regtype , 
			   long setPoint );

extern void kmot_SaveConfig( knet_dev_t * dev );

extern void kmot_ResetError( knet_dev_t * dev );

extern void kmot_SetBlockedTime( knet_dev_t * dev , int time );

extern void kmot_SetSampleTime( knet_dev_t * dev , int sample );

extern void kmot_SetFilterOrder( knet_dev_t * dev, int order);

extern void kmot_SetMode( knet_dev_t * dev , int mode );

extern void kmot_SetMargin( knet_dev_t * dev , int margin );

extern void kmot_GetSpeedMultiplier( knet_dev_t * dev ,  unsigned short *mult );

extern void kmot_SetSpeedMultiplier( knet_dev_t * dev , int mode );

extern void kmot_SetPrescale( knet_dev_t * dev , int mode );

extern void kmot_SetVelocityPrescale( knet_dev_t * dev, int mode );

#ifdef __cplusplus
}
#endif

#endif
