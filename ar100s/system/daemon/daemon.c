/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                daemon module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : daemon.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-13
* Descript: daemon module.
* Update  : date                auther      ver     notes
*           2012-5-13 15:06:10  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "daemon_i.h"

/* system daemon vars */
extern u32 debug_level;
u32 dtb_base;

/* the list of daemon notifier */
static struct notifier *daemon_list;

int daemon_register_service(__pNotifier_t pcb)
{
	return notifier_insert(&daemon_list, pcb);
}

static s32 startup_state_notify(s32 result)
{
	struct message message;
	u32 arisc_version[13] = {0};
	s32 ret;

	LOG("feedback startup result [%d]\n", result);

	save_state_flag(REC_HOTPULG | 0xd);

	/* initialize message */
	message.type = AR100_STARTUP_NOTIFY;
	message.attr = MESSAGE_ATTR_HARDSYN;
	message.result = result;
	message.count = sizeof(arisc_version) / sizeof(u32);
	message.paras = arisc_version;

	/* must end with '\0' */
	strncpy((char *)(arisc_version), SUB_VER, sizeof(arisc_version) - 1);

	ret = hwmsgbox_send_message(&message, SEND_MSG_TIMEOUT);

	if (ret == OK)
		LOG("send notify succeed\n");
	else
		LOG("send notify failed\n");

	save_state_flag(REC_HOTPULG | 0xe);

	return OK;
}

/*
 * NOTE:
 * this function is not reentrant and only can be used in process context,
 * and msg_paras define as static variable to save stack.
 */
static void message_process_loop(void)
{
	s32 ret;
	struct message message;
	static u32 msg_paras[32];

	message.paras = msg_paras;

	ret = hwmsgbox_query_message(&message, 0);
	if (ret == OK)
		message_coming_notify(&message);

	amp_msgbox_query_message();
}

extern void plat_process_loop(void);
void __attribute__((weak)) plat_process_loop(void)
{
	hdmi_main_loop();
	return;
};

static void daemon_main(void)
{
	/* initialize cpu */
	cpu_init();

	/* daemon & message & user defined task loop process */
	LOG("daemon service setup...\n");
	while (1) {
		/* message loop process */
		message_process_loop();

		/* daemon list process */
		if (((current_time_tick()) % DAEMON_ONCE_TICKS) == 0) {
			/* daemon run one time */
			printk("------------------------------\n");
			LOG("system tick:%d\n", DAEMON_ONCE_TICKS);
			LOG("debug_mask:%d\n", debug_level);
			LOG("uart_buadrate:%d\n", uart_get_baudrate());
			notifier_notify(&daemon_list, DAEMON_RUN_NOTIFY, 0);
		}

		/*
		 * maybe add user defined task process here
		 * refer to daemon list process
		 */
		plat_process_loop();
	}
}

static void dtb_base_init(void)
{
	dtb_base = read_dtb_base();
	cpucfg_set_little_endian_address((void *)(dtb_base), (void *)(dtb_base + ARISC_DTS_SIZE));
}

extern void plat_init_early(void);
void __attribute__((weak)) plat_init_early(void)
{
	return;
};

/*
*********************************************************************************************************
*                                       STARTUP ENTRY
*
* Description:  the entry of startup.
*
* Arguments  :  none.
*
* Returns    :  none.
*********************************************************************************************************
*/
void startup_entry(void)
{
	dtb_base_init();

	plat_init_early();

	notifier_init();
	save_state_flag(REC_HOTPULG | 0x0);

	ccu_init();
	save_state_flag(REC_HOTPULG | 0x1);

	pin_init();
	save_state_flag(REC_HOTPULG | 0x2);

	save_state_flag(REC_HOTPULG | 0x3);

	interrupt_init();
	save_state_flag(REC_HOTPULG | 0x4);

	arisc_para_init();
	save_state_flag(REC_HOTPULG | 0x5);

	debugger_init();
	save_state_flag(REC_HOTPULG | 0x6);
	LOG("debugger system ok\n");

	twi_init();
	LOG("twi driver ok\n");
	save_state_flag(REC_HOTPULG | 0x7);

	pmu_init();
	bmu_init();
	save_state_flag(REC_HOTPULG | 0x8);
	LOG("pmu & bmu driver ok\n");

	hwmsgbox_init();
	amp_msgbox_init();
	save_state_flag(REC_HOTPULG | 0x9);
	LOG("hwmsgbox driver ok\n");

	cpucfg_init();
	save_state_flag(REC_HOTPULG | 0xa);
	LOG("cpucfg driver ok\n");

	message_manager_init();
	LOG("message manager ok\n");

	timer_init();
	LOG("timer driver ok\n");

	standby_init();
	LOG("standby service ok\n");

	time_ticks_init();
	LOG("time ticks ok\n");

	watchdog_init();
	save_state_flag(REC_HOTPULG | 0xc);
	LOG("watchdog ok\n");

	/* feedback the startup state to ac327 */
	startup_state_notify(OK);
	platform_dts_parse_late();
	set_paras();
	save_state_flag(REC_HOTPULG | 0xf);
	LOG("startup feedback ok\n");

	hdmi_init();
	save_state_flag(REC_HOTPULG | 0x10);
	LOG("ar100 hdmi init done\n");

	ndma_init();
	save_state_flag(REC_HOTPULG | 0x11);
	LOG("dma driver ok\n");

	LOG("ar100 firmware version : %s\n", SUB_VER);
	/* enter daemon process main. */
	daemon_main();

	/* to avoid daemon main return. */
	ERR("system daemon exit\n");
	while (1)
		;
}

