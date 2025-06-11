#include <iostream>

int main() {
    std::string input;

    std::cout << "Welcome to my Self Made CMD" << std::endl;
   
    std::cout << "Enter a command: ";
    std::getline(std::cin, input);
    if(input == "ipconfig") {
        system("ipconfig");
    }

    else if(input == "dir") {
        system("dir");
    }

    else if(input == "cls") {
        system("cls");
    }

    else if(input == "exit") {
        std::cout << "Exiting..." << std::endl;
        return 0;
    }

    else {
        std::cout << "Unknown command: " << input << std::endl;
    }


    return 0;
}