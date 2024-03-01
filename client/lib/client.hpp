//
// Created by Jan on 01.01.2024.
//

#ifndef LIBMUMBLE_PROTOCOL_CLIENT_HPP
#define LIBMUMBLE_PROTOCOL_CLIENT_HPP

#pragma once

#include "mumble_client_export.h"

#include <pimpl.hpp>

#include <cstdint>
#include <string_view>

namespace libmumble_protocol::client {

class MUMBLE_CLIENT_EXPORT MumbleClient final {
   public:
	static constexpr std::uint16_t defaultPort = 64738;

	MumbleClient(std::string_view serverName, std::uint16_t port, std::string_view userName,
				 bool validateServerCertificate = true);
	~MumbleClient();

   private:
	struct Impl;
	libmumble_protocol::common::Pimpl<Impl> pimpl_;
};

}// namespace libmumble_protocol::client

#endif//LIBMUMBLE_PROTOCOL_CLIENT_HPP
