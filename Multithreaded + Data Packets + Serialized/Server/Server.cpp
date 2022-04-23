#include <iostream>
#include <windows.networking.sockets.h>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER_SIZE 512

int const MAX_SOCKETS = 20;
SOCKET Aux_Socket;
SOCKET ClientSockets[MAX_SOCKETS + 1] = { SOCKET_ERROR };
bool Active_Sockets[MAX_SOCKETS + 1] = { false };

struct packet {
	char name[16];
	bool flag;
	int size;
	char* letters;
};

struct serialized_packet {
	char* data;
	int length;
};

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

		for (size_t i = 0; i < my_packet.size; i++){
			RxPacket += my_packet.letters[i];
		}

		if (std::string(RxPacket) == "[q]" || num_bytes <= 0)
			break;
		
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

int main(int argc, char* argv[]) {

	int Socket_Number;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Could not start DLLs" << std::endl;
		return 0;
	}

	SOCKET ListenSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "Could not create socket" << std::endl;
		WSACleanup();
		return 0;
	}

	struct sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	SvrAddr.sin_port = htons(27000);
	if (bind(ListenSocket, (struct sockaddr*)&SvrAddr,
		sizeof(SvrAddr)) == SOCKET_ERROR) {
		std::cout << "Could not bind socket to port" << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	if (listen(ListenSocket, 1) == SOCKET_ERROR) {
		std::cout << "Could not start to listen" << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	while (true) {
		std::cout << "Ready to accept a connection" << std::endl;
		Aux_Socket = accept(ListenSocket, NULL, NULL);
		if (Aux_Socket == SOCKET_ERROR) {
			return 0;
		}
		else {
			Socket_Number = find_available_socket();
			if (Socket_Number < MAX_SOCKETS) {
				ClientSockets[Socket_Number] = Aux_Socket;
				send(ClientSockets[Socket_Number], "Welcome", sizeof("Welcome"), 0);
				std::thread(Run, Socket_Number).detach();
			}
			else {
				send(ClientSockets[MAX_SOCKETS], "Full",
					sizeof("Full"), 0);
				std::cout << "Connection Fail" << std::endl;
			}
		}
	}
	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}