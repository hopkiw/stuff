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

    int connect_socket;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        if ((connect_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            std::cout << "Failed to create socket" << std::endl;
            return 1;
        }
        if (connect(connect_socket, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        // OTHERWISE
        close(connect_socket);
    }

    fd_set reads, writes;

    char buf[1024];
    bool datatosend = false;
    int rres = 0;
    std::string msg;

    std::cout << "Connected. 'quit' to quit" << std::endl;
    while (true) {
        FD_ZERO(&reads);
        FD_ZERO(&writes);
        FD_SET(connect_socket, &reads);
        FD_SET(0, &reads);
        if (datatosend)
            FD_SET(connect_socket, &writes);
        if (rres != 0)
            FD_SET(1, &writes);

        struct timeval notime = {0, 0};
        int selectval = select(connect_socket + 1, &reads, &writes, NULL, &notime);
        if (selectval == -1) {
            std::cout << "some kind of select error" << std::endl;
            return 1;
        }

        if (selectval == 0) {
            FD_SET(connect_socket, &reads);
            FD_SET(0, &reads);
            if (datatosend)
                FD_SET(connect_socket, &writes);
            if (rres != 0)
                FD_SET(1, &writes);
            select(connect_socket + 1, &reads, &writes, NULL, NULL);
        }

        if (rres != 0) {
            if (FD_ISSET(1, &writes)) {
                std::cout << buf << std::endl;
                rres = 0;
            }
        }

        if (datatosend) {
            if (FD_ISSET(connect_socket, &writes)) {
                msg += "\r\n";
                if ((send(connect_socket, msg.c_str(), msg.length(), 0)) == -1) {
                    std::cout << "Failed to send message: " << std::endl;
                    return 1;
                }
                datatosend = false;
            }
        }

        if (FD_ISSET(connect_socket, &reads)) {
            memset(buf, 0, 1024);

            rres = recv(connect_socket, buf, 1024, 0);
            if (rres == -1) {
                std::cout << "Failed to send message: " << std::endl;
                return 1;
            }

            if (rres == 0) {
                std::cout << "Connection closed by remote host." << std::endl;
                return 0;
            }
        }

        if (FD_ISSET(0, &reads)) {
            if (std::cin.eof()) {
                continue;
            }
            std::getline(std::cin, msg);

            if (msg == "quit") {
                std::cout << "quitting!" << std::endl;
                if (close(connect_socket) != 0) {
                    std::cout << "Failed to close socket: " << std::endl;
                    return 1;
                }
                break;
            }
            datatosend = true;
        }
    }

    std::cout << "all done." << std::endl;
    return 0;
}
