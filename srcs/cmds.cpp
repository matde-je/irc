#include "../incs/irc.hpp"


void Server::topic(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0)
    {
        return;
    }
    if (args.size() != 2)
    {
        return;
    }
}

void Server::mode(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0)
    {
        return;
    }
    if (args.size() != 2)
    {
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
        for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
            if (it->getName() == channelName)
            {
                channel = &(*it);
                break;
            }
        }

        if (channel == NULL)
        {
            send(fd, "The Channel indicated cannot be found\r\n", 39, 0);
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

void Server::invite(int fd, std::vector<std::string> args)
{
    if (is_authentic(fd) != 0)
    {
        return;
    }
    if (args.size() != 2)
    {
        return;
    }
}
