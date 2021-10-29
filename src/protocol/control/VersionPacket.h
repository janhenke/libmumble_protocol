//
// Created by jan on 28.10.21.
//

#ifndef LIBMUMBLE_CLIENT_VERSIONPACKET_H
#define LIBMUMBLE_CLIENT_VERSIONPACKET_H

#include "declarations.h"

#include <cstdint>
#include <memory>
#include <string>

#if __has_include(<experimental/propagate_const>)

#include <experimental/propagate_const>
#include <utility>

#endif

namespace mumble_client::protocol::control {
struct VersionPacket {
	static constexpr PacketType packet_type = PacketType::Version;

	const uint32_t numeric_version;
	const std::string release;
	const std::string os;
	const std::string os_version;

	VersionPacket(const uint32_t numeric_version, std::string release, std::string os, std::string os_version)
		: numeric_version(numeric_version), release(std::move(release)), os(std::move(os)),
		  os_version(std::move(os_version)) {}
};
}// namespace mumble_client::protocol::control

#endif//LIBMUMBLE_CLIENT_VERSIONPACKET_H
