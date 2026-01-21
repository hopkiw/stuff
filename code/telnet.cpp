// Copyright hopkiw 2026
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Invalid input; too few arguments" << std::endl;
        std::cerr << "To get started, type " << argv[0] <<
        " IP address and port" << std::endl;
        return 1;
    }

    struct addrinfo hints;
    struct addrinfo *res, *rp;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
        std::cout << "error looking up host" << std::endl;
        return 1;
    }

    int sockfd;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            std::cout << "Failed to create socket" << std::endl;
            return 1;
        }
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        // OTHERWISE
        close(sockfd);
    }

    fd_set reads, writes;

    char buf[1024];
    int recv_bytes = 0;
    std::string msg;

    std::cout << "Connected. 'quit' to quit" << std::endl;
    while (true) {
        FD_ZERO(&reads);
        FD_ZERO(&writes);
        FD_SET(sockfd, &reads);
        FD_SET(STDIN_FILENO, &reads);
        if (msg.length())
            FD_SET(sockfd, &writes);
        if (recv_bytes != 0)
            FD_SET(1, &writes);

        int selectval = select(sockfd + 1, &reads, &writes, NULL, NULL);
        if (selectval == -1) {
            std::cout << "some kind of select error" << std::endl;
            return 1;
        }

        if (recv_bytes != 0) {
            if (FD_ISSET(STDOUT_FILENO, &writes)) {
                std::cout << buf << std::endl;
                recv_bytes = 0;
            }
        }

        if (FD_ISSET(sockfd, &writes)) {
            msg += "\r\n";
            if ((send(sockfd, msg.c_str(), msg.length(), 0)) == -1) {
                std::cout << "Failed to send message: " << std::endl;
                return 1;
            }
            msg.clear();
        }

        if (FD_ISSET(sockfd, &reads)) {
            memset(buf, 0, 1024);

            recv_bytes = recv(sockfd, buf, 1024, 0);
            if (recv_bytes == -1) {
                std::cout << "Failed to send message: " << std::endl;
                return 1;
            }

            if (recv_bytes == 0) {
                std::cout << "Connection closed by remote host." << std::endl;
                return 0;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &reads)) {
            if (std::cin.eof()) {
                continue;
            }
            std::getline(std::cin, msg);

            if (msg == "quit") {
                std::cout << "quitting!" << std::endl;
                if (close(sockfd) != 0) {
                    std::cout << "Failed to close socket: " << std::endl;
                    return 1;
                }
                break;
            }
        }
    }

    std::cout << "all done." << std::endl;
    return 0;
}
