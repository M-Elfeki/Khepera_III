/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2006/10/27 08:53:21 $
 * $Revision: 1.2 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/kb_pwm.c,v 1.2 2006/10/27 08:53:21 pbureau Exp $
 */

#include <sys/mman.h>
#include "korebot.h"
#include "kb_pxa_register.h"
#include "kb_gpio.h"


/*! 
 * \file   kb_pwm.c Support for KoreBot General Purpose IO.             
 *
 * \brief 
 *         This module provides useful basic facilities to use the
 *         KoreBot PWM and configure them.
 *
 * \author   Frederic Lambercy (K-Team SA)
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define MEMDEV "/dev/mem"
#define MAXPWM 2

#define PWM0 16		
#define PWM1 17

#define reg_val(reg) *((unsigned long *)reg)

static void* map_base_pwm0;
static void* map_base_pwm1;

static void* map_base_clk;
static int fd_pwm0;
static int fd_pwm1;
static int fd_clk;

/********************* PWM0 registers functions *************************/

static void bit_set_pwm0(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base_pwm0 + ((addr+(gpio/32)*4) & MAP_MASK);
	
  mask      = 1 << (gpio%32);
  
  reg_val(virt_addr) = reg_val(virt_addr) | mask;
}

static void bit_clear_pwm0(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base_pwm0 + ((addr+(gpio/32)*4) & MAP_MASK);
  mask      = 1 << (gpio%32);
  
  reg_val(virt_addr) = reg_val(virt_addr) & ~mask;
}

static int bit_test_pwm0(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base_pwm0 + ((addr+(gpio/32)*4) & MAP_MASK);
  mask      = 1 << (gpio%32);
  
  if(reg_val(virt_addr) & mask)
    return 1;
  else
    return 0;
}

static void reg_set_pwm0(unsigned long addr,unsigned long value)
{
  void * virt_addr;

 virt_addr = map_base_pwm0 + (addr & MAP_MASK);
 
  reg_val(virt_addr) = value;
}

static long int get_reg_pwm0(unsigned long addr)
{
  void * virt_addr;

  virt_addr = map_base_pwm0 + (addr & MAP_MASK);
 
  return reg_val(virt_addr);
}


/***************** PWM1 registers functions **************************/

static void bit_set_pwm1(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base_pwm1 + ((addr+(gpio/32)*4) & MAP_MASK);
	
  mask      = 1 << (gpio%32);
  
  reg_val(virt_addr) = reg_val(virt_addr) | mask;
}

static void bit_clear_pwm1(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base_pwm1 + ((addr+(gpio/32)*4) & MAP_MASK);
  mask      = 1 << (gpio%32);
  
  reg_val(virt_addr) = reg_val(virt_addr) & ~mask;
}

static int bit_test_pwm1(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base_pwm1 + ((addr+(gpio/32)*4) & MAP_MASK);
  mask      = 1 << (gpio%32);
  
  if(reg_val(virt_addr) & mask)
    return 1;
  else
    return 0;
}

static void reg_set_pwm1(unsigned long addr,unsigned long value)
{
  void * virt_addr;

 virt_addr = map_base_pwm1 + (addr & MAP_MASK);
 
  reg_val(virt_addr) = value;
}

static long int get_reg_pwm1(unsigned long addr)
{
  void * virt_addr;

  virt_addr = map_base_pwm1 + (addr & MAP_MASK);
 
  return reg_val(virt_addr);
}

/******************** CLK register functions *************************/


static void bit_set_clk(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base_clk + ((addr+(gpio/32)*4) & MAP_MASK);
	
  mask      = 1 << (gpio%32);
  
  reg_val(virt_addr) = reg_val(virt_addr) | mask;
}

static void bit_clear_clk(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base_clk + ((addr+(gpio/32)*4) & MAP_MASK);
  mask      = 1 << (gpio%32);
  
  reg_val(virt_addr) = reg_val(virt_addr) & ~mask;
}

/*!
 * Initialize the CLK module. This function must be called before any
 * other call the pwm functions.
 *
 * \return 
 * 	- 0 on success
 *	- a negative error code on failure
 */
int kb_clk_init()
{

  /* Open the memory device */
  if((fd_clk = open(MEMDEV, O_RDWR | O_SYNC)) == -1)
  {
    KB_ERROR("kb_clk_init",KB_ERROR_FILEOPEN,MEMDEV);
    return -KB_ERROR_FILEOPEN;
  }

  /* map registers to user space */
  map_base_clk = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_clk, CLK_BASE & ~MAP_MASK);
  if(map_base_clk == (void *) -1) 
  {
    KB_ERROR("kb_clk_init",KB_ERROR_MMAP,CLK_BASE & ~MAP_MASK);
    return -KB_ERROR_MMAP;
  }

  return 0;
}



/*!
 * Initialize the PWM0 module. This function must be called before any
 * other call the pwm functions.
 *
 * \return 
 * 	- 0 on success
 *	- a negative error code on failure
 */
int kb_pwm0_init()
{

  /* Open the memory device */
  if((fd_pwm0 = open(MEMDEV, O_RDWR | O_SYNC)) == -1)
  {
    KB_ERROR("kb_pwm0_init",KB_ERROR_FILEOPEN,MEMDEV);
    return -KB_ERROR_FILEOPEN;
  }

  /* map registers to user space */
  map_base_pwm0 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_pwm0, PWM0_BASE & ~MAP_MASK);
  if(map_base_pwm0 == (void *) -1) 
  {
    KB_ERROR("kb_pwm0_init",KB_ERROR_MMAP,PWM0_BASE & ~MAP_MASK);
    return -KB_ERROR_MMAP;
  }

  return 0;
}
/*!
 * Initialize the PWM1 module. This function must be called before any
 * other call the pwm functions.
 *
 * \return 
 * 	- 0 on success
 *	- a negative error code on failure
 */
int kb_pwm1_init()
{


	
  /* Open the memory device */
  if((fd_pwm1 = open(MEMDEV, O_RDWR | O_SYNC)) == -1)
  {
    KB_ERROR("kb_pwm1_init",KB_ERROR_FILEOPEN,MEMDEV);
    return -KB_ERROR_FILEOPEN;
  }

  /* map registers to user space */
  map_base_pwm1 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_pwm1, PWM1_BASE & ~MAP_MASK);
  if(map_base_pwm1 == (void *) -1) 
  {
    KB_ERROR("kb_pwm1_init",KB_ERROR_MMAP,PWM1_BASE & ~MAP_MASK);
    return -KB_ERROR_MMAP;
  }

  return 0;
}

/*!
 * Initialize the two PWM module. This function must be called before any
 * other call the pwm functions.
 *
 * \return 
 * 	- 0 on success
 *	- a negative error code on failure
 */
int kb_pwm_init()
{
	kb_pwm0_init();
	kb_pwm1_init();
	/* Initialize the CLK module registers*/
	kb_clk_init();
	kb_gpio_init();
}

/*!
 * Cleanup the PWM module. Must be called to free the module ressources.
 */
int kb_pwm_cleanup()
{
    if(munmap(map_base_pwm0, MAP_SIZE) == -1) KB_ERROR("kb_pwm0_cleanup",KB_ERROR_MMAP,map_base_pwm0);
    close(fd_pwm0);
	if(munmap(map_base_pwm1, MAP_SIZE) == -1) KB_ERROR("kb_pwm1_cleanup",KB_ERROR_MMAP,map_base_pwm1);
    close(fd_pwm1);
}


/*!
 * Configure the given IO as an PWM. The GPIO will  be configure as
 * an output.
 *
 * \param pwm A pwm number between 0 and 1
 * 
 */
int kb_pwm_activate(unsigned pwm)
{

	
  if(pwm >= MAXPWM)
  {
    KB_ERROR("kb_pwm_activate",KB_ERROR_INVALID);
    return -KB_ERROR_INVALID;
  }
  
 /*printf("CKEN = %x, GPDR = %x \r\n", get_reg(CKEN), get_reg(GPDR),get_reg(GPDR));
 printf("GPDR0 = %x, GAFR0_U = %x\n\r",get_reg(GPDR0),get_reg(GAFR0_U));  */
  
  if (pwm)
  {
	  /* Set gpio to output */
	  kb_gpio_dir(PWM1,0);
	  /* Set gpio to PWM mode */
	  kb_gpio_function(PWM1,2);
	  
  }

  else
  {

  	/* Set gpio to output */
  	kb_gpio_dir(PWM0,0);
  	/* Set gpio to PWM mode */
	kb_gpio_function(PWM0,2);
	  
  }
/* Set the PWM clock enalbe*/
 bit_set_clk(CKEN,pwm);
 
/* printf("CKEN = %x, GPDR = %x \r\n", get_reg(CKEN), get_reg(GPDR),get_reg(GPDR));
 printf("GPDR0 = %x, GAFR0_U = %x\n\r",get_reg(GPDR0),get_reg(GAFR0_U));  */
 
  return 0;
}

/*!
 * Desactivate the PWM.
 *
 * \param pwm A pwm number between 0 and 1
 * 
 */
int kb_pwm_desactivate(unsigned pwm)
{
  if(pwm >= MAXPWM)
  {
    KB_ERROR("kb_pwm_desactivate",KB_ERROR_INVALID);
    return -KB_ERROR_INVALID;
  }

  /* Clear the PWM clock enalbe*/
 bit_clear_clk(CKEN,pwm);
 
  return 0;
}




/*!
 * Configure the control register of the given PWM.
 *
 * \param pwm A pwm number between 0 and 1
 * \param pwm_sd Choose the shutdown method
 * \param prescale Determines the frequency of the PWM module clock (old PSCLK_PWMn = 3.6864 MHz / (PWM_CTRL[PRESCALE] + 1))
(PSCLK_PWMn = 13.0 MHz / (PWM_CTRL[PRESCALE] + 1))

  
 */
int kb_pwm_config(unsigned pwm, unsigned pwm_sd, unsigned prescale)
{
	unsigned long tmp;
	
  if(pwm >= MAXPWM)
  {
    KB_ERROR("kb_pwm_config",KB_ERROR_INVALID);
    return -KB_ERROR_INVALID;
  }
  
/*   printf("PWM_CTRL0 = %x, PWM_DUTY0 = %x, PWM_PERVAL0 = %x\r\n", get_reg(PWM_CTRL0), get_reg(PWM_PWDUTY0),get_reg(PWM_PERVAL0));
 printf("PWM_CTRL1 = %x, PWM_DUTY1 = %x, PWM_PERVAL1 = %x\r\n", get_reg(PWM_CTRL1), get_reg(PWM_PWDUTY1),get_reg(PWM_PERVAL1));  */
  
  
  if(pwm_sd)
  	tmp = (prescale & 0x003F) |  0x0040;	/* mask the prescale value and set the pwm shutdown bit*/
  else
	 tmp = (prescale & 0x003F);		/* mask the prescale value and clear the pwm shutdown bit*/

  
  /*printf("TMP = %x\r\n",tmp);	 */
  
  if (pwm)						/* configure PWM1 control register */ 
 	reg_set_pwm1(PWM_CTRL1,tmp);
 
  else							/* configure PWM0 control register */ 
	 reg_set_pwm0(PWM_CTRL0,tmp);
  
    return 0;
}


/*!
 * Configure the duty cycle of the given PWM.
 *
 * \param pwm A pwm number between 0 and 1
 * \param FDcycle Full Duty Cycle
 * \param Dcycle Duty Cycle of PWMn clock
 */
int kb_pwm_duty(unsigned pwm, unsigned FDcycle, unsigned Dcycle)
{
	unsigned long tmp;
	
  if(pwm >= MAXPWM)
  {
    KB_ERROR("kb_pwm_duty",KB_ERROR_INVALID);
    return -KB_ERROR_INVALID;
  }
  
  
  if(FDcycle)
  	tmp = (Dcycle & 0x03FF) |  0x0400;	/* mask the Duty cycle value and set the Full Duty Cycle bit*/
  else
	 tmp = (Dcycle & 0x03FF);		/* mask the Duty cycle value and clear the Full Duty Cycle bi*/
	 
  
  if (pwm)						/* configure PWM1 Duty cycle register */ 
 	reg_set_pwm1(PWM_PWDUTY1,tmp);
 
  else							/* configure PWM0 Duty cycle register */ 
	 reg_set_pwm0(PWM_PWDUTY0,tmp);
  /*
printf("TMP = %x\r\n",tmp);
 printf("PWM_CTRL0 = %x, PWM_DUTY0 = %x, PWM_PERVAL0 = %x\r\n", get_reg(PWM_CTRL0), get_reg(PWM_PWDUTY0),get_reg(PWM_PERVAL0));
 printf("PWM_CTRL1 = %x, PWM_DUTY1 = %x, PWM_PERVAL1 = %x\r\n", get_reg(PWM_CTRL1), get_reg(PWM_PWDUTY1),get_reg(PWM_PERVAL1));  
*/
  return 0;
  

}


/*!
 * Configure the frequency of the given PWM.
 *
 * \param pwm A pwm number between 0 and 1
 * \param pwm_period Period of given PWM
 */
int kb_pwm_period(unsigned pwm, unsigned pwm_period)
{

  if(pwm >= MAXPWM)
  {
    KB_ERROR("kb_pwm_period",KB_ERROR_INVALID);
    return -KB_ERROR_INVALID;
  }


  if (pwm)						/* configure PWM1 period register */ 
    reg_set_pwm1(PWM_PERVAL1,pwm_period);

  else							/* configure PWM0 preiod register */ 
    reg_set_pwm0(PWM_PERVAL0,pwm_period);
  
  return 0;
}
