#pragma once
#include <string>
#include <functional>
#include <map>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

struct Request {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
};

struct Response {
    int status = 200;
    std::string body;
    std::map<std::string, std::string> headers;
};

class HTTPServer {
private:
    SOCKET server_fd;
    int port;
    bool running = false;
    std::thread server_thread;
    std::mutex mtx;
    std::map<std::string, std::function<Response(const Request&)>> routes;

    void handle_client(SOCKET client_socket);
    static std::string read_line(SOCKET socket);
    static std::string read_headers(SOCKET socket, std::map<std::string, std::string>& headers);
    static void send_response(SOCKET socket, const Response& response);

public:
    HTTPServer(int port);
    ~HTTPServer();
    void start();
    void stop();
    void add_route(const std::string& method_path, std::function<Response(const Request&)> handler);
};