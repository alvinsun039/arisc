/****************************************************************************
 * hdmirx/cec/awCECLow.c
 *
 * Copyright (C) 2022-2024 AllWinnertech Ltd.
 * Author: huangwenhao <huangwenhao@allwinnertech.com>
 ***************************************************************************/

#include "include.h"

uint8_t bCECInt = _FALSE_;

void HDMIRx_WriteRegMaskU32(uintptr_t addr, u32 mask, u32 data)
{
	u32 temp = 0;

	temp = readl(addr);
	temp &= ~(mask);
	temp |= (mask & data);
	writel(temp, addr);
}

/**hardware cec soft & enable
* bit2--0
* bit4--0
* bit5--1
* bit8--1
* bit9--1
*/
void aw_cec_low_assist_ctrl(void)
{
	HDMIRx_WriteRegMaskU32(HDMIRX_ASSIST_CTR_REG, 0x334, 0x320);
}

/**
 * clear locked status
 * @param dev: address and device information
 * @return LOCKED status
 */
static int aw_cec_low_clear_lock(void)
{
	writel(readl(HDMIRX_CEC_LOCK) & ~CEC_LOCK_LOCKED_BUFFER_MASK, HDMIRX_CEC_LOCK);
	return 0;
}

/**
 * Enable interrupts
 * @param dev:  address and device information
 * @param mask: interrupt mask to enable
 * @return error code
 */
static void aw_cec_low_interrupt_enable(unsigned char mask)
{
	writel(readl(HDMIRX_IH_ENABLE_CEC_STAT0) | mask, HDMIRX_IH_ENABLE_CEC_STAT0);
}

/**
 * Disable interrupts
 * @param dev:  address and device information
 * @param mask: interrupt mask to disable
 * @return error code
 */
static void aw_cec_low_interrupt_disable(unsigned char mask)
{
	writel(HDMIRX_IH_ENABLE_CEC_STAT0, readl(HDMIRX_IH_ENABLE_CEC_STAT0) & ~mask);
}

/**
 * Clear interrupts
 * @param dev:  address and device information
 * @param mask: interrupt mask to clear
 * @return error code
 */
/*static void _dw_cec_interrupt_clear(dw_hdmi_dev_t *dev, unsigned char mask)
{
	dev_write(dev, CEC_MASK, mask);
}*/

/**
 * Read interrupts
 * @param dev:  address and device information
 * @param mask: interrupt mask to read
 * @return INT content
 */
int aw_cec_low_interrupt_get_state(void)
{
	return readl(HDMIRX_IH_CEC_STAT0);
}

/**
 * Clear interrupts state
 * @param dev:  address and device information
 * @param mask: interrupt mask to clear
 * @return INT content
 */
int aw_cec_low_interrupt_clear_state(unsigned state)
{
	/* write 1 to clear */
	writel(state, HDMIRX_IH_CEC_STAT0);
	return 0;
}

/**
 * Set cec logical address
 * @param dev:  address and device information
 * @param addr: logical address
 * @return INT content
 * @return error code or bytes configured
 */
int aw_cec_low_set_logical_addr(void)
{
	writel(CEC_ADDR_L_CEC_ADDR_L_0_MASK, HDMIRX_CEC_ADDR_L);
	writel(0, HDMIRX_CEC_ADDR_H);
	return 0;
}

/**
 * Get cec logical address
 * @param dev:  address and device information
 * @return error code or bytes configured
 */
int aw_cec_low_get_logical_addr(void)
{
	return ((readl(HDMIRX_CEC_ADDR_H) << 8) | readl(HDMIRX_CEC_ADDR_L));
}

/**
 * Write transmission buffer
 * @param dev:  address and device information
 * @param buf:  data to transmit
 * @param size: data length [byte]
 * @return error code or bytes configured
 */
int aw_cec_low_send_frame(unsigned char *buf, unsigned size, unsigned frame_type)
{
	unsigned i;
	unsigned char data;

	writel(size, HDMIRX_CEC_TX_CNT); /*0x220*/
	for (i = 0; i < size; i++)
		writel(*buf++, HDMIRX_CEC_TX_DATA + (i * 4));  /*230*/

	data = readl(HDMIRX_CEC_CTRL) & (~(CEC_CTRL_FRAME_TYP_MASK));
	data |= frame_type | CEC_CTRL_SEND_MASK;
	writel(data, HDMIRX_CEC_CTRL);  /*210*/

	return 0;
}

/**
 * Read reception buffer
 * @param dev:  address and device information
 * @param buf:  buffer to hold receive data
 * @return size: reception data length [byte]
 */
int aw_cec_low_receive_frame(unsigned char *buf, unsigned *size)
{
	unsigned i;
	unsigned char cnt;

	cnt = readl(HDMIRX_CEC_RX_CNT);   /* mask 7-5? */   /*0x224*/
	cnt = (cnt > CEC_RX_DATA_SIZE) ? CEC_RX_DATA_SIZE : cnt;

	for (i = 0; i < cnt; i++) {
		*buf++ = readl(HDMIRX_CEC_RX_DATA + (i * 4));  /*0x270*/
		//LOG("readl(CEC_RX_DATA + (%d * 4)) = %d\n", i, readl(CEC_RX_DATA + (i * 4)));
	}
	aw_cec_low_clear_lock(); /*0x228*/

	*size = cnt;
	return 0;
}

/**
 * Open a CEC controller
 * @warning Execute before start using a CEC controller
 * @param dev:  address and device information
 * @return error code
 */
void aw_cec_low_enable(void)
{
	unsigned char mask = 0x0;

	/* clear ctrl */
	writel(0, HDMIRX_CEC_CTRL);   /*0x210*/

	/* clear lock */
	writel(0, HDMIRX_CEC_LOCK);   /*0x228*/

	/* set cec logic address */
	aw_cec_low_set_logical_addr(); /*0x218  0x21c*/

	/* TODO */
	//writel(0xFF, CEC_WAKEUPCTRL);  /* disable wakeup */  /*0x22c*/

	/* clear all interrupt state */
	aw_cec_low_interrupt_clear_state(~0);  /*0x200*/

	/* enable irq */
	mask |= IH_ENABLE_CEC_STAT0_WAKEUP_MASK;
	mask |= IH_ENABLE_CEC_STAT0_DONE_MASK;
	mask |= IH_ENABLE_CEC_STAT0_EOM_MASK;
	mask |= IH_ENABLE_CEC_STAT0_NACK_MASK;
	mask |= IH_ENABLE_CEC_STAT0_ARB_LOST_MASK;
	mask |= IH_ENABLE_CEC_STAT0_ERROR_INITIATOR_MASK;
	mask |= IH_ENABLE_CEC_STAT0_ERROR_FOLLOW_MASK;
	aw_cec_low_interrupt_enable(mask);  /*0x204 enable*/

}

/**
 * Close a CEC controller
 * @warning Execute before stop using a CEC controller
 * @param dev:    address and device information
 * @return error code
 */
int aw_cec_low_disable(void)
{
	unsigned char mask = 0x0;

	/* disable interrupt*/
	mask |= IH_ENABLE_CEC_STAT0_WAKEUP_MASK;
	mask |= IH_ENABLE_CEC_STAT0_DONE_MASK;
	mask |= IH_ENABLE_CEC_STAT0_EOM_MASK;
	mask |= IH_ENABLE_CEC_STAT0_NACK_MASK;
	mask |= IH_ENABLE_CEC_STAT0_ARB_LOST_MASK;
	mask |= IH_ENABLE_CEC_STAT0_ERROR_INITIATOR_MASK;
	mask |= IH_ENABLE_CEC_STAT0_ERROR_FOLLOW_MASK;
	aw_cec_low_interrupt_disable(mask);

	/* TODO: support wakeup */
	/* _dw_cec_interrupt_clear(CEC_MASK_WAKEUP_MASK); */
	/* _dw_cec_interrupt_enable(CEC_MASK_WAKEUP_MASK); */
	/* _dw_cec_set_standby_mode(1); */

	return 0;
}

