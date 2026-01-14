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
    for (size_t i = 0; i < args.size(); i++) { 
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

void cmd_pwd(const std::vector<std::string>& args) {
    fs::path cwd = fs::current_path();
    std::cout << cwd.string() << '\n';
}

void cmd_cd(const std::vector<std::string>& args) {
    if (args.empty()) {
            const char* home = std::getenv("HOME");
            fs::path hd(home);
            fs::current_path(hd);
            return;
    }
    
    std::vector<std::string> subdirs;
    std::stringstream ss(args[0]);
    std::string subdir;

    while (std::getline(ss, subdir, '/')) {
        subdirs.push_back(subdir); 
    }

    fs::path nwd;

    if (subdirs[0] == "~") {
        const char* home = std::getenv("HOME");
        nwd = home;
        subdirs.erase(subdirs.begin());
    } else if (subdirs[0] == "") {
        nwd = "/"; 
    } else {
        nwd = fs::current_path();
    }

    for (std::string subdir : subdirs) {
        if (subdir == ".") {
            continue;
        } else if (subdir == "..") {
            nwd = nwd.parent_path(); 
        } else {
            nwd /= subdir; 
        } 

        if (!fs::is_directory(nwd)) {
            std::cout << "cd: " << args[0] << ": No such file or directory\n";
            return;
        }
    }

    fs::current_path(nwd);
} 

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    
    commands = {
        {"exit", cmd_exit},
        {"echo", cmd_echo},
        {"type", cmd_type},
        {"pwd", cmd_pwd},
        {"cd", cmd_cd}
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

        size_t n = input.length();
        std::string token = "";
        std::vector<std::string> tokens;

        for (size_t i = 0; i < n; i++) {
            if (input[i] == ' ') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token = "";
                }

                size_t j = i;
                while (j + 1 < n && input[j + 1] == ' ') j++;

                i = j;
                continue;
            }

            if (input[i] == '\"') {
                size_t j = i;
                while (j + 1 < n && input[j + 1] != '\"') j++;
                
                token += input.substr(i + 1, j - i);
                i = j + 1;
                continue;
            }

            if (input[i] == '\'') {
                size_t j = i;
                while (j + 1 < n && input[j + 1] != '\'') j++;
                
                token += input.substr(i + 1, j - i);
                i = j + 1;
                continue;
            }

            size_t j = i;
            while (j + 1 < n && input[j + 1] != ' ' && input[j + 1] != '\"' && input[j + 1] != '\'') j++;
            
            token += input.substr(i, j - i + 1);
            i = j;
        }

        if (!token.empty()) {
            tokens.push_back(token);
        }
        
        std::string cmd = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        if (commands.count(cmd)) {
            commands[cmd](args);        
            continue;
        } 
        
        bool exec_found = false;
        for (std::string dir : dirs) {
            std::string path = dir + '/' + cmd;
            fs::path p(path);

            if (fs::exists(p) && access(path.c_str(), X_OK) == 0) {
                std::system(input.c_str());
                exec_found = true;
                break;
            }
        }
            
        if (!exec_found) {
            std::cout << cmd << ": command not found\n";
        }
    }
}
