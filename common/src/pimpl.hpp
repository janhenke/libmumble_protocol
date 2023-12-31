//
// Created by JanHe on 29.12.2023.
//

#ifndef LIBMUMBLE_SERVER_PIMPL_HPP
#define LIBMUMBLE_SERVER_PIMPL_HPP

#pragma once

#if __has_include(<experimental/propagate_const>)
#include <experimental/propagate_const>
#define PROPAGATE_CONST(T) std::experimental::propagate_const<T>
#else
#define PROPAGATE_CONST(T) T
#endif

#include <memory>

namespace libmumble::server {

template<typename T>
class Pimpl {
   private:
	std::unique_ptr<T> m;

   public:
	Pimpl();

	template<typename... Args>
	Pimpl(Args &&...);

	~Pimpl();

	T *operator->();

	T &operator*();
};

#define DECLARE_PIMPL(T) PROPAGATE_CONST(Pimpl<T>)

}// namespace libmumble::server

#endif//LIBMUMBLE_SERVER_PIMPL_HPP
