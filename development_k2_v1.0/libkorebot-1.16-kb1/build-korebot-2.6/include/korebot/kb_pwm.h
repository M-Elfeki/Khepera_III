/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: flambercy $
 * $Date: 2006/01/26 14:51:09 $
 * $Revision: 1.1 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/kb_pwm.h,v 1.1 2006/01/26 14:51:09 flambercy Exp $
 */


  


extern int kb_pwm_init();
extern int kb_pwm_cleanup();
extern int kb_clk_init();
extern int kb_clk_cleanup();
extern int kb_pwm_activate(unsigned pwm);
extern int kb_pwm_desactivate(unsigned pwm);
extern int kb_pwm_config(unsigned pwm, unsigned pwm_sd, unsigned prescale);
extern int kb_pwm_duty(unsigned pwm, unsigned FDcycle, unsigned Dcycle);
extern int kb_pwm_period(unsigned pwm, unsigned pwm_period);
