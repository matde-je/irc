#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "irc.hpp"

class Client;
class Channel {
    private :
        std::string name;
        std::vector<Client> users;
        std::vector<Client> admins;
        std::vector<Client> kicked;
    
    public : 
        Channel();
        Channel(std::string name);
        std::string getName();
        void setName(std::string name);
        bool isAdmin(std::string name);
        bool isUser(std::string name);
        int KickUser(std::string name);
        void addUser(Client user);
        int userExists(Client User);
        void fixPartialExistence(Client user);
        int getClientIndex(std::string name);
        std::vector<Client> getUsers();

};

#endif