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
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>    // find

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
  std::vector<Header> Headers() const { return headers_; }
  std::string ToString() const;
  bool OK() const;

 private:
  int status_code_ = -1;
  int recv_bytes_ = 0;
  std::vector<Header> headers_;
};

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

void Response::ParseHeaders(const std::string& headers) {
    if (headers.length() == 0)
        return;

    typedef std::string::const_iterator iterator_t;

    iterator_t status_start = headers.begin();
    iterator_t status_end = std::find(status_start, headers.end(), '\r');
    if (status_end == headers.end()) {
        std::cout << "invalid response" << std::endl;
        return;
    }
    std::string check = &*(status_end);
    if (check.substr(0, 2) != "\r\n") {
        std::cout << "invalid response" << std::endl;
        return;
    }

    iterator_t verStart = status_start;
    iterator_t verEnd = std::find(verStart, status_end, ' ');
    if (verEnd == status_end) {
        std::cout << "invalid response" << std::endl;
        return;
    }
    std::string ver = std::string(verStart, verEnd);
    if (ver != "HTTP/1.1") {
        std::cout << "invalid HTTP version: " << ver << std::endl;
        return;
    }

    iterator_t code_start = verEnd + 1;
    iterator_t code_end = std::find(code_start, status_end, ' ');
    if (code_end == status_end) {
        std::cout << "invalid response" << std::endl;
        return;
    }
    std::string code = std::string(code_start, code_end);
    if (code.length() == 0) {
        std::cout << "error parsing headers:" << std::string(status_start, status_end) << std::endl;
        return;
    }
    status_code_ = std::stoi(code);

    iterator_t this_header_start = status_end + 2;
    for (; ;) {
        iterator_t this_header_end = std::find(this_header_start, headers.end(), '\r');
        iterator_t key_start = this_header_start;
        iterator_t key_end = std::find(this_header_start, this_header_end, ':');
        if (key_end == this_header_end) {
            std::cout << "invalid header format: " << std::endl;
            return;
        }
        std::string key(key_start, key_end);
        iterator_t value_start = key_end + 2;
        std::string value(value_start, this_header_end);
        headers_.push_back({key, value});
        this_header_start = this_header_end + 2;
        if (this_header_end == headers.end()) {
            break;
        }
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
    size_t longest = 0;
    for (auto hdr : headers) {
        if (hdr.first.length() > longest)
            longest = hdr.first.length();
    }
    for (auto hdr : headers) {
        std::cout << "Key: ";

        std::cout << std::left << std::setw(longest + 3) << ("\"" + hdr.first + "\"");
        std::cout << "Value: " << "\"" << hdr.second << "\"" << std::endl;
    }

    return 0;
}
