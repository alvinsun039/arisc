/*
 *  * Copyright 2000-2009
 *   * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *    *
 *     * SPDX-License-Identifier:	GPL-2.0+
 *     */
#include "include.h"
#include "irq_table.h"
#include <libfdt.h>
#include "types.h"
#include "messages.h"
#include "system/cpu.h"
#include "service/wakeup_source.h"
#include <driver/gpadc.h>
#include "sunxi_gpadc.h"

static struct softtimer key_timer;
extern u32 volatile wakeup_source;
extern u32 dtb_base;
static u8 valid_count;
static uint32_t key_vol, key_code, sample_channel;

#define KEY_VOL_OFFSET 100
#define MODULE_VERSION "1.0.1"
int sunxi_gpadc_read(int channel)
{
	u32 ints;
	int key = -1;
	u32 snum = 0;

	writel(1 << channel, GP_CS_EN_REG);
	udelay(1500);


	ints = readl(GP_DATA_INTS_REG);
	/* clear the pending data */
	writel((ints & (0x1 << channel)), GP_DATA_INTS_REG);

	/* if there is already data pending, read it */
	if (ints & (GPADC0_DATA_PENDING << channel)) {
		snum = readl(GP_CH0_DATA_REG + (channel * 4));
	}

	key = snum;

	return key;
}

int sunxi_get_gpadc_vol(int channel)
{
	u32 vol_m;

	if (channel < 0) {
		LOG("error: channel < 0\n");
		return -1;
	}

	vol_m = sunxi_gpadc_read(channel);

	/* convert to voltage, unit:mV */
	vol_m = (vol_m * 1800) / 4095;

	return vol_m;
}

static void key_timer_start(void)
{
	key_timer.cycle = 0x0;
	key_timer.expires = 0x0;
	key_timer.start = SOFTTIMER_ON;
}

static void key_timer_stop(void)
{
	stop_softtimer(&key_timer);
}

static s32 key_timer_handler(void *parg)
{
	int vol;

	vol = sunxi_get_gpadc_vol(sample_channel);

	if (vol >= (key_vol > KEY_VOL_OFFSET ? key_vol - KEY_VOL_OFFSET : 0) && vol < key_vol + KEY_VOL_OFFSET) {
		valid_count++;
		if (valid_count >= 5) {
			LOG("key_timer_handler-valid_count=%d, vol=%d\n", valid_count, vol);
			wakeup_source = CPUS_IRQ_MAPTO_CPUX(INTC_R_GPADC_IRQ);
			key_timer_stop();
			valid_count = 0;
			return OK;
		}
	} else {
		valid_count = 0;
	}

	key_timer_start();

	return OK;
}

static s32 key_timer_init(void)
{
	key_timer.cycle = 0x0;
	key_timer.expires = 0x0;
	key_timer.cb = key_timer_handler;
	key_timer.arg = NULL;
	key_timer.start = SOFTTIMER_OFF;
	add_softtimer(&key_timer);
	key_timer_start();

	return OK;
}

static void sunxi_gpadc_sample_rate_set(u32 clk_in, u32 round_clk)
{
	u32 div, reg_val, tacq;

	/*
	 * sr(clk_in / round_clk) = tacq'(tacq + 1) + conv(13) + wait(3)
	 * ==> sr - wait = tacq' + conv
	 * ==> div = sr - wait
	 * ==> tacq' = div - conv
	 * ==> tacq = div - conv - 1
	 */
	div = clk_in / round_clk - 3;
	tacq = div - 13 - 1;

	if (tacq > DEFAULT_TACQ_LIMIT) {
		LOG("tacq oversize, limit to %d\n", DEFAULT_TACQ_LIMIT);
		tacq = DEFAULT_TACQ_LIMIT;
	}

	reg_val = readl(GP_SR_REG);
	reg_val = (reg_val & ~GP_SR_DIV_MASK) | (div << GP_SR_DIV_SHIFT) | tacq;
	writel(reg_val, GP_SR_REG);
}

static int gpio_powerkey_dts_parse(void)
{
	void *fdt;
	int param_node;
	fdt = (void *)(dtb_base);
	param_node = fdt_path_offset(fdt, "/gpadc_params");
	if (param_node < 0) {
		LOG("no gpadc_params: %x fdt:%x\n", param_node, fdt);
		key_vol = 880;
		key_code = 0;
		return -1;
	}
	fdt_getprop_u32(fdt, param_node, "key_vol", &key_vol);
	fdt_getprop_u32(fdt, param_node, "key_code", &key_code);
	fdt_getprop_u32(fdt, param_node, "channel", &sample_channel);
	LOG("gpio_powerkey_dts_parse-key_vol=%d, key_code=%d, sample_channel=%d\n", key_vol, key_code, sample_channel);
	return 0;
}

int sunxi_gpadc_init(void)
{

	u32 reg_val = 0;

	/*set gpadc clock register*/
	reg_val = ((1 << 31) | (0 << 24) | (0 << 0));
	writel(reg_val, R_GPADC_CLOCK_REG);

	/*set gpadc gating reset register*/
	/* reset */
	reg_val = readl(R_GPADC_BUS_GATE_RST_REG);
	reg_val |= (1 << 16);
	writel(reg_val, R_GPADC_BUS_GATE_RST_REG);

	udelay(2);

	/* enable ADC gating */
	reg_val = readl(R_GPADC_BUS_GATE_RST_REG);
	reg_val |= (1 << 0);
	writel(reg_val, R_GPADC_BUS_GATE_RST_REG);

	gpio_powerkey_dts_parse();
	/*ADC sample ferquency divider*ferquency=CLK_IN/(n+3), 24000000/(0x5dbd+3)=1000Hz
	* ADC acquire time
	* time=CLK_IN/(n+3) ,24000000/(0x2f+3)=500000Hz
	*/
	sunxi_gpadc_sample_rate_set(DEFAULT_CLK_IN, DEFAULT_SR);

	/*choose continue work mode and enable ADC*/
	reg_val = readl(GP_CTRL_REG);
	reg_val &= ~(1<<18);
	reg_val |= ((1<<19) | (1<<16));
	writel(reg_val, GP_CTRL_REG);

	/* disable all key irq */
	writel(0, GP_DATA_INTC_REG);
	writel(1, GP_DATA_INTC_REG);

	key_timer_init();
	LOG("sunxi_gpadc module version :%s\n", MODULE_VERSION);

	return 0;
}

