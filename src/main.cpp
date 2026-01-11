#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdlib>
#include <filesystem>
#include <unistd.h>

namespace fs = std::filesystem;

std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> commands;
std::vector<std::string> dirs;

void cmd_exit(const std::vector<std::string>& args) {
    exit(0);
}

void cmd_echo(const std::vector<std::string>& args) {
    std::string out = ""; 
    for (size_t i = 0; i < args.size(); i++) {
        out += args[i] + " \n"[i == args.size() - 1]; 
    }

    std::cout << out;
}

void cmd_type(const std::vector<std::string>& args) {
    for (int i = 0; i < args.size(); i++) { 
        std::string cmd = args[i];
        if (commands.count(cmd)) {
            std::cout << cmd << " is a shell builtin\n"; 
            return;
        }

        for (std::string dir : dirs) {
            std::string path = dir + '/' + cmd;
            fs::path p(path);

            if (fs::exists(p) && access(path.c_str(), X_OK) == 0) {
                std::cout << cmd << " is " << path << '\n';
                return;
            }
        }

        std::cout << cmd << ": not found\n";
    }
}

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    
    commands = {
        {"exit", cmd_exit},
        {"echo", cmd_echo},
        {"type", cmd_type}
    };

    const char* path = std::getenv("PATH");
    std::stringstream ss(path);
    std::string dir;

    while (std::getline(ss, dir, ':')) {
        dirs.push_back(dir);
    }

    while (true) {
        std::cout << "$ ";

        std::string input;
        std::getline(std::cin, input);

        std::stringstream ss(input);
        std::vector<std::string> tokens;
        std::string token;

        while (ss >> token) {
            tokens.push_back(token);
        }

        std::string cmd = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        if (commands.count(cmd)) {
            commands[cmd](args);        
        } else {
            std::cout << cmd << ": command not found\n";
        }
    }
}
