/****************************************************************************
* hdmirx/application/awMain.c
*
* Copyright (C) 2014-2016 AllWinnertech Ltd.
* Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

/*****************************************************************************
*                    Includes
*****************************************************************************/
#include "include.h"

/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/

GlobalData g_Data;
static struct softtimer call_back_10msTimer;

/*****************************************************************************
*                    Static Function Prototypes
*****************************************************************************/
static void awInitTimers(void);


/*****************************************************************************
*  Name        : tdCallBack10msTimer
*  Description : call back function for 10ms timer
*  Params      :
*  Returns     :
*****************************************************************************/
s32 awCallBack10msTimer(void *parg)
{
	g_Data.uc10MsCount++;

	tdReset10msUpdateFlags();  // set 10ms update flags

	if (g_Data.uc10MsCount % 2 == 0) { // every 20ms
		// SysCheckLED();
	}

	if (g_Data.uc10MsCount % 5 == 0) { // every 50ms
		tdReset50msUpdateFlags();  // set 50ms update flags
	}

	if (g_Data.uc10MsCount % 10 == 0) { // every 100ms
		// tdSync100msTimerService();
	}

	if (g_Data.uc10MsCount % 50 == 0) { // every 500ms
		tdReset500msUpdateFlags();  // set 500ms update flags
	}

	if (g_Data.uc10MsCount == 100) { // 1000ms = 1s
		g_Data.uc10MsCount = 0;      // reset counter
		tdReset1000msUpdateFlags();  // set 1000ms update flags
		tdCECUpdateTimer();
	}
	awUpdateExpiringTimers();

	return OK;
}

/*****************************************************************************
*  Name        : InitTimers
*  Description : Initional timers
*  Params      :
*  Returns     :
*****************************************************************************/
static void awInitTimers(void)
{
	uint8_t i;
	for (i = 0; i < HDMI_PORT_NUM_MAX; i++) {
		hotplug_pullup_timer[i] = TIMER_INACTIVE;
	}

	call_back_10msTimer.cycle = 0;
	call_back_10msTimer.expires = 0;
	call_back_10msTimer.cb = awCallBack10msTimer;
	call_back_10msTimer.arg = NULL;
	call_back_10msTimer.start = SOFTTIMER_OFF;
	add_softtimer(&call_back_10msTimer);
	start_softtimer(&call_back_10msTimer);
}

/*****************************************************************************
*  Name        : hdmirx_main_loop
*  Description : main process
*  Params      :
*  Returns     :
*****************************************************************************/
void hdmirx_main_loop(void)
{
	/* Handle expired timers */
	awHandleExpiredTimers();

	aw_cec_high_loop();
}

/*****************************************************************************
*  Name        : hdmirx_init_app
*  Description : Initional function
*  Params      :
*  Returns     :
*****************************************************************************/
void hdmirx_init_app(void)
{
	g_Data.uc1000msUpdateFlags = 0;
	g_Data.uc500msUpdateFlags = 0;
	g_Data.uc50msUpdateFlags = 0;
	g_Data.uc10msUpdateFlags = 0;
	g_Data.uc10MsCount = 0;
	g_Data.bCECPrint = _TRUE_;   //_TRUE_: turn on CEC debug print by default, _FALSE_: turn off.
	g_Data.bHDMIPortNumber = 3;  // need modify later.

	awInitTimers();

	tdHdmiEdidInit();

	aw_cec_high_init();
	SysRestoreWakeupData();
}
