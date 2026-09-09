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
#include <libfdt.h>

extern u32 dtb_base;

enum {
	AXP517B_CHIPID = 0x4,
	AXP517P_CHIPID = 0x6,
};

/**
 * only called by bmu common function.
 */

static void bmu_axp517_clear_irq(void)
{
	u8 devaddr = RSB_RTSADDR_AXP517;
	u8 data;
	u8 regaddr;

	save_state_flag(REC_SHUTDOWN | 0x400);

	data = 0x00;
	for (regaddr = AXP517_IRQ_EN0; regaddr <= AXP517_IRQ_EN4; regaddr++) {
		pmu_reg_write(&devaddr, &regaddr, &data, 1);
	}

	data = 0xFF;

	for (regaddr = AXP517_IRQ0; regaddr <= AXP517_IRQ4; regaddr++) {
		pmu_reg_write(&devaddr, &regaddr, &data, 1);
	}

	regaddr = AXP517_IRQ_EN1;
	data = 0xc3;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	LOG("clear axp517 irq\n");
}

static void bmu_axp517_reset(void)
{
	save_state_flag(REC_SHUTDOWN | 0x401);

	bmu_axp517_clear_irq();

	LOG("reset axp517\n");
}

static bool is_axp2202(const char *compatible)
{
	if (!compatible)
		return FALSE;

	int len = strlen(compatible);

	return len >= 7 && strcmp(compatible + len - 7, "axp2202") == 0;
}

static bool bmu_axp517_shutdown_check_pmu(void)
{
	void *fdt;
	int pmu_node;
	const char *compatible;

	fdt = (void *)(dtb_base);
	pmu_node = fdt_path_offset(fdt, "pmu0");
	if (pmu_node < 0)
		return FALSE;

	compatible = fdt_getprop(fdt, pmu_node, "compatible", NULL);
	if (!compatible)
		return FALSE;

	return is_axp2202(compatible);
}

static bool bmu_axp517_shutdown_check(void)
{
	int status;

	if (RSB_RTSADDR_AXP517 == RSB_RTSADDR_AXP517_01) {
		if (bmu_axp517_shutdown_check_pmu())
			return FALSE;
	}

	status = bmu_check_status();
	if (status == FDT_NODE_OKAY) {
		return TRUE;
	}

	return FALSE;
}

static void bmu_axp517_shutdown(void)
{
	u8 devaddr = RSB_RTSADDR_AXP517;
	u8 regaddr;
	u8 data;

	save_state_flag(REC_SHUTDOWN | 0x402);

	bmu_axp517_clear_irq();

	if (bmu_axp517_shutdown_check()) {
		regaddr = AXP517_BATFET_CTRL;
		pmu_reg_read(&devaddr, &regaddr, &data, 1);
		data &= ~(0x30);
		data |= 0x10;
		pmu_reg_write(&devaddr, &regaddr, &data, 1);

		pmu_reg_read(&devaddr, &regaddr, &data, 1);
		data |= 0x08;
		pmu_reg_write(&devaddr, &regaddr, &data, 1);
	}

	LOG("close axp517 batfet\n");
}

static s32 bmu_axp517_charging_vbus_det(void)
{
	u8 devaddr = RSB_RTSADDR_AXP517;
	u8 regaddr = AXP517_STATUS0;
	u8 val;
	int status;

	save_state_flag(REC_SHUTDOWN | 0x501);

	status = bmu_check_status();
	if (status != FDT_NODE_OKAY)
		return FAIL;

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	/* vbus presence */
	if (val & 0x20) {
		if (val & 0x8)
			return OK;
	}
	return FAIL;
}

static s32 bmu_axp517_is_exist(void)
{
	u8 devaddr = RSB_RTSADDR_AXP517;
	u8 regaddr = AXP517_CHIP_ID_EXT;
	u8 val = 0;

	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	/* axp517 presence */
	switch (val) {
	case AXP517B_CHIPID:
	case AXP517P_CHIPID:
		LOG("axp517 0x%x exist\n", val);
		return OK;
	default:
		return FAIL;
	}
}

bmu_ops_t bmu_axp517_ops = {
	.bmu_is_exist = bmu_axp517_is_exist,
	.bmu_shutdown = bmu_axp517_shutdown,
	.bmu_reset = bmu_axp517_reset,
	.bmu_charging_vbus_det = bmu_axp517_charging_vbus_det,
};

