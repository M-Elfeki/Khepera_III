/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: flambercy $
 * $Date: 2006/01/26 14:50:58 $
 * $Revision: 1.2 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/kb_gpio.c,v 1.2 2006/01/26 14:50:58 flambercy Exp $
 */

#include <sys/mman.h>
#include "korebot.h"
#include "kb_pxa_register.h"

/*! 
 * \file   kb_gpio.c Support for KoreBot General Purpose IO.             
 *
 * \brief 
 *         This module provides useful basic facilities to use the
 *         KoreBot GPIO and configure them.
 *
 * \author   Pierre Bureau (K-Team SA)
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define MEMDEV "/dev/mem"
#define MAXIO 84
#define MAXPWM 2

#define reg_val(reg) *((unsigned long *)reg)

static void* map_base;
static int fd;

static void bit_set(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base + ((addr+(gpio/32)*4) & MAP_MASK);
	
  mask      = 1 << (gpio%32);
  
  reg_val(virt_addr) = reg_val(virt_addr) | mask;
}

static void bit_clear(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base + ((addr+(gpio/32)*4) & MAP_MASK);
  mask      = 1 << (gpio%32);
  
  reg_val(virt_addr) = reg_val(virt_addr) & ~mask;
}

static int bit_test(unsigned long addr,unsigned gpio)
{
  void * virt_addr;
  unsigned long mask;

  virt_addr = map_base + ((addr+(gpio/32)*4) & MAP_MASK);
  mask      = 1 << (gpio%32);
  
  if(reg_val(virt_addr) & mask)
    return 1;
  else
    return 0;
}

static void reg_set(unsigned long addr,unsigned long value)
{
  void * virt_addr;

 virt_addr = map_base + (addr & MAP_MASK);
 
  reg_val(virt_addr) = value;
}

static long int get_reg(unsigned long addr)
{
  void * virt_addr;

  virt_addr = map_base + (addr & MAP_MASK);
 
  return reg_val(virt_addr);
}

/*!
 * Initialize the GPIO module. This function must be called before any
 * other call the gpio functions.
 *
 * \return 
 * 	- 0 on success
 *	- a negative error code on failure
 */
int kb_gpio_init()
{

  /* Open the memory device */
  if((fd = open(MEMDEV, O_RDWR | O_SYNC)) == -1)
  {
    KB_ERROR("kb_gpio_init",KB_ERROR_FILEOPEN,MEMDEV);
    return -KB_ERROR_FILEOPEN;
  }

  /* map registers to user space */
  map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE & ~MAP_MASK);
  if(map_base == (void *) -1) 
  {
    KB_ERROR("kb_gpio_init",KB_ERROR_MMAP,GPIO_BASE & ~MAP_MASK);
    return -KB_ERROR_MMAP;
  }

  return 0;
}

/*!
 * Cleanup the GPIO module. Must be called to free the module ressources.
 */
int kb_gpio_cleanup()
{
    if(munmap(map_base, MAP_SIZE) == -1) KB_ERROR("kb_gpio_cleanup",KB_ERROR_MMAP,map_base);
    close(fd);
}

int kb_gpio_mode()
{
}

/*!
 * Configure the given GPIO pin function. Check first if the pin is freely
 * available on the KoreBot.
 *
 * \param gpio A gpio number between 0 and 84
 * \param function 	0:IO mode 
 * 			1:Alternate function 1 
 * 			2:Alternate function 2
 * 			3:Alternate function 3
 */
void kb_gpio_function(unsigned gpio, unsigned function)
{
  if(gpio > MAXIO || function > 3)
    KB_ERROR("kb_gpio_function",KB_ERROR_INVALID);

  if((function & 0x1))
    bit_set(GAFR,2*gpio);
  else
    bit_clear(GAFR,2*gpio);

  if((function & 0x2))
    bit_set(GAFR,(2*gpio)+1);
  else
    bit_clear(GAFR,(2*gpio)+1);
}

/*!
 * Configure the given GPIO as input or output. The GPIO function must be 
 * configured as well to use the pin as an IO.
 * 
 * \param gpio A gpio number between 0 and 84
 * \param dir The direction 0:output 1:input
 */
void kb_gpio_dir(unsigned gpio, unsigned dir)
{
  if(gpio > MAXIO)
    KB_ERROR("kb_gpio_dir",KB_ERROR_INVALID);

  if(dir)
    bit_clear(GPDR,gpio);
  else
    bit_set(GPDR,gpio);
}

/*! 
 * Set a given GPIO. The GPIO must be first configured to ouput mode.
 * If the GPIO is configured as input, it will be set when 
 * configured as ouput.
 *
 * \param gpio A gpio number between 0 and 84
 */
void kb_gpio_set(unsigned gpio)
{
  if(gpio > MAXIO)
    KB_ERROR("kb_gpio_set",KB_ERROR_INVALID);

  bit_set(GPSR,gpio);
}

/*! 
 * Clear a given GPIO. The GPIO must be first configured to ouput mode.
 * If the GPIO is configured as input, it will be cleared when 
 * configured as ouput.
 *
 * \param gpio A gpio number between 0 and 84
 */
void kb_gpio_clear(unsigned gpio)
{
  if(gpio > MAXIO)
    KB_ERROR("kb_gpio_clear",KB_ERROR_INVALID);

  bit_set(GPCR,gpio);
}

/*!
 * Get the current level for a given GPIO. It can be used regardless 
 * of the current GPIO configuration.
 *
 * \param gpio A gpio number between 0 and 84
 *
 * \return 
 * 	- 1 if the GPIO is set
 * 	- 0 if the GPIO is cleared
 * 	- an error code if the GPIO is invalid
 */
int kb_gpio_get(unsigned gpio)
{
  if(gpio > MAXIO)
  {
    KB_ERROR("kb_gpio_get",KB_ERROR_INVALID);
    return -KB_ERROR_INVALID;
  }

  return bit_test(GPLR, gpio);
}
