/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : intc.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-4-27
* Descript: interrupt controller public header.
* Update  : date                auther      ver     notes
*           2012-4-27 10:52:56  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __IRQNUM_CONFIG_H__
#define __IRQNUM_CONFIG_H__

/*
 * ------------------------------------------------------------------------------
 * r_intc interrupt source
 * ------------------------------------------------------------------------------
 */
#define INTC_R_NMI_IRQ		0
#define INTC_R_USB_IRQ		16
#define INTC_R_TIMER0_IRQ	20
#define INTC_R_TIMER1_IRQ	21
#define INTC_R_TIMER2_IRQ	22
#define INTC_R_TIMER3_IRQ	23
#define INTC_R_ALM0_IRQ		26
#define INTC_R_UART_IRQ		34
#define INTC_R_NDMA_IRQ		46
#define IRQ_SOUCE_MAX           (INTC_R_NDMA_IRQ + 1)

/*
 * ------------------------------------------------------------------------------
 * gic interrupt source
 * ------------------------------------------------------------------------------
 */
#define GIC_R_IR_IRQ		171
#define GIC_R_ALARM0_IRQ	200

#endif	/*__IRQNUM_CONFIG_H__*/
