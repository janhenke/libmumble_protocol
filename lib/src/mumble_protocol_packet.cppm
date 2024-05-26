module;

#include "Mumble.pb.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <spdlog/spdlog.h>

// TODO: Make this partition internal, for now there are build failures if it is not exported
export module mumble_protocol:packet;

import :utility;

namespace libmumble_protocol::common {

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
enum struct PacketType : uint16_t {
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

auto AssertProtbufferRuntimeVersion() -> void {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}

auto ParseNetworkBuffer(std::span<const std::byte, kMaxPacketLength>)
	-> std::tuple<PacketType, std::span<const std::byte>>;

class MumbleControlPacket {
   public:
	virtual ~MumbleControlPacket() = default;

	[[nodiscard]] auto Serialize(std::span<std::byte, kMaxPacketLength>) const -> std::size_t;

	[[nodiscard]] auto DebugString() const -> std::string;

   protected:
	[[nodiscard]] virtual auto PacketType() const -> enum PacketType = 0;

	[[nodiscard]] virtual auto Message() const -> google::protobuf::Message const & = 0;
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
							static_cast<std::uint32_t>(std::numeric_limits<std::uint8_t>::max()))
				   << 8);
		result |= (std::min(static_cast<std::uint32_t>(patch_),
							static_cast<std::uint32_t>(std::numeric_limits<std::uint8_t>::max())));
		return result;
	}

	[[nodiscard]] auto major() const -> std::uint16_t { return major_; }
	[[nodiscard]] auto minor() const -> std::uint16_t { return minor_; }
	[[nodiscard]] auto patch() const -> std::uint16_t { return patch_; }

	void parse(const std::uint64_t version) {
		major_ = static_cast<std::uint16_t>((version & 0xffff'0000'0000'0000) >> 48);
		minor_ = static_cast<std::uint16_t>((version & 0xffff'0000'0000) >> 32);
		patch_ = static_cast<std::uint16_t>((version & 0xffff'0000) >> 16);
	}
};

class MumbleVersionPacket : public MumbleControlPacket {
   public:
	MumbleVersionPacket(MumbleVersion mumble_version, std::string_view release, std::string_view operating_system,
						std::string_view operating_system_version);

	explicit MumbleVersionPacket(std::span<const std::byte>);

	~MumbleVersionPacket() override = default;

	[[nodiscard]] auto majorVersion() const -> std::uint16_t { return mumbleVersion_.major(); }

	[[nodiscard]] auto minorVersion() const -> std::uint16_t { return mumbleVersion_.minor(); }

	[[nodiscard]] auto patchVersion() const -> std::uint16_t { return mumbleVersion_.patch(); }

	[[nodiscard]] auto release() const -> std::string_view { return version_.release(); }

	[[nodiscard]] auto operatingSystem() const -> std::string_view { return version_.os(); }

	[[nodiscard]] auto operatingSystemVersion() const -> std::string_view { return version_.os_version(); }

   protected:
	[[nodiscard]] auto PacketType() const -> enum PacketType override;

	[[nodiscard]] auto Message() const -> google::protobuf::Message const & override;

   private:
	MumbleProto::Version version_;
	MumbleVersion mumbleVersion_;
};

class MumbleAuthenticatePacket : public MumbleControlPacket {
   public:
	MumbleAuthenticatePacket(std::string_view username, std::string_view password,
							 const std::vector<std::string_view> &tokens);

	explicit MumbleAuthenticatePacket(std::span<const std::byte>);

	~MumbleAuthenticatePacket() override = default;

	[[nodiscard]] auto username() const -> std::string_view { return authenticate_.username(); }

	[[nodiscard]] auto password() const -> std::string_view { return authenticate_.password(); }

	[[nodiscard]] auto tokens() const -> std::vector<std::string_view> {
		std::vector<std::string_view> result;
		result.reserve(authenticate_.tokens_size());
		for (const auto &token : authenticate_.tokens()) { result.emplace_back(token); }
		return result;
	};

	[[nodiscard]] auto celtVersions() const -> std::vector<std::int32_t> {
		std::vector<std::int32_t> result;
		result.reserve(authenticate_.celt_versions_size());
		for (const auto celtVersion : authenticate_.celt_versions()) { result.push_back(celtVersion); }
		return result;
	}

	[[nodiscard]] auto opusSupported() const -> bool { return authenticate_.opus(); }

   protected:
	[[nodiscard]] auto PacketType() const -> enum PacketType override;

	[[nodiscard]] auto Message() const -> google::protobuf::Message const & override;

   private:
	MumbleProto::Authenticate authenticate_;
};

class MumblePingPacket : public MumbleControlPacket {
   public:
	explicit MumblePingPacket(std::uint64_t);

	MumblePingPacket(std::uint64_t timestamp, std::uint32_t good, std::uint32_t late, std::uint32_t lost,
					 std::uint32_t re_sync, std::uint32_t udp_packets, std::uint32_t tcp_packets,
					 float udp_ping_average, float udp_ping_variation, float tcp_ping_average,
					 float tcp_ping_variation);

	explicit MumblePingPacket(std::span<const std::byte>);

   protected:
	[[nodiscard]] auto PacketType() const -> enum PacketType override;

	[[nodiscard]] auto Message() const -> google::protobuf::Message const & override;

   private:
	MumbleProto::Ping ping_;
};

class MumbleCryptographySetupPacket : public MumbleControlPacket {
   public:
	MumbleCryptographySetupPacket(std::span<const std::byte> &key, std::span<const std::byte> &client_nonce,
								  std::span<const std::byte> &server_nonce);

	explicit MumbleCryptographySetupPacket(std::span<const std::byte>);

	[[nodiscard]] auto key() const -> std::span<const std::byte> { return as_bytes(std::span(cryptSetup_.key())); }

	[[nodiscard]] auto clientNonce() const -> std::span<const std::byte> {
		return as_bytes(std::span(cryptSetup_.client_nonce()));
	}

	[[nodiscard]] auto serverNonce() const -> std::span<const std::byte> {
		return as_bytes(std::span(cryptSetup_.server_nonce()));
	}

   protected:
	[[nodiscard]] auto PacketType() const -> enum PacketType override;

	[[nodiscard]] auto Message() const -> google::protobuf::Message const & override;

   private:
	MumbleProto::CryptSetup cryptSetup_;
};

}// namespace libmumble_protocol::common

template<>
struct std::formatter<libmumble_protocol::common::PacketType> : public std::formatter<std::uint16_t> {
	auto format(const libmumble_protocol::common::PacketType &packet_type, std::format_context &ctx) const {
		return std::formatter<std::uint16_t>::format(static_cast<uint16_t>(packet_type), ctx);
	}
};

namespace libmumble_protocol::common {

std::tuple<PacketType, std::span<const std::byte>>
ParseNetworkBuffer(std::span<const std::byte, kMaxPacketLength> buffer) {

	std::uint16_t raw_packet_type;
	std::uint32_t payload_length;

	std::memcpy(&raw_packet_type, buffer.data(), sizeof(raw_packet_type));
	std::memcpy(&payload_length, buffer.data() + sizeof(raw_packet_type), sizeof(payload_length));

	return {static_cast<PacketType>(SwapNetworkBytes(raw_packet_type)),
			buffer.subspan(kHeaderLength, SwapNetworkBytes(payload_length))};
}

std::size_t MumbleControlPacket::Serialize(std::span<std::byte, kMaxPacketLength> buffer) const {

	const auto &message = this->Message();
	const std::size_t payload_bytes = message.ByteSizeLong();
	const size_t total_length = kHeaderLength + payload_bytes;
	const auto packet_type = SwapNetworkBytes(std::to_underlying(this->PacketType()));
	const auto payload_length = SwapNetworkBytes(static_cast<uint32_t>(payload_bytes));

	std::byte *data = buffer.data();
	std::memcpy(data, &packet_type, sizeof(packet_type));
	std::memcpy(data + sizeof(packet_type), &payload_length, sizeof(payload_length));
	message.SerializeToArray(data + kHeaderLength, static_cast<int>(payload_bytes));

	return total_length;
}

std::string MumbleControlPacket::DebugString() const { return Message().DebugString(); }

/*
 * Mumble version_ packet (ID 0)
 */

MumbleVersionPacket::MumbleVersionPacket(const std::span<const std::byte> buffer) : version_(), mumbleVersion_() {
	const auto bufferSize = std::size(buffer);
	version_.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
	mumbleVersion_.parse(version_.version_v2());
}

MumbleVersionPacket::MumbleVersionPacket(const MumbleVersion mumble_version, const std::string_view release,
										 const std::string_view operating_system,
										 const std::string_view operating_system_version)
	: version_(), mumbleVersion_(mumble_version) {

	version_.set_version_v1(static_cast<std::uint32_t>(mumbleVersion_));
	version_.set_version_v2(static_cast<std::uint64_t>(mumbleVersion_));
	version_.set_release(std::string(release));
	version_.set_os(std::string(operating_system));
	version_.set_os_version(std::string(operating_system_version));
}

PacketType MumbleVersionPacket::PacketType() const { return PacketType::Version; }
google::protobuf::Message const &MumbleVersionPacket::Message() const { return version_; }

/*
 * Mumble authenticate packet (ID 2)
 */
MumbleAuthenticatePacket::MumbleAuthenticatePacket(std::string_view username, std::string_view password,
												   const std::vector<std::string_view> &tokens) {
	authenticate_.set_username(std::string(username));
	authenticate_.set_password(std::string(password));
	for (const auto [index, token] : std::views::enumerate(tokens)) {
		authenticate_.set_tokens(static_cast<int>(index), std::string(token));
	}
	// only opus is supported
	// not setting any supported CELT versions
	authenticate_.set_opus(true);
}

MumbleAuthenticatePacket::MumbleAuthenticatePacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	authenticate_.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}

PacketType MumbleAuthenticatePacket::PacketType() const { return PacketType::Authenticate; }
google::protobuf::Message const &MumbleAuthenticatePacket::Message() const { return authenticate_; }

/*
 * Mumble ping packet (ID 3)
 */

MumblePingPacket::MumblePingPacket(std::uint64_t timestamp) { ping_.set_timestamp(timestamp); }

MumblePingPacket::MumblePingPacket(std::uint64_t timestamp, std::uint32_t good, std::uint32_t late, std::uint32_t lost,
								   std::uint32_t re_sync, std::uint32_t udp_packets, std::uint32_t tcp_packets,
								   float udp_ping_average, float udp_ping_variation, float tcp_ping_average,
								   float tcp_ping_variation) {
	ping_.set_timestamp(timestamp);
	ping_.set_good(good);
	ping_.set_late(late);
	ping_.set_lost(lost);
	ping_.set_resync(re_sync);
	ping_.set_udp_packets(udp_packets);
	ping_.set_tcp_packets(tcp_packets);
	ping_.set_udp_ping_avg(udp_ping_average);
	ping_.set_udp_ping_var(udp_ping_variation);
	ping_.set_tcp_ping_avg(tcp_ping_average);
	ping_.set_tcp_ping_var(tcp_ping_variation);
}

MumblePingPacket::MumblePingPacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	ping_.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}
PacketType MumblePingPacket::PacketType() const { return PacketType::Ping; }
const google::protobuf::Message &MumblePingPacket::Message() const { return ping_; }

/*
 * Mumble crypt setup packet (ID 15)
 */

MumbleCryptographySetupPacket::MumbleCryptographySetupPacket(std::span<const std::byte> &key,
															 std::span<const std::byte> &client_nonce,
															 std::span<const std::byte> &server_nonce) {
	if (!key.empty()) {
		std::string container;
		const auto count = std::size(key);
		container.reserve(count);
		std::memcpy(container.data(), key.data(), count);
		cryptSetup_.set_key(container);
	}
	if (!client_nonce.empty()) {
		std::string container;
		const auto count = std::size(client_nonce);
		container.reserve(count);
		std::memcpy(container.data(), client_nonce.data(), count);
		cryptSetup_.set_key(container);
	}
	if (!server_nonce.empty()) {
		std::string container;
		const auto count = std::size(server_nonce);
		container.reserve(count);
		std::memcpy(container.data(), server_nonce.data(), count);
		cryptSetup_.set_key(container);
	}
}

MumbleCryptographySetupPacket::MumbleCryptographySetupPacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	cryptSetup_.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}

PacketType MumbleCryptographySetupPacket::PacketType() const { return PacketType::CryptSetup; }
const google::protobuf::Message &MumbleCryptographySetupPacket::Message() const { return cryptSetup_; }

}// namespace libmumble_protocol::common
