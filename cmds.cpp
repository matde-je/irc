#include "irc.hpp"

void Server::topic(int fd, std::vector<std::string> args){
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 2) {return ;}
    
}

void Server::mode(int fd, std::vector<std::string> args){
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 2) {return ;}

}

void Server::kick(int fd, std::vector<std::string> args){
    if (is_authentic(fd) == 0 && args.size() == 2) {
        clear_client(fd);
    }
}

void Server::invite(int fd, std::vector<std::string> args){
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 2) {return ;}
}

