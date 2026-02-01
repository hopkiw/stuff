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
#include <algorithm>    // find

// todo: add 'request' and 'response' classes

namespace http {

class URI {
 public:
  std::string QueryString, Path, Protocol, Host, Port;

  static URI Parse(const std::string &uri) {
    URI result;

    typedef std::string::const_iterator iterator_t;

    if (uri.length() == 0)
        return result;

    iterator_t uriEnd = uri.end();

    iterator_t queryStart = std::find(uri.begin(), uriEnd, '?');

    iterator_t protocolStart = uri.begin();
    iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':');

    if (protocolEnd != uriEnd) {
        std::string prot = &*(protocolEnd);
        if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
            result.Protocol = std::string(protocolStart, protocolEnd);
            protocolEnd += 3;
        } else {
            protocolEnd = uri.begin();  // no protocol
        }
    } else {
        protocolEnd = uri.begin();  // no protocol
    }

    iterator_t hostStart = protocolEnd;
    iterator_t pathStart = std::find(hostStart, uriEnd, '/');

    iterator_t hostEnd = std::find(protocolEnd, (pathStart != uriEnd) ? pathStart : queryStart, ':');

    result.Host = std::string(hostStart, hostEnd);

    if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':')) {
        ++hostEnd;
        iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
        result.Port = std::string(hostEnd, portEnd);
    }

    if (pathStart != uriEnd)
        result.Path = std::string(pathStart, queryStart);

    if (queryStart != uriEnd)
        result.QueryString = std::string(queryStart, uri.end());

    return result;
    }   // URI::Parse
};  // URI

typedef std::pair<std::string, std::string> Header;

class Request {
 public:
  explicit Request(const URI& uri, const std::string& method = "GET") : method_{method}, uri_{uri} {}
  std::vector<Header> Headers() const { return headers_; }
  URI Uri() const { return uri_; }
  void AddHeader(const std::string&, const std::string&);
  std::string ToString() const;

 private:
  std::string method_;
  URI uri_;
  std::vector<Header> headers_;
};

void Request::AddHeader(const std::string& key, const std::string& value) {
    headers_.push_back({key, value});
}

std::string Request::ToString() const {
    std::string ret = method_ + " " + uri_.Path + " HTTP/1.1\r\n";
    ret += "Host: " + uri_.Host + "\r\n";
    for (auto pair : headers_)
        ret += pair.first + ": " + pair.second + "\r\n";
    ret += "\r\n";

    return ret;
}

class Response {
 public:
  Response() {}
  explicit Response(int status_code) : status_code_{status_code} {}

  void ParseHeaders(const std::string&);
  void SetStatusCode(int status_code) { status_code_ = status_code; }
  int StatusCode() const { return status_code_; }
  void SetRecvBytes(int recv_bytes) { recv_bytes_ = recv_bytes; }
  int RecvBytes() const { return recv_bytes_; }
  void AddHeader(const std::string&, const std::string&);
  std::vector<Header> Headers() const { return headers_; }
  std::string ToString() const;
  bool OK() const;

 private:
  int status_code_ = -1;
  int recv_bytes_ = 0;
  std::vector<Header> headers_;
};

void Response::AddHeader(const std::string& key, const std::string& value) {
    headers_.push_back({key, value});
}

bool Response::OK() const {
    return ((status_code_ >= 200) && (status_code_ < 300));
}

std::string Response::ToString() const {
    return "Status: " + std::to_string(status_code_);
}


class Client {
 public:
  Client() {}

  int Connect(const std::string&);
  Response Do(const Request&);

 private:
  int sockfd_ = -1;
};

Response Client::Do(const Request& request) {
    Response res;
    URI uri = request.Uri();
    if (sockfd_ == -1) {
        if (Connect(uri.Host) != 0) {
            std::cout << "error connecting" << std::endl;
            return res;
        }
        if (sockfd_ == -1) {
            std::cout << "error connecting also" << std::endl;
            return res;
        }
    }

    char buf[1024];
    int recv_bytes = 0;

    std::string msg = request.ToString();
    if ((send(sockfd_, msg.c_str(), msg.length(), 0)) == -1) {
        std::cout << "Failed to send message" << std::endl;
        return res;
    }

    std::ofstream output_file;

    std::string headers;
    int body = -1;
    int bodylen = 0;

    std::string newpath;
    if (uri.Path[0] == '/') {
        newpath.assign(uri.Path, 1, uri.Path.length());
    } else {
        newpath.assign(uri.Path, 0, uri.Path.length());
    }

    do {
        memset(buf, 0, 1024);
        recv_bytes = recv(sockfd_, buf, 1024, 0);
        if (recv_bytes == -1) {
            std::cout << "Failed to recv message" << std::endl;
            return res;
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
                output_file.open("./" + newpath);
            }
            for (int i = body; i < recv_bytes; ++i) {
                output_file << buf[i];
                ++bodylen;
            }
            body = 0;
        }
    } while (recv_bytes == 1024);

    close(sockfd_);
    sockfd_ = -1;
    output_file.flush();
    std::cout << "wrote file " << newpath << std::endl;

    res.ParseHeaders(headers);
    res.SetStatusCode(-1);
    res.SetRecvBytes(bodylen);

    return res;
}

int Client::Connect(const std::string& domain) {
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

void Response::ParseHeaders(const std::string &headers) {
    std::vector<Header> res;

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

        res.push_back({key, value});
        i += 2;
    }
}

}  // namespace http

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Invalid input; too few arguments" << std::endl;
        std::cerr << "To get started, type " << argv[0] << " <URL>" << std::endl;
        return 1;
    }

    http::URI uri = http::URI::Parse(argv[1]);
    if (uri.Host == "") {
        std::cout << "invalid URI" << std::endl;
        return 1;
    }

    http::Request request(uri);
    request.AddHeader("Connection", "Close");

    http::Client client;
    http::Response response = client.Do(request);

    if (!response.OK())
        std::cout << "Error response from server: " << response.StatusCode() << std::endl;

    auto headers = response.Headers();
    std::cout << "got " << headers.size() << " headers:" << std::endl;
    for (auto hdr : headers) {
        std::cout << "Key: \"" << hdr.first << "\", Value: \"" << hdr.second << "\"" << std::endl;
    }

    return 0;
}
