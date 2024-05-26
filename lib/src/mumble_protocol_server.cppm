//
// Created by JanHe on 26.05.2024.
//
module;

#include <cstdint>
#include <filesystem>
#include <thread>
#include <vector>

#include <asio.hpp>
#include <asio/ssl.hpp>

export module mumble_protocol:server;

import :utility;

namespace libmumble_protocol::server {

export class ServerStatePersistence {
public:
	virtual ~ServerStatePersistence() = default;

protected:
	virtual void dummy() = 0;
};

export class  MumbleServer final {
public:
	static constexpr std::uint16_t defaultPort = 64738;

	MumbleServer(ServerStatePersistence &server_state_persistance, const std::filesystem::path &certificate,
				 const std::filesystem::path &key_file, std::uint16_t concurrency = 0);
	MumbleServer(const MumbleServer &other) = delete;
	MumbleServer(MumbleServer &&other) noexcept = delete;

	auto operator=(const MumbleServer &other) -> MumbleServer & = delete;
	auto operator=(MumbleServer &&other) noexcept -> MumbleServer & = delete;

	~MumbleServer();

private:
	struct Impl;
	Pimpl<Impl> pimpl_;
};

struct MumbleServer::Impl final {

	ServerStatePersistence &persistance;

	std::vector<std::jthread> thread_handles;

	asio::io_context io_context;

	Impl(ServerStatePersistence &persistance, const std::filesystem::path &certificate, const std::filesystem::path &key_file,
		 const std::uint16_t concurrency)
		: persistance(persistance) {

		(void) certificate;
		(void) key_file;

		const std::uint16_t thread_count =
			concurrency != 0 ? concurrency : static_cast<std::uint16_t>(std::jthread::hardware_concurrency());
		for (std::size_t i = 0; i < thread_count; ++i) {
			thread_handles.emplace_back(&asio::io_context::run, &io_context);
		}
	}
	~Impl() {
		io_context.stop();
		for (auto &thread : thread_handles) { thread.join(); }
	}
};

MumbleServer::MumbleServer(ServerStatePersistence &server_state_persistance, const std::filesystem::path &certificate,
						   const std::filesystem::path &key_file, std::uint16_t concurrency)
	: pimpl_(server_state_persistance, certificate, key_file, concurrency) {}

MumbleServer::~MumbleServer() = default;

}// namespace libmumble_protocol::server
