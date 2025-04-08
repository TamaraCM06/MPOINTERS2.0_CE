#include "parsing.h"
#include <sstream>
#include <algorithm>

std::pair<std::string, std::vector<std::string>> CommandParser::parseCommand(const std::string& input) {
    // Find the opening and closing parentheses
    size_t open_paren = input.find('(');
    size_t close_paren = input.find(')');

    if (open_paren == std::string::npos || close_paren == std::string::npos || close_paren <= open_paren) {
        throw std::invalid_argument("Invalid command format. Expected format: command(arg1,arg2,...)");
    }

    // Extract the command name
    std::string command = input.substr(0, open_paren);
    command.erase(std::remove_if(command.begin(), command.end(), ::isspace), command.end()); // Remove spaces

    // Extract the arguments
    std::string args_str = input.substr(open_paren + 1, close_paren - open_paren - 1);
    std::vector<std::string> args;
    std::stringstream ss(args_str);
    std::string arg;

    while (std::getline(ss, arg, ',')) {
        arg.erase(std::remove_if(arg.begin(), arg.end(), ::isspace), arg.end()); // Remove spaces
        args.push_back(arg);
    }

    return {command, args};
}

std::pair<int, std::string> CommandParser::parseCreate(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw std::invalid_argument("Invalid arguments for create. Expected: create(size,type)");
    }

    int size = std::stoi(args[0]);
    std::string type = args[1];
    return {size, type};
}

std::pair<int, std::string> CommandParser::parseSet(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw std::invalid_argument("Invalid arguments for set. Expected: set(id,value)");
    }

    int id = std::stoi(args[0]);
    std::string value = args[1];
    return {id, value};
}

int CommandParser::parseGet(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        throw std::invalid_argument("Invalid arguments for get. Expected: get(id)");
    }

    return std::stoi(args[0]);
}

int CommandParser::parseRefCount(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        throw std::invalid_argument("Invalid arguments for refCount. Expected: refCount(id)");
    }

    return std::stoi(args[0]);
}