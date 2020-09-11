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

		int64_t result = 0;
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
		(void) first;
		(void) last;
		(void) value;
		return nullptr;
	}
}