#include "../incs/irc.hpp"
#include "../incs/channel.hpp"

Client::~Client() {}
Client::Client() {pass = false;}

std::string Server::get_nick(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd) {
            if (!clients[i].getNick().empty()) {
                return clients[i].getNick();
            }
    }}
    std::stringstream ss;
    ss << fd;
    return ("Client " + ss.str());
}

 ///nick <nickname>
void Server::nick(int fd, std::vector<std::string> args){
    if (has_pass(fd) == 1) {return ;}
    if (args.size() == 1) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].getNick() == args[0]) {
                send_message(fd, "433 " + args[0] + " :Nickname is already in use");
                return ;
            }
        }
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].getFd() == fd) {
                clients[i].setNick(args[0]);
                send_message(fd, ":" + args[0] + " NICK " + args[0]);
                send_message(fd, "001 " + args[0] + " :Welcome to the IRC server, " + args[0]);
                return ;
            }}}
}

void Server::who(int fd, std::vector<std::string> args) {
    if (args.size() < 1)
        return ;
    std::string channel_name = args[0];
    bool channel_found = false;
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        Client& client = *it;
        const std::vector<std::string>& channels = client.getChannel(); 
        for (std::vector<std::string>::const_iterator ch_it = channels.begin(); ch_it != channels.end(); ++ch_it) {
            const std::string& channel = *ch_it;  
            if (channel == channel_name) {
                channel_found = true;
                std::string response = ":server 352 " + client.getNick() + " " + channel_name + " 0 0 " + client.getNick() + " H :0 Unknown\r\n";
                send(fd, response.c_str(), response.size(), 0);
                break ; 
            }
        }
    }
    if (!channel_found) {
        send_message(fd, "403 " + channel_name + " :No such channel");
    } else {
        std::string end_response = ":server 315 " + channel_name + " :End of WHO list\r\n";
        send(fd, end_response.c_str(), end_response.size(), 0);
    }
}


void Server::send_message(int fd, std::string str) {
    std::string message = ":server " + str + "\r\n";
    send(fd, message.c_str(), message.length(), 0);
    std::cout << "Sending message: " << message << std::endl;
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
    else {
        send_message(fd, "Invalid number of args");
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

void Server::handle_cap_ls(int fd) {
    // Respond to the CAP LS request
    std::string response = ":server CAP * LS : \r\n"; // Indicating no capabilities
    send(fd, response.c_str(), response.length(), 0);
}

void Server::handle_cap_end(int fd) {
    // Respond to CAP END
    std::string end_response = ":server CAP * END\r\n";
    send(fd, end_response.c_str(), end_response.length(), 0);
}

std::string to_uppercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

int    Server::send_cmd(int fd, std::string cmd, std::vector<std::string> args) {
    //std::cout << cmd << std::endl;
    cmd = to_uppercase(cmd);
    if (cmd == "PASS"|| cmd == "/PASS") 
        {pass(fd, args); return 1;}
    else if (cmd == "USER"|| cmd == "/USER")
        {user(fd, args); return 1;}
    else if (cmd == "CAP") {
        if (!args.empty() && args[0] == "LS")
            handle_cap_ls(fd);
        if (!args.empty() && args[0] == "END")
            handle_cap_end(fd);
        return 1;}
    else if (cmd == "WHO")
        {who(fd, args); return 1;}
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
std::string Client::getNick() {return this->nick;}
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
void Client::removeChannel(const std::string& channelName)
{
    std::vector<std::string>::iterator it = std::find(channels.begin(), channels.end(), channelName);
    if (it != channels.end())
    {
        channels.erase(it);
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