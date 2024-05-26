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
	explicit Pimpl(Args&&...);

	Pimpl(const Pimpl& other) = delete;
	Pimpl(Pimpl&& other) noexcept = default;

	auto operator=(const Pimpl& other) -> Pimpl& = delete;
	auto operator=(Pimpl&& other) noexcept -> Pimpl& = default;

	~Pimpl();

	auto operator->() -> T*;

	auto operator*() -> T&;
};

} // namespace libmumble_protocol

#endif//LIBMUMBLE_SERVER_PIMPL_HPP
