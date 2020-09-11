#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
//
// Created by jan on 20.06.19.
//

#include <stdexcept>

#include "voice.hpp"

namespace mumble_client::protocol::voice {

	struct ping_packet::impl {

	};

	ping_packet::ping_packet() = default;

	ping_packet::~ping_packet() = default;

	std::tuple<int64_t, const std::byte *> decode_varint(const std::byte *first, const std::byte *last) {

		if (!first || first == last) {
			throw std::out_of_range{"Invalid range specified."};
		}

		int64_t result;
		auto ptr = first;

		if ((*ptr & std::byte{0x80}) == std::byte{0x00}) {
			result = std::to_integer<int64_t>(*ptr & std::byte{0x7f});
			return {result, ++ptr};
		} else if ((*ptr & std::byte{0xc0}) == std::byte{0x80}) {
			if (last - ptr < 2) {
				throw std::out_of_range{"Invalid range specified."};
			}
			result = std::to_integer<int64_t>(*ptr & std::byte{0x3f}) << 8;
			result |= std::to_integer<int64_t>(*++ptr);
			return {result, ++ptr};
		} else if ((*ptr & std::byte{0xe0}) == std::byte{0xc0}) {
			if (last - ptr < 3) {
				throw std::out_of_range{"Invalid range specified."};
			}
			result = std::to_integer<int64_t>(*ptr & std::byte{0x1f}) << 16;
			result |= std::to_integer<int64_t>(*++ptr) << 8;
			result |= std::to_integer<int64_t>(*++ptr);
			return {result, ++ptr};
		} else if ((*ptr & std::byte{0xf0}) == std::byte{0xe0}) {
			if (last - ptr < 4) {
				throw std::out_of_range{"Invalid range specified."};
			}
			result = std::to_integer<int64_t>(*ptr & std::byte{0x0f}) << 24;
			result |= std::to_integer<int64_t>(*++ptr) << 16;
			result |= std::to_integer<int64_t>(*++ptr) << 8;
			result |= std::to_integer<int64_t>(*++ptr);
			return {result, ++ptr};
		} else if ((*ptr & std::byte{0xf0}) == std::byte{0xf0}) {
			std::tuple<int64_t, const std::byte *> recursive_result;
			switch (std::to_integer<uint8_t>(*ptr & std::byte{0xfc})) {
				case 0xf0:
					if (last - ptr < 5) {
						throw std::out_of_range{"Invalid range specified."};
					}
					result = std::to_integer<int64_t>(*++ptr) << 24;
					result |= std::to_integer<int64_t>(*++ptr) << 16;
					result |= std::to_integer<int64_t>(*++ptr) << 8;
					result |= std::to_integer<int64_t>(*++ptr);
					return {result, ++ptr};
				case 0xf4:
					if (last - ptr < 9) {
						throw std::out_of_range{"Invalid range specified."};
					}
					result = std::to_integer<int64_t>(*++ptr) << 56;
					result |= std::to_integer<int64_t>(*++ptr) << 48;
					result |= std::to_integer<int64_t>(*++ptr) << 40;
					result |= std::to_integer<int64_t>(*++ptr) << 32;
					result |= std::to_integer<int64_t>(*++ptr) << 24;
					result |= std::to_integer<int64_t>(*++ptr) << 16;
					result |= std::to_integer<int64_t>(*++ptr) << 8;
					result |= std::to_integer<int64_t>(*++ptr);
					return {result, ++ptr};
					//TODO: Make this case work
				case 0xf8:
					recursive_result = decode_varint(++ptr, last);
					return {~std::get<int64_t>(recursive_result), std::get<const std::byte *>(recursive_result)};
				case 0xfc:
					result = ~std::to_integer<int64_t>(*ptr & std::byte{0x03});
					return {result, ++ptr};
			}
		}
		return std::tuple<int64_t, std::byte *>();
	}

	std::byte *encode_varint(std::byte *first, std::byte *last, int64_t value) {

		if (!first || first == last) {
			throw std::out_of_range{"Invalid range specified."};
		}

		int64_t i = value;
		auto ptr = first;

		if ((i & 0x8000000000000000LL) && (~i < 0x100000000LL)) {
			// Signed number.
			i = ~i;
			if (i <= 0x3) {
				// Shortcase for -1 to -4
				*ptr = std::byte(0xFC | i);
				return ++ptr;
			} else {
				*ptr = std::byte{0xF8};
				++ptr;
			}
		}

		if (i < 0x80) {
			// Need top bit clear
			*ptr = std::byte(i);
		} else if (i < 0x4000) {
			// Need top two bits clear
			*ptr = std::byte((i >> 8) | 0x80);
			*++ptr = std::byte(i & 0xFF);
		} else if (i < 0x200000) {
			// Need top three bits clear
			*ptr = std::byte((i >> 16) | 0xC0);
			*++ptr = std::byte((i >> 8) & 0xFF);
			*++ptr = std::byte(i & 0xFF);
		} else if (i < 0x10000000) {
			// Need top four bits clear
			*ptr = std::byte((i >> 24) | 0xE0);
			*++ptr = std::byte((i >> 16) & 0xFF);
			*++ptr = std::byte((i >> 8) & 0xFF);
			*++ptr = std::byte(i & 0xFF);
		} else if (i < 0x100000000LL) {
			// It's a full 32-bit integer.
			*ptr = std::byte(0xF0);
			*++ptr = std::byte((i >> 24) & 0xFF);
			*++ptr = std::byte((i >> 16) & 0xFF);
			*++ptr = std::byte((i >> 8) & 0xFF);
			*++ptr = std::byte(i & 0xFF);
		} else {
			// It's a 64-bit value.
			*ptr = std::byte(0xF4);
			*++ptr = std::byte((i >> 56) & 0xFF);
			*++ptr = std::byte((i >> 48) & 0xFF);
			*++ptr = std::byte((i >> 40) & 0xFF);
			*++ptr = std::byte((i >> 32) & 0xFF);
			*++ptr = std::byte((i >> 24) & 0xFF);
			*++ptr = std::byte((i >> 16) & 0xFF);
			*++ptr = std::byte((i >> 8) & 0xFF);
			*++ptr = std::byte(i & 0xFF);
		}
		return ++ptr;
	}
}
#pragma clang diagnostic pop