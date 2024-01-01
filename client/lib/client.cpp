//
// Created by Jan on 01.01.2024.
//

#include "client.hpp"

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <packet.hpp>
#include <pimpl_impl.hpp>

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
		const auto &endpoints = resolver.resolve(std::string(serverName), std::to_string(port));

		const auto connected_endpoint = asio::connect(m_tlsSocket.next_layer(), endpoints);
//		qDebug() << u8"Connected to endpoint: " << connected_endpoint.address().to_string().c_str() << u8", port: " << connected_endpoint.port();
		m_tlsSocket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
		m_tlsSocket.handshake(asio::ssl::stream_base::client);

		m_ioThread = std::thread{&asio::io_context::run, &m_ioContext};
	}

	~Impl() {
		m_ioContext.stop();
		m_ioThread.join();
	}
};

MumbleClient::MumbleClient(std::string_view serverName, uint16_t port, std::string_view userName,
						   bool validateServerCertificate)
	: m_pimpl(serverName, port, userName, validateServerCertificate) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}

}// namespace libmumble_protocol::client
