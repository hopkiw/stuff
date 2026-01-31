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
#include <fstream>
#include <string>
#include <vector>

namespace http {

typedef std::pair<std::string, std::string> Header;

class HTTPClient {
 public:
  HTTPClient() {}

  int Connect(const std::string&);
  int Get(const std::string&, const std::string&);
  std::vector<Header> Headers() { return headers_; }

 private:
  void Parse(const std::string&);

  std::vector<Header> headers_;
  int sockfd_ = -1;
};

int HTTPClient::Get(const std::string& domain, const std::string& path) {
    if (sockfd_ == -1) {
        if (Connect(domain) != 0) {
            std::cout << "error connecting" << std::endl;
            return 1;
        }
        if (sockfd_ == -1) {
            std::cout << "error connecting also" << std::endl;
            return 1;
        }
    }

    char buf[1024];
    int recv_bytes = 0;

    std::string msg = "GET " + path + " HTTP/1.1\r\n";
    msg += "Host: " + domain + "\r\n";
    msg += "Connection: close\r\n";
    msg += "\r\n";

    if ((send(sockfd_, msg.c_str(), msg.length(), 0)) == -1) {
        std::cout << "Failed to send message" << std::endl;
        return 1;
    }

    std::ofstream output_file;

    std::string headers;
    int body = -1;
    do {
        memset(buf, 0, 1024);
        recv_bytes = recv(sockfd_, buf, 1024, 0);
        if (recv_bytes == -1) {
            std::cout << "Failed to recv message" << std::endl;
            return 1;
        }

        if (recv_bytes == 0) {
            std::cout << "Connection closed by remote host." << std::endl;
            break;
        }
        if (body == -1) {
            for (int i = 0; i < recv_bytes - 4; ++i) {
                if (buf[i] == '\r'
                        && buf[i + 1] == '\n'
                        && buf[i + 2] == '\r'
                        && buf[i + 3] == '\n') {
                    body = i + 4;
                    break;
                }
                headers += buf[i];
            }
        }

        if (body != -1) {
            // store the remainder of buf as the start of the body
            if (!output_file.is_open()) {
                output_file.open("./" + path);
            }
            for (int i = body; i < recv_bytes; ++i) {
                output_file << buf[i];
            }
            body = 0;
        }
    } while (recv_bytes == 1024);
    output_file.flush();

    Parse(headers);
    return 0;
}

int HTTPClient::Connect(const std::string& domain) {
    struct addrinfo hints;
    struct addrinfo *res, *rp;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((getaddrinfo(domain.c_str(), "80", &hints, &res)) != 0) {
        std::cout << "error looking up host" << std::endl;
        return 1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        if ((sockfd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            std::cout << "Failed to create socket" << std::endl;
            return 1;
        }

        if (connect(sockfd_, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        // OTHERWISE
        close(sockfd_);
    }

    return 0;
}

void HTTPClient::Parse(const std::string &headers) {
    bool inheaders = false;
    std::string signature;
    for (size_t i = 0; i < headers.length();) {
        if (headers[i] == '\r' && headers[i + 1] == '\n') {
            inheaders = true;
            i += 2;
            continue;
        }

        if (!inheaders) {
            signature += headers[i];
            ++i;
            continue;
        }

        std::string key;
        for (; i < headers.length() && headers[i] != ':'; ++i) {
            key += headers[i];
        }
        i += 2;

        std::string value;
        for (; i < headers.length() && headers[i] != '\r'; ++i) {
            value += headers[i];
        }

        headers_.push_back({key, value});
        i += 2;
    }
}

}  // namespace http

int main(int argc, char** argv) {
    // command-line arguments
    if (argc < 3) {
        std::cerr << "Invalid input; too few arguments" << std::endl;
        std::cerr << "To get started, type " << argv[0] <<
        " IP address and path" << std::endl;
        return 1;
    }

    http::HTTPClient client;
    client.Get(argv[1], argv[2]);

    auto headers = client.Headers();
    std::cout << "got " << headers.size() << " headers:" << std::endl;
    for (auto hdr : headers) {
        std::cout << "Key: \"" << hdr.first << "\", Value: \"" << hdr.second << "\"" << std::endl;
    }

    std::cout << "wrote file ./" << argv[2] << std::endl;

    return 0;
}
