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
parseNetworkBuffer(std::span<const std::byte, maxPacketLength> buffer) {

	const auto packetType = static_cast<PacketType>(readIntegerFromNetworkBuffer<std::uint16_t>(buffer.first<2>()));
	const auto payloadLength = readIntegerFromNetworkBuffer<std::uint32_t>(buffer.subspan<2, 4>());

	return {packetType, buffer.subspan(headerLength, payloadLength)};
}

std::vector<std::byte> MumbleControlPacket::serialize() const {

	const auto packetType = this->packetType();
	const auto &message = this->message();
	const auto payloadBytes = message.ByteSizeLong();

	std::vector<std::byte> buffer;
	buffer.reserve(headerLength + payloadBytes);
	auto bufferBegin = std::begin(buffer);
	writeIntegerToNetworkBuffer(std::span(bufferBegin, 2), static_cast<std::uint16_t>(packetType));
	writeIntegerToNetworkBuffer(std::span(bufferBegin + 2, 4), static_cast<std::uint32_t>(payloadBytes));

	message.SerializeToArray(buffer.data() + headerLength, static_cast<int>(payloadBytes));

	return buffer;
}

/*
 * Mumble version packet (ID 0)
 */

MumbleVersionPacket::MumbleVersionPacket(const std::span<const std::byte> buffer) : m_version(), m_numericVersion(0) {
	const auto bufferSize = std::size(buffer);
	m_version.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
	// replace instance with the value from the package, cannot assign because of const members
	std::construct_at(&m_numericVersion, m_version.version());
}

MumbleVersionPacket::MumbleVersionPacket(const std::uint16_t majorVersion, const std::uint8_t minorVersion,
										 const std::uint8_t patchVersion, const std::string_view release,
										 const std::string_view operatingSystem,
										 const std::string_view operatingSystemVersion)
	: m_version(), m_numericVersion(majorVersion, minorVersion, patchVersion) {
	m_version.set_version(static_cast<std::uint32_t>(m_numericVersion));
	m_version.set_release(std::string(release));
	m_version.set_os(std::string(operatingSystem));
	m_version.set_os_version(std::string(operatingSystemVersion));
}

PacketType MumbleVersionPacket::packetType() const { return PacketType::Version; }
google::protobuf::MessageLite const &MumbleVersionPacket::message() const { return m_version; }

/*
 * Mumble authenticate packet (ID 2)
 */

MumbleAuthenticatePacket::MumbleAuthenticatePacket(std::string_view username, std::string_view password,
												   const std::vector<std::string_view> &tokens,
												   const std::vector<std::int32_t> &celtVersions, bool opusSupported) {
	m_authenticate.set_username(std::string(username));
	m_authenticate.set_password(std::string(password));
	for (const auto [index, token] : std::views::enumerate(tokens)) {
		m_authenticate.set_tokens(static_cast<int>(index), std::string(token));
	}
	for (const auto [index, celtVersion] : std::views::enumerate(celtVersions)) {
		m_authenticate.set_celt_versions(static_cast<int>(index), celtVersion);
	}
	m_authenticate.set_opus(opusSupported);
}

MumbleAuthenticatePacket::MumbleAuthenticatePacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	m_authenticate.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}

PacketType MumbleAuthenticatePacket::packetType() const { return PacketType::Authenticate; }
google::protobuf::MessageLite const &MumbleAuthenticatePacket::message() const { return m_authenticate; }

/*
 * Mumble ping packet (ID 3)
 */

MumblePingPacket::MumblePingPacket(std::uint64_t timestamp, std::uint32_t good, std::uint32_t late, std::uint32_t lost,
								   std::uint32_t reSync, std::uint32_t udpPackets, std::uint32_t tcpPackets,
								   float udpPingAverage, float udpPingVariation, float tcpPingAverage,
								   float tcpPingVariation) {
	m_ping.set_timestamp(timestamp);
	m_ping.set_good(good);
	m_ping.set_late(late);
	m_ping.set_lost(lost);
	m_ping.set_resync(reSync);
	m_ping.set_udp_packets(udpPackets);
	m_ping.set_tcp_packets(tcpPackets);
	m_ping.set_udp_ping_avg(udpPingAverage);
	m_ping.set_udp_ping_var(udpPingVariation);
	m_ping.set_tcp_ping_avg(tcpPingAverage);
	m_ping.set_tcp_ping_var(tcpPingVariation);
}

MumblePingPacket::MumblePingPacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	m_ping.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}
PacketType MumblePingPacket::packetType() const { return PacketType::Ping; }
const google::protobuf::MessageLite &MumblePingPacket::message() const { return m_ping; }

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
		m_cryptSetup.set_key(container);
	}
	if (!clientNonce.empty()) {
		std::string container;
		const auto count = std::size(clientNonce);
		container.reserve(count);
		std::memcpy(container.data(), clientNonce.data(), count);
		m_cryptSetup.set_key(container);
	}
	if (!serverNonce.empty()) {
		std::string container;
		const auto count = std::size(serverNonce);
		container.reserve(count);
		std::memcpy(container.data(), serverNonce.data(), count);
		m_cryptSetup.set_key(container);
	}
}

MumbleCryptographySetupPacket::MumbleCryptographySetupPacket(std::span<const std::byte> buffer) {
	const auto bufferSize = std::size(buffer);
	m_cryptSetup.ParseFromArray(buffer.data(), static_cast<int>(bufferSize));
}

PacketType MumbleCryptographySetupPacket::packetType() const { return PacketType::CryptSetup; }
const google::protobuf::MessageLite &MumbleCryptographySetupPacket::message() const { return m_cryptSetup; }

}// namespace libmumble_protocol::common
