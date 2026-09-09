/****************************************************************************
 * hdmirx/cec/awCECHigh.c
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

#include "include.h"

TCECSendBackACK MessageSendback;
CECMsgQueue g_SendMsgQueue;
CECMsgQueue g_RecMsgQueue;
CECMsgQueue g_SendMsgAckQueue;
TCECMessage OTP_ActiveSourceMsg;
TCECMessage OTP_ImageViewOnMsg;

uint8_t OTPFlag;
uint8_t CecWakeFlag;
uint8_t OTPBufferTime;
uint32_t RTC_Buffer;
uint32_t RTC_Value;
bool bCECEnabled;
bool bCECWakeupEnabled;
bool tx_done;
static int index;
static uint8_t one_touch_play_addressing_timer;
extern uint32_t volatile wakeup_source;

void initCECQueue(CECMsgQueue *cecQueue)
{
	cecQueue->pHead = cecQueue->pPush = cecQueue->pPop = cecQueue->queue;
	memset(cecQueue->queue, 0, sizeof(cecQueue->queue));
}

bool isQueueEmpty(CECMsgQueue *cecQueue)
{
	return cecQueue->pPush == cecQueue->pPop;
}

bool isQueueFull(CECMsgQueue *cecQueue)
{
	return (cecQueue->pPush + 1 == cecQueue->pPop) ||
			(cecQueue->pPush + 1 == cecQueue->queue + QUEUE_SIZE && cecQueue->pPop == cecQueue->queue);
}

bool enqueue(CECMsgQueue *cecQueue, TCECMessage msg)
{
	if (isQueueFull(cecQueue)) {
		WRN("Queue is full!\n");
		return FALSE;
	}
	*(cecQueue->pPush) = msg;
	cecQueue->pPush++;

	if (cecQueue->pPush == cecQueue->queue + QUEUE_SIZE) {
		cecQueue->pPush = cecQueue->queue;
	}
	return TRUE;
}

bool dequeue(CECMsgQueue *cecQueue, TCECMessage *msg)
{
	if (isQueueEmpty(cecQueue)) {
		//WRN("Queue is empty!\n");
		return FALSE;
	}
	*msg = *(cecQueue->pPop);
	cecQueue->pPop++;

	if (cecQueue->pPop == cecQueue->queue + QUEUE_SIZE) {
		cecQueue->pPop = cecQueue->queue;
	}
	return TRUE;
}

void tdEnableCECInterrupt(uint8_t bEnable)
{
	if (bEnable) {
		interrupt_enable(INTC_R_HDMI_ASSIST_RX_IRQ);
	} else {
		interrupt_disable(INTC_R_HDMI_ASSIST_RX_IRQ);
	}
}

void tdSendCECPacket(uint8_t ucMsgType, uint8_t *rpData, uint8_t ucMSgLength)
{
	int i = 0;

	if (bCECEnabled == _FALSE_) {
		LOG("CEC is disabled\n");
		return;
	}

	if (rpData == NULL) {
		ERR("rpData is null, skip\n");
		return;
	}
	/*
	LOG("\n***Send cmd To ARM: ");
	while (i < ucMSgLength) {
		LOG("%x ", rpData[i]);
		i++;
	}
	LOG("***\n");
	*/
	if ((ucMsgType != MSG_CMD_RES_BYPASS) && (ucMsgType != MSG_CMD_RES_ALL_DEVICE_LA) && (ucMsgType != MSG_CMD_RES_SENDBACK)) {
		ERR("Wrong Message Type!!!\n");
		return;
	}

	/*  ====msg layout of transmit command===
		rpData[0]              dst_la+src_la
		rpData[1]              opcode
		rpData[2]              Operand
						{msg content}
		rpData[ucOperandNum]   {msg content}
	*/

	// PacketSend.Packet.Msg.STB = PACKET_START_BYTE;
	memset(&PacketSend, 0, sizeof(PACKETDATA));
	PacketSend.Packet.Msg.Type = MCU_CMD_CECMessage;
	PacketSend.Packet.Msg.Len = ucMSgLength;
	PacketSend.Packet.Msg.SubType = ucMsgType;

	for (i = 0; i < PacketSend.Packet.Msg.Len; i++) {
		PacketSend.Packet.Msg.Data[i] = rpData[i];
	}

	SendCECtoHost(&PacketSend);
}

void tdCEC_OTPMessageInit(void)
{
	memset(&OTP_ImageViewOnMsg, 0, sizeof(TCECMessage));
	memset(&OTP_ActiveSourceMsg, 0, sizeof(TCECMessage));

	OTP_ImageViewOnMsg.Content.Msg.ucOpcode = OPCODE_IMAGE_VIEW_ON;
	OTP_ImageViewOnMsg.ucOperandNum = 0x2;

	OTP_ActiveSourceMsg.Content.Msg.ucOpcode = OPCODE_ACTIVE_SOURCE;
	OTP_ActiveSourceMsg.Content.Msg.ucOperands[0] = 0xFF;
	OTP_ActiveSourceMsg.Content.Msg.ucOperands[1] = 0xFF;
	OTP_ActiveSourceMsg.ucOperandNum = 0x4;
}

void tdCEC_SendOTPActiveSourceMsg(void)
{
	tdSendCECPacket(MSG_CMD_RES_BYPASS, OTP_ActiveSourceMsg.Content.Buffer, OTP_ActiveSourceMsg.ucOperandNum);
}

void tdCEC_SendOTPImageViewOnMsg(void)
{
	tdSendCECPacket(MSG_CMD_RES_BYPASS, OTP_ImageViewOnMsg.Content.Buffer, OTP_ImageViewOnMsg.ucOperandNum);
}

void tdCEC_EnableCec(bool enable)
{
	bCECEnabled = enable;
}

void tdCEC_EnableCecWakeup(bool enable)
{
	bCECWakeupEnabled = enable;
}

void tdCEC_SetOTPFlag(uint8_t status)
{
	OTPFlag = status;
}

uint8_t tdCEC_GetOTPFlag(void)
{
	return OTPFlag;
}

uint8_t tdCEC_GetCecWakeFlag(void)
{
	return CecWakeFlag;
}

void tdCEC_OTPProc(void)
{
	if (((current_time_tick()) % 50) == 0) {
		if (CecWakeFlag && tdCEC_GetOTPFlag() \
				&& (!amp_msgbox_check_arm_is_die())) {
			OTPBufferTime++;
			time_mdelay(1);

			if (OTPBufferTime > 4) {
				tdCEC_SendOTPImageViewOnMsg();
				tdCEC_SendOTPActiveSourceMsg();
				time_mdelay(10);

				tdCEC_SendOTPActiveSourceMsg();
				tdCEC_SendOTPImageViewOnMsg();
				tdCEC_SetOTPFlag(0);
				OTPBufferTime = 0;
				CecWakeFlag = 0;
			}
		}
	}
}

static void tdCECWakeup(void)
{
	one_touch_play_addressing_timer = TIMER_INACTIVE;
	wakeup_source = OneTouchPlay_Wakeup;
	CecWakeFlag = 1;
}

void tdCECUpdateTimer(void)
{
	if (one_touch_play_addressing_timer > TIMER_EXPIRED) {
		one_touch_play_addressing_timer--;
	}
}

void tdCEC_Standby(void)
{
	TCECMessage cec_msg;
	memset(&cec_msg, 0, sizeof(TCECMessage));

	if (bCECEnabled && bCECWakeupEnabled) {
		if (dequeue(&g_RecMsgQueue, &cec_msg)) {
			if (((cec_msg.Content.Msg.ucHeader & 0xF0) != 0) && (((cec_msg.Content.Msg.ucHeader & 0x0F) == TV_LA_Address) ||
															((cec_msg.Content.Msg.ucHeader & 0x0F) == BC_LA_Address))) {
				switch (cec_msg.Content.Msg.ucOpcode) {
				case OPCODE_TEXT_VIEW_ON:
					OTP_ImageViewOnMsg.Content.Msg.ucHeader = cec_msg.Content.Msg.ucHeader;
					OTP_ImageViewOnMsg.Content.Msg.ucOpcode = OPCODE_IMAGE_VIEW_ON;
					OTP_ImageViewOnMsg.ucOperandNum = cec_msg.ucOperandNum;
					// request PA from wakeup device
					cec_msg.Content.Msg.ucHeader =
						(cec_msg.Content.Msg.ucHeader & 0x0F) << 4 | (cec_msg.Content.Msg.ucHeader & 0xF0) >> 4;
					cec_msg.Content.Msg.ucOpcode = OPCODE_GIVE_PHYSICAL_ADDRESS;
					cec_msg.ucOperandNum = 0x2;
					enqueue(&g_SendMsgQueue, cec_msg);
					one_touch_play_addressing_timer = ONE_TOUCH_PLAY_ADDRESSING_TIME;
					break;

				case OPCODE_IMAGE_VIEW_ON:
					OTP_ImageViewOnMsg.Content.Msg.ucHeader = cec_msg.Content.Msg.ucHeader;
					OTP_ImageViewOnMsg.Content.Msg.ucOpcode = OPCODE_IMAGE_VIEW_ON;
					OTP_ImageViewOnMsg.ucOperandNum = cec_msg.ucOperandNum;
					// request PA from wakeup device
					cec_msg.Content.Msg.ucHeader =
						(cec_msg.Content.Msg.ucHeader & 0x0F) << 4 | (cec_msg.Content.Msg.ucHeader & 0xF0) >> 4;
					cec_msg.Content.Msg.ucOpcode = OPCODE_GIVE_PHYSICAL_ADDRESS;
					cec_msg.ucOperandNum = 0x2;
					enqueue(&g_SendMsgQueue, cec_msg);
					one_touch_play_addressing_timer = ONE_TOUCH_PLAY_ADDRESSING_TIME;
					break;

				case OPCODE_ACTIVE_SOURCE:
					OTP_ImageViewOnMsg.Content.Msg.ucHeader = cec_msg.Content.Msg.ucHeader & 0xF0;
					OTP_ActiveSourceMsg.Content.Msg.ucHeader = cec_msg.Content.Msg.ucHeader;
					OTP_ActiveSourceMsg.Content.Msg.ucOpcode = cec_msg.Content.Msg.ucOpcode;
					OTP_ActiveSourceMsg.Content.Msg.ucOperands[0] = cec_msg.Content.Msg.ucOperands[0];
					OTP_ActiveSourceMsg.Content.Msg.ucOperands[1] = cec_msg.Content.Msg.ucOperands[1];
					OTP_ActiveSourceMsg.ucOperandNum = cec_msg.ucOperandNum;
					tdCECWakeup();
					break;

				case OPCODE_REPORT_PHYSICAL_ADDRESS:
					if ((cec_msg.Content.Msg.ucHeader & 0xF0) == 0xF0)  // not response if initiator is unregisted
						break;

					if ((OTP_ImageViewOnMsg.Content.Msg.ucHeader & 0xF0) == (cec_msg.Content.Msg.ucHeader & 0xF0)) {
						/* physical address comes from the same device */
						OTP_ActiveSourceMsg.Content.Msg.ucOperands[0] = cec_msg.Content.Msg.ucOperands[0];
						OTP_ActiveSourceMsg.Content.Msg.ucOperands[1] = cec_msg.Content.Msg.ucOperands[1];
					}
					break;

				case OPCODE_GIVE_DEVICE_POWER_STATUS:
					if ((cec_msg.Content.Msg.ucHeader & 0x0F) != BC_LA_Address) {
						cec_msg.Content.Msg.ucHeader =
							(cec_msg.Content.Msg.ucHeader & 0x0F) << 4 | (cec_msg.Content.Msg.ucHeader & 0xF0) >> 4;
						cec_msg.Content.Msg.ucOpcode = OPCODE_REPORT_POWER_STATUS;
						cec_msg.Content.Msg.ucOperands[0] = POWER_IS_STANDBY;
						cec_msg.ucOperandNum = 0x3;
						enqueue(&g_SendMsgQueue, cec_msg);
					}
					break;

				default:
					// LOG("Ignore CEC 0x%x 0x%x\n", pOTPMsg->Content.Msg.ucHeader, pOTPMsg->Content.Msg.ucOpcode);
					break;
				}
			}
		} else {
			if (dequeue(&g_SendMsgQueue, &cec_msg)) {
				aw_cec_high_Send(cec_msg.Content.Buffer, cec_msg.ucOperandNum, CEC_CTRL_FRAME_TYP_IMMED);
			}
		}
		if (one_touch_play_addressing_timer == TIMER_EXPIRED) {
			tdCECWakeup();
		}
	}
}

void SysClearWakeupData(void)
{
	RTC_Value = 0;
	writel(RTC_Value, RTC_POWER_OFF_RECORD1_REG);
}

void SysBackupWakeupData(void)
{
	SysClearWakeupData();
	RTC_Value = RTCSettingValid;

	/* Backup to RTC_REG_1 */
	RTC_Buffer = (bCECEnabled ? 1 : 0);
	RTC_Value |= RTC_Buffer << CECEnable_bit;

	RTC_Buffer = (bCECWakeupEnabled ? 1 : 0);
	RTC_Value |= RTC_Buffer << CECWakeupEnable_bit;

	writel(RTC_Value, RTC_POWER_OFF_RECORD1_REG);

	LOG("Backup CEC setting\n");
}

void SysRestoreWakeupData(void)
{
	RTC_Value = readl(RTC_POWER_OFF_RECORD1_REG);
	RTC_Buffer = (RTC_Value & RTCSettingValid);

	if (RTC_Buffer == RTCSettingValid) {
		RTC_Buffer = RTC_Value;
		bCECEnabled = (((RTC_Buffer >> CECEnable_bit) & 0x01) ? 1 : 0);

		RTC_Buffer = RTC_Value;
		bCECWakeupEnabled = (((RTC_Buffer >> CECWakeupEnable_bit) & 0x01) ? 1 : 0);
		LOG("CEC Enable = %x\n", bCECEnabled);
		LOG("CEC Wakeup Enable = %x\n", bCECWakeupEnabled);
	} else {
		LOG("RTC REG is blank\n");
	}
}

void aw_cec_high_process_setting(uint8_t Type, uint8_t rpPacket)
{
	switch (Type) {
	case HOST_CEC_CMD_EnableCEC: {
		bCECEnabled = rpPacket & 0x01;
		LOG("bCECEnabled=%d\n", bCECEnabled);
	} break;

	case HOST_CEC_CMD_WakeupEnable:
		bCECWakeupEnabled = rpPacket & 0x01;
		LOG("bCECWakeupEnabled=%d\n", bCECWakeupEnabled);
		break;

	default:
		LOG("Unknown CEC Setting, Type = %x\n", Type);
		break;
	}
}

void aw_cec_high_process_msg(TCECMessage *rpPacket)
{
	TCECMessage cecOutMsg;
	cecOutMsg.ucOperandNum = rpPacket->ucOperandNum;

	u8 i;
	LOG("***CecCMD: ");
	for (i = 0; i < (rpPacket->ucOperandNum); i++) {
		cecOutMsg.Content.Buffer[i] = rpPacket->Content.Buffer[i];
		LOG("%x ", (cecOutMsg.Content.Buffer[i]));
	}
	LOG("\n");

	/*  ====msg layout of transmit command===
		rpPacket->Data[0]              dst_la+src_la
		rpPacket->Data[1]              opcode
		rpPacket->Data[2]              Operand
					{msg content}
		rpPacket->Data[ucOperandNum+2]   {msg content}
	*/

	enqueue(&g_SendMsgQueue, cecOutMsg);

}

int aw_cec_high_enable(void)
{
	/* 1.set CEC_CLK_GATING on
	   2.CLK_SRC_SEL to 24M
	   3.set SET CEC_CLK_GATING */
	writel(0x82100000, HDMIRX_32K_CLOCK);
	aw_cec_low_assist_ctrl();
	writel(0x0000007f, HDMIRX_IH_ENABLE_CEC_STAT0);
	udelay(200);
	aw_cec_low_enable();
	return 0;
}

int aw_cec_high_disable(void)
{
	aw_cec_low_disable();
	udelay(200);

	return 0;
}

sint32_t aw_cec_high_Receive(unsigned char *msg, unsigned *size)
{
	return aw_cec_low_receive_frame(msg, size);
}

sint32_t aw_cec_high_Send(unsigned char *msg, unsigned size, unsigned frame_type)
{
	uint32_t dw_frame_type = 0;

	switch (frame_type) {
	case AW_HDMI_CEC_FRAME_TYPE_RETRY:
		dw_frame_type = CEC_CTRL_FRAME_TYP_RETRY;
		break;
	case AW_HDMI_CEC_FRAME_TYPE_NORMAL:
		dw_frame_type = CEC_CTRL_FRAME_TYP_NORMAL;
		break;
	case AW_HDMI_CEC_FRAME_TYPE_IMMED:
		dw_frame_type = CEC_CTRL_FRAME_TYP_IMMED;
		break;
	}
	return aw_cec_low_send_frame(msg, size, dw_frame_type);
}

int aw_cec_high_Set_Logical_Addr(void)
{
	return aw_cec_low_set_logical_addr();
}

int aw_cec_high_interrupt_get_state(void)
{
	uint32_t dw_state = aw_cec_low_interrupt_get_state();
	uint32_t state = 0;

	if (dw_state & IH_CEC_STAT0_WAKEUP_MASK)
		state |= AW_HDMI_CEC_STAT_WAKEUP;
	if (dw_state & IH_CEC_STAT0_DONE_MASK)
		state |= AW_HDMI_CEC_STAT_DONE;
	if (dw_state & IH_CEC_STAT0_EOM_MASK)
		state |= AW_HDMI_CEC_STAT_EOM;
	if (dw_state & IH_CEC_STAT0_NACK_MASK)
		state |= AW_HDMI_CEC_STAT_NACK;
	if (dw_state & IH_CEC_STAT0_ARB_LOST_MASK)
		state |= AW_HDMI_CEC_STAT_ARBLOST;
	if (dw_state & IH_CEC_STAT0_ERROR_INITIATOR_MASK)
		state |= AW_HDMI_CEC_STAT_ERROR_INIT;
	if (dw_state & IH_CEC_STAT0_ERROR_FOLLOW_MASK)
		state |= AW_HDMI_CEC_STAT_ERROR_FOLL;

	return state;
}

void aw_cec_high_interrupt_clear_state(unsigned state)
{
	uint32_t dw_state = 0;

	if (state & AW_HDMI_CEC_STAT_WAKEUP)
		dw_state |= IH_CEC_STAT0_WAKEUP_MASK;
	if (state & AW_HDMI_CEC_STAT_DONE)
		dw_state |= IH_CEC_STAT0_DONE_MASK;
	if (state & AW_HDMI_CEC_STAT_EOM)
		dw_state |= IH_CEC_STAT0_EOM_MASK;
	if (state & AW_HDMI_CEC_STAT_NACK)
		dw_state |= IH_CEC_STAT0_NACK_MASK;
	if (state & AW_HDMI_CEC_STAT_ARBLOST)
		dw_state |= IH_CEC_STAT0_ARB_LOST_MASK;
	if (state & AW_HDMI_CEC_STAT_ERROR_INIT)
		dw_state |= IH_CEC_STAT0_ERROR_INITIATOR_MASK;
	if (state & AW_HDMI_CEC_STAT_ERROR_FOLL)
		dw_state |= IH_CEC_STAT0_ERROR_FOLLOW_MASK;

	aw_cec_low_interrupt_clear_state(dw_state);
}

s32 tdHDMIInterrupt(void *parg)
{
	//LOG("tdHDMIInterrupt\n");
	TCECMessage cec_msg;
	unsigned int len;
	memset(&cec_msg, 0, sizeof(TCECMessage));

	unsigned int stat = aw_cec_high_interrupt_get_state();
	//LOG("stat =0x%x\n", stat);

	aw_cec_high_interrupt_clear_state(stat);

	if (stat & AW_HDMI_CEC_STAT_ERROR_INIT) {
		tx_done = TRUE;
		MessageSendback.Content.Msg.ucACK = 3;   /* FAIL */
	} else if (stat & AW_HDMI_CEC_STAT_DONE) {
		tx_done = TRUE;
		MessageSendback.Content.Msg.ucACK = 0;   /* SUCCESS */
	} else if (stat & AW_HDMI_CEC_STAT_NACK) {
		tx_done = TRUE;
		MessageSendback.Content.Msg.ucACK = 1;   /* NACK */
	} else if (stat & AW_HDMI_CEC_STAT_ARBLOST) {
		tx_done = TRUE;
		MessageSendback.Content.Msg.ucACK = 2;   /* BUSY */
	} else if (stat & AW_HDMI_CEC_STAT_ERROR_FOLL) {
		tx_done = TRUE;
		MessageSendback.Content.Msg.ucACK = 3;   /* FAIL */
	}

	if (stat & AW_HDMI_CEC_STAT_EOM) {
		aw_cec_high_Receive(cec_msg.Content.Buffer, &len);
		cec_msg.ucOperandNum = len;

		if (index < 100)
			index++;
		else
			index = 0;
		cec_msg.index = index;
		enqueue(&g_RecMsgQueue, cec_msg);
	}

	return TRUE;
}

void aw_cec_high_init(void)
{
	uint32_t *parg = NULL;
	//uint32_t value = 0;
	tx_done  = _FALSE_;
	bCECEnabled = _FALSE_;
	bCECWakeupEnabled = _FALSE_;
	one_touch_play_addressing_timer = TIMER_INACTIVE;
	initCECQueue(&g_SendMsgQueue);
	initCECQueue(&g_RecMsgQueue);
	initCECQueue(&g_SendMsgAckQueue);
	aw_cec_high_enable();
	install_isr(INTC_R_HDMI_ASSIST_RX_IRQ, tdHDMIInterrupt, (void *)parg);

	/* set edid irq priority to highest level */
	/*
	value = readl(R_INTC_PRIO2_REG);
	value |= (0x3 << 16);
	writel(value, R_INTC_PRIO2_REG);
	*/

	/* enable edid irq */
	tdEnableCECInterrupt(_TRUE_);
}

void aw_cec_high_loop(void)
{
	TCECMessage cec_msg ;
	memset(&cec_msg, 0, sizeof(TCECMessage));
	tdCEC_OTPProc();
	if (tx_done == TRUE) {
		tdSendCECPacket(MSG_CMD_RES_SENDBACK, MessageSendback.Content.Buffer, 3);
		tx_done = FALSE;
	}

	if (dequeue(&g_RecMsgQueue, &cec_msg)) {
		LOG("OTT->TV %d cec header=0x%x ", cec_msg.index, cec_msg.Content.Msg.ucHeader);
		LOG("opcode=0x%x ", cec_msg.Content.Msg.ucOpcode);
		if (cec_msg.ucOperandNum > 0) {
			uint8_t j;
			LOG("data: ");
			for (j = 0; j < cec_msg.ucOperandNum - 2; j++)
				LOG("%x ", cec_msg.Content.Msg.ucOperands[j]);
		}
		LOG("\n");
		tdSendCECPacket(MSG_CMD_RES_BYPASS, cec_msg.Content.Buffer, cec_msg.ucOperandNum);
		return;
	}

	if (dequeue(&g_SendMsgQueue, &cec_msg)) {
		LOG("TV->OTT cec header=0x%x ", cec_msg.Content.Msg.ucHeader);
		LOG("opcode=0x%x ", cec_msg.Content.Msg.ucOpcode);
		if (cec_msg.ucOperandNum > 0) {
			uint8_t j;
			LOG("data: ");
			for (j = 0; j < cec_msg.ucOperandNum - 2; j++)
				LOG("%x ", cec_msg.Content.Msg.ucOperands[j]);
		}
		LOG("\n");
		MessageSendback.Content.Msg.ucHeader = cec_msg.Content.Msg.ucHeader;
		MessageSendback.Content.Msg.ucOpcode = cec_msg.Content.Msg.ucOpcode;
		aw_cec_high_Send(cec_msg.Content.Buffer, cec_msg.ucOperandNum, CEC_CTRL_FRAME_TYP_IMMED);
	}
}
