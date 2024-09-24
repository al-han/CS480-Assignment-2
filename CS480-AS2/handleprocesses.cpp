#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <cstring>
#include "processes.h"

void handle_processes(const std::string& input) {
    std::vector<std::string> commands;
    size_t start = 0;
    size_t end = input.find('|');

    // Split the input by the pipe symbol '|'
    while (end != std::string::npos) {
        commands.push_back(input.substr(start, end - start));
        start = end + 1;
        end = input.find('|', start);
    }
    commands.push_back(input.substr(start));

    int num_pipes = commands.size() - 1;
    int pipefds[2*num_pipes];

    for(int i = 0; i < num_pipes; i++){
        if(pipe(pipefds + i*2) < 0) {
            perror("couldn't pipe");
            exit(EXIT_FAILURE);
        }
    }

    int pid;
    int status;
    int command_index = 0;

        // Continue from the previous code snippet
    for(int i = 0; i <= num_pipes; i++){
        pid = fork();
        if(pid == 0) { // Child process
            if(i < num_pipes){ // Not the last command
                if(dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0){
                    perror("dup2"); // Redirect stdout to pipe write end
                    exit(EXIT_FAILURE);
                }
            }
            if(i > 0){ // Not the first command
                if(dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0){
                    perror("dup2"); // Redirect stdin to pipe read end
                    exit(EXIT_FAILURE);
                }
            }
            for(int j = 0; j < 2*num_pipes; j++){
                close(pipefds[j]); // Close all pipe fds
            }
            // Split the command by spaces to separate the command from its arguments
            std::vector<std::string> args;
            std::string cmd = commands[i];
            size_t start = 0, end = cmd.find(' ');
            while (end != std::string::npos) {
                args.push_back(cmd.substr(start, end - start));
                start = end + 1;
                end = cmd.find(' ', start);
            }
            args.push_back(cmd.substr(start));
            std::vector<char*> cargs(args.size() + 1); // +1 for NULL terminator
            for(size_t j = 0; j < args.size(); ++j) {
                cargs[j] = const_cast<char*>(args[j].c_str());
            }
            cargs[args.size()] = nullptr;
            execvp(cargs[0], cargs.data());
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if(pid < 0){
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
    // Parent process closes all pipe fds and waits for children
    for(int i = 0; i < 2 * num_pipes; i++){
        close(pipefds[i]);
    }
    for(int i = 0; i <= num_pipes; i++){
        wait(nullptr);
    }
}