//
// Created by jan on 28.10.21.
//

#include "Client.h"
#include "Mumble.pb.h"
#include "protocol/protocol.h"

#include <array>
#include <memory>
#include <span>
#include <string>
#include <type_traits>

#include <asio/ssl.hpp>
#include <asio/ts/net.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/dump.hpp>

namespace mumble_client::core {

void InitLogging() {
#ifdef NDEBUG
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#else
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
#endif
}

// Client::Impl

struct Client::Impl {
	Impl();

	asio::io_context io_context;
	asio::ssl::context tls_context;
	asio::ssl::stream<asio::ip::tcp::socket> tls_socket;

	std::array<std::byte, protocol::control::MaxPacketLength> receive_buffer;

	std::unique_ptr<protocol::control::VersionPacket> server_version;

	void ReadCompletionHandler(const std::error_code &ec, std::size_t bytes_transferred);

	void Connect(std::string_view user_name, std::string_view server, uint16_t port, bool validate_server_certificate);
	void ParseReceivedBuffer(std::size_t bytes_transferred);
	void ParseVersionPacket(std::span<std::byte> payload);
	void SendVersionPacket();
	void SendAuthenticatePaket(std::string_view user_name);
};

Client::Impl::Impl()
	: io_context(), tls_context(asio::ssl::context::tlsv12), tls_socket(io_context, tls_context), receive_buffer(),
	  server_version(nullptr) {}

void Client::Impl::ReadCompletionHandler(const std::error_code &ec, std::size_t bytes_transferred) {
	if (ec) {
		BOOST_LOG_TRIVIAL(error) << "Error reading from connection: " << ec.message();
		throw std::system_error(ec);
	} else {
		ParseReceivedBuffer(bytes_transferred);
		asio::async_read(tls_socket, asio::buffer(receive_buffer),
						 asio::transfer_at_least(protocol::control::HeaderLength),
						 [this](const std::error_code &error, std::size_t bytes_transferred) {
							 ReadCompletionHandler(error, bytes_transferred);
						 });
	}
}

void Client::Impl::ParseReceivedBuffer(std::size_t bytes_transferred) {

	BOOST_LOG_TRIVIAL(debug) << "Read " << bytes_transferred << " bytes from Socket: "
							 << boost::log::dump(receive_buffer.data(), protocol::control::HeaderLength);

	const std::span BufferSpan{receive_buffer};
	const protocol::control::Header Header{BufferSpan};
	BOOST_LOG_TRIVIAL(debug) << "Packet type " << static_cast<uint16_t>(Header.packet_type) << ", length "
							 << Header.packet_length;

	const auto Payload =
		BufferSpan.subspan(protocol::control::HeaderLength, protocol::control::HeaderLength + Header.packet_length);
	switch (Header.packet_type) {
		case protocol::control::PacketType::Version: ParseVersionPacket(Payload); break;
		case protocol::control::PacketType::UDPTunnel:
		case protocol::control::PacketType::Authenticate:
		case protocol::control::PacketType::Ping:
		case protocol::control::PacketType::Reject:
		case protocol::control::PacketType::ServerSync:
		case protocol::control::PacketType::ChannelRemove:
		case protocol::control::PacketType::ChannelState:
		case protocol::control::PacketType::UserRemove:
		case protocol::control::PacketType::UserState:
		case protocol::control::PacketType::BanList:
		case protocol::control::PacketType::TextMessage:
		case protocol::control::PacketType::PermissionDenied:
		case protocol::control::PacketType::ACL:
		case protocol::control::PacketType::QueryUsers:
		case protocol::control::PacketType::CryptSetup:
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
											Header.packet_type);
			break;
	}
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

	asio::async_read(tls_socket, asio::buffer(receive_buffer), asio::transfer_at_least(protocol::control::HeaderLength),
					 [this](const std::error_code &error, std::size_t bytes_transferred) {
						 ReadCompletionHandler(error, bytes_transferred);
					 });

	// begin Mumble handshake protocol
	SendVersionPacket();
	SendAuthenticatePaket(user_name);
}

void Client::Impl::ParseVersionPacket(const std::span<std::byte> payload) {
	MumbleProto::Version version;
	version.ParseFromArray(payload.data(), static_cast<int>(payload.size_bytes()));

	BOOST_LOG_TRIVIAL(debug) << "Version packet: " << version.DebugString();
	server_version = std::make_unique<protocol::control::VersionPacket>(version.version(), version.release(),
																		version.os(), version.os_version());
}

void Client::Impl::SendVersionPacket() {
	// first Mumble version with Opus support
	protocol::control::VersionPacket::Version client_version{1, 2, 4};
	const uint32_t NumericVersion(client_version);
	MumbleProto::Version version{};
	version.set_version(NumericVersion);
	version.set_release("");
	version.set_os("");
	version.set_os_version("");

	protocol::control::Header header{protocol::control::PacketType::Version,
									 static_cast<uint32_t>(version.SpaceUsedLong())};
	std::array<std::byte, protocol::control::MaxPacketLength> buffer{};
	std::span<std::byte> buffer_span{buffer};

	header.Write(buffer);
	std::span<std::byte> payload_span = buffer_span.last(protocol::control::MaxPayloadLength);
	version.SerializeToArray(payload_span.data(), static_cast<int>(payload_span.size_bytes()));

	asio::async_write(tls_socket, asio::buffer(buffer),
					  [this](const std::error_code &ec, std::size_t bytes_transferred) {});
}

void Client::Impl::SendAuthenticatePaket(const std::string_view user_name) {
	MumbleProto::Authenticate authenticate{};
	authenticate.set_username(std::string(user_name));
	authenticate.set_opus(true);

	protocol::control::Header header{protocol::control::PacketType::Authenticate,
									 static_cast<uint32_t>(authenticate.SpaceUsedLong())};
	std::array<std::byte, protocol::control::MaxPacketLength> buffer{};
	std::span<std::byte> buffer_span{buffer};

	header.Write(buffer);
	std::span<std::byte> payload_span = buffer_span.last(protocol::control::MaxPayloadLength);
	authenticate.SerializeToArray(payload_span.data(), static_cast<int>(payload_span.size_bytes()));

	asio::async_write(tls_socket, asio::buffer(buffer),
					  [this](const std::error_code &ec, std::size_t bytes_transferred) {});
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