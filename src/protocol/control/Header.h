//
// Created by jan on 26.10.21.
//

#ifndef _HEADER_H_
#define _HEADER_H_

#include "declarations.h"

#include <cstdint>
#include <span>

namespace mumble_client::protocol::control {
struct Header {
	static constexpr std::size_t length = HeaderLength;

	explicit Header(std::span<std::byte>);
	Header(PacketType packet_type, uint32_t packet_length);

	const PacketType packet_type;
	const uint32_t packet_length;

	void Write(std::span<std::byte> buffer) const;
};
}// namespace mumble_client::protocol::control

#endif//_HEADER_H_