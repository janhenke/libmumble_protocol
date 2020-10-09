//
// Created by jan on 09.10.20.
//

#ifndef LIBMUMBLE_CLIENT_CORE_HPP
#define LIBMUMBLE_CLIENT_CORE_HPP

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#if __has_include(<experimental/propagate_const>)

#include <experimental/propagate_const>

#endif

#include "mumble_client_export.h"

namespace mumble_client {

	class MUMBLE_CLIENT_EXPORT client {
	public:
		explicit client(std::string_view server, uint16_t port = 64738, bool validateServerCertificate = true);

		client(const client &&) = delete;

		~client();

		void operator()();

	private:
		struct impl;
#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
#else
		std::unique_ptr<impl> pImpl;
#endif
	};

}

#endif //LIBMUMBLE_CLIENT_CORE_HPP
