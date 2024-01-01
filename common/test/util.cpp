//
// Created by jan on 30.12.23.
//

#include <util.hpp>

#include <array>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Test the readIntegerFromNetworkBuffer function", "[common]") {

	SECTION("Read uint8_t") {

		const std::uint8_t expected = 0xff;
		const auto data = std::array<const std::byte, sizeof(std::uint8_t)>{std::byte{0xff}};

		const auto result = libmumble_protocol::common::readIntegerFromNetworkBuffer<std::uint8_t>(data);

		REQUIRE(sizeof(result) == sizeof(expected));
		REQUIRE(result == expected);
	}

	SECTION("Read uint16_t") {

		const std::uint16_t expected = 0xff00;
		const auto data = std::array<const std::byte, sizeof(std::uint16_t)>{std::byte{0xff}, std::byte{0x00}};

		const auto result = libmumble_protocol::common::readIntegerFromNetworkBuffer<std::uint16_t>(data);

		REQUIRE(sizeof(result) == sizeof(expected));
		REQUIRE(result == expected);
	}

	SECTION("Read uint32_t") {

		const std::uint32_t expected = 0xdeadff00;
		const auto data = std::array<const std::byte, sizeof(std::uint32_t)>{std::byte{0xde}, std::byte{0xad},
																			 std::byte{0xff}, std::byte{0x00}};

		const auto result = libmumble_protocol::common::readIntegerFromNetworkBuffer<std::uint32_t>(data);

		REQUIRE(sizeof(result) == sizeof(expected));
		REQUIRE(result == expected);
	}

	SECTION("Read uint64_t") {

		const std::uint64_t expected = 0xcafebabe'deadf0f0;
		const auto data = std::array<const std::byte, sizeof(std::uint64_t)>{
			std::byte{0xca}, std::byte{0xfe}, std::byte{0xba}, std::byte{0xbe},
			std::byte{0xde}, std::byte{0xad}, std::byte{0xf0}, std::byte{0xf0}};

		const auto result = libmumble_protocol::common::readIntegerFromNetworkBuffer<std::uint64_t>(data);

		REQUIRE(sizeof(result) == sizeof(expected));
		REQUIRE(result == expected);
	}
}

TEST_CASE("Test the writeIntegerToNetworkBuffer function", "[common]") {

	SECTION("Write uint8_t") {

		const std::uint8_t input = 0xff;
		const auto expected = std::array{std::byte{0xff}};
		std::array<std::byte, sizeof(std::uint8_t)> buffer{};

		const auto result = libmumble_protocol::common::writeIntegerToNetworkBuffer(buffer, input);

		REQUIRE(result == sizeof(input));
		REQUIRE(buffer == expected);
	}

	SECTION("Write uint16_t") {

		const std::uint16_t input = 0xff00;
		const auto expected = std::array{std::byte{0xff}, std::byte{0x0}};
		std::array<std::byte, sizeof(std::uint16_t)> buffer{};

		const auto result = libmumble_protocol::common::writeIntegerToNetworkBuffer(buffer, input);

		REQUIRE(result == sizeof(input));
		REQUIRE(buffer == expected);
	}

	SECTION("Write uint32_t") {

		const std::uint32_t input = 0xdeadff00;
		const auto expected = std::array{std::byte{0xde}, std::byte{0xad}, std::byte{0xff}, std::byte{0x00}};
		std::array<std::byte, sizeof(std::uint32_t)> buffer{};

		const auto result = libmumble_protocol::common::writeIntegerToNetworkBuffer(buffer, input);

		REQUIRE(result == sizeof(input));
		REQUIRE(buffer == expected);
	}

	SECTION("Write uint64_t") {

		const std::uint64_t input = 0xcafebabe'deadf0f0;
		const auto expected = std::array{std::byte{0xca}, std::byte{0xfe}, std::byte{0xba}, std::byte{0xbe},
										 std::byte{0xde}, std::byte{0xad}, std::byte{0xf0}, std::byte{0xf0}};
		std::array<std::byte, sizeof(std::uint64_t)> buffer{};

		const auto result = libmumble_protocol::common::writeIntegerToNetworkBuffer(buffer, input);

		REQUIRE(result == sizeof(input));
		REQUIRE(buffer == expected);
	}
}

TEST_CASE("Test the mumble protocol variable integer decode function", "[common]") {

	SECTION("Decode single byte") {
		const auto data = std::array{std::byte{0b0111'1111}};
		const std::int64_t expected = 0x000007fLL;

		const auto [bytes, result] = libmumble_protocol::common::decodeVariableInteger(data);

		REQUIRE(bytes == std::size(data));
		REQUIRE(result == expected);
	}

	SECTION("Decode two bytes") {
		const auto data = std::array{std::byte{0b1011'1111}, std::byte{0x00}};
		const std::int64_t expected = 0x00003f00LL;

		const auto [bytes, result] = libmumble_protocol::common::decodeVariableInteger(data);

		REQUIRE(bytes == std::size(data));
		REQUIRE(result == expected);
	}

	SECTION("Decode three bytes") {
		const auto data = std::array{std::byte{0b1101'1111}, std::byte{0x00}, std::byte{0xff}};
		const std::int64_t expected = 0x001f00ffLL;

		const auto [bytes, result] = libmumble_protocol::common::decodeVariableInteger(data);

		REQUIRE(bytes == std::size(data));
		REQUIRE(result == expected);
	}

	SECTION("Decode four bytes") {
		const auto data = std::array{std::byte{0b1110'1111}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}};
		const std::int64_t expected = 0x0f00ff00LL;

		const auto [bytes, result] = libmumble_protocol::common::decodeVariableInteger(data);

		REQUIRE(bytes == std::size(data));
		REQUIRE(result == expected);
	}

	SECTION("Decode five bytes") {
		const auto data =
			std::array{std::byte{0b1111'0000}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}};
		const std::int64_t expected = 0x00ff00ffULL;

		const auto [bytes, result] = libmumble_protocol::common::decodeVariableInteger(data);

		REQUIRE(bytes == std::size(data));
		REQUIRE(result == expected);
	}

	SECTION("Decode eight bytes") {
		const auto data =
			std::array{std::byte{0b1111'0100}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff},
					   std::byte{0x00},        std::byte{0xff}, std::byte{0x00}, std::byte{0xff}};
		const std::int64_t expected = 0x00ff00ff00ff00ffULL;

		const auto [bytes, result] = libmumble_protocol::common::decodeVariableInteger(data);

		REQUIRE(bytes == std::size(data));
		REQUIRE(result == expected);
	}

	SECTION("Decode inverted bytes") {
		const auto data = std::array{std::byte{0b1111'1000}, std::byte{0b0111'1111}};
		const std::int64_t expected = ~0x0000007fLL;

		const auto [bytes, result] = libmumble_protocol::common::decodeVariableInteger(data);

		REQUIRE(bytes == std::size(data));
		REQUIRE(result == expected);
	}

	SECTION("Decode small int optimization") {
		const auto data = std::array{std::byte{0b1111'1101}};
		const std::int64_t expected = ~0x0000001LL;

		const auto [bytes, result] = libmumble_protocol::common::decodeVariableInteger(data);

		REQUIRE(bytes == std::size(data));
		REQUIRE(result == expected);
	}
}

TEST_CASE("Test the mumble protocol variable integer encode function", "[common]") {

	SECTION("Encode one byte") {
		const std::int64_t value = 0x0000007fLL;
		const auto expectedData = std::array{std::byte{0b0111'1111}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode two bytes") {
		const std::int64_t value = 0x00003f00LL;
		const auto expectedData = std::array{std::byte{0b1011'1111}, std::byte{0x00}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode three bytes") {
		const std::int64_t value = 0x001f00ffLL;
		const auto expectedData = std::array{std::byte{0b1101'1111}, std::byte{0x00}, std::byte{0xff}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode four bytes") {
		const std::int64_t value = 0x0f00ff00LL;
		const auto expectedData = std::array{std::byte{0b1110'1111}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode five bytes") {
		const std::int64_t value = 0xff00ff00ULL;
		const auto expectedData =
			std::array{std::byte{0b1111'0000}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode eight bytes") {
		const std::int64_t value = 0x7f00ff00ff00ff00ULL;
		const auto expectedData =
			std::array{std::byte{0b1111'0100}, std::byte{0x7f}, std::byte{0x00}, std::byte{0xff}, std::byte{0x00},
					   std::byte{0xff},        std::byte{0x00}, std::byte{0xff}, std::byte{0x00}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode small int optimization") {
		const std::int64_t value = ~0x0000001LL;
		const auto expectedData = std::array{std::byte{0b1111'1101}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode inverted bytes") {
		const std::int64_t value = ~0x0000007fLL;
		const auto expectedData = std::array{std::byte{0b1111'1000}, std::byte{0b0111'1111}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode -2") {
		const std::int64_t value = -2;
		const auto expectedData = std::array{std::byte{0b1111'1101}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}

	SECTION("Encode -5") {
		const std::int64_t value = -5;
		const auto expectedData = std::array{std::byte{0b1111'1000}, std::byte{4}};
		std::array<std::byte, sizeof(expectedData)> buffer{};

		const auto count = libmumble_protocol::common::encodeVariableInteger(buffer, value);

		REQUIRE(count == std::size(expectedData));
		REQUIRE(buffer == expectedData);
	}
}
