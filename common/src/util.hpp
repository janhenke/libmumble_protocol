//
// Created by jan on 30.12.23.
//

#pragma once

#include "mumble_protocol_common_export.h"
#include "packet_definition.hpp"

#include <google/protobuf/message_lite.h>

#include <bit>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <span>
#include <tuple>

namespace libmumble_protocol::common {

MUMBLE_PROTOCOL_COMMON_EXPORT auto swapNetworkBytes(std::integral auto const i) {
	if constexpr (std::endian::native == std::endian::little) {
		return std::byteswap(i);
	} else {
		return i;
	}
}

template<typename T>
	requires(std::integral<T>)
MUMBLE_PROTOCOL_COMMON_EXPORT T readIntegerFromNetworkBuffer(std::span<const std::byte, sizeof(T)> buffer) {
	T result;
	std::memcpy(&result, buffer.data(), sizeof(T));

	return swapNetworkBytes(result);
}

MUMBLE_PROTOCOL_COMMON_EXPORT std::size_t writeIntegerToNetworkBuffer(std::integral auto const value,
																	  std::span<std::byte, sizeof(value)> buffer) {
	auto temp = swapNetworkBytes(value);
	std::memcpy(buffer.data(), &temp, sizeof(value));
	return sizeof(value);
}

MUMBLE_PROTOCOL_COMMON_EXPORT std::tuple<std::size_t, std::int64_t>
decodeVariableInteger(std::span<const std::byte> buffer);

MUMBLE_PROTOCOL_COMMON_EXPORT std::size_t encodeVariableInteger(std::span<std::byte> buffer, std::int64_t value);

MUMBLE_PROTOCOL_COMMON_EXPORT std::tuple<PacketType, std::span<const std::byte>>
	parseNetworkPaket(std::span<const std::byte, maxPacketLength>);

MUMBLE_PROTOCOL_COMMON_EXPORT void serializeProtocolMessage(std::span<std::byte, maxPacketLength>, PacketType,
															google::protobuf::MessageLite &);

}// namespace libmumble_protocol::common
