#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <map>
#include "network_utils.h"

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
    #define getcwd _getcwd
    #define chdir _chdir
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <dirent.h>
    #include <sys/stat.h>
#endif

class CommandPrompt {
private:
    std::vector<std::string> history;
    std::string currentDir;
    std::map<std::string, std::string> aliases;
    
    void updateCurrentDir() {
        char* cwd = getcwd(nullptr, 0);
        if (cwd) {
            currentDir = std::string(cwd);
            free(cwd);
        }
    }
    
    std::vector<std::string> parseCommand(const std::string& input) {
        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }
    
    void executeSystemCommand(const std::vector<std::string>& args) {
        if (args.empty()) return;
        
        std::string command;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) command += " ";
            command += args[i];
        }
        
        int result = system(command.c_str());
        if (result != 0) {
            std::cout << "Command failed with exit code: " << result << std::endl;
        }
    }
    
    void showHelp() {
        std::cout << "\n=== Custom Command Prompt Help ===" << std::endl;
        std::cout << "Built-in commands:" << std::endl;
        std::cout << "  help          - Show this help message" << std::endl;
        std::cout << "  exit/quit     - Exit the command prompt" << std::endl;
        std::cout << "  clear/cls     - Clear the screen" << std::endl;
        std::cout << "  pwd           - Print working directory" << std::endl;
        std::cout << "  cd <dir>      - Change directory" << std::endl;
        std::cout << "  ls/dir        - List directory contents" << std::endl;
        std::cout << "  history       - Show command history" << std::endl;
        std::cout << "  alias <name>=<command> - Create command alias" << std::endl;
        std::cout << "  aliases       - Show all aliases" << std::endl;
        std::cout << "  echo <text>   - Echo text to console" << std::endl;
        std::cout << "\nAll other commands are passed to the system shell." << std::endl;
        std::cout << "=======================================\n" << std::endl;
    }
    
    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }
    
    void listDirectory() {
#ifdef _WIN32
        WIN32_FIND_DATA findFileData;
        HANDLE hFind;
        
        std::string searchPath = currentDir + "\\*";
        hFind = FindFirstFile(searchPath.c_str(), &findFileData);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            std::cout << "Error accessing directory" << std::endl;
            return;
        }
        
        do {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
                std::cout << ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "[DIR]  " : "[FILE] ");
                std::cout << findFileData.cFileName << std::endl;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        
        FindClose(hFind);
#else
        DIR* dir = opendir(currentDir.c_str());
        if (dir == nullptr) {
            std::cout << "Error accessing directory" << std::endl;
            return;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                std::string fullPath = currentDir + "/" + entry->d_name;
                struct stat statbuf;
                
                if (stat(fullPath.c_str(), &statbuf) == 0) {
                    std::cout << (S_ISDIR(statbuf.st_mode) ? "[DIR]  " : "[FILE] ");
                } else {
                    std::cout << "[????] ";
                }
                std::cout << entry->d_name << std::endl;
            }
        }
        closedir(dir);
#endif
    }
    
    void changeDirectory(const std::string& path) {
        if (path.empty() || path == "~") {
#ifdef _WIN32
            const char* home = getenv("USERPROFILE");
#else
            const char* home = getenv("HOME");
#endif
            if (home && chdir(home) == 0) {
                updateCurrentDir();
                std::cout << "Changed to: " << currentDir << std::endl;
            } else {
                std::cout << "Could not change to home directory" << std::endl;
            }
        } else {
            if (chdir(path.c_str()) == 0) {
                updateCurrentDir();
                std::cout << "Changed to: " << currentDir << std::endl;
            } else {
                std::cout << "Directory not found: " << path << std::endl;
            }
        }
    }
    
    void showHistory() {
        std::cout << "\n=== Command History ===" << std::endl;
        for (size_t i = 0; i < history.size(); ++i) {
            std::cout << i + 1 << ": " << history[i] << std::endl;
        }
        std::cout << "========================\n" << std::endl;
    }
    
    void createAlias(const std::string& aliasCmd) {
        size_t eqPos = aliasCmd.find('=');
        if (eqPos == std::string::npos) {
            std::cout << "Usage: alias name=command" << std::endl;
            return;
        }
        
        std::string name = aliasCmd.substr(0, eqPos);
        std::string command = aliasCmd.substr(eqPos + 1);
        
        // Trim whitespace
        name.erase(name.find_last_not_of(" \t") + 1);
        name.erase(0, name.find_first_not_of(" \t"));
        command.erase(command.find_last_not_of(" \t") + 1);
        command.erase(0, command.find_first_not_of(" \t"));
        
        aliases[name] = command;
        std::cout << "Alias created: " << name << " -> " << command << std::endl;
    }
    
    void showAliases() {
        if (aliases.empty()) {
            std::cout << "No aliases defined." << std::endl;
            return;
        }
        
        std::cout << "\n=== Aliases ===" << std::endl;
        for (const auto& pair : aliases) {
            std::cout << pair.first << " -> " << pair.second << std::endl;
        }
        std::cout << "===============\n" << std::endl;
    }
    
    std::string getDirectoryName(const std::string& path) {
        size_t pos = path.find_last_of("/\\");
        if (pos == std::string::npos) {
            return path;
        }
        return path.substr(pos + 1);
    }
    
    std::string resolveAlias(const std::string& command) {
        auto it = aliases.find(command);
        return (it != aliases.end()) ? it->second : command;
    }
    
    void echoText(const std::vector<std::string>& args) {
        for (size_t i = 1; i < args.size(); ++i) {
            if (i > 1) std::cout << " ";
            std::cout << args[i];
        }
        std::cout << std::endl;
    }
    
public:
    CommandPrompt() {
        updateCurrentDir();
        std::cout << "=== Custom C++ Command Prompt ===" << std::endl;
        std::cout << "Type 'help' for available commands." << std::endl;
        std::cout << "Type 'exit' or 'quit' to leave.\n" << std::endl;
    }
    
    void run() {
        std::string input;
        
        while (true) {
            // Display prompt
            std::cout << "[" << getDirectoryName(currentDir) << "]$ ";
            
            // Get input
            if (!std::getline(std::cin, input)) {
                break; // EOF or error
            }
            
            // Skip empty input
            if (input.empty()) {
                continue;
            }
            
            // Add to history
            history.push_back(input);
            
            // Parse command
            std::vector<std::string> args = parseCommand(input);
            if (args.empty()) continue;
            
            // Resolve alias
            args[0] = resolveAlias(args[0]);
            
            // Handle built-in commands
            std::string cmd = args[0];
            std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
            
            if (cmd == "exit" || cmd == "quit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }
            else if (cmd == "help") {
                showHelp();
            }
            else if (cmd == "clear" || cmd == "cls") {
                clearScreen();
            }
            else if (cmd == "pwd") {
                std::cout << currentDir << std::endl;
            }
            else if (cmd == "cd") {
                if (args.size() > 1) {
                    changeDirectory(args[1]);
                } else {
                    changeDirectory("");
                }
            }
            else if (cmd == "ls" || cmd == "dir") {
                listDirectory();
            }
            else if (cmd == "history") {
                showHistory();
            }
            else if (cmd == "alias") {
                if (args.size() > 1) {
                    std::string aliasCmd = input.substr(input.find(' ') + 1);
                    createAlias(aliasCmd);
                } else {
                    std::cout << "Usage: alias name=command" << std::endl;
                }
            }
            else if (cmd == "aliases") {
                showAliases();
            }
            else if (cmd == "echo") {
                echoText(args);
            }
            else if (cmd == "ipconfig") {
                NetworkUtils::displayIpConfig();
            }
            else {
                // Execute as system command
                executeSystemCommand(args);
            }
        }
    }
};

int main() {
    try {
        CommandPrompt cmd;
        cmd.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}