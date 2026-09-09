/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : cec.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-3
* Descript: hdmi assist tx cec.
* Update  : date                auther      ver     notes
*           2025-11-5 15:44:00  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include "cec.h"

#if defined(CFG_SUN55IW7P1)
#define USE_CEC_PLAT	(1)
#define CEC_PRCM_BUS_REG	(0x07030160)
#define CEC_PRCM_CLK_REG	(0x07030164)
#define CEC_PRCM_32K_REG	(0x07030168)

#define CEC_SGPIO_REG		(0x0705808C)
#else
#define	USE_CEC_PLAT	(0)
#endif

u32 cec_readl(u32 offset)
{
	return readl(CECTOP_BASE_ADDR + offset);
}

void cec_writel(u32 offset, u32 val)
{
	writel(val, (CECTOP_BASE_ADDR + offset));
}

int hwcec_is_receive(void)
{
	u32 val = cec_readl(HWCEC_INT_STS);

	return (val & HWCEC_RCV_DONE_INT_STS_MASK) ? 1 : 0;
}

u8 hwcec_msg_receive(u8 *buf, u8 buf_size)
{
	u8 val = 0, i = 0;

	/* clear receive irq */
	cec_writel(HWCEC_INT_STS, HWCEC_RCV_DONE_INT_STS_MASK);

	/* check cec message is lock */
	if (!(cec_readl(HWCEC_LOCK) & HWCEC_LOCK_MASK)) {
		INF("hdmitx cec buffer unlock\n");
		return 0;
	}

	val = cec_readl(HWCEC_RECEIVE_LENGTH);
	if (val > buf_size)
		val = buf_size;
	if (val > CEC_MSG_MAX_SIZE)
		val = CEC_MSG_MAX_SIZE;

	for (i = 0; i < val; i++)
		*buf++ = (u8)cec_readl(HWCEC_RECEIVE_DATA_INDEX(i));

	/* clear cec lock status */
	cec_writel(HWCEC_LOCK, 0x0);
	return val;
}

u8 hwcec_msg_send(u8 *buf, u8 len)
{
	int i = 0;

	if (len > 16)
		return -1;

	cec_writel(HWCEC_SEND_LENGTH, len);

	for (i = 0; i < len; i++)
		cec_writel(HWCEC_SEND_DATA_INDEX(i), (u32)(*buf++));

	cec_writel(HWCEC_CTRL, 0x3);
	return len;
}

void hwcec_reg_dump(void)
{
	int i = 0;
	LOG("cec top:\n");
	for (i = 0; i < 0x20; i += 4)
		LOG("0x%x: 0x%x ", CECTOP_CONTROL + i, cec_readl(CECTOP_CONTROL + i));
	LOG("\n");

	LOG("cec hw:\n");
	for (i = 0; i <= 0x70; i += 4)
		LOG("0x%x: 0x%x ", HWCEC_INT_STS + i, cec_readl(HWCEC_INT_STS + i));
	LOG("\n");
}

void hwcec_init(void)
{
#if USE_CEC_PLAT
	u32 reg = readl(CEC_PRCM_BUS_REG);
	u32 mask = 0;

	if (reg == 0x00010000)
		return;

	writel(0x00010000, CEC_PRCM_BUS_REG);
	writel(0x82000000, CEC_PRCM_CLK_REG);
	writel(0x82100000, CEC_PRCM_32K_REG);
	/* config use cec gpio */
	writel(0x2, CEC_SGPIO_REG);

	/* cec top config */
	cec_writel(CECTOP_CONTROL, 0x304);

	/* hw cec config */
	cec_writel(HWCEC_INT_STS, 0xFF);
	cec_writel(HWCEC_CTRL, 0x00);
	cec_writel(HWCEC_LOCK, 0x00);

	cec_writel(HWCEC_ADDR_L, 0x10);
	cec_writel(HWCEC_ADDR_H, 0x00);

	mask |= HWCEC_RCV_ERR_INT_EN_MASK;
	mask |= HWCEC_SEND_ERR_INT_EN_MASK;
	mask |= HWCEC_ARBLST_INT_EN_MASK;
	mask |= HWCEC_NACK_INT_EN_MASK;
	mask |= HWCEC_RCV_DONE_INT_EN_MASK;
	mask |= HWCEC_SEND_DONE_INT_EN_MASK;
	cec_writel(HWCEC_INT_EN, mask);

	cec_writel(HWCEC_INT_MASK, HWCEC_WAKEUP_INT_MASK);
	LOG("hdmi tx hw cec init done\n");
#endif
}
