
#include "include.h"
#include "sunxi_arisc_rpm.h"

#ifdef RPM_DEBUG
#define DEBUG(...) LOG(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#define NUM_REMOTE_PROCESSOR (2)

enum msgbox_processor_id {
	PROCESSOR_ID_ARM   = AMP_ARM,
	PROCESSOR_ID_ARISC = AMP_RISC,
	PROCESSOR_ID_MIPS  = AMP_MIPS,
};

enum rpm_received_state {
	RPM_RECEIVED_IDLE,
	RPM_RECEIVED_HEADER,
	RPM_RECEIVED_PAYLOAD,
};

struct rpm_received_info {
	int state;
	uint8_t received_length;
	uint8_t payload[MAX_PAYLOAD_SIZE];
};

struct rpm_context {
	unsigned char seqnumber;
	rpm_message_cb callback;
	struct amp_msg amp_info;
};

static struct rpm_context rpm_ctxs[NUM_REMOTE_PROCESSOR] = {
	// arisc to arm
	{
		.seqnumber = 0,
		.callback  = NULL,
		.amp_info = {
			.local_amp	= PROCESSOR_ID_ARISC,
			.remote_amp = PROCESSOR_ID_ARM,
			.write_ch	= 1,
			.read_ch	= 1,
		},
	},
	// arisc to mips
	{
		.seqnumber = 0,
		.callback  = NULL,
		.amp_info = {
			.local_amp	= PROCESSOR_ID_ARISC,
			.remote_amp = PROCESSOR_ID_MIPS,
			.write_ch	= 1,
			.read_ch	= 1,
		},
	},
};

static int rpm_send_to_remote(struct rpm_context *ctx, uint8_t type, uint8_t lenght, const uint8_t *pdata)
{
	rpm_message_header_t header;

	memset(&header, 0, sizeof(header));

	header.magic = MESSAGE_AMP_MAGIC;
	header.seqnumber = ctx->seqnumber++;
	header.type  = type;
	header.count = lenght;

	DEBUG("send header : %d\n", sizeof(header));
	amp_msgbox_send_message(&ctx->amp_info, (uint8_t *)&header, sizeof(header), 1000);

	DEBUG("send payload: %d\n", lenght);
	amp_msgbox_send_message(&ctx->amp_info, (uint8_t *)pdata, lenght, 1000);

	return 0;
}

int rpm_send_to_arm(uint8_t type, uint8_t lenght, const uint8_t *pdata)
{
	return rpm_send_to_remote(&rpm_ctxs[0], type, lenght, pdata);
}

int rpm_send_to_mips(uint8_t type, uint8_t lenght, const uint8_t *pdata)
{
	return rpm_send_to_remote(&rpm_ctxs[1], type, lenght, pdata);
}

static void notify_msg(u32 remote, uint8_t type, uint8_t lenght, uint8_t *pdata)
{
	struct rpm_context *ctx = NULL;
#ifdef RPM_DEBUG
	int i = 0;
	LOG("rpm: remote %d type %x lenght %x\n", remote, type, lenght);
	while (i < lenght) {
		LOG("%x ", pdata[i++]);
	}
	LOG("\n\n");
#endif

	switch (remote) {
	case PROCESSOR_ID_ARM:
		ctx = &rpm_ctxs[0];
		break;
	case PROCESSOR_ID_MIPS:
		ctx = &rpm_ctxs[1];
		break;
	default:
		ERR("invalid rpm remote processor id(%d)\n", remote);
		break;
	}

	if (ctx->callback)
		ctx->callback(type, lenght, pdata);
}

static struct rpm_received_info receive_infos[2] = {
	// arm to cpus
	{
		.state = RPM_RECEIVED_IDLE,
		.received_length = 0,
	},
	// mips to cpus
	{
		.state = RPM_RECEIVED_IDLE,
		.received_length = 0,
	},
};

static s32 sunxi_arisc_receive_from_remote(u32 remote, u32 channel)
{
	uint8_t magic, type = 0;
	uint8_t payload_len = 0;
	uint32_t i = 0, rx;
	struct rpm_received_info *recvinfo = NULL;

	switch (remote) {
	case PROCESSOR_ID_ARM:
		recvinfo = &receive_infos[0];
		break;
	case PROCESSOR_ID_MIPS:
		recvinfo = &receive_infos[1];
		break;
	default:
		ERR("invalid rpm remote processor id(%d)\n", remote);
		break;
	}

	while (1) {
		if (amp_msgbox_remote_fifo_is_empty(remote, channel)) {
			if (recvinfo->state == RPM_RECEIVED_IDLE)
				break;
			else
				continue;;
		}

		rx = amp_msgbox_read_message(remote, channel);

		switch (recvinfo->state) {
		case RPM_RECEIVED_IDLE:
			magic = rx & 0xff;
			if (magic == MESSAGE_AMP_MAGIC) {
				recvinfo->received_length = 0;
				recvinfo->state = RPM_RECEIVED_HEADER;
				type		= (uint8_t)((rx >> 16) & 0xff);
				payload_len = (uint8_t)((rx >> 24) & 0xff);

				if (payload_len > MAX_PAYLOAD_SIZE) {
					ERR("rpm payload lenght exceeds MAX_PAYLOAD_SIZE(%d)\n", MAX_PAYLOAD_SIZE);
					payload_len = MAX_PAYLOAD_SIZE;
				}
			}
			DEBUG("rpm header: %x type %x len %x\n", rx, type, payload_len);
			break;
		case RPM_RECEIVED_HEADER:
			if (payload_len == 0) {
				// zero lenght payload message.
				notify_msg(remote, type, payload_len, recvinfo->payload);
				recvinfo->state = RPM_RECEIVED_IDLE;
			} else
				recvinfo->state = RPM_RECEIVED_PAYLOAD;

			break;
		case RPM_RECEIVED_PAYLOAD:
			i = 0;
			while (i < 4) {
				recvinfo->payload[recvinfo->received_length++] = (rx >> (i * 8)) & 0xFF;
				if (recvinfo->received_length >= payload_len)
					break;
				i++;
			}

			if (recvinfo->received_length == payload_len) {
				notify_msg(remote, type, payload_len, recvinfo->payload);
				recvinfo->state = RPM_RECEIVED_IDLE;
			}
			break;
		default:
			break;
		}
	}

	return 0;
}

int rpm_init(void)
{
	return amp_msgbox_register_service(sunxi_arisc_receive_from_remote);
}

int rpm_register_message_callback(int callback_type, rpm_message_cb cb)
{
	switch (callback_type) {
	case RPM_CB_MSG_FROM_ARM:
		rpm_ctxs[0].callback = cb;
		break;
	case RPM_CB_MSG_FROM_MIPS:
		rpm_ctxs[1].callback = cb;
		break;
	default:
		break;
	}
	return 0;
}

