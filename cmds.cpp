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
    //join/create:admin if channel doesnt exist
    if (args[0][0] != '#' || args.size() != 1)
       {send(fd, "Try /join #channel\n", 21, 0); return; } 
    std::cout << get_nick(fd) << ": joined " << args[0] << std::endl;
}

