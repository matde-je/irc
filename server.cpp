#include "irc.hpp"

Server::~Server() {}
Server::Server() {}

bool Server::signal = false; //has to be in the file

void Server::signal_handler(int signum) {
	(void)signum;
	std::cout << std::endl << "Signal Received" << std::endl;
	Server::signal = true;
}

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

//if you close the file descriptor via the client, you do not need to close it again in fds vector
void Server::close_fds() {
    for (size_t i = 0; i < clients.size(); i++) {
        std::cout << get_nick(clients[i].fd) << " disconnected" << std::endl;
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

void Server::parse(int fd, char *buf) {
    std::string str = buf;
    if (!str.empty() && str[str.length() - 1] == '\n') 
        str.erase(str.length() - 1); 
    if (!str.empty() && str[str.length() - 1] == '\r') 
        str.erase(str.length() - 1); 
    //std::cout << buf << std::endl;
    std::stringstream ss(str);
    std::string command;
    while (std::getline(ss, command)) { //split the string into individual commands based on '\n'
        if (!command.empty() && command[command.length() - 1] == '\r') 
            command.erase(command.length() - 1);
        size_t start = command.find_first_not_of("\t\v "); 
        if (start == std::string::npos) 
            continue; //no command provided
        std::string cmd = command.substr(start); //extract the command
        std::vector<std::string> arglist;
        size_t end = cmd.find_first_of(" ");
        if (end != std::string::npos) {
            std::string args = cmd.substr(end + 1); //get the rest of the string
            //std::cout << "passed: " << cmd << " " << args << std::endl;
            std::stringstream argStream(args);
            std::string arg;
            while (std::getline(argStream, arg, ' ')) {
                if (!arg.empty())
                    arglist.push_back(arg); }
            send_cmd(fd, cmd.substr(0, end), arglist);
    }}
}



void Server::loop() {
    while (Server::signal == false) {
        if (poll(&fds[0], fds.size(), -1) == -1 && Server::signal == false) //block indefinitely until an event occurs 
            {std::cerr << "poll() failed.\n"; return;}
        for (int i = 0; i < (int)fds.size(); i++) {
            if (fds[i].revents & POLLIN) { //revents is updated by poll(), if read(pollin) bit is set in revents, revents can have multiple bits set, representing different types of event
                if (fds[i].fd == socketfd) //POLLIN on the server socket file descriptor indicates that there is a new incoming connection, not new data
                    new_client();
                else {
                    char buf[4097];
                    int r = recv(fds[i].fd, buf, 4096, 0); //receive new data from fd that changed
                    if (r <= 0) {
                        std::cout << get_nick(fds[i].fd) << ": went away\n";
                        clear_client(fds[i].fd);
                        close(fds[i].fd);
                    }
                    else {
                        buf[r] = '\0';
                        parse(fds[i].fd, buf);}
        }}}}
    close_fds();
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
    if (listen(socketfd, SOMAXCONN) == -1) {std::cerr << "Listen failed.\n"; return;}

    struct pollfd poll; 
    poll.fd = socketfd; 
	poll.events = POLLIN; //POLLIN for reading data
	poll.revents = 0;
	fds.push_back(poll); //add server socket to pollfd
}

void Server::showClients(){
    for (int i = 0; i < clients.size(); i++)
    {
        std::cout << clients[i].fd << " " << clients[i].ip << std::endl;
    }
}