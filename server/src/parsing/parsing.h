#ifndef PARSING_H
#define PARSING_H

#include <string>
#include <vector>
#include <stdexcept>

class CommandParser {
public:
    // Parses a command like "command(arg1,arg2,...)" and returns the command name and arguments
    static std::pair<std::string, std::vector<std::string>> parseCommand(const std::string& input);

    // Validates and parses the "create" command
    static std::pair<int, std::string> parseCreate(const std::vector<std::string>& args);

    // Validates and parses the "set" command
    static std::pair<int, std::string> parseSet(const std::vector<std::string>& args);

    // Validates and parses the "get" command
    static int parseGet(const std::vector<std::string>& args);

    // Validates and parses the "increaseRefCount" and "decreaseRefCount" commands
    static int parseRefCount(const std::vector<std::string>& args);
};

#endif // PARSING_H