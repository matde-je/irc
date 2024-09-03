#include "../incs/irc.hpp"

// /msg <nickname> <message> (private message)
// or in channel
// open new page
void Server::msg(int fd, std::vector<std::string> args)
{
    // Ensure the user is authenticated
    if (is_authentic(fd) != 0 || args.size() < 2)
    {
        std::string error_message = ":yourserver.com 411 " + getClientFromFD(fd)->nick + " :No recipient given\r\n";
        send(fd, error_message.c_str(), error_message.size(), 0);
        return;
    }

    std::string recipient = args[0];
    std::string content;
    for (size_t j = 1; j < args.size(); ++j)
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

    Client *sender = getClientFromFD(fd);

    // Check if the recipient is a client (private message)
    Client *recipientClient = getClientFromNick(recipient);
    if (recipientClient != NULL)
    {
        std::string privmsg = ":" + sender->nick + " PRIVMSG " + recipient + " :" + content + "\r\n";
        send(recipientClient->fd, privmsg.c_str(), privmsg.size(), 0);
        std::string confirm = ":yourserver.com 250 " + sender->nick + " :Message sent to " + recipient + "\r\n";
        send(fd, confirm.c_str(), confirm.size(), 0);
        return;
    }

    // Check if the recipient is a channel (channel message)
    Channel *channel = getChannelFromName(recipient);
    if (channel != NULL && !sender->channel.empty() && sender->channel == channel->getName())
    {
        std::string channel_msg = ":" + sender->nick + " PRIVMSG " + recipient + " :" + content + "\r\n";
        std::vector<Client> users = channel->getUsers();
        for (size_t i = 0; i < users.size(); ++i)
        {
            if (users[i].fd != fd)
            {
                send(users[i].fd, channel_msg.c_str(), channel_msg.size(), 0);
            }
        }
        std::string confirm = ":yourserver.com 250 " + sender->nick + " :Message sent to " + recipient + "\r\n";
        send(fd, confirm.c_str(), confirm.size(), 0);
        return;
    }

    // If no valid recipient found
    std::string error_message = ":yourserver.com 404 " + sender->nick + " " + recipient + " :No such user or channel\r\n";
    send(fd, error_message.c_str(), error_message.size(), 0);
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

    // Join the new channel
    client->channel = channelName;
    channel->addUser(*client);

    // Notify other users in the channel
    std::string join_msg = ":" + client->nick + " JOIN " + channelName + "\r\n";
    std::vector<Client> users = channel->getUsers();
    for (size_t i = 0; i < users.size(); ++i)
    {
        send(users[i].fd, join_msg.c_str(), join_msg.size(), 0);
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
