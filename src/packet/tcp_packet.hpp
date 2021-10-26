//
// Created by jan on 23.09.20.
//

#ifndef LIBMUMBLE_CLIENT_TCP_PACKET_HPP
#define LIBMUMBLE_CLIENT_TCP_PACKET_HPP

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <version>

#if __has_include(<span>)
#include <span>
using std::span;
#else

#include <gsl/span>

using gsl::span;
#endif

#if __has_include(<experimental/propagate_const>)

#include <experimental/propagate_const>

#endif

#include "mumble_client_export.h"

namespace mumble_client::packet::tcp {

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
	enum struct MUMBLE_CLIENT_EXPORT packet_type : uint16_t
	{
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

	struct MUMBLE_CLIENT_EXPORT header
	{
		static header parse(span<std::byte>);

		header(packet_type type, uint32_t packet_length);

		const packet_type type;
		const uint32_t packet_length;
	};

	class MUMBLE_CLIENT_EXPORT version
	{
	 public:
		static constexpr packet_type type = packet_type::Version;

		[[nodiscard]] uint32_t numeric_version() const;

		[[nodiscard]] std::string_view release() const;

		[[nodiscard]] std::string_view os() const;

		[[nodiscard]] std::string_view os_version() const;

		version();

	private:
		struct impl;
#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
#else
		std::unique_ptr<impl> pImpl;
#endif
	};

	class MUMBLE_CLIENT_EXPORT authenticate
	{
	 public:
		static constexpr packet_type type = packet_type::Authenticate;

		[[nodiscard]] std::string_view username() const;

	 private:
		struct impl;
#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
#else
		std::unique_ptr<impl> pImpl;
#endif
	};

	class MUMBLE_CLIENT_EXPORT acl
	{
	 public:
		static constexpr packet_type type = packet_type::ACL;

		[[nodiscard]] uint32_t channel_id() const;

	 private:
		struct impl;
//#if __cpp_lib_experimental_propagate_const >= 201505L
#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
#else
		std::unique_ptr<impl> pImpl;
#endif
	};
}

#endif //LIBMUMBLE_CLIENT_TCP_PACKET_HPP
