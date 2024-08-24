#include "../incs/irc.hpp"

// /msg <nickname> <message> (private message)
// or in channel
// open new page
void Server::msg(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0)
    {
        return;
    }

    size_t i;
    for (i = 0; i < clients.size(); i++)
    {
        if (clients[i].nick == args[0])
        {
            std::string content;
            std::string privmess;
            for (size_t j = 1; j < args.size(); j++)
            {
                content += args[j] + " ";
            }
            if (!content.empty())
            {
                content = content.substr(0, content.length() - 1); // Remove trailing space
            }
            if (!content.empty() && content[0] == ':')
            {
                content = content.substr(1); // Remove leading colon
            }
            privmess = ":" + get_nick(fd) + " PRIVMSG " + args[0] + " :" + content + "\r\n";
            send(clients[i].fd, privmess.c_str(), privmess.size(), 0);
            std::string confirm = "Message sent to " + args[0] + "\r\n";
            send(fd, confirm.c_str(), confirm.size(), 0);
            return;
        }
    }

    std::string str;
    for (i = 0; i < clients.size(); i++)
    {
        if (clients[i].fd == fd)
        {
            if (!clients[i].channel.empty())
            {
                for (size_t j = 1; j < args.size(); j++)
                {
                    str += args[j] + " ";
                }
                if (!str.empty())
                {
                    str = str.substr(0, str.length() - 1); // Remove trailing space
                }
                if (!str.empty() && str[0] == ':')
                {
                    str = str.substr(1); // Remove leading colon
                }
                std::string message = ":" + clients[i].nick + " PRIVMSG " + clients[i].channel + " :" + str + "\r\n";
                send(clients[i].fd, message.c_str(), message.size(), 0);
                return;
            }
        }
    }

    send(fd, "Error in message.\r\n", 20, 0);
}

// /join #channel
// send to all clients in the channel
// join/create:admin if channel doesnt exist
// open another file with channel
// this as an error
void Server::join(int fd, std::vector<std::string> args)
{
    int newChannel = 0;
    Channel *channel = findOrMakeChannel(&newChannel, args[0]);

    if (is_authentic(fd) != 0)
    {
        return;
    }
    if (args.size() != 1 || args[0][0] != '#')
    {
        send(fd, "Try JOIN #channel\r\n", 21, 0);
        return;
    }
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].fd == fd)
        {
            send(fd, "FOUND CLIENT\r\n", 17, 0);
            if (channel->isUser(clients[i].nick))
            {
                send(fd, "YOU ARE ALREADY IN THE CHANNEL\r\n", 33, 0);
                return;
            }
            if (newChannel == 0)
            {
                if (channel->getUsers().size() >= channel->getLimit())
                {

                    const char *channelFullMsg = "CHANNEL IS FULL\r\n";
                    send(fd, channelFullMsg, strlen(channelFullMsg), 0);
                    return;
                }
            }
            if (clients[i].channel.empty() == false)
            {
                send(fd, "LEAVING PREVIOUS CHANNEL\r\n", 26, 0);
                std::cout << "in channel: " << clients[i].channel << std::endl;
                getChannelFromName(clients[i].channel)->KickUser(clients[i].nick);
                clients[i].channel = "";
            }
            clients[i].channel = channel->getName();
            for (size_t j = 0; j < channel->getUsers().size(); j++)
            {
                send(channel->getUsers()[j].fd, (clients[i].nick + " has joined " + args[0] + "\r\n").c_str(), clients[i].nick.length() + args[0].length() + 15, 0);
            }
            channel->addUser(clients[i]);
            if (newChannel == 1)
            {
                channel->addAdmin(clients[i].nick);
                send(clients[i].fd, "Channel created and you are the admin\r\n", 40, 0);
            }
            if (channel->isInvitee(clients[i].nick))
            {
                channel->removeInvitee(clients[i].nick);
                clients[i].invites.erase(std::remove(clients[i].invites.begin(), clients[i].invites.end(), channel->getName()), clients[i].invites.end());
            }
            std::string join_message = ":" + clients[i].nick + " JOIN " + args[0] + "\r\n";
            std::cout << clients[i].nick << ": joined " << args[0] << std::endl;
            send(fd, join_message.c_str(), join_message.size(), 0);
            return;
        }
    }
}

// previous
/*
void Server::join(int fd, std::vector<std::string> args) {
    Channel *channel = findOrMakeChannel(args[0]);
    if (is_authentic(fd) != 0) {return ;}
    if (args.size() != 1 || args[0][0] != '#')
       {send(fd, "Try JOIN #channel\r\n", 21, 0); return ; }
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == fd) {
            send(fd, "FOUND CLIENT\r\n", 14, 0);
            int exists = channel->userExists(clients[i]);
            if (exists == 1) {
                send(fd, "YOU ARE ALREADY IN THE CHANNEL\r\n", 33,0);
                return ;}
            else if(exists == 2){
                send(fd, "SOMETHING WENT WRONG BUT ITS ALL GOOD NOW :)\r\n", 47,0);
                channel->fixPartialExistence(clients[i]);
                return;
            }
            if (clients[i].channel != NULL){
                send(fd, "LEAVING PREVIOUS CHANNEL\r\n", 27, 0);
                clients[i].channel->KickUser(clients[i].name);
            }
            clients[i].channel = channel;
            channel->addUser(clients[i]);
            for (size_t j = 0; j < clients.size(); j++) {
                if (clients[j].channel == args[0] && i != j) {
                    std::string notify = clients[i].nick + " has joined " + args[0] + "\r\n";
                    send(clients[j].fd, notify.c_str(), notify.size(), 0);
                }}
            std::string join_message = ":" + clients[i].nick + " JOIN " + args[0] + "\r\n";
            std::cout << clients[i].nick << ": joined " << args[0] << std::endl;
            send(fd, join_message.c_str(), join_message.size(), 0);
            return ;
    }}
}
*/
