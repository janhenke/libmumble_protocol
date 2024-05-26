//
// Created by JanHe on 03.04.2024.
//

#include "server.hpp"

#include <pimpl_impl.hpp>

#include <asio.hpp>

#include <thread>
#include <vector>

namespace libmumble_protocol::server {

struct MumbleServer::Impl final {

	ServerStatePersistence& persistance;

	std::vector<std::jthread> thread_handles;

	asio::io_context io_context;

	Impl(ServerStatePersistence& persistance, const std::filesystem::path& certificate,
	     const std::filesystem::path& key_file,
	     const std::uint16_t concurrency)
		: persistance(persistance) {

		(void)certificate;
		(void)key_file;

		const std::uint16_t thread_count =
			concurrency != 0 ? concurrency : static_cast<std::uint16_t>(std::jthread::hardware_concurrency());
		for (std::size_t i = 0; i < thread_count; ++i) {
			thread_handles.emplace_back([this] { io_context.run(); });
		}
	}

	~Impl() {
		io_context.stop();
		for (auto& thread : thread_handles) { thread.join(); }
	}
};

MumbleServer::MumbleServer(ServerStatePersistence& server_state_persistance, const std::filesystem::path& certificate,
                           const std::filesystem::path& key_file, std::uint16_t concurrency) : pimpl_(
	server_state_persistance, certificate, key_file, concurrency) {}

MumbleServer::~MumbleServer() = default;

} // namespace libmumble_protocol::server
