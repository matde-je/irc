#include "../incs/irc.hpp"

void Server::msg(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0 || args.size() < 2)
    {
        std::string error_message = ":yourserver.com 411 " + getClientFromFD(fd)->getNick() + " :No recipient given\r\n";
        send(fd, error_message.c_str(), error_message.size(), 0);
        return;
    }

    std::string recipient = args[0];
    std::string content;
    for (size_t i = 1; i < args.size(); ++i)
    {
        content += args[i] + " ";
    }
    if (!content.empty())
    {
        content = content.substr(0, content.length() - 1); // Remove trailing space
    }
    if (!content.empty() && content[0] == ':')
    {
        content = content.substr(1); // Remove leading colon
    }

    Client* sender = getClientFromFD(fd);
    if (sender == NULL)
    {
        std::string error_message = ":yourserver.com 401 :Sender not found\r\n";
        send(fd, error_message.c_str(), error_message.size(), 0);
        return;
    }

    // Check if the recipient is a channel
    Channel* channel = getChannelFromName(recipient);
    if (channel != NULL)
    {
        std::string message = ":" + sender->getNick() + " PRIVMSG " + recipient + " :" + content + "\r\n";
        std::vector<Client> users = channel->getUsers();
        for (size_t i = 0; i < users.size(); ++i)
        {
            if (users[i].getFd() != fd)
            {
                send(users[i].getFd(), message.c_str(), message.size(), 0);
            }
        }
        return;
    }

    // Check if the recipient is a user
    Client* recipientClient = getClientFromNick(recipient);
    if (recipientClient != NULL)
    {
        std::string privmess = ":" + sender->getNick() + " PRIVMSG " + recipient + " :" + content + "\r\n";
        send(recipientClient->getFd(), privmess.c_str(), privmess.size(), 0);
        std::string confirm = "Message sent to " + recipient + "\r\n";
        send(fd, confirm.c_str(), confirm.size(), 0);
        return;
    }

    // If recipient is neither a channel nor a user
    std::string error_message = ":yourserver.com 401 " + sender->getNick() + " " + recipient + " :No such nick/channel\r\n";
    send(fd, error_message.c_str(), error_message.size(), 0);
}

void Server::joinForSure(std::string channelName, int newChannel, Channel *channel, Client *client)
{
    client->addChannel(channelName);
    channel->addUser(*client);

    std::string join_msg = ":" + client->getNick() + " JOIN " + channelName + "\r\n";
    std::vector<Client> users = channel->getUsers();
    if (newChannel)
    {
        channel->addAdmin(client->getNick());
    }

    if (channel->isInvitee(client->getNick()))
    {
        channel->removeInvitee(client->getNick());
        std::vector<std::string>::iterator it = std::find(client->getInvites().begin(), client->getInvites().end(), channelName);
        if (it != client->getInvites().end())
        {
            client->getInvites().erase(it);
        }
    }
    for (size_t i = 0; i < users.size(); ++i)
    {
        send(users[i].getFd(), join_msg.c_str(), join_msg.size(), 0);
    }
}


void Server::join(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0)
    {
        return;
    }

    if (args.size() < 1 || args.size() > 2 || args[0][0] != '#')
    {
        send(fd, "Usage: JOIN #channel [password]\r\n", 34, 0);
        return;
    }

    std::string channelName = args[0];
    std::string password = (args.size() == 2) ? args[1] : "";

    int newChannel = 0;
    Channel *channel = findOrMakeChannel(&newChannel, channelName);
    if (channel == NULL)
    {
        send(fd, "Error: Channel could not be found or created\r\n", 46, 0);
        return;
    }

    Client *client = getClientFromFD(fd);
    if (client == NULL)
    {
        send(fd, "Error: Client not found\r\n", 25, 0);
        return;
    }

    if (channel->isUser(client->getNick()))
    {
        send(fd, "You are already in this channel\r\n", 33, 0);
        return;
    }

    if (channel->getisLimited() && channel->getUsers().size() >= channel->getLimit())
    {
        send(fd, "Channel is full\r\n", 17, 0);
        return;
    }

    if (channel->isInviteOnly() && !channel->isInvitee(client->getNick()))
    {
        send(fd, "You are not invited to this channel\r\n", 38, 0);
        return;
    }

    if (channel->isPasswordProtected() && !channel->isInvitee(client->getNick()))
    {
        if (password.empty())
        {
            send(fd, "Password required\r\n", 21, 0);
            return;
        }
        else if (channel->getPassword() != password)
        {
            send(fd, "Incorrect password\r\n", 21, 0);
            return;
        }
    }

    // Handle leaving the previous channel

    // if (channel->isInvitee(client->getNick()))
    // {
    //     joinForSure(channelName, newChannel, channel, client);
    //     return;
    // }
    joinForSure(channelName, newChannel, channel, client);
}
