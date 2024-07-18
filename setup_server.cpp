#include <fcntl.h>
#include <iostream>
#include <sstream>
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
    public :
        Client();
        ~Client();
        int fd;
        std::string ip;
};

class Server {
    public :
        int port;
        int socketfd;
        static bool signal;
        std::vector<Client> clients;
        std::vector<struct pollfd> fds; //pollfd is used in poll() to monitor fds (sockets) for events
        void loop();
        void new_client();
        void clear_client(int fd);
        void new_data(int fd);
        void close_fd();
        void init_socket();
        static void signal_handler(int signum);
        Server();
        ~Server();
};

bool Server::signal = false;

Server::~Server() {}
Server::Server() {}

Client::~Client() {}
Client::Client() {}

void Server::clear_client(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd)
            {clients.erase(clients.begin() + i); break ;}
    }
    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i].fd == fd)
            {fds.erase(fds.begin() + i); break ;}
    }
}

//If you close the file descriptor via the client, you do not need to close it again in fds vector
void Server::close_fd() {
    for (size_t i = 0; i < clients.size(); i++) {
        std::cout << "Client " << clients[i].fd << " disconnected" << std::endl;
        close(clients[i].fd);
    }
    std::cout << "Server " << socketfd << " disconnected" << std::endl;
    close(socketfd);
}



//client also has a socket (fd)
void Server::new_client() {
    struct sockaddr_in	csin;
    socklen_t csin_len = sizeof(csin);
    int fd = accept(socketfd, (struct sockaddr*)&csin, &csin_len);
    if (fd == -1) {std::cerr << "Accept failed.\n"; return;}
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) //non-blocking socket
		{std::cerr << "Non-blocking failed" << std::endl; return;}
    std::cout << "New client " << fd <<  " from " << inet_ntoa(csin.sin_addr) << ": " << ntohs(csin.sin_port) << std::endl;

    struct pollfd poll; 
    poll.fd = fd; 
	poll.events = POLLIN; //POLLIN for reading data
	poll.revents = 0;

    Client client;
    client.fd = fd;
    client.ip = inet_ntoa(csin.sin_addr);
    clients.push_back(client);
	fds.push_back(poll);
}


void Server::new_data(int fd) {
    char buf[4097];
    int r = recv(fd, buf, 4096, 0);
    if (r <= 0) {
        close(fd);
        clear_client(fd);
        std::cout << "Client " << fd << ": went away\n";
    }
    else {
        buf[r] = '\0';
        std::cout << "Client " << fd << ": " << buf << std::endl;
        // for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)  {
        //     std::stringstream ss;
        //     ss << fd;
        //     if ((*it).fd != fd) { //don't send data back to the original client
        //         std::string message = "Client " + ss.str() + ": " + buf;
        //         send((*it).fd, message.c_str(), message.size(), 0); }
        // }
}}


void Server::loop() {
    while (Server::signal == false) {
        if (poll(&fds[0], fds.size(), -1) == -1 && Server::signal == false) //block indefinitely until an event occurs 
            {std::cerr << "poll() failed.\n"; return;}
        for (int i = 0; i < (int)fds.size(); i++) {
            if (fds[i].revents & POLLIN) { //revents is updated by poll(), if read(pollin) bit is set in revents, revents can have multiple bits set, representing different types of event
                if (fds[i].fd == socketfd) //POLLIN on the server socket file descriptor indicates that there is a new incoming connection, not new data
                    new_client();
                else {new_data(fds[i].fd);}
            }}}
    close_fd();
}


void Server::init_socket() {

    socketfd = socket(AF_INET, SOCK_STREAM, 0); //create server socket
	if (socketfd == -1) {std::cerr << "Creation of socket failed.\n"; return;}
    struct sockaddr_in	sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    int en = 1;

    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //allow the server to bind to the same address even if it is currently in use by another socket
		{std::cerr << "Reuse address of socket failed.\n"; return;}
	if (fcntl(socketfd, F_SETFL, O_NONBLOCK) == -1) //non-blocking socket
		{std::cerr << "Non blocking socket failed.\n"; return;}
    if (bind(socketfd, (struct sockaddr*)&sin, sizeof(sin)) == -1) {std::cerr << "Binding failed.\n"; return;}
    if (listen(socketfd, SOMAXCONN) == -1) {std::cerr << "Binding failed.\n"; return;}

    struct pollfd poll; 
    poll.fd = socketfd; 
	poll.events = POLLIN; //POLLIN for reading data
	poll.revents = 0;
	fds.push_back(poll); //add server socket to pollfd
}



void Server::signal_handler(int signum) {
	(void)signum;
	std::cout << std::endl << "Signal Received" << std::endl;
	Server::signal = true;
}


int main(int argc, char **argv) {
    if (argc != 2) {std::cerr << "Invalid number of arguments.\n"; return 1;}
    char* endptr;
    long num = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1]|| num > 65535 || num < 1024) {std::cerr << "Invalid port.\n"; return 1;}
    Server server;
    server.port = atoi(argv[1]);

    std::cout << "Server running." << std::endl;
    signal(SIGINT, Server::signal_handler);
    signal(SIGQUIT, Server::signal_handler); //ctrl + "\""
    server.init_socket();
	server.loop();
	std::cout << "Server closed." << std::endl;
}