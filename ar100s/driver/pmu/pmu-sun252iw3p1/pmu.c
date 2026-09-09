/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                pmu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pmu.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: power management unit.
* Update  : date                auther      ver     notes
*           2012-5-22 13:33:03  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include <libfdt.h>
#include "pmu_i.h"
#include "irq_table.h"

#define WATCHDOG_KEYFIELD (0x16aa << 16)
#define SUNXI_CHARGING_FLAG (0x61)

extern u32 dtb_base;

u32 pmu_runtime_addr;

extern struct arisc_para arisc_para;
extern struct notifier *wakeup_notify;

void watchdog_reset(void);

void pmu_shutdown(void)
{
}

void pmu_reset(void)
{
}

void pmu_charging_reset(void)
{
}

s32 pmu_set_voltage(u32 type, u32 voltage)
{
	return 0;
}

s32 pmu_get_voltage(u32 type)
{
	return 0;
}

s32 pmu_set_voltage_state(u32 type, u32 state)
{
	return 0;
}

s32 pmu_get_voltage_state(u32 type)
{
	return 0;
}

s32 pmu_query_event(u32 *event)
{
	return 0;
}

s32 pmu_clear_pendings(void)
{
	return 0;
}

void pmu_chip_init(void)
{
}

s32 pmu_reg_write(u8 *devaddr, u8 *regaddr, u8 *data, u32 len)
{
	return 0;
}

s32 pmu_reg_read(u8 *devaddr, u8 *regaddr, u8 *data, u32 len)
{
	return 0;
}

s32 pmu_reg_write_para(pmu_paras_t *para)
{
	return 0;
}

s32 pmu_reg_read_para(pmu_paras_t *para)
{
	return 0;
}

void watchdog_reset(void)
{
}

int nmi_int_handler(void *parg __attribute__ ((__unused__)), u32 intno)
{
	return TRUE;
}

s32 pmu_init(void)
{
	return OK;
}

// static s32 nmi_int_enable(void)
// {
// 	return OK;
// }

// static s32 nmi_int_disable(void)
// {
// 	return OK;
// }

extern u32 volatile wakeup_source;
s32 platform_nmi_handler(void *parg)
{
	return OK;
}

s32 pmu_standby_init(void)
{
	return OK;
}

s32 pmu_standby_exit(void)
{
	return OK;
}
