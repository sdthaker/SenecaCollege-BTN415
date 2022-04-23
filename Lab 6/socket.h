#pragma once
#include <string>
#include <iostream>
#include <windows.networking.sockets.h>

#define MAX_BUFFER_SIZE 512

class Node {
protected:
    static bool dlls_started;
    static int num_nodes;
    std::string ip;
    std::string role;
    std::string protocol;
    int port;
    SOCKET active_socket;
public:
    Node();
    Node(std::string ip, int port);
    ~Node();
    void start_dlls() const;
    bool create_socket();
    void display_info() const;
};

class Server_UDP : public Node {
public:
    Server_UDP();
    Server_UDP(std::string ip, int port);
    ~Server_UDP();
    bool bind_socket();
    int send_message(std::string message, std::string IP, int port);
    struct sockaddr_in receive_message(std::string& message);
};

class Client_UDP : public Node {
public:
    Client_UDP();
    int send_message(std::string message, std::string IP, int port);
    struct sockaddr_in receive_message(std::string& message);
};