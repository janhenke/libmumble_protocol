//
// Created by JanHe on 29.12.2023.
//

#ifndef LIBMUMBLE_SERVER_PIMPL_IMPL_HPP
#define LIBMUMBLE_SERVER_PIMPL_IMPL_HPP

#pragma once

#include <utility>

namespace libmumble::server {

template<typename T>
Pimpl<T>::Pimpl() : m{new T{}} {}

template<typename T>
template<typename... Args>
Pimpl<T>::Pimpl(Args &&...args) : m{new T{std::forward<Args>(args)...}} {}

template<typename T>
Pimpl<T>::~Pimpl() = default;

template<typename T>
T *Pimpl<T>::operator->() {
	return m.get();
}

template<typename T>
T &Pimpl<T>::operator*() {
	return *m.get();
}

}// namespace libmumble::server

#endif//LIBMUMBLE_SERVER_PIMPL_IMPL_HPP
