//
// Created by JanHe on 03.04.2024.
//

#include <cstdlib>

#include <server.hpp>

int main(int argc, char *argv[]) {
	(void) argc;
	(void) argv;

	libmumble_protocol::server::MumbleServer mumble_server;

	return EXIT_SUCCESS;
}
