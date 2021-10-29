//
// Created by jan on 26.10.21.
//

#include "Header.h"

#include <protocol/util.h>

namespace mumble_client::protocol::control {
Header::Header(const std::span<std::byte> buffer)
	: packet_type(static_cast<const PacketType>(std::to_integer<uint16_t>(buffer[0]) << 8
												| std::to_integer<uint16_t>(buffer[1]))),
	  packet_length(std::to_integer<uint32_t>(buffer[2]) << 24 | std::to_integer<uint32_t>(buffer[3]) << 16
					| std::to_integer<uint32_t>(buffer[4]) << 8 | std::to_integer<uint32_t>(buffer[5])) {}

Header::Header(const PacketType packet_type, const uint32_t packet_length)
	: packet_type(packet_type), packet_length(packet_length) {}

void Header::Write(std::span<std::byte> buffer) const {
	mumble_client::protocol::assert_min_buffer_size(buffer, HeaderLength);

	const auto numeric_packet_type = static_cast<uint16_t>(packet_type);
	buffer[0] = std::byte(numeric_packet_type >> 8);
	buffer[1] = std::byte(numeric_packet_type);

	buffer[2] = std::byte(packet_length >> 24);
	buffer[3] = std::byte(packet_length >> 16);
	buffer[4] = std::byte(packet_length >> 8);
	buffer[5] = std::byte(packet_length);
};
}// namespace mumble_client::protocol::control
