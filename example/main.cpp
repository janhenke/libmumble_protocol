#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

int main(int argc, char *argv[]) {

	boost::program_options::options_description description{"libmumble_client example application"};
	description.add_options()
			("help,h", "display help message")
			("server,s", boost::program_options::value<std::string>(), "server to connect to")
			("ignore-cert", "do not validate the TLS server certificate");

	boost::program_options::variables_map variablesMap;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variablesMap);
	boost::program_options::notify(variablesMap);

	if (variablesMap.count("help")) {
		std::cout << description << std::endl;
		return EXIT_SUCCESS;
	}

	const std::string serverName = variablesMap["server"].as<std::string>();
	const bool ignoreServerCert = variablesMap.count("ignore-cert");

	return EXIT_SUCCESS;
}

