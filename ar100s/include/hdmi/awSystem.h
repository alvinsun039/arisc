/****************************************************************************
 * hdmirx/awSystem.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/
#ifndef __SYS_H
#define __SYS_H
#include "dbgs.h"

#define MESSAGE_VERSION "00.00.00.03"
#define PRINT_MCU_VERSION()               \
	LOG("AW1938  ARISC  " MESSAGE_VERSION "\n")

void hdmirx_main_loop(void);
void hdmirx_init_app(void);

#ifdef CFG_HDMI_USED
void hdmi_main_loop(void);
void hdmi_init(void);
#else
static inline void hdmi_init(void) { return; }
static inline void hdmi_main_loop(void) { return; }
#endif

/* TIME CONSTANTS FOR 10MS TIMER */
#define TIMER_INACTIVE 0
#define TIMER_EXPIRED 1
#define TIME_OUT_830MS ((830 + 10) / 10)

#endif
