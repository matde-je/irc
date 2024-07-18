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


void Server::nick(int fd, std::vector<std::string> args){
    if (args.size() == 1) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].nick = args[0];
                std::cout << "Client " << fd << ": changed nick to " << args[0] << std::endl;
                break ;
            }}}
    
}

//USER <username> <mode> <unused> <realname>
//USER myusername 0 0 My Real Name
void Server::user(int fd, std::vector<std::string> args){
    if (args.size() == 4) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].name = args[0];
                if (clients[i].nick != "\0")
                    std::cout << clients[i].nick << ": changed name to " << args[0] << std::endl;
                else {std::cout << "Client " << fd << ": changed name to " << args[0] << std::endl;}
                return ;
            }}}
    else {
        send(fd, "Try: user <username> <mode> <unused> <realname>\n", 49, 0); return; }
}

void    Server::pass(int fd, std::vector<std::string> args){
    if (args.size() == 1 && !args[0].compare(password)) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].pass = true;
                break ;
            }}}
    else if (args.size() == 1) {
        send(fd, "Wrong password.\n", 17, 0); return; }
}




void    Server::cmd_parse(int fd, std::string cmd, std::vector<std::string> args, bool ver) {
    if (cmd != "pass") {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                if (clients[i].pass == false) {
                    send(fd, "You need to enter the password to access the server.\n", 54, 0); 
                    return ; }
        }}}
    if (ver == true) {
        for (size_t i = 0; i < args.size(); i++) {
            size_t pos = args[i].find('\n'); //removing nl
            if (pos != std::string::npos)
                args[i].erase(pos, 1);
            }
        send_cmd(fd, cmd, args);
    }
}

void    Server::send_cmd(int fd, std::string cmd, std::vector<std::string> args) {
    if (cmd == "pass")
        pass(fd, args);
    else if (cmd == "USER")
        user(fd, args);
    else if (cmd == "/nick")
        nick(fd, args);
    // else if (cmd == "/topic")
    //     topic(fd, args);
    // else if (cmd == "/mode")
    //     mode(fd, args);
    // else if (cmd == "/kick")
    //     kick(fd, args);
    // else if (cmd == "/invite")
    //     invite(fd, args);
    // else if (cmd == "/msg")
    //     msg(fd, args);
    else if (cmd == "/join")
        join(fd, args);
}