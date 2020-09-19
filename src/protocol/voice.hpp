//
// Created by jan on 20.06.19.
//

#ifndef LIBMUMBLE_CLIENT_VOICE_DATA_HPP
#define LIBMUMBLE_CLIENT_VOICE_DATA_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

#if __has_include(<span>)
#include <span>
using std::span;
#else

#include <gsl/span>

using gsl::span;
#endif

#include "mumble_client_export.h"

namespace mumble_client::protocol::voice {

	class MUMBLE_CLIENT_EXPORT ping_packet {
	public:
		ping_packet();

		virtual ~ping_packet();

	private:
		struct impl;
		std::unique_ptr<impl> pImpl;
	};

	MUMBLE_CLIENT_EXPORT
	std::tuple<int64_t, const std::size_t> decode_varint(span<const std::byte> buffer);

	MUMBLE_CLIENT_EXPORT
	std::size_t encode_varint(int64_t value, span<std::byte> buffer);
}

#endif //LIBMUMBLE_CLIENT_VOICE_DATA_HPP
