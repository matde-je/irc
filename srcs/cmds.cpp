#include "../incs/irc.hpp"



//TOPIC <channelname> (to show)or TOPIC <channelname> <new Topic>(to set)
void Server::topic(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0)
    {
        return;
    }

    Client *admin = getClientFromFD(fd);
    if (args.size() < 1)
    {
        std::string message = ":IRC 461 " + admin->getNick() + " TOPIC :Not enough parameters\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    Channel *channel = getChannelFromName(args[0]);
    if (channel == NULL)
    {
        std::string message = ":IRC 442 " + admin->getNick() + " :You're not on that channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (!channel->isAdmin(admin->getNick()) && channel->isTopicRestricted())
    {
        std::string message = ":IRC 482 " + admin->getNick() + " " + channel->getName() + " :You're not a channel operator\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (args.size() == 1)
    {
        std::string topic = channel->getTopic();
        if (topic.empty())
        {
            std::string message = ":IRC 331 " + admin->getNick() + " " + channel->getName() + " :No topic is set\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else
        {
            std::string message = ":IRC 332 " + admin->getNick() + " " + channel->getName() + " :" + topic + "\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    std::string new_topic = args[1];
    channel->setTopic(new_topic);

    // Notify all channel members about the new topic
    std::string topic_message = ":" + admin->getNick() + "!" + admin->getName() + "@" + admin->getIp() + " TOPIC " + channel->getName() + " :" + new_topic + "\r\n";
    std::vector<Client> members = channel->getUsers();
    std::vector<Client>::iterator it;
    for (it = members.begin(); it != members.end(); ++it)
    {
        send((*it).getFd(), topic_message.c_str(), topic_message.size(), 0);
    }
}
// /mode <channel> <mode> <args>
// /mode <channel> <mode> <args>
void Server::mode(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0 || args.size() < 2)
    {
        return;
    }

    Client *admin = getClientFromFD(fd);
    Channel *channel = getChannelFromName(args[0]);
    if (channel == NULL)
    {
        std::string message = ":IRC 442 " + admin->getNick() + " :You are not in a channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (!channel->isAdmin(admin->getNick()))
    {
        std::string message = ":IRC 482 " + admin->getNick() + " " + channel->getName() + " :You need to be a channel operator\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Trim any leading or trailing whitespace from args[1]
    std::string mode = args[1];
    mode.erase(0, mode.find_first_not_of(" \t\n\r\f\v"));
    mode.erase(mode.find_last_not_of(" \t\n\r\f\v") + 1);

    for(size_t i = 0; i < args.size(); i++)
    {
        std::cout << "args[" << i << "] = '" << args[i] << "'" << std::endl;
    }
    std::cout << mode << " | valid: " << (mode == "1") << std::endl;

    if (mode == "k") // MODE k <password> / MODE 0 (remove password)
    {
        if (args.size() < 3)
        {
            std::string message = ":IRC 461 " + admin->getNick() + " " + channel->getName() + " :Not enough parameters\r\n";
            send(fd, message.c_str(), message.size(), 0);
            return;
        }

        if (args[2] == "0")
        {
            channel->setPassword("");
            channel->setPasswordProtected(false);
            std::string message = ":IRC 324 " + admin->getNick() + " " + channel->getName() + " :Password removed\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else
        {
            channel->setPassword(args[2]);
            channel->setPasswordProtected(true);
            std::string message = ":IRC 324 " + admin->getNick() + " " + channel->getName() + " :Password set\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    if (mode == "t") // MODE t +/-(topic restricted)
    {
        if (args.size() < 3)
        {
            std::string message = ":IRC 461 " + admin->getNick() + " " + channel->getName() + " :Not enough parameters\r\n";
            send(fd, message.c_str(), message.size(), 0);
            return;
        }

        if (args[2] == "+")
        {
            channel->setTopicRestricted(true);
            std::string message = ":IRC 324 " + admin->getNick() + " " + channel->getName() + " :Topic restricted mode set\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else if (args[2] == "-")
        {
            channel->setTopicRestricted(false);
            std::string message = ":IRC 324 " + admin->getNick() + " " + channel->getName() + " :Topic restricted mode unset\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    if (mode == "l") // MODE l <limit> / MODE 0 (remove limit)
    {
                std::cout << "attempted invite only" << std::endl;

        if (args.size() < 3)
        {
            std::string message = ":IRC 461 " + admin->getNick() + " " + channel->getName() + " :Not enough parameters\r\n";
            send(fd, message.c_str(), message.size(), 0);
            return;
        }

        if (args[2] == "0")
        {
            channel->setisLimited(false);
            std::string message = ":IRC 324 " + admin->getNick() + " " + channel->getName() + " :User limit removed\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else
        {
            int limit = atoi(args[2].c_str());
            channel->setisLimited(true);
            channel->setLimit(limit);
            std::string message = ":IRC 324 " + admin->getNick() + " " + channel->getName() + " :User limit set to " + args[2] + "\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    if (mode == "i") // MODE i +/-(invite only)
    {
        if (args.size() < 3)
        {
            std::string message = ":IRC 461 " + admin->getNick() + " " + channel->getName() + " :Not enough parameters\r\n";
            send(fd, message.c_str(), message.size(), 0);
            return;
        }

        if (args[2] == "+")
        {
            channel->setInviteOnly(true);
            std::string message = ":IRC 324 " + admin->getNick() + " " + channel->getName() + " :Invite only mode set\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        else if (args[2] == "-")
        {
            channel->setInviteOnly(false);
            std::string message = ":IRC 324 " + admin->getNick() + " " + channel->getName() + " :Invite only mode unset\r\n";
            send(fd, message.c_str(), message.size(), 0);
        }
        return;
    }

    if (mode == "o") // MODE o <nickname> (give operator privileges)
    {
        if (args.size() < 3)
        {
            std::string message = ":IRC 461 " + admin->getNick() + " " + channel->getName() + " :Not enough parameters\r\n";
            send(fd, message.c_str(), message.size(), 0);
            return;
        }

        if (!channel->isUser(args[2]))
        {
            std::string message = ":IRC 441 " + admin->getNick() + " " + channel->getName() + " " + args[2] + " :Client is not in the channel\r\n";
            send(fd, message.c_str(), message.size(), 0);
            return;
        }
        channel->addAdmin(args[2]);
        std::string message = ":IRC 381 " + admin->getNick() + " " + channel->getName() + " :Operator privileges given to " + args[2] + "\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Handle unknown mode requests
    std::string unknown_mode = ":IRC 472 " + admin->getNick() + " " + channel->getName() + " :Unknown mode flag\r\n";
    send(fd, unknown_mode.c_str(), unknown_mode.size(), 0);
}
// /kick <channelname> <nickname>
void Server::kick(int fd, std::vector<std::string> args)
{
    std::cout << "attempted kick" << std::endl;
    // Check if the user is authenticated and there are exactly two arguments
    if (is_authentic(fd) != 0 || args.size() != 2)
    {
        std::string message = ":IRC 461 " + getClientFromFD(fd)->getNick() + " KICK :Not enough parameters\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    std::string channelName = args[0];
    std::string nick = args[1];

    Client* admin = getClientFromFD(fd);
    Channel* channel = getChannelFromName(channelName);

    if (channel == NULL)
    {
        std::string message = ":IRC 403 " + admin->getNick() + " " + channelName + " :No such channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (!channel->isAdmin(admin->getNick()))
    {
        std::string message = ":IRC 482 " + admin->getNick() + " " + channelName + " :You're not a channel operator\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (channel->isAdmin(nick))
    {
        std::string message = ":IRC 482 " + admin->getNick() + " " + channelName + " :Cannot kick an admin\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    Client* target = channel->getUserFromNick(nick);
    if (target == NULL)
    {
        std::string message = ":IRC 441 " + admin->getNick() + " " + channelName + " " + nick + " :They aren't on this channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Remove the client from the channel
    target->removeChannel(channelName);
    channel->KickUser(nick);

    std::string kickMessage = ":IRC KICK " + channelName + " " + nick + " :You have been kicked from the channel\r\n";
    send(target->getFd(), kickMessage.c_str(), kickMessage.size(), 0);

    // Notify other clients in the channel
    std::vector<Client> users = channel->getUsers();
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (users[i].getNick() != nick && users[i].getNick() != admin->getNick())
        {
            std::string notifyMessage = ":" + admin->getNick() + "!" + admin->getName() + "@" + admin->getIp() + " KICK " + channelName + " " + nick + ": has been kicked from the channel\r\n";
            send(users[i].getFd(), notifyMessage.c_str(), notifyMessage.size(), 0);
        }
    }

    // Confirm the kick action to the admin
    std::string adminMessage = ":IRC KICK " + channelName + " " + nick + " :Client has been kicked\r\n";
    send(fd, adminMessage.c_str(), adminMessage.size(), 0);
}

// /invite <channelname> <nickname>
void Server::invite(int fd, std::vector<std::string> args)
{
    // Ensure the user is authenticated and has provided exactly one argument
    if (is_authentic(fd) != 0 || args.size() != 1)
    {
        std::string message = ":IRC 461 " + getClientFromFD(fd)->getNick() + " INVITE :Not enough parameters\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    Client *inviter = getClientFromFD(fd);
    Channel *channel = getChannelFromName(args[0]);

    if (channel == NULL)
    {
        std::string message = ":IRC 442 " + inviter->getNick() + " :You are not in a channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    if (!channel->isAdmin(inviter->getNick()))
    {
        std::string message = ":IRC 482 " + inviter->getNick() + " " + channel->getName() + " :You're not a channel operator\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    Client *invitee = getClientFromNick(args[0]);
    if (invitee == NULL)
    {
        std::string message = ":IRC 401 " + inviter->getNick() + " " + args[0] + " :No such nick\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Ensure the invitee is not already in the channel
    if (invitee->isInChannel(channel->getName()))
    {
        std::string message = ":IRC 443 " + inviter->getNick() + " " + channel->getName() + " " + invitee->getNick() + " :User already in channel\r\n";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }

    // Add the invitee to the list of invitees for the channel
    channel->addInvitee(*invitee);
    invitee->getInvites().push_back(channel->getName());

    // Notify the invitee about the invitation
    std::string invite_message = ":IRC INVITE " + invitee->getNick() + " " + channel->getName() + "\r\n";
    send(invitee->getFd(), invite_message.c_str(), invite_message.size(), 0);

    // Confirm to the inviter that the invite has been sent
    std::string confirm_message = ":IRC 341 " + inviter->getNick() + " " + invitee->getNick() + " " + channel->getName() + " :Invite sent\r\n";
    send(fd, confirm_message.c_str(), confirm_message.size(), 0);
}