#include "irc.hpp"

Client::~Client() {}
Client::Client() {pass = false;}

std::string Server::get_nick(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            if (clients[i].nick != "\0")
                return clients[i].nick;
    }}
    std::stringstream ss;
    ss << fd;
    return ("Client " + ss.str());
}

//set /nick
void Server::nick(int fd, std::vector<std::string> args){
    if (is_authentic(fd) == 1) {return ;}
    if (args.size() == 1) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].nick = args[0];
                std::cout << "Client " << fd << ": changed nick to " << args[0] << std::endl;
                return ;
            }}}
}

//USER <username> <mode> <unused> <realname>
//USER myusername 0 0 My Real Name
void Server::user(int fd, std::vector<std::string> args){
    if (is_authentic(fd) == 1) {return ;}
    if (args.size() == 4) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].name = args[0];
                if (clients[i].nick != "\0")
                    std::cout << clients[i].nick << ": changed name to " << args[0] << std::endl;
                else {std::cout << "Client " << fd << ": changed name to " << args[0] << std::endl;}
                return ;
            }}}
    send(fd, "Try: USER <username> <mode> <unused> <realname>\r\n", 50, 0); 
}

void    Server::pass(int fd, std::vector<std::string> args){
    if (args.size() == 1 && args[0] == password ) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].pass = true;
                send(fd, "Authentication successful.\r\n", 29, 0);
                return ;
            }}}
    send(fd, "Wrong password.\r\n", 18, 0); 
}


int    Server::is_authentic(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                if (clients[i].pass == false) {
                    send(fd, "You must authenticate before joining a channel.\r\n", 50, 0); 
                    return 1; }
        }}
    return 0;
}

int    Server::send_cmd(int fd, std::string cmd, std::vector<std::string> args) {
    //std::cout << "Sending command: '" << cmd << "'" << std::endl;
    if (cmd == "PASS") 
        {pass(fd, args); return 1;}
    else if (cmd == "USER")
        {user(fd, args); return 1;}
    else if (cmd == "NICK")
        {nick(fd, args); return 1;}
    // else if (cmd == "/topic")
    //     {topic(fd, args); return 1;}
    // else if (cmd == "/mode")
    //     {mode(fd, args); return 1;}
    // else if (cmd == "/kick")
    //     {kick(fd, args); return 1;}
    // else if (cmd == "/invite")
    //     {invite(fd, args); return 1;}
    else if (cmd == "PRIVMSG")
        {msg(fd, args); return 1;}
    else if (cmd == "JOIN")
        {join(fd, args); return 1;}
    return 0;
}