/****************************************************************************
 * hdmirx/awUsrProtocol.h
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/
#ifndef _USERPROTOCOL_H_
#define _USERPROTOCOL_H_

typedef enum _tagMsg_HostToMCUType {
	HOST_TO_MCU_POWER_MODE = 0x10,
	HOST_TO_MCU_SET_PARAMETER,
	HOST_TO_MCU_DEBUG_COMMAND,
	HOST_TO_MCU_CEC_MESSAGE,
	HOST_TO_MCU_CEC_SETTING,
	HOST_TO_MCU_REQUEST_INFORMATION,
	/* start define for hdmi tx */
	HOST_TO_MCU_TX_BASE = 0x20,
	HOST_TO_MCU_SET_CEC_ON = HOST_TO_MCU_TX_BASE,
	HOST_TO_MCU_SET_CEC_OFF,
	HOST_TO_MCU_GET_CEC_BUFF,
	/* end define for hdmi tx */
} Msg_HostToMCUType;

typedef enum _tagHostSetParam {
	HOST_PARAM_CMD_AW_HDMIPortRemap = 0x0,
	HOST_PARAM_CMD_UpdateEDID = 0x01,
	HOST_PARAM_CMD_HOTPLUG,
	HOST_PARAM_CMD_EDIDVersion,
	HOST_PARAM_CMD_HDMI5VFlag,
	HOST_PARAM_CMD_DynamicAudioMode,
	HOST_PARAM_CMD_ResetEDIDModule = 0x20,
} HostSetParam;

typedef enum _tagHostDbgCmd {
	HOST_DBG_CMD_PrintEDIDTable = 0x01,
} HostDbgCmd;

typedef enum _tagHostReqInfo {
	HOST_REQ_CMD_HDMIPortNumber = 0x01,
	HOST_REQ_CMD_EDIDStatus,
	HOST_REQ_CMD_EDID_data,
} HostReqInfo;

typedef enum _tagMCUTypeInfo {
	MCU_CMD_NotifyKeyEntry = 0x61,
	MCU_ACK = 0x71,
	MCU_NACK,      // 0x72
	MCU_ACK_PARA,  // 0x73
	MCU_CMD_CECMessage = 0xF7,
	MCU_CMD_ReportInfo = 0xF8,
} MCUTypeInfo;

typedef enum _tagMCUReportInfo {
	MCU_RPT_EDID_Ready = 0x01,
	MCU_RPT_HDMI_EDID_DATA,
	MCU_RPT_HDMI_PORT_NUMBER,
} MCUReportInfo;

#endif /*_USERPROTOCOL_H_*/
