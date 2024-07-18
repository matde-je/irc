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

class Client {
    public :
        Client();
        ~Client();
        int fd;
        std::string ip;
        std::string name;
        std::string nick;
        bool pass;
};


class Server {
    public :
        int port;
        std::string password;
        int socketfd;
        static bool signal;
        std::vector<Client> clients;
        std::vector<struct pollfd> fds; //pollfd is used in poll() to monitor fds (sockets) for events
        void loop();
        void parse(int fd, char *buf);
        void new_client();
        void clear_client(int fd);
        void new_data(int fd);
        void close_fd();
        void init_socket();
        static void signal_handler(int signum);
        Server();
        ~Server();

        void cmd_parse(int fd, std::string comd ,std::vector<std::string> args, bool ver);
        void send_cmd(int fd, std::string cmd, std::vector<std::string> args);
        void topic(int fd, std::vector<std::string> args);
        void mode(int fd, std::vector<std::string> args);
        void kick(int fd, std::vector<std::string> args);
        void invite(int fd, std::vector<std::string> args);
        void msg(int fd, std::vector<std::string> args);
        void join(int fd, std::vector<std::string> args);
        void nick(int fd, std::vector<std::string> args);
        void user(int fd, std::vector<std::string> args);
        void pass(int fd, std::vector<std::string> args);
        std::string get_nick(int fd);
};