/*
 * A733 changes: Copyright (c) 2026 A733 contributors.
 * These changes are provided under BSD-3-Clause (see LICENSE); the
 * original file retains its upstream copyright and terms.
 */

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
		mmio_setbits_32(SUNXI_INITARCH_REG(cpu), AARCH64);
	} else {
		mmio_clrbits_32(SUNXI_INITARCH_REG(cpu), AARCH64);
	}
}

void sunxi_set_bootaddr(unsigned int cluster, unsigned int cpu, uintptr_t entry)
{
	mmio_write_32(SUNXI_CPUCFG_RVBAR_LO_REG(cpu), entry);
	mmio_write_32(SUNXI_CPUCFG_RVBAR_HI_REG(cpu), 0);
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
	mmio_setbits_32(HOTPLUG_CONTROL_REG(core), HOTPLUG_EN);
	mmio_setbits_32(HOTPLUG_POWERMODE_REG(core), POWER_ON);

	while (!sunxi_get_cpu_powerstate(cluster, core)) {
	}

	mmio_clrbits_32(HOTPLUG_CONTROL_REG(core), HOTPLUG_EN);
}

void sunxi_poweroff_cpu(unsigned int cluster, unsigned int core)
{
	/* Set mp0_spmc_pwr_on_cpuX = 0 */
	mmio_clrbits_32(HOTPLUG_POWERMODE_REG(core), POWER_ON);
}

/*standby power off cpu0*/

/*wait for cpu power off*/
#define MAX_TIMEOUT	(10000)
void cpucfg_cpu_suspend(void)
{
	unsigned int timeout = 0;

	sunxi_poweroff_cpu(0, 0);
	while (sunxi_get_cluster_powerstate()) {
		timeout++;
		if (timeout >= MAX_TIMEOUT) {
			mmio_clrbits_32(SUNXI_DSU_REG, BIT(0));
			mmio_setbits_32(SUNXI_DSU_REG, BIT(0));
			break;
		}
	}
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
	/* bp168: the boot address/RVBAR loses its value while the core
	 * domain is powered off, so it MUST be programmed after the
	 * domain power is enabled (HOTPLUG_EN) but before the core is
	 * released (POWER_ON).  The old order (RVBAR write before any
	 * power) silently lost the boot address on resume. */
	mmio_setbits_32(HOTPLUG_CONTROL_REG(0), HOTPLUG_EN);
	sunxi_set_bootaddr(0, 0, resume_addr);
	sunxi_init_archstate(0, 0, 1);
	mmio_setbits_32(HOTPLUG_POWERMODE_REG(0), POWER_ON);
	while (!sunxi_get_cpu_powerstate(0, 0)) {
	}
	mmio_clrbits_32(HOTPLUG_CONTROL_REG(0), HOTPLUG_EN);

	return 0;
}
