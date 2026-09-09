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
#include <libfdt.h>

#define SUNXI_CHARGING_FLAG_AXP2202 (0x08)
#define SUNXI_REBOOT_FLAG_AXP2202   (0x01)

extern u32 dtb_base;

static int pmu_charging_poweroff_check(void)
{
	void *fdt;
	int pmu_node, ret;
	uint32_t power_off_check = 0;

	fdt = (void *)(dtb_base);

	pmu_node = fdt_path_offset(fdt, "pmu0");
	if (pmu_node < 0)
		return 0;

	ret = fdt_getprop_u32(fdt, pmu_node, "pmu-charging-poweroff", &power_off_check);
	if (ret < 0)
		return 0;

	return power_off_check;
}

/**
 * axp2202 voltages info table,
 * the index of table is voltage type.
 */
pmu_onoff_reg_bitmap_t axp2202_onoff_reg_bitmap[] = {
	//dev_addr               //reg_addr             //offset //state //dvm_en
	{AXP2202_DCDC_CFG0,   	 0,    1,    0},//AWP2202_POWER_DCDC1
	{AXP2202_DCDC_CFG0,   	 1,    1,    0},//AXP2202_POWER_DCDC2
	{AXP2202_DCDC_CFG0,  	 2,    1,    0},//AXP2202_POWER_DCDC3
	{AXP2202_DCDC_CFG0,    	 3,    1,    0},//AXP2202_POWER_DCDC4
	{AXP2202_LDO_EN_CFG0,    0,    0,    0},//AXP2202_POWER_ALDO1
	{AXP2202_LDO_EN_CFG0,    1,    0,    0},//AXP2202_POWER_ALDO2
	{AXP2202_LDO_EN_CFG0,    2,    0,    0},//AXP2202_POWER_ALDO3
	{AXP2202_LDO_EN_CFG0,    3,    0,    0},//AXP2202_POWER_ALDO4
	{AXP2202_LDO_EN_CFG0,    4,    0,    0},//AXP2202_POWER_BLDO1
	{AXP2202_LDO_EN_CFG0,    5,    0,    0},//AXP2202_POWER_BLDO2
	{AXP2202_LDO_EN_CFG0,    6,    0,    0},//AXP2202_POWER_BLDO3
	{AXP2202_LDO_EN_CFG0,    7,    0,    0},//AXP2202_POWER_BLDO4
	{AXP2202_LDO_EN_CFG1,    0,    0,    0},//AXP2202_POWER_CLDO1
	{AXP2202_LDO_EN_CFG1,    1,    0,    0},//AXP2202_POWER_CLDO2
	{AXP2202_LDO_EN_CFG1,    2,    0,    0},//AXP2202_POWER_CLDO3
	{AXP2202_LDO_EN_CFG1,    3,    0,    0},//AXP2202_POWER_CLDO4
	{AXP2202_LDO_EN_CFG1,    4,    0,    0},//AXP2202_POWER_CPUSLDO
	{AXP2202_INVALID_ADDR,   0,    0,    0},//POWER_ONOFF_MAX
};

enum axp2202_power_status {
	AXP2202_POWER_BASE = 0,
	AXP2202_DCDC_EN_CFG = 4,
	AXP2202_ALDO_EN_CFG = 8,
	AXP2202_BLDO_EN_CFG = 12,
	AXP2202_CLDO_EN_CFG = 16,
	AXP2202_CPUS_LDO_CFG = 16,
};

/**
 * axp2202 specific function,
 * only used in axp2202.
 */
static u8 _axp2202_vbus_check(void)
{
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = AXP2202_COMM_STAT0;
	u8 data;
	u8 count;

	/* vbus presence */
	if (is_bmu_exist() == TRUE) {
		if (bmu_charging_vbus_det() == OK) {
			LOG("%s:%d bmu_charging_vbus_det\n", __func__, __LINE__);
			return 1;
		}
		return 0;
	} else {
		/* battery presence */
		pmu_reg_read(&devaddr, &regaddr, &data, 1);
		if (!(data & (1 << 3)))
			return 0;

		pmu_reg_read(&devaddr, &regaddr, &data, 1);
		if (data & (1 << 5)) {
			LOG("%s:%d vbus presence:0x%x\n", __func__, __LINE__, data);
			return 1;
		}
		/* disable IRQ */
		regaddr = AXP2202_INTEN2;
		pmu_reg_read(&devaddr, &regaddr, &data, 1);
		data &= ~(1 << 7);
		pmu_reg_write(&devaddr, &regaddr, &data, 1);

		/* read VBUS_IRQ */
		regaddr = AXP2202_INTSTS2;
		pmu_reg_read(&devaddr, &regaddr, &data, 1);
		/* vbus insert irq */
		if (data & (1 << 7)) {
			/* read VBUS GOOD */
			regaddr = AXP2202_COMM_STAT0;
			for (count = 0; count < 10; count++) {
				pmu_reg_read(&devaddr, &regaddr, &data, 1);
				if (data & (1 << 5))
					return 1;
				udelay(5 * 1000);
			}
		}
		return 0;
	}

}

static void _axp2202_pmu_softset(u8 val)
{
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = AXP2202_COMM_STAT0;
	u8 data;

	if (val) {
		pmu_reg_read(&devaddr, &regaddr, &data, 1);
		if ((data & (1 << 3))) {
			regaddr = AXP2202_COMM_CFG;
			pmu_reg_read(&devaddr, &regaddr, &data, 1);
			data |= 0x1;
			pmu_reg_write(&devaddr, &regaddr, &data, 1);
			mdelay(10);
		}
	}

	save_state_flag(REC_SHUTDOWN | 0x300 | val);

	regaddr = AXP2202_SOFT_PWROFF;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data |= 1 << val;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	if (val)
		LOG("reset system\n");
	else
		LOG("poweroff system\n");

	while (1)
		;
}

/**
 * axp2202 specific function,
 * only called by pmu common function.
 */
static void axp2202_pmu_shutdown(void)
{
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = 0x6A;
	u8 data = 0x03;

	save_state_flag(REC_SHUTDOWN | 0x201);
	if (is_bmu_exist() == TRUE)
		pmu_reg_write(&devaddr, &regaddr, &data, 1);

	regaddr = AXP2202_BUFFER0;
	data = 0;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	_axp2202_vbus_check();
	bmu_shutdown();
	_axp2202_pmu_softset(0);
}

static void axp2202_pmu_reset(void)
{
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = AXP2202_BUFFER0;
	u8 val = SUNXI_REBOOT_FLAG_AXP2202;

	save_state_flag(REC_SHUTDOWN | 0x202);
	_axp2202_vbus_check();
	pmu_reg_write(&devaddr, &regaddr, &val, 1);
	bmu_reset();
	_axp2202_pmu_softset(1);
}

static void axp2202_pmu_hot_reset(void)
{
	u8 data;
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = AXP2202_BUFFER0;
	u8 val = SUNXI_REBOOT_FLAG_AXP2202;

	save_state_flag(REC_SHUTDOWN | 0x202);
	_axp2202_vbus_check();
	pmu_reg_write(&devaddr, &regaddr, &val, 1);

	/* disable powerok reset function: clear BIT(3) before PRCM soft reset */
	regaddr = AXP2202_SOFT_PWROFF;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= ~(1 << 3);
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	bmu_reset();
}

static void axp2202_pmu_charging_reset(void)
{
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = AXP2202_BUFFER0;
	u8 val;

	if (pmu_charging_poweroff_check())
		return;

	save_state_flag(REC_SHUTDOWN | 0x203);
	val = _axp2202_vbus_check();
	if (val) {
		val = SUNXI_CHARGING_FLAG_AXP2202;
		pmu_reg_write(&devaddr, &regaddr, &val, 1);
		bmu_reset();
		_axp2202_pmu_softset(1);
	}
}

static s32 axp2202_pmu_set_voltage_state(u32 type, u32 state)
{
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr;
	u8 data;
	u32 offset;

	regaddr = axp2202_onoff_reg_bitmap[type].regaddr;
	offset  = axp2202_onoff_reg_bitmap[type].offset;
//	axp2202_onoff_reg_bitmap[type].state = state;

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

s32 axp2202_pmu_clear_pendings(void)
{
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr;
	u8 data = 0xff;

	for (regaddr = AXP2202_INTSTS1; regaddr <= AXP2202_INTSTS5; regaddr++) {
		pmu_reg_write(&devaddr, &regaddr, &data, 1);
	}

	return OK;
}

#ifdef CFG_PMU_POWER_CHECK
void axp2202_pmu_power_check(void)
{
	u32 type, status, offset;
	u8 regaddr, data;
	u8 axp2202_devaddr = RSB_RTSADDR_AXP2202;

	for (type = AXP2202_POWER_BASE, status = 1; type < AXP2202_DCDC_EN_CFG; type++, status++) {
		regaddr = axp2202_onoff_reg_bitmap[type].regaddr;
		offset = axp2202_onoff_reg_bitmap[type].offset;
		pmu_reg_read(&axp2202_devaddr, &regaddr, &data, 1);
		data = ((data >> offset) & 0x01);
		if (data)
			LOG("\033[;31mAXP2202_DCDC[%d] open \033[0m\n", status);
		else
			LOG("\033[;31mAXP2202_DCDC[%d] close \033[0m\n", status);
	}

	for (type = AXP2202_DCDC_EN_CFG, status = 1; type < AXP2202_ALDO_EN_CFG; type++, status++) {
		regaddr = axp2202_onoff_reg_bitmap[type].regaddr;
		offset = axp2202_onoff_reg_bitmap[type].offset;
		pmu_reg_read(&axp2202_devaddr, &regaddr, &data, 1);
		data = ((data >> offset) & 0x01);
		if (data)
			LOG("\033[;31mAXP2202_ALDO[%d] open \033[0m\n", status);
		else
			LOG("\033[;31mAXP2202_ALDO[%d] close \033[0m\n", status);
	}

	for (type = AXP2202_ALDO_EN_CFG, status = 1; type < AXP2202_BLDO_EN_CFG; type++, status++) {
		regaddr = axp2202_onoff_reg_bitmap[type].regaddr;
		offset = axp2202_onoff_reg_bitmap[type].offset;
		pmu_reg_read(&axp2202_devaddr, &regaddr, &data, 1);
		data = ((data >> offset) & 0x01);
		if (data)
			LOG("\033[;31mAXP2202_BLDO[%d] open \033[0m\n", status);
		else
			LOG("\033[;31mAXP2202_BLDO[%d] close \033[0m\n", status);
	}

	for (type = AXP2202_BLDO_EN_CFG, status = 1; type < AXP2202_CLDO_EN_CFG; type++, status++) {
		regaddr = axp2202_onoff_reg_bitmap[type].regaddr;
		offset = axp2202_onoff_reg_bitmap[type].offset;
		pmu_reg_read(&axp2202_devaddr, &regaddr, &data, 1);
		data = ((data >> offset) & 0x01);
		if (data)
			LOG("\033[;31mAXP2202_CLDO[%d] open \033[0m\n", status);
		else
			LOG("\033[;31mAXP2202_CLDO[%d] close \033[0m\n", status);
	}
}
#else
void axp2202_pmu_power_check(void)
{
	return;
}
#endif

#ifdef CFG_PMU_IRQ_CHECK

#define MAX_IRQ_REG_NUMS    (0x05)
#define REG_MAX_BIT    (0x08)
s32 axp2202_pmu_irq_check(void)
{
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 status_regaddr = AXP2202_INTEN1;
	u8 insert_regaddr = AXP2202_INTSTS1;
	u8 status_data, insert_data, offset, count;
	u8 irq_find = 0;
	u8 irq_mumber = 0;
	/* battery presence */
	for (count = 0 ; count < MAX_IRQ_REG_NUMS;  count++) {
		pmu_reg_read(&devaddr, &status_regaddr, &status_data, 1);
		pmu_reg_read(&devaddr, &insert_regaddr, &insert_data, 1);
		status_regaddr++;
		insert_regaddr++;
		for (offset = 0; offset < REG_MAX_BIT; offset++) {
			if (((status_data >> offset) & 0x01) && ((insert_data >> offset) & 0x01)) {
				irq_find = 1;
				break;
			}
			irq_mumber++;
		}
		if (irq_find) {
			break;
		}
	}
	return irq_mumber;
}
#else
s32 axp2202_pmu_irq_check(void)
{
	return -1;
}
#endif

pmu_ops_t pmu_axp2202_ops = {
	.pmu_shutdown = axp2202_pmu_shutdown,
	.pmu_reset = axp2202_pmu_reset,
	.pmu_hot_reset = axp2202_pmu_hot_reset,
	.pmu_charging_reset = axp2202_pmu_charging_reset,
	.pmu_set_voltage_state = axp2202_pmu_set_voltage_state,
	.pmu_clear_pendings = axp2202_pmu_clear_pendings,
};

