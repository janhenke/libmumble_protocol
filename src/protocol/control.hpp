//
// Created by jan on 12.05.20.
//

#include <memory>
#include <version>

#if __has_include(<experimental/propagate_const>)
#include <experimental/propagate_const>
#endif

#include "mumble_client_export.h"

#ifndef LIBMUMBLE_CLIENT_CONTROL_HPP
#define LIBMUMBLE_CLIENT_CONTROL_HPP

namespace mumble_client::protocol::control {

	class MUMBLE_CLIENT_EXPORT acl {


	private:
		struct impl;
//#if __cpp_lib_experimental_propagate_const >= 201505L
#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
#else
		std::unique_ptr<impl> pImpl;
#endif
	};
}

#endif //LIBMUMBLE_CLIENT_CONTROL_HPP
