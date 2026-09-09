#include <libfdt.h>
#include "types.h"
#include "messages.h"
#include "system/cpu.h"
#include "service/wakeup_source.h"
#include "irq_table.h"
#include "include.h"

static struct softtimer key_timer;
static u32 power_key_grp;
static u32 power_key_num;
static u32 delay_time;
static uint32_t key_debounce;

extern u32 volatile wakeup_source;
extern u32 dtb_base;

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
	u32 period;

	period = 1000 / TICK_PER_SEC;

	if (!((readl(PIN_REG_DATA(power_key_grp)) >> power_key_num) & 0x1)) {
		delay_time += period;
		if (delay_time >= key_debounce) {
			wakeup_source = CPUS_IRQ_MAPTO_CPUX(INTC_R_GPIOL_NS_IRQ);
			key_timer_stop();
			return OK;
		}
	} else {
		delay_time = 0;
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

static s32 gpio_powerkey_dts_parse(void)
{
	void *fdt;
	int32_t param_node;
	int len;
	const char *pin_char;
	char pin_num_string[4];

	fdt = (void *)(dtb_base);
	/* parse power tree */
	param_node = fdt_path_offset(fdt, "standby_param");
	if (param_node < 0) {
		WRN("no standby_param: %x fdt:%x\n", param_node, fdt);
		return -1;
	}

	/* parse power-key-gpio */
	pin_char = fdt_stringlist_get(fdt, param_node, "power-key-gpio", 0, &len);
	if (strncmp(pin_char, "PL", strlen("PL")) == 0) {
		power_key_grp = PIN_GRP_PL;
		strncpy(pin_num_string, pin_char + strlen("PL"), 2);
		power_key_num = dstr2int(pin_num_string, 2);
	} else if (strncmp(pin_char, "PM", strlen("PM")) == 0) {
		power_key_grp = PIN_GRP_PM;
		strncpy(pin_num_string, pin_char + strlen("PM"), 2);
		power_key_num = dstr2int(pin_num_string, 2);
	}

	return 0;
}

s32 gpio_pwrkey_wakeup_init(void)
{
	if (gpio_powerkey_dts_parse())
		return -1;
	if (!power_key_grp)
		return 0;

	/* set pinctrl to external interrupt */
	pin_set_multi_sel(power_key_grp, power_key_num, 0xe);

	/* set nrgative as external interrupt trigger edge */
	writel(readl(PIN_REG_INT_CFG(power_key_grp, power_key_num)) & (~(0xf << PIN_NUM_INT_CFG_OFFSET(power_key_num))), PIN_REG_INT_CFG(power_key_grp, power_key_num));
	writel(readl(PIN_REG_INT_CFG(power_key_grp, power_key_num)) | (0x1 << PIN_NUM_INT_CFG_OFFSET(power_key_num)), PIN_REG_INT_CFG(power_key_grp, power_key_num));

	/* clean external interrupt mask */
	writel(readl(PIN_REG_INT_STAT(power_key_grp)) & (~(0x1 << power_key_num)), PIN_REG_INT_STAT(power_key_grp));
	writel(readl(PIN_REG_INT_STAT(power_key_grp)) | (0x1 << power_key_num), PIN_REG_INT_STAT(power_key_grp));

	/* enable external interrupt */
	writel(readl(PIN_REG_INT_CTL(power_key_grp)) & (~(0x1 << power_key_num)), PIN_REG_INT_CTL(power_key_grp));
	writel(readl(PIN_REG_INT_CTL(power_key_grp)) | (0x1 << power_key_num), PIN_REG_INT_CTL(power_key_grp));

	key_timer_init();

	return 0;
}