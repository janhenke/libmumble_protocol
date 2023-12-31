//
// Created by jan on 31.12.23.
//

#pragma once

#include "mumble_protocol_common_export.h"

#include <cstddef>
#include <cstdint>

namespace libmumble_protocol::common {

/**
 * Maximum length of the payload part of the packet according to the specification.
 */
MUMBLE_PROTOCOL_COMMON_EXPORT constexpr std::size_t maxPayloadLength = 8 * 1024 * 1024 - 1;

/**
 * Length of the packet header.
 */
MUMBLE_PROTOCOL_COMMON_EXPORT constexpr std::size_t headerLength = 2 + 4;

/**
 * Maximum length of the entire packet (header + payload).
 */
MUMBLE_PROTOCOL_COMMON_EXPORT constexpr std::size_t maxPacketLength = headerLength + maxPayloadLength;

/**
 * All defined packet types.
 */
enum struct MUMBLE_PROTOCOL_COMMON_EXPORT PacketType : uint16_t {
	Version = 0,
	UDPTunnel = 1,
	Authenticate = 2,
	Ping = 3,
	Reject = 4,
	ServerSync = 5,
	ChannelRemove = 6,
	ChannelState = 7,
	UserRemove = 8,
	UserState = 9,
	BanList = 10,
	TextMessage = 11,
	PermissionDenied = 12,
	ACL = 13,
	QueryUsers = 14,
	CryptSetup = 15,
	ContextActionModify = 16,
	ContextAction = 17,
	UserList = 18,
	VoiceTarget = 19,
	PermissionQuery = 20,
	CodecVersion = 21,
	UserStats = 22,
	RequestBlob = 23,
	ServerConfig = 24,
	SuggestConfig = 25
};

}// namespace libmumble_protocol::common
