//
// Created by jan on 01.01.24.
//

#include "packet.hpp"
#include "util.hpp"

#include <cstring>
#include <memory>
#include <ranges>
#include <string>

namespace libmumble_protocol::common {

std::tuple<PacketType, std::span<const std::byte>>
ParseNetworkBuffer(std::span<const std::byte, kMaxPacketLength> buffer) {

	const auto header = std::bit_cast<Header>(buffer.subspan<kHeaderLength>());

	const auto packetType = static_cast<PacketType>(SwapNetworkBytes(header.packet_type));
	const auto payloadLength = SwapNetworkBytes(header.payload_length);

	return {packetType, buffer.subspan(kHeaderLength, payloadLength)};
}

std::vector<std::byte> MumbleControlPacket::Serialize() const {

	const auto &message = this->message();
	const std::size_t payloadBytes = message.ByteSizeLong();
	const Header header{SwapNetworkBytes(std::to_underlying(this->packetType())),
						static_cast<uint32_t>(SwapNetworkBytes(payloadBytes))};

	std::vector<std::byte> buffer(kHeaderLength + payloadBytes);
	std::byte *data = buffer.data();
	std::memcpy(data, &header, kHeaderLength);
	message.SerializeToArray(data + kHeaderLength, static_cast<int>(payloadBytes));

	return buffer;
}

std::string MumbleControlPacket::debugString() const { return message().DebugString(); }

/*
 * Mumble version packet (ID 0)
 */

MumbleVersionPacket::MumbleVersionPacket(const std::span<const std::byte> buffer) : version_(), numericVersion_(0) {
	const auto bufferSize = std::size(buffer);
	version_.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
	// replace instance with the value from the package, cannot assign because of const members
	std::construct_at(&numericVersion_, version_.version_v1());
}

MumbleVersionPacket::MumbleVersionPacket(const std::uint16_t majorVersion, const std::uint8_t minorVersion,
										 const std::uint8_t patchVersion, const std::string_view release,
										 const std::string_view operatingSystem,
										 const std::string_view operatingSystemVersion)
	: version_(), numericVersion_(majorVersion, minorVersion, patchVersion) {
	version_.set_version_v1(static_cast<std::uint32_t>(numericVersion_));
	version_.set_release(std::string(release));
	version_.set_os(std::string(operatingSystem));
	version_.set_os_version(std::string(operatingSystemVersion));
}

PacketType MumbleVersionPacket::packetType() const { return PacketType::Version; }
google::protobuf::Message const &MumbleVersionPacket::message() const { return version_; }

/*
 * Mumble authenticate packet (ID 2)
 */

MumbleAuthenticatePacket::MumbleAuthenticatePacket(std::string_view username, std::string_view password,
												   const std::vector<std::string_view> &tokens,
												   const std::vector<std::int32_t> &celtVersions, bool opusSupported) {
	authenticate_.set_username(std::string(username));
	authenticate_.set_password(std::string(password));
	for (const auto [index, token] : std::views::enumerate(tokens)) {
		authenticate_.set_tokens(static_cast<int>(index), std::string(token));
	}
	for (const auto [index, celtVersion] : std::views::enumerate(celtVersions)) {
		authenticate_.set_celt_versions(static_cast<int>(index), celtVersion);
	}
	authenticate_.set_opus(opusSupported);
}

MumbleAuthenticatePacket::MumbleAuthenticatePacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	authenticate_.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}

PacketType MumbleAuthenticatePacket::packetType() const { return PacketType::Authenticate; }
google::protobuf::Message const &MumbleAuthenticatePacket::message() const { return authenticate_; }

/*
 * Mumble ping packet (ID 3)
 */

MumblePingPacket::MumblePingPacket(std::uint64_t timestamp) { ping_.set_timestamp(timestamp); }

MumblePingPacket::MumblePingPacket(std::uint64_t timestamp, std::uint32_t good, std::uint32_t late, std::uint32_t lost,
								   std::uint32_t reSync, std::uint32_t udpPackets, std::uint32_t tcpPackets,
								   float udpPingAverage, float udpPingVariation, float tcpPingAverage,
								   float tcpPingVariation) {
	ping_.set_timestamp(timestamp);
	ping_.set_good(good);
	ping_.set_late(late);
	ping_.set_lost(lost);
	ping_.set_resync(reSync);
	ping_.set_udp_packets(udpPackets);
	ping_.set_tcp_packets(tcpPackets);
	ping_.set_udp_ping_avg(udpPingAverage);
	ping_.set_udp_ping_var(udpPingVariation);
	ping_.set_tcp_ping_avg(tcpPingAverage);
	ping_.set_tcp_ping_var(tcpPingVariation);
}

MumblePingPacket::MumblePingPacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	ping_.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}
PacketType MumblePingPacket::packetType() const { return PacketType::Ping; }
const google::protobuf::Message &MumblePingPacket::message() const { return ping_; }

/*
 * Mumble crypt setup packet (ID 15)
 */

MumbleCryptographySetupPacket::MumbleCryptographySetupPacket(std::span<const std::byte> &key,
															 std::span<const std::byte> &clientNonce,
															 std::span<const std::byte> &serverNonce) {
	if (!key.empty()) {
		std::string container;
		const auto count = std::size(key);
		container.reserve(count);
		std::memcpy(container.data(), key.data(), count);
		cryptSetup_.set_key(container);
	}
	if (!clientNonce.empty()) {
		std::string container;
		const auto count = std::size(clientNonce);
		container.reserve(count);
		std::memcpy(container.data(), clientNonce.data(), count);
		cryptSetup_.set_key(container);
	}
	if (!serverNonce.empty()) {
		std::string container;
		const auto count = std::size(serverNonce);
		container.reserve(count);
		std::memcpy(container.data(), serverNonce.data(), count);
		cryptSetup_.set_key(container);
	}
}

MumbleCryptographySetupPacket::MumbleCryptographySetupPacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	cryptSetup_.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}

PacketType MumbleCryptographySetupPacket::packetType() const { return PacketType::CryptSetup; }
const google::protobuf::Message &MumbleCryptographySetupPacket::message() const { return cryptSetup_; }

}// namespace libmumble_protocol::common
