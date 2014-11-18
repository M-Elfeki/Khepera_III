#include <korebot/korebot.h>
#define	ARM_SEARCH_LIMIT   0x41


static knet_dev_t * Arm;
static knet_dev_t * Gripper;

int kgripper_init( void )
 {
  int rc;
   
  if((rc = kb_init( 0 , NULL )) < 0 )
  {
         /* Unable to initialize the korebot library */
         KB_ERROR("kb_kh3_init",KB_ERROR_KH3KBINIT);
         return KH3_ERROR_KBINIT;
  }
 }

void kgripper_Arm_Set_Search_Limit( knet_dev_t * dev, unsigned char Search_Limit )
 {
   if(Search_Limit > 1)
     Search_Limit = 1;
   knet_write8( dev , ARM_SEARCH_LIMIT , Search_Limit );
 
 }

int main( int arc, char *argv[])
{
unsigned short Min_Position, Max_Position, Data16;

kgripper_init();

Arm = knet_open( "Kgripper:Arm" , KNET_BUS_I2C , 0 , NULL );
Gripper = knet_open( "Kgripper:Gripper" , KNET_BUS_I2C , 0 , NULL );
printf("Khepera3 Gripper example (C) K-Team S.A\r\n");
printf("Get an object\n");

kgripper_Arm_Set_Search_Limit( Arm, 1);

}

