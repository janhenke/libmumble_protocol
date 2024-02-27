//
// Created by jan on 31.12.23.
//

#pragma once

#include "mumble_protocol_common_export.h"

#include "Mumble.pb.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace libmumble_protocol::common {

MUMBLE_PROTOCOL_COMMON_EXPORT struct Header {
	const std::uint16_t packet_type;
	const std::uint32_t payload_length;
};

/**
 * Maximum length of the payload part of the packet according to the specification.
 */
MUMBLE_PROTOCOL_COMMON_EXPORT constexpr std::size_t kMaxPayloadLength = 8 * 1024 * 1024 - 1;

/**
 * Length of the packet header.
 */
MUMBLE_PROTOCOL_COMMON_EXPORT constexpr std::size_t kHeaderLength = 2 + 4;

/**
 * Maximum length of the entire packet (header + payload).
 */
MUMBLE_PROTOCOL_COMMON_EXPORT constexpr std::size_t kMaxPacketLength = kHeaderLength + kMaxPayloadLength;

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

MUMBLE_PROTOCOL_COMMON_EXPORT std::tuple<PacketType, std::span<const std::byte>>
	parseNetworkBuffer(std::span<const std::byte, kMaxPacketLength>);

class MUMBLE_PROTOCOL_COMMON_EXPORT MumbleControlPacket {
   public:
	virtual ~MumbleControlPacket() = default;

	[[nodiscard]] std::vector<std::byte> serialize() const;

	[[nodiscard]] std::string debugString() const;

   protected:
	[[nodiscard]] virtual PacketType packetType() const = 0;

	[[nodiscard]] virtual google::protobuf::Message const &message() const = 0;
};

struct MumbleNumericVersion {
	const std::uint16_t major;
	const std::uint8_t minor;
	const std::uint8_t patch;

	explicit MumbleNumericVersion(const std::uint32_t numeric_version)
		: major(((numeric_version >> 24) & 0xff) | ((numeric_version >> 16) & 0xff)),
		  minor((numeric_version >> 8) & 0xff), patch(numeric_version & 0xff){};

	MumbleNumericVersion(const std::uint16_t major, const std::uint8_t minor, const std::uint8_t patch)
		: major(major), minor(minor), patch(patch){};

	explicit operator std::uint32_t() const {
		return (major >> 8 & 0xff) << 24 | (major & 0xff) << 16 | minor << 8 | patch;
	}
};

struct MUMBLE_PROTOCOL_COMMON_EXPORT MumbleVersionPacket : public MumbleControlPacket {
	MumbleVersionPacket(std::uint16_t majorVersion, std::uint8_t minorVersion, std::uint8_t patchVersion,
						std::string_view release, std::string_view operatingSystem,
						std::string_view operatingSystemVersion);

	explicit MumbleVersionPacket(std::span<const std::byte>);

	~MumbleVersionPacket() override = default;

	std::uint16_t majorVersion() const { return m_numericVersion.major; }

	std::uint8_t minorVersion() const { return m_numericVersion.minor; }

	std::uint8_t patchVersion() const { return m_numericVersion.patch; }

	std::string_view release() const { return m_version.release(); }

	std::string_view operatingSystem() const { return m_version.os(); }

	std::string_view operatingSystemVersion() const { return m_version.os_version(); }

   protected:
	PacketType packetType() const override;

	google::protobuf::Message const &message() const override;

   private:
	MumbleProto::Version m_version;
	MumbleNumericVersion m_numericVersion;
};

struct MUMBLE_PROTOCOL_COMMON_EXPORT MumbleAuthenticatePacket : public MumbleControlPacket {
	MumbleAuthenticatePacket(std::string_view username, std::string_view password,
							 const std::vector<std::string_view> &tokens, const std::vector<std::int32_t> &celtVersions,
							 bool opusSupported);

	explicit MumbleAuthenticatePacket(std::span<const std::byte>);

	~MumbleAuthenticatePacket() override = default;

	std::string_view username() const { return m_authenticate.username(); }

	std::string_view password() const { return m_authenticate.password(); }

	std::vector<std::string_view> tokens() const {
		std::vector<std::string_view> result;
		result.reserve(m_authenticate.tokens_size());
		for (const auto &token : m_authenticate.tokens()) { result.emplace_back(token); }
		return result;
	};

	std::vector<std::int32_t> celtVersions() const {
		std::vector<std::int32_t> result;
		result.reserve(m_authenticate.celt_versions_size());
		for (const auto celtVersion : m_authenticate.celt_versions()) { result.push_back(celtVersion); }
		return result;
	}

	bool opusSupported() const { return m_authenticate.opus(); }

   protected:
	PacketType packetType() const override;

	google::protobuf::Message const &message() const override;

   private:
	MumbleProto::Authenticate m_authenticate;
};

struct MUMBLE_PROTOCOL_COMMON_EXPORT MumblePingPacket : public MumbleControlPacket {
	explicit MumblePingPacket(std::uint64_t);

	MumblePingPacket(std::uint64_t timestamp, std::uint32_t good, std::uint32_t late, std::uint32_t lost,
					 std::uint32_t reSync, std::uint32_t udpPackets, std::uint32_t tcpPackets, float udpPingAverage,
					 float udpPingVariation, float tcpPingAverage, float tcpPingVariation);

	explicit MumblePingPacket(std::span<const std::byte>);

   protected:
	PacketType packetType() const override;

	const google::protobuf::Message &message() const override;

   private:
	MumbleProto::Ping m_ping;
};

struct MumbleCryptographySetupPacket : public MumbleControlPacket {
	MumbleCryptographySetupPacket(std::span<const std::byte> &key, std::span<const std::byte> &clientNonce,
								  std::span<const std::byte> &serverNonce);

	explicit MumbleCryptographySetupPacket(std::span<const std::byte>);

	std::span<const std::byte> key() const { return as_bytes(std::span(m_cryptSetup.key())); }

	std::span<const std::byte> clientNonce() const { return as_bytes(std::span(m_cryptSetup.client_nonce())); }

	std::span<const std::byte> serverNonce() const { return as_bytes(std::span(m_cryptSetup.server_nonce())); }

   protected:
	PacketType packetType() const override;

	const google::protobuf::Message &message() const override;

   private:
	MumbleProto::CryptSetup m_cryptSetup;
};

}// namespace libmumble_protocol::common
