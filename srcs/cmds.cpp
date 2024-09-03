#include "../incs/irc.hpp"

// TOPIC (to show)or TOPIC <new Topic>(to set)
void Server::topic(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0)
    {
        return;
    }

    Client *client = getClientFromFD(fd);
    Channel *channel = getChannelFromName(client->channel);

    if (channel == NULL)
    {
        std::string message = ":yourserver.com 442 " + client->nick + " :You're not on that channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (!channel->isAdmin(client->nick) && channel->isTopicRestricted())
    {
        std::string message = ":yourserver.com 482 " + client->nick + " " + channel->getName() + " :You're not a channel operator\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (args.size() == 0)
    {
        std::string topic = channel->getTopic();
        if (topic.empty())
        {
            std::string message = ":yourserver.com 331 " + client->nick + " " + channel->getName() + " :No topic is set\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else
        {
            std::string message = ":yourserver.com 332 " + client->nick + " " + channel->getName() + " :" + topic + "\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    // Set the new topic
    std::string new_topic = args[0];
    channel->setTopic(new_topic);

    // Notify all channel members about the new topic
    std::string topic_message = ":" + client->nick + "!" + client->name + "@" + client->ip + " TOPIC " + channel->getName() + " :" + new_topic + "\r\n";
    std::vector<Client> members = channel->getUsers();
    std::vector<Client>::iterator it;
    for (it = members.begin(); it != members.end(); ++it)
    {
        send((*it).fd, topic_message.c_str(), topic_message.size(), 0);
    }
}

void Server::mode(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0 || args.size() < 2) // Expect at least 2 arguments
    {
        return;
    }

    Client *admin = getClientFromFD(fd);
    Channel *channel = getChannelFromName(admin->channel);
    if (channel == NULL)
    {
        std::string message = ":yourserver.com 442 " + admin->nick + " :You are not in a channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }
    if (!channel->isAdmin(admin->nick))
    {
        std::string message = ":yourserver.com 482 " + admin->nick + " " + channel->getName() + " :You need to be a channel operator\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (args[0] == "k") // MODE k <password> / MODE 0 (remove password)
    {
        if (args[1] == "0")
        {
            channel->setPassword("");
            std::string message = ":yourserver.com 324 " + admin->nick + " " + channel->getName() + " :Password removed\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else
        {
            channel->setPassword(args[1]);
            std::string message = ":yourserver.com 324 " + admin->nick + " " + channel->getName() + " :Password set\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    if (args[0] == "t") // MODE t +/-(topic restricted)
    {
        if (args[1] == "+")
        {
            channel->setTopicRestricted(true);
            std::string message = ":yourserver.com 324 " + admin->nick + " " + channel->getName() + " :Topic restricted mode set\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else if (args[1] == "-")
        {
            channel->setTopicRestricted(false);
            std::string message = ":yourserver.com 324 " + admin->nick + " " + channel->getName() + " :Topic restricted mode unset\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    if (args[0] == "l") // MODE l <limit> / MODE 0 (remove limit)
    {
        if (args[1] == "0")
        {
            channel->setisLimited(false);
            std::string message = ":yourserver.com 324 " + admin->nick + " " + channel->getName() + " :User limit removed\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else
        {
            int limit = atoi(args[1].c_str());
            channel->setisLimited(true);
            channel->setLimit(limit);
            std::string message = ":yourserver.com 324 " + admin->nick + " " + channel->getName() + " :User limit set to " + args[1] + "\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    if (args[0] == "i") // MODE i +/-(invite only)
    {
        if (args[1] == "+")
        {
            channel->setInviteOnly(true);
            std::string message = ":yourserver.com 324 " + admin->nick + " " + channel->getName() + " :Invite only mode set\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else if (args[1] == "-")
        {
            channel->setInviteOnly(false);
            std::string message = ":yourserver.com 324 " + admin->nick + " " + channel->getName() + " :Invite only mode unset\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    if (args[0] == "o") // MODE o <nickname> (give operator privileges)
    {
        if (!channel->isUser(args[1]))
        {
            std::string message = ":yourserver.com 441 " + admin->nick + " " + channel->getName() + " " + args[1] + " :Client is not in the channel\r\n";
            send(fd, message.c_str(), message.size(), 0);
            return;
        }
        channel->addAdmin(args[1]);
        std::string message = ":yourserver.com 381 " + admin->nick + " " + channel->getName() + " :Operator privileges given to " + args[1] + "\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Handle unknown mode requests
    std::string unknown_mode = ":yourserver.com 472 " + admin->nick + " " + channel->getName() + " :Unknown mode flag\r\n";
    send(fd, unknown_mode.c_str(), unknown_mode.size(), 0);
}

// /kick <nickname>
void Server::kick(int fd, std::vector<std::string> args)
{
    std::cout << "attempted kick" << std::endl;
    
    // Check if the user is authenticated and there is exactly one argument
    if (is_authentic(fd) != 0 || args.size() != 1)
    {
        std::string message = ":yourserver.com 461 " + getClientFromFD(fd)->nick + " KICK :Not enough parameters\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }
    
    std::string nick = args[0];
    Client *admin = getClientFromFD(fd);
    Channel *channel = getChannelFromName(admin->channel);

    if (channel == NULL)
    {
        std::string message = ":yourserver.com 403 " + admin->nick + " " + admin->channel + " :No such channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (!channel->isAdmin(admin->nick))
    {
        std::string message = ":yourserver.com 482 " + admin->nick + " " + channel->getName() + " :You're not a channel operator\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (channel->isAdmin(nick))
    {
        std::string message = ":yourserver.com 482 " + admin->nick + " " + channel->getName() + " :Cannot kick an admin\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    Client *clientToKick = channel->getUserFromNick(nick);
    if (clientToKick == NULL)
    {
        std::string message = ":yourserver.com 441 " + admin->nick + " " + channel->getName() + " " + nick + " :They aren't on this channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Remove the client from the channel and update their state
    channel->KickUser(nick);
    clientToKick->channel = "";
    std::string kickMessage = ":yourserver.com KICK " + channel->getName() + " " + nick + " :You have been kicked from the channel\r\n";
    send(clientToKick->fd, kickMessage.c_str(), kickMessage.size(), 0);

    // Notify other clients in the channel
    std::vector<Client> users = channel->getUsers();
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (users[i].nick != nick)
        {
            std::string notifyMessage = ":" + admin->nick + "!" + admin->name + "@" + admin->ip + " KICK " + channel->getName() + " " + nick + " :You have been kicked from the channel\r\n";
            send(users[i].fd, notifyMessage.c_str(), notifyMessage.size(), 0);
        }
    }

    // Confirm the kick action to the admin
    std::string adminMessage = ":yourserver.com KICK " + channel->getName() + " " + nick + " :Client has been kicked\r\n";
    send(fd, adminMessage.c_str(), adminMessage.size(), 0);
}

// /invite <nickname>
void Server::invite(int fd, std::vector<std::string> args)
{
    // Ensure the user is authenticated and has provided exactly one argument
    if (is_authentic(fd) != 0 || args.size() != 1)
    {
        std::string message = ":yourserver.com 461 " + getClientFromFD(fd)->nick + " INVITE :Not enough parameters\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    Client *inviter = getClientFromFD(fd);
    Channel *channel = getChannelFromName(inviter->channel);

    if (channel == NULL)
    {
        std::string message = ":yourserver.com 442 " + inviter->nick + " :You are not in a channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (!channel->isAdmin(inviter->nick))
    {
        std::string message = ":yourserver.com 482 " + inviter->nick + " " + channel->getName() + " :You're not a channel operator\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    Client *invitee = getClientFromNick(args[0]);
    if (invitee == NULL)
    {
        std::string message = ":yourserver.com 401 " + inviter->nick + " " + args[0] + " :No such nick\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Ensure the invitee is not already in the channel
    if (invitee->channel == channel->getName())
    {
        std::string message = ":yourserver.com 443 " + inviter->nick + " " + channel->getName() + " " + invitee->nick + " :User already in channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Add the invitee to the list of invitees for the channel
    channel->addInvitee(*invitee);
    invitee->invites.push_back(channel->getName());

    // Notify the invitee about the invitation
    std::string invite_message = ":yourserver.com INVITE " + invitee->nick + " " + channel->getName() + "\r\n";
    send(invitee->fd, invite_message.c_str(), invite_message.size(), 0);

    // Confirm to the inviter that the invite has been sent
    std::string confirm_message = ":yourserver.com 341 " + inviter->nick + " " + invitee->nick + " " + channel->getName() + " :Invite sent\r\n";
    send(fd, confirm_message.c_str(), confirm_message.size(), 0);
}
