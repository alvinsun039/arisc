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
#include <driver/smc.h>

#ifdef CFG_HDMIRX_USED
#define CEC_WAKE_FLAG     (0xA1)
#endif

#define RES_KEY_FIELD			(0x19380000)

extern u32 dram_crc_enable;
extern u32 dram_crc_src;
extern u32 dram_crc_len;
extern u32 volatile wakeup_source;
extern u32 volatile arm_is_die;
extern u32 axp_power_max;
extern u32 dtb_base;

static s32 result;
static u32 suspend_lock;
static u32 standby_type;
static u32 usb_standby_port;

static u32 hdmi_res1;

/* standby paras */
static uint32_t standby_vdd_cpu;
static uint32_t standby_vdd_sys;
static uint32_t standby_vdd_gpu;
static uint32_t standby_vcc_pll;
static uint32_t standby_vcc_io;
static uint32_t standby_vcc_nor;
static uint32_t standby_osc24m_on = 1;

/* dram para */
extern uint32_t dts_dram_para[96];

/* pmu ext check */
static uint32_t pmu_ext_power_max = -1;

/* timer_stamp paras */
static uint32_t timestamp_cnt_low_bak;
static uint32_t timestamp_cnt_hi_bak;
static uint32_t timestamp_cnt_freqid_bak;
static uint64_t rtc_last_time;

/* backup cpu PLL */
#define CPU_PLL_REG_BAK_NUM (2)
static u32 cpu_clk_restore_bak[CPU_PLL_REG_BAK_NUM] = {0};
static u32 cpu_pll_restore_bak[CPU_PLL_REG_BAK_NUM] = {0};
static u32 cpu_pll_pat0_restore_bak[CPU_PLL_REG_BAK_NUM] = {0};
static u32 cpu_pll_ssc_restore_bak[CPU_PLL_REG_BAK_NUM] = {0};

static u32 vdd_cpu_grp;
static u32 vdd_cpu_num;
static u32 vdd_ve_grp;
static u32 vdd_ve_num;
static u32 vdd_sys_grp;
static u32 vdd_sys_num;
static u32 vcc_dram_grp;
static u32 vcc_dram_num;
static u32 vcc_io_grp;
static u32 vcc_io_num;
static u32 gmac_wakeup_grp;
static u32 gmac_wakeup_num;

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
	gc_end_tick = 3
};

enum pll_state {
	pll_disable = 0,
	pll_enable  = 1
};

u8 get_vdd_sys_reg(void)
{
	return 0;
}

int set_vdd_sys_reg(int set_vol, int onoff)
{
	return 0;
}

static void device_special_handle(struct message *pmessage)
{
#ifdef CFG_HDMITX_USED
	hdmitx_update_type(pmessage->type);
#endif
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

	int len;
	const char *pin_char;
	char pin_num_string[4];

	const fdt32_t *gmac_intr;

	if (dts_has_parsed)
		return 0;

	fdt = (void *)(dtb_base);

	/* parse power tree */
	param_node = fdt_path_offset(fdt, "standby_param");
	if (param_node < 0) {
		WRN("no standby_param: %x fdt:%x\n", param_node, fdt);
		return -1;
	}

	fdt_getprop_u32(fdt, param_node, "vdd-cpu", &standby_vdd_cpu);
	fdt_getprop_u32(fdt, param_node, "vdd-sys", &standby_vdd_sys);
	fdt_getprop_u32(fdt, param_node, "vdd-gpu", &standby_vdd_gpu);
	fdt_getprop_u32(fdt, param_node, "vcc-pll", &standby_vcc_pll);
	fdt_getprop_u32(fdt, param_node, "vcc-io", &standby_vcc_io);
	fdt_getprop_u32(fdt, param_node, "vcc-nor", &standby_vcc_nor);
	fdt_getprop_u32(fdt, param_node, "osc24m-on", &standby_osc24m_on);

	LOG("standby_vdd_cpu 0x%x\n", standby_vdd_cpu);
	LOG("standby_vdd_sys 0x%x\n", standby_vdd_sys);
	LOG("standby_vdd_gpu 0x%x\n", standby_vdd_gpu);
	LOG("standby_vcc_pll 0x%x\n", standby_vcc_pll);
	LOG("standby_vcc_io 0x%x\n", standby_vcc_io);
	LOG("standby_vcc_nor 0x%x\n", standby_vcc_nor);
	LOG("standby_osc24m_on 0x%x\n", standby_osc24m_on);

	/* parse vdd-cpu-gpio */
	pin_char = fdt_stringlist_get(fdt, param_node, "vdd-cpu-gpio", 0, &len);
	if (strncmp(pin_char, "PL", strlen("PL")) == 0) {
		vdd_cpu_grp = PIN_GRP_PL;
		strncpy(pin_num_string, pin_char + strlen("PL"), 2);
		vdd_cpu_num = dstr2int(pin_num_string, 2);
	} else if (strncmp(pin_char, "PM", strlen("PM")) == 0) {
		vdd_cpu_grp = PIN_GRP_PM;
		strncpy(pin_num_string, pin_char + strlen("PM"), 2);
		vdd_cpu_num = dstr2int(pin_num_string, 2);
	}

	/* parse vdd-ve-gpio */
	pin_char = fdt_stringlist_get(fdt, param_node, "vdd-ve-gpio", 0, &len);
	if (strncmp(pin_char, "PL", strlen("PL")) == 0) {
		vdd_ve_grp = PIN_GRP_PL;
		strncpy(pin_num_string, pin_char + strlen("PL"), 2);
		vdd_ve_num = dstr2int(pin_num_string, 2);
	} else if (strncmp(pin_char, "PM", strlen("PM")) == 0) {
		vdd_ve_grp = PIN_GRP_PM;
		strncpy(pin_num_string, pin_char + strlen("PM"), 2);
		vdd_ve_num = dstr2int(pin_num_string, 2);
	}

	/* parse vdd-sys-gpio */
	pin_char = fdt_stringlist_get(fdt, param_node, "vdd-sys-gpio", 0, &len);
	if (strncmp(pin_char, "PL", strlen("PL")) == 0) {
		vdd_sys_grp = PIN_GRP_PL;
		strncpy(pin_num_string, pin_char + strlen("PL"), 2);
		vdd_sys_num = dstr2int(pin_num_string, 2);
	} else if (strncmp(pin_char, "PM", strlen("PM")) == 0) {
		vdd_sys_grp = PIN_GRP_PM;
		strncpy(pin_num_string, pin_char + strlen("PM"), 2);
		vdd_sys_num = dstr2int(pin_num_string, 2);
	}

	/* parse vcc-dram-gpio */
	pin_char = fdt_stringlist_get(fdt, param_node, "vcc-dram-gpio", 0, &len);
	if (strncmp(pin_char, "PL", strlen("PL")) == 0) {
		vcc_dram_grp = PIN_GRP_PL;
		strncpy(pin_num_string, pin_char + strlen("PL"), 2);
		vcc_dram_num = dstr2int(pin_num_string, 2);
	} else if (strncmp(pin_char, "PM", strlen("PM")) == 0) {
		vcc_dram_grp = PIN_GRP_PM;
		strncpy(pin_num_string, pin_char + strlen("PM"), 2);
		vcc_dram_num = dstr2int(pin_num_string, 2);
	}

	/* parse vcc-io-gpio */
	pin_char = fdt_stringlist_get(fdt, param_node, "vcc-io-gpio", 0, &len);
	if (strncmp(pin_char, "PL", strlen("PL")) == 0) {
		vcc_io_grp = PIN_GRP_PL;
		strncpy(pin_num_string, pin_char + strlen("PL"), 2);
		vcc_io_num = dstr2int(pin_num_string, 2);
	} else if (strncmp(pin_char, "PM", strlen("PM")) == 0) {
		vcc_io_grp = PIN_GRP_PM;
		strncpy(pin_num_string, pin_char + strlen("PM"), 2);
		vcc_io_num = dstr2int(pin_num_string, 2);
	}

	param_node = fdt_path_offset(fdt, "wol");
	if (param_node >= 0) {
		gmac_intr = fdt_getprop(fdt, param_node, "interrupts", &len);
		if (gmac_intr) {
			gmac_wakeup_grp = (fdt32_to_cpu(gmac_intr[0]) == 0) ? PIN_GRP_PL : PIN_GRP_PM;
			gmac_wakeup_num = fdt32_to_cpu(gmac_intr[1]);
		}
	}

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
	pll_restore[5].addr = CCU_PLL_VE_CTRL_REG;
	pll_restore[6].addr = CCU_PLL_ADC_CTRL_REG;
	pll_restore[7].addr = CCU_PLL_AUDIO0_CTRL_REG;

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

#ifdef CFG_HDMIRX_USED
		tdCEC_Standby();
#endif
#ifdef CFG_HDMITX_USED
		hdmitx_standby_loop();
#endif
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

	if (read_fake_poweroff_flag() == FAKE_POWEROFF_E) {
		if (vcc_dram_grp) {
			pin_write_data(vcc_dram_grp, vcc_dram_num, 0x0);
			pin_set_multi_sel(vcc_dram_grp, vcc_dram_num, 0x1);
		}
	} else {
		if (vcc_dram_grp) {
			pin_write_data(vcc_dram_grp, vcc_dram_num, 0x1);
			pin_set_multi_sel(vcc_dram_grp, vcc_dram_num, 0x1);
		}
	}

	if (vdd_cpu_grp) {
		pin_write_data(vdd_cpu_grp, vdd_cpu_num, 0x0);
		pin_set_multi_sel(vdd_cpu_grp, vdd_cpu_num, 0x1);
	}

	if (vdd_ve_grp) {
		pin_write_data(vdd_ve_grp, vdd_ve_num, 0x0);
		pin_set_multi_sel(vdd_ve_grp, vdd_ve_num, 0x1);
	}

	if (vdd_sys_grp) {
		pin_write_data(vdd_sys_grp, vdd_sys_num, 0x0);
		pin_set_multi_sel(vdd_sys_grp, vdd_sys_num, 0x1);
	}

	if (vcc_io_grp) {
		pin_write_data(vcc_io_grp, vcc_io_num, 0x0);
		pin_set_multi_sel(vcc_io_grp, vcc_io_num, 0x1);
	}

	/* vcc-nor powerdown */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vcc_nor >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_OFF);
	}

	/* vdd-cpu powerdown */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vdd_cpu >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_OFF);
	}
	/* vdd-sys poweroff */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vdd_sys >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_OFF);
	}
	/* vcc-pll powerdown */
	for (type = 0; type < axp_power_max && !standby_osc24m_on; type++) {
		if ((standby_vcc_pll >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_OFF);
	}
}

static void dm_resume(void)
{
	u32 type;

	/* vcc-pll powerup */
	for (type = 0; type < axp_power_max && !standby_osc24m_on; type++) {
		if ((standby_vcc_pll >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vdd-sys poweron */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vdd_sys >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vdd-cpu poweron */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vdd_cpu >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_ON);
	}

	/* vcc-nor poweron */
	for (type = 0; type < axp_power_max; type++) {
		if ((standby_vcc_nor >> type) & 0x1)
			pmu_set_voltage_state(type, POWER_VOL_ON);
	}

	if (vcc_dram_grp) {
		pin_write_data(vcc_dram_grp, vcc_dram_num, 0x1);
		pin_set_multi_sel(vcc_dram_grp, vcc_dram_num, 0x1);
		time_mdelay(2);
	}

	if (vcc_io_grp) {
		pin_write_data(vcc_io_grp, vcc_io_num, 0x1);
		pin_set_multi_sel(vcc_io_grp, vcc_io_num, 0x1);
		time_mdelay(2);
	}

	if (vdd_sys_grp) {
		pin_write_data(vdd_sys_grp, vdd_sys_num, 0x1);
		pin_set_multi_sel(vdd_sys_grp, vdd_sys_num, 0x1);
		time_mdelay(2);
	}

	if (vdd_ve_grp) {
		pin_write_data(vdd_ve_grp, vdd_ve_num, 0x1);
		pin_set_multi_sel(vdd_ve_grp, vdd_ve_num, 0x1);
		time_mdelay(2);
	}

	if (vdd_cpu_grp) {
		pin_write_data(vdd_cpu_grp, vdd_cpu_num, 0x1);
		pin_set_multi_sel(vdd_cpu_grp, vdd_cpu_num, 0x1);
		time_mdelay(2);
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
		LOG("after_crc: 0x%x\n", after_crc);
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

static void res_ctrl_save(void)
{
	hdmi_res1 = readl(HDMI_RES1_REG);
}

static void res_ctrl_restore(void)
{
	writel(RES_KEY_FIELD | hdmi_res1, HDMI_RES1_REG);
}

static bool support_gmac_wakeup(void)
{
	uint32_t gmac_gpio_cfg_value;
	uint32_t gmac_gpio_int_ctl_value;

	if (!gmac_wakeup_grp)
		return FALSE;

	gmac_gpio_cfg_value = PIN_REG_CFG_VALUE(gmac_wakeup_grp, gmac_wakeup_num);
	gmac_gpio_int_ctl_value = PIN_REG_INT_CTL_VALUE(gmac_wakeup_grp);
	LOG("gmac_gpio_cfg_value: 0x%x, gmac_gpio_int_ctl_value: 0x%x\n", gmac_gpio_cfg_value, gmac_gpio_int_ctl_value);
	LOG("gmac_wakeup_grp: 0x%x, gmac_wakeup_num: 0x%x\n", gmac_wakeup_grp, gmac_wakeup_num);

	if ((((gmac_gpio_cfg_value >> (gmac_wakeup_num % 7 * 4)) & 0xf) != 0xe) ||
			(((gmac_gpio_int_ctl_value >> gmac_wakeup_num) & 0x1) != 0x1)) {
		WRN("not support gamc wakeup\n");
		return FALSE;
	}

	return TRUE;
}

static void wol_standby_init(void)
{
	u32 wake_src_type = 0x1;
	u32 wakeup_root_irq = (gmac_wakeup_grp == PIN_GRP_PL) ? (GIC_R_GPIOL_NS_IRQ - 32) : (GIC_R_GPIOM_NS_IRQ - 32);
	unsigned int paras_buf[1] = {wake_src_type << 30 | wakeup_root_irq};
	message_block_t wol_message = {
		.state = 0,
		.attr = 0,
		.type = 0,
		.result = 0,
		.count = 1,
		.paras = paras_buf,
	};
	set_wakeup_src(&wol_message);
}

static void wol_standby_exit(void)
{
	u32 wake_src_type = 0x1;
	u32 wakeup_root_irq = (gmac_wakeup_grp == PIN_GRP_PL) ? (GIC_R_GPIOL_NS_IRQ - 32) : (GIC_R_GPIOM_NS_IRQ - 32);
	unsigned int paras_buf[1] = {wake_src_type << 30 | wakeup_root_irq};
	message_block_t wol_message = {
		.state = 0,
		.attr = 0,
		.type = 0,
		.result = 0,
		.count = 1,
		.paras = paras_buf,
    };
    clear_wakeup_src(&wol_message);
}

static void device_suspend(void)
{
	if (support_gmac_wakeup()) {
		wol_standby_init();
	}
	usb_standby_init();
	sunxi_timestamp_suspend();
	smc_standby_init();
	if (is_pmu_exist()) {
		pmu_standby_init();
		twi_standby_init();
	}
	hwmsgbox_super_standby_init();
#if CFG_IR_USED
	ir_init();
#endif
	res_ctrl_save();
}

static void device_resume(void)
{
	res_ctrl_restore();
#if CFG_IR_USED
	ir_exit();
#endif
	hwmsgbox_super_standby_exit();
	if (is_pmu_exist()) {
		twi_standby_exit();
		pmu_standby_exit();
	}
	smc_standby_exit();
	sunxi_timestamp_resume();
	usb_standby_exit();
	if (support_gmac_wakeup()) {
		wol_standby_exit();
	}
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
		// writel((readl((CPU_PLL_REG(i)) & (~CPU_PLL_LDO_EN_MASK)) | CPU_PLL_LDO_EN(0)), CPU_PLL_REG(i));
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
		// writel((readl((CPU_PLL_REG(i)) & (~CPU_PLL_LDO_EN_MASK)) | CPU_PLL_LDO_EN(1)), CPU_PLL_REG(i));
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
	writel(readl(ANA_PWR_RST_REG) | (0x1 << 0), ANA_PWR_RST_REG);

	/* USB isolation */
	usb_iso_suspend();

	/* System reset control */
	writel(readl(VDD_SYS_PWR_RST_REG) & (~(0x1 << 0)), VDD_SYS_PWR_RST_REG);

	/* Digital Domain ISO Control */
	writel(readl(VDD_SYS_PWROFF_GATING_REG) | (0x1 << 2) | (0x1 << 8), VDD_SYS_PWROFF_GATING_REG);
}

static void nsi_resume(void)
{
	u32 reg_val, i;
	const u32 WEND_OFFSET = 0x200;
	const u32 WR_COHERENCY = 0x0004;
	const u32 USB_AHB_WEND_ENABLE_OFFSET = 16;

	//nsi TAG_N credit count
	writel(0x40005, 0x2403200 + 0x6c);
	//RA_N FIFO Mode Tx Register
	writel(0xFF, 0x2402C00 + 0x14);
	writel(0xFF, 0x2402E00 + 0x14);
	writel(0xFF, 0x2403000 + 0x14);
	// isp rtlpr
	writel(0x1, 0x2400000 + 0xc);
	// csi rtlpr
	writel(0x1, 0x2400200 + 0xc);

	for (i = 17; i < 22; i++) {
		reg_val = readl(NSI_BASE + (WEND_OFFSET * i) + WR_COHERENCY);
		reg_val |= (1 << USB_AHB_WEND_ENABLE_OFFSET);
		writel(reg_val, NSI_BASE + (WEND_OFFSET * i) + WR_COHERENCY);
	}
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
	writel(readl(ANA_PWR_RST_REG) & (~(0x1 << 0)) & (~(0x1 << 1)), ANA_PWR_RST_REG);
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

	device_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x3);

	cpu_pll_off();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x4);

	dram_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x5);

	if (!support_gmac_wakeup()) {
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
	}

	return OK;
}

static s32 standby_process_exit(struct message *pmessage)
{
	u32 resume_entry = pmessage->paras[1];

	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x1);
	if (!support_gmac_wakeup()) {
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
	}
	dram_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x7);

	cpu_pll_on();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x8);

	device_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x9);

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

#ifdef CFG_HDMIRX_USED
	tdCEC_OTPMessageInit();
#endif

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

#ifdef CFG_HDMIRX_USED
	tdCEC_SetOTPFlag(1);
#endif

	return OK;
}

u32 is_suspend_lock(void)
{
	return suspend_lock;
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
	iosc_freq_init();
	suspend_para_prepare();
	clk_suspend();
	pmu_shutdown();
	LOG("system_shutdown\r\n");
	while (1) {
		__asm volatile("wfi");
	}
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
#ifdef CFG_HDMIRX_USED
			SysBackupWakeupData();
#endif
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

void fake_pwroff_wakeup_src_init(void)
{
	gpio_pwrkey_wakeup_init();
	save_state_flag(REC_FAKEPOWEROFF | REC_BEFORE_INIT | 0x00);
}


s32 fake_poweroff(struct message *pmessage)
{
	arm_is_die = 1;

#ifdef CFG_HDMIRX_USED
	u32 cec_en = 0;
	u32 cec_wakeup_en = 0;

	cec_en = readl(RTC_FAKE_PRI_DATA_REG) & (1 << 1);
	cec_wakeup_en = readl(RTC_FAKE_PRI_DATA_REG) & (1 << 0);

	tdCEC_EnableCec(_FALSE_);
	tdCEC_EnableCecWakeup(_FALSE_);
#endif

	save_state_flag(REC_FAKEPOWEROFF | REC_ENTER | 0x01);

#ifdef CFG_HDMIRX_USED
	if (cec_en) {
		tdCEC_EnableCec(_TRUE_);
		if (cec_wakeup_en)
			tdCEC_EnableCecWakeup(_TRUE_);
		save_state_flag(REC_FAKEPOWEROFF | REC_ENTER | 0x01);
	}
#endif

	standby_dts_parse();
	fake_poweroff_pmu_dts_parse();
	fake_poweroff_pin_dts_parse();
	save_state_flag(REC_FAKEPOWEROFF | REC_ENTER | 0x02);

	fake_poweroff_pmu_wakeup_init();
	fake_poweroff_pins_wakeup_init();
	save_state_flag(REC_FAKEPOWEROFF | REC_ENTER | 0x03);

	fake_pwroff_wakeup_src_init();
	save_state_flag(REC_FAKEPOWEROFF | REC_ENTER | 0x04);

	standby_process_init(pmessage);
	save_state_flag(REC_FAKEPOWEROFF | REC_BEFORE_EXIT | 0x00);

	device_special_handle(pmessage);

	LOG("wait wakeup\n");

	wait_wakeup();
	save_state_flag(REC_FAKEPOWEROFF | REC_AFTER_EXIT | 0x00);

	dm_resume();
	save_state_flag(REC_FAKEPOWEROFF | REC_AFTER_EXIT | 0x01);

	save_fake_poweroff_flag(BOOT_NORMAL);
	save_state_flag(REC_FAKEPOWEROFF | REC_AFTER_EXIT | 0x02);

	time_mdelay(5);
	save_state_flag(REC_FAKEPOWEROFF | REC_AFTER_EXIT | 0x03);

#ifdef CFG_HDMIRX_USED
	if (tdCEC_GetCecWakeFlag())
		save_state_flag(REC_FAKEPOWEROFF | REC_AFTER_EXIT | CEC_WAKE_FLAG);
#endif

	// /* FIXME: use pmu reset anyway and power down vcc-dram */
	// /* watchdog_reset(); */
	system_reset();
	save_state_flag(REC_FAKEPOWEROFF | REC_AFTER_EXIT | 0x04);

	return 0;
}

/* feedback pmu irq */
s32 get_pmu_irq(struct message *pmessage)
{
	return OK;
}
