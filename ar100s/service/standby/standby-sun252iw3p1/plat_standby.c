/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                               standby module
*
*                                    (c) Copyright 2012-2016, superm China
*                                             All Rights Reserved
*
* File    : extended_super_standby.c
* By      : superm
* Version : v1.0
* Date    : 2013-1-16
* Descript: extended super-standby module public header.
* Update  : date                auther      ver     notes
*           2013-1-16 17:04:28  superm       1.0     Create this file.
*********************************************************************************************************
*/
#include <libfdt.h>
#include "../standby_i.h"
#include "wakeup_source.h"
#include "cpucfg_regs.h"

extern u32 volatile wakeup_source;


static s32 result;
static u32 suspend_lock;
static u32 standby_type;

static void suspend_para_prepare(void)
{

}

static s32 standby_dts_parse(void)
{
	return 0;
}

static void wait_wakeup(void)
{
	save_state_flag(REC_ESTANDBY | REC_WAIT_WAKEUP | 0x01);
	// wakeup_timer_start();
	wakeup_source = NO_WAKESOURCE;
	LOG("out of wait wakeup:\n");
	while (1) {
		LOG("wait for wakeup source \n");
		/*
		 * maybe add user defined task process here
		 */
		if (wakeup_source != NO_WAKESOURCE) {
			LOG("wakeup: %d\n", wakeup_source);
			break;
		}
		/* e902 low power mode */
		cpu_enter_doze();
	}
	wakeup_timer_stop();
}

static void wait_cpu0_resume(void)
{
	s32 ret;
	struct message message;

	/* no paras for resume notify message */
	message.paras = NULL;

	printk("wait ac327 resume...\n");

	/* wait cpu0 restore finished. */
	while (1) {
		ret = hwmsgbox_query_message(&message, 0);
		if (ret != OK)
			continue; /* no message, query again */

		/* query valid message */
		if (message.type == SSTANDBY_RESTORE_NOTIFY) {
			/* cpu0 restore, feedback wakeup event. */
			LOG("cpu0 restore finished\n");
			/* init feedback message */
			message.count = 1;
			message.result = 0;
			message.paras = (u32 *)&wakeup_source;
			/* synchronous message, need feedback. */
			hwmsgbox_feedback_message(&message, SEND_MSG_TIMEOUT);
			break;
		} else {
			/* invalid message detected, ignore it, by sunny at 2012-6-28 11:33:13. */
			ERR("standby ignore message [%x]\n", message.type);
		}
	}

	wakeup_source = NO_WAKESOURCE;
}

#define CPUPLL_DEBUGx
#ifdef CPUPLL_DEBUG
static void dbg_log(void)
{
	int i;
	LOG("CPUA_CLK_REG 0x%x\n", readl(CPUA_CLK_REG));
	LOG("CPUB_CLK_REG 0x%x\n", readl(CPUB_CLK_REG));
	LOG("DSU_CLK_REG 0x%x\n", readl(DSU_CLK_REG));
	for (i = 0; i < 4; i++) {
		LOG("CPU_PLL_PAT0_REG[%d] 0x%x\n", i, readl(CPU_PLL_PAT0_REG(i)));
		LOG("CPU_PLL_PAT1_REG[%d] 0x%x\n", i, readl(CPU_PLL_PAT1_REG(i)));
		/* CPU BACK PLL hasn't LFM reg */
		if (i)
			LOG("CPU_PLL_LFM_REG[%d] 0x%x\n", i - 1, readl(CPU_PLL_LFM_REG(i - 1)));
		LOG("CPU_PLL_REG[%d] 0x%x\n", i, readl(CPU_PLL_REG(i)));
	}
}
#else
#define dbg_log()
#endif


static u32 platform_standby_type(void)
{
	u32 type = 0;

	/* usb standby */
	if (interrupt_get_enabled(INTC_R_USB_IRQ)) {
		type |= CPUS_WAKEUP_USB;
	}

	return type;
}

#define CPUX_CFG_BASE				(0x08008000)
#define CPUX_CORE_STATUS_REG(n)		(CPUX_CFG_BASE + 0x110 + (n * 0x80))

static inline void wait_for_cpux_wfi(void)
{
	while ((readl(CPUX_CORE_STATUS_REG(0)) & 0x01)) {
		/* wait core0 enter wfi mode*/
	}
}

#define sun252IW3P1_PMC_BASE		(0x08000000)
#define PMC_BASE(n)			(sun252IW3P1_PMC_BASE + 0x1000 * (n))
#define SW_MODE_CTRL(m)			(0x100 + 0x4 * (m))
#define SW_CTRL_CONFIG			0x38
#define IGN_WKUP_TRIG			BIT(1)

static s32 standby_process_init(struct message *pmessage)
{
	suspend_lock = 1;

	LOG("standby_process_init\n");
	suspend_para_prepare();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x1);

	wait_for_cpux_wfi();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x2);

	cpucfg_cpu_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x3);

	return OK;
}

static s32 standby_process_exit(struct message *pmessage)
{
	u32 resume_entry = pmessage->paras[1];

	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x1);
	LOG("standby_process_exit\n");

	cpucfg_cpu_resume(resume_entry);
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xC);

	wait_cpu0_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xD);

	suspend_lock = 0;

	return OK;
}

/*
*********************************************************************************************************
*                                       ENTEY OF TALK-STANDBY
*
* Description:  the entry of extended super-standby.
*
* Arguments  :  request:request command message.
*
* Returns    :  OK if enter extended super-standby succeeded, others if failed.
*********************************************************************************************************
*/
static s32 standby_entry(struct message *pmessage)
{
	save_state_flag(REC_ESTANDBY | REC_ENTER);

	standby_dts_parse();

	save_state_flag(REC_ESTANDBY | REC_ENTER | 0x1);

	/* backup cpus source clock */
	// iosc_freq_init();

	/* parse standby type from enabled interrupt */
	standby_type = platform_standby_type();

	/*
	 * --------------------------------------------------------------------------
	 *
	 * initialize enter super-standby porcess
	 *
	 * --------------------------------------------------------------------------
	 */
	save_state_flag(REC_ESTANDBY | REC_BEFORE_INIT);
	result = standby_process_init(pmessage);
	save_state_flag(REC_ESTANDBY | REC_AFTER_INIT);

	/*
	 * --------------------------------------------------------------------------
	 *
	 * wait valid wakeup source porcess
	 *
	 * --------------------------------------------------------------------------
	 */
	LOG("wait wakeup\n");
	save_state_flag(REC_ESTANDBY | REC_WAIT_WAKEUP);
	wait_wakeup();

	/*
	 * --------------------------------------------------------------------------
	 *
	 * exit super-standby wakeup porcess
	 *
	 * --------------------------------------------------------------------------
	 */
	save_state_flag(REC_ESTANDBY | REC_BEFORE_EXIT);
	standby_process_exit(pmessage);
	save_state_flag(REC_ESTANDBY | REC_AFTER_EXIT);

	return OK;
}

u32 is_suspend_lock(void)
{
	return suspend_lock;
}

int cpu_op(struct message *pmessage)
{
	u32 mpidr = pmessage->paras[0];
	u32 entrypoint = pmessage->paras[1];
	u32 cpu_state = pmessage->paras[2];
	u32 cluster_state = pmessage->paras[3]; /* unused variable */
	u32 system_state = pmessage->paras[4];

	LOG("mpidr:%x, entrypoint:%x; cpu_state:%x, cluster_state:%x, system_state:%x\n", mpidr, entrypoint, cpu_state, cluster_state, system_state);
	if (entrypoint && system_state == arisc_power_off) {
		standby_entry(pmessage);
	}

	return 0;
}

static void system_shutdown(void)
{

}

static void system_reset(void)
{

}

int sys_op(struct message *pmessage)
{
	u32 state = pmessage->paras[0];

	LOG("state:%x\n", state);

	switch (state) {
	case arisc_system_shutdown:
		{
			save_state_flag(REC_SHUTDOWN | 0x101);
			// pmu_charging_reset();
			system_shutdown();
			break;
		}
	case arisc_system_reset:
	case arisc_system_reboot:
		{
			save_state_flag(REC_SHUTDOWN | 0x102);
			system_reset();
			break;
		}
	case arisc_uboot_shutdown:
		{
			save_state_flag(REC_SHUTDOWN | 0x103);
			system_shutdown();
			break;
		}
	default:
		{
			WRN("invaid system power state (%d)\n", state);
			return -EINVAL;
		}
	}

	return 0;
}

s32 fake_poweroff(struct message *pmessage)
{
	return 0;
}

/* feedback pmu irq */
s32 get_pmu_irq(struct message *pmessage)
{
	return OK;
}

u8 get_vdd_sys_reg(void)
{
	return 0;
}

int set_vdd_sys_reg(int set_vol, int onoff)
{
	return 0;
}