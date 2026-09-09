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
#include "include.h"
#include <libfdt.h>

extern u32 dtb_base;

static u32 bmu_exist = FALSE;

extern bmu_ops_t bmu_axp515_ops;
extern bmu_ops_t bmu_axp517_ops;
extern bmu_ops_t bmu_axp519_ops;

#ifdef CFG_FDT_INIT_ARISC_PMU_USED
static bmu_ops_t *bmu_ops_p;
u32 bmu_runtime_addr;

struct bmu_match_data {
	const char name[30];
	u8 twi_addr;
	u8 chipid_addr;
	u8 chipid_mask;
	u8 chipid_val;
	bmu_ops_t *ops_p;
};

static struct bmu_match_data bmu_match_tlb[] = {
#ifdef CFG_AXP515_USED
	{"x-powers,axp515", _RSB_RTSADDR_AXP515, 0x3, 0xcf, 0x46, &bmu_axp515_ops},
	{"x-powers,axp515", _RSB_RTSADDR_AXP515, 0x3, 0xcf, 0x49, &bmu_axp515_ops},
#endif
#ifdef CFG_AXP517_USED
	{"x-powers,axp517-t0", RSB_RTSADDR_AXP517_T0, 0xe, 0xff, 0x04, &bmu_axp517_ops},
	{"x-powers,axp517-01", RSB_RTSADDR_AXP517_01, 0xe, 0xff, 0x04, &bmu_axp517_ops},
#endif
};

#else /* CFG_FDT_INIT_ARISC_PMU_USED */

#if defined CFG_AXP515_USED
static bmu_ops_t *bmu_ops_p = &bmu_axp515_ops;
u32 bmu_runtime_addr;
#elif defined CFG_AXP519_USED
static bmu_ops_t *bmu_ops_p = &bmu_axp519_ops;
u32 bmu_runtime_addr;
#elif defined CFG_AXP517_USED
static bmu_ops_t *bmu_ops_p = &bmu_axp517_ops;
u32 bmu_runtime_addr = RSB_RTSADDR_AXP517_01;
#else
#error "sun60iw2p1 not support no bmu"
#endif

#endif /* CFG_FDT_INIT_ARISC_PMU_USED */

void bmu_shutdown(void)
{
	if (is_bmu_exist() == FALSE)
		return;

	if (bmu_ops_p->bmu_shutdown)
		bmu_ops_p->bmu_shutdown();
}

void bmu_reset(void)
{
	if (is_bmu_exist() == FALSE)
		return;

	if (bmu_ops_p->bmu_reset)
		bmu_ops_p->bmu_reset();
}

s32 bmu_charging_vbus_det(void)
{
	if (is_bmu_exist() == FALSE)
		return -1;

	if (bmu_ops_p->bmu_charging_vbus_det)
		return bmu_ops_p->bmu_charging_vbus_det();

	return -1;
}

s32 bmu_charging_reset(void)
{
	if (is_bmu_exist() == FALSE)
		return -1;

	if (bmu_ops_p->bmu_charging_reset)
		return bmu_ops_p->bmu_charging_reset();

	return -1;
}

#ifdef CFG_FDT_INIT_ARISC_PMU_USED
static void bmu_init_from_dts(void)
{
	u32 i;
	u8 val = 0;

	for (i = 0; i < sizeof(bmu_match_tlb) / sizeof(bmu_match_tlb[0]); i++) {
		pmu_reg_read(&bmu_match_tlb[i].twi_addr, &bmu_match_tlb[i].chipid_addr,
			     &val, 1);
		if ((val & bmu_match_tlb[i].chipid_mask) == bmu_match_tlb[i].chipid_val) {
			bmu_ops_p = bmu_match_tlb[i].ops_p;
			bmu_runtime_addr = bmu_match_tlb[i].twi_addr;
			LOG("bmu is %s\n", bmu_match_tlb[i].name);
			break;
		}
	}
}
#endif

s32 bmu_init(void)
{
#ifdef CFG_FDT_INIT_ARISC_PMU_USED
	bmu_init_from_dts();
#endif
	/* power_mode may parse from dts */
	if (!bmu_ops_p || bmu_ops_p->bmu_is_exist() < 0) {
		bmu_exist = FALSE;
		LOG("bmu is not exist\n");
	} else {
		bmu_exist = TRUE;
		LOG("bmu is exist\n");
	}
	return OK;
}

int bmu_check_status(void)
{
	void *fdt;
	int bmu_node, ret;

	fdt = (void *)(dtb_base);

	bmu_node = fdt_path_offset(fdt, "bat_supply");
	if (bmu_node < 0)
		return FALSE;

	ret = fdt_get_status(fdt, bmu_node);
	if (ret < 0)
		return FALSE;

	return ret;
}

u32 is_bmu_exist(void)
{
	return bmu_exist;
}
