#include "HTTPServer.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>

HTTPServer::HTTPServer(int port) : port(port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "Setsockopt failed: " << WSAGetLastError() << "\n";
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

HTTPServer::~HTTPServer() {
    stop();
    closesocket(server_fd);
    WSACleanup();
}

void HTTPServer::start() {
    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
        return;
    }
    running = true;
    server_thread = std::thread([this]() {
        while (running) {
            sockaddr_in client_addr{};
            int addr_len = sizeof(client_addr);
            SOCKET client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
            if (client_socket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
                }
                continue;
            }
            std::thread client_thread(&HTTPServer::handle_client, this, client_socket);
            client_thread.detach();
        }
        });
    std::cout << "Server started on port " << port << "\n";
}

void HTTPServer::stop() {
    running = false;

   
    closesocket(server_fd);

    if (server_thread.joinable()) {
        server_thread.join();
    }
}

void HTTPServer::add_route(const std::string& method_path, std::function<Response(const Request&)> handler) {
    std::lock_guard<std::mutex> lock(mtx);
    routes[method_path] = handler;
}

std::string HTTPServer::read_line(SOCKET socket) {
    std::string line;
    char ch;

    while (true) {
        int bytes_received = recv(socket, &ch, 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Connection closed by client\n";
            }
            else {
                std::cerr << "Recv error: " << WSAGetLastError() << "\n";
            }
            return "";
        }

        if (ch == '\n') break;
        if (ch != '\r') line += ch;
    }

    return line;
}

std::string HTTPServer::read_headers(SOCKET socket, std::map<std::string, std::string>& headers) {
    std::string line;
    while (true) {
        line = read_line(socket);
        if (line.empty()) break;

        
        if (line.empty() || line == "\r" || line == "\n") {
            break;
        }

        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            std::string value = line.substr(colon + 2);
            headers[key] = value;
        }
    }

    std::string body;
    if (headers.count("content-length")) {
        try {
            int length = std::stoi(headers["content-length"]);
            if (length > 0) {
                body.resize(length);
                int total_received = 0;
                while (total_received < length) {
                    int received = recv(socket, &body[total_received], length - total_received, 0);
                    if (received <= 0) break;
                    total_received += received;
                }
            }
        }
        catch (...) {
            
        }
    }
    return body;
}

void HTTPServer::send_response(SOCKET socket, const Response& response) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << response.status << " ";

    
    switch (response.status) {
    case 200: oss << "OK"; break;
    case 201: oss << "Created"; break;
    case 400: oss << "Bad Request"; break;
    case 404: oss << "Not Found"; break;
    default: oss << "OK"; break;
    }

    oss << "\r\n";
    oss << "Content-Type: application/json\r\n";
    oss << "Content-Length: " << response.body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "\r\n";
    oss << response.body;

    std::string response_str = oss.str();
    int sent = send(socket, response_str.c_str(), response_str.size(), 0);
    if (sent == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << "\n";
    }
}

void HTTPServer::handle_client(SOCKET client_socket) {
    Request req;

    
    DWORD timeout = 5000; 
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    std::string request_line = read_line(client_socket);
    if (request_line.empty()) {
        closesocket(client_socket);
        return;
    }

   
    std::cout << "Request: " << request_line << std::endl;

    std::istringstream iss(request_line);
    iss >> req.method >> req.path;

 
    req.body = read_headers(client_socket, req.headers);

    Response res;
    std::string key = req.method + " " + req.path;

    std::cout << "Route key: " << key << std::endl;

    
    if (routes.count(key)) {
        res = routes[key](req);
    }
   

    else if (req.path.find("/tasks/") == 0) {
       
        size_t last_slash = req.path.find_last_of('/');
        if (last_slash != std::string::npos) {
            std::string base_path = req.path.substr(0, last_slash);
            std::string dynamic_key = req.method + " " + base_path + "/";

            std::cout << "Trying dynamic key: " << dynamic_key << std::endl;

            if (routes.count(dynamic_key)) {
                res = routes[dynamic_key](req);
            }
            else {
                res.status = 404;
                res.body = "{\"error\":\"Route not found: " + dynamic_key + "\"}";
            }
        }
        else {
            res.status = 404;
            res.body = "{\"error\":\"Invalid path\"}";
        }
    }
    else {
        res.status = 404;
        res.body = "{\"error\":\"Route not found: " + key + "\"}";
    }

  
    std::cout << "Response status: " << res.status << std::endl;
    std::cout << "Response body: " << res.body << std::endl;

    send_response(client_socket, res);


    closesocket(client_socket);
}