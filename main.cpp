#include "irc.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {std::cerr << "Invalid number of arguments.\n"; return 1;}
    char* endptr;
    long num = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1]|| num > 65535 || num < 1024) {std::cerr << "Error: invalid port.\n"; return 1;}
    
    Server server;
    signal(SIGINT, Server::signal_handler);
    signal(SIGQUIT, Server::signal_handler); //ctrl + "\""
    server.port = atoi(argv[1]);
    server.password = argv[2];
    std::cout << "Server running." << std::endl;
    server.init_socket();
	server.loop();
	std::cout << "Server closed." << std::endl;
}