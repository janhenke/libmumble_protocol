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
	static constexpr auto ping_period = 20s;

	std::thread io_thread;
	asio::io_context io_context;
	asio::ssl::context tls_context;
	asio::ssl::stream<asio::ip::tcp::socket> tls_socket;

	asio::steady_timer ping_timer;

	std::array<std::byte, kMaxPacketLength> receive_buffer;
	std::array<std::byte, kMaxPacketLength> send_buffer;

	Impl(std::string_view serverName, uint16_t port, std::string_view userName, bool validateServerCertificate)
		: tls_context(asio::ssl::context_base::tlsv13_client), tls_socket(io_context, tls_context),
		  ping_timer(io_context), receive_buffer(), send_buffer() {

		tls_context.set_default_verify_paths();
		tls_socket.set_verify_mode(validateServerCertificate ? asio::ssl::verify_peer : asio::ssl::verify_none);
		tls_socket.set_verify_callback(asio::ssl::host_name_verification(std::string{serverName}));

		asio::ip::tcp::resolver resolver{io_context};
		const std::string service = std::to_string(port);
		const auto& endpoints = resolver.resolve(serverName, service);

		const auto connectedEndpoint = asio::connect(tls_socket.next_layer(), endpoints);
		spdlog::debug("Connected to endpoint: {}, port: {}", connectedEndpoint.address().to_string(),
		              connectedEndpoint.port());
		tls_socket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
		tls_socket.handshake(asio::ssl::stream_base::client);

		// start periodic ping sender
		ping_timer.expires_after(ping_period);
		ping_timer.async_wait([this](const std::error_code& ec) {
			if (ec) {
				spdlog::critical("Timer wait failed with {}", ec.message());
				throw std::system_error(ec);
			}
			pingTimerCompletionHandler();
		});

		asio::async_read(tls_socket, asio::buffer(receive_buffer), asio::transfer_at_least(kHeaderLength),
		                 [this](const std::error_code& error, std::size_t bytes_transferred) {
			                 readCompletionHandler(error, bytes_transferred);
		                 });

		// begin Mumble handshake protocol
		// TODO: Replace with real values, for not these are only placeholders
		queuePacket(MumbleVersionPacket({1, 4, 287}, "1.4.287", "Linux", "5.4.32"));

		queuePacket(MumbleAuthenticatePacket(userName, "", {}));

		io_thread = std::thread{[this] { io_context.run(); }};
	}

	~Impl() {
		io_context.stop();
		io_thread.join();
	}

	void readCompletionHandler(const std::error_code& ec, const std::size_t bytesTransferred) {
		if (ec) {
			spdlog::critical("Error reading from socket: {}", ec.message());
			throw std::system_error(ec);
		}

		const auto bufferBegin = std::begin(receive_buffer);
		spdlog::debug("Read {} bytes from Socket: {}", bytesTransferred, spdlog::to_hex(bufferBegin, bufferBegin + 32));

		const auto [packetType, payload] = ParseNetworkBuffer(receive_buffer);
		auto not_implemented = [&packetType]() {
			spdlog::warn("No handler implemented for control packet type: {}",
			             static_cast<std::underlying_type_t<enum PacketType>>(packetType));
		};
		switch (packetType) {
			case PacketType::Version:
				handleVersionPacket(payload);
				break;
			case PacketType::UDPTunnel:
			case PacketType::Authenticate:
				not_implemented();
				break;
			case PacketType::Ping:
				handlePingPacket(payload);
				break;
			case PacketType::Reject:
			case PacketType::ServerSync:
			case PacketType::ChannelRemove:
			case PacketType::ChannelState:
			case PacketType::UserRemove:
			case PacketType::UserState:
			case PacketType::BanList:
			case PacketType::TextMessage:
			case PacketType::PermissionDenied:
			case PacketType::ACL:
			case PacketType::QueryUsers:
				not_implemented();
				break;
			case PacketType::CryptSetup:
				handleCryptSetupPacket(payload);
				break;
			case PacketType::ContextActionModify:
			case PacketType::ContextAction:
			case PacketType::UserList:
			case PacketType::VoiceTarget:
			case PacketType::PermissionQuery:
			case PacketType::CodecVersion:
			case PacketType::UserStats:
			case PacketType::RequestBlob:
			case PacketType::ServerConfig:
			case PacketType::SuggestConfig:
				not_implemented();
				break;
		}

		asio::async_read(tls_socket, asio::buffer(receive_buffer), asio::transfer_at_least(kHeaderLength),
		                 [this](const std::error_code& error, std::size_t bytes_transferred) {
			                 readCompletionHandler(error, bytes_transferred);
		                 });
	}

	void queuePacket(const MumbleControlPacket& packet) {

		const std::size_t size = packet.Serialize(send_buffer);

		asio::async_write(tls_socket, asio::buffer(send_buffer, size),
		                  [](const std::error_code& ec, std::size_t bytes_transferred) {
			                  if (ec) {
				                  spdlog::critical("Error writing to socket: {}", ec.message());
				                  throw std::system_error(ec);
			                  }
			                  spdlog::debug("Wrote {} bytes to socket", bytes_transferred);
		                  });
	}

	void pingTimerCompletionHandler() {

		const auto timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		queuePacket(MumblePingPacket(timestamp));

		ping_timer.expires_after(ping_period);
		ping_timer.async_wait([this](const std::error_code& ec) {
			if (ec) {
				spdlog::critical("Timer wait failed with {}", ec.message());
				throw std::system_error(ec);
			}
			pingTimerCompletionHandler();
		});
	}

	static void handleVersionPacket(const std::span<const std::byte> payload) {
		MumbleVersionPacket versionPacket(payload);

		spdlog::debug("Received version packet: \n{}", versionPacket.DebugString());
		spdlog::info("Server version {}.{}.{}", versionPacket.majorVersion(), versionPacket.minorVersion(),
		             versionPacket.patchVersion());
	}

	static void handlePingPacket(const std::span<const std::byte> payload) {
		MumblePingPacket pingPacket(payload);

		spdlog::debug("Received server ping: \n{}", pingPacket.DebugString());
	}

	static void handleCryptSetupPacket(const std::span<const std::byte> payload) {
		MumbleCryptographySetupPacket cryptographySetupPacket(payload);

		spdlog::debug("Received crypt setup: \n{}", cryptographySetupPacket.DebugString());
	}
};

MumbleClient::MumbleClient(std::string_view serverName, uint16_t port, std::string_view userName,
                           bool validateServerCertificate)
	: pimpl_(serverName, port, userName, validateServerCertificate) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}

MumbleClient::~MumbleClient() = default;

} // namespace libmumble_protocol::client
