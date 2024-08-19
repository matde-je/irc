#include "irc.hpp"

Client::~Client() {}
Client::Client() {pass = false; channel = NULL;}

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
    if (has_pass(fd) == 1) {return ;}
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
//check
void Server::user(int fd, std::vector<std::string> args){
    if (has_pass(fd) == 1) {return ;}
    if (args.size() == 4) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].name = args[0];
                std::cout << "Client " << fd << ": changed name to " << args[0] << std::endl;
                send(fd, ("NAME set to " + args[0] + "\r\n").c_str(), 13 + args[0].length(), 0);
                send(fd, "\r\n", 2, 0);
                return ;
            }}}
            
}

void    Server::pass(int fd, std::vector<std::string> args){
    if (args.size() == 1 && args[0] == password ) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].pass = true;
                send(fd, "Password correct.\r\n", 20, 0);
                return ;
            }}}
    send(fd, "Wrong password.\r\n", 18, 0); 
}

int    Server::has_pass(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            if (clients[i].pass == false) {
                send(fd, "You must enter password.\r\n", 27, 0); 
                return 1; }
        }}
    return 0;
}

int    Server::is_authentic(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            if (clients[i].pass == false) {
                send(fd, "You must enter password.\r\n", 27, 0); 
                return 1; }
            if (clients[i].name == "\0" || clients[i].nick == "\0") {
                send(fd, "You must authenticate before interacting.\r\n", 44, 0); 
                return 2; }
        }}
    return 0;
}

int    Server::send_cmd(int fd, std::string cmd, std::vector<std::string> args) {
    std::cout << "Sending command: '" << cmd << "'" << std::endl;
    if (cmd == "PASS") 
        {pass(fd, args); return 1;}
    else if (cmd == "USER")
        {user(fd, args); return 1;}
    else if (cmd == "NICK" || cmd == "/nick")
        {nick(fd, args); return 1;}
    else if (cmd == "TOPIC" || cmd == "/topic")
        {topic(fd, args); return 1;}
    else if (cmd == "MODE" || cmd == "/mode")
        {mode(fd, args); return 1;}
    else if (cmd == "KICK" || cmd == "/kick")
        {kick(fd, args); return 1;}
    else if (cmd == "INVITE" || cmd == "/invite")
        {invite(fd, args); return 1;}
    else if (cmd == "PRIVMSG" || cmd == "/msg")
        {msg(fd, args); return 1;}
    else if (cmd == "JOIN" || cmd == "/join")
        {join(fd, args); return 1;}
    else if (cmd == "SHOW" || cmd == "/show")
        {
            std::cout << "Showing clients\n";
            showClients(fd); return 1;}
    return 0;
}