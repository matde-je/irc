#include "../incs/irc.hpp"
#include "../incs/channel.hpp"

Client::~Client() {}
Client::Client() {pass = false;}

std::string Server::get_nick(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd) {
            if (!clients[i].getNick().empty())
                return clients[i].getNick();
    }}
    std::stringstream ss;
    ss << fd;
    return ("Client " + ss.str());
}

//  /nick <nickname>
void Server::nick(int fd, std::vector<std::string> args){
    if (has_pass(fd) == 1) {return ;}
    if (args.size() == 1) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].getFd() == fd) {
                clients[i].setNick(args[0]);
                std::cout << "Client " << fd << ": changed nick to " << args[0] << std::endl;
                return ;
            }}}
}

//USER <username> <mode> <unused> <realname>
//USER myusername 0 0 My Real Name
//check
void Server::user(int fd, std::vector<std::string> args) {
    if (has_pass(fd) == 1) {return;}
    
    if (args.size() == 4) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].getFd() == fd) {
                clients[i].setName(args[0]);
                std::cout << "Client " << fd << ": changed name to " << args[0] << std::endl;
                std::string message = "NAME set to " + args[0] + "\r\n";
                send(fd, message.c_str(), message.size(), 0);
                return;
            }
        }
    }
}

void    Server::pass(int fd, std::vector<std::string> args){
    if (args.size() == 1 && args[0] == password ) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].getFd() == fd) {
                clients[i].setPass(true);
                send(fd, "Password correct.\r\n", 20, 0);
                return ;
            }}}
    send(fd, "Wrong password.\r\n", 18, 0); 
}

int    Server::has_pass(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd) {
            if (clients[i].getPass() == false) {
                send(fd, "You must enter password.\r\n", 27, 0); 
                return 1; }
        }}
    return 0;
}

int    Server::is_authentic(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd) {
            if (clients[i].getPass() == false) {
                send(fd, "You must enter password.\r\n", 27, 0); 
                return 1; }
            if (clients[i].getName() == "\0" || clients[i].getNick() == "\0") {
                send(fd, "You must authenticate before interacting.\r\n", 44, 0); 
                return 2; }
        }}
    return 0;
}

std::string to_uppercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

int    Server::send_cmd(int fd, std::string cmd, std::vector<std::string> args) {
    cmd = to_uppercase(cmd);
    if (cmd == "PASS"|| cmd == "/PASS") 
        {pass(fd, args); return 1;}
    else if (cmd == "USER"|| cmd == "/USER")
        {user(fd, args); return 1;}
    else if (cmd == "NICK" || cmd == "/NICK")
        {nick(fd, args); return 1;}
    else if (cmd == "TOPIC" || cmd == "/TOPIC")
        {topic(fd, args); return 1;}
    else if (cmd == "MODE" || cmd == "/MODE")
        {mode(fd, args); return 1;}
    else if (cmd == "KICK" || cmd == "/KICK")
        {kick(fd, args); return 1;}
    else if (cmd == "INVITE" || cmd == "/INVITE")
        {invite(fd, args); return 1;}
    else if (cmd == "PRIVMSG" || cmd == "/PRIVMSG")
        {msg(fd, args); return 1;}
    else if (cmd == "JOIN" || cmd == "/JOIN")
        {
            join(fd, args);
            return 1;
        }
    else if (cmd == "SHOW" || cmd == "/SHOW")
        {
            std::cout << "Showing clients\n";
            showClients(fd, args); 
            return 1;
        }
    else if(cmd == "CHANNEL"){
        std::cout << "Showing channels: \n";
        showChannels(fd);
        return 1;
    }
    else {
        std::string error_message = "ERROR :Unknown command " + cmd + "\r\n";
        send(fd, error_message.c_str(), error_message.size(), 0);
        return 0;
    }
    return 0;
}

//getters
int Client::getFd() {return fd;}
std::string Client::getIp() {return ip;}
std::string Client::getName() {return name;}
std::string Client::getNick() {return nick;}
bool Client::getPass() {return pass;}
std::vector<std::string> Client::getChannel() {return channels;}
std::vector<std::string> Client::getInvites() {return invites;}

//setters

void Client::setFd(int fd) {this->fd = fd;}
void Client::setIp(std::string ip) {this->ip = ip;}
void Client::setName(std::string name) {this->name = name;}
void Client::setNick(std::string nick) {this->nick = nick;}
void Client::setPass(bool pass) {this->pass = pass;}
void Client::addChannel(std::string channel) {channels.push_back(channel);}
void Client::removeChannel(std::string channel) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i] == channel) {
            channels.erase(channels.begin() + i);
            return;
        }
    }
}
void Client::addInvite(std::string invite) {invites.push_back(invite);}
void Client::removeInvite(std::string invite) {
    for (size_t i = 0; i < invites.size(); i++) {
        if (invites[i] == invite) {
            invites.erase(invites.begin() + i);
            return;
        }
    }
}


bool Client::isInChannel(std::string channel) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i] == channel) {
            return true;
        }
    }
    return false;
}