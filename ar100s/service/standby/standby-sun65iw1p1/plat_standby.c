/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                               standby module
*
*                                    (c) Copyright 2012-2016, superm China
*                                             All Rights Reserved
*
* File    : extended_super_standby.c
* By      : superm
* Version : v1.0
* Date    : 2013-1-16
* Descript: extended super-standby module public header.
* Update  : date                auther      ver     notes
*           2013-1-16 17:04:28  superm       1.0     Create this file.
*********************************************************************************************************
*/
#include <libfdt.h>
#include "../standby_i.h"
#include "wakeup_source.h"
#include "cpucfg_regs.h"
#include "../../../driver/pmu/pmu_i.h"

extern u32 dram_crc_enable;
extern u32 dram_crc_src;
extern u32 dram_crc_len;
extern u32 volatile wakeup_source;
extern u32 axp_power_max;
extern u32 dtb_base;

static s32 result;
static u32 suspend_lock;
static u32 standby_type;
static u32 usb_standby_port;

/* standby paras */
static uint32_t standby_vdd_cpul;
static uint32_t standby_vdd_cpum;
static uint32_t standby_vdd_cpub;
static uint32_t standby_vdd_sys;
static uint32_t standby_vdd_gpu;
static uint32_t standby_vcc_pll;
static uint32_t standby_vcc_io;
static uint32_t standby_osc24m_on = 1;

/* dram para */
extern uint32_t dts_dram_para[];

/* pmu ext check */
static uint32_t pmu_ext_power_max = -1;

/* timer_stamp paras */
static uint32_t timestamp_cnt_low_bak;
static uint32_t timestamp_cnt_hi_bak;
static uint32_t timestamp_cnt_freqid_bak;
static uint64_t rtc_last_time;

/* backup cpu PLL */
#define CPU_PLL_REG_BAK_NUM (3)
static u32 cpu_clk_restore_bak[CPU_PLL_REG_BAK_NUM] = {0};
static u32 cpu_pll_restore_bak[CPU_PLL_REG_BAK_NUM] = {0};
static u32 cpu_pll_pat0_restore_bak[CPU_PLL_REG_BAK_NUM] = {0};
static u32 cpu_pll_ssc_restore_bak[CPU_PLL_REG_BAK_NUM] = {0};

/* backup PLL */
typedef struct PLL_PAMA {
	u32 addr;
	u8  pll_en;
	u8  pll_ldo_en;
	u8  lock_en;
	u8  out_put_en;
	u32 factor;
} rPLL_PAMA;
rPLL_PAMA pll_restore[9];

/*  backup cpu/cpus source clock */
typedef struct BUS_PAMA {
	u32 busAddr;
	u8  bus_clk_src;
	u8  M;
	u8  flag;//check the first write is or not default;
} rBUS_PAMA;
rBUS_PAMA bus_restore[7];

typedef struct SYS_BUS_PAMA {
	u32 busAddr;
	u8 bus_clk_src;
	u8 M;
} SYS_BUS_CLK;
SYS_BUS_CLK bus_ctl_restore[4];

enum bus_clock_mode {
	osc_24M = 0,
	rtc_32k = 1,
	rc16m  = 2,
	pll_peri0 = 3,
	clock_bak = 4,
	ccmu_clk = 5,
	prcm_clk = 6,
	bus_clk = 7,
	gic_cci_clk = 8,
	start_clk = 9,
	close_clk = 10
};

enum bus_tick_mode {
	bus_start_tick = 0,
	bus_end_tick = 4,
	prcm_start_tick = 4,
	prcm_end_tick = 7
};

enum sys_bus_mode {
	nm_start_tick = 0,
	nm_end_tick = 2,
	gc_start_tick = 2,
	gc_end_tick = 4
};

enum pll_state {
	pll_disable = 0,
	pll_enable  = 1
};

/* axp2202 dcdc3(dram) - pwm */
static void axp_dcdc_mode_set(int on)
{
#if defined CFG_AXP2202_USED
	static u8 axp_dcdc3_pwm;
	static u32 pwm_has_parsed;
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = AXP2202_DCDC_CFG1;
	u8 data;

	if (!pwm_has_parsed) {
		pmu_reg_read(&devaddr, &regaddr, &axp_dcdc3_pwm, 1);
		pwm_has_parsed = 1;
	}

	if (axp_dcdc3_pwm & (1 << 4)) {
		if (!on) {
			data = axp_dcdc3_pwm & (~(1 << 4));
			pmu_reg_write(&devaddr, &regaddr, &data, 1);
		} else {
		pmu_reg_write(&devaddr, &regaddr, &axp_dcdc3_pwm, 1);
		}
	}
#endif
}

u8 get_vdd_sys_reg(void)
{
#if defined CFG_AXP2202_USED
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = AXP2202_DCDC2_CFG;
	u8 reg_value;

	if (pmu_reg_read(&devaddr, &regaddr,
			 &reg_value, 1)) {
		return -1;
	}

	return reg_value;
#endif
}

int set_vdd_sys_reg(int set_vol, int onoff)
{
#if defined CFG_AXP2202_USED
	u8 devaddr = RSB_RTSADDR_AXP2202;
	u8 regaddr = AXP2202_DCDC2_CFG;
	u8 reg_value;

	/* read cfg value */
	if (pmu_reg_read(&devaddr, &regaddr,
			 &reg_value, 1)) {
		return -1;
	}

	/* set voltage */
	reg_value &= ~0x7f;
	set_vol &= 0x7f;
	reg_value |= set_vol;
	if (pmu_reg_write(&devaddr, &regaddr,
			  &reg_value, 1)) {
		return -1;
	}

	/* set on/onff */
	regaddr = AXP2202_OUTPUT_CTL0;
	if (pmu_reg_read(&devaddr, &regaddr,
			 &reg_value, 1)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << 1);
	} else {
		reg_value |= (1 << 1);
	}

	regaddr = AXP2202_OUTPUT_CTL0;
	if (pmu_reg_write(&devaddr, &regaddr,
			  &reg_value, 1)) {
		return -1;
	}
#endif
	return 0;
}


static uint64_t rtc_get_timestamp_in_seconds_since(uint64_t last)
{
	uint32_t rtc_ymd, rtc_hms;
	uint64_t rtc_s;

	rtc_ymd = readl(RTC_YMD) & RTC_YMD_MASK;
	rtc_hms = readl(RTC_HMS);

	rtc_s = rtc_ymd * 24 * 3600;
	rtc_s += (rtc_hms & RTC_HMS_S_MASK);
	rtc_s += ((rtc_hms & RTC_HMS_M_MASK) >> RTC_HMS_M_SHIFT) * 60;
	rtc_s += ((rtc_hms & RTC_HMS_H_MASK) >> RTC_HMS_H_SHIFT) * 3600;

	if (last)
		rtc_s = rtc_s - last;

	return rtc_s;
}

static void dcxo_disable(void)
{
	u32 val;

	val = readl(RTC_XO_CTRL_REG) & (1 << 31);
	if ((!standby_osc24m_on) && (!!val)) {
		ccu_24mhosc_disable();
	}
}

static void dcxo_enable(void)
{
	u32 val;

	val = readl(RTC_XO_CTRL_REG) & (1 << 31);
	if ((!standby_osc24m_on) && (!!val)) {
		ccu_24mhosc_enable();
		time_mdelay(1);
	}
}

static s32 standby_dts_parse(void)
{
	static bool dts_has_parsed = FALSE;
	void *fdt;
	int32_t param_node;

	if (dts_has_parsed)
		return 0;

	fdt = (void *)(dtb_base);

	/* parse power tree */
	param_node = fdt_path_offset(fdt, "standby_param");
	if (param_node < 0) {
		WRN("no standby_param: %x fdt:%x\n", param_node, fdt);
		return -1;
	}

	fdt_getprop_u32(fdt, param_node, "vdd-cpul", &standby_vdd_cpul);
	fdt_getprop_u32(fdt, param_node, "vdd-cpum", &standby_vdd_cpum);
	fdt_getprop_u32(fdt, param_node, "vdd-cpub", &standby_vdd_cpub);
	fdt_getprop_u32(fdt, param_node, "vdd-sys", &standby_vdd_sys);
	fdt_getprop_u32(fdt, param_node, "vdd-gpu", &standby_vdd_gpu);
	fdt_getprop_u32(fdt, param_node, "vcc-pll", &standby_vcc_pll);
	fdt_getprop_u32(fdt, param_node, "vcc-io", &standby_vcc_io);
	fdt_getprop_u32(fdt, param_node, "osc24m-on", &standby_osc24m_on);

	LOG("standby_vdd_cpul 0x%x\n", standby_vdd_cpul);
	LOG("standby_vdd_cpum 0x%x\n", standby_vdd_cpum);
	LOG("standby_vdd_cpub 0x%x\n", standby_vdd_cpub);
	LOG("standby_vdd_sys 0x%x\n", standby_vdd_sys);
	LOG("standby_vdd_gpu 0x%x\n", standby_vdd_gpu);
	LOG("standby_vcc_pll 0x%x\n", standby_vcc_pll);
	LOG("standby_vcc_io 0x%x\n", standby_vcc_io);
	LOG("standby_osc24m_on 0x%x\n", standby_osc24m_on);

	dts_has_parsed = TRUE;

	return 0;
}

static void suspend_para_prepare(void)
{
	if (!!pll_restore[0].addr)
		return;

	/* set up restore paras */
	pll_restore[0].addr = CCU_PLL_PERI0_CTRL_REG;
	pll_restore[1].addr = CCU_PLL_PERI1_CTRL_REG;
	pll_restore[2].addr = CCU_PLL_GPU_CTRL_REG;
	pll_restore[3].addr = CCU_PLL_VIDEO0_CTRL_REG;
	pll_restore[4].addr = CCU_PLL_VIDEO1_CTRL_REG;
	pll_restore[5].addr = CCU_PLL_VIDEO2_CTRL_REG;
	pll_restore[6].addr = CCU_PLL_VE_CTRL_REG;
	pll_restore[7].addr = CCU_PLL_AUDIO0_CTRL_REG;
	pll_restore[8].addr = CCU_PLL_AUDIO1_CTRL_REG;

	bus_restore[0].busAddr = CCU_AHB_CFG_REG;
	bus_restore[1].busAddr = CCU_APB0_CFG_REG;
	bus_restore[2].busAddr = CCU_APB1_CFG_REG;
	bus_restore[3].busAddr = CCU_APB_UART_CFG_REG;
	bus_restore[4].busAddr = AHBS_CFG_REG;
	bus_restore[5].busAddr = APBS0_CFG_REG;
	bus_restore[6].busAddr = APBS1_CFG_REG;

	bus_ctl_restore[0].busAddr = CCU_NSI_CFG_REG;
	bus_ctl_restore[1].busAddr = CCU_MBUS_CLK_REG;
	bus_ctl_restore[2].busAddr = CCU_GIC_CFG_REG;
	bus_ctl_restore[3].busAddr = CCU_CPU_SYS_DP_CLK_REG;

	/* get pmu_ext type */
	pmu_ext_power_max = pmu_ext_is_exist();
}

static void wait_wakeup(void)
{
	save_state_flag(REC_ESTANDBY | REC_WAIT_WAKEUP | 0x01);
	wakeup_timer_start();
	//wakeup_source = NO_WAKESOURCE;
	while (1) {
		/*
		 * maybe add user defined task process here
		 */


		if (wakeup_source != NO_WAKESOURCE) {
			LOG("wakeup: %d\n", wakeup_source);
			break;
		}

		/* FIXME later */
		writel(readl(LP_CTRL_REG) | ((1 << 24) | (1 << 25) | (1 << 26) | (1 << 27)), LP_CTRL_REG);
		cpu_enter_doze();
		writel(readl(LP_CTRL_REG) & (~((1 << 24) | (1 << 25) | (1 << 26) | (1 << 27))), LP_CTRL_REG);
	}
	wakeup_timer_stop();
}

static void wait_cpu0_resume(void)
{
	s32 ret;
	struct message message;

	/* no paras for resume notify message */
	message.paras = NULL;

	printk("wait ac327 resume...\n");

	/* wait cpu0 restore finished. */
	while (1) {
		ret = hwmsgbox_query_message(&message, 0);
		if (ret != OK)
			continue; /* no message, query again */

		/* query valid message */
		if (message.type == SSTANDBY_RESTORE_NOTIFY) {
			/* cpu0 restore, feedback wakeup event. */
			LOG("cpu0 restore finished\n");
			/* init feedback message */
			message.count = 1;
			message.result = 0;
			message.paras = (u32 *)&wakeup_source;
			/* synchronous message, need feedback. */
			hwmsgbox_feedback_message(&message, SEND_MSG_TIMEOUT);
			break;
		} else {
			/* invalid message detected, ignore it, by sunny at 2012-6-28 11:33:13. */
			ERR("standby ignore message [%x]\n", message.type);
		}
	}

	wakeup_source = NO_WAKESOURCE;
}

static void dm_suspend(void)
{
	u32 type;

	/* vdd-io powerdown */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vcc_io >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_OFF);
	}

	/* vdd-cpul powerdown */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vdd_cpul >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_OFF);
	}

	/* vdd-cpum powerdown */
	for (type = 0; type < pmu_ext_power_max; type++) {
		if ((standby_vdd_cpum >> type) & 0x1)
			pmu_ext_set_voltage_state(type, POWER_VOL_OFF);
	}

	/* vdd-cpub powerdown */
	for (type = 0; type < pmu_ext_power_max; type++) {
		if ((standby_vdd_cpub >> type) & 0x1)
			pmu_ext_set_voltage_state(type, POWER_VOL_OFF);
	}

	/* vdd-sys powerdown */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vdd_sys >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_OFF);
	}

	/* vdd-gpu powerdown */
	for (type = 0; type < pmu_ext_power_max; type++) {
		if ((standby_vdd_gpu >> type) & 0x1)
			pmu_ext_set_voltage_state(type, POWER_VOL_OFF);
	}

	/* vcc-pll powerdown */
	for (type = 0; type < axp_power_max && !standby_osc24m_on; type++) {
		if ((standby_vcc_pll >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_OFF);
	}
	axp_dcdc_mode_set(0);
}

static void dm_resume(void)
{
	u32 type;

	axp_dcdc_mode_set(1);
	/* vcc-pll powerdown */
	for (type = 0; type < axp_power_max && !standby_osc24m_on; type++) {
		if ((standby_vcc_pll >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vdd-gpu powerdown */
	for (type = 0; type < pmu_ext_power_max; type++) {
		if ((standby_vdd_gpu >> type) & 0x1)
			pmu_ext_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vdd-sys powerdown */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vdd_sys >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vdd-cpub powerdown */
	for (type = 0; type < pmu_ext_power_max; type++) {
		if ((standby_vdd_cpub >> type) & 0x1)
			pmu_ext_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vdd-cpum powerdown */
	for (type = 0; type < pmu_ext_power_max; type++) {
		if ((standby_vdd_cpum >> type) & 0x1)
			pmu_ext_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vdd-cpul powerdown */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vdd_cpul >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vdd-io powerdown */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vcc_io >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_ON);
	}
}

static void dram_suspend(void)
{
	/* calc dram checksum */
	if (standby_dram_crc_enable()) {
		before_crc = standby_dram_crc();
		LOG("before_crc: 0x%x\n", before_crc);
	}

	dram_power_save_process((void *)(&dts_dram_para[0]));
}

static void dram_resume(void)
{
	dram_power_up_process((void *)(&dts_dram_para[0]));

	/* calc dram checksum */
	if (standby_dram_crc_enable()) {
		after_crc = standby_dram_crc();
		if (after_crc != before_crc) {
			save_state_flag(REC_SSTANDBY | REC_DRAM_DBG | 0xf);
			ERR("dram crc error...\n");
			ERR("---->>>>LOOP<<<<----\n");
			while (1)
				;
		}
	}
}

static void sunxi_timestamp_suspend(void)
{
	timestamp_cnt_low_bak = readl(SUNXI_TIMESTAMP_STA_CNT_LOW_REG);
	timestamp_cnt_hi_bak = readl(SUNXI_TIMESTAMP_STA_CNT_HI_REG);
	timestamp_cnt_freqid_bak = readl(SUNXI_TIMESTAMP_CTRL_CNT_FREQID_REG);

	rtc_last_time = rtc_get_timestamp_in_seconds_since(0);
}

static void sunxi_timestamp_resume(void)
{
	uint64_t timestamp_last_cnt;
	uint64_t rtc_suspend_time_s, timestamp_suspend_cnt;

	timestamp_last_cnt = (uint64_t)timestamp_cnt_hi_bak << 32;
	timestamp_last_cnt |= timestamp_cnt_low_bak;
	rtc_suspend_time_s = rtc_get_timestamp_in_seconds_since(rtc_last_time);
	timestamp_suspend_cnt = rtc_suspend_time_s * timestamp_cnt_freqid_bak;
	timestamp_last_cnt += timestamp_suspend_cnt;

	/* disable timestamp firstly */
	writel(0x0, TS_CTRL_REG_BASE);
	/* restore freq */
	writel(timestamp_cnt_freqid_bak, SUNXI_TIMESTAMP_CTRL_CNT_FREQID_REG);
	/* restore low */
	writel((timestamp_last_cnt & 0xffffffff), SUNXI_TIMESTAMP_CTRL_CNT_LOW_REG);
	/* restore high */
	writel(((timestamp_last_cnt >> 32) & 0xffffffff), SUNXI_TIMESTAMP_CTRL_CNT_HI_REG);
	/* enable timestamp again */
	writel(0x1, TS_CTRL_REG_BASE);
}

static u32 usb_standby_port_support(void)
{
	u32 port = 0, val;

	val = readl(USB0_USB_CTRL_REG);
	LOG("[%s] 0x%x:0x%x [%x]\n", __func__, USB0_USB_CTRL_REG, val, val & USB_STANDBY_CLOCK_SEL);
	if ((val & USB_STANDBY_CLOCK_SEL) == USB_CTRL_STANDBY_MODE)
		port |= 0x1; /* usb0 support usb standby */

	val = readl(USB1_USB_CTRL_REG);
	LOG("[%s] 0x%x:0x%x [%x]\n", __func__, USB1_USB_CTRL_REG, val, val & USB_STANDBY_CLOCK_SEL);
	if ((val & USB_STANDBY_CLOCK_SEL) == USB_CTRL_STANDBY_MODE)
		port |= 0x2; /* usb1 support usb standby */

	return port;
}

static void usb_standby_init(void)
{
	/* parse which port support usb standby from USB Standby Clock Sel */
	usb_standby_port = usb_standby_port_support();
}

static void usb_standby_exit(void)
{
	/* restore all ports support usb standby to zero */
	usb_standby_port = 0;
}

static void usb_iso_suspend(void)
{
	u32 val;
	u8 usb0_en = usb_standby_port & 0x1 ? 1 : 0; /* usb0 support usb standby ? */
	u8 usb1_en = usb_standby_port & 0x2 ? 1 : 0; /* usb1 support usb standby ? */
	u8 usb_en  = usb_standby_port ? 1 : 0;

	LOG("[%s] standby_type :0x%x\n", __func__, standby_type);
	LOG("usb0: %d, usb1: %d, usb: %d, port: 0x%x\n",
	    usb0_en, usb1_en, usb_en, usb_standby_port);

	if (!(standby_type & CPUS_WAKEUP_USB)) { /* super standby */
		val = readl(VDD_SYS_PWROFF_GATING_REG);
		LOG("[0] 0x%x: 0x%x\n", VDD_SYS_PWROFF_GATING_REG, val);
		val |= VDD_USB2CPUS_GATING(1);
		val |= VDD_SYS2USB_GATING(1);
		writel(val, VDD_SYS_PWROFF_GATING_REG);
		LOG("[1] 0x%x: 0x%x\n", VDD_SYS_PWROFF_GATING_REG, readl(VDD_SYS_PWROFF_GATING_REG));
	} else { /* usb standby */
		if (usb0_en) {
			/* bugfix: AW1919 SIDDQ need enable when resume */
			val = readl(USB_PHY_CTRL_REG(0));
			LOG("[0] 0x%x: 0x%x\n", USB_PHY_CTRL_REG(0), val);
			val &= ~USB_PHY_SIDDQ_MASK;
			writel(val, USB_PHY_CTRL_REG(0));
			LOG("[1] 0x%x: 0x%x\n", USB_PHY_CTRL_REG(0), readl(USB_PHY_CTRL_REG(0)));

			val = readl(CCU_USB0_CLK_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB0_CLK_REG, val);
			val &= ~USB0_CLKEN_MASK;
			writel(val, CCU_USB0_CLK_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB0_CLK_REG, readl(CCU_USB0_CLK_REG));

			val = readl(CCU_USB0_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB0_GAR_REG, val);
			val &= ~USB0_OHCI_GATING_MASK;
			val &= ~USB0_EHCI_GATING_MASK;
			writel(val, CCU_USB0_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB0_GAR_REG, readl(CCU_USB0_GAR_REG));
		}

		if (usb1_en) {
			/* bugfix: AW1919 SIDDQ need enable when resume */
			val = readl(USB_PHY_CTRL_REG(1));
			LOG("[0] 0x%x: 0x%x\n", USB_PHY_CTRL_REG(1), val);
			val &= ~USB_PHY_SIDDQ_MASK;
			writel(val, USB_PHY_CTRL_REG(1));
			LOG("[1] 0x%x: 0x%x\n", USB_PHY_CTRL_REG(1), readl(USB_PHY_CTRL_REG(1)));

			val = readl(CCU_USB1_CLK_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB1_CLK_REG, val);
			val &= ~USB1_CLKEN_MASK;
			writel(val, CCU_USB1_CLK_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB1_CLK_REG, readl(CCU_USB1_CLK_REG));

			val = readl(CCU_USB1_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB1_GAR_REG, val);
			val &= ~USB1_OHCI_GATING_MASK;
			val &= ~USB1_EHCI_GATING_MASK;
			writel(val, CCU_USB1_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB1_GAR_REG, readl(CCU_USB1_GAR_REG));
		}

		if (usb_en) {
			val = readl(USB_DCTRL_REG);
			LOG("[0] 0x%x: 0x%x\n", USB_DCTRL_REG, val);
			val |= USB_U3U2_U2_MAP_SEL;
			writel(val, CCU_USB2P0_SYS_PHY_REF_CLK_REG);
			LOG("[1] 0x%x: 0x%x\n", USB_DCTRL_REG, readl(USB_DCTRL_REG));

			val = readl(CCU_USB2P0_SYS_PHY_REF_CLK_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB2P0_SYS_PHY_REF_CLK_REG, val);
			val &= ~CCU_USB2P0_SYS_PHY_REF_CLK_MASK;
			writel(val, CCU_USB2P0_SYS_PHY_REF_CLK_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB2P0_SYS_PHY_REF_CLK_REG, readl(CCU_USB2P0_SYS_PHY_REF_CLK_REG));

			val = readl(CCU_USB2P0_SYS_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB2P0_SYS_GAR_REG, val);
			val &= ~USB2P0_SYS_AHB_CLK_MASK;
			writel(val, CCU_USB2P0_SYS_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB2P0_SYS_GAR_REG, readl(CCU_USB2P0_SYS_GAR_REG));
		}

		/* The isolation is necessary */
		val = readl(VDD_SYS_PWROFF_GATING_REG);
		LOG("[0] 0x%x: 0x%x\n", VDD_SYS_PWROFF_GATING_REG, val);
		val |= VDD_SYS2USB_GATING(1);
		writel(val, VDD_SYS_PWROFF_GATING_REG);
		LOG("[1] 0x%x: 0x%x\n", VDD_SYS_PWROFF_GATING_REG, readl(VDD_SYS_PWROFF_GATING_REG));

		if (usb0_en) {
			/* bugfix: AW1919 HCI & PHY rst not assert when resume */
			val = readl(USB_RST_CTRL_REG(0));
			LOG("[0] 0x%x: 0x%x\n", USB_RST_CTRL_REG(0), val);
			val |= USB_PHY_RST_MASK;
			writel(val, USB_RST_CTRL_REG(0));
			LOG("[1] 0x%x: 0x%x\n", USB_RST_CTRL_REG(0), readl(USB_RST_CTRL_REG(0)));

			val = readl(CCU_USB0_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB0_GAR_REG, val);
			val |= USB0_EHCI_RST_MASK;
			val |= USB0_OHCI_RST_MASK;
			writel(val, CCU_USB0_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB0_GAR_REG, readl(CCU_USB0_GAR_REG));
		}

		if (usb1_en) {
			/* bugfix: AW1919 HCI & PHY rst not assert when resume */
			val = readl(USB_RST_CTRL_REG(1));
			LOG("[0] 0x%x: 0x%x\n", USB_RST_CTRL_REG(1), val);
			val |= USB_PHY_RST_MASK;
			writel(val, USB_RST_CTRL_REG(1));
			LOG("[1] 0x%x: 0x%x\n", USB_RST_CTRL_REG(1), readl(USB_RST_CTRL_REG(1)));

			val = readl(CCU_USB1_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB1_GAR_REG, val);
			val |= USB1_EHCI_RST_MASK;
			val |= USB1_OHCI_RST_MASK;
			writel(val, CCU_USB1_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB1_GAR_REG, readl(CCU_USB1_GAR_REG));
		}

		if (usb_en) {
			val = readl(CCU_USB2P0_SYS_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB2P0_SYS_GAR_REG, val);
			val &= ~USB2P0_SYS_RSTN_MASK;
			writel(val, CCU_USB2P0_SYS_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB2P0_SYS_GAR_REG, readl(CCU_USB2P0_SYS_GAR_REG));
		}
	}
}

static void usb_iso_resume(void)
{
	u32 val;
	u8 usb0_en = usb_standby_port & 0x1 ? 1 : 0; /* usb0 support usb standby ? */
	u8 usb1_en = usb_standby_port & 0x2 ? 1 : 0; /* usb1 support usb standby ? */
	u8 usb_en  = usb_standby_port ? 1 : 0;

	LOG("[%s] standby_type :0x%x\n", __func__, standby_type);
	LOG("usb0: %d, usb1: %d, usb: %d, port: 0x%x\n",
	    usb0_en, usb1_en, usb_en, usb_standby_port);

	if (!(standby_type & CPUS_WAKEUP_USB)) { /* super standby */
		val = readl(VDD_SYS_PWROFF_GATING_REG);
		LOG("[0] 0x%x: 0x%x\n", VDD_SYS_PWROFF_GATING_REG, val);
		val &= ~VDD_USB2CPUS_GATING(1);
		val &= ~VDD_SYS2USB_GATING(1);
		writel(val, VDD_SYS_PWROFF_GATING_REG);
		LOG("[1] 0x%x: 0x%x\n", VDD_SYS_PWROFF_GATING_REG, readl(VDD_SYS_PWROFF_GATING_REG));
	} else { /* usb standby */
		if (usb_en) {
			val = readl(CCU_USB2P0_SYS_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB2P0_SYS_GAR_REG, val);
			val |= USB2P0_SYS_RSTN_MASK;
			writel(val, CCU_USB2P0_SYS_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB2P0_SYS_GAR_REG, readl(CCU_USB2P0_SYS_GAR_REG));
		}

		if (usb0_en) {
			val = readl(CCU_USB0_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB0_GAR_REG, val);
			val |= USB0_EHCI_RST_MASK;
			val |= USB0_OHCI_RST_MASK;
			writel(val, CCU_USB0_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB0_GAR_REG, readl(CCU_USB0_GAR_REG));

			val = readl(USB_RST_CTRL_REG(0));
			LOG("[0] 0x%x: 0x%x\n", USB_RST_CTRL_REG(0), val);
			val |= USB_PHY_RST_MASK;
			writel(val, USB_RST_CTRL_REG(0));
			LOG("[1] 0x%x: 0x%x\n", USB_RST_CTRL_REG(0), readl(USB_RST_CTRL_REG(0)));
		}

		if (usb1_en) {
			val = readl(CCU_USB1_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB1_GAR_REG, val);
			val |= USB1_EHCI_RST_MASK;
			val |= USB1_OHCI_RST_MASK;
			writel(val, CCU_USB1_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB1_GAR_REG, readl(CCU_USB1_GAR_REG));

			val = readl(USB_RST_CTRL_REG(1));
			LOG("[0] 0x%x: 0x%x\n", USB_RST_CTRL_REG(1), val);
			val |= USB_PHY_RST_MASK;
			writel(val, USB_RST_CTRL_REG(1));
			LOG("[1] 0x%x: 0x%x\n", USB_RST_CTRL_REG(1), readl(USB_RST_CTRL_REG(1)));
		}

		/* The isolation is necessary */
		val = readl(VDD_SYS_PWROFF_GATING_REG);
		LOG("[0] 0x%x: 0x%x\n", VDD_SYS_PWROFF_GATING_REG, val);
		val &= ~VDD_SYS2USB_GATING(1);
		writel(val, VDD_SYS_PWROFF_GATING_REG);
		LOG("[1] 0x%x: 0x%x\n", VDD_SYS_PWROFF_GATING_REG, readl(VDD_SYS_PWROFF_GATING_REG));

		if (usb_en) {
			val = readl(CCU_USB2P0_SYS_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB2P0_SYS_GAR_REG, val);
			val |= USB2P0_SYS_AHB_CLK_MASK;
			writel(val, CCU_USB2P0_SYS_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB2P0_SYS_GAR_REG, readl(CCU_USB2P0_SYS_GAR_REG));

			val = readl(CCU_USB2P0_SYS_PHY_REF_CLK_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB2P0_SYS_PHY_REF_CLK_REG, val);
			val |= CCU_USB2P0_SYS_PHY_REF_CLK_MASK;
			writel(val, CCU_USB2P0_SYS_PHY_REF_CLK_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB2P0_SYS_PHY_REF_CLK_REG, readl(CCU_USB2P0_SYS_PHY_REF_CLK_REG));
		}

		if (usb0_en) {
			val = readl(CCU_USB0_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB0_GAR_REG, val);
			val |= USB0_OHCI_GATING_MASK;
			val |= USB0_EHCI_GATING_MASK;
			writel(val, CCU_USB0_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB0_GAR_REG, readl(CCU_USB0_GAR_REG));

			val = readl(CCU_USB0_CLK_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB0_CLK_REG, val);
			val |= USB0_CLKEN_MASK;
			writel(val, CCU_USB0_CLK_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB0_CLK_REG, readl(CCU_USB0_CLK_REG));
		}

		if (usb1_en) {
			val = readl(CCU_USB1_GAR_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB1_GAR_REG, val);
			val |= USB1_OHCI_GATING_MASK;
			val |= USB1_EHCI_GATING_MASK;
			writel(val, CCU_USB1_GAR_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB1_GAR_REG, readl(CCU_USB1_GAR_REG));

			val = readl(CCU_USB1_CLK_REG);
			LOG("[0] 0x%x: 0x%x\n", CCU_USB1_CLK_REG, val);
			val |= USB1_CLKEN_MASK;
			writel(val, CCU_USB1_CLK_REG);
			LOG("[1] 0x%x: 0x%x\n", CCU_USB1_CLK_REG, readl(CCU_USB1_CLK_REG));
		}
	}
}

static bool is_chip_secure(void)
{
	return !!((readl(SID_SEC_MODE_STA) & SID_SEC_MODE_MASK));
}

static struct {
	u32 region_low;
	u32 region_high;
	u32 region_attr;
} smc_region_save[SMC_REGION_COUNT];

static void smc_write_reg(uint32_t val, unsigned long paddr)
{
	writel(val, paddr);
#if defined(SUNXI_SMC1_PBASE)
	/* The configuration of SMC1 and SMC0 needs to be exactly the same */
	writel(val, paddr + (SUNXI_SMC1_PBASE - SUNXI_SMC_PBASE));
#endif
}

static void sunxi_smc_en(void)
{
	/* Set SMC_FUNCTION_BYPASS = NO_BYPASS */
	smc_write_reg(0x0, SMC_ACTION_REG);
}

#if defined(CFG_SUNXI_SMC_GLITCH_WORKAROUND)
static bool sunxi_smc_need_glitch_workaround(void)
{
	return is_chip_secure();
}

/*
 * This function is called from libdram.
 */
void sunxi_smc_en_with_glitch_workaround(void)
{
	if (!sunxi_smc_need_glitch_workaround())
		return;
	LOG("SMC Driver clk glitch workaround in arisc\n");
	sunxi_smc_en();
}
#endif /* CFG_SUNXI_SMC_GLITCH_WORKAROUND */

static void smc_standby_init(void)
{
	int read_idx;
	for (read_idx = 0; read_idx < SMC_REGION_COUNT; read_idx++) {
		if (readl(SMC_REGION_ATTRIBUTE_REG(read_idx)) == 0)
			break;
		LOG("SMC Driver: Save region %d\n", read_idx);
		smc_region_save[read_idx].region_low =
		    readl(SMC_REGION_SETUP_LOW_REG(read_idx));
		smc_region_save[read_idx].region_high =
		    readl(SMC_REGION_SETUP_HIGH_REG(read_idx));
		smc_region_save[read_idx].region_attr =
		    readl(SMC_REGION_ATTRIBUTE_REG(read_idx));
	}
}

static void smc_standby_exit(void)
{
	int read_idx, resume_idx;

	if (!is_chip_secure()) {
		LOG("SMC Driver: non-secure chip\n");
		return;
	}

	/* 1. Enable smc control */
#if !defined(CFG_SUNXI_SMC_GLITCH_WORKAROUND)
	sunxi_smc_en();
#endif
	/* Make sure SMC is NOT bypassed accidentally */
	if (readl(SMC_ACTION_REG) & SMC_FUNCTION_BYPASS) {
		printk("SMC Driver is not enabled!\n");
		while (1) {
		}
	}

	/* Disable SMC-BYPASS for all masters: all masters (cpu|gpu...) must access dram through SMC */
	smc_write_reg(0x0, SMC_MST0_BYP_REG);
	smc_write_reg(0x0, SMC_MST1_BYP_REG);
	/* Set all masters (cpu|gpu...) to non-secure: Secure permission must be set explicitly later. */
	smc_write_reg(0xffffffff, SMC_MST0_SEC_REG);
	smc_write_reg(0xffffffff, SMC_MST1_SEC_REG);
#if defined(SUNXI_SMC_HAS_ITLV)
	/* Set SMC interleave. See spl/drivers/secure/smc.c: sunxi_smc_config_interleave() */
	smc_write_reg((1 << 7) | (0x01 << 4), SMC_ITLV_CTRL_REG);   //0x90: 128B, uniform interleave
#endif

	/* 2. resume region settings */
	resume_idx = 0;
	for (read_idx = 0; read_idx < SMC_REGION_COUNT; read_idx++) {
		if (smc_region_save[read_idx].region_attr == 0)
			break;
		LOG("SMC Driver: Restore region %d\n", resume_idx);
		smc_write_reg(smc_region_save[resume_idx].region_low,
			      SMC_REGION_SETUP_LOW_REG(resume_idx));
		smc_write_reg(smc_region_save[resume_idx].region_high,
			      SMC_REGION_SETUP_HIGH_REG(resume_idx));
		smc_write_reg(smc_region_save[resume_idx].region_attr,
			      SMC_REGION_ATTRIBUTE_REG(resume_idx));
		resume_idx++;
	}
}

static void device_suspend(void)
{
	usb_standby_init();
	sunxi_timestamp_suspend();
	smc_standby_init();
	pmu_standby_init();
	twi_standby_init();
	hwmsgbox_super_standby_init();
}

static void device_resume(void)
{
	hwmsgbox_super_standby_exit();
	twi_standby_exit();
	pmu_standby_exit();
	smc_standby_exit();
	sunxi_timestamp_resume();
	usb_standby_exit();
}

static void all_pll_set(enum pll_state sta)
{
	u32 reg_val, i;

	for (i = 0; i < (sizeof(pll_restore) / sizeof(pll_restore[0])); i++) {
		if (sta == pll_disable) {
			/* save PLLs SET */
			reg_val = readl(pll_restore[i].addr);
			pll_restore[i].factor = reg_val & PLL_FACKOR_MASK;

			/* disable PLLs */
			writel((readl(pll_restore[i].addr) & (~PLL_ENABLE_MASK)), pll_restore[i].addr);
			writel((readl(pll_restore[i].addr) & (~PLL_LDO_ENABLE_MASK)), pll_restore[i].addr);
			writel((readl(pll_restore[i].addr) & (~PLL_LOCK_ENABLE_MASK)), pll_restore[i].addr);
		} else if (sta == pll_enable) {
			reg_val = readl(pll_restore[i].addr);

			/* set n m p */
			reg_val &= ~PLL_FACKOR_MASK;
			reg_val |= pll_restore[i].factor;
			writel(reg_val, pll_restore[i].addr);

			/* set pll_en & pll_ldo_en, disable out_put_en */
			reg_val = readl(pll_restore[i].addr);
			reg_val |= 1 << PLL_ENABLE_SHIFT;
			reg_val |= 1 << PLL_LDO_ENABLE_SHIFT;
			reg_val &= ~PLL_OUTPUT_ENABLE_MASK;
			writel(reg_val, pll_restore[i].addr);

			/* set lock_en */
			reg_val = readl(pll_restore[i].addr);
			reg_val |= 1 << PLL_LOCK_ENABLE_SHIFT;
			writel(reg_val, pll_restore[i].addr);
			while (!(readl(pll_restore[i].addr) & PLL_LOCK_STATUS_MASK))
				;
		}
	}
	if (sta == pll_enable) {
		time_udelay(20);
		for (i = 0; i < (sizeof(pll_restore) / sizeof(pll_restore[0])); i++) {
			reg_val = readl(pll_restore[i].addr);
			reg_val |= 1 << PLL_OUTPUT_ENABLE_SHIFT;
			writel(reg_val, pll_restore[i].addr);
		}
	}
}

static void bus_clock_set(enum bus_clock_mode mode, enum bus_clock_mode bus)
{
	u32 reg_val, bus_tick, start_tick, end_tick;
	if (bus == ccmu_clk) {
		start_tick = bus_start_tick;
		end_tick = bus_end_tick;
	} else {
		start_tick = prcm_start_tick;
		end_tick = prcm_end_tick;
	}
	for (bus_tick = start_tick; bus_tick < end_tick; bus_tick++) {
		reg_val = readl(bus_restore[bus_tick].busAddr);
		if (mode == clock_bak && bus_restore[bus_tick].flag != 0) {
			reg_val |=  bus_restore[bus_tick].M << CCU_FACTOR_M_SHIFT;
			writel(reg_val, bus_restore[bus_tick].busAddr);
			reg_val &= ~(CCU_CLK_SRC_SEL_MASK);
			reg_val |=  bus_restore[bus_tick].bus_clk_src << CCU_CLK_SRC_SEL_SHIFT;
			writel(reg_val, bus_restore[bus_tick].busAddr);
			bus_restore[bus_tick].flag = 0;
		} else {
			if (bus_restore[bus_tick].flag == 0) {
				bus_restore[bus_tick].bus_clk_src = (reg_val & CCU_CLK_SRC_SEL_MASK) >> CCU_CLK_SRC_SEL_SHIFT;
				bus_restore[bus_tick].M = (reg_val & CCU_FACTOR_M_MASK) >> CCU_FACTOR_M_SHIFT;
				bus_restore[bus_tick].flag = 1;
			}
			reg_val &= ~(CCU_CLK_SRC_SEL_MASK);
			reg_val |= (mode << CCU_CLK_SRC_SEL_SHIFT);
			writel(reg_val, bus_restore[bus_tick].busAddr);
			reg_val &= ~(CCU_FACTOR_M_MASK);
			writel(reg_val, bus_restore[bus_tick].busAddr);
		}
	}
}

static void bus_clock_ctl(enum bus_clock_mode mode, enum bus_clock_mode control)
{
	u32 reg_val, bus_tick, start_tick, end_tick;
	if (control == bus_clk) {
		start_tick = nm_start_tick;
		end_tick = nm_end_tick;
	} else {
		start_tick = gc_start_tick;
		end_tick = gc_end_tick;
	}
	for (bus_tick = start_tick; bus_tick < end_tick; bus_tick++) {
		reg_val = readl(bus_ctl_restore[bus_tick].busAddr);
		if (mode == clock_bak) {
			reg_val = readl(bus_ctl_restore[bus_tick].busAddr);
			reg_val &= (~(1 << 31));
			reg_val |= (1 << 31);
			writel(reg_val, bus_ctl_restore[bus_tick].busAddr);

			reg_val |=  bus_ctl_restore[bus_tick].M;
			reg_val |= (1 << 27);
			writel(reg_val, bus_ctl_restore[bus_tick].busAddr);

			reg_val &= ~(0x7 << 24);
			reg_val |=  bus_ctl_restore[bus_tick].bus_clk_src << 24;
			reg_val |= (1 << 27);
			writel(reg_val, bus_ctl_restore[bus_tick].busAddr);

		} else {
			bus_ctl_restore[bus_tick].bus_clk_src = (reg_val & (0x7 << 24)) >> 24;
			bus_ctl_restore[bus_tick].M = (reg_val & 0x3ff);

			reg_val = readl(bus_ctl_restore[bus_tick].busAddr);

			reg_val = readl(bus_ctl_restore[bus_tick].busAddr);
			reg_val &= (~(0x7 << 24));
			writel(reg_val, bus_ctl_restore[bus_tick].busAddr);
			reg_val &= (~(0x1f << 0));
			writel(reg_val, bus_ctl_restore[bus_tick].busAddr);
			reg_val &= (~(1 << 31));
			reg_val |= (0 << 31);
			writel(reg_val, bus_ctl_restore[bus_tick].busAddr);

		}
	}
}

static void clk_suspend(void)
{
	/* change the rv 24M to RC16M */
	writel(0x82000000, RV_24M_CLK_REG);

	/* set ahb apb0 apb1 cpus(ahb) apbs0 clk to RC16M, clear factor m */
	bus_clock_set(rc16m, ccmu_clk);

	/* close ccmu nsi and mbus clk */
	bus_clock_ctl(rc16m, bus_clk);

	/* close trace and gic clock source */
	bus_clock_ctl(rc16m, gic_cci_clk);

	LOG("clk_suspend\n");

	/*
	 * set apbs1 clk to RC16M, clear factor m
	 * then change the baudrate of uart and twi...
	 */
	twi_clkchangecb(CCU_CLK_CLKCHG_REQ, iosc_freq);
	uart_clkchangecb(CCU_CLK_CLKCHG_REQ, iosc_freq);
	/* close prcm cpus apbs1 apbs2 clk to RC16M, clear factor m */
	bus_clock_set(rc16m, prcm_clk);
	time_mdelay(10);
	uart_clkchangecb(CCU_CLK_CLKCHG_DONE, iosc_freq);
	twi_clkchangecb(CCU_CLK_CLKCHG_DONE, iosc_freq);
	time_mdelay(10);

	/* disable PLLs */
	all_pll_set(pll_disable);
	dcxo_disable();
}

static void clk_suspend_late(void)
{
	return;
}

static void clk_resume_early(void)
{
	dcxo_enable();
}

static void clk_resume(void)
{
	all_pll_set(pll_enable);
	/*
	 * set apbs1 clk to 24M
	 * then change the baudrate of uart and twi...
	 */
	twi_clkchangecb(CCU_CLK_CLKCHG_REQ, CCU_HOSC_FREQ);
	uart_clkchangecb(CCU_CLK_CLKCHG_REQ, CCU_HOSC_FREQ);
	bus_clock_set(clock_bak, prcm_clk);
	time_mdelay(10);
	uart_clkchangecb(CCU_CLK_CLKCHG_DONE, CCU_HOSC_FREQ);
	twi_clkchangecb(CCU_CLK_CLKCHG_DONE, CCU_HOSC_FREQ);
	time_mdelay(10);

	/* restore ahb apb0 apb1 cpus(ahb) apbs0 clk */
	bus_clock_ctl(clock_bak, gic_cci_clk);
	bus_clock_ctl(clock_bak, bus_clk);
	bus_clock_set(clock_bak, ccmu_clk);

	/* change the rv 24M to HOSC */
	writel(0x80000000, RV_24M_CLK_REG);
}

/*standby power off cpu*/
static void cpu_pll_off(void)
{
	int i;

	LOG("cpu off \n");

	/* backup and set cpu clk rc16m */
	for (i = 0; i < CPU_PLL_REG_BAK_NUM; i++) {
		cpu_clk_restore_bak[i] = readl(CPU_CLK_REG(i));
		writel(((readl(CPU_CLK_REG(i)) & (~CPU_CLK_SRC_SEL_MASK)) | CPU_CLK_SRC_SEL(2)), CPU_CLK_REG(i));
	}

	/* disable cpu pll */
	for (i = 0; i < CPU_PLL_REG_BAK_NUM; i++) {
		cpu_pll_pat0_restore_bak[i] = readl(CPU_PLL_PAT0_REG(i));
		cpu_pll_ssc_restore_bak[i] = readl(CPU_PLL_SSC_REG(i));
		cpu_pll_restore_bak[i] = readl(CPU_PLL_REG(i));

		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_OUTPUT_MASK)) | CPU_PLL_OUTPUT(0)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_LOCK_EN_MASK)) | CPU_PLL_LOCK_EN(0)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_EN_MASK)) | CPU_PLL_EN(0)), CPU_PLL_REG(i));
		writel((readl((CPU_PLL_REG(i)) & (~CPU_PLL_LDO_EN_MASK)) | CPU_PLL_LDO_EN(0)), CPU_PLL_REG(i));
	}
}

/*standby power on cpu*/
static void cpu_pll_on(void)
{
	int i;

	LOG("cpu on \n");
	/* enable cpu pll */
	for (i = 0; i < CPU_PLL_REG_BAK_NUM; i++) {
		/* set pll on */
		writel((readl((CPU_PLL_REG(i)) & (~CPU_PLL_LDO_EN_MASK)) | CPU_PLL_LDO_EN(1)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_OUTPUT_MASK)) | CPU_PLL_OUTPUT(1)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_EN_MASK)) | CPU_PLL_EN(1)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_LOCK_EN_MASK)) | CPU_PLL_LOCK_EN(1)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_UPDATE_MASK)) | CPU_PLL_UPDATE(1)), CPU_PLL_REG(i));
		while ((readl(CPU_PLL_REG(i)) & CPU_PLL_UPDATE_MASK))
			;

		while (!(readl(CPU_PLL_REG(i)) & CPU_PLL_LOCK_STATUS_MASK))
			;
	}

	time_mdelay(20);

	/* set ssc and enable ssc mode */
	for (i = 0; i < CPU_PLL_REG_BAK_NUM; i++) {
		writel(((readl(CPU_PLL_PAT0_REG(i)) & (~(0x3 << 29))) | (cpu_pll_pat0_restore_bak[i] & (0x3 << 29))), CPU_PLL_PAT0_REG(i));
		writel(((readl(CPU_PLL_SSC_REG(i)) & (~(0x1ffff << 12))) | (cpu_pll_ssc_restore_bak[i] & (0x1ffff << 12))), CPU_PLL_SSC_REG(i));
		writel(((readl(CPU_PLL_SSC_REG(i)) & (~(0xf << 0))) | (cpu_pll_ssc_restore_bak[i] & (0xf << 0))), CPU_PLL_SSC_REG(i));
		writel((readl(CPU_PLL_SSC_REG(i)) | (0x1 << 31)), CPU_PLL_SSC_REG(i));
	}

	for (i = 0; i < CPU_PLL_REG_BAK_NUM; i++) {
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_OUTPUT_MASK)) | CPU_PLL_OUTPUT(0)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_FACTOR_MASK)) | (cpu_pll_restore_bak[i] & CPU_PLL_FACTOR_MASK)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_LOCK_EN_MASK)) | CPU_PLL_LOCK_EN(0)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_LOCK_EN_MASK)) | CPU_PLL_LOCK_EN(1)), CPU_PLL_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_UPDATE_MASK)) | CPU_PLL_UPDATE(1)), CPU_PLL_REG(i));
		while ((readl(CPU_PLL_REG(i)) & CPU_PLL_UPDATE_MASK))
			;

		while (!(readl(CPU_PLL_REG(i)) & CPU_PLL_LOCK_STATUS_MASK))
			;

		time_mdelay(20);
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_OUTPUT_MASK)) | CPU_PLL_OUTPUT(1)), CPU_PLL_REG(i));
	}

	/* disable ssc mode */
	for (i = 0; i < CPU_PLL_REG_BAK_NUM; i++) {
		writel((readl(CPU_PLL_SSC_REG(i)) & (~(0x1 << 31))), CPU_PLL_SSC_REG(i));
		writel(((readl(CPU_PLL_REG(i)) & (~CPU_PLL_UPDATE_MASK)) | CPU_PLL_UPDATE(1)), CPU_PLL_REG(i));
		while ((readl(CPU_PLL_REG(i)) & CPU_PLL_UPDATE_MASK))
			;
	}

	/* set cpu clk */
	for (i = 0; i < CPU_PLL_REG_BAK_NUM; i++) {
		writel(((readl(CPU_CLK_REG(i)) & (~CPU_CLK_SRC_SEL_MASK)) | CPU_CLK_SRC_SEL(3)), CPU_CLK_REG(i));
	}
}

static void system_suspend(void)
{
	/* Analog Domain ISO Control */
	writel(readl(ANA_PWR_RST_REG) | (0x1 << 0) | (0x1 << 1) | (0x1 << 3), ANA_PWR_RST_REG);

	/* USB isolation */
	usb_iso_suspend();

	/* System reset control */
	writel(readl(VDD_SYS_PWR_RST_REG) & (~(0x1 << 0)), VDD_SYS_PWR_RST_REG);

	/* Digital Domain ISO Control */
	writel(readl(VDD_SYS_PWROFF_GATING_REG) | (0x1 << 2), VDD_SYS_PWROFF_GATING_REG);
}

static void nsi_resume(void)
{
	//nsi TAG_N credit count
	writel(0x40005, 0x2402C00 + 0x6c);

	//RA_N FIFO Mode Tx Register
	writel(0xFF, 0x2402600 + 0x14);
	writel(0xFF, 0x2402800 + 0x14);
	writel(0xFF, 0x2402a00 + 0x14);

	// isp rtlpr
	writel(0x1, 0x2400000 + 0xc);
	// csi rtlpr
	writel(0x1, 0x2400200 + 0xc);
}

static void system_resume(void)
{
	/* Digital Domain ISO Control */
	writel(readl(VDD_SYS_PWROFF_GATING_REG) & (~(0x1 << 2)), VDD_SYS_PWROFF_GATING_REG);

	/* System reset control */
	writel(readl(VDD_SYS_PWR_RST_REG) | (0x1 << 0), VDD_SYS_PWR_RST_REG);

	/* USB isolation */
	usb_iso_resume();

	/* Analog Domain ISO Control */
	writel(readl(ANA_PWR_RST_REG) & (~(0x1 << 0)) & (~(0x1 << 1)) & (~(0x1 << 3)), ANA_PWR_RST_REG);
}

static void rtc_vccio_det_suspend(void)
{
	u32 val = 0;

	/* disable vcc-io detect */
	val = readl(RTC_VDD_OFF_GATING_CTRL);
	val |= RTC_VCCIO_DETECT_EN;
	writel(val, RTC_VDD_OFF_GATING_CTRL);

	/* disable vcc-io detect bypass */
	val = readl(RTC_VDD_OFF_GATING_CTRL);
	val &= ~(RTC_VCCIO_OUTPUT_EN);
	writel(val, RTC_VDD_OFF_GATING_CTRL);
}

static void rtc_vccio_det_resume(void)
{
	u32 val = 0;

	/* enable vcc-io detect */
	val = readl(RTC_VDD_OFF_GATING_CTRL);
	val &= ~(RTC_VCCIO_DETECT_EN);
	writel(val, RTC_VDD_OFF_GATING_CTRL);

	/* enable vcc-io output */
	val = readl(RTC_VDD_OFF_GATING_CTRL);
	val |= RTC_VCCIO_OUTPUT_EN;
	writel(val, RTC_VDD_OFF_GATING_CTRL);
}

static u32 platform_standby_type(void)
{
	u32 type = 0;

	/* usb standby */
	if (interrupt_get_enabled(INTC_R_USB_IRQ)) {
		type |= CPUS_WAKEUP_USB;
	}

	return type;
}

static s32 standby_process_init(struct message *pmessage)
{
	suspend_lock = 1;

	suspend_para_prepare();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x1);

	cpucfg_cpu_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x2);

	//hexdump("CCU", (char *)(CCU_REG_BASE), 0x1ff0);
	//hexdump("PRCM", (char *)(R_PRCM_REG_BASE), 0xff0);
	//hexdump("CPU_PLL", (char *)(CPU_PLL_REG_BASE), 0x50c);
	//hexdump("CPUIDLE_CFG_REG_BASE", (char *)(CPUIDLE_CFG_REG_BASE), 0xb0);
	//hexdump("CPUIDLE_CLU0_CFG_REG_BASE", (char *)(CPUIDLE_CLU0_CFG_REG_BASE), 0x2f4);
	//hexdump("CPUIDLE_CLU1_CFG_REG_BASE", (char *)(CPUIDLE_CLU1_CFG_REG_BASE), 0x2f4);
	//hexdump("CPUIDLE_CCI_PPU_CFG_REG_BASE", (char *)(CPUIDLE_CCI_PPU_CFG_REG_BASE), 0xc40);

	device_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x3);
	cpu_pll_off();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x4);

	dram_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x5);

	clk_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x6);

	system_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x7);

	clk_suspend_late();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x8);

	rtc_vccio_det_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x9);

	dm_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0xA);

	return OK;
}

static s32 standby_process_exit(struct message *pmessage)
{
	u32 resume_entry = pmessage->paras[1];

	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x1);

	dm_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x2);

	rtc_vccio_det_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x3);

	clk_resume_early();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x4);

	system_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x5);

	clk_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x6);

	dram_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x7);

	cpu_pll_on();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x8);

	device_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x9);

	//hexdump("CCU", (char *)(CCU_REG_BASE), 0x1ff0);
	//hexdump("PRCM", (char *)(R_PRCM_REG_BASE), 0xff0);
	//hexdump("CPU_PLL", (char *)(CPU_PLL_REG_BASE), 0x50c);
	//hexdump("CPUIDLE_CFG_REG_BASE", (char *)(CPUIDLE_CFG_REG_BASE), 0xb0);
	//hexdump("CPUIDLE_CLU0_CFG_REG_BASE", (char *)(CPUIDLE_CLU0_CFG_REG_BASE), 0x2f4);
	//hexdump("CPUIDLE_CLU1_CFG_REG_BASE", (char *)(CPUIDLE_CLU1_CFG_REG_BASE), 0x2f4);
	//hexdump("CPUIDLE_CCI_PPU_CFG_REG_BASE", (char *)(CPUIDLE_CCI_PPU_CFG_REG_BASE), 0xc40);

	cpucfg_cpu_resume(resume_entry);
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xA);

	wait_cpu0_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xB);

	nsi_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xC);

	suspend_lock = 0;

	return OK;
}

/*
*********************************************************************************************************
*                                       ENTEY OF TALK-STANDBY
*
* Description:  the entry of extended super-standby.
*
* Arguments  :  request:request command message.
*
* Returns    :  OK if enter extended super-standby succeeded, others if failed.
*********************************************************************************************************
*/
static s32 standby_entry(struct message *pmessage)
{
	save_state_flag(REC_ESTANDBY | REC_ENTER);

	standby_dts_parse();

	save_state_flag(REC_ESTANDBY | REC_ENTER | 0x1);

	/* backup cpus source clock */
	iosc_freq_init();

	/* parse standby type from enabled interrupt */
	standby_type = platform_standby_type();

	/*
	 * --------------------------------------------------------------------------
	 *
	 * initialize enter super-standby porcess
	 *
	 * --------------------------------------------------------------------------
	 */
	save_state_flag(REC_ESTANDBY | REC_BEFORE_INIT);
	result = standby_process_init(pmessage);
	save_state_flag(REC_ESTANDBY | REC_AFTER_INIT);

	/*
	 * --------------------------------------------------------------------------
	 *
	 * wait valid wakeup source porcess
	 *
	 * --------------------------------------------------------------------------
	 */
	LOG("wait wakeup\n");
	save_state_flag(REC_ESTANDBY | REC_WAIT_WAKEUP);

	wait_wakeup();

	/*
	 * --------------------------------------------------------------------------
	 *
	 * exit super-standby wakeup porcess
	 *
	 * --------------------------------------------------------------------------
	 */
	save_state_flag(REC_ESTANDBY | REC_BEFORE_EXIT);
	standby_process_exit(pmessage);
	save_state_flag(REC_ESTANDBY | REC_AFTER_EXIT);

	return OK;
}

u32 is_suspend_lock(void)
{
	return suspend_lock;
}

#define MEM_MSG_RTC_WAY
#ifdef MEM_MSG_RTC_WAY
#define MEM_MSG_BASE_ADDR	(RTC_REG_BASE + 0x110)
#else
#define SCP_DRAM_BASE		(0x48100000)
#define MEM_MSG_BASE_ADDR	(SCP_DRAM_BASE)
#endif

/* only support cluster 2 core 0/1 */
#define MEM_MSG_CPU_MAP_CHANNEL_ADDR(n)		(MEM_MSG_BASE_ADDR + (n) * 4)

enum {
	SUNXI_CPUIDLE_STAGE_DONE = 0,
	SUNXI_CPUIDLE_STAGE_INIT,
	SUNXI_CPUIDLE_STAGE_PROCESS,
};

static void arisc_cluster2_cpu_done(unsigned int cpu)
{
	writel(SUNXI_CPUIDLE_STAGE_DONE, MEM_MSG_CPU_MAP_CHANNEL_ADDR(cpu));
}

void plat_init_early(void)
{
	arisc_cluster2_cpu_done(0);
	arisc_cluster2_cpu_done(1);
}

static void arisc_cluster2_cpu_op(unsigned int cpu)
{
	if (readl(MEM_MSG_CPU_MAP_CHANNEL_ADDR(cpu)) != SUNXI_CPUIDLE_STAGE_INIT)
		return;

	writel(SUNXI_CPUIDLE_STAGE_PROCESS, MEM_MSG_CPU_MAP_CHANNEL_ADDR(cpu));

	while ((readl(0x070032e0 + cpu * 4) & 0x40) == 0)
		;
	writel(readl(0x09810060 + cpu * 4) & ~0x1, (0x09810060 + cpu * 4));
	writel(readl(0x07003070 + cpu * 4) | 0x30, (0x07003070 + cpu * 4));

	writel(SUNXI_CPUIDLE_STAGE_DONE, MEM_MSG_CPU_MAP_CHANNEL_ADDR(cpu));
}

void plat_process_loop(void)
{
	arisc_cluster2_cpu_op(0);
	arisc_cluster2_cpu_op(1);
}

int cpu_op(struct message *pmessage)
{
	u32 mpidr = pmessage->paras[0];
	u32 entrypoint = pmessage->paras[1];
	u32 cpu_state = pmessage->paras[2];
	u32 cluster_state = pmessage->paras[3]; /* unused variable */
	u32 system_state = pmessage->paras[4];

	LOG("mpidr:%x, entrypoint:%x; cpu_state:%x, cluster_state:%x, system_state:%x\n", mpidr, entrypoint, cpu_state, cluster_state, system_state);
	if (entrypoint && system_state == arisc_power_off) {
		standby_entry(pmessage);
	}

	return 0;
}

static void system_shutdown(void)
{
	pmu_shutdown();
}

static void system_reset(void)
{
	pmu_reset();
}

int sys_op(struct message *pmessage)
{
	u32 state = pmessage->paras[0];

	LOG("state:%x\n", state);

	switch (state) {
	case arisc_system_shutdown:
		{
			save_state_flag(REC_SHUTDOWN | 0x101);
			pmu_charging_reset();
			system_shutdown();
			break;
		}
	case arisc_system_reset:
	case arisc_system_reboot:
		{
			save_state_flag(REC_SHUTDOWN | 0x102);
			system_reset();
			break;
		}
	case arisc_uboot_shutdown:
		{
			save_state_flag(REC_SHUTDOWN | 0x103);
			system_shutdown();
			break;
		}
	default:
		{
			WRN("invaid system power state (%d)\n", state);
			return -EINVAL;
		}
	}

	return 0;
}

s32 fake_poweroff(struct message *pmessage)
{
	return 0;
}

/* feedback pmu irq */
s32 get_pmu_irq(struct message *pmessage)
{
	return OK;
}
