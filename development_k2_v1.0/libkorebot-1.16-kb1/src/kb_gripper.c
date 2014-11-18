/*! 
 * \file   khepera3.c Khepera3 Gripper layer              
 *
 * \brief 
 *         This module is layer for communication with the kheperaIII Gripper. It offers simple interface 
 *         to the user.
 *        
 * \author   Frederic Lambercy (K-Team SA)                               
 *
 * \note     Copyright (C) 2010 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#include "korebot.h"


/*! kgripper_init initializes the KoreBot library
 * This function needs to be called BEFORE any other functions.
 * 
 * \param none
 *
 * \return A value:
 *       - <0 on error 
 *       - 0 on success 
 *
 */
int kgripper_init( void )
{
 int rc;

 /* First of all this function initializes the korebot library */
 
 if((rc = kb_init( 0 , NULL )) < 0 )
 {
 	/* Unable to initialize the korebot library */
	KB_ERROR("kb_kh3_init",KB_ERROR_KH3KBINIT);
	return KH3_ERROR_KBINIT;
 }
}
/************************ ARM ****************************************/
/********************** READ FUNCTION ********************************/

/*--------------------------------------------------------------------*/
/*! 
 * This function return the revision of the Arm Gripper OS
 *
 * \param dev    	K-Net Device Descriptor
 * \return OSVersion 	Version (4 MSB) and Revision (4 LSB) of the arm Gripper (Ex: 0xA1)
 *

 */ 
unsigned char kgripper_Arm_Get_Version( knet_dev_t * dev )
{
  unsigned char OSVersion;
  knet_read8( dev , ARM_VERSION , &OSVersion );

  return OSVersion;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the actual Position of the Gripper Arm
 *
 * \param dev    	K-Net Device Descriptor
 * \return Position 	Position Value from 300 (Arm in high position) to 900 (Arm on the ground)
 *			These values depend on the mechanical limit of the system. Read the Max_position and Min_position 
 *			parameters to know the limit position
 *

 */ 
unsigned short kgripper_Arm_Get_Position( knet_dev_t * dev )
{
  unsigned short Position;
  knet_read16( dev , ARM_POSITION , &Position );

  return Position;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the actual Speed of the Gripper Arm
 *
 * \param dev    	K-Net Device Descriptor
 * \return Speed 	Speed Value, positiv if the arm goes move downwards and negativ if the arm mount
 *

 */ 
unsigned char kgripper_Arm_Get_Speed( knet_dev_t * dev )
{
  unsigned char Speed;
  knet_read8( dev , ARM_SPEED , &Speed );

  return Speed;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the actual Current of the Gripper Arm
 *
 * \param dev    	K-Net Device Descriptor
 * \return Current 	Position Value from 0 (= 0 mA) to 1023 (= 2000mA)
 *
 */ 
unsigned short kgripper_Arm_Get_Current( knet_dev_t * dev )
{
  unsigned short Current;
  knet_read16( dev , ARM_CURRENT , &Current );

  return Current;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return if the arm is on the target or not
 *
 * \param dev    	K-Net Device Descriptor
 * \return OnTarget 	Position Flag (1 = Arm on target, 0 = Arm in movement)
 *

 */ 
unsigned char kgripper_Arm_OnTarget( knet_dev_t * dev )
{
  unsigned char OnTarget;
  knet_read8( dev , ARM_ON_TARGET , &OnTarget );

  return OnTarget;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the Battery Voltage
 *
 * \param dev    	K-Net Device Descriptor
 * \return Voltage 	Voltage of the gripper Battery (Vbat = value / 102.4) Ex: 758 = 7.4V
 *
 */ 
unsigned short kgripper_Arm_Get_Voltage( knet_dev_t * dev )
{
  unsigned short Voltage;
  knet_read16( dev , ARM_VOLTAGE , &Voltage );

  return Voltage;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the remaining capacity of the battery
 *
 * \param dev    	K-Net Device Descriptor
 * \return Capacity 	Remaining capacity of the battery calculate on the voltage (100% if > 8.2V, 0% if < 6.44V)
 *

 */ 
unsigned char kgripper_Arm_Get_Capacity( knet_dev_t * dev )
{
  unsigned char Capacity;
  knet_read8( dev , ARM_CAPACITY , &Capacity );

  return Capacity;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the mechanical limit of the gripper
 *
 * \param dev    	K-Net Device Descriptor
 * \param Max_Position 	Maximal position of the arm (ground)
 * \param Min_Position  Minimal position of the arm (position up)
 *

 */ 
void kgripper_Arm_Get_Limits( knet_dev_t * dev , 
		     unsigned short * Min_Position , 
		     unsigned short * Max_Position )
{

   if (Min_Position != NULL )
    knet_read16( dev , ARM_MIN_POSITION ,  Min_Position );
  
  if ( Max_Position != NULL )
    knet_read16( dev , ARM_MAX_POSITION ,  Max_Position );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the Position order of the Arm
 *
 * \param dev    	K-Net Device Descriptor
 * \return order 	Position Order that the arm must reach
 *
 */ 
unsigned short kgripper_Arm_Get_Order( knet_dev_t * dev )
{
  unsigned short Order;
  knet_read16( dev , ARM_ORDER , &Order );

  return Order;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the maximal speed of the arm
 *
 * \param dev    	K-Net Device Descriptor
 * \return Max_Speed 	Maximal speed reach by the arm when moving
 *

 */ 
unsigned char kgripper_Arm_Get_Max_Speed( knet_dev_t * dev )
{
  unsigned char Max_Speed;
  knet_read8( dev , ARM_MAX_SPEED , &Max_Speed );

  return Max_Speed;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the search limit flag
 *
 * \param dev    	K-Net Device Descriptor
 * \return Search_Limit	Flag that indicate if the arm is performing a search of the mechanical limit. Reset when finished.
 *

 */ 
unsigned char kgripper_Arm_Get_Search_Limit( knet_dev_t * dev )
{
  unsigned char Search_Limit;
  knet_read8( dev , ARM_SEARCH_LIMIT , &Search_Limit );

  return Search_Limit;
  
}

/********************** WRITE FUNCTION ********************************/
/*--------------------------------------------------------------------*/
/*! 
 * This function set a new Position order to reach. If the value is out of the mechanical limit, the gripper will limit itself automatically
 *
 * \param dev    	K-Net Device Descriptor
 * \param order 	Position Order that the arm must reach
 *
 */ 
void kgripper_Arm_Set_Order( knet_dev_t * dev, unsigned short Order )
{
  knet_set_order( dev ,  KGRIPPER_ORDER_MASK );
  knet_write16( dev , ARM_ORDER , Order );



}

/*--------------------------------------------------------------------*/
/*! 
 * This function set the maximum speed that the arm can reach
 *
 * \param dev    	K-Net Device Descriptor
 * \param Max_Speed 	Maximal speed from 0 to 15 (default = 5)
 *
 */ 
void kgripper_Arm_Set_Max_Speed( knet_dev_t * dev, unsigned char Max_Speed )
{
  if(Max_Speed > 15)
    Max_Speed = 15;
  knet_write8( dev , ARM_MAX_SPEED , Max_Speed );

}

/*--------------------------------------------------------------------*/
/*! 
 * This function start (=1) or stop (=0) the search limit procedure
 *
 * \param dev    	K-Net Device Descriptor
 * \param Search_Limit 	Flag to start the search limit procedure. reset automatically when the procedure is done
 *
 */ 
void kgripper_Arm_Set_Search_Limit( knet_dev_t * dev, unsigned char Search_Limit )
{
  if(Search_Limit > 1)
    Search_Limit = 1;
  knet_write8( dev , ARM_SEARCH_LIMIT , Search_Limit );

}


/*************************** GRIPPER *********************************/
/********************** READ FUNCTION ********************************/
/*--------------------------------------------------------------------*/
/*! 
 * This function return the revision of the head Gripper OS
 *
 * \param dev    	K-Net Device Descriptor
 * \return OSVersion 	Version (4 MSB) and Revision (4 LSB) of the Gripper (Ex: 0xA1)
 *

 */ 
unsigned char kgripper_Gripper_Get_Version( knet_dev_t * dev )
{
  unsigned char OSVersion;
  knet_read8( dev , GRIPPER_VERSION , &OSVersion );

  return OSVersion;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the actual Position of the Gripper finger
 *
 * \param dev    	K-Net Device Descriptor
 * \return Position 	Position Value from 0 (Gripper closed) to ~200 (Gripper open)
 *			These values depend on the mechanical limit of the system. Read the Max_position
 *			parameters to know the limit position
 *

 */ 
unsigned char kgripper_Gripper_Get_Position( knet_dev_t * dev )
{
  unsigned char Position;
  knet_read8( dev , GRIPPER_POSITION , &Position );

  return Position;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the Position order of the Gripper finger
 *
 * \param dev    	K-Net Device Descriptor
 * \return order 	Position Order that the gripper must reach
 *
 */ 
unsigned char kgripper_Gripper_Get_Order( knet_dev_t * dev )
{
  unsigned char Order;
  knet_read8( dev , GRIPPER_ORDER , &Order );

  return Order;
  
}


/*--------------------------------------------------------------------*/
/*! 
 * This function return the actual Speed of the Gripper finger
 *
 * \param dev    	K-Net Device Descriptor
 * \return Speed 	Speed Value, positiv if the gripper opening and negativ if closing
 *

 */ 
unsigned char kgripper_Gripper_Get_Speed( knet_dev_t * dev )
{
  unsigned char Speed;
  knet_read8( dev , GRIPPER_SPEED , &Speed );

  return Speed;
  
}


/*--------------------------------------------------------------------*/
/*! 
 * This function return the actual Current of the Gripper Finger
 *
 * \param dev    	K-Net Device Descriptor
 * \return Current 	Current Value from 0 (= 0 mA) to 1023 (= 220mA)
 *
 */ 
unsigned short kgripper_Gripper_Get_Current( knet_dev_t * dev )
{
  unsigned short Current;
  knet_read16( dev , GRIPPER_CURRENT , &Current );

  return Current;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the Torque Limit of the Gripper Finger
 *
 * \param dev    	K-Net Device Descriptor
 * \return Torque 	Torque Value from 0 (= 0 mA) to 1023 (= 220mA)
 *
 */ 
unsigned short kgripper_Gripper_Get_Torque( knet_dev_t * dev )
{
  unsigned short Torque;
  knet_read16( dev , GRIPPER_TORQUE , &Torque );

  return Torque;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the Resistivity of the object grip by the Finger
 *
 * \param dev    	K-Net Device Descriptor
 * \return Torque 	Resistivity Value from 0 (= insulator) to 1023 (= conductor)
 *
 */ 
unsigned short kgripper_Gripper_Get_Resistivity( knet_dev_t * dev )
{
  unsigned short Resistivity;
  knet_read16( dev , GRIPPER_RESISTIVITY , &Resistivity );

  return Resistivity;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the ambiant light value of the two IR sensor in the finger
 *
 * \param dev    	K-Net Device Descriptor
 * \param Amb_IR_Left 	Ambiant light value of the sensor in the left finger (0 = no light, 1023 = IR satured of IR light)
 * \param Amb_IR_right 	Ambiant light value of the sensor in the right finger (0 = no light, 1023 = IR satured of IR light)
 *

 */ 
void kgripper_Gripper_Get_Ambiant_IR_Light( knet_dev_t * dev , 
		     unsigned short * Amb_IR_Left , 
		     unsigned short * Amb_IR_Right )
{

   if (Amb_IR_Left != NULL )
    knet_read16( dev , GRIPPER_AMB_IR_LEFT ,  Amb_IR_Left );
  
  if ( Amb_IR_Right != NULL )
    knet_read16( dev , GRIPPER_AMB_IR_RIGHT ,  Amb_IR_Right );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return the Distance value measured by the two IR sensor in the finger
 *
 * \param dev    	K-Net Device Descriptor
 * \param Dist_IR_Left 	Distance value of the sensor in the left finger (0 = no object, 1023 = Object very close from the left finger)
 * \param Dist_IR_right	Distance value of the sensor in the right finger (0 = no object, 1023 = Object very close from the right finger)
 *

 */ 
void kgripper_Gripper_Get_Distance_Sensors( knet_dev_t * dev , 
		     unsigned short * Dist_IR_Left , 
		     unsigned short * Dist_IR_Right )
{

   if (Dist_IR_Left != NULL )
    knet_read16( dev , GRIPPER_DIST_IR_LEFT ,  Dist_IR_Left );
  
  if ( Dist_IR_Right != NULL )
    knet_read16( dev , GRIPPER_DIST_IR_RIGHT ,  Dist_IR_Right );
}


/*--------------------------------------------------------------------*/
/*! 
 * This function return if the Gripper Finger is on the target or not
 *
 * \param dev    	K-Net Device Descriptor
 * \return OnTarget 	Position Flag (1 = Gripper on target, 0 = Gripper in movement)
 *

 */ 
unsigned char kgripper_Gripper_OnTarget( knet_dev_t * dev )
{
  unsigned char OnTarget;
  knet_read8( dev , GRIPPER_ON_TARGET , &OnTarget );

  return OnTarget;
  
}

/*--------------------------------------------------------------------*/
/*! 
 * This function return if an object is between the two finger (use the optical barrier)
 *
 * \param dev    	K-Net Device Descriptor
 * \return Object 	Presence of an object flag (0 = no object, 1 = object detected)
 *

 */ 
unsigned char kgripper_Gripper_Object_Detected( knet_dev_t * dev )
{
  unsigned char Object;
  knet_read8( dev , GRIPPER_OPTICAL_BARR , &Object );

  return Object;
  
}


/*--------------------------------------------------------------------*/
/*! 
 * This function return the search limit flag of the gripper finger
 *
 * \param dev    	K-Net Device Descriptor
 * \return Search_Limit	Flag that indicate if the gripper is performing a search of the mechanical limit. Reset when finished.
 *

 */ 
unsigned char kgripper_Gripper_Get_Search_Limit( knet_dev_t * dev )
{
  unsigned char Search_Limit;
  knet_read8( dev , GRIPPER_SEARCH_LIMIT , &Search_Limit );

  return Search_Limit;
  
}
/*--------------------------------------------------------------------*/
/*! 
 * This function return the mechanical limit of the gripper finger
 *
 * \param dev    	K-Net Device Descriptor
 * \param Max_Position 	Maximal position of the Gripper (Open)
 *

 */ 
unsigned char kgripper_Gripper_Get_Limits( knet_dev_t * dev )
{
  unsigned char Max_Position;
  
  knet_read8( dev , GRIPPER_MAX_POSITION ,  &Max_Position );

  return Max_Position;
}

/********************** WRITE FUNCTION ********************************/
/*--------------------------------------------------------------------*/
/*! 
 * This function set a new Position order to reach. If the value is out of the mechanical limit, the gripper will limit itself automatically
 *
 * \param dev    	K-Net Device Descriptor
 * \param order 	Position Order that the gripper finger must reach
 *
 */ 
void kgripper_Gripper_Set_Order( knet_dev_t * dev, unsigned char Order )
{
  knet_write8( dev , GRIPPER_ORDER , Order );
}

/*--------------------------------------------------------------------*/
/*! 
 * This function set the maximum Torque that the gripper use to grip object
 *
 * \param dev    	K-Net Device Descriptor
 * \param Max_Torque 	Maximal Torque from 100 to 700 (default = 400)
 *
 */ 
void kgripper_Gripper_Set_Torque( knet_dev_t * dev, unsigned short Torque )
{
  knet_set_order( dev ,  KGRIPPER_ORDER_MASK );
  knet_write16( dev , GRIPPER_TORQUE , Torque );

}

/*--------------------------------------------------------------------*/
/*! 
 * This function start (=1) or stop (=0) the search limit procedure of the gripper
 *
 * \param dev    	K-Net Device Descriptor
 * \param Search_Limit 	Flag to start the search limit procedure. reset automatically when the procedure is done
 *
 */ 
void kgripper_GripperSet_Search_Limit( knet_dev_t * dev, unsigned char Search_Limit )
{
  if(Search_Limit > 1)
    Search_Limit = 1;
  knet_write8( dev , GRIPPER_SEARCH_LIMIT , Search_Limit );

}





