/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: flambercy $
 * $Date: 2006/01/26 14:50:58 $
 * $Revision: 1.2 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/kb_pxa_register.h,v 1.2 2006/01/26 14:50:58 flambercy Exp $
 */

/*! 
 * \file   kb_pxa_register.h PXA270 register definition
 *
 * \brief Include file for low level CPU access
 *
 * \author   Julien Tharin (K-Team SA) 
 * \note     Copyright (C) 2009 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

/* see "Intel PXA27x Processor Family Developer's Manual", pages 1023-1024
    	Table 24-41. GPIO Controller Register Summary
*/

#define GPIO_BASE 0x40E00000

#define GPLR		0x40E00000
#define GPLR0		(0x40E00000)  /* GPIO Pin Level Register GPIO<31:0> */
#define GPLR1		(0x40E00004)  /* GPIO Pin Level Register GPIO <63:32> */
#define GPLR2		(0x40E00008)  /* GPIO Pin Level Register GPIO <95:64> */
#define GPLR3		(0x40E00100)  /* GPIO Pin Level Register GPIO <120:96> */

#define GPDR		0x40E0000C
#define GPDR0		(0x40E0000C)  /* GPIO Pin Direction Register GPIO<31:0> */
#define GPDR1		(0x40E00010)  /* GPIO Pin Direction Register GPIO<63:32> */
#define GPDR2		(0x40E00014)  /* GPIO Pin Direction Register GPIO<95:64> */
#define GPDR3		(0x40E0010C)  /* GPIO Pin Direction Register GPIO<120:96> */

#define GPSR		0x40E00018
#define GPSR0		(0x40E00018)  /* GPIO Pin Output Set Register GPIO<31:0> */
#define GPSR1		(0x40E0001C)  /* GPIO Pin Output Set Register GPIO<63:32> */
#define GPSR2		(0x40E00020)  /* GPIO Pin Output Set Register GPIO<95:64> */
#define GPSR3		(0x40E00118)  /* GPIO Pin Output Set Register GPIO<120:96> */

#define GPCR		0x40E00024
#define GPCR0		(0x40E00024)  /* GPIO Pin Output Clear Register GPIO<31:0> */
#define GPCR1		(0x40E00028)  /* GPIO Pin Output Clear Register GPIO <63:32> */
#define GPCR2		(0x40E0002C)  /* GPIO Pin Output Clear Register GPIO <95:64> */
#define GPCR3		(0x40E00124)  /* GPIO Pin Output Clear Register GPIO <120:96> */

#define GRER		0x40E00030
#define GRER0		(0x40E00030)  /* GPIO Rising-Edge Detect Register GPIO<31:0> */
#define GRER1		(0x40E00034)  /* GPIO Rising-Edge Detect Register GPIO<63:32> */
#define GRER2		(0x40E00038)  /* GPIO Rising-Edge Detect Register GPIO<80:64> */
#define GRER3		(0x40E00130)  /* GPIO Rising-Edge Detect Register GPIO<120:96> */

#define GFER		0x40E0003C
#define GFER0		(0x40E0003C)  /* GPIO Falling-Edge Detect Register GPIO<31:0> */
#define GFER1		(0x40E00040)  /* GPIO Falling-Edge Detect Register GPIO<63:32> */
#define GFER2		(0x40E00044)  /* GPIO Falling-Edge Detect Register GPIO<95:64> */
#define GFER3		(0x40E0013C)  /* GPIO Falling-Edge Detect Register GPIO<120:96> */

#define GEDR		0x40E00048
#define GEDR0		(0x40E00048)  /* GPIO Edge Detect Status Register GPIO<31:0> */
#define GEDR1		(0x40E0004C)  /* GPIO Edge Detect Status Register GPIO<63:32> */
#define GEDR2		(0x40E00050)  /* GPIO Edge Detect Status Register GPIO<95:64> */
#define GEDR3		(0x40E00148)  /* GPIO Edge Detect Status Register GPIO<120:96> */

#define GAFR		0x40E00054
#define GAFR0_L		(0x40E00054)  /* GPIO Alternate Function Select Register GPIO<15:0> */
#define GAFR0_U		(0x40E00058)  /* GPIO Alternate Function Select Register GPIO<31:16> */
#define GAFR1_L		(0x40E0005C)  /* GPIO Alternate Function Select Register GPIO<47:32> */
#define GAFR1_U		(0x40E00060)  /* GPIO Alternate Function Select Register GPIO<63:48> */
#define GAFR2_L		(0x40E00064)  /* GPIO Alternate Function Select Register GPIO<79:64> */
#define GAFR2_U		(0x40E00068)  /* GPIO Alternate Function Select Register GPIO<95:80> */
#define GAFR3_L		(0x40E0006C)  /* GPIO Alternate Function Select Register GPIO<111:96> */
#define GAFR3_U		(0x40E00070)  /* GPIO Alternate Function Select Register GPIO<120:112> */

#define CLK_BASE 0x41300000

#define CKEN	0x41300004			/* Clock enable register */

#define PWM0_BASE 0x40B00000

#define PWM_CTRL0		0x40B00000	/* PWM0 control register */
#define PWM_PWDUTY0	0x40B00004	/* PWM0 duty control register */
#define PWM_PERVAL0	0x40B00008	/* PWM0 Period control register */

#define PWM1_BASE 0x40C00000

#define PWM_CTRL1		0x40C00000	/* PWM1 control register */
#define PWM_PWDUTY1	0x40C00004	/* PWM1 duty control register */
#define PWM_PERVAL1	0x40C00008	/* PWM1 Period control register */
