//
// Created by jan on 01.01.24.
//

#include "packet.hpp"
#include "util.hpp"

#include <cstring>
#include <memory>
#include <ranges>
#include <string>

#include <spdlog/spdlog.h>

namespace libmumble_protocol {

std::tuple<PacketType, std::span<const std::byte> >
ParseNetworkBuffer(std::span<const std::byte, kMaxPacketLength> buffer) {

	std::uint16_t raw_packet_type;
	std::uint32_t payload_length;

	std::memcpy(&raw_packet_type, buffer.data(), sizeof(raw_packet_type));
	std::memcpy(&payload_length, buffer.data() + sizeof(raw_packet_type), sizeof(payload_length));

	return {static_cast<PacketType>(SwapNetworkBytes(raw_packet_type)),
	        buffer.subspan(kHeaderLength, SwapNetworkBytes(payload_length))};
}

std::size_t MumbleControlPacket::Serialize(std::span<std::byte, kMaxPacketLength> buffer) const {

	const auto& message = this->Message();
	const std::size_t payload_bytes = message.ByteSizeLong();
	const size_t total_length = kHeaderLength + payload_bytes;
	const auto packet_type = SwapNetworkBytes(std::to_underlying(this->PacketType()));
	const auto payload_length = SwapNetworkBytes(static_cast<uint32_t>(payload_bytes));

	std::byte* data = buffer.data();
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
google::protobuf::Message const& MumbleVersionPacket::Message() const { return version_; }

/*
 * Mumble authenticate packet (ID 2)
 */
MumbleAuthenticatePacket::MumbleAuthenticatePacket(std::string_view username, std::string_view password,
                                                   const std::vector<std::string_view>& tokens) {
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
google::protobuf::Message const& MumbleAuthenticatePacket::Message() const { return authenticate_; }

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
const google::protobuf::Message& MumblePingPacket::Message() const { return ping_; }

/*
 * Mumble crypt setup packet (ID 15)
 */

MumbleCryptographySetupPacket::MumbleCryptographySetupPacket(std::span<const std::byte>& key,
                                                             std::span<const std::byte>& client_nonce,
                                                             std::span<const std::byte>& server_nonce) {
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
const google::protobuf::Message& MumbleCryptographySetupPacket::Message() const { return cryptSetup_; }

} // namespace libmumble_protocol
