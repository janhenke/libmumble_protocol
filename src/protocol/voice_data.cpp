//
// Created by jan on 20.06.19.
//

#include "voice_data.hpp"

namespace mumble_client::protocol::voice {

	struct ping_packet::impl {

	};

	ping_packet::ping_packet() = default;

	ping_packet::~ping_packet() = default;


	int64_t decode_varint(const uint8_t **data_ptr) {

		auto data = *data_ptr;
		int64_t result = 0;

		if ((*data & 0x80) == 0x00) {
			result = *data & 0x7f;
		} else if ((*data & 0xc0) == 0x80) {
			result = (*data & 0x3f) << 8;
			data++;
			result |= *data;
		} else if ((*data & 0xe0) == 0xc0) {
			result = (*data & 0x1f) << 16;
			++data;
			result |= *data << 8;
			++data;
			result |= *data;
		} else if ((*data & 0xf0) == 0xe0) {
			result = (*data & 0x0f) << 24;
			++data;
			result |= *data << 16;
			++data;
			result |= *data << 8;
			++data;
			result |= *data;
		} else if ((*data & 0xf0) == 0xf0) {
			switch (*data & 0xfc) {
				case 0xf0:
					++data;
					result = *data << 24;
					++data;
					result |= *data << 16;
					++data;
					result |= *data << 8;
					++data;
					result |= *data;
					break;
				case 0xf4:
					++data;
					result = static_cast<uint64_t >(*data) << 56;
					++data;
					result |= static_cast<uint64_t >(*data) << 48;
					++data;
					result |= static_cast<uint64_t >(*data) << 40;
					++data;
					result |= static_cast<uint64_t >(*data) << 32;
					++data;
					result |= *data << 24;
					++data;
					result |= *data << 16;
					++data;
					result |= *data << 8;
					++data;
					result |= *data;
					break;
				case 0xf8:
					++data;
					result = -1 * decode_varint(&data);
					break;
				case 0xfc:
					result = ~(+data & 0x03);
					break;
			}
		}

		++data;
		return result;
	}
}