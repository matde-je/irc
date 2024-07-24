#include "irc.hpp"

// /msg <nickname> <message> (private message)
//or in channel
//open new page
void Server::msg(int fd, std::vector<std::string> args) {
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 2) {send(fd, "Error in message arguments.\r\n", 30, 0); return ;}
    size_t i;
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
                    if (content[0] == ':')
                            content = content.substr(1, content.length());
                    privmess = ":" + get_nick(fd) + " PRIVMSG " + args[0] + " :" + content + "\r\n";
                    send(clients[i].fd, privmess.c_str(), privmess.size(), 0);
                    std::string confirm = "Message sent to " + args[0] + "\r\n";
                    send(fd, confirm.c_str(), confirm.size(), 0);
                    return;
        }}}}
    std::string str;
    for (i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            if (clients[i].channel != "\0") {
                for (size_t k = 0; k < clients.size(); k++) {
                    if (clients[k].channel == clients[i].channel && i != k) {
                        for (size_t j = 1; j < args.size(); j++) 
                            str += args[j] + " ";
                        if (!str.empty()) 
                            str = str.substr(0, str.length() - 1);
                        if (str[0] == ':')
                            str = str.substr(1, str.length());
                        std::string message = ":" + clients[i].nick + " PRIVMSG " + clients[k].channel + " :" + str + "\r\n";
                        send(clients[k].fd, message.c_str(), message.size(), 0);
                        return ;
            }}}}}
    send(fd, "Error in message.\r\n", 20, 0);
}

// /join #channel
//send to all clients in the channel
//join/create:admin if channel doesnt exist
//open another file with channel
//this as an error
void Server::join(int fd, std::vector<std::string> args) {
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 1 || args[0][0] != '#')
       {send(fd, "Try JOIN #channel\r\n", 21, 0); return ; } 
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            if (clients[i].channel == args[0]) {return ;}
            clients[i].channel = args[0];   
            for (size_t j = 0; j < clients.size(); j++) {
                if (clients[j].channel == args[0] && i != j) {
                    std::string notify = clients[i].nick + " has joined " + args[0] + "\r\n";
                    send(clients[j].fd, notify.c_str(), notify.size(), 0);
                }}
            std::string join_message = ":" + clients[i].nick + " JOIN " + args[0] + "\r\n";
            std::cout << clients[i].nick << ": joined " << args[0] << std::endl;
            send(fd, join_message.c_str(), join_message.size(), 0);
            return ; 
    }}
}

