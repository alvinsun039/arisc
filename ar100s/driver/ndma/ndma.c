/*
 *********************************************************************************************************
 *												AR100 SYSTEM
 *									 AR100 Software System Develop Kits
 *												ndma  module
 *
 *									(c) Copyright 2012-2026, Guojun Ma China
 *											 All Rights Reserved
 *
 * File	: ndma.c
 * By	  : Guojun Ma
 * Version : v1.0
 * Date	: 2025-12-29
 * Descript: ndma controller public interfaces.
 * Update  : date				auther	  ver	 notes
 *		   2025-12-29 20:21:23  Guojun Ma 1.0	 Create this file.
 *********************************************************************************************************
 */

#include <string.h>
#include <stdint.h>
#include "ndma_i.h"

#define SUNXI_NDMA_MODULE_VERSION	"1.0.1"

#ifdef DMA_PKGNUM
#define DMA_PKG_NUM_MAX	0xFFFFFFFF
#endif

#ifdef DMA_START_CHAN
#define SUNXI_DMA_CHAN_START DMA_START_CHAN
#else
#define SUNXI_DMA_CHAN_START 0
#endif

#ifdef DMA_DEBUG
#define DMA_ERR(fmt, arg...) LOG("%s()%d " fmt, __func__, __LINE__, ##arg)
#else
#define DMA_ERR(fmt, arg...) do {} while (0)
#endif

static struct ndma_chan	dma_chan_source[NR_MAX_CHAN];

/* Static LLI pool for DMA descriptors */
static struct ndma_lli	 dma_lli_pool[NR_MAX_CHAN];
static uint8_t				 dma_lli_used[NR_MAX_CHAN];

/* Avoid repeated requests irq, which may led to unkown problem such as memory leak */
static uint32_t irq_inited;

static struct ndma_lli *lli_alloc(void)
{
	int i;
	for (i = 0; i < NR_MAX_CHAN; i++) {
		if (!dma_lli_used[i]) {
			dma_lli_used[i] = 1;
			return &dma_lli_pool[i];
		}
	}
	return NULL;
}

static void lli_free(struct ndma_lli *lli)
{
	int i;
	for (i = 0; i < NR_MAX_CHAN; i++) {
		if (&dma_lli_pool[i] == lli) {
			dma_lli_used[i] = 0;
			return;
		}
	}
}

static inline void ndma_irq_set(uint32_t chan, bool enable)
{
	if (!enable) {
		uint32_t high = 0;
			high = (chan >= HIGH_CHAN) ? 1 : 0;
			writel(0, DMA_IRQ_EN(high));
	}
}

static inline uint32_t ndma_irq_status(uint32_t chan_num, bool enable)
{
	uint32_t status = 0;
	if (enable) {
		uint32_t high = 0;
			high = (chan_num >= HIGH_CHAN) ? 1 : 0;
			writel(0xffffffff, DMA_IRQ_STAT(high));
	} else {
		uint32_t status_l = 0, status_h = 0;
#if START_CHAN_OFFSET < HIGH_CHAN
		status_l = readl(DMA_IRQ_STAT(0));
		writel(status_l, DMA_IRQ_STAT(0));
#endif
#if NR_MAX_CHAN + START_CHAN_OFFSET > HIGH_CHAN
		status_h = readl(DMA_IRQ_STAT(1));
		writel(status_h, DMA_IRQ_STAT(1));
#endif
		status = (chan_num + START_CHAN_OFFSET >= HIGH_CHAN) \
					 ? (status_h >> ((chan_num + START_CHAN_OFFSET - HIGH_CHAN) << DMA_IRQ_STAT_OFFSET)) \
					 : (status_l >> ((chan_num + START_CHAN_OFFSET) << DMA_IRQ_STAT_OFFSET));
		return status;
	}
	return 0;
}

static inline void ndma_cfg_lli(struct ndma_lli *lli, uint32_t src_addr,
							uint32_t dst_addr, uint32_t len,
							struct dma_slave_config *config, enum dma_transfer_direction dir)
{
	if (NULL == lli || NULL == config)
		return;

	if (convert_buswidth((uint32_t *)&config->src_addr_width))
		DMA_ERR("[dma] src addr width is over, use default val\n");

	if (convert_buswidth((uint32_t *)&config->dst_addr_width))
		DMA_ERR("[dma] dst addr width is over, use default val\n");

	lli->cfg = SRC_CONTI(config->conti_mode)	 | \
			   SRC_BURST(config->src_maxburst)   | \
			   SRC_WIDTH(config->src_addr_width) | \
			   DST_BURST(config->dst_maxburst)   | \
			   DST_WIDTH(config->dst_addr_width);

	lli->src = src_addr;
	lli->dst = dst_addr;
	lli->len = len;
	lli->para = NORMAL_WAIT;
}

static void ndma_dump_chan_regs(u32 chan_num)
{
#ifdef DMA_DEBUG
	LOG("Chan %d reg:\n"
		   "\tDMA_IRQ_EN(0x%x):   0x%x\n"
		   "\tDMA_IRQ_STAT(0x%x): 0x%x\n"
		   "\tDMA_GATE(0x%x):	 0x%x\n"
		   "\tDMA_GATE_LEN(0x%x): 0x%x\n"
		   "\tDMA_CFG(0x%x):	  0x%x\n"
		   "\tDMA_CUR_SRC(0x%x):  0x%x\n"
		   "\tDMA_CUR_DST(0x%x):  0x%x\n"
		   "\tDMA_CNT(0x%x):	  0x%x\n\n",
		   chan_num,
		   DMA_IRQ_EN(chan_num), (uint32_t)readl(DMA_IRQ_EN(chan_num)),
		   DMA_IRQ_STAT(chan_num), (uint32_t)readl(DMA_IRQ_STAT(chan_num)),
		   DMA_GATE, (uint32_t)readl(DMA_GATE),
		   DMA_GATE_LEN, (uint32_t)readl(DMA_GATE_LEN),
		   DMA_CFG(chan_num), (uint32_t)readl(DMA_CFG(chan_num)),
		   DMA_CUR_SRC(chan_num), (uint32_t)readl(DMA_CUR_SRC(chan_num)),
		   DMA_CUR_DST(chan_num), (uint32_t)readl(DMA_CUR_DST(chan_num)),
		   DMA_CNT(chan_num), (uint32_t)readl(DMA_CNT(chan_num)));
#endif
}

static void ndma_dump_lli(struct ndma_chan *chan, struct ndma_lli *lli)
{
#ifdef DMA_DEBUG
	LOG("channum:%x\n"
		   "\t\tdesc:desc - 0x%x desc p - 0x%x desc v - 0x%x\n"
		   "\t\tlli: v- 0x%x v_lln - 0x%x s - 0x%x d - 0x%x c - 0x%x\n"
		   "\t\tdist - 0x%x len - 0x%x para - 0x%x p_lln - 0x%x\n",
		   chan->chan_count,
	   (uint32_t)chan->desc, (uint32_t)chan->desc->p_lln, (uint32_t)chan->desc->vlln,
	   (uint32_t)lli, (uint32_t)lli->vlln, (uint32_t)lli->src, (uint32_t)lli->dst, (uint32_t)lli->cfg,
		   (uint32_t)lli->dst, (uint32_t)lli->len, (uint32_t)lli->para, (uint32_t)lli->p_lln);
#endif
}

static void *ndma_lli_list(struct ndma_lli *prev, struct ndma_lli *next,
						struct ndma_chan *chan)
{
	uint32_t temp_desc;

	if ((!prev && !chan) || !next)
		return NULL;

	temp_desc = (unsigned long)next;
	if (!prev) {
		chan->desc = next;
		chan->desc->p_lln = temp_desc;
		chan->desc->vlln = next;
	} else {
		prev->p_lln = temp_desc;
		prev->vlln = next;
	}

	next->p_lln = LINK_END;
	next->vlln = NULL;

	return next;
}

static s32 ndma_dma_irq_handle(void *parg)
{
	int i = 0;
	uint32_t status_l = 0, status_h = 0;

#if START_CHAN_OFFSET < HIGH_CHAN
	status_l = readl(DMA_IRQ_STAT(0));
	writel(status_l, DMA_IRQ_STAT(0));
#endif
#if NR_MAX_CHAN + START_CHAN_OFFSET > HIGH_CHAN
	status_h = readl(DMA_IRQ_STAT(1));
	writel(status_h, DMA_IRQ_STAT(1));
#endif

	for (i = SUNXI_DMA_CHAN_START; i < NR_MAX_CHAN; i++) {
		struct ndma_chan *chan = &dma_chan_source[i];
		uint32_t chan_num = chan->chan_count;
		uint32_t status = 0;

		if (chan->used == 0)
			continue;

		status = (chan_num + START_CHAN_OFFSET >= HIGH_CHAN) \
				 ? (status_h >> ((chan_num + START_CHAN_OFFSET - HIGH_CHAN) << DMA_IRQ_STAT_OFFSET)) \
				 : (status_l >> ((chan_num + START_CHAN_OFFSET) << DMA_IRQ_STAT_OFFSET));

		if (!(chan->irq_type & status))
			continue;

		dma_callback cb = NULL;
		void *cb_data = NULL;

		cb = chan->callback;
		cb_data = chan->callback_param;

		if (cb)
			cb(cb_data);
	}

	return 0;
}

static void ndma_dma_clk_init(void)
{
	/* enbale clock gating and set reset as de-assert state*/
	writel(SUNXI_CLK_NDMA_EN | SUNXI_RST_DMA, SUNXI_CLK_NDMA_REG);
}

static void ndma_dma_free_ill(struct ndma_chan *chan)
{
	struct ndma_lli *li_adr = NULL, *next = NULL;

	if (NULL == chan) {
		DMA_ERR("[dma] chan is NULL\n");
		return ;
	}

	li_adr = chan->desc;
	chan->desc = NULL;

	while (li_adr) {
		next = li_adr->vlln;
		lli_free(li_adr);
		li_adr = next;
	}
}

ndma_chan_status_t ndma_chan_request(struct ndma_chan **dma_chan)
{
	int i = 0;
	struct ndma_chan *chan;

	for (i = SUNXI_DMA_CHAN_START; i < NR_MAX_CHAN; i++) {
		chan = &dma_chan_source[i];
		if (chan->used == 0) {
			chan->used = 1;
			chan->chan_count = i;
			*dma_chan = &dma_chan_source[i];
			return NDMA_CHAN_STATUS_FREE;
		}
	}

	return NDMA_CHAN_STATUS_BUSY;
}

ndma_status_t ndma_set_wrap_mode(struct ndma_chan *chan, uint32_t start_wrap_addr,
		uint32_t end_wrap_addr, enum dma_wrapping_mode mode,  bool enable)
{
	if (NULL == chan)
		return NDMA_STATUS_INVALID_PARAMETER;

	if (enable) {
		if (mode == DMA_WRAPPING_SRC_EN) {
			chan->wrap_mode |= SRC_WRAP_MODE;
			writel(start_wrap_addr, DMA_SRC_WRAP_START_ADDR(chan->chan_count));
			writel(end_wrap_addr, DMA_SRC_WRAP_END_ADDR(chan->chan_count));
		} else if (mode == DMA_WRAPPING_DST_EN) {
			chan->wrap_mode |= DST_WRAP_MODE;
			writel(start_wrap_addr, DMA_DST_WRAP_START_ADDR(chan->chan_count));
			writel(end_wrap_addr, DMA_DST_WRAP_END_ADDR(chan->chan_count));
		}
	} else {
		if (mode == DMA_WRAPPING_SRC_EN) {
			chan->wrap_mode &= ~SRC_WRAP_MODE;
			writel(0, DMA_SRC_WRAP_START_ADDR(chan->chan_count));
			writel(0, DMA_SRC_WRAP_END_ADDR(chan->chan_count));
		} else if (mode == DMA_WRAPPING_DST_EN) {
			chan->wrap_mode &= ~DST_WRAP_MODE;
			writel(0, DMA_DST_WRAP_START_ADDR(chan->chan_count));
			writel(0, DMA_DST_WRAP_END_ADDR(chan->chan_count));
		}
	}

	return NDMA_STATUS_OK;
}

ndma_status_t ndma_prep_memcpy(struct ndma_chan *chan,
					   uint32_t dest, uint32_t src, uint32_t len)
{
	struct ndma_lli *l_item = NULL;
	struct dma_slave_config *config = NULL;

	if ((NULL == chan) || (dest == 0 || src == 0)) {
		DMA_ERR("[dma] chan is NULL\n");
		return NDMA_STATUS_INVALID_PARAMETER;
	}

	/* Free old descriptor before allocating new one */
	if (chan->desc)
		ndma_dma_free_ill(chan);

	l_item = lli_alloc();
	if (!l_item)
		return NDMA_STATUS_NO_MEM;
	memset(l_item, 0, sizeof(struct ndma_lli));

	config = &chan->cfg;
	ndma_cfg_lli(l_item, src, dest, len, config, DMA_MEM_TO_MEM);

	l_item->cfg |= SRC_DRQ(DRQSRC_SDRAM) \
				   | DST_DRQ(DRQDST_SDRAM) \
				   | DST_INCREMENT_MODE \
				   | SRC_INCREMENT_MODE;

	ndma_lli_list(NULL, l_item, chan);
	ndma_dump_lli(chan, l_item);

	return NDMA_STATUS_OK;
}

ndma_status_t ndma_prep_device(struct ndma_chan *chan,
					   uint32_t dest, uint32_t src,
					   uint32_t len, enum dma_transfer_direction dir)
{
	struct ndma_lli *l_item = NULL;
	struct dma_slave_config *config = NULL;

	if ((NULL == chan) || (dest == 0 || src == 0)) {
		DMA_ERR("[dma] chan is NULL\n");
		return NDMA_STATUS_INVALID_PARAMETER;
	}

	/* Free old descriptor before allocating new one */
	if (chan->desc)
		ndma_dma_free_ill(chan);

	l_item = lli_alloc();
	if (!l_item)
		return NDMA_STATUS_NO_MEM;
	memset(l_item, 0, sizeof(struct ndma_lli));

	config = &chan->cfg;
	if (dir == DMA_MEM_TO_DEV) {
		ndma_cfg_lli(l_item, src, dest, len, config, dir);
		l_item->cfg |= GET_DST_DRQ(config->slave_id) \
					   | SRC_INCREMENT_MODE \
					   | DST_NOCHANGE_MODE \
					   | SRC_DRQ(DRQSRC_SDRAM);
	} else if (dir == DMA_DEV_TO_MEM) {
		ndma_cfg_lli(l_item, src, dest, len, config, dir);
		l_item->cfg |= GET_SRC_DRQ(config->slave_id)  \
						| DST_INCREMENT_MODE \
						| SRC_NOCHANGE_MODE \
						| DST_DRQ(DRQSRC_SDRAM);
	} else if (dir == DMA_DEV_TO_DEV) {
		ndma_cfg_lli(l_item, src, dest, len, config, dir);
		l_item->cfg |= GET_SRC_DRQ(config->slave_id) \
					   | DST_NOCHANGE_MODE \
					   | SRC_NOCHANGE_MODE \
					   | GET_DST_DRQ(config->slave_id);
	}

	ndma_lli_list(NULL, l_item, chan);
	ndma_dump_lli(chan, l_item);

	return NDMA_STATUS_OK;
}

ndma_status_t ndma_callback_install(struct ndma_chan *chan,
					  dma_callback callback, void *callback_param)
{
	if (NULL == chan || NULL == callback || NULL == callback_param) {
		DMA_ERR("[dma] param is NULL\n");
		return NDMA_STATUS_INVALID_PARAMETER;
	}

	chan->callback = callback;
	chan->callback_param = callback_param;

	return NDMA_STATUS_OK;
}

ndma_status_t ndma_slave_config(struct ndma_chan *chan,
					  struct dma_slave_config *config)
{
	if (NULL == config || NULL == chan) {
		DMA_ERR("[dma] dma config is NULL\n");
		return NDMA_STATUS_INVALID_PARAMETER;
	}

	if (convert_burst(&config->src_maxburst))
		DMA_ERR("[dma] src bst len is over, use default val\n");
	if (convert_burst(&config->dst_maxburst))
		DMA_ERR("[dma] dst bst len is over, use default val\n");

	memcpy(&chan->cfg, config, sizeof(struct dma_slave_config));

	return NDMA_STATUS_OK;
}

enum ndma_status ndma_tx_status(struct ndma_chan *chan, uint32_t *left_size)
{
	uint32_t cfg_val = 0;

	*left_size = readl(DMA_CNT(chan->chan_count));
	cfg_val = readl(DMA_CFG(chan->chan_count));

	if (cfg_val & DMA_BUSY_STATUS)
		return DMA_IN_PROGRESS;

	return DMA_COMPLETE;
}

ndma_status_t ndma_start(struct ndma_chan *chan)
{
	uint32_t irq_val = 0;
	uint32_t high = 0;
	uint32_t desc_addr;
	uint32_t cfg_val = 0;

	if (NULL == chan) {
		DMA_ERR("[dma] chan is NULL\n");
		return NDMA_STATUS_INVALID_PARAMETER;
	}

	if (!chan->irq_type)
		chan->irq_type = IRQ_PKG;

	high = (chan->chan_count + START_CHAN_OFFSET >= HIGH_CHAN) ? 1 : 0;
	irq_val = readl(DMA_IRQ_EN(high));
	irq_val |= SHIFT_IRQ_MASK(chan->irq_type, chan->chan_count);
	writel(irq_val, DMA_IRQ_EN(high));

	desc_addr = (unsigned long)chan->desc;

	struct ndma_lli *chan_desc = (struct ndma_lli *)desc_addr;
	writel(chan_desc->src, DMA_CUR_SRC(chan->chan_count));
	writel(chan_desc->dst, DMA_CUR_DST(chan->chan_count));
	writel(chan_desc->len & NDMA_BYTE_COUNTER_REG_MASK, DMA_CNT(chan->chan_count));

	/* clear cfg reg */
	cfg_val = readl(DMA_CFG(chan->chan_count));
	cfg_val &= ~(DMA_WAIT(NDMA_WAIT_STATE_MASK)
			   | DST_WIDTH(NDMA_DEST_DATA_WIDTH_MASK)
			   | DST_BURST(NDMA_DEST_BST_LEN_MASK)
			   | DST_ADDR_MODE(NDMA_DEST_ADDR_TYPE_MASK)
			   | DST_DRQ(NDMA_DEST_DRQ_TYPE_MASK)
			   | SRC_WIDTH(NDMA_SRC_DATA_WIDTH_MASK)
			   | SRC_BURST(NDMA_SRC_BST_LEN_MASK)
			   | SRC_ADDR_MODE(NDMA_SRC_ADDR_TYPE_MASK)
			   | SRC_DRQ(NDMA_SRC_DRQ_TYPE_MASK)
			   | SRC_CONTI(NDMA_CONTI_MODE_MASK)
			   | SRC_WRAP_MODE
			   | DST_WRAP_MODE
			   );

	cfg_val |= chan_desc->cfg | chan_desc->para | REMAIN_MODE | chan->wrap_mode;
	writel(cfg_val, DMA_CFG(chan->chan_count));

	cfg_val = readl(DMA_CFG(chan->chan_count));
	cfg_val |= DMA_LOADING;
	writel(cfg_val, DMA_CFG(chan->chan_count));

	ndma_dump_chan_regs(chan->chan_count);

	return NDMA_STATUS_OK;
}

ndma_status_t ndma_stop(struct ndma_chan *chan)
{
	uint32_t cfg_val = 0;
	if (NULL == chan) {
		DMA_ERR("[dma] chan is NULL\n");
		return NDMA_STATUS_INVALID_PARAMETER;
	}

	cfg_val = readl(DMA_CFG(chan->chan_count));
	cfg_val &= ~DMA_LOADING;
	writel(cfg_val, DMA_CFG(chan->chan_count));

	return NDMA_STATUS_OK;
}

ndma_status_t ndma_chan_free(struct ndma_chan *chan)
{
	unsigned long irq_val = 0;
	uint32_t high = 0;

	if (NULL == chan || !chan->used) {
		DMA_ERR("[dma] chan is NULL or not used\n");
		return NDMA_STATUS_INVALID_PARAMETER;
	}

	high = (chan->chan_count + START_CHAN_OFFSET >= HIGH_CHAN) ? 1 : 0;

	irq_val = readl(DMA_IRQ_EN(high));
	irq_val &= ~(SHIFT_IRQ_MASK(chan->irq_type, chan->chan_count));
	writel(irq_val, DMA_IRQ_EN(high));;

	ndma_dma_free_ill(chan);

	chan->callback = NULL;
	chan->callback_param = NULL;
	chan->wrap_mode = 0;
	chan->used = 0;

	return NDMA_STATUS_OK;
}

ndma_status_t ndma_chan_desc_free(struct ndma_chan *chan)
{
	ndma_dma_free_ill(chan);

	return NDMA_STATUS_OK;
}

static void ndma_resource_get(void)
{
	uint32_t i = 0;
	struct ndma_chan *chan;
	uint32_t gate;

	/* Detect that if there is a master that has not freed DMA, it will not memset! */
	for (i = SUNXI_DMA_CHAN_START; i < NR_MAX_CHAN; i++) {
		chan = &dma_chan_source[i];
		if (chan->used == 0) {
			memset(chan, 0, sizeof(struct ndma_chan));
		}
	}

	for (i = START_CHAN_OFFSET + SUNXI_DMA_CHAN_START; i < START_CHAN_OFFSET + NR_MAX_CHAN; i++) {
		ndma_irq_set(i, 0);
		ndma_irq_status(i, 1);
	}

	/* disable auto gating */
	gate = readl(DMA_GATE);
	writel(gate | DMA_AUTO_CLK_GATE, DMA_GATE);

	ndma_dma_clk_init();

	/*request dma irq*/
	if (!(irq_inited & 0x1)) {
		if (install_isr(INTC_R_NDMA_IRQ, ndma_dma_irq_handle, NULL) != OK)
			DMA_ERR("[dma] request irq error\n");
		else
			irq_inited |= 0x1;
	}
	interrupt_enable(INTC_R_NDMA_IRQ);
}

/* only need to be executed once */
void ndma_init(void)
{
	ndma_resource_get();
}