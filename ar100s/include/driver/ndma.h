/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                ndma  module
*
*                                    (c) Copyright 2012-2026, Guojun Ma China
*                                             All Rights Reserved
*
* File    : ndma.h
* By      : Guojun Ma
* Version : v1.0
* Date    : 2025-12-29
* Descript: ndma controller public interfaces.
* Update  : date                auther      ver     notes
*           2025-12-29 20:21:23 Guojun Ma   1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __NDMA_H__
#define __NDMA_H__
#include "include.h"

#define IRQ_HALF		0x01			/* Half package transfer interrupt pending */
#define IRQ_PKG			0x02			/* One package complete interrupt pending */
#define IRQ_QUEUE		0x04			/* All list complete transfer interrupt pending */

#define DRQSRC_SDRAM			0
#define DRQSRC_LSPSRAM			1
/* #define DRQSRC_RESERVED		2 */
#define DRQSRC_S_I2S0_RX		3
#define DRQSRC_ADDA0_RX			4
#define DRQSRC_ADDA_LP			5
#define DRQSRC_S_UART0_RX		6
#define DRQSRC_S_UART1_RX		7
#define DRQSRC_ADDA1_RX			8
#define DRQSRC_S_TWI0_RX		9
#define DRQSRC_S_TWI1_RX		10
#define DRQSRC_S_TWI2_RX		11
#define DRQSRC_S_SPI_RX		 	12
#define DRQSRC_S_I2S1_RX		13

#define DRQDST_SDRAM			0
#define DRQDST_LSPSRAM			1
/* #define DRQDST_RESERVED		2 */
#define DRQDST_S_I2S0_TX		3
#define DRQDST_ADDA0_TX 		4
/* #define DRQDST_RESERVED		5 */
#define DRQDST_S_UART0_TX		6
#define DRQDST_S_UART1_TX		7
/* #define DRQDST_RESERVED		8 */
#define DRQDST_S_TWI0_TX		9
#define DRQDST_S_TWI1_TX		10
#define DRQDST_S_TWI2_TX		11
#define DRQDST_S_SPI_TX		 	12
/* #define DRQDST_RESERVED		13 */

#define sunxi_slave_id(d, s) 	(((d)<<15) | (s))

struct ndma_lli {
	uint32_t	cfg;
	uint32_t	src;
	uint32_t	dst;
	uint32_t	len;
	uint32_t	para;
	uint32_t	p_lln;
	struct ndma_lli *vlln;
};

typedef void (*dma_callback)(void *param);

/**
 * enum dma_slave_buswidth - defines bus width of the DMA slave
 * device, source or target buses
 */
enum dma_slave_buswidth {
	DMA_SLAVE_BUSWIDTH_UNDEFINED = 0,
	DMA_SLAVE_BUSWIDTH_1_BYTE = 1,
	DMA_SLAVE_BUSWIDTH_2_BYTES = 2,
	DMA_SLAVE_BUSWIDTH_4_BYTES = 4,
};

enum dma_slave_burst {
	DMA_SLAVE_BURST_1 = 1,
	DMA_SLAVE_BURST_4 = 4,
};

/**
 * enum dma_transfer_direction - dma transfer mode and direction indicator
 * @DMA_MEM_TO_MEM: Async/Memcpy mode
 * @DMA_MEM_TO_DEV: Slave mode & From Memory to Device
 * @DMA_DEV_TO_MEM: Slave mode & From Device to Memory
 * @DMA_DEV_TO_DEV: Slave mode & From Device to Device
 */
enum dma_transfer_direction {
	DMA_MEM_TO_MEM = 0,
	DMA_MEM_TO_DEV = 1,
	DMA_DEV_TO_MEM = 2,
	DMA_DEV_TO_DEV = 3,
	DMA_TRANS_NONE,
};

/**
 * enum dma_continue_mode - dma continue or no
 * @DMA_CONTI_DISABLE: dma continue mode disable
 * @DMA_CONTI_ENABLE: dma continue mode enable
 */
enum dma_continue_mode {
	DMA_CONTI_DISABLE = 0,
	DMA_CONTI_ENABLE = 1,
};

/**
 * enum ndma_status - DMA transaction status
 * @DMA_COMPLETE: transaction completed
 * @DMA_IN_PROGRESS: transaction not yet processed
 * @DMA_PAUSED: transaction is paused
 * @DMA_ERROR: transaction failed
 */
enum ndma_status {
	DMA_INVALID_PARAMETER = -2,
	DMA_ERROR = -1,
	DMA_COMPLETE,
	DMA_IN_PROGRESS,
	DMA_PAUSED,
};


/**
 * enum dma_wrapping_mode - defines wrapping mode for src and dst
 * @DMA_WRAPPING_SRC_EN: Enable src wrapping mode
 * @DMA_WRAPPING_DST_EN: Enable dst wrapping mode
 */
enum dma_wrapping_mode {
	DMA_WRAPPING_SRC_EN = 0,     /* Enable src wrapping mode */
	DMA_WRAPPING_DST_EN = 1,     /* Enable dst wrapping mode */
};

/**
 * struct dma_slave_config - dma slave channel runtime config
 * @direction: whether the data shall go in or out on this slave
 * channel, right now. DMA_MEM_TO_DEV and DMA_DEV_TO_MEM are
 * legal values. DEPRECATED, drivers should use the direction argument
 * to the device_prep_slave_sg and device_prep_dma_cyclic functions or
 * the dir field in the dma_interleaved_template structure.
 * @src_addr: this is the physical address where DMA slave data
 * should be read (RX), if the source is memory this argument is
 * ignored.
 * @dst_addr: this is the physical address where DMA slave data
 * should be written (TX), if the source is memory this argument
 * is ignored.
 * @src_addr_width: this is the width in bytes of the source (RX)
 * register where DMA data shall be read. If the source
 * is memory this may be ignored depending on architecture.
 * Legal values: 1, 2, 4, 8.
 * @dst_addr_width: same as src_addr_width but for destination
 * target (TX) mutatis mutandis.
 * @src_maxburst: the maximum number of words (note: words, as in
 * units of the src_addr_width member, not bytes) that can be sent
 * in one burst to the device. Typically something like half the
 * FIFO depth on I/O peripherals so you don't overflow it. This
 * may or may not be applicable on memory sources.
 * @dst_maxburst: same as src_maxburst but for destination target
 * mutatis mutandis.
 * @slave_id: Slave requester id. Only valid for slave channels. The dma
 * slave peripheral will have unique id as dma requester which need to be
 * pass as slave config.
 *
 * This struct is passed in as configuration data to a DMA engine
 * in order to set up a certain channel for DMA transport at runtime.
 * The DMA device/engine has to provide support for an additional
 * callback in the dma_device structure, device_config and this struct
 * will then be passed in as an argument to the function.
 *
 * The rationale for adding configuration information to this struct is as
 * follows: if it is likely that more than one DMA slave controllers in
 * the world will support the configuration option, then make it generic.
 * If not: if it is fixed so that it be sent in static from the platform
 * data, then prefer to do that.
 */
struct dma_slave_config {
	enum dma_transfer_direction direction;
	enum dma_continue_mode conti_mode;
	unsigned long src_addr;
	unsigned long dst_addr;
	enum dma_slave_buswidth src_addr_width;
	enum dma_slave_buswidth dst_addr_width;
	uint32_t src_maxburst;
	uint32_t dst_maxburst;
	uint32_t slave_id;
};

typedef void (*sunxi_dma_timeout_callback)(void *param);

struct ndma_desc {
	bool is_bmode;
	bool is_timeout;
	bool byte_per_pkg;
	unsigned timeout_steps;
	unsigned timeout_fun;
	sunxi_dma_timeout_callback callback;
	void *callback_param;
};

struct ndma_chan {
	uint8_t used:1;
	uint8_t chan_count:4;
	bool	cyclic:1;
	struct dma_slave_config cfg;
	uint32_t periods_pos;
	uint32_t buf_len;
	uint32_t new_val;
	uint32_t old_val;
	uint32_t val;
	struct ndma_lli *desc;
	uint32_t	irq_type;
	uint32_t	wrap_mode;
	dma_callback callback;
	void *callback_param;
	struct ndma_desc	*extend_desc;
};

/** This enum defines the DMA CHANNEL status. */
typedef enum {
	NDMA_CHAN_STATUS_BUSY  = 0, 		/* DMA channel status busy */
	NDMA_CHAN_STATUS_FREE = 1 		/* DMA channel status free */
} ndma_chan_status_t;

/* This enum defines the return type of GPIO API. */
typedef enum {
	NDMA_STATUS_INVALID_PARAMETER	= -22,		/* Invalid input parameter. */
	NDMA_STATUS_NO_MEM			= -12,		/* No memory. */
	NDMA_STATUS_ERR_PERM			= -1,		/* Operation not permitted. */
	NDMA_STATUS_OK			= 0		/* The DMA status ok. */
} ndma_status_t;


ndma_chan_status_t ndma_chan_request(struct ndma_chan **dma_chan);
ndma_status_t ndma_prep_memcpy(struct ndma_chan *chan,
				       uint32_t dest, uint32_t src, uint32_t len);
ndma_status_t ndma_prep_device(struct ndma_chan *chan,
				       uint32_t dest, uint32_t src,
				       uint32_t len, enum dma_transfer_direction dir);
ndma_status_t ndma_callback_install(struct ndma_chan *chan,
					  dma_callback callback,
					  void *callback_param);
ndma_status_t ndma_slave_config(struct ndma_chan *chan, struct dma_slave_config *config);
enum ndma_status ndma_tx_status(struct ndma_chan *chan, uint32_t *bytes);
ndma_status_t ndma_start(struct ndma_chan *chan);
ndma_status_t ndma_stop(struct ndma_chan *chan);
ndma_status_t ndma_chan_free(struct ndma_chan *chan);
ndma_status_t ndma_chan_desc_free(struct ndma_chan *chan);
ndma_status_t ndma_set_wrap_mode(struct ndma_chan *chan, uint32_t start_wrap_addr,
		uint32_t end_wrap_addr, enum dma_wrapping_mode mode,  bool enable);
#ifdef CFG_NDMA_USED
void ndma_init(void);
#else
static inline void ndma_init(void) { return; }
#endif
#endif /* __NDMA_H__ */
