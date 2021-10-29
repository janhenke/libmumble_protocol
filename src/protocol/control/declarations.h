//
// Created by jan on 28.10.21.
//

#ifndef _DECLARATIONS_H_
#define _DECLARATIONS_H_

#include "mumble_client_export.h"

#include <cstdint>

namespace mumble_client::protocol::control {
/**
	  * Maximum length of the payload part of the packet according to the specification.
	  */
constexpr std::size_t MaxPayloadLength = 8 * 1024 * 1024 - 1;

/**
	  * Length of the packet header.
	  */
constexpr std::size_t HeaderLength = 2 + 4;

/**
	  * Maximum length of the entire packet (header + payload).
	  */
constexpr std::size_t MaxPacketLength = HeaderLength + MaxPayloadLength;

/**
	  * All defined packet types.
	  */
enum struct MUMBLE_CLIENT_EXPORT PacketType : uint16_t {
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
}// namespace mumble_client::protocol::control

#endif//_DECLARATIONS_H_