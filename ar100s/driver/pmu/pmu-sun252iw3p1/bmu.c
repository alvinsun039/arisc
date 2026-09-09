/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                bmu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : bmu.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: power management unit.
* Update  : date                auther      ver     notes
*           2012-5-22 13:33:03  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include "include.h"
#include <libfdt.h>

extern u32 dtb_base;
u32 bmu_runtime_addr;
extern bmu_ops_t bmu_axp515_ops;
extern bmu_ops_t bmu_axp517_ops;
extern bmu_ops_t bmu_axp519_ops;

void bmu_shutdown(void)
{
}

void bmu_reset(void)
{
}

s32 bmu_charging_vbus_det(void)
{
	return 0;
}

s32 bmu_charging_reset(void)
{
	return 0;
}


s32 bmu_init(void)
{
	return 0;
}

int bmu_check_status(void)
{
	return 0;
}

u32 is_bmu_exist(void)
{
	return 0;
}
