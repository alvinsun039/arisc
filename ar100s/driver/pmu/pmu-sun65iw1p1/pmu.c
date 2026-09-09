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
#define HOT_RESET_HEAD_MAGIC    (0x5A5A5A5A)
#define HOT_RESET_TAIL_MAGIC    (0xA5A5A5A5)
#define RTC_BOOT_REASON_CMD_REBOOT      (0x6)

extern u32 dtb_base;

/* dram para */
extern uint32_t dts_dram_para[];
extern uint32_t dram_head_magic;
extern uint32_t dram_tail_magic;

#if defined CFG_AXP2202_USED
extern pmu_ops_t pmu_axp2202_ops;
static pmu_ops_t *pmu_ops_p = &pmu_axp2202_ops;
u32 axp_power_max = AXP2202_POWER_MAX;
u32 pmu_runtime_addr = RSB_RTSADDR_AXP2202_B;
#else
#error "sun65iw1p1 not support no pmu"
#endif

extern struct arisc_para arisc_para;
extern struct notifier *wakeup_notify;

void watchdog_reset(void);
void prcm_reset(void);

static u32 pmu_exist = FALSE;

/**
 * Check whether the board supports hot reset via DTS property 'pmu_hot_reset'.
 * Returns non-zero if hot reset is allowed, 0 if only PMU power-cycle reset is allowed.
 */
static int pmu_hot_reset_check(void)
{
	void *fdt;
	int pmu_node, ret;
	uint32_t power_noreset_check = 0;

	fdt = (void *)(dtb_base);

	pmu_node = fdt_path_offset(fdt, "pmu0");
	if (pmu_node < 0)
		return 0;

	ret = fdt_getprop_u32(fdt, pmu_node, "pmu_hot_reset", &power_noreset_check);
	if (ret < 0)
		return 0;

	return power_noreset_check;
}

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

	/* Hot reset (PRCM) only when DTS enables it AND reboot reason matches.
	 * If DTS does not configure pmu_hot_reset, always use PMU power-cycle reset. */
	if (pmu_hot_reset_check() && (readl(RTC_REBOOT_REG) == RTC_BOOT_REASON_CMD_REBOOT)) {
		if (pmu_ops_p->pmu_hot_reset)
			pmu_ops_p->pmu_hot_reset();
		/* Hot reset path: DRAM self-refresh + PRCM soft reset */
		dram_head_magic = HOT_RESET_HEAD_MAGIC;
		dram_tail_magic = HOT_RESET_TAIL_MAGIC;
		dram_power_save_process((void *)(&dts_dram_para[0]));
		save_state_flag(REC_SHUTDOWN | 0x500);
		prcm_reset();
	} else {
		if (pmu_ops_p->pmu_reset)
			pmu_ops_p->pmu_reset();
	}

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
	writel((0x1 | (0x1 << 8) | WATCHDOG_KEYFIELD), R_WDOG_REG_BASE + 0x10);

	/* set reset after 0.5s */
	writel(((0 << 4) | WATCHDOG_KEYFIELD), R_WDOG_REG_BASE + 0x14);
	mdelay(1);

	/* enable watchdog */
	writel((readl(R_WDOG_REG_BASE + 0x14) | 0x1 | WATCHDOG_KEYFIELD), R_WDOG_REG_BASE + 0x14);
	while (1)
		;
}

/* this function is for unified management of soc registers
 * that need to be configured in warm reset scenarios. */
void hotreset_reset_regs(void)
{
	/* reset PPU REG */
	writel(0x8, PPU_VI_REG);
	writel(0x8, PPU_VE_REG);
	writel(0x8, PPU_HSI_REG);
	writel(0x8, PPU_VO_REG);
}

void prcm_reset(void)
{
	hotreset_reset_regs();

	/* Switch CPUS domain clocks (AHBS/AHBS0/APBS1) to RC16M before PRCM reset to prevent CPUS hang during the reset process */
	writel(0x02000000, AHBS_CFG_REG);
	writel(0x02000000, APBS0_CFG_REG);
	writel(0x02000000, APBS1_CFG_REG);

	/* Use PRCM reset instead of WDT reset to maintain PRCM-related registers */
	writel(0x0, VDD_SYS_PWR_RST_REG);
	u32 i = 16000;
	while (i--)
		;
	writel(0x1, VDD_SYS_PWR_RST_REG);

	while (1) {
		__asm volatile("wfi");
	}
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
	interrupt_clear_pending(INTC_R_NMI_IRQ);
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
