#include "irc.hpp"

void Server::topic(int fd, std::vector<std::string> args){
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 2) {return ;}
    
}

void Server::mode(int fd, std::vector<std::string> args){
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 2) {return ;}

}

// /kick #channel <nickname>
void Server::kick(int fd, std::vector<std::string> args){
    std::cout << "attemptred kick" << std::endl;
    if (is_authentic(fd) == 0 && args.size() == 2) {
        std::string channel = args[0];
        std::string client = args[1];
        
        // Find the client in the clients vector
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].channel == channel && clients[i].nick == client) {
                // Remove the client from the channel
                clients[i].channel = "";
                send(clients[i].fd, "You have been kicked from the channel\r\n", 36, 0);
                
                // Notify other clients in the channel about the kick
                for (size_t j = 0; j < clients.size(); j++) {
                    if (clients[j].channel == channel && i != j) {
                        std::string notify = client + " has been kicked from " + channel + "\r\n";
                        send(clients[j].fd, notify.c_str(), notify.size(), 0);
                    }
                }
                
                // Send a kick message to the client
                std::string kick_message = "KICK " + channel + " " + client + "\r\n";
                send(fd, kick_message.c_str(), kick_message.size(), 0);
                
                // TODO: Implement any additional logic for handling the kick
                
                return;
            }
        }
        
        // Client not found in the channel
        std::string error_message = "Client " + client + " not found in channel " + channel + "\r\n";
        send(fd, error_message.c_str(), error_message.size(), 0);
    }
}

void Server::invite(int fd, std::vector<std::string> args){
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 2) {return ;}
}

