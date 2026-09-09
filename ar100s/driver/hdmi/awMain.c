/****************************************************************************
* hdmirx/application/awMain.c
*
* Copyright (C) 2014-2016 AllWinnertech Ltd.
* Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/
#include <libfdt.h>
#include "include.h"
#include "hdmitx/hdmitx.h"

PACKETDATA PacketSend;
TCECMessage aw_cec_message;
uint8_t Buffer[EDID_DATA_PER_PACK + 1];

extern u32 dtb_base;
static uint32_t hdmirx_enable;
static uint32_t hdmitx_enable;
static u32 hdmi_init_from_dts(void)
{
	static u32 dts_has_parsed;
	void *fdt;
	int arisc_hdmi_node;

	if (!!dts_has_parsed)
		return 0;

	fdt = (void *)(dtb_base);

	/* parse arisc_hdmi tree */
	arisc_hdmi_node = fdt_path_offset(fdt, "arisc-hdmi");
	if (arisc_hdmi_node < 0)
		WRN("no arisc_hdmi: %x fdt:%x\n", arisc_hdmi_node, fdt);

	fdt_getprop_u32(fdt, arisc_hdmi_node, "hdmirx_enable", &hdmirx_enable);
	fdt_getprop_u32(fdt, arisc_hdmi_node, "hdmitx_enable", &hdmitx_enable);
	LOG("hdmirx_enable = %d hdmitx_enable = %d\n", hdmirx_enable, hdmitx_enable);

	dts_has_parsed = 1;

	return 0;
}

static uint8_t CheckSumCalc(uint8_t *rpData, uint8_t ucLength)
{
	uint8_t i, ucData;

	ucData = 0;
	for (i = 0; i < ucLength; i++)
		ucData += rpData[i];

	ucData = 0 - ucData;

	return ucData;
}

void awSendPacketToHost(RPPACKETDATA rpPacket, uint8_t ucLen)
{

	if (amp_msgbox_check_arm_is_die()) {
		/* MCU->HOST communication blocked */
		LOG("McuComm blocked\n");
		return;
	}
/*
	int i;
	LOG("OTT-->tv");
	for (i = 0 ; i < ucLen ; i++) {
		LOG("%x", rpPacket->Buffer[i]);
	}
	LOG("\n");
*/
	rpPacket->Buffer[0] = PACKET_START_BYTE;
	rpPacket->Buffer[ucLen - 2] = CheckSumCalc(&(rpPacket->Buffer[1]), ucLen - 6);  // Checksum
	rpPacket->Buffer[ucLen - 1] = PACKET_END_BYTE;

	rpm_send_to_arm(MESSAGE_TYPE_TO_ARM, ucLen, rpPacket->Buffer);
}

void hdmirx_cmd_handler(uint8_t MsgType, uint8_t length, uint8_t *pdata)
{
	int i;
	uint8_t MsgCode;

	switch (MsgType) {
	case HOST_TO_MCU_POWER_MODE: {
	} break;

	case HOST_TO_MCU_SET_PARAMETER: {
		memset(Buffer, 0, sizeof(Buffer));
		MsgCode = pdata[1];
		switch (MsgCode) {
		case HOST_PARAM_CMD_AW_HDMIPortRemap:
			/*
			* pdata[3]~pdata[5] contains HDMI map info,
			* see tdHdmiMapSet() for details.
			*/
			LOG("HDMI MAP %d,  %d,  %d\n", pdata[2], pdata[3], pdata[4]);
			tdHdmiMapSet(&pdata[2]);
			break;

		case HOST_PARAM_CMD_UpdateEDID: {
			for (i = 0; i < EDID_DATA_PER_PACK + 1; i++) {
				Buffer[i] = pdata[i + 2];  // first byte is packet number
			}
			awProcessHDMICommand(HDMI_CMD_UpdateEDID, Buffer);
		} break;

		case HOST_PARAM_CMD_HOTPLUG:
			Buffer[0] = pdata[2];  // Port number
			Buffer[1] = pdata[3];  //
			awProcessHDMICommand(HDMI_CMD_HOTPLUG, Buffer);
			break;

		case HOST_PARAM_CMD_EDIDVersion:
			Buffer[0] = pdata[2];  // Port mask
			awProcessHDMICommand(HDMI_CMD_EDIDVersion, Buffer);
			break;

		case HOST_PARAM_CMD_ResetEDIDModule:
			awProcessHDMICommand(HDMI_CMD_ResetEDIDModule, NULL);
			break;

		default:
			break;
		}
	} break;

	case HOST_TO_MCU_DEBUG_COMMAND: {
		MsgCode = pdata[1];
		switch (MsgCode) {
		case HOST_DBG_CMD_PrintEDIDTable:
			awProcessHDMICommand(HDMI_CMD_PrintEDIDTable, NULL);
		default:
			break;
		}
	} break;

	case HOST_TO_MCU_CEC_MESSAGE: {
		memset(&aw_cec_message, 0, sizeof(TCECMessage));
		aw_cec_message.ucOperandNum = pdata[1];   // exclude header and opcode
		aw_cec_message.Content.Buffer[0] = pdata[2];  // header
		aw_cec_message.Content.Buffer[1] = pdata[3];  // opcode
		//LOG("aw_cec_message.ucOperandNum =%x\n", aw_cec_message.ucOperandNum);

		for (i = 0; i < aw_cec_message.ucOperandNum - 2; i++) {
			aw_cec_message.Content.Buffer[2 + i] = pdata[4 + i];
		}
		/*
		LOG("Get CEC Message:  ");
		for (i = 0; i < (aw_cec_message.ucOperandNum); i++) {
			LOG(" 0x%x ", (aw_cec_message.Content.Buffer[i]));
		}
		LOG("\n");
		*/
		aw_cec_high_process_msg(&aw_cec_message);
	} break;

	case HOST_TO_MCU_CEC_SETTING: {
		LOG("Host Setting  %d, %d\n", pdata[1], pdata[3]);
		aw_cec_high_process_setting(pdata[1], pdata[3]);

	} break;

	case HOST_TO_MCU_REQUEST_INFORMATION: {
		MsgCode = pdata[1];
		switch (MsgCode) {
		case HOST_REQ_CMD_HDMIPortNumber:
		awProcessHDMICommand(HDMI_CMD_HDMIPortNumber, NULL);
		break;

		case HOST_REQ_CMD_EDIDStatus:
			awProcessHDMICommand(HDMI_CMD_EDIDStatus, NULL);
			break;

		case HOST_REQ_CMD_EDID_data:
			awProcessHDMICommand(HDMI_CMD_REQ_EDID_data, NULL);
			break;

		default:
			LOG("Unknown MsgCode %x\n", MsgCode);
			break;
		}
	} break;

	default:
		LOG("Unknown type\n");
		break;
	}
}

void hdmi_host_message_cb(uint8_t type, uint8_t length, uint8_t *pdata)
{
	uint8_t MsgType;

	MsgType = pdata[0];
	if (type != MESSAGE_TYPE_FROM_ARM) {
		ERR("Unknown ARM Message type (%x)\n", type);
		return;
	}

#ifdef CFG_HDMIRX_USED
	if (hdmirx_enable)
		hdmirx_cmd_handler(MsgType, length, pdata);
#endif

#ifdef CFG_HDMITX_USED
	if (hdmitx_enable)
		hdmitx_cmd_handler(length, pdata);
#endif

	return;
}

/*****************************************************************************
*  Name        : hdmi_main_loop
*  Description : main function
*  Params      :
*  Returns     :
*****************************************************************************/
void hdmi_main_loop(void)
{
#ifdef CFG_HDMIRX_USED
	if (hdmirx_enable)
		hdmirx_main_loop();
#endif

#ifdef CFG_HDMITX_USED
	if (hdmitx_enable)
		hdmitx_main_loop();
#endif
}

/*****************************************************************************
*  Name        : hdmi_init
*  Description : Initional function
*  Params      :
*  Returns     :
*****************************************************************************/
void hdmi_init(void)
{
	PRINT_MCU_VERSION();
	hdmi_init_from_dts();

	rpm_register_message_callback(RPM_CB_MSG_FROM_ARM, hdmi_host_message_cb);

#ifdef CFG_HDMIRX_USED
	if (hdmirx_enable)
		hdmirx_init_app();
#endif

#ifdef CFG_HDMITX_USED
	if (hdmitx_enable)
		hdmitx_init();
#endif
}
