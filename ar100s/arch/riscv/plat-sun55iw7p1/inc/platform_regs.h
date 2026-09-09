/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 io module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : io.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-18
* Descript: io module public header.
* Update  : date                auther      ver     notes
*           2012-5-18 10:04:21  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __REG_BASE_H__
#define __REG_BASE_H__

#define NSI_BASE			(0x02400000)
#define SYS_CFG_REG_BASE		(0x03000000)
#define CCU_REG_BASE			(0x03008000)
#define CPUX_HWMSGBOX_REG_BASE		(0x03210000)
#define CPUS_HWMSGBOX_REG_BASE		(0x03211000)
#define RTC_REG_BASE			(0x07200000)
#define R_CPUCFG_REG_BASE		(CPUIDLE_CFG_REG_BASE)
#define CPUIDLE_CFG_REG_BASE		(0x07000000)
#define CPUIDLE_CLU0_CFG_REG_BASE	(0x07001000)
#define CPUIDLE_CLU1_CFG_REG_BASE	(0x07002000)
#define CPUIDLE_CLU2_CFG_REG_BASE	(0x07003000)
#define CPUIDLE_CCI_PPU_CFG_REG_BASE	(0x07004000)
#define R_PRCM_REG_BASE			(0x07030000)
#define R_TMR01_REG_BASE		(0x07208000)
#define R_WDOG_REG_BASE			(0x07040000)
#define E902_CFG_BASE			(0x07032000)
#define R_INTC_REG_BASE			(0x03018000)
#define R_PIO_REG_BASE			(0x07058000)
#define R_CIR_REG_BASE			(0x070A8000)
#define R_UART_REG_BASE			(0x07100000)
#define R_TWI_REG_BASE			(0x07108000)
#define CPU_PLL_REG_BASE		(0x03008000)
#define CPUSUBSYS_REG_BASE		(0x08210000)
#define TS_STAT_REG_BASE		(0x08220000)
#define TS_CTRL_REG_BASE		(0x08230000)
#define RISCV_CLIC_BASE			(0xE0800000)
#define USB0_REG_BASE			(0x0C101000)
#define USB1_REG_BASE			(0x0C200000)
#define HSI_SYS_APP				(0x0DC80000)
#define USB2P0_PHY_APP_BASE		(0x0C000000)
#define USB2P0_SYS_DIG_APP_BASE	(0x0C000000)
#define CPUS_MM_DRAM_LBP		(0x4002C000)

/* rtc domain record reg */
#define RTC_YMD				(RTC_REG_BASE + 0x10)
#define RTC_HMS				(RTC_REG_BASE + 0x14)
#define RTC_LOSC_CTRL			(RTC_REG_BASE + 0x000)
#define RTC_LOSC_OUT_GATING		(RTC_REG_BASE + 0x060)
#define RTC_FAKE_PRI_DATA_REG		(RTC_REG_BASE + 0x104)
#define RTC_FAKE_POWER_OFF_REG		(RTC_REG_BASE + 0x108)
#define RTC_RECORD_REG			(RTC_REG_BASE + 0x10c)
#define RTC_DTB_BASE_STORE_REG		(RTC_RECORD_REG)
#define RTC_XO_WRT_PROTECT		(RTC_REG_BASE + 0x15c)
#define RTC_XO_CTRL_REG			(RTC_REG_BASE + 0x160)
#define RTC_VDD_OFF_GATING_CTRL		(RTC_REG_BASE + 0x1f4)
#define RTC_CFG_REG			(RTC_REG_BASE + 0x310)
#define RTC_IR_CODE_STORE_REG		(RTC_REG_BASE + 0x120)

#define RTC_YMD_MASK			(0xFFFF)
#define RTC_HMS_H_SHIFT			(16)
#define RTC_HMS_H_MASK			(0x1F << RTC_HMS_H_SHIFT)
#define RTC_HMS_M_SHIFT			(8)
#define RTC_HMS_M_MASK			(0x3F << RTC_HMS_M_SHIFT)
#define RTC_HMS_S_SHIFT			(0)
#define RTC_HMS_S_MASK			(0x3F << RTC_HMS_S_SHIFT)

/* RTC_VDD_OFF_GATING_CTRL */
#define RTC_VCCIO_OUTPUT_EN     (0x01 << 7)
#define RTC_VCCIO_DETECT_EN     (0x01)

/* CPU CLK */
#define CPU_CLK_REG(n)			(CPU_PLL_REG_BASE + (n) * 0x0008 + 0x0600)

/* CPU PLL REG */
#define CPU_PLL_REG(n)			(CPU_PLL_REG_BASE + (n) * 0x0020 + 0x0040)
#define CPU_PLL_PAT0_REG(n)		(CPU_PLL_REG_BASE + (n) * 0x0020 + 0x0044)
#define CPU_PLL_SSC_REG(n)		(CPU_PLL_REG_BASE + (n) * 0x0020 + 0x0054)

#define CPU_PLL_EN_SHIFT		(31)
#define CPU_PLL_EN_MASK			(1 << CPU_PLL_EN_SHIFT)
#define CPU_PLL_EN(n)			((n) << CPU_PLL_EN_SHIFT)

#define CPU_PLL_LDO_EN_SHIFT		(30)
#define CPU_PLL_LDO_EN_MASK		(1 << CPU_PLL_LDO_EN_SHIFT)
#define CPU_PLL_LDO_EN(n)		((n) << CPU_PLL_LDO_EN_SHIFT)

#define CPU_PLL_LOCK_EN_SHIFT		(29)
#define CPU_PLL_LOCK_EN_MASK		(1 << CPU_PLL_LOCK_EN_SHIFT)
#define CPU_PLL_LOCK_EN(n)		((n) << CPU_PLL_LOCK_EN_SHIFT)

#define CPU_PLL_LOCK_STATUS_SHIFT	(28)
#define CPU_PLL_LOCK_STATUS_MASK	(1 << CPU_PLL_LOCK_STATUS_SHIFT)
#define CPU_PLL_LOCK_STATUS(n)		((n) << CPU_PLL_LOCK_STATUS_SHIFT)

#define CPU_PLL_OUTPUT_SHIFT		(27)
#define CPU_PLL_OUTPUT_MASK		(1 << CPU_PLL_OUTPUT_SHIFT)
#define CPU_PLL_OUTPUT(n)		((n) << CPU_PLL_OUTPUT_SHIFT)

#define CPU_PLL_UPDATE_SHIFT		(26)
#define CPU_PLL_UPDATE_MASK		(1 << CPU_PLL_UPDATE_SHIFT)
#define CPU_PLL_UPDATE(n)		((n) << CPU_PLL_UPDATE_SHIFT)

#define CPU_PLL_FACTOR_M0_SHIFT		(20)
#define CPU_PLL_FACTOR_M0_MASK		(0x3 << CPU_PLL_FACTOR_M0_SHIFT)
#define CPU_PLL_FACTOR_M0(n)		((n) << CPU_PLL_FACTOR_M0_SHIFT)

#define CPU_PLL_FACTOR_P_SHIFT		(16)
#define CPU_PLL_FACTOR_P_MASK		(0x7 << CPU_PLL_FACTOR_P_SHIFT)
#define CPU_PLL_FACTOR_P(n)		((n) << CPU_PLL_FACTOR_P_SHIFT)

#define CPU_PLL_FACTOR_N_SHIFT		(8)
#define CPU_PLL_FACTOR_N_MASK		(0xff << CPU_PLL_FACTOR_N_SHIFT)
#define CPU_PLL_FACTOR_N(n)		((n) << CPU_PLL_FACTOR_N_SHIFT)

#define CPU_PLL_FACTOR_M1_SHIFT		(0)
#define CPU_PLL_FACTOR_M1_MASK		(0xf << CPU_PLL_FACTOR_M1_SHIFT)
#define CPU_PLL_FACTOR_M1(n)		((n) << CPU_PLL_FACTOR_M1_SHIFT)

#define CPU_PLL_FACTOR_MASK		(CPU_PLL_FACTOR_M1_MASK | CPU_PLL_FACTOR_N_MASK | CPU_PLL_FACTOR_P_MASK | CPU_PLL_FACTOR_M0_MASK)

/* CCU PLL REG */
#define PLL_OUTPUT_ENABLE_SHIFT		(27)
#define PLL_LOCK_STATUS_SHIFT		(28)
#define PLL_LOCK_ENABLE_SHIFT		(29)
#define PLL_LDO_ENABLE_SHIFT		(30)
#define PLL_ENABLE_SHIFT		(31)

#define PLL_OUTPUT_ENABLE_MASK		(0x1 << PLL_OUTPUT_ENABLE_SHIFT)
#define PLL_LOCK_STATUS_MASK		(0x1 << PLL_LOCK_STATUS_SHIFT)
#define PLL_LOCK_ENABLE_MASK		(0x1 << PLL_LOCK_ENABLE_SHIFT)
#define PLL_LDO_ENABLE_MASK		(0x1 << PLL_LDO_ENABLE_SHIFT)
#define PLL_ENABLE_MASK			(0x1 << PLL_ENABLE_SHIFT)

#if 0
#define PLL_FACTOR_M0_MASK		(0x1 << 0)
#define PLL_FACTOR_M1_MASK		(0x1 << 1)
#define PLL_FACTOR_P2_MASK		(0x1f << 2)
#define PLL_FACTOR_N_MASK		(0xff << 8)
#define PLL_FACTOR_P0_MASK		(0x1f << 16)
#define PLL_FACTOR_P1_MASK		(0x1f << 20)
#define PLL_FACKOR_MASK			(PLL_FACTOR_M0_MASK | PLL_FACTOR_M1_MASK | PLL_FACTOR_P2_MASK |\
					PLL_FACTOR_N_MASK | PLL_FACTOR_P0_MASK | PLL_FACTOR_P1_MASK)
#endif
#define PLL_FACKOR_MASK			(0x0077ff1f)

/* CCU NSI/MBUS/GIC/CCI REG */
#define NMGC_CLK_GATING_SHIFT		(31)
#define NMGC_CLK_GATING_MASK		(0x1 << NMGC_CLK_GATING_SHIFT)
#define NMGC_CLK_GATING(n)		((n) << NMGC_CLK_GATING_SHIFT)

#define NMGC_CLK_SRC_SEL_SHIFT		(24)
#define NMGC_CLK_SRC_SEL_MASK		(0x7 << NMGC_CLK_SRC_SEL_SHIFT)
#define NMGC_CLK_SRC_SEL(n)		((n) << NMGC_CLK_SRC_SEL_SHIFT)

#define NMGC_CLK_DIV_SHIFT		(0)
#define NMGC_CLK_DIV_MASK		(0x1f << NMGC_CLK_DIV_SHIFT)
#define NMGC_CLK_DIV_SEL(n)		((n) << NMGC_CLK_DIV_SHIFT)

/* CPU_CLK_REG */
#define CPU_CLK_SRC_SEL_SHIFT		(24)
#define CPU_CLK_SRC_SEL_MASK		(0x7 << CPU_CLK_SRC_SEL_SHIFT)
#define CPU_CLK_SRC_SEL(n)		((n) << CPU_CLK_SRC_SEL_SHIFT)

/* CCU_CPU_SYS_DP_CLK_REG */
#define CPU_SYS_DP_CLK_SRC_SEL_SHIFT		(24)
#define CPU_SYS_DP_CLK_SRC_SEL_MASK		(0x7 << CPU_SYS_DP_CLK_SRC_SEL_SHIFT)
#define CPU_SYS_DP_CLK_SRC_SEL(n)		((n) << CPU_SYS_DP_CLK_SRC_SEL_SHIFT)

#define CPU_SYS_DP_CLK_DIV1_SHIFT		(0)
#define CPU_SYS_DP_CLK_DIV1_MASK		(0x1f << CPU_SYS_DP_CLK_DIV1_SHIFT)
#define CPU_SYS_DP_CLK_DIV1(n)			((n) << CPU_SYS_DP_CLK_DIV1_SHIFT)

#define CPU_SYS_DP_CLK_UPD_SHIFT		(27)
#define CPU_SYS_DP_CLK_UPD_MASK			(0x1 << CPU_SYS_DP_CLK_UPD_SHIFT)
#define CPU_SYS_DP_CLK_UPD(n)			((n) << CPU_SYS_DP_CLK_UPD_SHIFT)

/* CCU REG */
#define CCU_PLL_PERI0_CTRL_REG			(CCU_REG_BASE + 0x00A0)
#define CCU_PLL_PERI1_CTRL_REG			(CCU_REG_BASE + 0x00C0)
#define CCU_PLL_GPU_CTRL_REG			(CCU_REG_BASE + 0x00E0)
#define CCU_PLL_VIDEO0_CTRL_REG			(CCU_REG_BASE + 0x0120)
#define CCU_PLL_VIDEO1_CTRL_REG			(CCU_REG_BASE + 0x0140)
#define CCU_PLL_VE_CTRL_REG			(CCU_REG_BASE + 0x0220)
#define CCU_PLL_ADC_CTRL_REG			(CCU_REG_BASE + 0x0240)
#define CCU_PLL_AUDIO0_CTRL_REG			(CCU_REG_BASE + 0x0260)
#define CCU_AHB_CFG_REG				(CCU_REG_BASE + 0x0500)
#define CCU_APB0_CFG_REG			(CCU_REG_BASE + 0x0510)
#define CCU_APB1_CFG_REG			(CCU_REG_BASE + 0x0518)
#define CCU_APB_UART_CFG_REG			(CCU_REG_BASE + 0x0538)
#define CCU_CPU_SYS_DP_CLK_REG			(CCU_REG_BASE + 0x0548)
#define CCU_GIC_CFG_REG				(CCU_REG_BASE + 0x0560)
#define CCU_NSI_CFG_REG				(CCU_REG_BASE + 0x0580)
#define CCU_MBUS_CLK_REG			(CCU_REG_BASE + 0x0588)
#define CCU_MSGBOX_BGR_REG			(CCU_REG_BASE + 0x0744)
#define R_MSGBOX_GATE_RST_REG			(CCU_REG_BASE + 0x074c)
#define CCU_SPINLOCK_BGR_REG			(CCU_REG_BASE + 0x0724)
#define CCU_USB0_CLK_REG				(CCU_REG_BASE + 0x1300)
#define CCU_USB0_GAR_REG				(CCU_REG_BASE + 0x1304)
#define CCU_USB1_CLK_REG				(CCU_REG_BASE + 0x1308)
#define CCU_USB1_GAR_REG				(CCU_REG_BASE + 0x130C)
#define CCU_USB2P0_SYS_PHY_REF_CLK_REG	(CCU_REG_BASE + 0x1340)
#define CCU_USB2P0_SYS_GAR_REG			(CCU_REG_BASE + 0x1344)

/* CCU_CFG_REG */
#define CCU_CLK_SRC_SEL_SHIFT		(24)
#define CCU_CLK_SRC_SEL_MASK		(0x7 << CCU_CLK_SRC_SEL_SHIFT)
#define CCU_CLK_SRC_SEL(n)		((n) << CCU_CLK_SRC_SEL_SHIFT)

#define CCU_FACTOR_M_SHIFT		(0)
#define CCU_FACTOR_M_MASK		(0x1f << CCU_FACTOR_M_SHIFT)
#define CCU_FACTOR_M(n)			((n) << CCU_FACTOR_M_SHIFT)

/* CCU_USB0_CLK_REG */
#define USB0_CLKEN_MASK			(0x1 << 31)

/* CCU_USB0_GAR_REG */
#define USB0_EHCI_RST_MASK		(0x1 << 20)
#define USB0_OHCI_RST_MASK		(0x1 << 16)
#define USB0_EHCI_GATING_MASK		(0x1 << 4)
#define USB0_OHCI_GATING_MASK		(0x1 << 0)

/* CCU_USB1_CLK_REG */
#define USB1_CLKEN_MASK			(0x1 << 31)

/* CCU_USB1_GAR_REG */
#define USB1_EHCI_RST_MASK		(0x1 << 20)
#define USB1_OHCI_RST_MASK		(0x1 << 16)
#define USB1_EHCI_GATING_MASK		(0x1 << 4)
#define USB1_OHCI_GATING_MASK		(0x1 << 0)

/* CCU_USB2P0_SYS_PHY_REF_CLK_REG */
#define CCU_USB2P0_SYS_PHY_REF_CLK_MASK	(0x1 << 31)

/* CCU_USB2P0_SYS_GAR_REG */
#define USB2P0_SYS_RSTN_MASK	(1 << 16)
#define USB2P0_SYS_AHB_CLK_MASK	(1 << 0)

/* USB */
#define USB0_USB_CTRL_REG		(USB0_REG_BASE + 0x800)
#define USB1_USB_CTRL_REG		(USB1_REG_BASE + 0x800)
#define USB_STANDBY_CLOCK_SEL		(0x1 << 31)
#define USB_CTRL_NORMAL_MODE		(0x0 << 31)
#define USB_CTRL_STANDBY_MODE		(0x1 << 31) /* usb clock switch to RC 16M clock */

/* HSI */
#define USB_PHY_CTRL_REG(n)		(USB2P0_PHY_APP_BASE + 0x0010 + (n * 0x0020))
#define USB_PHY_SIDDQ_MASK		(1 << 3)
#define USB_RST_CTRL_REG(n)		(USB2P0_PHY_APP_BASE + 0x0028 + (n * 0x0020))
#define USB_PHY_RST_MASK		(1 << 0)
#define USB_DCTRL_REG			(USB2P0_SYS_DIG_APP_BASE + 0x0008)
#define USB_U3U2_U2_MAP_SEL		(1 << 0)

/* SMC */
#define SUNXI_SMC_PBASE			(0x0a000000)
#define SMC_ACTION_REG			(SUNXI_SMC_PBASE + 0x0004)
#define SMC_FUNCTION_BYPASS		(1UL<<6)
#define SMC_MST0_BYP_REG		(SUNXI_SMC_PBASE + 0x0070)
#define SMC_MST1_BYP_REG		(SUNXI_SMC_PBASE + 0x0074)
#define SMC_MST0_SEC_REG		(SUNXI_SMC_PBASE + 0x0080)
#define SMC_MST1_SEC_REG		(SUNXI_SMC_PBASE + 0x0084)
#define SMC_REGION_COUNT (8)	/* total 16, not all used, save some space */
#define SMC_BMP_ENABLE
#define SMC_BMP_SRAM_BASE		(SUNXI_SMC_PBASE + 0x1000)
#define SMC_BMP_SRAM_SIZE_KB		(2) /* SMC DRM bitmap SRAM size (KB) */
#define SMC_REGION_SETUP_LOW_REG(x)	(SUNXI_SMC_PBASE + 0x100 + 0x10*(x))
#define SMC_REGION_SETUP_HIGH_REG(x)	(SUNXI_SMC_PBASE + 0x104 + 0x10*(x))
#define SMC_REGION_ATTRIBUTE_REG(x)	(SUNXI_SMC_PBASE + 0x108 + 0x10*(x))
#define SUNXI_SMC_REG_MAX		(0x30c)	/* 0x21c + 15 * 0x10 */

/* SID */
#define SUNXI_SID_PBASE			(0x03408000)
#define SID_SEC_MODE_STA		(SUNXI_SID_PBASE + 0xA0)
#define SID_SEC_MODE_MASK		(0x1)

/* timerstamp regs */
#define SUNXI_TIMESTAMP_STA_CNT_LOW_REG		(TS_STAT_REG_BASE)
#define SUNXI_TIMESTAMP_STA_CNT_HI_REG		(TS_STAT_REG_BASE + 0x04)
#define SUNXI_TIMESTAMP_CTRL_CNT_LOW_REG	(TS_CTRL_REG_BASE + 0x08)
#define SUNXI_TIMESTAMP_CTRL_CNT_HI_REG		(TS_CTRL_REG_BASE + 0x0c)
#define SUNXI_TIMESTAMP_CTRL_CNT_FREQID_REG	(TS_CTRL_REG_BASE + 0x20)

/* prcm regs */
#define CPUS_CFG_REG			(R_PRCM_REG_BASE + 0x000)
#define AHBS_CFG_REG			(CPUS_CFG_REG)
#define APBS0_CFG_REG			(R_PRCM_REG_BASE + 0x00c)
#define APBS1_CFG_REG			(R_PRCM_REG_BASE + 0x010)
#define R_TIMER_BUS_GATE_RST_REG	(R_PRCM_REG_BASE + 0x11c)
#define R_PWM_BUS_GATE_RST_REG		(R_PRCM_REG_BASE + 0x13c)
#define R_UART_BUS_GATE_RST_REG		(R_PRCM_REG_BASE + 0x18c)
#define R_TWI_BUS_GATE_RST_REG		(R_PRCM_REG_BASE + 0x19c)
#define R_IR_RX_CLOCK_REG		(R_PRCM_REG_BASE + 0x1c0)
#define R_IR_RX_BUS_GATE_RST_REG	(R_PRCM_REG_BASE + 0x1cc)
#define RTC_BUS_GATE_RST_REG		(R_PRCM_REG_BASE + 0x20c)
#define RV_24M_CLK_REG			(R_PRCM_REG_BASE + 0x210)
#define PLL_CTRL_REG1			(R_PRCM_REG_BASE + 0x244)
#define VDD_SYS_PWROFF_GATING_REG	(R_PRCM_REG_BASE + 0x250)
#define ANA_PWR_RST_REG			(R_PRCM_REG_BASE + 0x254)
#define VDD_SYS_PWR_RST_REG		(R_PRCM_REG_BASE + 0x260)
#define NMI_INT_EN_REG			(R_PRCM_REG_BASE + 0x324)
#define LP_CTRL_REG			(R_PRCM_REG_BASE + 0x33c)
#define R_TIMER0_CLK_REG		(R_PRCM_REG_BASE + 0x100)

/* APBS0_CFG_REG */
#define APBS0_CLK_SRC_SEL_SHIFT (24)
#define APBS0_FACTOR_M_SHIFT (0)

#define APBS0_CLK_SRC_SEL_MASK (0x7 << APBS0_CLK_SRC_SEL_SHIFT)
#define APBS0_FACTOR_M_MASK (0x1f << APBS0_FACTOR_M_SHIFT)

#define APBS0_CLK_SRC_SEL(n) ((n) << APBS0_CLK_SRC_SEL_SHIFT)
#define APBS0_FACTOR_M(n) ((n) << APBS0_FACTOR_M_SHIFT)

/* APBS1_CFG_REG */
#define APBS1_CLK_SRC_SEL_SHIFT (24)
#define APBS1_FACTOR_M_SHIFT (0)

#define APBS1_CLK_SRC_SEL_MASK (0x7 << APBS1_CLK_SRC_SEL_SHIFT)
#define APBS1_FACTOR_M_MASK (0x1f << APBS1_FACTOR_M_SHIFT)

#define APBS1_CLK_SRC_SEL(n) ((n) << APBS1_CLK_SRC_SEL_SHIFT)
#define APBS1_FACTOR_M(n) ((n) << APBS1_FACTOR_M_SHIFT)

/* CPUS_CFG_REG */
#define CPUS_CLK_SRC_SEL_SHIFT (24)
#define CPUS_FACTOR_M_SHIFT (0)

#define CPUS_CLK_SRC_SEL_MASK (0x7 << CPUS_CLK_SRC_SEL_SHIFT)
#define CPUS_FACTOR_M_MASK (0x1f << CPUS_FACTOR_M_SHIFT)

#define CPUS_CLK_SRC_SEL(n) ((n) << CPUS_CLK_SRC_SEL_SHIFT)
#define CPUS_FACTOR_M(n) ((n) << CPUS_FACTOR_M_SHIFT)

/* VDD_SYS_PWROFF_GATING_REG */
#define VDD_USB2CPUS_GATING_SHIFT	(8)
#define VDD_SYS2USB_GATING_SHIFT	(3)

#define VDD_USB2CPUS_GATING_MASK	(0x1 << VDD_USB2CPUS_GATING_SHIFT)
#define VDD_SYS2USB_GATING_MASK		(0x1 << VDD_SYS2USB_GATING_SHIFT)

#define VDD_USB2CPUS_GATING(n)		((n) << VDD_USB2CPUS_GATING_SHIFT)
#define VDD_SYS2USB_GATING(n)		((n) << VDD_SYS2USB_GATING_SHIFT)

/* R_PIO_REG*/
#define PIN_REG_CFG(n, i)           ((volatile u32 *)(R_PIO_REG_BASE + ((n)) * 0x80 + ((i) >> 3) * 0x4 + 0x00))
#define PIN_REG_DLEVEL(n, i)        ((volatile u32 *)(R_PIO_REG_BASE + ((n)) * 0x80 + ((i) >> 3) * 0x4 + 0x20))
#define PIN_REG_PULL(n, i)          ((volatile u32 *)(R_PIO_REG_BASE + ((n)) * 0x80 + ((i) >> 4) * 0x4 + 0x30))
#define PIN_REG_DATA(n)             ((volatile u32 *)(R_PIO_REG_BASE + ((n)) * 0x80 + 0x10))

#define PIN_REG_CFG_VALUE(n, i)     readl(PIN_REG_CFG(n, i))
#define PIN_REG_DLEVEL_VALUE(n, i)  readl(PIN_REG_DLEVEL(n, i))
#define PIN_REG_PULL_VALUE(n, i)    readl(PIN_REG_PULL(n, i))
#define PIN_REG_DATA_VALUE(n)       readl(PIN_REG_DATA(n))

#define PIN_REG_INT_CFG(n, i)       ((volatile u32 *)(R_PIO_REG_BASE + (n) * 0x80 + ((i) >> 3) * 0x4 + 0x40))
#define PIN_REG_INT_CTL(n)          ((volatile u32 *)(R_PIO_REG_BASE + (n) * 0x80 + 0x50))
#define PIN_REG_INT_STAT(n)         ((volatile u32 *)(R_PIO_REG_BASE + (n) * 0x80 + 0x54))
#define PIN_REG_INT_DEBOUNCE(n)     ((volatile u32 *)(R_PIO_REG_BASE + (n) * 0x80 + 0x58))

#define PIN_REG_INT_CFG_VALUE(n, i)       readl(PIN_REG_INT_CFG(n, i))
#define PIN_REG_INT_CTL_VALUE(n)          readl(PIN_REG_INT_CTL(n))
#define PIN_REG_INT_STAT_VALUE(n)         readl(PIN_REG_INT_STAT(n))
#define PIN_REG_INT_DEBOUNCE_VALUE(n)     readl(PIN_REG_INT_DEBOUNCE(n))

#define PIN_NUM_INT_CFG_OFFSET(i)       ((i % 8) * 0x4)

#define CCU_HOSC_FREQ               (24000000)	//24M
#define CCU_LOSC_FREQ               (31250)	//31250
#define CCU_CPUS_PLL0_FREQ          (200000000)
#define CCU_APBS2_PLL0_FREQ         (200000000)
#define CCU_IOSC_FREQ               (16000000)	//16M
#define CCU_CPUS_POST_DIV           (100000000)	//cpus post div source clock freq
#define CCU_PERIPH0_FREQ            (600000000)	//600M

#define E902_WAKEUP_MASK0_REG (E902_CFG_BASE + 0x64)
#define E902_WAKEUP_MASK1_REG (E902_CFG_BASE + 0x68)

#define MASK0_START_INTERRUPT 16
#define MASK1_START_INTERRUPT 48

/* MSGBOX_REG */
#define HWMSGBOX_REG_BASE				(CPUX_HWMSGBOX_REG_BASE)
#define MSGBOX_VER_REG(m, n)            (HWMSGBOX_REG_BASE + 0x10 + m*0x1000 + n*0x100)
#define MSGBOX_MSG_DEBUG_REG(m, n)              (HWMSGBOX_REG_BASE + 0x40 + m*0x1000 + n*0x100)
#define MSGBOX_FIFO_STA_REG(m, n, c)    (HWMSGBOX_REG_BASE + 0x50 + m*0x1000 + n*0x100 + c*0x4)
#define MSGBOX_MSG_STA_REG(m, n, c)     (HWMSGBOX_REG_BASE + 0x60 + m*0x1000 + n*0x100 + c*0x4)
#define MSGBOX_MESG_REG(m, n, c)        (HWMSGBOX_REG_BASE + 0x70 + m*0x1000 + n*0x100 + c*0x4)

/* RESCAL_CTRL */
#define HDMI_RES1_REG		(SYS_CFG_REG_BASE + 0x0164)

#endif
