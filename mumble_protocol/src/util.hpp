//
// Created by jan on 30.12.23.
//

#pragma once

#include "mumble_protocol_export.h"
#include "packet.hpp"

#include <bit>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <expected>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>

namespace libmumble_protocol {

auto SwapNetworkBytes(std::integral auto const number) {
	if constexpr (std::endian::native == std::endian::little) {
		return std::byteswap(number);
	} else {
		return number;
	}
}

MUMBLE_PROTOCOL_EXPORT auto DecodeVariableInteger(
	std::span<const std::byte> tuple) -> std::expected<std::tuple<std::size_t, std::int64_t>, std::u8string>;

MUMBLE_PROTOCOL_EXPORT auto EncodeVariableInteger(std::span<std::byte> buffer,
                                                  std::int64_t value) -> std::expected<std::size_t, std::u8string>;

} // namespace libmumble_protocol
