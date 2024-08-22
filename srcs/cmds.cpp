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

// /kick #channel <nickname>
void Server::kick(int fd, std::vector<std::string> args)
{
    std::cout << "attemptred kick" << std::endl;
    if (is_authentic(fd) == 0 && args.size() == 2)
    {
        Channel *channel = NULL;
        for (size_t i = 0; i < channels.size(); i++)
        {
            if (channels[i].getName() == args[0])
                channel = &channels[i];
        }
        if (channel == NULL)
        {
            send(fd, "The Channel indicated cannot be found\r\n", 39, 0);
            return;
        }
        Client *client = NULL;

        // Find the client in the clients vector
        for (size_t i = 0; i < clients.size(); i++)
        {
            if (clients[i].name == args[1])
            {
                client = &clients[i];
            }
        }

        if (client == NULL)
        {
            std::string error_message = "Client " + args[1] + " not found in channel: " + channel->getName() + "\r\n";
            send(fd, error_message.c_str(), error_message.size(), 0);
        }

        // Remove the client from the channel
        channel->KickUser(client->name);
        client->channel = NULL;
        send(client->fd, "You have been kicked from the channel\r\n", 36, 0);

        // Notify other clients in the channel about the kick
        for (size_t j = 0; j < channel->getUsers().size(); j++)
        {
                std::string notify = client->name + " has been kicked from " + channel->getName() + "\r\n";
                send(channel->getUsers()[j].fd, notify.c_str(), notify.size(), 0);

        }

        // Send a kick message to the client
        std::string kick_message = "KICK " + channel->getName() + " " + client->name + "\r\n";
        send(fd, kick_message.c_str(), kick_message.size(), 0);

        // TODO: Implement any additional logic for handling the kick

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
