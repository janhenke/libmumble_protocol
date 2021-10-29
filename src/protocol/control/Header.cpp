//
// Created by jan on 26.10.21.
//

#include "Header.h"

namespace mumble_client::protocol::control {
Header::Header(const std::span<std::byte> buffer)
	: packet_type(static_cast<const PacketType>(std::to_integer<uint16_t>(buffer[0]) << 8
												| std::to_integer<uint16_t>(buffer[1]))),
	  packet_length(std::to_integer<uint32_t>(buffer[2]) << 24 | std::to_integer<uint32_t>(buffer[3]) << 16
					| std::to_integer<uint32_t>(buffer[4]) << 8 | std::to_integer<uint32_t>(buffer[5])) {}

Header::Header(const PacketType packet_type, const uint32_t packet_length)
	: packet_type(packet_type), packet_length(packet_length) {}

void Header::Write(std::span<std::byte> buffer){mumble_client::protocol::};
}// namespace mumble_client::protocol::control
