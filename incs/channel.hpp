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
        bool isLimited;
        size_t limit;
        bool topicRestricted;
        bool passwordProtected;
        std::string password;

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
        int getClientIndex(std::string nick);
        std::vector<Client> getUsers();
        Client *getUserFromFD(int fd);
        Client *getUserFromNick(std::string nick);
        void addAdmin(std::string nick);
        void removeAdmin(std::string nick);
        void addInvitee(Client client);
        void setTopic(std::string topic);
        std::string getTopic();
        void removeInvitee(std::string nick);
        bool isInvitee(std::string nick);
        bool isTopicRestricted();
        size_t getLimit();
        bool isInviteOnly();
        void setLimit(size_t limit);
        bool getisLimited();
        void setInviteOnly(bool inviteOnly);
        void setisLimited(bool isLimited);
        void setTopicRestricted(bool topicRestricted);
        void setPassword(std::string password);
        std::string getPassword();
        bool isPasswordProtected();
        std::vector<Client> getAdmins();
        void setPasswordProtected(bool passwordProtected);

};

#endif