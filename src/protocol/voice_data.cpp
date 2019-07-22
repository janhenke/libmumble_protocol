//
// Created by jan on 20.06.19.
//

#include "voice_data.hpp"

namespace mumble_client::protocol::voice {

	struct ping_packet::impl {

	};

	ping_packet::ping_packet() = default;

	ping_packet::~ping_packet() = default;


#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
	size_t decode_varint(int64_t &result, const uint8_t *data) {

		result = 0;

		if ((data[0] & 0x80) == 0x00) {
			result = data[0] & 0x7f;
			return 1;
		} else if ((data[0] & 0xc0) == 0x80) {
			result = (data[0] & 0x3f) << 8;
			result |= data[1];
			return 2;
		} else if ((data[0] & 0xe0) == 0xc0) {
			result = (data[0] & 0x1f) << 16;
			result |= data[1] << 8;
			result |= data[2];
			return 3;
		} else if ((data[0] & 0xf0) == 0xe0) {
			result = (data[0] & 0x0f) << 24;
			result |= data[1] << 16;
			result |= data[2] << 8;
			result |= data[3];
			return 4;
		} else if ((data[0] & 0xf0) == 0xf0) {
			size_t offset = 0;
			switch (data[0] & 0xfc) {
				case 0xf0:
					result = data[1] << 24;
					result |= data[2] << 16;
					result |= data[3] << 8;
					result |= data[4];
					return 5;
				case 0xf4:
					result = static_cast<uint64_t >(data[1]) << 56;
					result |= static_cast<uint64_t >(data[2]) << 48;
					result |= static_cast<uint64_t >(data[3]) << 40;
					result |= static_cast<uint64_t >(data[4]) << 32;
					result |= data[5] << 24;
					result |= data[6] << 16;
					result |= data[7] << 8;
					result |= data[8];
					return 9;
				case 0xf8:
					offset = decode_varint(result, &data[1]);
					result = ~result;
					return offset + 1;
				case 0xfc:
					result = ~(data[0] & 0x03);
					return 1;
			}
		}
		return 1;
	}
#pragma clang diagnostic pop
}