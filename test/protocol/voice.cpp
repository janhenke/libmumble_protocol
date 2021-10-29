//
// Created by jan on 19.08.20.
//

#include <array>
#include <cstdint>

#include <boost/test/unit_test.hpp>

#include <protocol/util.h>

template<std::size_t N>
void test_decode_varint(const std::array<std::byte, N> &data, int64_t expected) {

	const auto[result, count] = mumble_client::protocol::voice::decode_varint(data);

	BOOST_REQUIRE_EQUAL(result, expected);
	BOOST_REQUIRE_EQUAL(count, std::size(data));
}

BOOST_AUTO_TEST_CASE(decode_varint) {

	test_decode_varint(std::array{std::byte{0b0111'1111}}, 0x0000007fLL);

	test_decode_varint(std::array{std::byte{0b1011'1111}, std::byte{0x00}}, 0x00003f00LL);

	test_decode_varint(std::array{std::byte{0b1101'1111}, std::byte{0x00}, std::byte{0xff}}, 0x001f00ffLL);

	test_decode_varint(std::array{std::byte{0b1110'1111}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}},
	                   0x0f00ff00LL);

	test_decode_varint(std::array{std::byte{0b1111'0000},
	                              std::byte{0x00},
	                              std::byte{0xff},
	                              std::byte{0x00},
	                              std::byte{0xff}},
	                   0x00ff00ffULL);

	test_decode_varint(std::array{std::byte{0b1111'0100},
	                              std::byte{0x00},
	                              std::byte{0xff},
	                              std::byte{0x00},
	                              std::byte{0xff},
	                              std::byte{0x00},
	                              std::byte{0xff},
	                              std::byte{0x00},
	                              std::byte{0xff}},
	                   0x00ff00ff00ff00ffULL);

	test_decode_varint(std::array{std::byte{0b1111'1000}, std::byte{0b0111'1111}}, ~0x0000007fLL);

	test_decode_varint(std::array{std::byte{0b1111'1101}}, ~0x0000001LL);
}

// required by the test macro below
namespace std {
	std::ostream &operator<<(std::ostream &stream, const std::byte byte) {
		stream << std::hex << std::to_integer<int>(byte);
		return stream;
	}
}

template<std::size_t N>
void test_encode_varint(int64_t value, const std::array<std::byte, N> &expected_data) {

	std::array<std::byte, N> buffer{};
	auto begin = std::begin(buffer);
	auto end = std::end(buffer);
	const auto count = mumble_client::protocol::voice::encode_varint(value, buffer);

	BOOST_REQUIRE_EQUAL_COLLECTIONS(std::begin(buffer),
	                                std::end(buffer),
	                                std::begin(expected_data),
	                                std::end(expected_data));
	BOOST_REQUIRE_EQUAL(count, std::size(expected_data));
}

BOOST_AUTO_TEST_CASE(encode_varint) {

	test_encode_varint(~0x0000001LL, std::array{std::byte{0b1111'1101}});

	test_encode_varint(~0x0000007fLL, std::array{std::byte{0b1111'1000}, std::byte{0b0111'1111}});

	test_encode_varint(0x0000007fLL, std::array{std::byte{0b0111'1111}});

	test_encode_varint(0x00003f00LL, std::array{std::byte{0b1011'1111}, std::byte{0x00}});

	test_encode_varint(0x001f00ffLL, std::array{std::byte{0b1101'1111}, std::byte{0x00}, std::byte{0xff}});

	test_encode_varint(0x0f00ff00LL,
	                   std::array{std::byte{0b1110'1111}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}});

	test_encode_varint(0xff00ff00ULL,
	                   std::array{std::byte{0b1111'0000},
	                              std::byte{0xff},
	                              std::byte{0x00},
	                              std::byte{0xff},
	                              std::byte{0x00}});

	test_encode_varint(0x7f00ff00ff00ff00ULL,
	                   std::array{std::byte{0b1111'0100},
	                              std::byte{0x7f},
	                              std::byte{0x00},
	                              std::byte{0xff},
	                              std::byte{0x00},
	                              std::byte{0xff},
	                              std::byte{0x00},
	                              std::byte{0xff},
	                              std::byte{0x00}});

	test_encode_varint(-2, std::array{std::byte{0b1111'1101}});

	test_encode_varint(-5, std::array{std::byte{0b1111'1000}, std::byte{4}});
}
