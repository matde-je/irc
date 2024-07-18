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
//send to all clients in the channel
//join/create:admin if channel doesnt exist
int Server::join(int fd, std::vector<std::string> args){
    if (args[0][0] != '#' || args.size() != 1)
       {send(fd, "Try /join #channel\n", 21, 0); return 0; } 

    size_t pos = args[0].find('#'); //removing #
        if (pos != std::string::npos)
            args[0].erase(pos, 1);
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            clients[i].channel = args[0];   
            std::string mess = "Currently in #" + args[0] + "\n";
            send(fd, mess.c_str(), mess.size(), 0);
            std::cout << get_nick(fd) << ": joined #" << args[0] << std::endl;
            return 1;
    }}
    return 0;
}
