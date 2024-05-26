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
constexpr std::size_t kMaxPayloadLength = 8 * 1024 * 1024 - 1;

/**
 * Length of the packet header.
 */
constexpr std::size_t kHeaderLength = 2 + 4;

/**
 * Maximum length of the entire packet (header + payload).
 */
constexpr std::size_t kMaxPacketLength = kHeaderLength + kMaxPayloadLength;

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

MUMBLE_PROTOCOL_EXPORT auto ParseNetworkBuffer(
	std::span<const std::byte, kMaxPacketLength>) -> std::tuple<PacketType, std::span<const std::byte>>;

class MUMBLE_PROTOCOL_EXPORT MumbleControlPacket {
public:
	virtual ~MumbleControlPacket() = default;

	[[nodiscard]] auto Serialize(std::span<std::byte, kMaxPacketLength>) const -> std::size_t;

	[[nodiscard]] auto DebugString() const -> std::string;

protected:
	[[nodiscard]] virtual auto PacketType() const -> PacketType = 0;

	[[nodiscard]] virtual auto Message() const -> google::protobuf::Message const& = 0;
};

//
// The mumble version format (v2) is an uint64:
// major   minor   patch   reserved/unused
// 0xFFFF  0xFFFF  0xFFFF  0xFFFF
// (big-endian)
//
class MUMBLE_PROTOCOL_EXPORT MumbleVersion {
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

	[[nodiscard]] auto major() const { return major_; }
	[[nodiscard]] auto minor() const { return minor_; }
	[[nodiscard]] auto patch() const { return patch_; }

	void parse(const std::uint64_t version) {
		major_ = static_cast<std::uint16_t>((version & 0xffff'0000'0000'0000) >> 48);
		minor_ = static_cast<std::uint16_t>((version & 0xffff'0000'0000) >> 32);
		patch_ = static_cast<std::uint16_t>((version & 0xffff'0000) >> 16);
	}
};

class MUMBLE_PROTOCOL_EXPORT MumbleVersionPacket final : public MumbleControlPacket {
public:
	MumbleVersionPacket(MumbleVersion mumble_version, std::string_view release, std::string_view operating_system,
	                    std::string_view operating_system_version);

	explicit MumbleVersionPacket(std::span<const std::byte>);

	MumbleVersionPacket(const MumbleVersionPacket& other) = default;
	MumbleVersionPacket(MumbleVersionPacket&& other) noexcept = default;
	auto operator=(const MumbleVersionPacket& other) -> MumbleVersionPacket& = default;
	auto operator=(MumbleVersionPacket&& other) noexcept -> MumbleVersionPacket& = default;

	~MumbleVersionPacket() override = default;

	auto majorVersion() const { return mumbleVersion_.major(); }

	auto minorVersion() const { return mumbleVersion_.minor(); }

	auto patchVersion() const { return mumbleVersion_.patch(); }

	auto release() const -> std::string_view { return version_.release(); }

	auto operatingSystem() const -> std::string_view { return version_.os(); }

	auto operatingSystemVersion() const -> std::string_view { return version_.os_version(); }

protected:
	auto PacketType() const -> enum PacketType override;

	auto Message() const -> google::protobuf::Message const& override;

private:
	MumbleProto::Version version_;
	MumbleVersion mumbleVersion_;
};

class MUMBLE_PROTOCOL_EXPORT MumbleAuthenticatePacket final : public MumbleControlPacket {
public:
	MumbleAuthenticatePacket(std::string_view username, std::string_view password,
	                         const std::vector<std::string_view>& tokens);

	explicit MumbleAuthenticatePacket(std::span<const std::byte>);

	MumbleAuthenticatePacket(const MumbleAuthenticatePacket& other) = default;
	MumbleAuthenticatePacket(MumbleAuthenticatePacket&& other) noexcept = default;
	auto operator=(const MumbleAuthenticatePacket& other) -> MumbleAuthenticatePacket& = default;
	auto operator=(MumbleAuthenticatePacket&& other) noexcept -> MumbleAuthenticatePacket& = default;

	~MumbleAuthenticatePacket() override = default;

	auto username() const -> std::string_view { return authenticate_.username(); }

	auto password() const -> std::string_view { return authenticate_.password(); }

	auto tokens() const -> std::vector<std::string_view> {
		std::vector<std::string_view> result;
		result.reserve(authenticate_.tokens_size());
		for (const auto& token : authenticate_.tokens()) {
			result.emplace_back(token);
		}
		return result;
	};

	auto celtVersions() const -> std::vector<std::int32_t> {
		std::vector<std::int32_t> result;
		result.reserve(authenticate_.celt_versions_size());
		for (const auto celtVersion : authenticate_.celt_versions()) {
			result.push_back(celtVersion);
		}
		return result;
	}

	auto opusSupported() const { return authenticate_.opus(); }

protected:
	auto PacketType() const -> enum PacketType override;

	auto Message() const -> google::protobuf::Message const& override;

private:
	MumbleProto::Authenticate authenticate_;
};

class MUMBLE_PROTOCOL_EXPORT MumblePingPacket final : public MumbleControlPacket {
public:
	explicit MumblePingPacket(std::uint64_t);

	MumblePingPacket(std::uint64_t timestamp, std::uint32_t good, std::uint32_t late, std::uint32_t lost,
	                 std::uint32_t re_sync, std::uint32_t udp_packets, std::uint32_t tcp_packets,
	                 float udp_ping_average,
	                 float udp_ping_variation, float tcp_ping_average, float tcp_ping_variation);

	explicit MumblePingPacket(std::span<const std::byte>);

protected:
	auto PacketType() const -> enum PacketType override;

	auto Message() const -> const google::protobuf::Message& override;

private:
	MumbleProto::Ping ping_;
};

class MUMBLE_PROTOCOL_EXPORT MumbleCryptographySetupPacket final : public MumbleControlPacket {
public:
	MumbleCryptographySetupPacket(std::span<const std::byte>& key, std::span<const std::byte>& client_nonce,
	                              std::span<const std::byte>& server_nonce);

	explicit MumbleCryptographySetupPacket(std::span<const std::byte>);

	auto key() const -> std::span<const std::byte> {
		return as_bytes(std::span(cryptSetup_.key()));
	}

	auto clientNonce() const -> std::span<const std::byte> {
		return as_bytes(std::span(cryptSetup_.client_nonce()));
	}

	auto serverNonce() const -> std::span<const std::byte> {
		return as_bytes(std::span(cryptSetup_.server_nonce()));
	}

protected:
	auto PacketType() const -> enum PacketType override;

	auto Message() const -> const google::protobuf::Message& override;

private:
	MumbleProto::CryptSetup cryptSetup_;
};

} // namespace libmumble_protocol

template <>
struct std::formatter<libmumble_protocol::PacketType> : std::formatter<std::uint16_t> {
	auto format(const libmumble_protocol::PacketType& packet_type, std::format_context& ctx) const {
		return std::formatter<std::uint16_t>::format(static_cast<uint16_t>(packet_type), ctx);
	}
};
