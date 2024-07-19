CC = c++

CFLAGS = -g -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp server.cpp client.cpp cmds.cpp

OBJS = $(SRCS:.cpp=.o)

name = ircserv

# Build name
$(name): $(OBJS)
		$(CC) $(CFLAGS) $(OBJS) -o $(name)

# Compile source files
%.o: %.cpp
		$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
fclean:
		rm -f $(OBJS) $(name)

clean:
		rm -f $(OBJS)
