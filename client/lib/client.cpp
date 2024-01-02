//
// Created by Jan on 01.01.2024.
//

#include "client.hpp"

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <packet.hpp>
#include <pimpl_impl.hpp>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>

#include <array>
#include <chrono>
#include <thread>

namespace libmumble_protocol::client {

using namespace std::chrono_literals;

struct MumbleClient::Impl final {
	static constexpr auto s_pingPeriod = 20s;

	std::thread m_ioThread;
	asio::io_context m_ioContext;
	asio::ssl::context m_tlsContext;
	asio::ssl::stream<asio::ip::tcp::socket> m_tlsSocket;

	asio::steady_timer m_pingTimer;

	std::array<std::byte, common::maxPacketLength> m_receiveBuffer;

	Impl(std::string_view serverName, uint16_t port, std::string_view userName, bool validateServerCertificate)
		: m_tlsContext(asio::ssl::context_base::tlsv13_client), m_tlsSocket(m_ioContext, m_tlsContext),
		  m_pingTimer(m_ioContext), m_receiveBuffer() {

		m_tlsContext.set_default_verify_paths();
		m_tlsSocket.set_verify_mode(validateServerCertificate ? asio::ssl::verify_peer : asio::ssl::verify_none);
		m_tlsSocket.set_verify_callback(asio::ssl::host_name_verification(std::string{serverName}));

		asio::ip::tcp::resolver resolver{m_ioContext};
		const auto &endpoints = resolver.resolve(serverName, std::to_string(port));

		const auto connectedEndpoint = asio::connect(m_tlsSocket.next_layer(), endpoints);
		spdlog::debug("Connected to endpoint: {}, port: {}", connectedEndpoint.address().to_string(),
					  connectedEndpoint.port());
		m_tlsSocket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
		m_tlsSocket.handshake(asio::ssl::stream_base::client);

		// start periodic ping sender
		m_pingTimer.expires_after(s_pingPeriod);
		m_pingTimer.async_wait([this](const std::error_code &ec) {
			if (ec) {
				spdlog::critical("Timer wait failed with {}", ec.message());
				throw std::system_error(ec);
			}
			pingTimerCompletionHandler();
		});

		asio::async_read(m_tlsSocket, asio::buffer(m_receiveBuffer), asio::transfer_at_least(common::headerLength),
						 [this](const std::error_code &error, std::size_t bytes_transferred) {
							 readCompletionHandler(error, bytes_transferred);
						 });

		// begin Mumble handshake protocol
		// TODO: Replace with real values, for not these are only placeholders
		queuePacket(common::MumbleVersionPacket(1, 3, 4, "1.3.4", "Linux", "5.4.32"));

		queuePacket(common::MumbleAuthenticatePacket(userName, "", {}, {}, true));

		m_ioThread = std::thread{&asio::io_context::run, &m_ioContext};
	}

	~Impl() {
		m_ioContext.stop();
		m_ioThread.join();
	}

	void readCompletionHandler(const std::error_code &ec, std::size_t bytesTransferred) {
		if (ec) {
			spdlog::critical("Error reading from socket: {}", ec.message());
			throw std::system_error(ec);
		}

		const auto bufferBegin = std::begin(m_receiveBuffer);
		spdlog::debug("Read {} bytes from Socket: {}", bytesTransferred, spdlog::to_hex(bufferBegin, bufferBegin + 32));

		const auto [packetType, payload] = common::parseNetworkBuffer(m_receiveBuffer);
		auto not_implemented = [&packetType]() {
			spdlog::warn("No handler implemented for control packet type: {}",
						 static_cast<std::underlying_type_t<enum common::PacketType>>(packetType));
		};
		switch (packetType) {
			case common::PacketType::Version: handleVersionPacket(payload); break;
			case common::PacketType::UDPTunnel:
			case common::PacketType::Authenticate: not_implemented(); break;
			case common::PacketType::Ping: handlePingPacket(payload); break;
			case common::PacketType::Reject:
			case common::PacketType::ServerSync:
			case common::PacketType::ChannelRemove:
			case common::PacketType::ChannelState:
			case common::PacketType::UserRemove:
			case common::PacketType::UserState:
			case common::PacketType::BanList:
			case common::PacketType::TextMessage:
			case common::PacketType::PermissionDenied:
			case common::PacketType::ACL:
			case common::PacketType::QueryUsers: not_implemented(); break;
			case common::PacketType::CryptSetup: handleCryptSetupPacket(payload); break;
			case common::PacketType::ContextActionModify:
			case common::PacketType::ContextAction:
			case common::PacketType::UserList:
			case common::PacketType::VoiceTarget:
			case common::PacketType::PermissionQuery:
			case common::PacketType::CodecVersion:
			case common::PacketType::UserStats:
			case common::PacketType::RequestBlob:
			case common::PacketType::ServerConfig:
			case common::PacketType::SuggestConfig: not_implemented(); break;
		}

		asio::async_read(m_tlsSocket, asio::buffer(m_receiveBuffer), asio::transfer_at_least(common::headerLength),
						 [this](const std::error_code &error, std::size_t bytes_transferred) {
							 readCompletionHandler(error, bytes_transferred);
						 });
	}

	void queuePacket(const common::MumbleControlPacket &packet) {
		asio::async_write(m_tlsSocket, asio::buffer(packet.serialize()),
						  [](const std::error_code &ec, std::size_t bytes_transferred) {
							  if (ec) {
								  spdlog::critical("Error writing to socket: {}", ec.message());
								  throw std::system_error(ec);
							  }
							  spdlog::debug("Wrote {} bytes to socket", bytes_transferred);
						  });
	}

	void pingTimerCompletionHandler() {

		const auto timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		queuePacket(common::MumblePingPacket(timestamp));

		m_pingTimer.expires_after(s_pingPeriod);
		m_pingTimer.async_wait([this](const std::error_code &ec) {
			if (ec) {
				spdlog::critical("Timer wait failed with {}", ec.message());
				throw std::system_error(ec);
			}
			pingTimerCompletionHandler();
		});
	}

	static void handleVersionPacket(const std::span<const std::byte> payload) {
		common::MumbleVersionPacket versionPacket(payload);

		spdlog::debug("Received version packet: \n{}", versionPacket.debugString());
	}

	static void handlePingPacket(const std::span<const std::byte> payload) {
		common::MumblePingPacket pingPacket(payload);

		spdlog::debug("Received server ping: \n{}", pingPacket.debugString());
	}

	static void handleCryptSetupPacket(const std::span<const std::byte> payload) {
		common::MumbleCryptographySetupPacket cryptographySetupPacket(payload);

		spdlog::debug("Received crypt setup: \n{}", cryptographySetupPacket.debugString());
	}
};

MumbleClient::MumbleClient(std::string_view serverName, uint16_t port, std::string_view userName,
						   bool validateServerCertificate)
	: m_pimpl(serverName, port, userName, validateServerCertificate) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}

MumbleClient::~MumbleClient() = default;

}// namespace libmumble_protocol::client
