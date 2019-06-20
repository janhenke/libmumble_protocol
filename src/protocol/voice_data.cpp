//
// Created by jan on 20.06.19.
//

#include "voice_data.hpp"

namespace mumble_client::protocol::voice {

	struct ping_packet::impl {

	};

	ping_packet::ping_packet() = default;

	ping_packet::~ping_packet() = default;


	std::size_t decode_varint(int64_t &result, const uint8_t *data) {

		result = 0;
		std::size_t offset = 0;
		uint8_t v = data[offset++];

		if ((v & 0x80) == 0x00) {
			result = v & 0x7f;
		}

		return offset;
	}
}