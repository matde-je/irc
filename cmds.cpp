#include "irc.hpp"

// void Server::topic(int fd, std::vector<std::string> args){

// }

// void Server::mode(int fd, std::vector<std::string> args){

// }

// void Server::kick(int fd, std::vector<std::string> args){

// }

// void Server::invite(int fd, std::vector<std::string> args){

// }

//  /msg <nickname> <message> (private message)
//or in channel
//open new page
void Server::msg(int fd, std::vector<std::string> args) {
    if (is_authentic(fd) == 1) {return ;}
    size_t i;
    if (args.size() >= 2) {
        for (i = 0; i < clients.size(); i++) {
            if (clients[i].nick == args[0]) {
                std::string content;
                std::string privmess;
                for (size_t k = 0; k < clients.size(); k++) {
                    if (clients[k].channel == clients[i].channel && i != k) {
                        for (size_t j = 1; j < args.size(); j++) 
                            content += args[j] + " ";
                        if (!content.empty()) 
                            content = content.substr(0, content.length() - 1);   
                        privmess = ":" + get_nick(fd) + " PRIVMSG " + args[0] + " :" + content + "\r\n";
                        send(clients[i].fd, privmess.c_str(), privmess.size(), 0);
                        std::string confirm = "Message sent to " + args[0] + "\r\n";
                        send(fd, confirm.c_str(), confirm.size(), 0);
                        return;
            }}}}}
    std::string str;
    for (i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            if (clients[i].channel != "\0") {
                for (size_t k = 0; k < clients.size(); k++) {
                    if (clients[k].channel == clients[i].channel && i != k) {
                        for (size_t j = 0; j < args.size(); j++) 
                            str += args[j] + " ";
                        if (!str.empty()) 
                            str = str.substr(0, str.length() - 1); 
                        std::string message = ":" + clients[i].nick + " PRIVMSG " + clients[k].channel + " :" + str + "\r\n";
                        send(clients[k].fd, message.c_str(), message.size(), 0);
                        return ;
            }}}}}
    send(fd, "Error, in message.\r\n", 30, 0);
}

// /join #channel
//send to all clients in the channel
//join/create:admin if channel doesnt exist
//open another file with channel
//this as an error
void Server::join(int fd, std::vector<std::string> args) {
    if (is_authentic(fd) == 1) {return ;}
    if (args.size() != 1 || args[0][0] != '#')
       {send(fd, "Try JOIN #channel\r\n", 21, 0); return ; } 
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            if (clients[i].channel == args[0]) {return ;}
            clients[i].channel = args[0];   
            for (size_t j = 0; j < clients.size(); j++) {
                if (clients[j].channel == args[0] && clients[j].fd != fd) {
                    std::string notify = clients[i].nick + " has joined " + args[0] + "\r\n";
                    send(clients[j].fd, notify.c_str(), notify.size(), 0);
                }}
            std::string join_message = ":" + clients[i].nick + " JOIN " + args[0] + "\r\n";
            std::cout << clients[i].nick << ": joined " << args[0] << std::endl;
            send(fd, join_message.c_str(), join_message.size(), 0);
            return ; 
    }}
}

