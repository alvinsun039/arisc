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

#include "bmu_i.h"

/**
 * only called by bmu common function.
 */

static void bmu_axp519_clear_irq(void)
{
	u8 devaddr = RSB_RTSADDR_AXP519;
	u8 regaddr = AXP519_IRQ_EN0;
	u8 data = 0xFF;

	save_state_flag(REC_SHUTDOWN | 0x400);

	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	regaddr = AXP519_IRQ_EN1;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	regaddr = AXP519_IRQ0;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	regaddr = AXP519_IRQ1;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	regaddr = AXP519_IRQ_EN0;
	data = 0xF7;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
}

static void bmu_axp519_reset(void)
{
	save_state_flag(REC_SHUTDOWN | 0x401);

	bmu_axp519_clear_irq();

	LOG("reset axp519\n");
}

static void bmu_axp519_shutdown(void)
{
	save_state_flag(REC_SHUTDOWN | 0x402);

	bmu_axp519_clear_irq();

	LOG("close axp519 batfet\n");
}

static s32 bmu_axp519_charging_vbus_det(void)
{
	u8 devaddr = RSB_RTSADDR_AXP519;
	u8 regaddr = AXP519_SYS_STATUS;
	u8 val;

	save_state_flag(REC_SHUTDOWN | 0x501);

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	/* vbus presence */
	if ((val & 0x01) == 0x01) {
		regaddr = AXP519_WORK_CFG;
		pmu_reg_read(&devaddr, &regaddr, &val, 1);
		if (!(val & 0x01)) {
			regaddr = AXP519_CHG_SET1;
			pmu_reg_read(&devaddr, &regaddr, &val, 1);

			if (!(val & 0x40)) {
				return OK;
			}
		}
	}
	return FAIL;
}

static s32 bmu_axp519_is_exist(void)
{
	u8 devaddr = RSB_RTSADDR_AXP519;
	u8 regaddr = AXP519_CHIP_ID;
	u8 val = 0;

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	/* axp519 presence */
	if ((val & 0x7) == 0x3) {
		LOG("axp519 exist\n");
		return OK;
	}
	return FAIL;
}

bmu_ops_t bmu_axp519_ops = {
	.bmu_is_exist = bmu_axp519_is_exist,
	.bmu_shutdown = bmu_axp519_shutdown,
	.bmu_reset = bmu_axp519_reset,
	.bmu_charging_vbus_det = bmu_axp519_charging_vbus_det,
};

