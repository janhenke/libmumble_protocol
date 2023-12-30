//
// Created by jan on 30.12.23.
//

#include "util.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <stdexcept>

namespace libmumble_protocol::common {

std::tuple<std::size_t, std::int64_t> decodeVariableInteger(std::span<const std::byte> span) {
	// n.b. this code assumes little endian

	const std::size_t span_size = std::size(span);
	std::array<std::byte, sizeof(std::int64_t)> buffer{};
	buffer.fill(std::byte{0x00});

	if (span.empty()) { throw std::out_of_range{"Input span contains too few elements."}; }

	if ((span[0] & std::byte{0x80}) == std::byte{0x00}) {
		buffer[0] = span[0] & std::byte{0x7f};
		return {1, std::bit_cast<std::int64_t>(buffer)};
	} else if ((span[0] & std::byte{0xc0}) == std::byte{0x80}) {
		if (span_size < 2) { throw std::out_of_range{"Input span contains too few elements."}; }
		buffer[1] = span[0] & std::byte{0x3f};
		buffer[0] = span[1];
		return {2, std::bit_cast<std::int64_t>(buffer)};
	} else if ((span[0] & std::byte{0xe0}) == std::byte{0xc0}) {
		if (span_size < 3) { throw std::out_of_range{"Input span contains too few elements."}; }
		buffer[2] = span[0] & std::byte{0x1f};
		buffer[1] = span[1];
		buffer[0] = span[2];
		return {3, std::bit_cast<std::int64_t>(buffer)};
	} else if ((span[0] & std::byte{0xf0}) == std::byte{0xe0}) {
		if (span_size < 4) { throw std::out_of_range{"Input span contains too few elements."}; }
		buffer[3] = span[0] & std::byte{0x0f};
		buffer[2] = span[1];
		buffer[1] = span[2];
		buffer[0] = span[3];
		return {4, std::bit_cast<std::int64_t>(buffer)};
	} else if ((span[0] & std::byte{0xf0}) == std::byte{0xf0}) {

		// handle recursive call
		if ((span[0] & std::byte{0xfc}) == std::byte{0xf8}) {
			const auto [count, result] = decodeVariableInteger(span.last(span_size - 1));
			return {count + 1, ~result};
		}

		switch (std::to_integer<std::uint8_t>(span[0] & std::byte{0xfc})) {
			case 0xf0:
				if (span_size < 5) { throw std::out_of_range{"Input span contains too few elements."}; }
				buffer[3] = span[1];
				buffer[2] = span[2];
				buffer[1] = span[3];
				buffer[0] = span[4];
				return {5, std::bit_cast<std::int64_t>(buffer)};
			case 0xf4:
				if (span_size < 9) { throw std::out_of_range{"Input span contains too few elements."}; }
				buffer[7] = span[1];
				buffer[6] = span[2];
				buffer[5] = span[3];
				buffer[4] = span[4];
				buffer[3] = span[5];
				buffer[2] = span[6];
				buffer[1] = span[7];
				buffer[0] = span[8];
				return {9, std::bit_cast<std::int64_t>(buffer)};
			case 0xfc: buffer[0] = span[0] & std::byte{0x03}; return {1, ~std::bit_cast<std::int64_t>(buffer)};
		}
	}
	return {0, 0};
}

std::size_t encodeVariableInteger(std::span<std::byte> buffer, std::int64_t value) {
	// n.b. this code assumes little endian

	if (buffer.empty()) { throw std::out_of_range{"Invalid range specified."}; }

	std::int64_t input = value;
	std::size_t offset = 0;

	if ((input & 0x8000'0000'0000'0000LL) && (~input < 0x1'0000'0000LL)) {
		// Signed number.
		input = ~input;
		if (input <= 0x3) {
			// shortcut for -1 to -4
			buffer[0] = std::byte(0xFC | input);
			return 1;
		} else {
			buffer[0] = std::byte{0xF8};
			offset = 1;
		}
	}

	const auto bytes = std::bit_cast<std::array<std::byte, sizeof(std::int64_t)>>(input);
	if (input < 0x80) {
		// Need top bit clear
		buffer[offset] = bytes[0];
		return offset + 1;
	} else if (input < 0x4000) {
		// Need top two bits clear
		buffer[offset] = bytes[1] | std::byte{0x80};
		buffer[offset + 1] = bytes[0];
		return offset + 2;
	} else if (input < 0x200000) {
		// Need top three bits clear
		buffer[offset] = bytes[2] | std::byte{0xC0};
		buffer[offset + 1] = bytes[1];
		buffer[offset + 2] = bytes[0];
		return offset + 3;
	} else if (input < 0x10000000) {
		// Need top four bits clear
		buffer[offset] = bytes[3] | std::byte{0xE0};
		buffer[offset + 1] = bytes[2];
		buffer[offset + 2] = bytes[1];
		buffer[offset + 3] = bytes[0];
		return offset + 4;
	} else if (input < 0x100000000LL) {
		// It's a full 32-bit integer.
		buffer[offset] = std::byte(0xF0);
		buffer[offset + 1] = bytes[3];
		buffer[offset + 2] = bytes[2];
		buffer[offset + 3] = bytes[1];
		buffer[offset + 4] = bytes[0];
		return offset + 5;
	} else {
		// It's a 64-bit value.
		buffer[offset] = std::byte(0xF4);
		buffer[offset + 1] = bytes[7];
		buffer[offset + 2] = bytes[6];
		buffer[offset + 3] = bytes[5];
		buffer[offset + 4] = bytes[4];
		buffer[offset + 5] = bytes[3];
		buffer[offset + 6] = bytes[2];
		buffer[offset + 7] = bytes[1];
		buffer[offset + 8] = bytes[0];
		return offset + 9;
	}
}

}// namespace libmumble_protocol::common
