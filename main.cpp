#include "irc.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {std::cerr << "Invalid number of arguments.\n"; return 1;}
    char* endptr;
    long num = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1]|| num > 65535 || num <= 0 || argv[1][0] == '0') {std::cerr << "Error: invalid port.\n"; return 1;}
  

}