//
// Created by JanHe on 29.12.2023.
//

#ifndef LIBMUMBLE_SERVER_PIMPL_HPP
#define LIBMUMBLE_SERVER_PIMPL_HPP

#pragma once

#include <memory>

namespace libmumble_protocol::common {

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

}// namespace libmumble_protocol::common

#endif//LIBMUMBLE_SERVER_PIMPL_HPP
