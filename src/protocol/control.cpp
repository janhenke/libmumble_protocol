//
// Created by jan on 12.05.20.
//

#include "control.hpp"
#include "Mumble.pb.h"

namespace mumble_client::protocol::control {

	struct acl::impl {
		MumbleProto::ACL mumble_packet;
	};
}