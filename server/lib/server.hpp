//
// Created by JanHe on 03.04.2024.
//

#ifndef LIBMUMBLE_PROTOCOL_SERVER_LIB_SERVER_HPP
#define LIBMUMBLE_PROTOCOL_SERVER_LIB_SERVER_HPP

#include "mumble_server_export.h"

#include <pimpl.hpp>

namespace libmumble_protocol::server {

class MUMBLE_SERVER_EXPORT MumbleServer final {
   public:
	MumbleServer();
	~MumbleServer();

   private:
	struct Impl;
	libmumble_protocol::common::Pimpl<Impl> pimpl_;
};

}// namespace libmumble_protocol::server

#endif//LIBMUMBLE_PROTOCOL_SERVER_LIB_SERVER_HPP
