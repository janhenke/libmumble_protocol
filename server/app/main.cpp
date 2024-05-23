//
// Created by JanHe on 03.04.2024.
//

#include <cstdlib>
#include <iostream>
#include <string_view>

#include <server.hpp>

#include <boost/program_options.hpp>
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

class PostgreSqlPersistence final : public libmumble_protocol::server::ServerStatePersistence {

	pqxx::connection connection_;

   public:
	explicit PostgreSqlPersistence(std::string_view url) : connection_(pqxx::zview{url}){};
	~PostgreSqlPersistence() override = default;

   protected:
	void dummy() override{};
};

auto main(int argc, char *argv[]) -> int {
	std::uint16_t port = 0;
	std::string cert_file;
	std::string key_file;
	bool generate_missing_certificate;
	std::string database_url;

	boost::program_options::options_description description{"libmumble_server example application"};
	description.add_options()("help,h", "display help message");
	description.add_options()("port,p",
							  boost::program_options::value<std::uint16_t>(&port)->default_value(
								  libmumble_protocol::server::MumbleServer::defaultPort),
							  "port number to use");
	description.add_options()("cert,c", boost::program_options::value<std::string>(&cert_file),
							  "Location of the certificate file");
	description.add_options()("key,k", boost::program_options::value<std::string>(&key_file),
							  "Location of the key file");
	description.add_options()("generate-cert", boost::program_options::bool_switch(&generate_missing_certificate),
							  "generate TLS certificate if the specified file is missing");
	description.add_options()("database,d", boost::program_options::value<std::string>(&database_url),
							  "Connection URL for the postgresql database");

	boost::program_options::variables_map variables_map;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variables_map);
	boost::program_options::notify(variables_map);

	if (variables_map.count("help") != 0U) {
		std::cout << description << '\n';
		return EXIT_SUCCESS;
	}

#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	PostgreSqlPersistence persistence{database_url};
	libmumble_protocol::server::MumbleServer mumble_server{persistence, {cert_file}, {key_file}};

	return EXIT_SUCCESS;
}
