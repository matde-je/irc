#include "../incs/irc.hpp"

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
        if (clients[i].getFd() == fd)
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
        std::cout << get_nick(clients[i].getFd()) << " disconnected" << std::endl;
        close(clients[i].getFd());
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
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking mode for client socket.\n";
        close(fd);
        return;
    }
    std::cout << "New client " << fd <<  " from " << inet_ntoa(csin.sin_addr) << ": " << ntohs(csin.sin_port) << std::endl;

    struct pollfd poll; 
    poll.fd = fd; 
	poll.events = POLLIN; //POLLIN for reading data
	poll.revents = 0;

    Client client;
    client.setFd(fd);
    client.getIp() = inet_ntoa(csin.sin_addr);
    clients.push_back(client);
	fds.push_back(poll);
}

void Server::parse(int fd, const char *buf) {
    partial += buf;
    std::string str = partial;
    if (!str.empty() && str[str.length() - 1] == '\n') {
        std::stringstream ss(str);
        std::string command;
        while (std::getline(ss, command, '\n')) {
            if (!command.empty() && command[command.length() - 1] == '\r') 
                command.erase(command.length() - 1);
            size_t start = command.find_first_not_of("\t\v "); 
            if (start == std::string::npos) 
                continue;
            std::string cmd = command.substr(start);
            std::vector<std::string> arglist;
            size_t end = cmd.find_first_of(" ");
            if (end != std::string::npos) {
                std::string args = cmd.substr(end + 1);
                std::stringstream argStream(args);
                std::string arg;
                while (std::getline(argStream, arg, ' ')) {
                    if (!arg.empty())
                        arglist.push_back(arg); }
                send_cmd(fd, cmd.substr(0, end), arglist);
            } else {
                send_cmd(fd, cmd, arglist);
            }
        }
        partial.clear();
}}


void Server::loop() {
    while (Server::signal == false) {
        if (poll(&fds[0], fds.size(), -1) == -1 && Server::signal == false) //block indefinitely until an event occurs 
            {std::cerr << "poll() failed.\n"; return;}
        for (int i = 0; i < (int)fds.size(); i++) {
            if (fds[i].revents & POLLIN) {
                 //revents is updated by poll(), if read(pollin) bit is set in revents, revents can have multiple bits set, representing different types of event
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

Channel *Server::getChannelFromName(std::string name) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].getName() == name) {
            return &channels[i];
        }
    }
    return NULL;
}

//SHOW <channelname>
void Server::showClients(int fd,  std::vector<std::string> args) {
    Client client;
    for (std::size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].getFd() == fd)
            {client = clients[i]; break ;}
    }
    Channel *channel = getChannelFromName(args[0]);
    if (channel == NULL){
        send(fd,( "you" + client.getName() + " are not in a channel\r\n").c_str(), 26 + client.getName().length(), 0);
        return;
    }
    std::cout << "Client " << client.getNick() << " requested to show clients" << std::endl;
    for (std::size_t i = 0;i < channel->getUsers().size(); i++)
    {
            send(fd, channel->getUsers()[i].getNick().c_str(),channel->getUsers()[i].getNick().length(), 0);
            send(fd, "\r\n", 2, 0);
            
    }
}

void Server::showChannels(int fd){
    Client *client = getClientFromFD(fd);
    for (size_t i = 0; i < channels.size(); i++){
        if(channels[i].getName() == client->getName()){
            send(fd, "You are in this CHANNEL------>", 30, 0);
        }
        send(fd, ("channel "" :" + channels[i].getName() + "\r\n").c_str(), channels[i].getName().length() + 12, 0);
    }
}

Channel* Server::findOrMakeChannel(int *newChannel, std::string name) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].getName() == name) {
            *newChannel = 0;
            return &channels[i];
        }
    }
    channels.push_back(Channel(name));
    *newChannel = 1;
    return &channels.back();
}

Client *Server::getClientFromFD(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd) {
            return &clients[i];
        }
    }
    return NULL;
}

Client *Server::getClientFromNick(std::string nick) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getNick() == nick) {
            return &clients[i];
        }
    }
    return NULL;
}

