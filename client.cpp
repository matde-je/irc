#include "irc.hpp"

Client::~Client() {}
Client::Client() {pass = false;}

std::string Server::get_nick(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            if (clients[i].nick != "\0")
                return clients[i].nick;
    }}
    std::stringstream ss;
    ss << fd;
    return ("Client " + ss.str());
}


void Server::nick(int fd, std::vector<std::string> args){
    if (args.size() == 1) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].nick = args[0];
                std::cout << "Client " << fd << ": changed nick to " << args[0] << std::endl;
                return ;
            }}}
}

//USER <username> <mode> <unused> <realname>
//USER myusername 0 0 My Real Name
void Server::user(int fd, std::vector<std::string> args){
    if (args.size() == 4) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].name = args[0];
                if (clients[i].nick != "\0")
                    std::cout << clients[i].nick << ": changed name to " << args[0] << std::endl;
                else {std::cout << "Client " << fd << ": changed name to " << args[0] << std::endl;}
                return ;
            }}}
    send(fd, "Try: USER <username> <mode> <unused> <realname>\n", 49, 0); 
}

void    Server::pass(int fd, std::vector<std::string> args){
    if (args.size() == 1 && !args[0].compare(password)) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                clients[i].pass = true;
                return ;
            }}}
    send(fd, "Wrong password.\n", 17, 0); 
}




void    Server::cmd_parse(int fd, std::string cmd, std::vector<std::string> args, bool ver, std::string buf) {
    if (cmd != "/pass") {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                if (clients[i].pass == false) {
                    send(fd, "Enter password to access server.\n", 34, 0); 
                    return ; }
        }}}
    if (ver == true) {
        for (size_t i = 0; i < args.size(); i++) {
            size_t pos = args[i].find('\n'); //removing nl
            if (pos != std::string::npos)
                args[i].erase(pos, 1);
            }
        ver = send_cmd(fd, cmd, args);
    }
    if (ver == 0) {
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].fd == fd) {
                std::string name = clients[i].nick;
                if (clients[i].channel != "\0") {
                    std::string chan = clients[i].channel;
                    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
                        if ((*it).fd != fd && (*it).channel == chan) { //don't send data back to the original client, and send it to everyone in the channel
                                std::string mess = name + ": " + buf;
                            send((*it).fd, mess.c_str(), mess.size(), 0); }
                    }
    }}}}
    
}

int    Server::send_cmd(int fd, std::string cmd, std::vector<std::string> args) {
    if (cmd == "/pass")
        {pass(fd, args); return 1;}
    else if (cmd == "USER")
        {user(fd, args); return 1;}
    else if (cmd == "/nick")
        {nick(fd, args); return 1;}
    // else if (cmd == "/topic")
    //     {topic(fd, args); return 1;}
    // else if (cmd == "/mode")
    //     {mode(fd, args); return 1;}
    // else if (cmd == "/kick")
    //     {kick(fd, args); return 1;}
    // else if (cmd == "/invite")
    //     {invite(fd, args); return 1;}
    else if (cmd == "/msg")
        {msg(fd, args); return 1;}
    else if (cmd == "/join")
        {join(fd, args); return 1;}
    return 0;
}