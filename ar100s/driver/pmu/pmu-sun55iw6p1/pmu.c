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

#if defined CFG_AXP2202_USED
extern pmu_ops_t pmu_axp2202_ops;
static pmu_ops_t *pmu_ops_p = &pmu_axp2202_ops;
extern void axp2202_pmu_power_check(void);
static void (*f_pmu_power_check)(void) = axp2202_pmu_power_check;

/* get pmu wakeup source irq */
extern s32 axp2202_pmu_irq_check(void);
static s32 (*f_pmu_irq_check)(void) = axp2202_pmu_irq_check;

u32 axp_power_max = AXP2202_POWER_MAX;
u32 pmu_runtime_addr = RSB_RTSADDR_AXP2202_B;
#else
#error "sun55iw6p1 not support no pmu"
#endif

extern struct arisc_para arisc_para;
extern struct notifier *wakeup_notify;

void watchdog_reset(void);

static u32 pmu_exist = FALSE;

void pmu_shutdown(void)
{
	if (is_pmu_exist() == FALSE)
		return;

	twi_standby_init();

	while (!!twi_get_status()) {
		LOG("wait twi bus idle loop\n");
		time_mdelay(1000 * 2);
		if (!twi_send_clk_9pulse())
			break;
	}

	if (pmu_ops_p->pmu_shutdown)
		pmu_ops_p->pmu_shutdown();
}

void pmu_reset(void)
{
	if (is_pmu_exist() == FALSE) {
		watchdog_reset();
	}

	twi_standby_init();

	while (!!twi_get_status()) {
		LOG("wait twi bus idle loop\n");
		time_mdelay(1000 * 2);
		if (!twi_send_clk_9pulse())
			break;
	}

	if (pmu_ops_p->pmu_reset)
		pmu_ops_p->pmu_reset();
}

void pmu_charging_reset(void)
{
	if (is_pmu_exist() == FALSE)
		return;

	twi_standby_init();

	while (!!twi_get_status()) {
		time_mdelay(1000 * 2);
		LOG("wait twi bus idle loop\n");
	}

	if (pmu_ops_p->pmu_charging_reset)
		pmu_ops_p->pmu_charging_reset();
}


void pmu_watchdog_reset(void)
{
	twi_standby_init();
	while (!!twi_get_status()) {
		time_mdelay(1000 * 2);
		LOG("wait twi bus idle loop\n");
	}

	LOG("down cpu vol\n");
	//dcdc1 down 900mv
	u8 devaddr = RSB_RTSADDR_AXP1530;
	u8 regaddr = AXP1530_DC1OUT_VOL;
	u8 data = 0x28;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	//dcdc2 down 900mv
	regaddr = AXP1530_DC2OUT_VOL;
	data = 0xa8;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	mdelay(20);

	watchdog_reset();
	return;
}

s32 pmu_set_voltage(u32 type, u32 voltage)
{
	s32 ret = OK;

	if (is_pmu_exist() == FALSE)
		return -ENODEV;

	return ret;
}

s32 pmu_get_voltage(u32 type)
{
	s32 ret = OK;

	if (is_pmu_exist() == FALSE)
		return -ENODEV;

	return ret;
}

s32 pmu_set_voltage_state(u32 type, u32 state)
{
	s32 ret = OK;

	if (is_pmu_exist() == FALSE)
		return -ENODEV;

	if (pmu_ops_p->pmu_set_voltage_state)
		ret = pmu_ops_p->pmu_set_voltage_state(type, state);

	return ret;
}

s32 pmu_get_voltage_state(u32 type)
{
	s32 ret = OK;

	if (is_pmu_exist() == FALSE)
		return OK;

	return ret;
}

s32 pmu_query_event(u32 *event)
{
	s32 ret = OK;

	if (is_pmu_exist() == FALSE)
		return OK;

	return ret;
}

s32 pmu_clear_pendings(void)
{
	s32 ret = OK;

	if (is_pmu_exist() == FALSE)
		return OK;

	return ret;
}

void pmu_power_check(void)
{
	if (is_pmu_exist() == FALSE)
		return;

	f_pmu_power_check();
}

void pmu_chip_init(void)
{
	if (is_pmu_exist() == FALSE)
		return;
}

s32 pmu_reg_write(u8 *devaddr, u8 *regaddr, u8 *data, u32 len)
{
	ASSERT(len <= AXP_TRANS_BYTE_MAX);

	return twi_write(devaddr[0], regaddr, data, len);
}

s32 pmu_reg_read(u8 *devaddr, u8 *regaddr, u8 *data, u32 len)
{
	ASSERT(len <= AXP_TRANS_BYTE_MAX);

	return twi_read(devaddr[0], regaddr, data, len);
}

s32 pmu_reg_write_para(pmu_paras_t *para)
{
	return pmu_reg_write(para->devaddr, para->regaddr, para->data, para->len);
}

s32 pmu_reg_read_para(pmu_paras_t *para)
{
	return pmu_reg_read(para->devaddr, para->regaddr, para->data, para->len);
}

void watchdog_reset(void)
{
	LOG("watchdog reset\n");

	/* disable watchdog int */
	writel(0x0, R_WDOG_REG_BASE + 0x0);

	/* reset whole system */
	writel((0x1 | (0x1 << 8) | WATCHDOG_KEYFIELD), R_WDOG_REG_BASE + 0x14);

	/* set reset after 0.5s */
	writel(((0 << 4) | WATCHDOG_KEYFIELD), R_WDOG_REG_BASE + 0x18);
	mdelay(1);

	/* enable watchdog */
	writel((readl(R_WDOG_REG_BASE + 0x18) | 0x1 | WATCHDOG_KEYFIELD), R_WDOG_REG_BASE + 0x18);
	while (1)
		;
}

int nmi_int_handler(void *parg __attribute__ ((__unused__)), u32 intno)
{
	return TRUE;
}

#ifdef CFG_FDT_INIT_ARISC_PMU_USED
static void pmu_init_from_dts(void)
{
	void *fdt;
	int pmu_node, ret;
	uint32_t pmu_addr = 0;

	fdt = (void *)(dtb_base);

	pmu_node = fdt_path_offset(fdt, "pmu0");
	if (pmu_node < 0)
		return;

	ret = fdt_getprop_u32(fdt, pmu_node, "reg", &pmu_addr);
	if (ret < 0)
		return;

	pmu_runtime_addr = pmu_addr;
}
#endif

s32 pmu_init(void)
{
	u32 power_mode = 0;

	/* power_mode may parse from dts */
	if (power_mode == POWER_MODE_AXP) {
		pmu_exist = TRUE;
	} else {
		pmu_exist = FALSE;
		LOG("pmu is not exist\n");
		return OK;
	}

#ifdef CFG_FDT_INIT_ARISC_PMU_USED
	pmu_init_from_dts();
#endif

	return OK;
}

s32 pmu_exit(void)
{
	return OK;
}

u32 is_pmu_exist(void)
{
	return pmu_exist;
}


static s32 nmi_int_enable(void)
{
	writel(readl(NMI_INT_EN_REG) | (1 << 1), NMI_INT_EN_REG);

	return OK;
}

static s32 nmi_int_disable(void)
{
	writel(readl(NMI_INT_EN_REG) & (~(1 << 1)), NMI_INT_EN_REG);

	return OK;
}

extern u32 volatile wakeup_source;
s32 platform_nmi_handler(void *parg)
{
	nmi_int_disable();

	wakeup_source = (GIC_R_EXTERNAL_NMI_IRQ - GIC_SRC_SPI);

	return OK;
}

s32 pmu_standby_init(void)
{
	nmi_int_enable();

	return OK;
}

s32 pmu_standby_exit(void)
{
	nmi_int_disable();

	return OK;
}

s32 pmu_irq_check(void)
{
	s32 result;
	if (is_pmu_exist() == FALSE)
		return -1;
	result = f_pmu_irq_check();
	return result;
}
