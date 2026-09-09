/*
 * include/cfgs.h
 *
 * Descript: system configure header.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: superm <superm@allwinnertech.com>
 *
 */
#ifndef __PLATFORM_CFGS_H__
#define __PLATFORM_CFGS_H__

#define AXP_TRANS_BYTE_MAX		(8)	/* the max number of pmu transfer byte */

#define TWI_CLOCK_FREQ                  (200 * 1000)	/* the twi source clock freq */
#define TICK_PER_SEC                    (100)

#define RSB_RTSADDR_AXP2202		(pmu_runtime_addr)
#define RSB_RTSADDR_AXP2202_B	(0x34)
#define RSB_RTSADDR_AXP2202_C	(0x35)

#define RSB_RTSADDR_TCS4838		(0x41)
#define RSB_RTSADDR_SY8827G		(0x60)
#define RSB_RTSADDR_AXP1530		(0x36)

#define RSB_RTSADDR_AXP517		(bmu_runtime_addr)
#define RSB_RTSADDR_AXP517_T0	(0x34)
#define RSB_RTSADDR_AXP517_01	(0x35)


/* amp msgbox config */
#define AMP_ARM                         (0)
#define AMP_RISC                        (1)
#define AMP_MIPS                        (2)
#define AMP_LOCAL                       (AMP_RISC)
#define AMP_PSCI                        (AMP_ARM)

#define MSGBOX_NUM                      (2)
#define CHANNEL_MAX                     (4)

/* define the specify channel to recieve power manage command from arm*/
#define CHANNEL_PSCI                    (3)


/* uart config */
#define UART_BAUDRATE                   (115200 / 2)

/* devices define */
#define ARISC_DTS_SIZE (0x00100000)

#endif
