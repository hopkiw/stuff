// Copyright 2026 hopkiw
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// TODO: support strings
// TODO: path lookups in run_programs
// TODO: error if receiving EOF in main loop

typedef std::vector<std::string> Args;

std::string find_program(const std::string& program, const std::vector<std::string>& paths) {
    std::string res = program;
    for (const auto& path : paths) {
        std::string candidate = path + "/" + program;
        struct stat sb;
        if (lstat(candidate.c_str(), &sb) == 0) {
            res = candidate;
            break;
        }
    }
    return res;
}

int run_program(const Args& args, const std::vector<std::string>& paths) {
    auto program = args[0];
    if (program[0] != '/') {
        program = find_program(program, paths);
        if (program[0] != '/') {
            std::cout << "unable to find program: " << program << std::endl;
            return 1;
        }
    }
    int cpid = fork();
    if (cpid == -1) {
        perror("fork");
        return 1;
    }
    if (cpid == 0) {
        char** argv = new char* [args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = const_cast<char*>(args[i].c_str());
        }
        argv[args.size()] = NULL;
        int ret = execve(program.c_str(), argv, NULL);
        if (ret == -1) {
            perror(std::string("execve" + std::to_string(getpid())).c_str());
            exit(1);
        }
        std::cout << "execve failed, ret is:" << ret << std::endl;
        exit(1);
    }

    int wstatus;
    int ret = waitpid(cpid, &wstatus, 0);
    if (ret == -1) {
        perror("waitpid");
        return 1;
    }

    return 0;
}

int run_programs(const std::vector<Args>& programs, const std::vector<std::string>& paths) {
    std::vector<Args> programs_ = programs;
    for (auto& ref : programs_) {
        auto program = ref[0];
        if (program[0] != '/') {
            program = find_program(program, paths);
            if (program[0] != '/') {
                std::cout << "unable to find program: " << program << std::endl;
                return 1;
            }
            ref[0] = program;
        }
    }

    std::vector<int> pids;
    std::vector<int> pipes;
    for (size_t i = 0; i < programs_.size() - 1; ++i) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return 1;
        }
        pipes.push_back(pipefd[0]);
        pipes.push_back(pipefd[1]);
    }

    int pipe_idx = 0;
    for (auto it = programs_.begin(); it != programs_.end(); ++it) {
        auto program = *it;
        int cpid = fork();
        if (cpid == -1) {
            perror("fork");
            return 1;
        }
        if (cpid == 0) {
            if (it == programs_.begin()) {
                dup2(pipes[1], STDOUT_FILENO);
            } else if ((it + 1) == programs_.end()) {
                int idx = pipes.size() - 2;
                dup2(pipes[idx], STDIN_FILENO);
            } else {
                dup2(pipes[pipe_idx + 3], STDOUT_FILENO);
                dup2(pipes[pipe_idx], STDIN_FILENO);
            }
            for (auto pipefd : pipes)
                close(pipefd);

            char** argv = new char* [program.size() + 1];
            for (size_t i = 0; i < program.size(); ++i) {
                argv[i] = const_cast<char*>(program[i].c_str());
            }
            argv[program.size()] = NULL;
            int ret = execve(program[0].c_str(), argv, NULL);
            if (ret == -1) {
                perror(std::string("execve" + std::to_string(getpid())).c_str());
                exit(1);
            }
            std::cout << "execve failed, ret is:" << ret << std::endl;
            exit(1);
        }
        pids.push_back(cpid);
        if (it != programs_.begin() && (it + 1) != programs_.end())
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

int main(int argc, const char* argv[], const char* env[]) {
    if (argc == 1000)
        std::cout << "using argv" << argv[0] << env[0];

    std::string path;
    for (const char** var = env; *var != nullptr; ++var) {
        std::string field(*var);
        if (field.substr(0, 5) == "PATH=") {
            path = field.substr(5);
            break;
        }
    }

    std::vector<std::string> paths;
    for (size_t i = 0, end = 0; end != std::string::npos; i = end + 1) {
        end = path.find(':', i);
        std::string p = path.substr(i, end - i);
        paths.push_back(p);
        if (paths.size() > 26) {
            exit(2);
        }
    }

    for (; ;) {
        if (std::cin.eof()) {
            return 0;
        }

        std::string input;
        std::cout << ": ";
        std::getline(std::cin, input);

        if (std::cin.eof()) {
            return 0;
        }

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
            run_program(programs[0], paths);
        } else {
            run_programs(programs, paths);
        }
    }

    return 0;
}
