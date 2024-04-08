//
// Created by Jan on 01.01.2024.
//

#include <client.hpp>

#include <boost/program_options.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

auto main(int argc, char *argv[]) -> int {
	using namespace std::chrono_literals;
	using namespace libmumble_protocol::client;

	std::string server_name;
	std::uint16_t port = 0;
	std::string user_name;
	bool ignore_server_cert;

	boost::program_options::options_description description{"libmumble_client example application"};
	description.add_options()("help,h", "display help message");
	description.add_options()("server,s", boost::program_options::value<std::string>(&server_name),
							  "server to connect to");
	description.add_options()(
		"port,p", boost::program_options::value<std::uint16_t>(&port)->default_value(MumbleClient::defaultPort),
		"port number to use");
	description.add_options()("username,u", boost::program_options::value<std::string>(&user_name),
							  "user name to connect as");
	description.add_options()("ignore-cert", boost::program_options::bool_switch(&ignore_server_cert),
							  "do not validate the TLS server certificate");

	boost::program_options::variables_map variablesMap;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variablesMap);
	boost::program_options::notify(variablesMap);

	if (variablesMap.count("help")) {
		std::cout << description << '\n';
		return EXIT_SUCCESS;
	}

#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	for (MumbleClient mumbleClient(server_name, port, user_name, !ignore_server_cert);;) {
		std::this_thread::sleep_for(1s);
	}

	return EXIT_SUCCESS;
}
