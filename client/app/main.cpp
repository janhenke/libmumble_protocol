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

int main(int argc, char *argv[]) {
	using namespace std::chrono_literals;
	using namespace libmumble_protocol::client;

	std::string serverName;
	std::uint16_t port = 0;
	std::string userName;
	bool ignoreServerCert;

	boost::program_options::options_description description{"libmumble_client example application"};
	description.add_options()("help,h", "display help message");
	description.add_options()("server,s", boost::program_options::value<std::string>(&serverName),
							  "server to connect to");
	description.add_options()(
		"port,p", boost::program_options::value<std::uint16_t>(&port)->default_value(MumbleClient::s_defaultPort),
		"port number to use");
	description.add_options()("username,u", boost::program_options::value<std::string>(&userName),
							  "user name to connect as");
	description.add_options()("ignore-cert", boost::program_options::bool_switch(&ignoreServerCert),
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

	for (MumbleClient mumbleClient(serverName, port, userName, !ignoreServerCert);;) {
		std::this_thread::sleep_for(1s);
	}

	return EXIT_SUCCESS;
}
