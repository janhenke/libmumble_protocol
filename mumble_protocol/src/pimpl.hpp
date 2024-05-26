//
// Created by JanHe on 29.12.2023.
//

#ifndef LIBMUMBLE_SERVER_PIMPL_HPP
#define LIBMUMBLE_SERVER_PIMPL_HPP

#pragma once

#include <memory>

namespace libmumble_protocol {

template <typename T>
class Pimpl {
	std::unique_ptr<T> m;

public:
	Pimpl();

	template <typename... Args>
	Pimpl(Args&&...);

	~Pimpl();

	auto operator->() -> T*;

	auto operator*() -> T&;
};

} // namespace libmumble_protocol

#endif//LIBMUMBLE_SERVER_PIMPL_HPP
