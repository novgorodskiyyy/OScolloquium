#include "HTTPServer.hpp"
#include "router.hpp"
#include <iostream>

int main() {
    HTTPServer server(8080);
    Router router;
    router.setup_routes(server);

    std::cout << "To-Do API Server started on http://localhost:8080\n";
    std::cout << "Waiting for Postman requests...\n";

    server.start();

    
    std::cin.get();
    server.stop();

    return 0;
}