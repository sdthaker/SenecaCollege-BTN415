#include "socket.h"
#include <string>
#include <iostream>
#include <windows.networking.sockets.h>
#include <stdlib.h>
#include <thread>
#pragma comment(lib, "Ws2_32.lib")

bool Node::dlls_started = false;
int Node::num_nodes = 0;

int const MAX_SOCKETS = 20;
SOCKET Aux_Socket;
SOCKET ClientSockets[MAX_SOCKETS + 1] = { SOCKET_ERROR };
bool Active_Sockets[MAX_SOCKETS + 1] = { false };

int find_available_socket(void) {
	int socket_number = MAX_SOCKETS;
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (!Active_Sockets[i]) {
			socket_number = i;
			break;
		}
	}
	return socket_number;
}

void Run(int Index) {
	std::cout << "Thread Started for Index " << Index << std::endl;
	Active_Sockets[Index] = true;
	while (true) {

		packet my_packet;

		char RxBuffer[MAX_BUFFER_SIZE] = {};
		memset(RxBuffer, 0, MAX_BUFFER_SIZE);
		int num_bytes = recv(ClientSockets[Index], RxBuffer, MAX_BUFFER_SIZE, 0);
		my_packet = packet_deserializer(RxBuffer);

		std::string RxPacket;

		for (size_t i = 0; i < my_packet.size; i++) {
			RxPacket += my_packet.letters[i];
		}

		if (std::string(RxPacket) == "[q]" || num_bytes <= 0)
			break;

		//	std::cout << "Packet name: " << my_packet.name << std::endl;
		//	std::cout << "Packet size: " << my_packet.size << std::endl;

		std::cout << "From Thread " << Index << " : ";
		std::cout << RxPacket << std::endl;

		struct serialized_packet serialized_data = packet_serializer(my_packet);

		for (size_t i = 0; i < MAX_SOCKETS; i++) {
			if (Active_Sockets[i] && Index != i) {
				send(ClientSockets[i], serialized_data.data, serialized_data.length, 0);
			}
		}
	}

	std::cout << "Closing Connection" << std::endl;
	closesocket(ClientSockets[Index]);
	Active_Sockets[Index] = false;
}

packet create_packet() {
	packet my_packet;
	std::cout << "Enter a packet name: ";
	std::cin >> my_packet.name;
	std::cout << "What is the size of the packet? ";
	std::cin >> my_packet.size;

	if (my_packet.size > 10) {
		my_packet.flag = true;
	}
	else {
		my_packet.flag = false;
	}

	my_packet.letters = new char[my_packet.size + 1];
	for (int i = 0; i < my_packet.size; i++) {
		std::cout << "Enter element " << i << ": ";
		std::cin >> my_packet.letters[i];
	}

	++my_packet.size;
	my_packet.letters[my_packet.size - 1] = '\0';

	return my_packet;
}

serialized_packet packet_serializer(packet my_packet) {
	//beware of Windows completing groups of four bytes
	char* my_serialized_packet = new char[16 * sizeof(char) + 1 * sizeof(bool) +
		1 * sizeof(int) + my_packet.size * sizeof(char)];
	char* auxptr = my_serialized_packet;

	memcpy(auxptr, my_packet.name, 16 * sizeof(char));
	auxptr += 16 * sizeof(char);
	memcpy(auxptr, &my_packet.flag, 1 * sizeof(bool));
	auxptr += 1 * sizeof(bool);
	memcpy(auxptr, &my_packet.size, 1 * sizeof(int));
	auxptr += 1 * sizeof(int);
	memcpy(auxptr, my_packet.letters, my_packet.size * sizeof(char));

	serialized_packet output;
	output.data = my_serialized_packet;
	output.length = 16 * sizeof(char) + 1 * sizeof(bool) + 1 * sizeof(int) + my_packet.size * sizeof(char);
	return output;
}

packet packet_deserializer(char* serialized_packet) {
	packet deserialized_packet;
	char* auxptr = serialized_packet;

	memcpy(deserialized_packet.name, auxptr, 16 * sizeof(char));
	auxptr += 16 * sizeof(char);
	memcpy(&deserialized_packet.flag, auxptr, sizeof(bool));
	auxptr += 1 * sizeof(bool);
	memcpy(&deserialized_packet.size, auxptr, sizeof(int));
	auxptr += 1 * sizeof(int);
	deserialized_packet.letters = new char[deserialized_packet.size];
	memcpy(deserialized_packet.letters, auxptr, sizeof(char) * deserialized_packet.size);

	return deserialized_packet;
}

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

Server_TCP::Server_TCP() : Node() {
	this->role = "server";
	this->client_socket = INVALID_SOCKET;
}

Server_TCP::Server_TCP(std::string ip, int port) : Node(ip, port) {
	this->protocol = "tcp";
	this->role = "server";
	this->client_socket = INVALID_SOCKET;
}

Server_TCP::~Server_TCP() {
	closesocket(this->client_socket);
	std::cout << "Closing client socket" << std::endl;
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
}

bool Server_TCP::listen_for_connections() {
	if (listen(this->active_socket, 1) == SOCKET_ERROR) {
		std::cout << "Could not start to listen" << std::endl;
		return false;
	}
	else {
		return true;
	}
}

bool Server_TCP::accept_connection() {

	int Socket_Number;

	while (true) {
		std::cout << "Ready to accept a connection" << std::endl;
		Aux_Socket = accept(this->active_socket, NULL, NULL);
		if (Aux_Socket == SOCKET_ERROR) {
			return 0;
		}
		else {
			Socket_Number = find_available_socket();
			if (Socket_Number < MAX_SOCKETS) {
				ClientSockets[Socket_Number] = Aux_Socket;
				send(ClientSockets[Socket_Number], "Welcome from server", sizeof("Welcome from server"), 0);
				std::thread(Run, Socket_Number).detach();
			}
			else {
				send(ClientSockets[MAX_SOCKETS], "Full",
					sizeof("Full"), 0);
				std::cout << "Connection Fail" << std::endl;
			}
		}
	}
}

void Server_TCP::close_connection() {
	closesocket(this->client_socket);
}

Client_TCP::Client_TCP() : Node() {
	this->protocol = "tcp";
	this->role = "client";
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
}

void sendMsg(SOCKET ClientSocket, packet TxPacket) {
	while (true) {

		TxPacket = create_packet();
		struct serialized_packet serialized_data = packet_serializer(TxPacket);

		packet my_packet = packet_deserializer(serialized_data.data);

		send(ClientSocket, serialized_data.data, serialized_data.length, 0);
		if (!strcmp(my_packet.letters, "[q]"))
			break;
	}
}

void receiveMsg(char* RxBuffer, SOCKET ClientSocket) {
	while (true) {

		packet my_packet;

		char RxBuffer[MAX_BUFFER_SIZE] = {};
		memset(RxBuffer, 0, MAX_BUFFER_SIZE);
		int num_bytes = recv(ClientSocket, RxBuffer, MAX_BUFFER_SIZE, 0);
		my_packet = packet_deserializer(RxBuffer);

		std::string RxPacket;

		for (size_t i = 0; i < my_packet.size; i++) {
			RxPacket += my_packet.letters[i];
		}

		std::cout << "\nRx: " << RxPacket << std::endl;
	}
}

void Client_TCP::send_data() {
	char RxBuffer[128] = {};
	packet my_packet;

	recv(this->active_socket, RxBuffer, sizeof(RxBuffer), 0);
	if (!strcmp(RxBuffer, "Full")) {
		std::cout << "Server full" << std::endl;
		//return 0;
	}
	else {
		std::thread(receiveMsg, RxBuffer, this->active_socket).detach();
		std::thread tx(sendMsg, this->active_socket, my_packet);
		tx.join();
	}
}