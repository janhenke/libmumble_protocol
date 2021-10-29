//
// Created by jan on 28.10.21.
//

#ifndef _CLIENT_H_
#define _CLIENT_H_

#pragma once

#include "mumble_client_export.h"

#include <cstdint>
#include <memory>
#include <string_view>

#if __has_include(<experimental/propagate_const>)
#include <experimental/propagate_const>
#endif

namespace mumble_client {
namespace core {
class MUMBLE_CLIENT_EXPORT Client {
   public:
	explicit Client(std::string_view server, uint16_t port = 64738, bool validate_server_certificate = true);

	Client(const Client &&) = delete;

	~Client();

	void operator()();

   private:
	struct Impl;
#if __has_include(<experimental/propagate_const>)
	std::experimental::propagate_const<std::unique_ptr<Impl>> PImpl;
#else
	std::unique_ptr<Impl> PImpl;
#endif
};
}// namespace core

using mumble_client::core::Client;
}// namespace mumble_client

#endif//_CLIENT_H_
