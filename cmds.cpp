#include "irc.hpp"

// void Server::topic(int fd, std::vector<std::string> args){

// }

// void Server::mode(int fd, std::vector<std::string> args){

// }

// void Server::kick(int fd, std::vector<std::string> args){

// }

// void Server::invite(int fd, std::vector<std::string> args){

// }

///msg <nickname> <message> (private message)
void Server::msg(int fd, std::vector<std::string> args){
    if (args.size() != 2)
        {send(fd, "Try /join /msg <nickname> <message>\n", 37, 0); return ; }
    size_t i;
    for (i = 0; i < clients.size(); i++) {
        if (clients[i].nick == args[0]) {
            send(clients[i].fd, (args[1] + "\n").c_str(), args[1].size() + 1, 0);
            return ;
        }}
    send(fd, "Error, nick isnt avaliable.\n", 27, 0);
}

// /join #channel
//send to all clients in the channel
//join/create:admin if channel doesnt exist
void Server::join(int fd, std::vector<std::string> args) {
    if (args.size() != 1 || args[0][0] != '#')
       {send(fd, "Try /join #channel\n", 21, 0); return ; } 

    size_t pos = args[0].find('#'); //removing #
        if (pos != std::string::npos)
            args[0].erase(pos, 1);

    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            clients[i].channel = args[0];   
            std::string mess = "Currently in #" + args[0] + "\n";
            send(fd, mess.c_str(), mess.size(), 0);
            std::cout << get_nick(fd) << ": joined #" << args[0] << std::endl;
            return ;
    }}
}
