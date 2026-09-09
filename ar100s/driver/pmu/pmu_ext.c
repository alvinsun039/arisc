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
 * tcs4838 voltages info table,
 * the index of table is voltage type.
 */
pmu_onoff_reg_bitmap_t tcs4838_onoff_reg_bitmap[] = {
	//reg_addr          //offset //state //dvm_en
	{TCS4838_VSEL0,   	 7,    1,    0},//TCS4838_DCDC0
	{TCS4838_VSEL1,   	 7,    1,    0},//TCS4838_DCDC1
};

/**
 * sy8827g voltages info table,
 * the index of table is voltage type.
 */
pmu_onoff_reg_bitmap_t sy8827g_onoff_reg_bitmap[] = {
	//reg_addr          //offset //state //dvm_en
	{SY8827G_VSEL0,   	 7,    1,    0},//SY8827G_DCDC0
	{SY8827G_VSEL1,   	 7,    1,    0},//SY8827G_DCDC1
};

/**
 * pmu_ext type
 */
enum {
	TCS4838_ID = 1,
	SY8827G_ID,
	AXP1530_ID,
};

enum axp1530_power_status {
	AXP1530_DCDC1_EN_CFG = 0,
	AXP1530_DCDC2_EN_CFG,
	AXP1530_DCDC3_EN_CFG,
	AXP1530_ALDO_EN_CFG,
	AXP1530_DLDO_EN_CFG,
	AXP1530_POWER_MAX_CFG
};

static int pmu_chip_id;
/**
 * pmu_ext check,
 */
s32 pmu_ext_is_exist(void)
{
	u8 devaddr;
	u8 regaddr;
	u8 data = 0;

	devaddr = RSB_RTSADDR_AXP1530;
	regaddr = AXP1530_VERSION;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	if (data == 0x49) {
		pmu_chip_id = AXP1530_ID;
		return AXP1530_POWER_MAX;
	}

	devaddr = RSB_RTSADDR_TCS4838;
	regaddr = TCS4838_ID1;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= 0xE0;

	if (data == 0x80) {
		pmu_chip_id = TCS4838_ID;
		return TCS4838_POWER_MAX;
	}

	devaddr = RSB_RTSADDR_SY8827G;
	regaddr = SY8827G_ID1;

	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= 0xE0;

	if (data == 0x80) {
		pmu_chip_id = SY8827G_ID;
		return SY8827G_POWER_MAX;
	}

	return FALSE;
}

/**
 * axp1530 specific function,
 */
s32 axp1530_backup(u32 type, u32 state)
{
	u8 devaddr = RSB_RTSADDR_AXP1530;
	u8 regaddr;
	u32 offset_mask = 0;
	static u8 reg_bak[3];
	static u8 poweron_stable;

	if (state == POWER_VOL_OFF) {
		if (poweron_stable) {
			poweron_stable = 0;
		}
		/* backup */
		if (type == AXP1530_POWER_DCDC1) {
			regaddr = AXP1530_DC1OUT_VOL;
			pmu_reg_read(&devaddr, &regaddr, &reg_bak[0], 1);

			regaddr = AXP1530_DC2OUT_VOL;
			pmu_reg_read(&devaddr, &regaddr, &reg_bak[1], 1);
			offset_mask = 0x03;
		} else if (type == AXP1530_POWER_DCDC3) {
			regaddr = AXP1530_DC3OUT_VOL;
			pmu_reg_read(&devaddr, &regaddr, &reg_bak[2], 1);
			offset_mask = 0x04;
		}
	} else if (state == POWER_VOL_ON) {
		/* wait poweron stable */
		if (!poweron_stable) {
			time_mdelay(30);
			poweron_stable = 1;
		}

		/* recovery */
		if (type == AXP1530_POWER_DCDC1) {
			regaddr = AXP1530_DC1OUT_VOL;
			pmu_reg_write(&devaddr, &regaddr, &reg_bak[0], 1);

			regaddr = AXP1530_DC2OUT_VOL;
			pmu_reg_write(&devaddr, &regaddr, &reg_bak[1], 1);
			offset_mask = 0x03;
		} else if (type == AXP1530_POWER_DCDC3) {
			regaddr = AXP1530_DC3OUT_VOL;
			pmu_reg_write(&devaddr, &regaddr, &reg_bak[2], 1);
			offset_mask = 0x04;
		}
	}
	return offset_mask;
}

static void axp1530_shutdown(void)
{
	u8 devaddr = RSB_RTSADDR_AXP1530;
	u8 regaddr;
	u8 data;
	bool onoff = 0;

	regaddr = AXP1530_WRITE_LOCK;
	data = 0x05;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	regaddr = AXP1530_ERROR_MANAGEMENT;
	data = 0x04;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	regaddr = AXP1530_STARTUP_SEQ_SET;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= ~(1 << 2);
	data |= (onoff << 2);
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
}

/**
 * pmu_ext specific function,
 */
s32 pmu_ext_backup(u8 devaddr, u32 state)
{
	u8 regaddr;
	u8 i;
	static u8 reg_bak[3];

	if (state == POWER_VOL_OFF) {
		/* backup */
		for (i = 0; i < 3; i++) {
			regaddr = i;
			pmu_reg_read(&devaddr, &regaddr, &reg_bak[i], 1);
			if (i < 2)
				reg_bak[i] &= ~(1 << 7);
		}
	} else if (state == POWER_VOL_ON) {
		/* recovery */
		for (i = 0; i < 3; i++) {
			regaddr = i;
			pmu_reg_write(&devaddr, &regaddr, &reg_bak[i], 1);
		}
	}
	return 0;
}

/**
 * pmu_ext specific function,
 */

s32 pmu_ext_set_voltage_state(u32 type, u32 state)
{
	u8 devaddr;
	u8 regaddr;
	u8 data;
	u32 state_mask;

	switch (pmu_chip_id) {
	case TCS4838_ID:
		devaddr = RSB_RTSADDR_TCS4838;
		regaddr = tcs4838_onoff_reg_bitmap[type].regaddr;
		pmu_ext_backup(devaddr, state);
		state_mask = 1 << tcs4838_onoff_reg_bitmap[type].offset;
		break;
	case SY8827G_ID:
		devaddr = RSB_RTSADDR_SY8827G;
		regaddr = sy8827g_onoff_reg_bitmap[type].regaddr;
		pmu_ext_backup(devaddr, state);
		state_mask = 1 << sy8827g_onoff_reg_bitmap[type].offset;
		break;
	case AXP1530_ID:
		devaddr = RSB_RTSADDR_AXP1530;
		regaddr = AXP1530_OUTPUT_POWER_ON_OFF_CTL;
		state_mask = axp1530_backup(type, state);
		break;
	default:
		WRN("invaid pmu_ext type (%d)\n", pmu_chip_id);
		return -EINVAL;
	}

	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= ~state_mask;
	data |= (state == POWER_VOL_ON) ? state_mask : 0;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	if (state == POWER_VOL_ON) {
		//delay 1ms for open PMU output
		time_mdelay(1);
	}

	return OK;
}

#ifdef CFG_PMU_POWER_CHECK
void pmu_ext_power_check(void)
{
	u32 type, status;
	u8 data;
	u8 axp1530_devaddr = RSB_RTSADDR_AXP1530;
	u8 axp1530_regaddr = AXP1530_OUTPUT_POWER_ON_OFF_CTL;

	/*  if exist axp1530 pmu, excute the following code*/

	for (type = AXP1530_DCDC1_EN_CFG, status = 1; type < AXP1530_POWER_MAX_CFG; type++, status++) {
		pmu_reg_read(&axp1530_devaddr, &axp1530_regaddr, &data, 1);
		data = ((data >> type) & 0x01);

		switch (type) {
		case AXP1530_DCDC1_EN_CFG:
		case AXP1530_DCDC2_EN_CFG:
		case AXP1530_DCDC3_EN_CFG:
			if (data)
				LOG("\033[;31mAXP1530_DCDC[%d] open \033[0m\n", status);
			else
				LOG("\033[;31mAXP1530_DCDC[%d] close \033[0m\n", status);
			break;
		case AXP1530_ALDO_EN_CFG:
			if (data)
				LOG("\033[;31mAXP1530_ALDO1 open \033[0m\n", status);
			else
				LOG("\033[;31mAXP1530_ALDO1 close \033[0m\n", status);
			break;
		case AXP1530_DLDO_EN_CFG:
			if (data)
				LOG("\033[;31mAXP1530_DLDO1 open \033[0m\n", status);
			else
				LOG("\033[;31mAXP1530_DLDO1 close \033[0m\n", status);
			break;
		default:
			break;
		}
	}
}
#else
void pmu_ext_power_check(void)
{
	return;
}
#endif

void pmu_ext_shutdown(void)
{
	switch (pmu_chip_id) {
	case AXP1530_ID:
		axp1530_shutdown();
		break;
	default:
		WRN("invaid pmu_ext type (%d)\n", pmu_chip_id);
		break;
	}
}

