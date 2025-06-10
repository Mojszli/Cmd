#include <iostream>
#include <string>

int main() {
    std::string input;
    std::cout << "Enter a command: ";
    std::getline(std::cin, input);

    if(input == "ipconfig") {
        std::cout << "Executing ipconfig command..." << std::endl;
        std::system("ipconfig");
        } 
    
    else {
        std::cout << "Unknown command: " << input << std::endl;
    }

    return 0;
}