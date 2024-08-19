#include "irc.hpp"
#include "channel.hpp"

Channel::Channel(){
    this->name = "default channel name";
}

Channel::Channel(std::string name){
    this->name = name;
}

std::string Channel::getName(){
    return this->name;
}

void Channel::setName(std::string){
    this->name = name;
}

bool Channel::isUser(std::string name){
    for (size_t i = 0; i < users.size(); i++){
        if (users[i].name == name)
            return true;
    }
    return false;
}

bool Channel::isAdmin(std::string name){
    for (size_t i = 0; i < admins.size(); i++){
        if (admins[i].name == name)
            return true;
    }
    return false;
}

int Channel::getClientIndex(std::string name){
    for(size_t i = 0; i < users.size(); i++){
        if (users[i].name == name)
            return i;
    }
    return 0;
}

/**
 * @brief -1 means the user doesnt exist, 0 means all okay
 * 
 * @param name 
 * @return int 
 */
int Channel::KickUser(std::string name){

    if(!isUser(name)){
        return -1;
    }
    int index = getClientIndex(name);
    std::cout << "client: " << name << ", removed from channel: " << getName() << std::endl;
    users.erase(users.begin() + index);
    return 0;
}

void Channel::addUser(Client user){
    this->users.push_back(user);
}

/**
 * @brief 1 means user exists, 0 means he doesn't, and 2 means he partially exists (either user points to Channel or is in the channel list)
 * 
 * @param user 
 * @return int 
 */
int Channel::userExists(Client user){
    bool condition1 = isUser(user.name);
    if (user.channel == NULL)
        return 0;
    bool condition2 = user.channel->getName() == this->name;
    if ( condition1 && condition2) {
        return 1;
    } else if ( condition1 || condition2) {
        return 2;
    } else {
        return 0;
    }
}

void Channel::fixPartialExistence(Client user){
    if (user.channel->getName() == this->name){
        addUser(user);
    }
    else if(isUser(user.name)){
        user.channel = this;
    }
}

std::vector<Client> Channel::getUsers(){
    return this->users;
}