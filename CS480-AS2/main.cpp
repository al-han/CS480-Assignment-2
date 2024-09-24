#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <vector>
#include "processes.h"

int main() {
    while(true) {
        std::cout << "cssc4405% ";
        std::string userInput;
        std::getline(std::cin, userInput);
        if(userInput == "exit") {
            std::cout << "Exiting..." <<std::endl;
            break;
        }else if(userInput.find("|") != std::string::npos){
            execute_pipe(userInput);
        }else {
            execute_command(userInput, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
        }
        if(userInput == "exCheck") {
            std::string filename;
            std::cout << "Checking..." <<std::endl;
            std::getline(std::cin, filename);
            isExecutable(filename);
        }
    }
}
