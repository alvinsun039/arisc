#include <libfdt.h>
#include "types.h"
#include "messages.h"
#include "service/wakeup_source.h"
#include "service/fake_power_off.h"
#include "../standby_i.h"

extern u32 dtb_base;

s32 fake_poweroff_pmu_dts_parse(void)
{
	/* do nothing now */

	return 0;
}

#define FAKE_POWEROFF_LED0_PIN (0)
#define FAKE_POWEROFF_LED1_PIN (1)
#define FAKE_POWEROFF_WAKE0_PIN (2)
#define FAKE_POWEROFF_WAKE1_PIN (3)
#define FAKE_POWEROFF_PIN_NUM (4)
static fake_poweroff_pinctrl_t fake_poweroff_pin[FAKE_POWEROFF_PIN_NUM];
s32 fake_poweroff_pin_dts_parse(void)
{
	void *fdt = (void *)(dtb_base);
	int32_t box_node, pin_node, phandle_num;
	uint32_t phandle[4];
	u32 i;
	char *pin_name_str, pinctrl[12];
	/* parse box node */
	box_node = fdt_path_offset(fdt, "/box_start_os0");
	if (box_node < 0)
	    return -1;

	for (i = 0; i < FAKE_POWEROFF_PIN_NUM; i++) {
		sprintf(pinctrl, "pinctrl-%d", i);
		phandle_num = fdt_getprop_u32(fdt, box_node, pinctrl, phandle);
		if (phandle_num < 0)
			continue;

		pin_node =  fdt_node_offset_by_phandle(fdt, phandle[0]);
		if (pin_node < 0)
			continue;

		fdt_getprop_string(fdt, pin_node, "allwinner,pins",
				&pin_name_str);
		strcpy(fake_poweroff_pin[i].name, pin_name_str);
		fdt_getprop_u32(fdt, pin_node, "allwinner,muxsel",
				&fake_poweroff_pin[i].mux);
		fdt_getprop_u32(fdt, pin_node, "allwinner,data",
				&fake_poweroff_pin[i].data);
		fdt_getprop_u32(fdt, pin_node, "allwinner,drive",
				&fake_poweroff_pin[i].drive);
		fdt_getprop_u32(fdt, pin_node, "allwinner,pull",
				&fake_poweroff_pin[i].pull);
		fdt_getprop_u32(fdt, pin_node, "allwinner,eint",
				&fake_poweroff_pin[i].eint);

		LOG("pin:%s mux:%d data:%d drive:%d pull:%d eint: %d\n",
			fake_poweroff_pin[i].name, fake_poweroff_pin[i].mux,
			fake_poweroff_pin[i].data, fake_poweroff_pin[i].drive,
			fake_poweroff_pin[i].pull, fake_poweroff_pin[i].eint);
	}

	return 0;
}

void fake_poweroff_pmu_wakeup_init(void)
{
	/* clear pmu pending before irq enable, avoid woken by mistake */
	pmu_clear_pendings();
}

static s32 fake_poweroff_pinx_wakeup_init(fake_poweroff_pinctrl_t *pin)
{
	u32 wakeup_paras;
	char pin_num_str[8];
	u32 pin_grp, pin_num;
	struct message message_ws;

	/* pin name is "PL1", "PM2" generally, so name[1] is 'L' or 'M' */
	if (pin->name[1] < 'L')
		return -1;

	pin_grp = pin->name[1] - 'L' + PIN_GRP_PL;

	/* pin name is "PL1", "PM2" generally, so pin_num_str is '1' or '2' */
	strcpy(pin_num_str, pin->name + 2);
	pin_num = dstr2int(pin_num_str, strlen(pin_num_str));

	/* set pinctrl to external interrupt */
	pin_set_multi_sel(pin_grp, pin_num, 0xe);

	/* set negative as external interrupt trigger edge */
	writel(readl(PIN_REG_INT_CFG(pin_grp, pin_num)) & (~(0xf << PIN_NUM_INT_CFG_OFFSET(pin_num))), PIN_REG_INT_CFG(pin_grp, pin_num));
	writel(readl(PIN_REG_INT_CFG(pin_grp, pin_num)) | (pin->eint << PIN_NUM_INT_CFG_OFFSET(pin_num)), PIN_REG_INT_CFG(pin_grp, pin_num));

	/* clean external interrupt pending */
	writel(readl(PIN_REG_INT_STAT(pin_grp)) & (~(0x1 << pin_num)), PIN_REG_INT_STAT(pin_grp));
	writel(readl(PIN_REG_INT_STAT(pin_grp)) | (0x1 << pin_num), PIN_REG_INT_STAT(pin_grp));

	/* enable external interrupt */
	writel(readl(PIN_REG_INT_CTL(pin_grp)) & (~(0x1 << pin_num)), PIN_REG_INT_CTL(pin_grp));
	writel(readl(PIN_REG_INT_CTL(pin_grp)) | (0x1 << pin_num), PIN_REG_INT_CTL(pin_grp));

	/* set wakeup source */
	wakeup_paras = GIC_R_GPIOL_NS_IRQ - 32 + (pin->name[1] - 'L') * 2;
	message_ws.paras = &wakeup_paras;
	set_wakeup_src(&message_ws);

	return 0;
}

void fake_poweroff_pins_wakeup_init(void)
{
	fake_poweroff_pinx_wakeup_init(&fake_poweroff_pin[FAKE_POWEROFF_WAKE0_PIN]);
	fake_poweroff_pinx_wakeup_init(&fake_poweroff_pin[FAKE_POWEROFF_WAKE1_PIN]);
}
