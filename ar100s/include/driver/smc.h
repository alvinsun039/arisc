/*
 * include/drivers/smc.h
 *
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Wuyan <Wuyan@allwinnertech.com>
 *
 */
#ifndef __SMC_H__
#define __SMC_H__

#ifdef CFG_SMC
void smc_standby_init(void);
void smc_standby_exit(void);
#else
static inline void smc_standby_init(void) {}
static inline void smc_standby_exit(void) {}
#endif

#endif  /* __SMC_H__ */
