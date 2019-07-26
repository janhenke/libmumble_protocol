//
// Created by jan on 20.06.19.
//

#include <cstddef>
#include <cstdint>
#include <memory>

#include "mumble_client_protocol_export.h"

#ifndef LIBMUMBLE_CLIENT_VOICE_DATA_HPP
#define LIBMUMBLE_CLIENT_VOICE_DATA_HPP

namespace mumble_client::protocol::voice {

	class MUMBLE_CLIENT_PROTOCOL_EXPORT ping_packet {
	public:
		ping_packet();

		virtual ~ping_packet();

	private:
		struct impl;
		std::unique_ptr<impl> pImpl;
	};

	std::byte *decode_varint(std::byte *start, const std::byte *limit, int64_t &result);

	std::byte *encode_varint(std::byte *start, const std::byte *limit, int64_t value);
}

#endif //LIBMUMBLE_CLIENT_VOICE_DATA_HPP
