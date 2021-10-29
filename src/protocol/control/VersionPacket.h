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

	struct Version {
		const uint16_t major;
		const uint8_t minor;
		const uint8_t patch;

		explicit Version(const uint32_t numeric_version)
			: major(((numeric_version >> 24) & 0xff) | ((numeric_version >> 16) & 0xff)),
			  minor((numeric_version >> 8) & 0xff), patch(numeric_version & 0xff){};

		Version(const uint16_t major, const uint8_t minor, const uint8_t patch)
			: major(major), minor(minor), patch(patch){};

		explicit operator uint32_t() const {
			return (major >> 8 & 0xff) << 24 | (major & 0xff) << 16 | minor << 8 | patch;
		}
	};

	const Version version;
	const std::string release;
	const std::string os;
	const std::string os_version;

	VersionPacket(const uint32_t numeric_version, std::string release, std::string os, std::string os_version)
		: version(numeric_version), release(std::move(release)), os(std::move(os)), os_version(std::move(os_version)) {}
};
}// namespace mumble_client::protocol::control

#endif//LIBMUMBLE_CLIENT_VERSIONPACKET_H
