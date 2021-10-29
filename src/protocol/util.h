//
// Created by jan on 26.10.21.
//

#ifndef _UTIL_H_
#define _UTIL_H_

#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>

namespace mumble_client::protocol {
inline void assert_min_buffer_size(const std::span<std::byte> &buffer, const std::size_t min_size) {
	if (std::size(buffer) < min_size) { throw std::out_of_range{"Buffer does not satisfy minimum size."}; }
}

namespace voice {
std::tuple<int64_t, const std::size_t> decode_varint(std::span<const std::byte> buffer);

std::size_t encode_varint(int64_t value, std::span<std::byte> buffer);
}// namespace voice
}// namespace mumble_client::protocol

#endif//_UTIL_H_
