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
        std::string topic;
        bool InviteOnly;
        size_t limit;
        bool topicRestricted;
    
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
        void setTopic(std::string topic);
        std::string getTopic();
        void removeInvitee(std::string nick);
        bool isInvitee(std::string nick);
        bool isTopicRestricted();
        size_t getLimit();
};

#endif