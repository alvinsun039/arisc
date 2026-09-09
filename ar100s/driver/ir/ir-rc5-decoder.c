/*
 * drivers/ir/ir-rc5-decoder.c
 * [ take care ] no need care repeat key in ar100s
 *
 * Copyright (C) 2022-2026 AllWinnertech Ltd.
 * Author: qinguangzhi <qinguangzhi@allwinnertech.com>
 *
 */

#include "ir_i.h"

#define RC5_NBITS		    (14)
#define RC5_SZ_NBITS		(15)
#define RC5X_NBITS		    (20)
#define CHECK_RC5X_NBITS	(8)
#define RC5_UNIT		    (888888) /* ns */
#define RC5_BIT_START	    (1 * RC5_UNIT)
#define RC5_BIT_END		    (1 * RC5_UNIT)
#define RC5X_SPACE		    (4 * RC5_UNIT)
#define RC5_TRAILER		    (6 * RC5_UNIT) /* In reality, approx 100 */

enum rc5_state {
	STATE_INACTIVE,
	STATE_BIT_START,
	STATE_BIT_END,
	STATE_CHECK_RC5X,
	STATE_FINISHED,
};

struct rc5_dec {
    u32 state;
    u32 bits;
    u32 count;
    u32 is_rc5x;
};

u32 ir_rc5_decode(ir_raw_buffer_t *ir_buffer)
{
	u32 scancode = 0;
	u32 i = 0;
	u32 duration = 0;
	u32 pulse = 0;

	//u32 toggle;

	struct rc5_dec data = {0, 0, 0, 0};

	for (; i < ir_buffer->count; i++) {

		duration = ir_buffer->data[i].duration;
		pulse = ir_buffer->data[i].pulse;

		if (!geq_margin(duration, RC5_UNIT, RC5_UNIT / 2)) {
			goto error;
		}

again:
		//printk("RC5 decode started at state %d (%d us %d)\n", data.state, duration, pulse);

		if (!geq_margin(duration, RC5_UNIT, RC5_UNIT / 2))
			continue;

		switch (data.state) {

		case STATE_INACTIVE:   // STATE_INACTIVE = 0
			if (!pulse) {
				//printk("check STATE_INACTIVE pulse error\n");
				goto error;
			}
			data.state = STATE_BIT_START;
			data.count = 1;
			decrease_duration(&duration, RC5_BIT_START);
			goto again;

		case STATE_BIT_START:  // STATE_BIT_START = 1
			if (!pulse && geq_margin(duration, RC5_TRAILER, RC5_UNIT / 2)) {
				data.state = STATE_FINISHED;
				goto again;
			}

			if (!eq_margin(duration, RC5_BIT_START, RC5_UNIT / 2))
				break;

			data.bits <<= 1;
			if (!pulse)
				data.bits |= 1;
			data.count++;
			data.state = STATE_BIT_END;
			break;

		case STATE_BIT_END:    // STATE_BIT_END = 2
			if (data.count == CHECK_RC5X_NBITS)
				data.state = STATE_CHECK_RC5X;
			else
				data.state = STATE_BIT_START;

			decrease_duration(&duration, RC5_BIT_END);
			goto again;

		case STATE_CHECK_RC5X:  // STATE_CHECK_RC5X = 3
			if (!pulse && geq_margin(duration, RC5X_SPACE, RC5_UNIT / 2)) {
				data.is_rc5x = 1;
				decrease_duration(&duration, RC5X_SPACE);
			} else
				data.is_rc5x = 0;
			data.state = STATE_BIT_START;
			goto again;

		case STATE_FINISHED:   // STATE_FINISHED = 4
			if (data.is_rc5x && data.count == RC5X_NBITS) {
				u8 xdata, command, system;

				xdata    = (data.bits & 0x0003F) >> 0;
				command  = (data.bits & 0x00FC0) >> 6;
				system   = (data.bits & 0x1F000) >> 12;
				//toggle   = (data.bits & 0x20000) ? 1 : 0;
				command += (data.bits & 0x40000) ? 0 : 0x40;
				scancode = system << 16 | command << 8 | xdata;

			} else if (!data.is_rc5x && data.count == RC5_NBITS) {
				u8 command, system;

				command  = (data.bits & 0x0003F) >> 0;
				system   = (data.bits & 0x007C0) >> 6;
				//toggle   = (data.bits & 0x00800) ? 1 : 0;
				command += (data.bits & 0x01000) ? 0 : 0x40;
				scancode = system << 8 | command;

			} else if (!data.is_rc5x && data.count == RC5_SZ_NBITS) {
				u8 command, system;

				command  = (data.bits & 0x0003F) >> 0;
				system   = (data.bits & 0x02FC0) >> 6;
				//toggle   = (data.bits & 0x01000) ? 1 : 0;
				scancode = system << 6 | command;

			} else {
				//printk("--------error------\n");
				//printk("data.count = %d \n", data.count);
				goto error;
			}
			goto success;
		}
	}

success:
	//printk("scancode = %x \n", scancode);
	return scancode;
error:
	return FALSE;
}
