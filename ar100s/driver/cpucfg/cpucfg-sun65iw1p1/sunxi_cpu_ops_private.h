/*
 * Copyright (c) 2020, Allwinner. All rights reserved.
 *
 * Author: Fan Qinghua <fanqinghua@allwinnertech.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_CPU_OPS_PRIVATE_H
#define SUNXI_CPU_OPS_PRIVATE_H

#define BIT(n)							(0x1 << (n))

#define SUNXI_CPU_OPS_BASE				0x07001000
#define SUNXI_CPU_PWRS_BASE				(SUNXI_CPU_OPS_BASE + 0x50)
#define PWRS_STAT_REG(cluster, core)		(SUNXI_CPU_PWRS_BASE + (cluster) * 0x1000 + (core) * 0x4)
#define POWER_MASK						0xff
#define POWER_OFF						0xff

#define SUNXI_CPU_CFG_BASE				(SUNXI_CPU_OPS_BASE + 0x70)
#define SUNXI_CPUS_CFG_REG(cluster, core)	(SUNXI_CPU_CFG_BASE + (cluster) * 0x1000 + (core) * 0x4)
#define CPU_IRQ_MASK_BIT				BIT(4)
#define CPU_ENTER_IDLE_BIT				BIT(16)

#define HOTPLUG_CONTROL_BASE 			(SUNXI_CPU_OPS_BASE + 0x80)
#define HOTPLUG_CONTROL_REG(cluster, core)	(HOTPLUG_CONTROL_BASE + (cluster) * 0x1000 + (core) * 0x4)
#define WAKEUP_MASK						BIT(0)
#define HOTPLUG_REQ						BIT(16)

#define SUNXI_CPUIDLE(cluster)			(SUNXI_CPU_OPS_BASE + 0x100 + (cluster) * 0x1000)
#define SUNXI_PWR_SW_DELAY(cluster)			(SUNXI_CPU_OPS_BASE + 0x140 + (cluster) * 0x1000)
#define SUNXI_F1F2_CONFIG_DELAY(cluster)	(SUNXI_CPU_OPS_BASE + 0x144 + (cluster) * 0x1000)

#define SUNXI_CLUS_OPS_BASE							0x08210040
#define SUNXI_INITARCH_REG(cluster, core)			(SUNXI_CLUS_OPS_BASE + (cluster) * 0x40 + (core) * 0x10)
#define AARCH64							BIT(0)
#define SUNXI_CPUCFG_RVBAR_LO_REG(cluster, core)	(SUNXI_CLUS_OPS_BASE + 0x4 + (cluster) * 0x40 + (core) * 0x10)
#define SUNXI_CPUCFG_RVBAR_HI_REG(cluster, core)	(SUNXI_CLUS_OPS_BASE + 0x8 + (cluster) * 0x40 + (core) * 0x10)

typedef unsigned int uint32_t;
typedef unsigned long uintptr_t;

static inline void mmio_write_32(uintptr_t addr, uint32_t value)
{
	*(volatile uint32_t *)addr = value;
}

static inline uint32_t mmio_read_32(uintptr_t addr)
{
	return *(volatile uint32_t *)addr;
}

static inline void mmio_clrbits_32(uintptr_t addr, uint32_t clear)
{
	mmio_write_32(addr, mmio_read_32(addr) & ~clear);
}

static inline void mmio_setbits_32(uintptr_t addr, uint32_t set)
{
	mmio_write_32(addr, mmio_read_32(addr) | set);
}

#endif /* SUNXI_CPU_OPS_PRIVATE_H */
