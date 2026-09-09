/****************************************************************************
 * hdmirx/awCECHigh.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

#ifndef _TCECHIGH_H_
#define _TCECHIGH_H_

#include "include.h"
#include "awCECLow.h"

#define CEC_TX_STATUS_OK            (1 << 0)
#define CEC_TX_STATUS_NACK          (1 << 1)
#define CEC_TX_STATUS_ERROR         (1 << 2)
#define CEC_TX_STATUS_MAX_RETRIES   (1 << 3)

#define CEC_RX_STATUS_OK            (1 << 0)
#define CEC_RX_STATUS_TIMEOUT       (1 << 1)
#define CEC_RX_STATUS_FEATURE_ABORT (1 << 2)

#define QUEUE_SIZE 24
#define OneTouchPlay_Wakeup 10
#define TV_LA_Address 0x00   // TV
#define BC_LA_Address 0x0F   // Broadcast
#define ARC_LA_Address 0x05  // Audio system

/* TIME CONSTANTS FOR 1S TIMER */
#define ONE_TOUCH_PLAY_ADDRESSING_TIME (10 + 1)

/* RTC_POWER_OFF_RECORD1_REG */
#define CECEnable_bit 2
#define CECWakeupEnable_bit 3

#define RTCSettingValid 0x3

#define BIT(n) (1U << (n))

enum {
	AW_HDMI_CEC_STAT_WAKEUP 	= BIT(0),
	AW_HDMI_CEC_STAT_DONE		= BIT(1),
	AW_HDMI_CEC_STAT_EOM		= BIT(2),
	AW_HDMI_CEC_STAT_NACK		= BIT(3),
	AW_HDMI_CEC_STAT_ARBLOST	= BIT(4),
	AW_HDMI_CEC_STAT_ERROR_INIT	= BIT(5),
	AW_HDMI_CEC_STAT_ERROR_FOLL	= BIT(6),
};

enum {
	AW_HDMI_CEC_FRAME_TYPE_RETRY    = 0,
	AW_HDMI_CEC_FRAME_TYPE_NORMAL   = 1,
	AW_HDMI_CEC_FRAME_TYPE_IMMED    = 2,
};

enum {
	CEC_WAKEEVENT_None = 0,
	CEC_WAKEEVENT_TEXT_VIEW_ON,
	CEC_WAKEEVENT_IMAGE_VIEW_ON,
	CEC_WAKEEVENT_ACTIVE_SOURCE,
	CEC_WAKEEVENT_ADDRING = 8,
	CEC_WAKEEVENT_TEXT_VIEW_ON_ADDRING,
	CEC_WAKEEVENT_IMAGE_VIEW_ON_ADDRING,
	CEC_WAKEEVENT_ACTIVE_SOURCE_ADDRING,
};
enum {
	CEC_PWRSTATUS_ON = 0,
	CEC_PWRSTATUS_STB,
	CEC_PWRSTATUS_STB2ON,
	CEC_PWRSTATUS_ON2STB,
};

typedef uint16_t TLogicDevices;
typedef uint16_t TPhysicalAddr;
typedef struct {
	TLogicDevices deviceEnum;       // enum of present devices
	TLogicDevices newDeviceEnum;    // new attached devices which have not been addressed
	TLogicDevices deviceAddressed;  // the device has been addressed by main chip
	TPhysicalAddr physicalAddr[16];
} TDeviceList;

typedef struct tagCECMsgQueue {
	TCECMessage queue[QUEUE_SIZE];
	TCECMessage *pHead;
	TCECMessage *pPush;
	TCECMessage *pPop;
} CECMsgQueue;

void hdmitx_update_type(unsigned char type);
void hdmitx_standby_loop(void);
void tdCECUpdateTimer(void);
void tdCEC_Standby(void);
void tdCEC_OTPMessageInit(void);
void tdCEC_SetOTPFlag(uint8_t status);
void tdCEC_EnableCec(bool enable);
void tdCEC_EnableCecWakeup(bool enable);
uint8_t tdCEC_GetCecWakeFlag(void);
void SysBackupWakeupData(void);
void SysRestoreWakeupData(void);
sint32_t aw_cec_high_Send(unsigned char *msg, unsigned size, unsigned frame_type);
void aw_cec_high_process_msg(TCECMessage *rpPacket);
void aw_cec_high_process_setting(uint8_t Type, uint8_t rpPacket);
void aw_cec_high_init(void);
void aw_cec_high_loop(void);
#endif
