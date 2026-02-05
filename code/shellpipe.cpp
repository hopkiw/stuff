// Copyright 2026 hopkiw
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// TODO: strings
// TODO: repeated whitespace
// TODO: more than two commands

void run_two_programs(const std::vector<std::string>& words1, const std::vector<std::string>& words2) {
    int pipefd[2];
    pid_t cpid, cpid2;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    cpid2 = fork();
    if (cpid2 == -1) {
        perror("fork");
        return;
    }
    if (cpid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        char * *argv = new char * [words2.size()];
        for (size_t i = 0; i < words2.size(); ++i) {
            argv[i] = const_cast<char*>(words2[i].c_str());
        }
        int ret = execve(words2[0].c_str(), argv, NULL);
        if (ret == -1) {
            perror(std::string("execve" + std::to_string(getpid())).c_str());
            return;
        }
        std::cout << "should be impossible, ret is:" << ret << std::endl;
        return;
    }

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        return;
    }
    if (cpid == 0) {
        dup2(pipefd[1], 1);
        dup2(pipefd[1], 2);
        close(pipefd[0]);
        close(pipefd[1]);

        char * *argv = new char * [words1.size()];
        for (size_t i = 0; i < words1.size(); ++i) {
            argv[i] = const_cast<char*>(words1[i].c_str());
        }
        int ret = execve(words1[0].c_str(), argv, NULL);
        if (ret == -1) {
            perror(std::string("execve" + std::to_string(getpid())).c_str());
            return;
        }
        std::cout << "should be impossible, ret is:" << ret << std::endl;
        return;
    }

    close(pipefd[0]);
    close(pipefd[1]);

    int wstatus;
    int ret = waitpid(cpid, &wstatus, 0);
    if (ret == -1) {
        perror("waitpid");
        return;
    }
    std::cout << "pid " << ret << "(" << words1[0] << ") exited with status: ";
    if (wstatus != 0 && WIFEXITED(wstatus))
        std::cout << WEXITSTATUS(wstatus) << std::endl;
    else
        std::cout << 0 << std::endl;


    ret = waitpid(cpid2, &wstatus, 0);
    if (ret == -1) {
        perror("waitpid");
        return;
    }
    std::cout << "pid " << ret << "(" << words2[0] << ") exited with status: ";
    if (wstatus != 0 && WIFEXITED(wstatus))
        std::cout << WEXITSTATUS(wstatus) << std::endl;
    else
        std::cout << 0 << std::endl;
}

int main() {
    run_two_programs({"/usr/bin/ls", "/"}, {"/usr/bin/grep", "l"});

    return 0;
}
