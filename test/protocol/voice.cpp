//
// Created by jan on 19.08.20.
//

#include <array>
#include <cstdint>

#include <boost/test/unit_test.hpp>

#include <protocol/voice.hpp>

template<std::size_t N>
void test_decode_varint(const std::array<std::byte, N> &encoded, int64_t expected) {

	const auto begin = std::cbegin(encoded);
	const auto end = std::cend(encoded);
	const auto[result, ptr] = mumble_client::protocol::voice::decode_varint(begin, end);

	BOOST_REQUIRE_EQUAL(result, expected);
	BOOST_REQUIRE_EQUAL(ptr, end);
}

BOOST_AUTO_TEST_CASE(decode_varint) {

	test_decode_varint(std::array{std::byte{0b0111'1111}}, 0x0000007fLL);

	test_decode_varint(std::array{std::byte{0b1011'1111}, std::byte{0x00}}, 0x00003f00LL);

	test_decode_varint(std::array{std::byte{0b1101'1111}, std::byte{0x00}, std::byte{0xff}}, 0x001f00ffLL);

	test_decode_varint(std::array{std::byte{0b1110'1111}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}},
	                   0x0f00ff00LL);

//	test_decode_varint(std::array{std::byte{0b1111'0011}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}},
//	                   0x00ff00ffLL);
}
