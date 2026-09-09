/*
 * drivers/ir/ir-nec-decoder.c
 * [ take care ] no need care repeat key in ar100s
 *
 * Copyright (C) 2022-2026 AllWinnertech Ltd.
 * Author: qinguangzhi <qinguangzhi@allwinnertech.com>
 *
 */

#include "ir_i.h"

#define NEC_NBITS		(32)
#define NEC_UNIT		(562500)  /* ns */
#define NEC_HEADER_PULSE	(16 * NEC_UNIT)
#define NECX_HEADER_PULSE	(8  * NEC_UNIT) /* Less common NEC variant */
#define NEC_HEADER_SPACE	(8  * NEC_UNIT)
#define NEC_REPEAT_SPACE	(4  * NEC_UNIT)
#define NEC_BIT_PULSE		(1  * NEC_UNIT)
#define NEC_BIT_0_SPACE		(1  * NEC_UNIT)
#define NEC_BIT_1_SPACE		(3  * NEC_UNIT)
#define	NEC_TRAILER_PULSE	(1  * NEC_UNIT)
#define	NEC_TRAILER_SPACE	(10 * NEC_UNIT) /* even longer in reality */
#define NECX_REPEAT_BITS	(1)


enum nec_state {
	STATE_INACTIVE,
	STATE_HEADER_SPACE,
	STATE_BIT_PULSE,
	STATE_BIT_SPACE,
	STATE_TRAILER_PULSE,
	STATE_TRAILER_SPACE,
};

struct nec_dec {
	int state;
	u32 count;
	u32 bits;
};

const u8 byte_rev_table[256] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

static inline u8 bitrev8(u8 byte)
{
	return byte_rev_table[byte];
}

u32 ir_nec_decode(ir_raw_buffer_t *ir_buffer)
{
	u32 scancode = 0;
	u32 i = 0;
	u32 duration;
	u32 pulse;
	u8 address, not_address, command, not_command;
	int send_32bits = 0;
	struct nec_dec data;

	data.state = 0;
	data.count = 0;
	data.bits = 0;

	for (i = 0; i < ir_buffer->count; i++) {
		duration = ir_buffer->data[i].duration;
		pulse = ir_buffer->data[i].pulse;

		//printk("NEC decode started at state %d (%d us %d)\n", data.state, duration, pulse);
		switch (data.state) {

		case STATE_INACTIVE:
			if (!pulse) {
				//printk("check STATE_INACTIVE state pulse error\n");
				break;
			}

			if (eq_margin(duration, NEC_HEADER_PULSE, NEC_UNIT * 3) ||
					eq_margin(duration, NECX_HEADER_PULSE, NEC_UNIT)) {  /* Actual value ~ 7200000 ns*/
				data.state = STATE_HEADER_SPACE;
			}

			break;

		case STATE_HEADER_SPACE:
			if (pulse) {
				//printk("check STATE_HEADER_SPACE state pulse error\n");
				break;
			}
			if (eq_margin(duration, NEC_HEADER_SPACE, NEC_UNIT *3)) { /* Actual value ~ 3700000 ns */
				data.state = STATE_BIT_PULSE;
			} else {
				//printk("check leading code 4.5ms is error\n");
				goto error;
			}
			break;

		case STATE_BIT_PULSE:
			if (!pulse) {
				//printk("check STATE_BIT_PULSE state pulse error\n");
				goto error;
			}
			if (!eq_margin(duration, NEC_BIT_PULSE, NEC_UNIT / 2)) {/* Actual value ~ 400000 ns */
				//printk("check STATE_BIT_PULSE duration error");
				goto error;
			}
			data.state = STATE_BIT_SPACE;

			break;

		case STATE_BIT_SPACE:
			if (pulse) {
				printk("check STATE_BIT_SPACE state pulse error\n");
				goto error;
			}
			data.bits <<= 1;
			if (eq_margin(duration, NEC_BIT_1_SPACE, NEC_UNIT))
				data.bits |= 1;
			else if (!eq_margin(duration, NEC_BIT_0_SPACE, NEC_UNIT / 2)) {
				//printk("check STATE_BIT_SPACE state duration error\n");
				goto error;
			}

			data.count++;

			if (data.count == NEC_NBITS)
				data.state = STATE_TRAILER_PULSE;
			else
				data.state = STATE_BIT_PULSE;

			break;

		case STATE_TRAILER_PULSE:
			if (!pulse) {
				//printk("check STATE_TRAILER_PULSE state pulse error\n");
				goto error;
			}

			if (!eq_margin(duration, NEC_TRAILER_PULSE, NEC_UNIT / 2)) {
				//printk("check STATE_TRAILER_PULSE duration error\n");
				goto error;
			}

			data.state = STATE_TRAILER_SPACE;
			break;

		case STATE_TRAILER_SPACE:
			if (pulse) {
				//printk("check STATE_TRAILER_SPACE state pulse error\n");
				goto error;
			}
			if (duration > NEC_TRAILER_SPACE) {
				if (data.count == NEC_NBITS) {
					address     = bitrev8((data.bits >> 24) & 0xff);
					not_address = bitrev8((data.bits >> 16) & 0xff);
					command	    = bitrev8((data.bits >>  8) & 0xff);
					not_command = bitrev8((data.bits >>  0) & 0xff);

					if ((command ^ not_command) != 0xff) {
						//IR_dprintk(1, "NEC checksum error: received 0x%08x\n",data->bits);
						send_32bits = 1;
					}

					if (send_32bits) {
						/* NEC transport, but modified protocol, used by at
							* least Apple and TiVo remotes */
						scancode = data.bits;
						//IR_dprintk(1, "NEC (modified) scancode 0x%08x\n", scancode);
					} else {
						/* Extended NEC */
						scancode = address << 8  |
						not_address << 16 |
						command;
						//IR_dprintk(1, "NEC scancode 0x%06x\n", scancode);
					}
					goto success;
				}
			} else {
				//printk("check STATE_TRAILER_SPACE duration error\n");
				goto error;
			}
			break;
		default:
			printk("NEC unknown mode\n");
			goto error;
		}
	}
success:
	//printk("data.bits = %x \n", data.bits);
	//printk("scancode = %x \n", scancode);
	return scancode;
error:
	//printk("error\n");
	return FALSE;

}
