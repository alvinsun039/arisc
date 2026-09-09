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

#include "pmu_i.h"

/**
 * axp1530 voltages info table,
 * the index of table is voltage type.
 */
pmu_onoff_reg_bitmap_t axp1530_onoff_reg_bitmap[] = {
	 //reg_addr                    //offset //state //dvm_en
	{AXP1530_OUTPUT_POWER_ON_OFF_CTL,    0,    1,    0},//AXP1530_POWER_DCDC1
	{AXP1530_OUTPUT_POWER_ON_OFF_CTL,    1,    1,    0},//AXP1530_POWER_DCDC2
	{AXP1530_OUTPUT_POWER_ON_OFF_CTL,    2,    1,    0},//AXP1530_POWER_DCDC3
	{AXP1530_OUTPUT_POWER_ON_OFF_CTL,    3,    1,    0},//AXP1530_POWER_ALDO1
	{AXP1530_OUTPUT_POWER_ON_OFF_CTL,    4,    1,    0},//AXP1530_POWER_DLDO1
	{AXP1530_INVALID_ADDR,               0,    0,    0},//POWER_ONOFF_MAX
};

/**
 * axp1530 specific function,
 * only called by pmu common function.
 */
static void axp1530_pmu_shutdown(void)
{
	u8 devaddr = RSB_RTSADDR_AXP1530;
	u8 regaddr = AXP1530_POWER_DOMN_SEQUENCE;
	u8 data = 1 << 7;

	save_state_flag(REC_SHUTDOWN | 0x101);

	/* power off system, disable DCDC & LDO */
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	pmu_reg_read(&devaddr, &regaddr, &data, 1);

	LOG("poweroff system\n");
	while (1)
		;
}

static void axp1530_pmu_reset(void)
{
	u8 devaddr = RSB_RTSADDR_AXP1530;
	u8 regaddr = AXP1530_POWER_DOMN_SEQUENCE;
	u8 data;

	save_state_flag(REC_SHUTDOWN | 0x201);

	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data |= 1 << 6;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	LOG("reset system\n");
	while (1)
		;
}

static s32 axp1530_pmu_set_voltage_state(u32 type, u32 state)
{
	u8 devaddr = RSB_RTSADDR_AXP1530;
	u8 regaddr;
	u8 data;
	u32 offset;

	regaddr = axp1530_onoff_reg_bitmap[type].regaddr;
	offset  = axp1530_onoff_reg_bitmap[type].offset;
	axp1530_onoff_reg_bitmap[type].state = state;

	//read-modify-write
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= (~(1 << offset));
	data |= (state << offset);
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	if (state == POWER_VOL_ON) {
		//delay 1ms for open PMU output
		time_mdelay(1);
	}

	return OK;
}

pmu_ops_t pmu_axp1530_ops = {
	.pmu_shutdown = axp1530_pmu_shutdown,
	.pmu_reset = axp1530_pmu_reset,
	.pmu_set_voltage_state = axp1530_pmu_set_voltage_state,
};

