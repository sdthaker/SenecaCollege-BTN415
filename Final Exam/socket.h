#pragma once
#include <string>
#include <iostream>
#include <windows.networking.sockets.h>

#define MAX_BUFFER_SIZE 512

struct data_packet {
    int seq;
    unsigned char ack : 1;
    unsigned char syn : 1;
    unsigned char flag : 1;
    unsigned char rst : 1;
    unsigned char : 4; //padding
    unsigned int length;
    char* data;
    unsigned char crc;
};

struct serialized_packet {
    char* data;
    int length;
};

data_packet create_packet();
void display(unsigned char byte);
unsigned char calculate_crc(data_packet my_packet);
serialized_packet data_packet_serializer(data_packet new_packet);
data_packet data_packet_deserializer(char* serialized_packet);

class Node {
public:
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

class Server_TCP : public Node {
private:
    SOCKET client_socket;
public:
    Server_TCP();
    Server_TCP(std::string ip, int port);
    ~Server_TCP();
    bool bind_socket();
    bool listen_for_connections();
    bool accept_connection();
    int send_message(std::string message);
    int send_data_packet(data_packet message);
    int receive_message(std::string& message);
    int receive_data_packet(data_packet& my_packet);
    void close_connection();
};

class Client_TCP : public Node {
public:
    Client_TCP();
    bool connect_socket(std::string ip, int port);
    int send_message(std::string message);
    int send_data_packet(data_packet message);
    int receive_data_packet(data_packet& my_packet);
    int receive_message(std::string& message);
};