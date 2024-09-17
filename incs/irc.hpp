#pragma once

#include <fcntl.h>
#include <iostream>
#include <string>
#include <sstream>
#include <string>
#include <cstring>
#include <climits>
#include <algorithm>
#include <cstdlib>

#include <vector> 
#include <sys/socket.h> 
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> 
#include <csignal> 
#include "channel.hpp"

#include <map>

//info about each client
class Client {
    private :
        int fd;
        std::string ip;
        std::string name;
        std::string nick;
        bool pass;
        std::vector<std::string> channels;
        std::vector<std::string> invites;

    public :
        Client();
        ~Client();

        int getFd();
        std::string getIp();
        std::string getName();
        std::string getNick();
        bool getPass();
        std::vector<std::string> getChannel();
        std::vector<std::string> getInvites();

        void setFd(int fd);
        void setIp(std::string ip);
        void setName(std::string name);
        void setNick(std::string nick);
        void setPass(bool pass);
        void addChannel(std::string channel);
        void removeChannel(const std::string& channelName);
        void addInvite(std::string invites);
        void removeInvite(std::string invite);

        bool isInChannel(std::string channel);

};

class Server {
    public :
        std::vector<Channel> channels;
        int port;
        std::map<int, std::string> client_buffers;
        std::string password;
        int socketfd;
        static bool signal;
        std::vector<Client> clients;
        std::vector<struct pollfd> fds; //pollfd is used in poll() to monitor fds (sockets) for events
        void loop();
        void parse(int fd, const char *buf);
        void new_client();
        void clear_client(int fd);
        void close_fds();
        void init_socket();
        static void signal_handler(int signum);
        Server();
        ~Server();

        int send_cmd(int fd, std::string cmd, std::vector<std::string> args);
        int is_authentic(int fd);
        void topic(int fd, std::vector<std::string> args);
        void mode(int fd, std::vector<std::string> args);
        void kick(int fd, std::vector<std::string> args);
        void invite(int fd, std::vector<std::string> args);
        void msg(int fd, std::vector<std::string> args);
        void join(int fd, std::vector<std::string> args);
        void send_error(int fd, std::string str);
        void nick(int fd, std::vector<std::string> args);
        void user(int fd, std::vector<std::string> args);
        void pass(int fd, std::vector<std::string> args);
        int  has_pass(int fd);
        std::string get_nick(int fd);

        Channel *getChannelFromName(std::string name);
        Channel *findOrMakeChannel(int *newChannel, std::string name);
        void showClients(int fd,  std::vector<std::string> args);
        void showChannels(int fd);

        Client *getClientFromFD(int fd);
        Client *getClientFromNick(std::string nick);

        void joinForSure(std::string channelName, int newChannel, Channel *channel, Client *client);
};