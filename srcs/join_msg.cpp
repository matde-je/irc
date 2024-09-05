#include "../incs/irc.hpp"

// /msg <nickname> <message> (private message)
// or in channel
// open new page
void Server::msg(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0 || args.size() < 2)
    {
        std::string error_message = ":yourserver.com 411 " + getClientFromFD(fd)->nick + " :No recipient given\r\n";
        send(fd, error_message.c_str(), error_message.size(), 0);
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

void Server::joinForSure(std::string channelName, int fd, int newChannel, Channel *channel, Client *client)
{
    // Join the new channel
    client->channel = channelName;
    channel->addUser(*client);

    // Notify other users in the channel
    std::string join_msg = ":" + client->nick + " JOIN " + channelName + "\r\n";
    std::vector<Client> users = channel->getUsers();
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (users[i].fd != client->fd)
        {
            send(users[i].fd, join_msg.c_str(), join_msg.size(), 0);
        }
    }

    // Handle new channel creation
    if (newChannel)
    {
        channel->addAdmin(client->nick);
        send(fd, "Channel created and you are the admin\r\n", 40, 0);
    }

    // Remove invite if necessary
    if (channel->isInvitee(client->nick))
    {
        channel->removeInvitee(client->nick);
        std::vector<std::string>::iterator it = std::find(client->invites.begin(), client->invites.end(), channelName);
        if (it != client->invites.end())
        {
            client->invites.erase(it);
        }
    }

    // Send join message to the client
    std::string join_message = ":" + client->nick + " JOIN " + channelName + "\r\n";
    send(fd, join_message.c_str(), join_message.size(), 0);
}

// /join #channel
// send to all clients in the channel
// join/create:admin if channel doesnt exist
// open another file with channel
// this as an error
void Server::join(int fd, std::vector<std::string> args)
{
    // Ensure the client is authenticated
    if (is_authentic(fd) != 0)
    {
        return;
    }

    // Check for correct arguments
    if (args.size() < 1 || args.size() > 2 || args[0][0] != '#')
    {
        send(fd, "Usage: JOIN #channel [password]\r\n", 34, 0);
        return;
    }

    std::string channelName = args[0];
    std::string password = (args.size() == 2) ? args[1] : "";

    // Find or create the channel
    int newChannel = 0;
    Channel *channel = findOrMakeChannel(&newChannel, channelName);
    if (channel == NULL)
    {
        send(fd, "Error: Channel could not be found or created\r\n", 46, 0);
        return;
    }

    // Get the client who is joining
    Client *client = getClientFromFD(fd);
    if (client == NULL)
    {
        send(fd, "Error: Client not found\r\n", 25, 0);
        return;
    }

    // Check if the client is already in the channel
    if (channel->isUser(client->nick))
    {
        send(fd, "You are already in this channel\r\n", 33, 0);
        return;
    }

    // Check if the channel is full
    if (channel->getisLimited() && channel->getUsers().size() >= channel->getLimit())
    {
        send(fd, "Channel is full\r\n", 17, 0);
        return;
    }

    if (channel->isInvitee(client->nick))
    {
        joinForSure(channelName, fd, newChannel, channel, client);
        return;
    }

    // Check if the channel is invite-only and if the client is invited
    if (channel->isInviteOnly() && !channel->isInvitee(client->nick))
    {
        send(fd, "You are not invited to this channel\r\n", 38, 0);
        return;
    }

    // Check for password protection
    if (channel->isPasswordProtected())
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
                for (size_t i = 0; i < previousChannel->getUsers().size() && hasNewAdmin; i++) // find the first non admin from users and make him admin
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
    joinForSure(channelName, fd, newChannel, channel, client);
}
