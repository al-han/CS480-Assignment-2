#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <sstream>
#include "processes.h"
#include <string>

bool isExecutable(const std::string& filename) {
     if(access(filename.c_str(), X_OK) == 0) {
        std::cout << "Success! This file is executable." <<std::endl;
        return true;
     }
     else {
        std::cerr << "Error: this file is not executeable" << std::endl;
        return false;
     }
}

std::vector<std::string> split(const std::string &s, char delimiter){
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while(std::getline(tokenStream, token, delimiter)){
        tokens.push_back(token);
    }
    return tokens;
}

std::string find_in_path(const std::string &command){
    const char* path_env = std::getenv("PATH");
    if(!path_env){
        std::cerr << "Error: PATH environment variable not found" << std::endl;
        return "";
    }

    std::vector<std::string> paths = split(path_env, ':');
    for(const std::string &path : paths){
        std::string full_path = path + "/" + command;
        if(access(full_path.c_str(), X_OK) == 0){
            return full_path;
        }
    }
    return "";
}

void execute_command(const std::string& command, int in_fd, int out_fd, int err_fd) {
    std::vector<std::string> args = split(command, ' ');
    std::vector<char*> argv;
    for(std::vector<std::string>::iterator it = args.begin(); it != args.end(); ++it){
        argv.push_back(&(*it)[0]);
    }
    argv.push_back(NULL);
    
    pid_t pid = fork();
    if(pid == 0){
        if(in_fd != STDIN_FILENO){
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if(out_fd != STDOUT_FILENO){
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        if(err_fd != STDERR_FILENO){
            dup2(err_fd, STDERR_FILENO);
            close(err_fd);
        }
        std::string cmd_path = find_in_path(argv[0]);
        if(!cmd_path.empty()){
            execv(cmd_path.c_str(), &argv[0]);
        }else{
            std::cerr << "Error: command not found" << argv[0] << std::endl;
        }
        exit(0);
    }else if(pid < 0){
        int status;
        waitpid(pid, &status, 0);
    }else{
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

void execute_pipe(const std::string& command) {
    std::vector<std::string> commands;
    size_t pos = 0, found;
    while ((commands[pos].find('|')) != std::string::npos) {
        commands.push_back(commands[pos].substr(pos, found - pos));
        pos = found + 1;
    }
    commands.push_back(commands[pos].substr(pos));

    int fd[2];
    pid_t pid;
    int fd_in = 0;

    for (size_t i = 0; i < commands.size(); ++i) {
        pipe(fd);
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            dup2(fd_in, 0); // Change the input according to the old one
            if (i < commands.size() - 1) {
                dup2(fd[1], 1);
            }
            close(fd[0]);
            close(fd[1]);

            std::vector<char*> args;
            char* cmd_token = strtok(&commands[i][0], " ");
            while (cmd_token != NULL) {
                args.push_back(cmd_token);
                cmd_token = strtok(NULL, " ");
            }
            args.push_back(NULL);

            char* cmd = args[0];
            if (!isExecutable(cmd)) {
                std::string pathString = find_in_path(cmd);
                const char* full_path = pathString.c_str();

                if (full_path) {
                    cmd = new char[strlen(full_path) + 1];
                    strcpy(cmd, full_path);
                } else {
                    std::cerr << "execvp: Not an executable file!\n";
                    exit(EXIT_FAILURE);
                }
            }
            if (execvp(cmd, args.data()) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process
            wait(NULL);
            close(fd[1]);
            fd_in = fd[0];
        }
    }
}