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

#define CCU_REG_BASE            (0x02002000)
#define SYS_CFG_REG_BASE        (0x03000000)
#define CPUX_HWMSGBOX_REG_BASE  (0x03003000)
#define CPUS_HWMSGBOX_REG_BASE  (0x07094000)
#define RTC_REG_BASE            (0x07090000)
#define R_CPUCFG_REG_BASE       (0x07050000)
#define R_PRCM_REG_BASE         (0x07010000)
#define R_TMR01_REG_BASE        (0x07091000)
#define R_WDOG_REG_BASE         (0x07021000)
#define R_INTC_REG_BASE         (0x07024000)
#define R_PIO_REG_BASE          (0x07096000)
#define R_CIR_REG_BASE          (0x07040000)
#define R_UART_REG_BASE         (0x07080000)
#define R_TWI_REG_BASE          (0x07083000)
#define CPUSUBSYS_REG_BASE      (0x08000000)
#define TS_STAT_REG_BASE        (0x08010000)
#define TS_CTRL_REG_BASE        (0x08020000)
#define CPUCFG_REG_BASE         (0x08815000)
#define CPU_PLL_REG_BASE        (0x08870000)
#define RISCV_CLIC_BASE         (0xE0800000)
#define E902_CFG_BASE           (0x07032000)
#define USB0_REG_BASE           (0x04101000)
#define USB1_REG_BASE           (0x04200000)
#define CCU_BUS_CTL             (0x7FFF)
#define CPUS_MM_DRAM_LBP	(0x4002C000)

/* sys cfg reg */
#define SYS_CFG_VER_REG         (SYS_CFG_REG_BASE + 0x024)

/* e902 cfg reg */
#define E902_PAD_LPMD_REG       (E902_CFG_BASE + 0X80)

/* rtc domain record reg */
#define RTC_LOSC_CTRL           (RTC_REG_BASE + 0x000)
#define RTC_LOSC_OUT_GATING     (RTC_REG_BASE + 0x060)
#define RTC_RECORD_REG          (RTC_REG_BASE + 0x10c)
#define RTC_DTB_BASE_STORE_REG  (RTC_RECORD_REG)
#define RTC_GP_DATA5_REG	(RTC_REG_BASE + 0x114)
#define RTC_XO_WRT_PROTECT      (RTC_REG_BASE + 0x15c)
#define RTC_XO_CTRL_REG         (RTC_REG_BASE + 0x160)
#define RTC_VDD_OFF_GATING_CTRL (RTC_REG_BASE + 0x1f4)
#define RTC_IR_CODE_STORE_REG   (RTC_REG_BASE + 0x120)
#define RTC_CFG_REG             (RTC_REG_BASE + 0x310)

/* RTC_VDD_OFF_GATING_CTRL */
#define RTC_VCCIO_DEBONCE_EN    (0x100)
#define RTC_VCCIO_OUTPUT_EN     (0x01 << 7)
#define RTC_VCCIO_DETECT_EN     (0x01)

/*
 *  SID config info
 */
#define SUNXI_SID_PBASE               (0x03006000)
#define SID_SEC_MODE_STA              (SUNXI_SID_PBASE + 0xA0)
#define SID_SEC_MODE_MASK             (0x1)
#define SID_ALDO_EFUSE                (SUNXI_SID_PBASE + 0x214)

/*smc config info*/
#define SUNXI_SMC_PBASE               (0x03110000)
#define SMC_ACTION_REG                (SUNXI_SMC_PBASE + 0x0004)
#define SMC_MST0_BYP_REG              (SUNXI_SMC_PBASE + 0x0070)
#define SMC_MST1_BYP_REG              (SUNXI_SMC_PBASE + 0x0074)
#define SMC_MST0_SEC_REG              (SUNXI_SMC_PBASE + 0x0080)
#define SMC_MST1_SEC_REG              (SUNXI_SMC_PBASE + 0x0084)

#define SMC_REGION_COUNT              (8) /*total 16,not all used, save some space*/
#define SMC_REGIN_SETUP_LOW_REG(x)    (SUNXI_SMC_PBASE + 0x100 + 0x10*(x))
#define SMC_REGIN_SETUP_HIGH_REG(x)   (SUNXI_SMC_PBASE + 0x104 + 0x10*(x))
#define SMC_REGIN_ATTRIBUTE_REG(x)    (SUNXI_SMC_PBASE + 0x108 + 0x10*(x))

/* prcm regs */
#define CPUS_CFG_REG                (R_PRCM_REG_BASE + 0x000)
#define AHBS_CFG_REG                (CPUS_CFG_REG)
#define APBS0_CFG_REG               (R_PRCM_REG_BASE + 0x00c)
#define APBS1_CFG_REG               (R_PRCM_REG_BASE + 0x010)
#define R_TIMER_BUS_GATE_RST_REG    (R_PRCM_REG_BASE + 0x11c)
#define R_PWM_BUS_GATE_RST_REG      (R_PRCM_REG_BASE + 0x13c)
#define R_MSGBOX_GATE_RST_REG       (R_PRCM_REG_BASE + 0x17c)
#define R_UART_BUS_GATE_RST_REG     (R_PRCM_REG_BASE + 0x18c)
#define R_TWI_BUS_GATE_RST_REG      (R_PRCM_REG_BASE + 0x19c)
#define R_IR_RX_CLOCK_REG           (R_PRCM_REG_BASE + 0x1c0)
#define R_IR_RX_BUS_GATE_RST_REG    (R_PRCM_REG_BASE + 0x1cc)
#define RTC_BUS_GATE_RST_REG        (R_PRCM_REG_BASE + 0x20c)
#define RISCV_24M_CLK_REG           (R_PRCM_REG_BASE + 0x210)
#define PLL_CTRL_REG1               (R_PRCM_REG_BASE + 0x244)
#define VDD_SYS_PWROFF_GATING_REG   (R_PRCM_REG_BASE + 0x250)
#define ANA_PWR_RST_REG             (R_PRCM_REG_BASE + 0x254)
#define VDD_SYS_PWR_RST_REG         (R_PRCM_REG_BASE + 0x260)
#define NMI_INT_EN_REG              (R_PRCM_REG_BASE + 0x324)
#define LP_CTRL_REG                 (R_PRCM_REG_BASE + 0x33c)
#define R_TIMER0_CLK_REG            (R_PRCM_REG_BASE + 0x100)

/* APBS0_CFG_REG */
#define APBS0_CLK_SRC_SEL_SHIFT     (24)
#define APBS0_FACTOR_M_SHIFT        (0)

#define APBS0_CLK_SRC_SEL_MASK      (0x7 << APBS0_CLK_SRC_SEL_SHIFT)
#define APBS0_FACTOR_M_MASK         (0x1f << APBS0_FACTOR_M_SHIFT)

#define APBS0_CLK_SRC_SEL(n)        ((n) << APBS0_CLK_SRC_SEL_SHIFT)
#define APBS0_FACTOR_M(n)           ((n) << APBS0_FACTOR_M_SHIFT)

/* APBS1_CFG_REG */
#define APBS1_CLK_SRC_SEL_SHIFT     (24)
#define APBS1_FACTOR_M_SHIFT        (0)

#define APBS1_CLK_SRC_SEL_MASK      (0x7 << APBS1_CLK_SRC_SEL_SHIFT)
#define APBS1_FACTOR_M_MASK         (0x1f << APBS1_FACTOR_M_SHIFT)

#define APBS1_CLK_SRC_SEL(n)        ((n) << APBS1_CLK_SRC_SEL_SHIFT)
#define APBS1_FACTOR_M(n)           ((n) << APBS1_FACTOR_M_SHIFT)

/* CPUS_CFG_REG */
#define CPUS_CLK_SRC_SEL_SHIFT      (24)
#define CPUS_FACTOR_M_SHIFT         (0)

#define CPUS_CLK_SRC_SEL_MASK       (0x7 << CPUS_CLK_SRC_SEL_SHIFT)
#define CPUS_FACTOR_M_MASK          (0x1f << CPUS_FACTOR_M_SHIFT)

#define CPUS_CLK_SRC_SEL(n)         ((n) << CPUS_CLK_SRC_SEL_SHIFT)
#define CPUS_FACTOR_M(n)            ((n) << CPUS_FACTOR_M_SHIFT)

//name by pll function
#define CCU_PLL_DDR0_REG            (CCU_REG_BASE + 0X020)
#define CCU_PLL_PERIPH0_REG         (CCU_REG_BASE + 0x0A0)
#define CCU_PLL_PERIPH1_REG         (CCU_REG_BASE + 0x0C0)
#define CCU_PLL_VIDEO0_REG          (CCU_REG_BASE + 0x120)
#define CCU_PLL_VIDEO1_REG          (CCU_REG_BASE + 0x140)
#define CCU_PLL_VE_REG              (CCU_REG_BASE + 0x220)
#define CCU_PLL_AUDIO0_REG          (CCU_REG_BASE + 0x260)
#define CCU_PLL_AUDIO1_REG          (CCU_REG_BASE + 0x280)
#define CCU_PLL_NPU_REG             (CCU_REG_BASE + 0x2A0)

#define CCU_AHB_CFG_REG             (CCU_REG_BASE + 0x500)
#define CCU_APB0_CFG_REG            (CCU_REG_BASE + 0x510)
#define CCU_APB1_CFG_REG            (CCU_REG_BASE + 0x518)
#define CCU_APB_UART_CFG_REG        (CCU_REG_BASE + 0x538)
#define CCU_TRACE_CFG_REG           (CCU_REG_BASE + 0x540)
#define CCU_GIC_CFG_REG             (CCU_REG_BASE + 0x560)
#define CCU_NSI_CFG_REG             (CCU_REG_BASE + 0x580)
#define CCU_MBUS_CLK_REG            (CCU_REG_BASE + 0x588)
#define CCU_MSGBOX_BGR_REG          (CCU_REG_BASE + 0x744)
#define CCU_SPINLOCK_BGR_REG        (CCU_REG_BASE + 0x724)
#define CCU_USB0_CLOCK_REG          (CCU_REG_BASE + 0x1300)
#define CCU_USB1_CLOCK_REG          (CCU_REG_BASE + 0x1308)
#define CCU_USB0_BUS_GATING_RST_REG (CCU_REG_BASE + 0x1304)
#define CCU_USB1_BUS_GATING_RST_REG (CCU_REG_BASE + 0x130c)
#define CCU_CLK24M_GATE_EN_REG      (CCU_REG_BASE + 0x1310)

#define USB0_USB_CTRL_REG           (USB0_REG_BASE + 0x800)
#define USB1_USB_CTRL_REG           (USB1_REG_BASE + 0x800)
#define USB_STANDBY_CLOCK_SEL       (0x1 << 31)
#define USB_CTRL_NORMAL_MODE        (0x0 << 31)
#define USB_CTRL_STANDBY_MODE       (0x1 << 31) /* usb clock switch to RC 16M clock */
/* host controller functional state */
#define OHCI_CTRL_HCFS              (3 << 6)
/* pre-shifted values for HCFS */
#define OHCI_USB_RESET              (0 << 6)
#define OHCI_USB_RESUME             (1 << 6)
#define OHCI_USB_OPER               (2 << 6)
#define OHCI_USB_SUSPEND            (3 << 6)

/* CPU CLK */
#define CPU_CLK_REG                 (CPU_PLL_REG_BASE + 0x044)
#define DSU_CLK_REG                 (CPU_PLL_REG_BASE + 0x04c)
#define CPU_L0_REG                  (CPU_PLL_REG_BASE + 0x04)
#define DSU_PLL_REG                 (CPU_PLL_REG_BASE + 0x08)

/* CPU PLL */
#define CPU_PLL_REG(n)			(CPU_PLL_REG_BASE + (n) * 4)
#define CPU_PLL_EN			(1 << 31)
#define CPU_PLL_LDO_EN			(1 << 30)
#define CPU_PLL_LOCK_EN			(1 << 29)
#define CPU_PLL_LOCK_STATUS		(1 << 28)
#define CPU_PLL_OUTPUT			(1 << 27)
#define CPU_PLL_UPDATE			(1 << 26)
#define CPU_PLL_FACTOR			0x3FFF0F

/* CPU_CLK_REG */
#define CPU_CLK_SRC_SEL_SHIFT		(24)
#define CPU_CLK_SRC_SEL_MASK		(0x7 << CPU_CLK_SRC_SEL_SHIFT)
#define CPU_CLK_SRC_SEL(n)		((n) << CPU_CLK_SRC_SEL_SHIFT)

#define CPU_FACTOR_P_MASK		(0x3 << 16)

/* CCU_CFG_REG */
#define CCU_CLK_SRC_SEL_SHIFT		(24)
#define CCU_FACTOR_M_SHIFT		(0)
#define CCU_CLK_SRC_SEL_MASK		(0x7 << CCU_CLK_SRC_SEL_SHIFT)
#define CCU_FACTOR_M_MASK		(0x3ff << CCU_FACTOR_M_SHIFT)
#define CCU_CLK_SRC_SEL(n)		((n) << CCU_CLK_SRC_SEL_SHIFT)
#define CCU_FACTOR_M(n)			((n) << CCU_FACTOR_M_SHIFT)
#define CCU_CLK_CTL_MASK		(1 << 31)

/* DSU_CLK_REG */
#define DSU_CLK_SRC_SEL_SHIFT		(24)
#define DSU_FACTOR_P_MASK		(0x3 << 16)
#define DSU_FACTOR_N_MASK		(0x3 << 8)
#define DSU_FACTOR_M1_MASK		(0x3 << 2)
#define DSU_FACTOR_M_MASK		(0x3 << 0)
#define DSU_CLK_SRC_SEL_MASK		(0x7 << DSU_CLK_SRC_SEL_SHIFT)
#define DSU_FACTOR_MASK		(DSU_FACTOR_P_MASK | DSU_FACTOR_N_MASK | DSU_FACTOR_M1_MASK | DSU_FACTOR_M_MASK)
#define DSU_CLK_SRC_SEL(n)		((n) << DSU_CLK_SRC_SEL_SHIFT)
#define DSU_FACTOR_P(n)			((n) << DSU_FACTOR_P_SHIFT)

/* ANA_PWR_RST_REG */
#define ANA_EN_SHIFT		(0)
#define ANA_EN_MCU_SHIFT	(1)
#define ANA_EN_CPU_SHIFT	(3)

#define ANA_EN_MASK		(0x1 << ANA_EN_SHIFT)
#define ANA_EN_MCU_MASK		(0x1 << ANA_EN_MCU_SHIFT)
#define ANA_EN_CPU_MASK		(0x1 << ANA_EN_CPU_SHIFT)

#define ANA_EN_CTRL(n)		((n) << ANA_EN_SHIFT)
#define ANA_EN_MCU_CTRL(n)	((n) << ANA_EN_MCU_SHIFT)
#define ANA_EN_CPU_CTRL(n)	((n) << ANA_EN_CPU_SHIFT)

/* VDD_SYS_PWR_RST_REG */
#define VDD_SYS_MODULE_RST_SHIFT	(0)
#define VDD_SYS_MODULE_RST_MASK		(0x1 << VDD_SYS_MODULE_RST_SHIFT)
#define VDD_SYS_MODULE_RST(n)		((n) << VDD_SYS_MODULE_RST_SHIFT)

/* VDD_SYS_PWROFF_GATING_REG */
#define VDD_USB2CPUS_GATING_SHIFT	(8)
#define VDD_SYS2USB_GATING_SHIFT	(3)
#define VDD_SYS2CPUS_GATING_SHIFT	(2)

#define VDD_USB2CPUS_GATING_MASK	(0x1 << VDD_USB2CPUS_GATING_SHIFT)
#define VDD_SYS2CPUS_GATING_MASK	(0x1 << VDD_SYS2CPUS_GATING_SHIFT)

#define VDD_SYS2CPUS_GATING(n)		((n) << VDD_SYS2CPUS_GATING_SHIFT)
#define VDD_USB2CPUS_GATING(n)		((n) << VDD_USB2CPUS_GATING_SHIFT)
#define VDD_SYS2USB_GATING(n)		((n) << VDD_SYS2USB_GATING_SHIFT)

/* CCU_PLL_REG */
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

#define PLL_FACTOR_M0_MASK		(0x1 << 0)
#define PLL_FACTOR_M1_MASK		(0x1 << 1)
#define PLL_FACTOR_P2_MASK		(0x1f << 2)
#define PLL_FACTOR_N_MASK		(0xff << 8)
#define PLL_FACTOR_P0_MASK		(0x1f << 16)
#define PLL_FACTOR_P1_MASK		(0x1f << 20)

#define PLL_FACKOR_MASK			(PLL_FACTOR_M0_MASK | PLL_FACTOR_M1_MASK | PLL_FACTOR_P2_MASK |\
					PLL_FACTOR_N_MASK | PLL_FACTOR_P0_MASK | PLL_FACTOR_P1_MASK)

/* CCU_USB0_CLOCK_REG */
#define SCLK_GATING_OHCI0_MASK		(0x1 << 31)
#define USBPHY0_RST_MASK		(0x1 << 30)
#define SCLK_GATING_USBPHY0_MASK	(0x1 << 29) /* reserved */

/* CCU_USB1_CLOCK_REG */
#define SCLK_GATING_OHCI1_MASK		(0x1 << 31)
#define USBPHY1_RST_MASK		(0x1 << 30)
#define SCLK_GATING_USBPHY1_MASK	(0x1 << 29) /* reserved */

/* CCU_USB0_BUS_GATING_RST_REG */
#define USBEHCI0_RST_MASK		(0x1 << 20)
#define USBOHCI0_RST_MASK		(0x1 << 16)
#define USBEHCI0_GATING_MASK		(0x1 << 4)
#define USBOHCI0_GATING_MASK		(0x1 << 0)

/* CCU_USB1_BUS_GATING_RST_REG */
#define USBEHCI1_RST_MASK		(0x1 << 20)
#define USBOHCI1_RST_MASK		(0x1 << 16)
#define USBEHCI1_GATING_MASK		(0x1 << 4)
#define USBOHCI1_GATING_MASK		(0x1 << 0)

/* CCU_CLK24M_GATE_EN_REG */
#define USB24M_GATE_EN_MASK		(0x1 << 0)

#define CPU_SSC_REG(n)			(CPU_PLL_REG_BASE + 0x38 + (n) * 4)
#define CPU_SSC_EN			(1 << 31)
#define CPU_SSC_CLK_SEL			(1 << 29)
#define CPU_SSC_FACTOR			((0x1ffff << 12) | 0x0f)

/* R_PIO_REG*/
#define PIN_REG_CFG(n, i)	((volatile u32 *)(R_PIO_REG_BASE + ((n) - 1) * 0x30 + ((i) >> 3) * 0x4 + 0x00))
#define PIN_REG_DLEVEL(n, i)	((volatile u32 *)(R_PIO_REG_BASE + ((n) - 1) * 0x30 + ((i) >> 3) * 0x4 + 0x14))
#define PIN_REG_PULL(n, i)	((volatile u32 *)(R_PIO_REG_BASE + ((n) - 1) * 0x30 + ((i) >> 4) * 0x4 + 0x24))
#define PIN_REG_DATA(n)		((volatile u32 *)(R_PIO_REG_BASE + ((n) - 1) * 0x30 + 0x10))

#define PIN_REG_CFG_VALUE(n, i)		readl(PIN_REG_CFG(n, i))
#define PIN_REG_DLEVEL_VALUE(n, i)	readl(PIN_REG_DLEVEL(n, i))
#define PIN_REG_PULL_VALUE(n, i)	readl(PIN_REG_PULL(n, i))
#define PIN_REG_DATA_VALUE(n)		readl(PIN_REG_DATA(n))

#define PIN_REG_INT_CFG(n, i)	((volatile u32 *)(R_PIO_REG_BASE + (n - 1) * 0x20 + ((i) >> 3) * 0x4 + 0x200))
#define PIN_REG_INT_CTL(n)	((volatile u32 *)(R_PIO_REG_BASE + (n - 1) * 0x20 + 0x210))
#define PIN_REG_INT_STAT(n)	((volatile u32 *)(R_PIO_REG_BASE + (n - 1) * 0x20 + 0x214))
#define PIN_REG_INT_DEBOUNCE(n)	((volatile u32 *)(R_PIO_REG_BASE + (n - 1) * 0x20 + 0x218))

#define PIN_NUM_INT_CFG_OFFSET(i)	((i % 8) * 0x4)

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

#endif
