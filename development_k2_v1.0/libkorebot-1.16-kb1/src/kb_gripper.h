/*--------------------------------------------------------------------
 * kb_gripper.h - KoreBot Library - Khepera3 Gripper functions
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Date: 2010/02/05 
 * $Revision: 1.0 $
 *-------------------------------------------------------------------*/ 

#ifndef __kgripper__
#define __kgripper__

#include "knet.h"

/*--------------------------------------------------------------------
 *!  KoreMotor Order Mask (See knet_set_order)
 */
#define KGRIPPER_ORDER_MASK ( KNET_ORDER_LITTLE | \
                         KNET_ORDER_REP_ADR | KNET_ORDER_LSB_FIRST )

/*--------------------------------------------------------------------
 *! Gripper Registers Definition 
 *! Arm  
 */
#define	ARM_VERSION		0x30
#define	ARM_POSITION		0x31
#define	ARM_SPEED		0x33
#define	ARM_CURRENT		0x34
#define	ARM_ON_TARGET		0x36
#define	ARM_VOLTAGE		0x37
#define	ARM_CAPACITY		0x39
#define	ARM_MAX_POSITION	0x3A
#define	ARM_MIN_POSITION	0x3C
#define	ARM_ORDER		0x3E
#define	ARM_MAX_SPEED		0x40
#define	ARM_SEARCH_LIMIT	0x41

/*--------------------------------------------------------------------
 *! Gripper Registers Definition 
 *! Gripper
 */
#define	GRIPPER_VERSION		0x30
#define	GRIPPER_POSITION	0x31
#define	GRIPPER_ORDER		0x32
#define	GRIPPER_SPEED		0x33
#define	GRIPPER_CURRENT		0x34
#define	GRIPPER_TORQUE		0x36
#define	GRIPPER_RESISTIVITY     0x38
#define	GRIPPER_AMB_IR_LEFT     0x3A
#define	GRIPPER_AMB_IR_RIGHT    0x3C
#define	GRIPPER_DIST_IR_LEFT    0x3E
#define	GRIPPER_DIST_IR_RIGHT   0x40
#define	GRIPPER_OPTICAL_BARR    0x42
#define	GRIPPER_ON_TARGET	0x43
#define	GRIPPER_SEARCH_LIMIT	0x44
#define	GRIPPER_MAX_POSITION	0x45



/*--------------------------------------------------------------------
 *! Prototypes Declaration
 */
extern int kgripper_init( void );
extern unsigned char kgripper_Arm_Get_Version( knet_dev_t * dev );
extern unsigned short kgripper_Arm_Get_Position( knet_dev_t * dev );
extern unsigned char kgripper_Arm_Get_Speed( knet_dev_t * dev );
extern unsigned short kgripper_Arm_Get_Current( knet_dev_t * dev );
extern unsigned char kgripper_Arm_OnTarget( knet_dev_t * dev );
extern unsigned short kgripper_Arm_Get_Voltage( knet_dev_t * dev );
extern unsigned char kgripper_Arm_Get_Capacity( knet_dev_t * dev );
extern void kgripper_Arm_Get_Limits( knet_dev_t * dev , unsigned short * Min_Position ,  unsigned short * Max_Position );
extern unsigned short kgripper_Arm_Get_Order( knet_dev_t * dev );
extern unsigned char kgripper_Arm_Get_Max_Speed( knet_dev_t * dev );
extern unsigned char kgripper_Arm_Get_Search_Limit( knet_dev_t * dev );
extern void kgripper_Arm_Set_Order( knet_dev_t * dev, unsigned short Order );
extern void kgripper_Arm_Set_Max_Speed( knet_dev_t * dev, unsigned char Max_Speed );
extern void kgripper_Arm_Set_Search_Limit( knet_dev_t * dev, unsigned char Search_Limit );

extern unsigned char kgripper_Gripper_Get_Version( knet_dev_t * dev );
extern unsigned char kgripper_Gripper_Get_Position( knet_dev_t * dev );
extern unsigned char kgripper_Gripper_Get_Order( knet_dev_t * dev );
extern unsigned char kgripper_Gripper_Get_Speed( knet_dev_t * dev );
extern unsigned short kgripper_Gripper_Get_Current( knet_dev_t * dev );
extern unsigned short kgripper_Gripper_Get_Torque( knet_dev_t * dev );
extern unsigned short kgripper_Gripper_Get_Resistivity( knet_dev_t * dev );
extern void kgripper_Gripper_Get_Ambiant_IR_Light( knet_dev_t * dev , unsigned short * Amb_IR_Left , unsigned short * Amb_IR_Right );
extern void kgripper_Gripper_Get_Distance_Sensors( knet_dev_t * dev , unsigned short * Dist_IR_Left , unsigned short * Dist_IR_Right );
extern unsigned char kgripper_Gripper_OnTarget( knet_dev_t * dev );
extern unsigned char kgripper_Gripper_Object_Detected( knet_dev_t * dev );
extern unsigned char kgripper_Gripper_Get_Search_Limit( knet_dev_t * dev );
extern unsigned char kgripper_Gripper_Get_Limits( knet_dev_t * dev );
extern void kgripper_Gripper_Set_Order( knet_dev_t * dev, unsigned char Order );
extern void kgripper_Gripper_Set_Torque( knet_dev_t * dev, unsigned short Torque );
extern void kgripper_GripperSet_Search_Limit( knet_dev_t * dev, unsigned char Search_Limit );

#endif /* __khepera3__ */
