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

	std::byte *decode_varint(std::byte *start, const std::byte *const limit, int64_t &result) {

		auto ptr = start;

		if (!ptr) { return nullptr; }

		result = 0;

		if (ptr == limit) { return ptr; }

		if ((*ptr & std::byte{0x80}) == std::byte{0x00}) {
			result = std::to_integer<int64_t>(*ptr++ & std::byte{0x7f});
			return ptr;
		} else if ((*ptr & std::byte{0xc0}) == std::byte{0x80}) {
			result = std::to_integer<int64_t>(*ptr++ & std::byte{0x3f}) << 8;
			if (ptr == limit) { return ptr; }
			result |= std::to_integer<int64_t>(*ptr++);
			return ptr;
		} else if ((*ptr & std::byte{0xe0}) == std::byte{0xc0}) {
			result = std::to_integer<int64_t>(*ptr++ & std::byte{0x1f}) << 16;
			if (ptr == limit) { return ptr; }
			result |= std::to_integer<int64_t>(*ptr++) << 8;
			if (ptr == limit) { return ptr; }
			result |= std::to_integer<int64_t>(*ptr++);
			return ptr;
		} else if ((*ptr & std::byte{0xf0}) == std::byte{0xe0}) {
			result = std::to_integer<int64_t>(*ptr++ & std::byte{0x0f}) << 24;
			if (ptr == limit) { return ptr; }
			result |= std::to_integer<int64_t>(*ptr++) << 16;
			if (ptr == limit) { return ptr; }
			result |= std::to_integer<int64_t>(*ptr++) << 8;
			if (ptr == limit) { return ptr; }
			result |= std::to_integer<int64_t>(*ptr++);
			return ptr;
		} else if ((*ptr & std::byte{0xf0}) == std::byte{0xf0}) {
			switch (std::to_integer<uint8_t>(*ptr & std::byte{0xfc})) {
				case 0xf0:
					++ptr;
					result = std::to_integer<int64_t>(*ptr++) << 24;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++) << 16;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++) << 8;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++);
					return ptr;
				case 0xf4:
					++ptr;
					result = std::to_integer<int64_t>(*ptr++) << 56;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++) << 48;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++) << 40;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++) << 32;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++) << 24;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++) << 16;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++) << 8;
					if (ptr == limit) { return ptr; }
					result |= std::to_integer<int64_t>(*ptr++);
					return ptr;
				case 0xf8:
					ptr = decode_varint(++ptr, limit, result);
					result = ~result;
					return ptr;
				case 0xfc:
					result = ~std::to_integer<int64_t>(*ptr++ & std::byte{0x03});
			}
		}
		return ptr;
	}

#pragma clang diagnostic pop

	std::byte *encode_varint(std::byte *start, const std::byte *limit, const int64_t value) {

		auto ptr = start;

		if (!ptr) {
			return nullptr;
		}

		return ptr;
	}
}