#include <fcntl.h>
#include <iostream>
#include <string>
#include <cstring>
#include <climits>
#include <algorithm>
#include <cstdlib>

#include <vector> 
#include <sys/socket.h> 
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> 
#include <csignal> 

class Client {
    private :


    public :
        Client();
        ~Client();
};

class Server {
    private :

    public :
        int port;
        int ser_socket_fd;
        static bool signal;
        std::vector<Client> clients;
        std::vector<struct pollfd> fds; //pollfd is used in poll() to monitor fds (sockets) for events
        void loop();
        void close_fd();
        void init_socket();
        static void signal_handler(int signum);
        Server();
        ~Server();

};

Server::~Server() {}
Server::Server() {}

Client::~Client() {}
Client::Client() {}

void Server::close_fd() {

}

void Server::signal_handler(int signum) {
	(void)signum;
	std::cout << std::endl << "Signal Received" << std::endl;
	signal = true;
}

void Server::loop() {
    while (signal == false) {
        if (poll(&fds[0], ) == -1)
            {std::cerr << "poll() failed.\n"; return;}
        
    }
    close_fd();
}

void Server::init_socket() {
    int	s = socket(AF_INET, SOCK_STREAM, 0); //create server socket
	if (s == -1) {std::cerr << "Creation of socket failed.\n"; return;}
    struct sockaddr_in	sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    int en = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //allow the server to bind to the same address even if it is currently in use by another socket
		{std::cerr << "Reuse address of socket failed.\n"; return;}
	if (fcntl(s, F_SETFL, O_NONBLOCK) == -1) //non-blocking socket
		{std::cerr << "Non blocking socket failed.\n"; return;}
    if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) == -1) {std::cerr << "Binding failed.\n"; return;}
    if (listen(s, SOMAXCONN) == -1) {std::cerr << "Binding failed.\n"; return;}
    struct pollfd poll; 
    poll.fd = s; 
	poll.events = POLLIN; //POLLIN for reading data
	poll.revents = 0;
	fds.push_back(poll); //add server socket to pollfd
}



int main(int argc, char **argv) {
    if (argc != 2) {std::cerr << "Invalid number of arguments.\n"; return 1;}
    char* endptr;
    long num = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1]|| num > 65535 || num < 1024) {std::cerr << "Error: invalid port.\n"; return 1;}
    Server server;
    server.port = atoi(argv[1]);
    signal(SIGINT, Server::signal_handler);
    signal(SIGQUIT, Server::signal_handler); //ctrl + "\""
    server.init_socket();
	server.loop();
	std::cout << "Server closed." << std::endl;
    server.close_fd();
}