/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : hdmitx.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-3
* Descript: hdmi assist tx cec.
* Update  : date                auther      ver     notes
*           2025-11-5 15:44:00  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include "cec/cec.h"

struct hdmi_tx_s {
	bool init_done;
	bool fakeoff;
	bool cec_enable;
	bool cec_return;
	u32 cec_index;
	u32 cec_irq;
	CECMsgQueue cec_queue;
};

extern uint32_t volatile wakeup_source;
struct hdmi_tx_s hdmi;

#define INTC_R_HDMI_ASSIST_TX_IRQ  42
#if defined(CFG_SUN55IW7P1)
#define SUPPORT_RTC_FLAG	(1)
#define RTC_BASE 		(0x07200000)
#define RTC_GP_DATA(x)	(RTC_BASE + 0x100 + (x * 4))
#define RTC_GP_INDEX	(9)
#else
#define SUPPORT_RTC_FLAG	(0)
#endif


void _init_queue(CECMsgQueue *q)
{
	q->pHead = q->pPush = q->pPop = q->queue;
	memset(q->queue, 0, sizeof(q->queue));
}

bool _queue_empty(CECMsgQueue *q)
{
	return q->pPush == q->pPop;
}

bool _queue_full(CECMsgQueue *q)
{
	return (q->pPush + 1 == q->pPop) ||
			(q->pPush + 1 == q->queue + QUEUE_SIZE && q->pPop == q->queue);
}

bool _enqueue(CECMsgQueue *q, TCECMessage msg)
{
	if (_queue_full(q)) {
		WRN("Queue is full!\n");
		return FALSE;
	}

	*(q->pPush) = msg;
	q->pPush++;

	if (q->pPush == q->queue + QUEUE_SIZE) {
		q->pPush = q->queue;
	}

	return TRUE;
}

bool _dequeue(CECMsgQueue *q, TCECMessage *msg)
{
	if (_queue_empty(q)) {
		//WRN("Queue is empty!\n");
		return FALSE;
	}
	*msg = *(q->pPop);
	q->pPop++;

	if (q->pPop == q->queue + QUEUE_SIZE) {
		q->pPop = q->queue;
	}
	return TRUE;
}

int _hdmitx_cec_msg_print(u8 *buff, u8 len)
{
	int i = 0;

	LOG("tx cec msg:");
	for (i = 0; i < len; i++)
		LOG(" 0x%x", buff[i]);
	LOG("\n");
	return 0;
}

int _hdmitx_cmd_cec_on(void)
{
	_init_queue(&hdmi.cec_queue);
	hdmi.cec_enable = 1;
	printk("hdmi tx cec cmd set on\n");
	return 0;
}

int _hdmitx_cmd_cec_off(void)
{
	hdmi.cec_enable = 0;
	printk("hdmi tx cec cmd set off\n");
	return 0;
}

int _hdmitx_cmd_cec_return(void)
{
	bool ret = FALSE;
	TCECMessage cec_msg;
	int i = 0;

	while (1) {
		memset(&cec_msg, 0x0, sizeof(TCECMessage));
		ret = _dequeue(&hdmi.cec_queue, &cec_msg);
		if (ret == FALSE)
			return 0;

		if (cec_msg.ucOperandNum > 0) {
			LOG("cec[%d]:", cec_msg.index);
			for (i = 0; i < cec_msg.ucOperandNum; i++)
				LOG(" 0x%02x", cec_msg.Content.Buffer[i]);
			LOG("\n");
			/* TODO, maybe add rpm_send_to_arm */
		}
	}
	return 0;
}

/*
 * CEC wakeup opcode table (OTT as Playback Device 0x4):
 *   Based on HDMI CEC Specification + TV compatibility extensions
 *
 *   Match modes:
 *   - MATCH_MODE_SINGLE: single opcode, match buf[1] only
 *   - MATCH_MODE_DUAL: dual-opcode cross-message, first msg matches op1,
 *                     wait for second msg matching op2
 *   - MATCH_MODE_OPCODE_OPERAND: opcode+operand in same message,
 *                               match buf[1]=op1 AND buf[2]=op2
 *
 *   Standard CEC messages (Section 13.2/13.13):
 *   - 0x86 <Set Stream Path>: TV requests device output
 *   - 0x85 <Request Active Source>: TV queries active source
 *   - 0x44 <User Control Pressed> + operand: Remote control commands
 *   - 0x41 <Play>: User pressed play button
 *
 *   TV compatibility (non-standard combinations):
 *   - 0x87 + 0x82: Some TVs send Routing Change then Active Source
 *   - 0x8F + 0x8C: Some TVs query power status sequence
 */
enum cec_wakeup_match_mode {
	MATCH_MODE_SINGLE,         /* single opcode */
	MATCH_MODE_DUAL,           /* dual-opcode cross-message */
	MATCH_MODE_OPCODE_OPERAND, /* opcode + operand in same message */
};

struct cec_wakeup_entry {
	u8 op1;
	u8 op2;
	enum cec_wakeup_match_mode mode;
};

static const struct cec_wakeup_entry wakeup_table[] = {
	/* dual-opcode cross-message: TV compatibility for non-standard TVs */
	{0x87, 0x82, MATCH_MODE_DUAL}, /* Routing Change + Active Source */
	{0x8F, 0x8C, MATCH_MODE_DUAL}, /* Give Power Status + Report Power Status */

	/* opcode + operand: <User Control Pressed> [UI Command] */
	{0x44, 0x6D, MATCH_MODE_OPCODE_OPERAND}, /* [Power On] */
	{0x44, 0x6B, MATCH_MODE_OPCODE_OPERAND}, /* [Power Toggle] */

	/* single opcode: standard CEC wakeup messages */
	{0x86, 0x00, MATCH_MODE_SINGLE}, /* <Set Stream Path> - most important */
	{0x85, 0x00, MATCH_MODE_SINGLE}, /* <Request Active Source> */
	{0x41, 0x00, MATCH_MODE_SINGLE}, /* <Play> - optional */
};

static void trigger_cec_wakeup(u8 match_idx)
{
#if SUPPORT_RTC_FLAG
	u32 val = 0xFF400000;

	val |= wakeup_table[match_idx].op1 << 8;
	val |= wakeup_table[match_idx].op2 << 0;
	writel(val, RTC_GP_DATA(RTC_GP_INDEX));
#endif
	wakeup_source = OneTouchPlay_Wakeup;
	INF("hdmi tx cec wakeup: op1=0x%x op2=0x%x mode=%d\n",
	    wakeup_table[match_idx].op1, wakeup_table[match_idx].op2,
	    wakeup_table[match_idx].mode);
}

int _hdmitx_cec_wakeup_check(u8 *buf)
{
	static u8 matched, match_idx;
	u8 opcode = buf[1];
	u8 operand = buf[2];
	int i;

	/* Handle second message of dual-opcode cross-message mode */
	if (matched) {
		matched = 0;
		if (wakeup_table[match_idx].op2 == opcode) {
			trigger_cec_wakeup(match_idx);
			return 1;
		}
	}

	/* Scan wakeup table for first message match */
	for (i = 0; i < ARRAY_SIZE(wakeup_table); i++) {
		if (wakeup_table[i].op1 != opcode)
			continue;

		switch (wakeup_table[i].mode) {
		case MATCH_MODE_SINGLE:
			trigger_cec_wakeup(i);
			return 1;

		case MATCH_MODE_OPCODE_OPERAND:
			if (wakeup_table[i].op2 == operand) {
				trigger_cec_wakeup(i);
				return 1;
			}
			break;

		case MATCH_MODE_DUAL:
			matched = 1;
			match_idx = i;
			return 0;
		}
	}

	return 0;
}

int _hdmitx_cec_message_handler(u8 *buf, u8 len)
{
	TCECMessage cec_msg;

	memcpy(cec_msg.Content.Buffer, buf, len);
	cec_msg.ucOperandNum = len;
	cec_msg.index = hdmi.cec_index;

	hdmi.cec_index++;
	if (hdmi.cec_index > 100)
		hdmi.cec_index = 0;

	_enqueue(&hdmi.cec_queue, cec_msg);
	return 0;
}

void _hdmitx_loop(void)
{
	u8 buf[16], ret;

	if (hdmi.fakeoff) {
		hwcec_init();
	} else {
		if (!hdmi.cec_enable) {
			goto exit;
		}
	}

	if (hwcec_is_receive() == 0)
		goto exit;

	ret = hwcec_msg_receive(buf, ARRAY_SIZE(buf));
	if (ret <= 0)
		goto exit;

	_hdmitx_cec_msg_print(buf, ret);

	if (_hdmitx_cec_wakeup_check(buf))
		goto exit;

	_hdmitx_cec_message_handler(buf, ret);
exit:
	return;
}

void hdmitx_standby_loop(void)
{
	if (hdmi.init_done)
		_hdmitx_loop();
}

void hdmitx_main_loop(void)
{
	if (hdmi.init_done)
		_hdmitx_loop();
}

int hdmitx_cmd_handler(uint8_t length, uint8_t *pdata)
{
	uint8_t cmd = pdata[0];

	switch (cmd) {
	case HOST_TO_MCU_SET_CEC_ON:
		_hdmitx_cmd_cec_on();
		break;
	case HOST_TO_MCU_SET_CEC_OFF:
		_hdmitx_cmd_cec_off();
		break;
	case HOST_TO_MCU_GET_CEC_BUFF:
		_hdmitx_cmd_cec_return();
		break;
	default:
		WRN("hdmi tx unsupport cmd: %d\n", cmd);
		break;
	}
	return 0;
}

void hdmitx_update_type(unsigned char type)
{
	if (type == FAKE_POWER_OFF_REQ)
		hdmi.fakeoff = 1;
	else
		hdmi.fakeoff = 0;
}

int hdmitx_init(void)
{
	memset(&hdmi, 0x0, sizeof(struct hdmi_tx_s));
	hdmi.init_done = 1;
	return 0;
}
