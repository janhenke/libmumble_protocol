//
// Created by JanHe on 03.04.2024.
//

#ifndef LIBMUMBLE_PROTOCOL_SERVER_LIB_SERVER_HPP
#define LIBMUMBLE_PROTOCOL_SERVER_LIB_SERVER_HPP

#include "mumble_protocol_export.h"

#include <pimpl.hpp>

#include <cstdint>
#include <filesystem>

namespace libmumble_protocol::server {

class ServerStatePersistence {
public:
	virtual ~ServerStatePersistence() = default;

protected:
	virtual void dummy() = 0;
};

class MUMBLE_PROTOCOL_EXPORT MumbleServer final {
public:
	static constexpr std::uint16_t defaultPort = 64738;

	MumbleServer(ServerStatePersistence& server_state_persistance, const std::filesystem::path& certificate,
	             const std::filesystem::path& key_file, std::uint16_t concurrency = 0);

	MumbleServer(const MumbleServer& other) = delete;
	MumbleServer(MumbleServer&& other) noexcept = delete;

	auto operator=(const MumbleServer& other) -> MumbleServer& = delete;
	auto operator=(MumbleServer&& other) noexcept -> MumbleServer& = delete;

	~MumbleServer();

private:
	struct Impl;
	Pimpl<Impl> pimpl_;
};

} // namespace libmumble_protocol::server

#endif//LIBMUMBLE_PROTOCOL_SERVER_LIB_SERVER_HPP
