#ifndef __NDMA_SUN252IW3P1_H__
#define __NDMA_SUN252IW3P1_H__

#define HIGH_CHAN				8
#define LINK_END				0xFFFFF800			/* lastest link must be 0xfffff800 */
#define NR_MAX_CHAN				8					/* total of channels */
#define START_CHAN_OFFSET	 	0
#define SUNXI_CLK_NDMA_REG	 	(0x07030178)
#define SUNXI_CLK_NDMA_EN	 	(0x1 << 0)
#define SUNXI_RST_DMA		 	(0x1 << 16)

#define SUNXI_DMAC_PBASE		(0x07206000)

#define DMA_IRQ_EN(x)		(SUNXI_DMAC_PBASE + 0x00 + x * 0)		/* Interrupt enable register */
#define DMA_IRQ_STAT(x)		(SUNXI_DMAC_PBASE + 0x04 + x * 0)		/* Interrupt status register */
#define DMA_IRQ_STAT_OFFSET 1
#define DMA_GATE			(SUNXI_DMAC_PBASE + 0x08)				/* DMA gating register */
#define DMA_AUTO_CLK_GATE   (0x1 << 16)
#define DMA_GATE_LEN		(SUNXI_DMAC_PBASE + 0x0c)				/* DMA gating length register */
#define DMA_CFG(x)			(SUNXI_DMAC_PBASE + (0x100 + ((x + START_CHAN_OFFSET) << 5)))	/* Configuration register RO */
#define DMA_CUR_SRC(x)		(SUNXI_DMAC_PBASE + (0x104 + ((x + START_CHAN_OFFSET) << 5)))	/* Current source address RO */
#define DMA_CUR_DST(x)		(SUNXI_DMAC_PBASE + (0x108 + ((x + START_CHAN_OFFSET) << 5)))	/* Current destination address RO */
#define DMA_CNT(x)			(SUNXI_DMAC_PBASE + (0x10c + ((x + START_CHAN_OFFSET) << 5)))	/* Byte counter left register RO */
#define DMA_SRC_WRAP_END_ADDR(x)	(SUNXI_DMAC_PBASE + (0x110 + ((x + START_CHAN_OFFSET) << 5)))	/* Source wrap end address register */
#define DMA_SRC_WRAP_START_ADDR(x)	(SUNXI_DMAC_PBASE + (0x114 + ((x + START_CHAN_OFFSET) << 5)))	/* Source wrap start address register */
#define DMA_DST_WRAP_END_ADDR(x)	(SUNXI_DMAC_PBASE + (0x118 + ((x + START_CHAN_OFFSET) << 5)))	/* Destination wrap end address register */
#define DMA_DST_WRAP_START_ADDR(x)	(SUNXI_DMAC_PBASE + (0x11c + ((x + START_CHAN_OFFSET) << 5)))	/* Destination wrap start address register */

#define SHIFT_IRQ_MASK(val, ch) ({	\
		(ch + START_CHAN_OFFSET) >= HIGH_CHAN	\
		? (val) << ((ch + START_CHAN_OFFSET - HIGH_CHAN) << 1) \
		: (val) << ((ch + START_CHAN_OFFSET) << 1);	\
		})

#define E902_TO_E907_MEM_SHIFT		0x40000000
#define NDMA_LOADING_SHIFT			31
#define NDMA_BUSY_STATUS_SHIFT		30
#define NDMA_CONTI_MODE_SHIFT		29
#define NDMA_CONTI_MODE_MASK		0x01
#define NDMA_WAIT_STATE_SHIFT		26
#define REMAIN_MODE_SHIFT			14
#define NDMA_WAIT_STATE_MASK		0x07
#define NDMA_DEST_DATA_WIDTH_SHIFT	24
#define NDMA_DEST_DATA_WIDTH_MASK	0x03
#define NDMA_DEST_BST_LEN_SHIFT		23
#define NDMA_DEST_BST_LEN_MASK		0x01
#define NDMA_DEST_ADDR_TYPE_SHIFT	21
#define NDMA_DEST_ADDR_TYPE_MASK	0x03
#define NDMA_DEST_DRQ_TYPE_SHIFT	15
#define NDMA_DEST_DRQ_TYPE_MASK		0x3f
#define NDMA_SRC_DATA_WIDTH_SHIFT	9
#define NDMA_SRC_DATA_WIDTH_MASK	0x03
#define NDMA_SRC_BST_LEN_SHIFT		8
#define NDMA_SRC_BST_LEN_MASK		0x01
#define NDMA_SRC_ADDR_TYPE_SHIFT	6
#define NDMA_SRC_ADDR_TYPE_MASK		0x03
#define NDMA_SRC_DRQ_TYPE_SHIFT		0
#define NDMA_SRC_DRQ_TYPE_MASK		0x3f
#define DST_SRQ_MASK				NDMA_DEST_DRQ_TYPE_MASK
#define SRC_SRQ_MASK				NDMA_SRC_DRQ_TYPE_MASK
#define NDMA_CFG_MASK(mask, shift)	(mask << shift)
#define NDMA_CFG_SET(val, shift)	(val << shift)

#define NDMA_BYTE_COUNTER_REG_MASK	 0x3ffff

#define DMA_LOADING			((1) << NDMA_LOADING_SHIFT)
#define DMA_BUSY_STATUS		((1) << NDMA_BUSY_STATUS_SHIFT)
#define SRC_CONTI(x)		((x) << NDMA_CONTI_MODE_SHIFT)
#define SRC_WIDTH(x)		((x) << NDMA_SRC_DATA_WIDTH_SHIFT)
#define SRC_BURST(x)		((x) << NDMA_SRC_BST_LEN_SHIFT)
#define SRC_ADDR_MODE(x)	((x) << NDMA_SRC_ADDR_TYPE_SHIFT)
#define SRC_NOCHANGE_MODE	(0x01 << NDMA_SRC_ADDR_TYPE_SHIFT)
#define SRC_INCREMENT_MODE  (0x00 << NDMA_SRC_ADDR_TYPE_SHIFT)
#define SRC_DRQ(x)			((x) << NDMA_SRC_DRQ_TYPE_SHIFT)
#define DST_WIDTH(x)		((x) << NDMA_DEST_DATA_WIDTH_SHIFT)
#define DST_BURST(x)		((x) << NDMA_DEST_BST_LEN_SHIFT)
#define DST_ADDR_MODE(x)	((x) << NDMA_DEST_ADDR_TYPE_SHIFT)
#define DST_NOCHANGE_MODE	(0x01 << NDMA_DEST_ADDR_TYPE_SHIFT)
#define DST_INCREMENT_MODE	(0x00 << NDMA_DEST_ADDR_TYPE_SHIFT)
#define DST_DRQ(x)			((x) << NDMA_DEST_DRQ_TYPE_SHIFT)
#define DMA_WAIT(x)			((x) << NDMA_WAIT_STATE_SHIFT)
#define NORMAL_WAIT			(0x03 << NDMA_WAIT_STATE_SHIFT)
#define REMAIN_MODE			(0x1 << REMAIN_MODE_SHIFT)
#define SRC_WRAP_MODE		(0x1 << 12)
#define DST_WRAP_MODE		(0x1 << 11)

#define GET_SRC_DRQ(x)		((x) & (NDMA_SRC_DRQ_TYPE_MASK << NDMA_SRC_DRQ_TYPE_SHIFT))
#define GET_DST_DRQ(x)		((x) & (NDMA_DEST_DRQ_TYPE_MASK << NDMA_DEST_DRQ_TYPE_SHIFT))

static inline int32_t convert_buswidth(uint32_t *addr_width)
{
	if (*addr_width > 4)
		return -1;

	switch (*addr_width) {
	case 2:
		*addr_width = 1;
		break;
	case 4:
		*addr_width = 2;
		break;
	default:
		/* For 1 byte width or fallback */
		*addr_width = 0;
		break;
	}

	return 0;
}

static inline int32_t convert_burst(uint32_t *maxburst)
{
	if (*maxburst > 4)
		return -1;

	switch (*maxburst) {
	case 1:
		*maxburst = 0;
		break;
	case 4:
		*maxburst = 1;
		break;
	default:
		*maxburst = 0;
		break;
	}

	return 0;
}

#endif /*__NDMA_SUN252IW3P1_H__ */
