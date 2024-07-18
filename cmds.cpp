#include "irc.hpp"

// void Server::topic(int fd, std::vector<std::string> args){

// }

// void Server::mode(int fd, std::vector<std::string> args){

// }

// void Server::kick(int fd, std::vector<std::string> args){

// }

// void Server::invite(int fd, std::vector<std::string> args){

// }

// void Server::msg(int fd, std::vector<std::string> args){

// }

// /join #channel
void Server::join(int fd, std::vector<std::string> args){
    // code/write this  join/create:admin if doesnt exist channel (args[0])

    std::cout << "Client " << fd << ": joined channel" << args[0] << std::endl;
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
                break ;
            }}}
    else {
        send(fd, "Try: user <username> <mode> <unused> <realname>\n", 49, 0); return; }
    std::cout << "Client " << fd << ": changed name to " << args[0] << std::endl;
}

void    Server::pass(int fd, std::vector<std::string> args){
    size_t pos = args[0].find('\n'); //removing nl
    if (pos != std::string::npos)
        args[0].erase(pos, 1);
    if (args.size() == 1 && !args[0].compare(password)) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].pass = true;
                break ;
            }}}
    else if (args.size() == 1) {
        send(fd, "Wrong password.\n", 17, 0); return; }
}

void    Server::cmd_parse(int fd, std::string cmd , std::vector<std::string> args) {
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