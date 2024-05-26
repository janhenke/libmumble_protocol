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

MUMBLE_PROTOCOL_EXPORT auto SwapNetworkBytes(std::integral auto const i) {
	if constexpr (std::endian::native == std::endian::little) {
		return std::byteswap(i);
	} else {
		return i;
	}
}

MUMBLE_PROTOCOL_EXPORT std::expected<std::tuple<std::size_t, std::int64_t>, std::u8string>
DecodeVariableInteger(std::span<const std::byte> tuple);

MUMBLE_PROTOCOL_EXPORT std::expected<std::size_t, std::u8string>
EncodeVariableInteger(std::span<std::byte> buffer, std::int64_t value);

} // namespace libmumble_protocol
