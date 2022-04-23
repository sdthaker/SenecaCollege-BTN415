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
    //std::cout << "Node class default ctor is called\n";
}

Node::Node(std::string ip, int port) {
    start_dlls();
    num_nodes++;
    this->active_socket = INVALID_SOCKET;
    this->ip = ip;
    this->port = port;
    //std::cout << "Node class overloaded ctor is called with arugments, ip: "
        //<< ip << ", port: " << port << "\n";
}

Node::~Node() {
    closesocket(this->active_socket);
    std::cout << "Closing socket" << std::endl;
    num_nodes--;
    if (num_nodes == 0) {
        WSACleanup();
    }
    //std::cout << "Node class dtor is called\n";
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
    //std::cout << "Node class's start_dlls function is called\n";
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
    //std::cout << "Node class's create_socket function is called\n";
    //return true;
}

void Node::display_info() const {
    std::cout << "IP: " << this->ip << std::endl;
    std::cout << "port: " << this->port << std::endl;
    std::cout << "protocol: " << this->protocol << std::endl;
    std::cout << "dlls: " << dlls_started << std::endl;
    //std::cout << "Node class's display_info function is called\n";
}

Server_TCP::Server_TCP() : Node() {
    this->role = "server";
    this->client_socket = INVALID_SOCKET;
    //std::cout << "Server_TCP class's default ctor is called\n";
}

Server_TCP::Server_TCP(std::string ip, int port) : Node(ip, port) {
    this->protocol = "tcp";
    this->role = "server";
    this->client_socket = INVALID_SOCKET;
    //std::cout << "Server_TCP class's custom ctor is called with arugments, ip: "
        //<< ip << ", port: " << port << "\n";
}

Server_TCP::~Server_TCP() {
    closesocket(this->client_socket);
    std::cout << "Closing client socket" << std::endl;
    //std::cout << "Server_TCP class dtor is called\n";
}

bool Server_TCP::bind_socket() {
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
    /*std::cout << "Server_TCP class's bind_socket function is called\n";
    return true;*/
}

bool Server_TCP::listen_for_connections() {
    if (listen(this->active_socket, 1) == SOCKET_ERROR) {
        std::cout << "Could not start to listen" << std::endl;
        return false;
    }
    else {
        return true;
    }
    //std::cout << "Server_TCP class's listen_for_connections function is called\n";
    //return true;
}

bool Server_TCP::accept_connection() {
    this->client_socket = accept(this->active_socket, NULL, NULL);
    if (this->client_socket == INVALID_SOCKET) {
        std::cout << "Failed to accept connection" << std::endl;
        return false;
    }
    else {
        return true;
    }
    //std::cout << "Server_TCP class's accept_connection function is called\n";
    //return true;

}

int Server_TCP::send_message(std::string message) {
    return send(this->client_socket, message.c_str(), message.length(), 0);
    /* std::cout << "Server_TCP class's send_message function is called with arguments, message: " << message << "\n";
    return 0;*/
}

int Server_TCP::receive_message(std::string& message) {
    char RxBuffer[MAX_BUFFER_SIZE] = {};
    memset(RxBuffer, 0, MAX_BUFFER_SIZE);
    int num_bytes = recv(this->client_socket, RxBuffer, MAX_BUFFER_SIZE, 0);
    message = RxBuffer;
    return num_bytes;
    /*std::cout << "Server_TCP class's receive_message function is called with arguments, message: " << message << "\n";
    return 0;*/
}

void Server_TCP::close_connection() {
    closesocket(this->client_socket);
    //std::cout << "Server_TCP class's close_connection function is called\n";
}

Client_TCP::Client_TCP() : Node() {
    this->protocol = "tcp";
    this->role = "client";
    //std::cout << "Client_TCP class's default ctor is called\n";
}

bool Client_TCP::connect_socket(std::string ip, int port) {
    struct sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(port);
    SvrAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    if ((connect(this->active_socket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
        std::cout << "Failed to connect to server" << std::endl;
        return false;
    }
    else {
        return true;
    }
    //std::cout << "Client_TCP class's custom ctor is called with arugments, ip: " << ip
    //    << ", port: " << port << "\n";
    //return true;
}

int Client_TCP::send_message(std::string message) {
    return send(this->active_socket, message.c_str(), message.length(), 0);
    /*std::cout << "Client_TCP class's send_message function is called with arguments, message: " << message << "\n";
    return 0;*/
}

int Client_TCP::receive_message(std::string& message) {
    char RxBuffer[MAX_BUFFER_SIZE] = {};
    memset(RxBuffer, 0, MAX_BUFFER_SIZE);
    int num_bytes = recv(this->active_socket, RxBuffer, MAX_BUFFER_SIZE, 0);
    message = RxBuffer;
    return num_bytes;
    /*std::cout << "Client_TCP class's receive_message function is called with arguments, message: " << message << "\n";
    return 0;*/
}