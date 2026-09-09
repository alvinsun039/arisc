#ifndef __SUNXI_GPADC_H__
#define __SUNXI_GPADC_H__
#include "include.h"

#define GPADC_REG_BASE		(R_GPADC_BASE)
/* GPADC register offset */
/* Sample Rate config register */
#define GP_SR_REG			(GPADC_REG_BASE + 0x00)

/* control register */
#define GP_CTRL_REG			(GPADC_REG_BASE + 0x04)

/* compare and select enable register */
#define GP_CS_EN_REG			(GPADC_REG_BASE + 0x08)

/* FIFO interrupt config register */
#define GP_FIFO_INTC_REG		(GPADC_REG_BASE + 0x0c)

/* FIFO interrupt status register */
#define GP_FIFO_INTS_REG		(GPADC_REG_BASE + 0x10)

/* FIFO data register */
#define GP_FIFO_DATA_REG		(GPADC_REG_BASE + 0x14)

/* calibration data register */
#define GP_CB_DATA_REG			(GPADC_REG_BASE + 0x18)

#define GP_DATAL_INTC_REG		(GPADC_REG_BASE + 0x20)
#define GP_DATAH_INTC_REG		(GPADC_REG_BASE + 0x24)
#define GP_DATA_INTC_REG		(GPADC_REG_BASE + 0x28)
#define GP_DATAL_INTS_REG		(GPADC_REG_BASE + 0x30)
#define GP_DATAH_INTS_REG		(GPADC_REG_BASE + 0x34)
#define GP_DATA_INTS_REG		(GPADC_REG_BASE + 0x38)

/* channel N compare data register */
#define GP_CH0_CMP_DATA_REG		(GPADC_REG_BASE + 0x40)
#define GP_CH1_CMP_DATA_REG		(GPADC_REG_BASE + 0x44)
#define GP_CH2_CMP_DATA_REG		(GPADC_REG_BASE + 0x48)
#define GP_CH3_CMP_DATA_REG		(GPADC_REG_BASE + 0x4c)
#define GP_CH4_CMP_DATA_REG		(GPADC_REG_BASE + 0x50)
#define GP_CH5_CMP_DATA_REG		(GPADC_REG_BASE + 0x54)
#define GP_CH6_CMP_DATA_REG		(GPADC_REG_BASE + 0x58)
#define GP_CH7_CMP_DATA_REG		(GPADC_REG_BASE + 0x5c)
#define GP_CH8_CMP_DATA_REG		(GPADC_REG_BASE + 0x60)
#define GP_CH9_CMP_DATA_REG		(GPADC_REG_BASE + 0x64)
#define GP_CHA_CMP_DATA_REG		(GPADC_REG_BASE + 0x68)
#define GP_CHB_CMP_DATA_REG		(GPADC_REG_BASE + 0x6c)
#define GP_CHC_CMP_DATA_REG		(GPADC_REG_BASE + 0x70)
#define GP_CHD_CMP_DATA_REG		(GPADC_REG_BASE + 0x74)
#define GP_CHE_CMP_DATA_REG		(GPADC_REG_BASE + 0x78)
#define GP_CHF_CMP_DATA_REG		(GPADC_REG_BASE + 0x7c)

#define GP_BOOTSTRAP_EN_REG		(GPADC_REG_BASE + 0x160)

#define THS_EN_REG				(GPADC_REG_BASE + 0x04)

/* channel N data register */
#define GP_CH0_DATA_REG			(GPADC_REG_BASE + 0x80)
#define GP_CH1_DATA_REG			(GPADC_REG_BASE + 0x84)
#define GP_CH2_DATA_REG			(GPADC_REG_BASE + 0x88)
#define GP_CH3_DATA_REG			(GPADC_REG_BASE + 0x8c)
#define GP_CH4_DATA_REG			(GPADC_REG_BASE + 0x90)
#define GP_CH5_DATA_REG			(GPADC_REG_BASE + 0x94)
#define GP_CH6_DATA_REG			(GPADC_REG_BASE + 0x98)
#define GP_CH7_DATA_REG			(GPADC_REG_BASE + 0x9c)
#define GP_CH8_DATA_REG			(GPADC_REG_BASE + 0xa0)
#define GP_CH9_DATA_REG			(GPADC_REG_BASE + 0xa4)
#define GP_CHA_DATA_REG			(GPADC_REG_BASE + 0xa8)
#define GP_CHB_DATA_REG			(GPADC_REG_BASE + 0xac)
#define GP_CHC_DATA_REG			(GPADC_REG_BASE + 0xb0)
#define GP_CHD_DATA_REG			(GPADC_REG_BASE + 0xb4)
#define GP_CHE_DATA_REG			(GPADC_REG_BASE + 0xb8)
#define GP_CHF_DATA_REG			(GPADC_REG_BASE + 0xc0)

#define GPADC0_DATA_PENDING		(1 << 0)	/* gpadc0 has data */
#define DEFAULT_SR			(400000UL)
#define DEFAULT_CLK_IN			(24000000UL)
#define DEFAULT_TACQ_LIMIT		(0xffff)
#define GP_SR_DIV_MASK			(0xffffffff)
#define GP_SR_DIV_SHIFT			16

#endif
