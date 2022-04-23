#include "socket.h"
#include <string>
#include <iostream>
#include <windows.networking.sockets.h>
#include <stdlib.h>
#pragma comment(lib, "Ws2_32.lib")

bool Node::dlls_started = false;
int Node::num_nodes = 0;

Node::Node() {
    start_dlls();
    num_nodes++;
    this->active_socket = INVALID_SOCKET;
    this->protocol = "tcp";
    this->ip = "127.0.0.1";
    this->port = 27000;
}

Node::Node(std::string ip, int port) {
    start_dlls();
    num_nodes++;
    this->active_socket = INVALID_SOCKET;
    this->ip = ip;
    this->port = port;
}

Node::~Node() {
    closesocket(this->active_socket);
    std::cout << "Closing socket" << std::endl;
    num_nodes--;
    if (num_nodes == 0) {
        WSACleanup();
    }
}

void Node::start_dlls() const {
    if (!dlls_started) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "Could not start DLLs" << std::endl;
            exit(EXIT_FAILURE);
        }
        else {
            dlls_started = true;
        }
    }
}

bool Node::create_socket() {
    if (this->protocol == "tcp") {
        this->active_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    else {
        this->active_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

    if (this->active_socket == INVALID_SOCKET) {
        std::cout << "Could not create socket" << std::endl;
        return false;
    }
    else {
        return true;
    }
}

void Node::display_info() const {
    std::cout << "IP: " << this->ip << std::endl;
    std::cout << "port: " << this->port << std::endl;
    std::cout << "protocol: " << this->protocol << std::endl;
    std::cout << "dlls: " << dlls_started << std::endl;
}


//Server_UDP
Server_UDP::Server_UDP() : Node() {
    this->role = "server";
}

Server_UDP::Server_UDP(std::string ip, int port) : Node(ip, port) {
    this->protocol = "udp";
    this->role = "server";
}

Server_UDP::~Server_UDP() {
    std::cout << "Closing client socket" << std::endl;
}

bool Server_UDP::bind_socket() {
    struct sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_addr.s_addr = inet_addr(this->ip.c_str());
    SvrAddr.sin_port = htons(this->port);
    if (bind(this->active_socket, (struct sockaddr*)&SvrAddr,
        sizeof(SvrAddr)) == SOCKET_ERROR) {
        std::cout << "Could not bind socket to address" << std::endl;
        return false;
    }
    else {
        return true;
    }
}

int Server_UDP::send_message(std::string message, std::string IP, int port) {

    struct sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_addr.s_addr = inet_addr(IP.c_str());
    SvrAddr.sin_port = htons(port);

    return sendto(this->active_socket, message.c_str(), sizeof(message), 0,
        (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
}

struct sockaddr_in Server_UDP::receive_message(std::string& message) {

    char RxBuffer[MAX_BUFFER_SIZE] = {};

    struct sockaddr_in CltAddr;
    int addr_len = sizeof(CltAddr);

    std::cout << "Ready to receive messages." << std::endl;

    recvfrom(this->active_socket, RxBuffer, sizeof(RxBuffer), 0,
        (struct sockaddr*)&CltAddr, &addr_len);

    message = RxBuffer;
    return CltAddr;
}


//Client_UDP
Client_UDP::Client_UDP() : Node() {
    this->protocol = "udp";
    this->role = "client";
}

int Client_UDP::send_message(std::string message, std::string IP, int port) {

    if (IP == this->ip && port == this->port) {
        struct sockaddr_in SvrAddr;
        SvrAddr.sin_family = AF_INET;
        SvrAddr.sin_addr.s_addr = inet_addr(IP.c_str());
        SvrAddr.sin_port = htons(port);

        return sendto(this->active_socket, message.c_str(), message.length(), 0,
            (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
    }
    else {
        return -1;
    }
}

struct sockaddr_in Client_UDP::receive_message(std::string& message) {

    struct sockaddr_in SvrAddr;
    int addr_len = sizeof(SvrAddr);

    char RxBuffer[MAX_BUFFER_SIZE] = {};

    int bytes = recvfrom(this->active_socket, RxBuffer, sizeof(RxBuffer), 0,
                    (struct sockaddr*)&SvrAddr, &addr_len);

    message = RxBuffer;
    return SvrAddr;
}