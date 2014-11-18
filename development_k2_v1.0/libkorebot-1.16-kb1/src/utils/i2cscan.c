#include <korebot/korebot.h>
#include "i2ccom.h"

int printdev(i2c_t * i2c, i2c_dev_t dev, void * param)
{
  printf("Found device 0x%x\r\n",dev);
}
	      
int main(int argc, char *argv[]) {
  i2c_t i2c;
  int rc;

  /* Set the libkorebot debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  if(i2c_open( &i2c , NULL)) {
    perror("open");
    exit(1);
  }

  printf("K-Team I2C scan\r\n");

  i2c_scan(&i2c,printdev,NULL);
}    
