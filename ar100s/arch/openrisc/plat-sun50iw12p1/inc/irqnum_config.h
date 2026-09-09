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
#define INTC_R_NMI_IRQ          0
#define INTC_R_TIMER0_IRQ       1
#define INTC_R_TIMER1_IRQ       2
#define INTC_R_TIMER2_IRQ       3
#define INTC_R_ALM0_IRQ         9
#define INTC_R_WDT0_IRQ         11
#define INTC_R_WDT1_IRQ         12
#define INTC_R_PPU_IRQ          13
#define INTC_R_GPIOL_NS_IRQ     14
#define INTC_R_GPIOL_S_IRQ      15
#define INTC_R_GPIOM_NS_IRQ     16
#define INTC_R_GPIOM_S_IRQ      17
#define INTC_R_USB_IRQ          18
#define INTC_R_UART_IRQ         21
#define INTC_TWI0_IRQ           26
#define INTC_TWI1_IRQ           27
#define INTC_R_IRRX_IRQ         32
#define INTC_R_PWM_IRQ          36
#define INTC_R_TZMA_IRQ         37
#define INTC_GIC_C0_IRQ         38
#define IRQ_SOUCE_MAX           (INTC_GIC_C0_IRQ + 1)

/*
 * ------------------------------------------------------------------------------
 * gic interrupt source
 * ------------------------------------------------------------------------------
 */
#define GIC_USB0_EHCI_IRQ  62
#define GIC_USB0_OHCI_IRQ  63
#define GIC_USB0_OTG_IRQ   64
#define GIC_USB1_EHCI_IRQ  65
#define GIC_USB1_OHCI_IRQ  66
#define GIC_USB2_EHCI_IRQ  67
#define GIC_USB2_OHCI_IRQ  68
#define GIC_R_EXTERNAL_NMI_IRQ  172
#define GIC_R_ALARM0_IRQ        179
#define GIC_R_GPIOL_S_IRQ  180
#define GIC_R_GPIOL_NS_IRQ 181
#define GIC_R_GPIOM_S_IRQ  182
#define GIC_R_GPIOM_NS_IRQ 183
#define GIC_R_IR_IRQ       187


#endif	/*__IRQNUM_CONFIG_H__*/
