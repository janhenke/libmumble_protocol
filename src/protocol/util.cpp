//
// Created by jan on 26.10.21.
//

#include <stdexcept>

#include "util.h"

namespace mumble_client::protocol::voice {
std::tuple<int64_t, const std::size_t> decode_varint(std::span<const std::byte> buffer) {
	if (buffer.empty()) { throw std::out_of_range{"Invalid range specified."}; }

	int64_t result;

	if ((buffer[0] & std::byte{0x80}) == std::byte{0x00}) {
		result = std::to_integer<int64_t>(buffer[0] & std::byte{0x7f});
		return {result, 1};
	} else if ((buffer[0] & std::byte{0xc0}) == std::byte{0x80}) {
		if (std::size(buffer) < 2) { throw std::out_of_range{"Invalid range specified."}; }
		result = std::to_integer<int64_t>(buffer[0] & std::byte{0x3f}) << 8;
		result |= std::to_integer<int64_t>(buffer[1]);
		return {result, 2};
	} else if ((buffer[0] & std::byte{0xe0}) == std::byte{0xc0}) {
		if (std::size(buffer) < 3) { throw std::out_of_range{"Invalid range specified."}; }
		result = std::to_integer<int64_t>(buffer[0] & std::byte{0x1f}) << 16;
		result |= std::to_integer<int64_t>(buffer[1]) << 8;
		result |= std::to_integer<int64_t>(buffer[2]);
		return {result, 3};
	} else if ((buffer[0] & std::byte{0xf0}) == std::byte{0xe0}) {
		if (std::size(buffer) < 4) { throw std::out_of_range{"Invalid range specified."}; }
		result = std::to_integer<int64_t>(buffer[0] & std::byte{0x0f}) << 24;
		result |= std::to_integer<int64_t>(buffer[1]) << 16;
		result |= std::to_integer<int64_t>(buffer[2]) << 8;
		result |= std::to_integer<int64_t>(buffer[3]);
		return {result, 4};
	} else if ((buffer[0] & std::byte{0xf0}) == std::byte{0xf0}) {

		// handle recursive call
		if ((buffer[0] & std::byte{0xfc}) == std::byte{0xf8}) {
			auto [i, count] = decode_varint(buffer.last(std::size(buffer) - 1));
			return {~i, count + 1};
		}

		switch (std::to_integer<uint8_t>(buffer[0] & std::byte{0xfc})) {
			case 0xf0:
				if (std::size(buffer) < 5) { throw std::out_of_range{"Invalid range specified."}; }
				result = std::to_integer<int64_t>(buffer[1]) << 24;
				result |= std::to_integer<int64_t>(buffer[2]) << 16;
				result |= std::to_integer<int64_t>(buffer[3]) << 8;
				result |= std::to_integer<int64_t>(buffer[4]);
				return {result, 5};
			case 0xf4:
				if (std::size(buffer) < 9) { throw std::out_of_range{"Invalid range specified."}; }
				result = std::to_integer<int64_t>(buffer[1]) << 56;
				result |= std::to_integer<int64_t>(buffer[2]) << 48;
				result |= std::to_integer<int64_t>(buffer[3]) << 40;
				result |= std::to_integer<int64_t>(buffer[4]) << 32;
				result |= std::to_integer<int64_t>(buffer[5]) << 24;
				result |= std::to_integer<int64_t>(buffer[6]) << 16;
				result |= std::to_integer<int64_t>(buffer[7]) << 8;
				result |= std::to_integer<int64_t>(buffer[8]);
				return {result, 9};
			case 0xfc: result = ~std::to_integer<int64_t>(buffer[0] & std::byte{0x03}); return {result, 1};
		}
	}
	return std::tuple<int64_t, std::size_t>();
}

std::size_t encode_varint(int64_t value, std::span<std::byte> buffer) {

	if (buffer.empty()) { throw std::out_of_range{"Invalid range specified."}; }

	int64_t i = value;
	std::size_t offset = 0;

	if ((i & 0x8000'0000'0000'0000LL) && (~i < 0x1'0000'0000LL)) {
		// Signed number.
		i = ~i;
		if (i <= 0x3) {
			// Shortcase for -1 to -4
			buffer[0] = std::byte(0xFC | i);
			return 1;
		} else {
			buffer[0] = std::byte{0xF8};
			offset = 1;
		}
	}

	if (i < 0x80) {
		// Need top bit clear
		buffer[offset] = std::byte(i);
		return offset + 1;
	} else if (i < 0x4000) {
		// Need top two bits clear
		buffer[offset] = std::byte((i >> 8) | 0x80);
		buffer[offset + 1] = std::byte(i & 0xFF);
		return offset + 2;
	} else if (i < 0x200000) {
		// Need top three bits clear
		buffer[offset] = std::byte((i >> 16) | 0xC0);
		buffer[offset + 1] = std::byte((i >> 8) & 0xFF);
		buffer[offset + 2] = std::byte(i & 0xFF);
		return offset + 3;
	} else if (i < 0x10000000) {
		// Need top four bits clear
		buffer[offset] = std::byte((i >> 24) | 0xE0);
		buffer[offset + 1] = std::byte((i >> 16) & 0xFF);
		buffer[offset + 2] = std::byte((i >> 8) & 0xFF);
		buffer[offset + 3] = std::byte(i & 0xFF);
		return offset + 4;
	} else if (i < 0x100000000LL) {
		// It's a full 32-bit integer.
		buffer[offset] = std::byte(0xF0);
		buffer[offset + 1] = std::byte((i >> 24) & 0xFF);
		buffer[offset + 2] = std::byte((i >> 16) & 0xFF);
		buffer[offset + 3] = std::byte((i >> 8) & 0xFF);
		buffer[offset + 4] = std::byte(i & 0xFF);
		return offset + 5;
	} else {
		// It's a 64-bit value.
		buffer[offset] = std::byte(0xF4);
		buffer[offset + 1] = std::byte((i >> 56) & 0xFF);
		buffer[offset + 2] = std::byte((i >> 48) & 0xFF);
		buffer[offset + 3] = std::byte((i >> 40) & 0xFF);
		buffer[offset + 4] = std::byte((i >> 32) & 0xFF);
		buffer[offset + 5] = std::byte((i >> 24) & 0xFF);
		buffer[offset + 6] = std::byte((i >> 16) & 0xFF);
		buffer[offset + 7] = std::byte((i >> 8) & 0xFF);
		buffer[offset + 8] = std::byte(i & 0xFF);
		return offset + 9;
	}
}
}// namespace mumble_client::protocol::voice
