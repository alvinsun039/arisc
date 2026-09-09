/*
 * Copyright (c) 2020, Allwinner. All rights reserved.
 *
 * Author: Fan Qinghua <fanqinghua@allwinnertech.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sunxi_cpu_ops_private.h>

void sunxi_init_archstate(unsigned int cluster, unsigned int cpu, unsigned long arch64)
{
	if (arch64) {
		mmio_setbits_32(SUNXI_INITARCH_REG(cluster, cpu), AARCH64);
	} else {
		mmio_clrbits_32(SUNXI_INITARCH_REG(cluster, cpu), AARCH64);
	}
}

void sunxi_set_bootaddr(unsigned int cluster, unsigned int cpu, uintptr_t entry)
{
	mmio_write_32(SUNXI_CPUCFG_RVBAR_LO_REG(cluster, cpu), entry);
	mmio_write_32(SUNXI_CPUCFG_RVBAR_HI_REG(cluster, cpu), 0);
}

void sunxi_poweron_cpu(unsigned int cluster, unsigned int cpu)
{
	while ((mmio_read_32(PWRS_STAT_REG(cluster, cpu)) & POWER_MASK) != POWER_OFF)
		;

	mmio_setbits_32(HOTPLUG_CONTROL_REG(cluster, cpu), HOTPLUG_REQ);

	while (mmio_read_32(HOTPLUG_CONTROL_REG(cluster, cpu)) & HOTPLUG_REQ)
		;
}

void sunxi_poweroff_cpu(unsigned int cluster, unsigned int core)
{
	/*no need to poweroff cpu after disable gic wakeup*/
}

void sunxi_disable_gic_wakeup(unsigned int cluster, unsigned int cpu)
{
	mmio_setbits_32(HOTPLUG_CONTROL_REG(cluster, cpu), WAKEUP_MASK);
}

/*standby power off cpu0*/

/*wait for cpu power off*/
void cpucfg_cpu_suspend(void)
{
	while ((mmio_read_32(PWRS_STAT_REG(0, 0)) & POWER_MASK) != POWER_OFF)
		;

	sunxi_poweroff_cpu(0, 0);
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
	sunxi_init_archstate(0, 0, AARCH64);
	/*power on cpu0*/
	sunxi_disable_gic_wakeup(0, 0);
	sunxi_poweron_cpu(0, 0);

	return 0;
}
