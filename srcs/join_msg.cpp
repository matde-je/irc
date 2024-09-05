#include "../incs/irc.hpp"

// /msg <nickname> <message>
// /msg <channel> <message>
void Server::msg(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0 || args.size() < 2)
    {
        std::string error_message = ":yourserver.com 411 " + getClientFromFD(fd)->nick + " :No recipient given\r\n";
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
        content = content.substr(0, content.length() - 1);
    }
    if (!content.empty() && content[0] == ':')
    {
        content = content.substr(1);
    }

    Client* sender = getClientFromFD(fd);
    if (sender == NULL)
    {
        std::string error_message = ":yourserver.com 401 :Sender not found\r\n";
        send(fd, error_message.c_str(), error_message.size(), 0);
        return;
    }

    Client* recipientClient = getClientFromNick(recipient);
    if (recipientClient != NULL)
    {
        if (sender->channel == recipientClient->channel && !sender->channel.empty())
        {
            std::string privmess = ":" + sender->nick + " PRIVMSG " + recipient + " :" + content + "\r\n";
            send(recipientClient->fd, privmess.c_str(), privmess.size(), 0);
            std::string confirm = "Message sent to " + recipient + "\r\n";
            send(fd, confirm.c_str(), confirm.size(), 0);
            return;
        }
        else
        {
            std::string error_message = ":yourserver.com 404 " + sender->nick + " " + recipient + " :Recipient not in the same channel\r\n";
            send(fd, error_message.c_str(), error_message.size(), 0);
            return;
        }
    }


    Channel* channel = getChannelFromName(recipient);
    if (channel != NULL)
    {
        if (sender->channel == recipient)
        {
            std::string message = ":" + sender->nick + " PRIVMSG " + recipient + " :" + content + "\r\n";
            std::vector<Client> users = channel->getUsers();
            for (size_t i = 0; i < users.size(); ++i)
            {
                if (users[i].fd != fd)
                {
                    send(users[i].fd, message.c_str(), message.size(), 0);
                }
            }
            return;
        }
        else
        {
            std::string error_message = ":yourserver.com 404 " + sender->nick + " " + recipient + " :You are not in the channel\r\n";
            send(fd, error_message.c_str(), error_message.size(), 0);
            return;
        }
    }

    std::string error_message = ":yourserver.com 401 " + sender->nick + " " + recipient + " :No such nick/channel\r\n";
    send(fd, error_message.c_str(), error_message.size(), 0);
}

void Server::joinForSure(std::string channelName, int newChannel, Channel *channel, Client *client)
{
    client->channel = channelName;
    channel->addUser(*client);

    std::string join_msg = ":" + client->nick + " JOIN " + channelName + "\r\n";
    std::vector<Client> users = channel->getUsers();
    if (newChannel)
    {
        channel->addAdmin(client->nick);
    }

    if (channel->isInvitee(client->nick))
    {
        channel->removeInvitee(client->nick);
        std::vector<std::string>::iterator it = std::find(client->invites.begin(), client->invites.end(), channelName);
        if (it != client->invites.end())
        {
            client->invites.erase(it);
        }
    }
    for (size_t i = 0; i < users.size(); ++i)
    {
        send(users[i].fd, join_msg.c_str(), join_msg.size(), 0);
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

    if (channel->isUser(client->nick))
    {
        send(fd, "You are already in this channel\r\n", 33, 0);
        return;
    }

    if (channel->getisLimited() && channel->getUsers().size() >= channel->getLimit())
    {
        send(fd, "Channel is full\r\n", 17, 0);
        return;
    }

    if (channel->isInviteOnly() && !channel->isInvitee(client->nick))
    {
        send(fd, "You are not invited to this channel\r\n", 38, 0);
        return;
    }

    if (channel->isPasswordProtected() && !channel->isInvitee(client->nick))
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
    if (!client->channel.empty())
    {
        Channel *previousChannel = getChannelFromName(client->channel);
        if (previousChannel->isAdmin(client->nick))
        {
            if (previousChannel->getAdmins().size() == 1)
            {
                bool hasNewAdmin = false;
                for (size_t i = 0; i < previousChannel->getUsers().size() && hasNewAdmin; i++)
                {
                    if (!previousChannel->isAdmin(previousChannel->getUsers()[i].nick))
                    {
                        previousChannel->addAdmin(previousChannel->getUsers()[i].nick);
                        hasNewAdmin = true;
                        break;
                    }
                }
            }
            previousChannel->removeAdmin(client->nick);
        }
        if (previousChannel != NULL)
        {
            previousChannel->KickUser(client->nick);
            std::string part_msg = ":" + client->nick + " PART " + client->channel + "\r\n";
            std::vector<Client> previousUsers = previousChannel->getUsers();
            for (size_t i = 0; i < previousUsers.size(); ++i)
            {
                send(previousUsers[i].fd, part_msg.c_str(), part_msg.size(), 0);
            }
        }
        client->channel = "";
    }
    // if (channel->isInvitee(client->nick))
    // {
    //     joinForSure(channelName, newChannel, channel, client);
    //     return;
    // }
    joinForSure(channelName, newChannel, channel, client);
}
