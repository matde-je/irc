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
//open new page
void Server::msg(int fd, std::vector<std::string> args){
    if (is_authentic(fd) == 1) {return ;}
    if (args.size() != 2)
        {send(fd, "Try /msg <nickname> <message>\r\n", 32, 0); return ; }
    size_t i;
    for (i = 0; i < clients.size(); i++) {
        if (clients[i].nick == args[0]) {
            std::string str = get_nick(fd) + " sent you a private message: " + args[1] + "\r\n";
            send(clients[i].fd, str.c_str(), str.size(), 0);
            return ;
        }}
    send(fd, "Error, nick isnt avaliable.\r\n", 30, 0);
}

// /join #channel
//send to all clients in the channel
//join/create:admin if channel doesnt exist
//open another file with channel
void Server::join(int fd, std::vector<std::string> args) {
    if (is_authentic(fd) == 1) {return ;}
    if (args.size() != 1 || args[0][0] != '#')
       {send(fd, "Try /join #channel\r\n", 21, 0); return ; } 

    size_t pos = args[0].find('#'); //removing #
        if (pos != std::string::npos)
            args[0].erase(pos, 1);

    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            clients[i].channel = args[0];   
            std::string mess = "Currently in #" + args[0] + "\r\n";
            send(fd, mess.c_str(), mess.size(), 0);
            std::cout << clients[i].nick << ": joined #" << args[0] << std::endl;
            for (size_t j = 0; j < clients.size(); j++) {
                if (clients[j].channel == args[0] && clients[j].fd != fd) {
                    std::string notify = clients[i].nick + " has joined " + args[0] + "\r\n";
                    send(clients[j].fd, notify.c_str(), notify.size(), 0);
                }
            }
            return ;
    }}
}

