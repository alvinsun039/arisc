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
static void bmu_eta6973_reset(void)
{
	u8 devaddr = RSB_RTSADDR_ETA6973;
	u8 regaddr = ETA6973_REG_0B;
	u8 val;

	save_state_flag(REC_SHUTDOWN | 0x401);

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	val |= 0x80;
	pmu_reg_write(&devaddr, &regaddr, &val, 1);

	LOG("reset eta6973\n");
}

static void bmu_eta6973_shutdown(void)
{
	u8 devaddr = RSB_RTSADDR_ETA6973;
	u8 regaddr = ETA6973_REG_07;
	u8 val;

	save_state_flag(REC_SHUTDOWN | 0x402);

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	val |= 0x80;
	pmu_reg_write(&devaddr, &regaddr, &val, 1);

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	val &= 0xF7;
	pmu_reg_write(&devaddr, &regaddr, &val, 1);

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	val |= 0x20;
	pmu_reg_write(&devaddr, &regaddr, &val, 1);

	LOG("close eta6973 batfet\n");
}

static s32 bmu_eta6973_charging_vbus_det(void)
{
	u8 devaddr = RSB_RTSADDR_ETA6973;
	u8 regaddr = ETA6973_REG_0A;
	u32 reg_val;
	u32 pin_grp = PIN_GRP_PL;
	u32 pin_num = 7;
	u8 val;

	save_state_flag(REC_SHUTDOWN | 0x501);

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	/* vbus presence */
	if ((val & 0x80) == 0x80) {
		reg_val = pin_read_data(pin_grp, pin_num);
		if (!reg_val) {
			regaddr = ETA6973_REG_0B;
			pmu_reg_read(&devaddr, &regaddr, &val, 1);
			val |= 0x80;
			pmu_reg_write(&devaddr, &regaddr, &val, 1);
			return OK;
		}
	}
	return FAIL;
}

static s32 bmu_eta6973_is_exist(void)
{
	u8 devaddr = RSB_RTSADDR_ETA6973;
	u8 regaddr = ETA6973_REG_0B;
	u8 val = 0;

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	/* eta6973 presence */
	if (((val & 0x78) == 0x78) || ((val & 0x78) == 0x38)) {
		LOG("eta6973/eta6974 exist\n");
		return OK;
	}
	return FAIL;
}

bmu_ops_t bmu_eta6973_ops = {
	.bmu_is_exist = bmu_eta6973_is_exist,
	.bmu_shutdown = bmu_eta6973_shutdown,
	.bmu_reset = bmu_eta6973_reset,
	.bmu_charging_vbus_det = bmu_eta6973_charging_vbus_det,
};

