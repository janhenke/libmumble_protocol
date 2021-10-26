//
// Created by jan on 09.10.20.
//

#include "core.hpp"

#include <array>
#include <string>

#include <asio/ssl.hpp>
#include <asio/ts/net.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/manipulators/dump.hpp>

#include "packet/tcp_packet.hpp"

namespace mumble_client
{

	void init_logging()
	{
#ifdef NDEBUG
		boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#else
		boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
#endif
	}

	struct client::impl
	{
		explicit impl(std::string_view server, uint16_t port = 64738, bool validateServerCertificate = true);

		asio::io_context ioContext;
		asio::ssl::context tlsContext;
		asio::ssl::stream<asio::ip::tcp::socket> tlsStream;

		std::array<std::byte, packet::tcp::MaxPacketLength> receiveBuffer;
	};

	client::impl::impl(const std::string_view server, const uint16_t port, const bool validateServerCertificate)
		: ioContext(),
		  tlsContext(asio::ssl::context::tlsv12),
		  tlsStream(ioContext, tlsContext),
		  receiveBuffer()
	{

		tlsContext.set_default_verify_paths();
		tlsStream.set_verify_mode(validateServerCertificate ? asio::ssl::verify_peer : asio::ssl::verify_none);
		tlsStream.set_verify_callback(asio::ssl::host_name_verification(std::string{ server }));

		asio::ip::tcp::resolver resolver{ ioContext };
		const auto& endpoints = resolver.resolve(server, std::to_string(port));

		asio::async_connect(tlsStream.lowest_layer(),
			endpoints,
			[this](const std::error_code& error, const asio::ip::tcp::endpoint& endpoint)
			{
				if (!error)
				{
					BOOST_LOG_TRIVIAL(debug) << "Connected to endpoint: "
											 << endpoint.address().to_string()
											 << ":"
											 << endpoint.port();
					tlsStream.lowest_layer().set_option(asio::ip::tcp::no_delay(true));

					tlsStream.handshake(asio::ssl::stream_base::client);

					std::size_t length = tlsStream.read_some(asio::buffer(receiveBuffer));
					BOOST_LOG_TRIVIAL(debug) << "Read " << length << " bytes from Socket: "
											 << boost::log::dump(receiveBuffer.data(),
												 packet::tcp::HeaderLength);
					const auto header = packet::tcp::header::parse(receiveBuffer);

				}
				else
				{
					BOOST_LOG_TRIVIAL(error) << "Connection failed: " << error.message();
				}
			});
	}

	client::client(const std::string_view server,
		const uint16_t port,
		const bool validateServerCertificate)
		: pImpl(new client::impl{ server,
								  port,
								  validateServerCertificate })
	{

	}

	client::~client() = default;

	void client::operator()()
	{
		init_logging();

		pImpl->ioContext.run();
	}
}
