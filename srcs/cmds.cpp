#include "../incs/irc.hpp"

//TOPIC (to show)or TOPIC <new Topic>(to set)
void Server::topic(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0)
    {return;}
    Client *admin = getClientFromFD(fd);
    Channel *channel = getChannelFromName(admin->channel);
    if (channel == NULL)
    {
        send(fd, "You are not in a channel\r\n", 26, 0);
        return;
    }
    if (!channel->isAdmin(admin->nick) && channel->isTopicRestricted())
    {
        send(fd, "You need to be an admin to set the topic\r\n", 42, 0);
        return;
    }
    if (args.size() == 0)
    {
        std::string topic = channel->getTopic();
        std::string message = "TOPIC " + channel->getName() + " :" + topic + "\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }
    std::string new_topic = args[0];
    channel->setTopic(new_topic);
}

void Server::mode(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0 || args.size() != 2)
    {return;}
    Client *admin = getClientFromFD(fd);
    Channel *channel = getChannelFromName(admin->channel);
    if (channel == NULL)
    {
        send(fd, "You are not in a channel\r\n", 26, 0);
        return;
    }
    if (!channel->isAdmin(admin->nick))
    {
        send(fd, "You need to be an admin to set the mode\r\n", 42, 0);
        return;
    }
    if (args[0] == "k")// MODE k <password> / MODE 0 (remove password)
    {
        if (args[1] == "0")
        {
            channel->setPassword("");
            send(fd, "Password removed\r\n", 18, 0);
        }
        else
        {
            channel->setPassword(args[1]);
            send(fd, "Password set\r\n", 14, 0);
        }
        return;
    }
    if (args[0] == "t")// MODE t +/-(topic restricted)
    {
        if (args[1] == "+")
        {
            channel->setTopicRestricted(true);
            send(fd, "Topic restricted mode set\r\n", 28, 0);
        }
        else if (args[1] == "-")
        {
            channel->setTopicRestricted(false);
            send(fd, "Topic restricted mode unset\r\n", 30, 0);
        }
        return;
    }
    if (args[0] == "l")// MODE l <limit> / MODE 0(set user limit)
    {
        if (args[1] == "0")
        {
            channel->setisLimited(false);
            send(fd, "User limit removed\r\n", 20, 0);
        }
        else if(args[1] == "1")
        {
            channel->setisLimited(true);
            channel->setLimit(atoi(args[2].c_str()));
            send(fd, "User limit set\r\n", 16, 0);
        }
        return;
    }
    if (args[0] == "i")// MODE i +/-(invite only)
    {
        if (args[1] == "+")
        {
            channel->setInviteOnly(true);
            send(fd, "Invite only mode set\r\n", 23, 0);
        }
        else if (args[1] == "-")
        {
            channel->setInviteOnly(false);
            send(fd, "Invite only mode unset\r\n", 25, 0);
        }
        return;
    }
    if (args[0] == "o")// MODE o <nickname> (give operator privileges)
    {
        if (!channel->isUser(args[1]))
        {
            send(fd, "Client is not in the channel\r\n", 29, 0);
            return;
        }
        channel->addAdmin(args[1]);
        send(fd, "Operator privileges given\r\n", 29, 0);
        return;
    }
}

// /kick <nickname>
void Server::kick(int fd, std::vector<std::string> args)
{
    std::cout << "attempted kick" << std::endl;
    if (is_authentic(fd) == 0 && args.size() == 1)
    {
        std::string nick = args[0];
        // Find the client in the channel
        Client* ronaldo = getClientFromFD(fd);

        std::string channelName = ronaldo->channel;

        // Find the channel
        Channel* channel = NULL;
        for (size_t i = 0; i < channels.size(); i++)
        {
            if (channels[i].getName() == channelName)
            {
                channel = &channels[i];
                break;
            }
        }

        if (channel == NULL)
        {
            send(fd, "The Channel indicated cannot be found\r\n", 39, 0);
            return;
        }

        if(!channel->isAdmin(ronaldo->nick)){
            send(fd, "To KICK another user you need to be an admin\r\n", 48, 0);
            return;
        }

        if(channel->isAdmin(nick)){
            send(fd, "You cannot kick an admin\r\n", 27, 0);
            return;
        }

        Client *ball = NULL;
        ball = channel->getUserFromNick(nick);
        if (ball == NULL)
        {
            std::string error_message = "Client " + nick + " not found in channel: " + channel->getName() + "\r\n";
            send(fd, error_message.c_str(), error_message.size(), 0);
            return;
        }

        // Remove the client from the channel
        channel->KickUser(nick);
        ball->channel = "";
        send(ball->fd, "You have been kicked from the channel\r\n", 36, 0);
        std::vector<Client> users = channel->getUsers();
        // Notify other clients in the channel about the kick
        for (size_t i = 0; i < users.size(); i++)
        {
            if (users[i].nick != nick)
            {
                std::string notify = nick + " has been kicked from " + channel->getName() + "\r\n";
                send(users[i].fd, notify.c_str(), notify.size() + 1, 0);
            }
        }

        // Send a kick message to the client
        std::string kick_message = "KICK " + channel->getName() + " " + nick + "\r\n";
        send(fd, kick_message.c_str(), kick_message.size(), 0);

        return;
    }
}

// /invite <nickname>
void Server::invite(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0 || args.size() != 1)
    {return;}
    Client *inviter = getClientFromFD(fd);
    Channel *channel = getChannelFromName(inviter->channel);
    if (channel == NULL)
    {
        send(fd, "You are not in a channel\r\n", 26, 0);
        return;
    }
    if (!channel->isAdmin(inviter->nick))
    {
        send(fd, "You need to be an admin to invite\r\n", 36, 0);
        return;
    }
    Client *invitee = getClientFromNick(args[0]);
    if (invitee == NULL)
    {
        send(fd, "Client not found\r\n", 18, 0);
        return;
    }
    invitee->invites.push_back(channel->getName());
    send(invitee->fd, "You have been invited to the channel: ", 34, 0);
    send(invitee->fd, (channel->getName() + "\r\n").c_str(), channel->getName().size() + 2, 0);

    channel->addInvitee(*invitee);
    send(fd, "Invite sent\r\n", 13, 0);
    return ;
}
