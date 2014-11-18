 #include "korebot.h"
 
 
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
 unsigned char kgripper_Arm_Get_Version( knet_dev_t* dev )
 {
   unsigned char OSVersion;
   knet_read8( dev , ARM_VERSION , &OSVersion );
 
   return OSVersion;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned short kgripper_Arm_Get_Position( knet_dev_t * dev )
 {
   unsigned short Position;
   knet_read16( dev , ARM_POSITION , &Position );
 
   return Position;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Arm_Get_Speed( knet_dev_t * dev )
 {
   unsigned char Speed;
   knet_read8( dev , ARM_SPEED , &Speed );
 
   return Speed;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned short kgripper_Arm_Get_Current( knet_dev_t * dev )
 {
   unsigned short Current;
   knet_read16( dev , ARM_CURRENT , &Current );
 
   return Current;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Arm_OnTarget( knet_dev_t * dev )
 {
   unsigned char OnTarget;
   knet_read8( dev , ARM_ON_TARGET , &OnTarget );
 
   return OnTarget;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned short kgripper_Arm_Get_Voltage( knet_dev_t * dev )
 {
   unsigned short Voltage;
   knet_read16( dev , ARM_VOLTAGE , &Voltage );
 
   return Voltage;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Arm_Get_Capacity( knet_dev_t * dev )
 {
   unsigned char Capacity;
   knet_read8( dev , ARM_CAPACITY , &Capacity );
 
   return Capacity;
   
 }
 
 /*--------------------------------------------------------------------*/
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
 unsigned short kgripper_Arm_Get_Order( knet_dev_t * dev )
 {
   unsigned short Order;
   knet_read16( dev , ARM_ORDER , &Order );
 
   return Order;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Arm_Get_Max_Speed( knet_dev_t * dev )
 {
   unsigned char Max_Speed;
   knet_read8( dev , ARM_MAX_SPEED , &Max_Speed );
 
   return Max_Speed;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Arm_Get_Search_Limit( knet_dev_t * dev )
 {
   unsigned char Search_Limit;
   knet_read8( dev , ARM_SEARCH_LIMIT , &Search_Limit );
 
   return Search_Limit;
   
 }
 
 /********************** WRITE FUNCTION ********************************/
 /*--------------------------------------------------------------------*/
 void kgripper_Arm_Set_Order( knet_dev_t * dev, unsigned short Order )
 {
   knet_set_order( dev ,  KGRIPPER_ORDER_MASK );
   knet_write16( dev , ARM_ORDER , Order );
 
 
 
 }
 
 /*--------------------------------------------------------------------*/
 void kgripper_Arm_Set_Max_Speed( knet_dev_t * dev, unsigned char Max_Speed )
 {
   if(Max_Speed > 15)
     Max_Speed = 15;
   knet_write8( dev , ARM_MAX_SPEED , Max_Speed );
 
 }
 
 /*--------------------------------------------------------------------*/
 void kgripper_Arm_Set_Search_Limit( knet_dev_t * dev, unsigned char Search_Limit )
 {
   if(Search_Limit > 1)
     Search_Limit = 1;
   knet_write8( dev , ARM_SEARCH_LIMIT , Search_Limit );
 
 }
 
 
 /*************************** GRIPPER *********************************/
 /********************** READ FUNCTION ********************************/
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Gripper_Get_Version( knet_dev_t * dev )
 {
   unsigned char OSVersion;
   knet_read8( dev , GRIPPER_VERSION , &OSVersion );
 
   return OSVersion;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Gripper_Get_Position( knet_dev_t * dev )
 {
   unsigned char Position;
   knet_read8( dev , GRIPPER_POSITION , &Position );
 
   return Position;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Gripper_Get_Order( knet_dev_t * dev )
 {
   unsigned char Order;
   knet_read8( dev , GRIPPER_ORDER , &Order );
 
   return Order;
   
 }
 
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Gripper_Get_Speed( knet_dev_t * dev )
 {
   unsigned char Speed;
   knet_read8( dev , GRIPPER_SPEED , &Speed );
 
   return Speed;
   
 }
 
 
 /*--------------------------------------------------------------------*/
 unsigned short kgripper_Gripper_Get_Current( knet_dev_t * dev )
 {
   unsigned short Current;
   knet_read16( dev , GRIPPER_CURRENT , &Current );
 
   return Current;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned short kgripper_Gripper_Get_Torque( knet_dev_t * dev )
 {
   unsigned short Torque;
   knet_read16( dev , GRIPPER_TORQUE , &Torque );
 
   return Torque;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned short kgripper_Gripper_Get_Resistivity( knet_dev_t * dev )
 {
   unsigned short Resistivity;
   knet_read16( dev , GRIPPER_RESISTIVITY , &Resistivity );
 
   return Resistivity;
   
 }
 
 /*--------------------------------------------------------------------*/
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
 unsigned char kgripper_Gripper_OnTarget( knet_dev_t * dev )
 {
   unsigned char OnTarget;
   knet_read8( dev , GRIPPER_ON_TARGET , &OnTarget );
 
   return OnTarget;
   
 }
 
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Gripper_Object_Detected( knet_dev_t * dev )
 {
   unsigned char Object;
   knet_read8( dev , GRIPPER_OPTICAL_BARR , &Object );
 
   return Object;
   
 }
 

 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Gripper_Get_Search_Limit( knet_dev_t * dev )
 {
   unsigned char Search_Limit;
   knet_read8( dev , GRIPPER_SEARCH_LIMIT , &Search_Limit );
 
   return Search_Limit;
   
 }
 /*--------------------------------------------------------------------*/
 unsigned char kgripper_Gripper_Get_Limits( knet_dev_t * dev )
 {
   unsigned char Max_Position;
   
   knet_read8( dev , GRIPPER_MAX_POSITION ,  &Max_Position );
 
   return Max_Position;
 }
 
 /********************** WRITE FUNCTION ********************************/
 /*--------------------------------------------------------------------*/
 void kgripper_Gripper_Set_Order( knet_dev_t * dev, unsigned char Order )
 {
   knet_write8( dev , GRIPPER_ORDER , Order );
 }
 
 /*--------------------------------------------------------------------*/
 void kgripper_Gripper_Set_Torque( knet_dev_t * dev, unsigned short Torque )
 {
   knet_set_order( dev ,  KGRIPPER_ORDER_MASK );
   knet_write16( dev , GRIPPER_TORQUE , Torque );
 
 }
 
 /*--------------------------------------------------------------------*/
 void kgripper_GripperSet_Search_Limit( knet_dev_t * dev, unsigned char Search_Limit )
 {
   if(Search_Limit > 1)
     Search_Limit = 1;
   knet_write8( dev , GRIPPER_SEARCH_LIMIT , Search_Limit );
 
 }
