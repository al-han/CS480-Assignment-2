#ifndef PROCESSES_H
#define PROCESSES_H

#include <string>

void execute_command(const std::string& command, int in_fd, int out_fd, int err_fd);
std::string find_in_path(const std::string &command);
std::vector<std::string> split(const std::string &s, char delimiter);
void execute_pipe(const std::string& command);
bool isExecutable(const std::string& filename);
#endif // PROCESSES_H