#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <string_view>
#include "command-parser.h"


void portScan(std::string host, std::vector<int> ports, bool service) {
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "[!] Error: could not create socket" << std::endl;
        return;
    }
    struct hostent *he = gethostbyname(host.c_str());
    struct in_addr **addr_list;
    if (he == nullptr) {
        std::cerr << "[!] Error: could not resolve hostname" << std::endl;
        return;
    }
    addr_list = (struct in_addr **) he -> h_addr_list;
    std::string ip = inet_ntoa(*addr_list[0]);

    // Set up socket address structure
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip.c_str());
    server.sin_family = AF_INET;

    // Loop through the specified ports
    for (int port : ports) {
        // Set the port number and print a message
        server.sin_port = htons(port);

        // Connect to server
        if (connect(sock, (struct sockaddr *) &server, sizeof(server)) != -1) {
            // If the connection was successful, print the port number and service name
            if (service) {
                struct servent *serv = getservbyport(htons(port), NULL);
                if (serv == NULL) {
                    std::cout << "[+] Port " << port << " is open" << std::endl;
                } else {
                    std::cout << "[+] Port " << port << " | " << serv->s_name << " | is open" << std::endl;
                }
            } else {
                std::cout << "[+] Port " << port << " is open" << std::endl;
            }

            close(sock);
            sock = socket(AF_INET, SOCK_STREAM, 0);
        } else {
            // If the connection timed out, assume the port is closed
            close(sock);
            sock = socket(AF_INET, SOCK_STREAM, 0);
        }
    }

    close(sock);
}

int main(int argc, char* argv[]) {
    portparser::parse(argc, argv);
    std::vector<int> ports = portparser::ports();
    std::string_view host_view = portparser::hostname();
    std::string host = {host_view.begin(), host_view.end()};
    bool service = portparser::show_service();

    portScan(host, ports, service);

    return 0;
}