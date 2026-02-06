// Copyright 2026 hopkiw
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// TODO: support strings

typedef std::vector<std::string> Args;

int run_program(const Args& args) {
    int cpid = fork();
    if (cpid == -1) {
        perror("fork");
        return 1;
    }
    if (cpid == 0) {
        char** argv = new char* [args.size()];
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = const_cast<char*>(args[i].c_str());
        }
        int ret = execve(args[0].c_str(), argv, NULL);
        if (ret == -1) {
            perror(std::string("execve" + std::to_string(getpid())).c_str());
            return 1;
        }
        std::cout << "execve failed, ret is:" << ret << std::endl;
        return 1;
    }

    int wstatus;
    int ret = waitpid(cpid, &wstatus, 0);
    if (ret == -1) {
        perror("waitpid");
        return 1;
    }

    return 0;
}

int run_programs(const std::vector<Args>& programs) {
    std::vector<int> pids;
    std::vector<int> pipes;
    for (size_t i = 0; i < programs.size() - 1; ++i) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return 1;
        }
        pipes.push_back(pipefd[0]);
        pipes.push_back(pipefd[1]);
    }

    int pipe_idx = 0;
    for (auto it = programs.begin(); it != programs.end(); ++it) {
        auto program = *it;
        int cpid = fork();
        if (cpid == -1) {
            perror("fork");
            return 1;
        }
        if (cpid == 0) {
            if (it == programs.begin()) {
                dup2(pipes[1], STDOUT_FILENO);
            } else if ((it + 1) == programs.end()) {
                int idx = pipes.size() - 2;
                dup2(pipes[idx], STDIN_FILENO);
            } else {
                dup2(pipes[pipe_idx + 3], STDOUT_FILENO);
                dup2(pipes[pipe_idx], STDIN_FILENO);
            }
            for (auto pipefd : pipes)
                close(pipefd);

            char** argv = new char* [program.size()];
            for (size_t i = 0; i < program.size(); ++i) {
                argv[i] = const_cast<char*>(program[i].c_str());
            }
            int ret = execve(program[0].c_str(), argv, NULL);
            if (ret == -1) {
                perror(std::string("execve" + std::to_string(getpid())).c_str());
                return 1;
            }
            std::cout << "execve failed, ret is:" << ret << std::endl;
            return 1;
        }
        pids.push_back(cpid);
        if (it != programs.begin() && (it + 1) != programs.end())
            pipe_idx += 2;
    }

    for (auto pipefd : pipes) {
        close(pipefd);
    }

    for (auto rit = pids.rbegin(); rit != pids.rend(); ++rit) {
        // auto pid = *rit;
        int wstatus;
        int ret = waitpid(-1, &wstatus, 0);
        if (ret == -1) {
            perror("waitpid");
            return 1;
        }
    }

    return 0;
}

int main() {
    for (; ;) {
        std::string input;
        std::cout << ": ";
        std::getline(std::cin, input);

        std::vector<Args> programs;
        {
            Args words;
            std::string word;
            for (size_t i = 0; i < input.length(); ++i) {
                if (input[i] == ' ') {
                    if (word == "|") {
                        programs.push_back(words);
                        words.clear();
                        word.clear();
                        continue;
                    }
                    words.push_back(word);
                    word.clear();
                    continue;
                }
                word += input[i];
            }
            words.push_back(word);
            programs.push_back(words);
        }
        if (programs.size() == 1) {
            run_program(programs[0]);
        } else {
            run_programs(programs);
        }
    }

    return 0;
}
