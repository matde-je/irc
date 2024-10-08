#CC = c++

#CFLAGS = -g -Wall -Wextra -Werror -std=c++98

#SRCS = main.cpp server.cpp client.cpp cmds.cpp join_msg.cpp channel.cpp

#OBJS = $(SRCS:.cpp=.o)

#name = ircserv

# Build name
#$(name): $(OBJS)
#		$(CC) $(CFLAGS) $(OBJS) -o $(name)

# Compile source files
#%.o: %.cpp
#		$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
#fclean:
#		rm -f $(OBJS) $(name)

#clean:
#		rm -f $(OBJS)

#run: re
#		./ircserv 4555 123

#re: fclean $(name)
# Compiler and compilation flags
#
# REMOVE @ IF DONT WANT SILENT.
CC = c++
CFLAGS = -g  -std=c++98 -Iincs -Wall -Wextra -Werror

# Directories
SRCDIR = srcs
INCDIR = incs
BUILDDIR = build

# Source files
SRCS = $(SRCDIR)/main.cpp \
	$(SRCDIR)/server.cpp \
	$(SRCDIR)/client.cpp \
	$(SRCDIR)/cmds.cpp \
	$(SRCDIR)/join_msg.cpp \
	$(SRCDIR)/channel.cpp

# Object files
OBJS = $(BUILDDIR)/main.o \
	$(BUILDDIR)/server.o \
	$(BUILDDIR)/client.o \
	$(BUILDDIR)/cmds.o \
	$(BUILDDIR)/join_msg.o \
	$(BUILDDIR)/channel.o

# Executable name
name = ircserv

# Colors
GREEN = \033[1;32m
RESET = \033[0m
BLUE = \033[1;34m

# Build the executable
$(name): $(OBJS)
		clear
		@echo "$(GREEN)Building $(name)...$(RESET)"
		@$(CC) $(CFLAGS) $(OBJS) -o $(name)
		@echo "$(GREEN)Build successful!$(RESET)"

# Compile source files into object files
$(BUILDDIR)/main.o: $(SRCDIR)/main.cpp
		@mkdir -p $(BUILDDIR)
		@echo "$(BLUE)Compiling $<...$(RESET)"
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(BLUE)Compilation of $< successful!$(RESET)"

$(BUILDDIR)/server.o: $(SRCDIR)/server.cpp
		@mkdir -p $(BUILDDIR)
		@echo "$(BLUE)Compiling $<...$(RESET)"
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(BLUE)Compilation of $< successful!$(RESET)"

$(BUILDDIR)/client.o: $(SRCDIR)/client.cpp
		@mkdir -p $(BUILDDIR)
		@echo "$(BLUE)Compiling $<...$(RESET)"
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(BLUE)Compilation of $< successful!$(RESET)"

$(BUILDDIR)/cmds.o: $(SRCDIR)/cmds.cpp
		@mkdir -p $(BUILDDIR)
		@echo "$(BLUE)Compiling $<...$(RESET)"
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(BLUE)Compilation of $< successful!$(RESET)"

$(BUILDDIR)/join_msg.o: $(SRCDIR)/join_msg.cpp
		@mkdir -p $(BUILDDIR)
		@echo "$(BLUE)Compiling $<...$(RESET)"
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(BLUE)Compilation of $< successful!$(RESET)"

$(BUILDDIR)/channel.o: $(SRCDIR)/channel.cpp
		@mkdir -p $(BUILDDIR)
		@echo "$(BLUE)Compiling $<...$(RESET)"
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(BLUE)Compilation of $< successful!$(RESET)"

# Clean build files
fclean:
		@rm -f $(OBJS) $(name)

clean:
		@rm -f $(OBJS)
		@rm -rf $(BUILDDIR)

# Run the program
run: re
		@./ircserv 4555 123

mem: re	
		valgrind --leak-check=full --show-leak-kinds=all --log-file="val.log" ./ircserv 4555 123

user:
		nc localhost 4555
		user:
				@echo "PASS 123" | nc localhost 4555

# Rebuild the project
re: fclean $(name)

