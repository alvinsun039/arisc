/*
 * Copyright (c) 2020, Allwinner. All rights reserved.
 *
 * Author: Fan Qinghua <fanqinghua@allwinnertech.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sunxi_cpu_ops_private.h>

/*
 * PMC
 */
#define sun252IW3P1_CCU_BASE		(0x02002000)
#define CPU_CFG_BGR_REG		0xB08
#define CPU_CFG_RST_REG		0xB10
#define CCU_BGR_ENABLE			(BIT(16) | BIT(0))
#define sun252IW3P1_PMC_BASE		(0x08000000)
#define PMC_BASE(n)			(sun252IW3P1_PMC_BASE + 0x1000 * (n))
#define SW_MODE_CTRL(m)			(0x100 + 0x4 * (m))
#define TIMER_MODE_CONFIG		0x600
#define DELAY_CTRL_CONFIG		0x700
#define SW_CTRL_CONFIG			0x38
#define IGN_WKUP_TRIG			BIT(1)

void sunxi_init_archstate(unsigned int cluster, unsigned int cpu, unsigned long arch64)
{
	if (arch64) {
		/*bit9:8=10*/
		mmio_write_32(SUNXI_INITARCH_REG(cpu), (mmio_read_32(SUNXI_INITARCH_REG(cpu)) & ~(0x3 << 8)) | (0x2 << 8));
	} else {
		/*bit9:8=01*/
		mmio_write_32(SUNXI_INITARCH_REG(cpu), (mmio_read_32(SUNXI_INITARCH_REG(cpu)) & ~(0x3 << 8)) | (0x1 << 8));
	}
}

void sunxi_set_bootaddr(unsigned int cluster, unsigned int cpu, uintptr_t entry)
{
	mmio_write_32(0x08008000 + 0x100, entry);
	mmio_write_32(0x08008000 + 0x104, 0);
}

int sunxi_get_cpu_powerstate(unsigned int cluster, unsigned int core)
{
	if ((mmio_read_32(PPU_PWSR(core + 1)) & 0xf) == STATE_ON)
		return 1;
	else
		return 0;
}

int sunxi_get_cluster_powerstate(void)
{
	if ((mmio_read_32(PPU_PWSR(0)) & 0xf) == STATE_ON)
		return 1;
	else
		return 0;
}

void sunxi_poweron_cpu(unsigned int cluster, unsigned int core)
{
	/* Set core power mode to manal mode and power on */
	mmio_write_32(PMC_BASE(core) + SW_MODE_CTRL(0), 0x3 << 3);
	mmio_write_32(PMC_BASE(core) + SW_MODE_CTRL(1), 0x11);
}

void sunxi_poweroff_cpu(unsigned int cluster, unsigned int core)
{
	/* Set core0 poweroff */
	mmio_write_32(PMC_BASE(core) + SW_MODE_CTRL(0), 0x2 << 3);
	mmio_write_32(PMC_BASE(core) + SW_MODE_CTRL(1), 0x2 << 3);
	mmio_write_32(PMC_BASE(core) + SW_CTRL_CONFIG, (mmio_read_32(PMC_BASE(0) + SW_CTRL_CONFIG) | IGN_WKUP_TRIG));
}

/*standby power off cpu0*/

/*wait for cpu power off*/
void cpucfg_cpu_suspend(void)
{
	sunxi_poweroff_cpu(0, 0);
	/*TODO: Confirm whether core0 and the cluster are powered off*/
}
/*power off vdd_cpu*/
/*power off vdd_sys*/


/*wakeup cpu*/
/*power on vdd_sys*/
/*power on vdd_cpu*/
/*init pll_cluster*/
/*init pll_cpu*/
int cpucfg_cpu_resume(unsigned int resume_addr)
{
	/*set cpu boot addr*/
	sunxi_set_bootaddr(0, 0, resume_addr);
	/*set cpu0 to aarch64*/
	sunxi_init_archstate(0, 0, 1);
	/*power on cpu0*/
	sunxi_poweron_cpu(0, 0);

	return 0;
}
