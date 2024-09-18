#include "../incs/irc.hpp"
#include "../incs/channel.hpp"

Channel::Channel()
{
    this->name = "default channel name";
    this->topic = "default topic";
    this->topicRestricted = true;
    this->InviteOnly = false;
    this->isLimited = false;
    this->limit = 999;
    this->passwordProtected = false;
}

Channel::Channel(std::string name)
{
    this->name = name;
    this->topic = "default topic";
    this->topicRestricted = true;
    this->InviteOnly = false;
    this->isLimited = false;
    this->limit = 999;
    this->passwordProtected = false;
}

std::string Channel::getName()
{
    return this->name;
}

void Channel::setName(std::string name)
{
    this->name = name;
}

bool Channel::isUser(std::string nick)
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getNick() == nick)
            return true;
    }
    return false;
}

bool Channel::isAdmin(std::string nick)
{
    for (size_t i = 0; i < admins.size(); i++)
    {
        if (admins[i].getNick() == nick)
            return true;
    }
    return false;
}

int Channel::getClientIndex(std::string nick)
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getNick() == nick)
            return i;
    }
    return -1;
}

/**
 * @brief -1 means the user doesnt exist, 0 means all okay
 *
 * @param nick
 * @return int
 */
int Channel::KickUser(std::string nick)
{
    if (!isUser(nick))
    {
        return -1;
    }
    int index = getClientIndex(nick);
    if (index == -1)
    {
        std::cout << "unable to remove " << nick << " from the channel " << getName() << std::endl;
        return -1;
    }
    std::cout << "client: " << nick << ", removed from channel: " << getName() << std::endl;
    users.erase(users.begin() + index);
    return 0;
}
void Channel::addUser(Client user)
{
    this->users.push_back(user);
}

/**
 * @brief 1 means user exists, 0 means he doesn't, and 2 means he partially exists (either user points to Channel or is in the channel list)
 *
 * @param user
 * @return int
 */
// int Channel::userExists(Client user){
//     bool condition1 = isUser(user.name);
//     if (user.channel == NULL)
//         return 0;
//     bool condition2 = user.channel->getName() == this->name;
//     if ( condition1 && condition2) {
//         return 1;
//     } else if ( condition1 || condition2) {
//         return 2;
//     } else {
//         return 0;
//     }
// }

std::vector<Client> Channel::getUsers()
{
    return this->users;
}

Channel::~Channel()
{
}

Client *Channel::getUserFromNick(std::string nick)
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getNick() == nick)
            return &users[i];
    }
    return NULL;
}

Client *Channel::getUserFromFD(int fd)
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getFd() == fd)
        {
            return &users[i];
        }
    }
    return NULL;
}

void Channel::addAdmin(std::string nick)
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getNick() == nick)
        {
            admins.push_back(users[i]);
            return;
        }
    }
}

void Channel::addInvitee(Client client)
{
    invitees.push_back(client); // Push the Client object into the invitees vector
    std::cout << client.getNick(); // Print the 'nick' member of the Client object
    return;
}

void Channel::setTopic(std::string topic)
{
    this->topic = topic;
}

std::string Channel::getTopic()
{
    return this->topic;
}

void Channel::removeInvitee(std::string nick)
{
    for (size_t i = 0; i < invitees.size(); i++)
    {
        if (invitees[i].getNick() == nick)
        {
            invitees.erase(invitees.begin() + i);
            return;
        }
    }
}

bool Channel::isInvitee(std::string nick)
{
    for (size_t i = 0; i < invitees.size(); i++)
    {
        if (invitees[i].getNick() == nick)
            return true;
    }
    return false;
}

bool Channel::isTopicRestricted()
{
    return this->topicRestricted;
}

size_t Channel::getLimit()
{
    return this->limit;
}

bool Channel::isInviteOnly()
{
    return this->InviteOnly;
}

void Channel::setInviteOnly(bool InviteOnly)
{
    this->InviteOnly = InviteOnly;
}

bool Channel::getisLimited()
{
    return this->isLimited;
}

void Channel::setisLimited(bool isLimited)
{
    this->isLimited = isLimited;
}

void Channel::setLimit(size_t limit)
{
    this->limit = limit;
}

void Channel::setTopicRestricted(bool topicRestricted)
{
    this->topicRestricted = topicRestricted;
}

void Channel::setPassword(std::string password)
{
    this->password = password;
}

std::string Channel::getPassword()
{
    return this->password;
}

bool Channel::isPasswordProtected()
{
    return this->passwordProtected;
}

void Channel::removeAdmin(std::string nick)
{
    for (size_t i = 0; i < admins.size(); i++)
    {
        if (admins[i].getNick() == nick)
        {
            admins.erase(admins.begin() + i);
            return;
        }
    }
}

std::vector<Client> Channel::getAdmins()
{
    return this->admins;
}

void Channel::setPasswordProtected(bool passwordProtected)
{
    this->passwordProtected = passwordProtected;
}