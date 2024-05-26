//
// Created by jan on 30.12.23.
//

#include "util.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace libmumble_protocol {

std::expected<std::tuple<std::size_t, std::int64_t>, std::u8string>
DecodeVariableInteger(std::span<const std::byte> buffer) {

	const std::size_t spanSize = std::size(buffer);

	if (buffer.empty()) { std::unexpected{u8"Input buffer contains too few elements."}; }

	if ((buffer[0] & std::byte{0x80}) == std::byte{0x00}) {
		return {{1, std::bit_cast<std::uint8_t>(buffer[0] & std::byte{0x7f})}};
	} else if ((buffer[0] & std::byte{0xc0}) == std::byte{0x80}) {
		if (spanSize < 2) { std::unexpected{u8"Input buffer contains too few elements."}; }
		std::uint16_t result = 0;
		std::memcpy(&result, buffer.data(), 2);
		return {{2, SwapNetworkBytes(result) & 0x3fff}};
	} else if ((buffer[0] & std::byte{0xe0}) == std::byte{0xc0}) {
		if (spanSize < 3) { std::unexpected{u8"Input buffer contains too few elements."}; }
		std::uint32_t result = 0;
		std::memcpy(&result, buffer.data(), 3);
		result = SwapNetworkBytes(result);
		return {{3, (result >> 8) & 0x1f'ffff}};
	} else if ((buffer[0] & std::byte{0xf0}) == std::byte{0xe0}) {
		if (spanSize < 4) { std::unexpected{u8"Input buffer contains too few elements."}; }
		std::uint32_t result = 0;
		std::memcpy(&result, buffer.data(), 4);
		return {{4, SwapNetworkBytes(result) & 0x0fff'ffff}};
	} else if ((buffer[0] & std::byte{0xf0}) == std::byte{0xf0}) {

		// handle recursive call
		if ((buffer[0] & std::byte{0xfc}) == std::byte{0xf8}) {
			return DecodeVariableInteger(buffer.last(spanSize - 1))
				.transform([](std::tuple<std::size_t, std::int64_t> tuple) {
					const auto [count, result] = tuple;
					return std::make_tuple(count + 1, ~result);
				});
		}

		std::int32_t i32 = 0;
		std::int64_t i64 = 0;
		switch (std::to_integer<std::uint8_t>(buffer[0] & std::byte{0xfc})) {
			case 0xf0:
				if (spanSize < 5) { std::unexpected{u8"Input buffer contains too few elements."}; }
				std::memcpy(&i32, buffer.data() + 1, 4);
				return {{5, SwapNetworkBytes(i32)}};
			case 0xf4:
				if (spanSize < 9) { std::unexpected{u8"Input buffer contains too few elements."}; }
				std::memcpy(&i64, buffer.data() + 1, 8);
				return {{9, SwapNetworkBytes(i64)}};
			case 0xfc:
				return {{1, ~std::bit_cast<std::uint8_t>(buffer[0] & std::byte{0x03})}};
		}
	}
	return {{0, 0}};
}

std::expected<std::size_t, std::u8string> EncodeVariableInteger(std::span<std::byte> buffer, std::int64_t value) {

	if (buffer.empty()) { std::unexpected{u8"Invalid range specified."}; }

	std::int64_t input = value;
	std::size_t offset = 0;

	if ((input < 0) && (~input < 0x1'0000'0000LL)) {
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

	if (input < 0x80) {
		// Need top bit clear
		if (buffer.size() < offset + 1) { return std::unexpected{u8"Destination buffer too small."}; }
		const auto temp = static_cast<std::uint8_t>(input);
		std::memcpy(buffer.data() + offset, &temp, 1);
		return offset + 1;
	} else if (input < 0x4000) {
		// Need top two bits clear
		if (buffer.size() < offset + 2) { return std::unexpected{u8"Destination buffer too small."}; }
		const auto temp = SwapNetworkBytes(static_cast<std::uint16_t>(input));
		std::memcpy(buffer.data() + offset, &temp, 2);
		buffer[offset] = buffer[offset] | std::byte{0x80};
		return offset + 2;
	} else if (input < 0x200000) {
		// Need top three bits clear
		if (buffer.size() < offset + 3) { return std::unexpected{u8"Destination buffer too small."}; }
		const auto temp = SwapNetworkBytes(static_cast<std::uint32_t>(input << 8));
		std::memcpy(buffer.data() + offset, &temp, 3);
		buffer[offset] = buffer[offset] | std::byte{0xC0};
		return offset + 3;
	} else if (input < 0x10000000) {
		// Need top four bits clear
		if (buffer.size() < offset + 4) { return std::unexpected{u8"Destination buffer too small."}; }
		const auto temp = SwapNetworkBytes(static_cast<std::uint32_t>(input));
		std::memcpy(buffer.data() + offset, &temp, 4);
		buffer[offset] = buffer[offset] | std::byte{0xE0};
		return offset + 4;
	} else if (input < 0x100000000LL) {
		// It's a full 32-bit integer.
		if (buffer.size() < offset + 5) { return std::unexpected{u8"Destination buffer too small."}; }
		buffer[offset] = std::byte(0xF0);
		const auto temp = SwapNetworkBytes(static_cast<std::uint32_t>(input));
		std::memcpy(buffer.data() + offset + 1, &temp, 4);
		return offset + 5;
	} else {
		// It's a 64-bit value.
		if (buffer.size() < offset + 9) { return std::unexpected{u8"Destination buffer too small."}; }
		buffer[offset] = std::byte(0xF4);
		const auto temp = SwapNetworkBytes(static_cast<std::int64_t>(input));
		const auto bytes = std::bit_cast<std::array<std::byte, sizeof(std::int64_t)> >(temp);
		std::memcpy(buffer.data() + offset + 1, bytes.data(), 8);
		return offset + 9;
	}
}

} // namespace libmumble_protocol
