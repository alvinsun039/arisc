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

#define TWI_CLOCK_FREQ		(200 * 1000)	/* the twi source clock freq */
#define TICK_PER_SEC		(100)


#ifndef CFG_FDT_INIT_ARISC_PMU_USED
#define RSB_RTSADDR_AXP8191		(0x36)
#define RSB_RTSADDR_AXP519		(0x3c)
#define RSB_RTSADDR_AXP515		(0x34)
#define RSB_RTSADDR_AXP517		(0x34)
#else
#define RSB_RTSADDR_AXP8191		(pmu_runtime_addr)
#define RSB_RTSADDR_AXP519		(bmu_runtime_addr)
#define RSB_RTSADDR_AXP515		(bmu_runtime_addr)
#define RSB_RTSADDR_AXP517		(bmu_runtime_addr)
#endif

#define _RSB_RTSADDR_AXP515	(0x34)
#define RSB_RTSADDR_AXP517_T0	(0x34)
#define RSB_RTSADDR_AXP517_01	(0x35)

/* uart config */
#define UART_BAUDRATE		(115200 / 2)

/* devices define */
#define ARISC_DTS_SIZE		(0x00100000)

#endif
