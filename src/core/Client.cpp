//
// Created by jan on 28.10.21.
//

#include "Client.h"
#include "Mumble.pb.h"
#include "protocol/protocol.h"

#include <array>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include <asio/ssl.hpp>
#include <asio/ts/net.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/dump.hpp>

namespace mumble_client::core {

using namespace std::chrono_literals;

void InitLogging() {
#ifdef NDEBUG
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#else
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
#endif
}

// Client::Impl

struct Client::Impl {
	static constexpr auto PingPeriod = 20s;

	Impl();

	asio::io_context io_context;
	asio::ssl::context tls_context;
	asio::ssl::stream<asio::ip::tcp::socket> tls_socket;

	asio::steady_timer ping_timer;

	std::array<std::byte, protocol::control::MaxPacketLength> receive_buffer;

	std::deque<std::vector<std::byte>> send_queue;
	std::mutex send_queue_mutex;

	std::unique_ptr<protocol::control::VersionPacket> server_version;

	void ReadCompletionHandler(const std::error_code &ec, std::size_t bytes_transferred);

	void QueueMessage(const std::vector<std::byte> &message);

	void WriteCompletionHandler(const std::error_code &ec, std::size_t bytes_transferred);

	void PingTimerCompletionHandler();

	void Connect(std::string_view user_name, std::string_view server, uint16_t port, bool validate_server_certificate);

	void HandleVersionPacket(std::span<std::byte> payload);

	void SendVersionPacket();

	void SendAuthenticatePaket(std::string_view user_name);

	static void HandlePingPacket(std::span<std::byte> payload);

	static void HandleRejectPacket(std::span<std::byte> payload);

	static void HandleServerSyncPacket(std::span<std::byte> payload);

	static void HandleChannelStatePacket(std::span<std::byte> payload);

	static void HandleUserStatePacket(std::span<std::byte> payload);

	static void HandleCryptSetupPacket(std::span<std::byte> payload);
};

Client::Impl::Impl()
	: io_context(), tls_context(asio::ssl::context::tlsv12), tls_socket(io_context, tls_context),
	  ping_timer(io_context), receive_buffer(), send_queue(), send_queue_mutex(), server_version(nullptr) {}

void Client::Impl::ReadCompletionHandler(const std::error_code &ec, std::size_t bytes_transferred) {
	if (ec) {
		BOOST_LOG_TRIVIAL(error) << "Error reading from socket: " << ec.message();
		throw std::system_error(ec);
	}

	BOOST_LOG_TRIVIAL(debug) << "Read " << bytes_transferred << " bytes from Socket: "
							 << boost::log::dump(receive_buffer.data(), protocol::control::HeaderLength);

	const std::span buffer_span{receive_buffer};
	const protocol::control::Header header{buffer_span};
	BOOST_LOG_TRIVIAL(debug) << "Packet type " << static_cast<uint16_t>(header.packet_type) << ", length "
							 << header.packet_length;

	const auto payload = buffer_span.subspan(protocol::control::HeaderLength, header.packet_length);
	switch (header.packet_type) {
		case protocol::control::PacketType::Version: HandleVersionPacket(payload); break;
		case protocol::control::PacketType::UDPTunnel:
		case protocol::control::PacketType::Authenticate:
			BOOST_LOG_TRIVIAL(error) << "No handler implemented for control packet type: "
									 << static_cast<std::underlying_type_t<protocol::control::PacketType>>(
											header.packet_type);
			break;
		case protocol::control::PacketType::Ping: HandlePingPacket(payload); break;
		case protocol::control::PacketType::Reject: HandleRejectPacket(payload); break;
		case protocol::control::PacketType::ServerSync: HandleServerSyncPacket(payload); break;
		case protocol::control::PacketType::ChannelRemove:
			BOOST_LOG_TRIVIAL(error) << "No handler implemented for control packet type: "
									 << static_cast<std::underlying_type_t<protocol::control::PacketType>>(
											header.packet_type);
			break;
		case protocol::control::PacketType::ChannelState: HandleChannelStatePacket(payload); break;
		case protocol::control::PacketType::UserRemove:
			BOOST_LOG_TRIVIAL(error) << "No handler implemented for control packet type: "
									 << static_cast<std::underlying_type_t<protocol::control::PacketType>>(
											header.packet_type);
			break;
		case protocol::control::PacketType::UserState: HandleUserStatePacket(payload); break;
		case protocol::control::PacketType::BanList:
		case protocol::control::PacketType::TextMessage:
		case protocol::control::PacketType::PermissionDenied:
		case protocol::control::PacketType::ACL:
		case protocol::control::PacketType::QueryUsers:
			BOOST_LOG_TRIVIAL(error) << "No handler implemented for control packet type: "
									 << static_cast<std::underlying_type_t<protocol::control::PacketType>>(
											header.packet_type);
			break;
		case protocol::control::PacketType::CryptSetup: HandleCryptSetupPacket(payload); break;
		case protocol::control::PacketType::ContextActionModify:
		case protocol::control::PacketType::ContextAction:
		case protocol::control::PacketType::UserList:
		case protocol::control::PacketType::VoiceTarget:
		case protocol::control::PacketType::PermissionQuery:
		case protocol::control::PacketType::CodecVersion:
		case protocol::control::PacketType::UserStats:
		case protocol::control::PacketType::RequestBlob:
		case protocol::control::PacketType::ServerConfig:
		case protocol::control::PacketType::SuggestConfig:
			BOOST_LOG_TRIVIAL(error) << "No handler implemented for control packet type: "
									 << static_cast<std::underlying_type_t<protocol::control::PacketType>>(
											header.packet_type);
			break;
	}

	asio::async_read(tls_socket, asio::buffer(receive_buffer), asio::transfer_at_least(protocol::control::HeaderLength),
					 [this](const std::error_code &error, std::size_t bytes_transferred) {
						 ReadCompletionHandler(error, bytes_transferred);
					 });
}

void Client::Impl::QueueMessage(const std::vector<std::byte> &message) {
	if (std::lock_guard{send_queue_mutex}; send_queue.empty()) {
		asio::async_write(tls_socket, asio::buffer(message),
						  [this](const std::error_code &ec, std::size_t bytes_transferred) {
							  WriteCompletionHandler(ec, bytes_transferred);
						  });
	} else {
		send_queue.push_back(message);
	}
}

void Client::Impl::WriteCompletionHandler(const std::error_code &ec, std::size_t bytes_transferred) {
	if (ec) {
		BOOST_LOG_TRIVIAL(error) << "Error writing to socket: " << ec.message();
		throw std::system_error(ec);
	}

	BOOST_LOG_TRIVIAL(debug) << "Wrote " << bytes_transferred << " bytes to socket";

	if (std::lock_guard{send_queue_mutex}; !send_queue.empty()) {
		const auto message = send_queue.front();
		send_queue.pop_front();
		asio::async_write(tls_socket, asio::buffer(message),
						  [this](const std::error_code &ec, std::size_t bytes_transferred) {
							  WriteCompletionHandler(ec, bytes_transferred);
						  });
	}
}

void Client::Impl::PingTimerCompletionHandler() {

	MumbleProto::Ping ping;
	const auto Timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	ping.set_timestamp(static_cast<uint64_t>(Timestamp));
	const auto PingAsString = ping.SerializeAsString();
	const auto PingBuffer = std::as_bytes(std::span(PingAsString));

	protocol::control::Header header{protocol::control::PacketType::Ping,
									 static_cast<uint32_t>(std::size(PingAsString))};
	const auto HeaderBuffer = header.Serialize();

	std::vector<std::byte> message{std::cbegin(HeaderBuffer), std::cend(HeaderBuffer)};
	message.insert(std::cend(message), std::cbegin(PingBuffer), std::cend(PingBuffer));

	QueueMessage(message);

	ping_timer.expires_after(PingPeriod);
	ping_timer.async_wait([this](const std::error_code &ec) {
		if (ec) {
			BOOST_LOG_TRIVIAL(error) << "Timer wait failed with " << ec.message();
			throw std::system_error(ec);
		}
		PingTimerCompletionHandler();
	});
}

void Client::Impl::Connect(const std::string_view user_name, const std::string_view server, const uint16_t port,
						   const bool validate_server_certificate) {
	tls_context.set_default_verify_paths();
	tls_socket.set_verify_mode(validate_server_certificate ? asio::ssl::verify_peer : asio::ssl::verify_none);
	tls_socket.set_verify_callback(asio::ssl::host_name_verification(std::string{server}));

	asio::ip::tcp::resolver resolver{io_context};
	const auto &endpoints = resolver.resolve(server, std::to_string(port));

	const auto connected_endpoint = asio::connect(tls_socket.next_layer(), endpoints);
	BOOST_LOG_TRIVIAL(debug) << "Connected to endpoint: " << connected_endpoint.address().to_string()
							 << ", port: " << connected_endpoint.port();
	tls_socket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
	tls_socket.handshake(asio::ssl::stream_base::client);

	// start periodic ping sender
	ping_timer.expires_after(PingPeriod);
	ping_timer.async_wait([this](const std::error_code &ec) {
		if (ec) {
			BOOST_LOG_TRIVIAL(error) << "Timer wait failed with " << ec.message();
			throw std::system_error(ec);
		}
		PingTimerCompletionHandler();
	});

	asio::async_read(tls_socket, asio::buffer(receive_buffer), asio::transfer_at_least(protocol::control::HeaderLength),
					 [this](const std::error_code &error, std::size_t bytes_transferred) {
						 ReadCompletionHandler(error, bytes_transferred);
					 });

	// begin Mumble handshake protocol
	SendVersionPacket();
	SendAuthenticatePaket(user_name);
}

void Client::Impl::HandleVersionPacket(const std::span<std::byte> payload) {
	MumbleProto::Version version;
	version.ParseFromArray(payload.data(), static_cast<int>(payload.size_bytes()));

	BOOST_LOG_TRIVIAL(debug) << "Version packet: " << version.DebugString();
	server_version = std::make_unique<protocol::control::VersionPacket>(version.version(), version.release(),
																		version.os(), version.os_version());
}

void Client::Impl::SendVersionPacket() {
	protocol::control::VersionPacket::Version client_version{1, 3, 4};
	const uint32_t NumericVersion(client_version);

	MumbleProto::Version version{};
	version.set_version(NumericVersion);
	version.set_release("1.3.4");
	version.set_os("Linux");
	version.set_os_version("5.4.32");
	const auto VersionAsString = version.SerializeAsString();
	const auto VersionBuffer = std::as_bytes(std::span(VersionAsString));

	protocol::control::Header header{protocol::control::PacketType::Version,
									 static_cast<uint32_t>(std::size(VersionAsString))};
	const auto HeaderBuffer = header.Serialize();

	std::vector<std::byte> message{std::cbegin(HeaderBuffer), std::cend(HeaderBuffer)};
	message.insert(std::cend(message), std::cbegin(VersionBuffer), std::cend(VersionBuffer));

	QueueMessage(message);
}

void Client::Impl::SendAuthenticatePaket(const std::string_view user_name) {
	MumbleProto::Authenticate authenticate{};
	authenticate.set_username(std::string(user_name));
	authenticate.set_opus(true);
	const auto AuthenticateAsString = authenticate.SerializeAsString();
	const auto AuthenticateBuffer = std::as_bytes(std::span(AuthenticateAsString));

	protocol::control::Header header{protocol::control::PacketType::Authenticate,
									 static_cast<uint32_t>(std::size(AuthenticateAsString))};
	const auto HeaderBuffer = header.Serialize();

	std::vector<std::byte> message{std::cbegin(HeaderBuffer), std::cend(HeaderBuffer)};
	message.insert(std::cend(message), std::cbegin(AuthenticateBuffer), std::cend(AuthenticateBuffer));

	QueueMessage(message);
}

void Client::Impl::HandlePingPacket(const std::span<std::byte> payload) {
	MumbleProto::Ping ping;
	ping.ParseFromArray(payload.data(), static_cast<int>(payload.size_bytes()));

	BOOST_LOG_TRIVIAL(trace) << "Received server ping: " << std::endl << ping.DebugString();
}

void Client::Impl::HandleRejectPacket(std::span<std::byte> payload) {
	MumbleProto::Reject reject;
	reject.ParseFromArray(payload.data(), static_cast<int>(payload.size_bytes()));

	BOOST_LOG_TRIVIAL(debug) << "Received server reject: " << std::endl << reject.DebugString();
}

void Client::Impl::HandleServerSyncPacket(std::span<std::byte> payload) {
	MumbleProto::ServerSync server_sync;
	server_sync.ParseFromArray(payload.data(), static_cast<int>(payload.size_bytes()));

	BOOST_LOG_TRIVIAL(debug) << "Received server sync: " << std::endl << server_sync.DebugString();
}

void Client::Impl::HandleChannelStatePacket(std::span<std::byte> payload) {
	MumbleProto::ChannelState channel_state;
	channel_state.ParseFromArray(payload.data(), static_cast<int>(payload.size_bytes()));

	BOOST_LOG_TRIVIAL(debug) << "Received channel state: " << std::endl << channel_state.DebugString();
}

void Client::Impl::HandleUserStatePacket(std::span<std::byte> payload) {
	MumbleProto::UserState user_state;
	user_state.ParseFromArray(payload.data(), static_cast<int>(payload.size_bytes()));

	BOOST_LOG_TRIVIAL(debug) << "Received user state: " << std::endl << user_state.DebugString();
}

void Client::Impl::HandleCryptSetupPacket(std::span<std::byte> payload) {
	MumbleProto::CryptSetup crypt_setup;
	crypt_setup.ParseFromArray(payload.data(), static_cast<int>(payload.size_bytes()));

	BOOST_LOG_TRIVIAL(debug) << "Received crypt setup: " << std::endl << crypt_setup.DebugString();
}

// Client

Client::Client(std::string_view user_name, const std::string_view server, const uint16_t port,
			   const bool validate_server_certificate)
	: PImpl(std::make_unique<Client::Impl>()) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	PImpl->Connect(user_name, server, port, validate_server_certificate);
}

Client::~Client() = default;

void Client::operator()() {
	InitLogging();

	PImpl->io_context.run();
}
}// namespace mumble_client::core