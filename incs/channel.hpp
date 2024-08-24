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
        std::vector<Client> invitees;
    
    public : 
        Channel();
        ~Channel();
        Channel(std::string name);
        std::string getName();
        void setName(std::string name);
        bool isAdmin(std::string nick);
        bool isUser(std::string nick);
        int KickUser(std::string nick);
        void addUser(Client user);
        int userExists(Client User);
        void fixPartialExistence(Client user);
        int getClientIndex(std::string nick);
        std::vector<Client> getUsers();
        Client *getUserFromFD(int fd);
        Client *getUserFromNick(std::string nick);
        void addAdmin(std::string nick);
        void addInvitee(std::string nick);

};

#endif