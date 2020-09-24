//
// Created by jan on 23.09.20.
//

#include "tcp_packet.hpp"
#include "Mumble.pb.h"

namespace mumble_client::packet::tcp {

	struct version::impl {
		MumbleProto::Version mumble_packet;
	};

	uint32_t version::numeric_version() const {
		return pImpl->mumble_packet.has_version() ? pImpl->mumble_packet.version() : 0;
	}

	std::string_view version::release() const {
		return pImpl->mumble_packet.has_release() ? pImpl->mumble_packet.release() : std::string_view();
	}

	std::string_view version::os() const {
		return pImpl->mumble_packet.has_os() ? pImpl->mumble_packet.os() : std::string_view();
	}

	std::string_view version::os_version() const {
		return pImpl->mumble_packet.has_os_version() ? pImpl->mumble_packet.os_version() : std::string_view();
	}

	version::version() = default;

	struct authenticate::impl {
		MumbleProto::Authenticate mumble_packet;
	};

	std::string_view authenticate::username() const {
		return pImpl->mumble_packet.has_username() ? pImpl->mumble_packet.username() : std::string_view();
	}

	struct acl::impl {
		MumbleProto::ACL mumble_packet;
	};

	uint32_t acl::channel_id() const {
		return pImpl->mumble_packet.channel_id();
	}
}
