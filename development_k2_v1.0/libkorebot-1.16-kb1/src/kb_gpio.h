/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: flambercy $
 * $Date: 2006/01/26 14:50:58 $
 * $Revision: 1.2 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/kb_gpio.h,v 1.2 2006/01/26 14:50:58 flambercy Exp $
 */
  


extern int kb_gpio_init();
extern int kb_gpio_cleanup();
extern void kb_gpio_dir(unsigned gpio, unsigned dir);
extern void kb_gpio_set(unsigned gpio);
extern void kb_gpio_clear(unsigned gpio);
extern int kb_gpio_get(unsigned gpio);
void kb_gpio_function(unsigned gpio, unsigned function);
