//
// Created by jan on 31.12.23.
//

#pragma once

#include "mumble_protocol_export.h"

#include "Mumble.pb.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace libmumble_protocol {

/**
 * Maximum length of the payload part of the packet according to the specification.
 */
MUMBLE_PROTOCOL_EXPORT constexpr std::size_t kMaxPayloadLength = 8 * 1024 * 1024 - 1;

/**
 * Length of the packet header.
 */
MUMBLE_PROTOCOL_EXPORT constexpr std::size_t kHeaderLength = 2 + 4;

/**
 * Maximum length of the entire packet (header + payload).
 */
MUMBLE_PROTOCOL_EXPORT constexpr std::size_t kMaxPacketLength = kHeaderLength + kMaxPayloadLength;

/**
 * All defined packet types.
 */
enum struct MUMBLE_PROTOCOL_EXPORT PacketType : uint16_t {
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

MUMBLE_PROTOCOL_EXPORT std::tuple<PacketType, std::span<const std::byte> >
ParseNetworkBuffer(std::span<const std::byte, kMaxPacketLength>);

class MUMBLE_PROTOCOL_EXPORT MumbleControlPacket {
public:
	virtual ~MumbleControlPacket() = default;

	[[nodiscard]] std::size_t Serialize(std::span<std::byte, kMaxPacketLength>) const;

	[[nodiscard]] std::string DebugString() const;

protected:
	[[nodiscard]] virtual enum PacketType PacketType() const = 0;

	[[nodiscard]] virtual google::protobuf::Message const& Message() const = 0;
};

//
// The mumble version format (v2) is an uint64:
// major   minor   patch   reserved/unused
// 0xFFFF  0xFFFF  0xFFFF  0xFFFF
// (big-endian)
//
class MumbleVersion {
	std::uint16_t major_;
	std::uint16_t minor_;
	std::uint16_t patch_;

public:
	MumbleVersion() : major_(0), minor_(0), patch_(0) {}

	MumbleVersion(const std::uint16_t major, const std::uint16_t minor, const std::uint16_t patch)
		: major_(major), minor_(minor), patch_(patch) {}

	explicit operator std::uint64_t() const {
		std::uint64_t result = static_cast<std::uint64_t>(major_) << 48;
		result |= static_cast<std::uint64_t>(minor_) << 32;
		result |= static_cast<std::uint64_t>(patch_) << 16;
		return result;
	}

	//
	// Legacy versions: These versions are kept around for backward compatibility, but
	// have since been replaced by other version formats.
	//
	// Mumble legacy version format (v1) is an uint32:
	// major   minor  patch
	// 0xFFFF  0xFF   0xFF
	// (big-endian)
	//
	explicit operator std::uint32_t() const {
		std::uint32_t result = (static_cast<std::uint32_t>(major_) << 16);
		result |= (std::min(static_cast<std::uint32_t>(minor_),
		                    static_cast<std::uint32_t>(std::numeric_limits<std::uint8_t>::max())) << 8);
		result |= (std::min(static_cast<std::uint32_t>(patch_),
		                    static_cast<std::uint32_t>(std::numeric_limits<std::uint8_t>::max())));
		return result;
	}

	[[nodiscard]] std::uint16_t major() const { return major_; }
	[[nodiscard]] std::uint16_t minor() const { return minor_; }
	[[nodiscard]] std::uint16_t patch() const { return patch_; }

	void parse(const std::uint64_t version) {
		major_ = static_cast<std::uint16_t>((version & 0xffff'0000'0000'0000) >> 48);
		minor_ = static_cast<std::uint16_t>((version & 0xffff'0000'0000) >> 32);
		patch_ = static_cast<std::uint16_t>((version & 0xffff'0000) >> 16);
	}
};

class MUMBLE_PROTOCOL_EXPORT MumbleVersionPacket : public MumbleControlPacket {
public:
	MumbleVersionPacket(MumbleVersion mumble_version, std::string_view release, std::string_view operating_system,
	                    std::string_view operating_system_version);

	explicit MumbleVersionPacket(std::span<const std::byte>);

	~MumbleVersionPacket() override = default;

	std::uint16_t majorVersion() const { return mumbleVersion_.major(); }

	std::uint16_t minorVersion() const { return mumbleVersion_.minor(); }

	std::uint16_t patchVersion() const { return mumbleVersion_.patch(); }

	std::string_view release() const { return version_.release(); }

	std::string_view operatingSystem() const { return version_.os(); }

	std::string_view operatingSystemVersion() const { return version_.os_version(); }

protected:
	enum PacketType PacketType() const override;

	google::protobuf::Message const& Message() const override;

private:
	MumbleProto::Version version_;
	MumbleVersion mumbleVersion_;
};

class MUMBLE_PROTOCOL_EXPORT MumbleAuthenticatePacket : public MumbleControlPacket {
public:
	MumbleAuthenticatePacket(std::string_view username, std::string_view password,
	                         const std::vector<std::string_view>& tokens);

	explicit MumbleAuthenticatePacket(std::span<const std::byte>);

	~MumbleAuthenticatePacket() override = default;

	std::string_view username() const { return authenticate_.username(); }

	std::string_view password() const { return authenticate_.password(); }

	std::vector<std::string_view> tokens() const {
		std::vector<std::string_view> result;
		result.reserve(authenticate_.tokens_size());
		for (const auto& token : authenticate_.tokens()) { result.emplace_back(token); }
		return result;
	};

	std::vector<std::int32_t> celtVersions() const {
		std::vector<std::int32_t> result;
		result.reserve(authenticate_.celt_versions_size());
		for (const auto celtVersion : authenticate_.celt_versions()) { result.push_back(celtVersion); }
		return result;
	}

	bool opusSupported() const { return authenticate_.opus(); }

protected:
	enum PacketType PacketType() const override;

	google::protobuf::Message const& Message() const override;

private:
	MumbleProto::Authenticate authenticate_;
};

class MUMBLE_PROTOCOL_EXPORT MumblePingPacket : public MumbleControlPacket {
public:
	explicit MumblePingPacket(std::uint64_t);

	MumblePingPacket(std::uint64_t timestamp, std::uint32_t good, std::uint32_t late, std::uint32_t lost,
	                 std::uint32_t re_sync, std::uint32_t udp_packets, std::uint32_t tcp_packets,
	                 float udp_ping_average,
	                 float udp_ping_variation, float tcp_ping_average, float tcp_ping_variation);

	explicit MumblePingPacket(std::span<const std::byte>);

protected:
	enum PacketType PacketType() const override;

	const google::protobuf::Message& Message() const override;

private:
	MumbleProto::Ping ping_;
};

class MumbleCryptographySetupPacket : public MumbleControlPacket {
public:
	MumbleCryptographySetupPacket(std::span<const std::byte>& key, std::span<const std::byte>& client_nonce,
	                              std::span<const std::byte>& server_nonce);

	explicit MumbleCryptographySetupPacket(std::span<const std::byte>);

	std::span<const std::byte> key() const { return as_bytes(std::span(cryptSetup_.key())); }

	std::span<const std::byte> clientNonce() const { return as_bytes(std::span(cryptSetup_.client_nonce())); }

	std::span<const std::byte> serverNonce() const { return as_bytes(std::span(cryptSetup_.server_nonce())); }

protected:
	enum PacketType PacketType() const override;

	const google::protobuf::Message& Message() const override;

private:
	MumbleProto::CryptSetup cryptSetup_;
};

} // namespace libmumble_protocol

template <>
struct std::formatter<libmumble_protocol::PacketType> : public std::formatter<std::uint16_t> {
	auto format(const libmumble_protocol::PacketType& packet_type, std::format_context& ctx) const {
		return std::formatter<std::uint16_t>::format(static_cast<uint16_t>(packet_type), ctx);
	}
};
