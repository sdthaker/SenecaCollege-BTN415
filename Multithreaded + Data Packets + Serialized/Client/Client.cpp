#include <iostream>
#include <string>
#include <windows.networking.sockets.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER_SIZE 512

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


void sendMsg(SOCKET ClientSocket, packet TxPacket) {
	while (true) {

		TxPacket = create_packet();
		struct serialized_packet serialized_data = packet_serializer(TxPacket);

		packet my_packet = packet_deserializer(serialized_data.data);

		send(ClientSocket, serialized_data.data, serialized_data.length, 0);
		if (!strcmp(my_packet.letters,"[q]"))
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

		std::cout << "\nRx: " << RxPacket<< std::endl;
	}
}

int main()
{
	packet my_packet;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Could not start DLLs" << std::endl;
		return 0;
	}

	SOCKET ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "Could not create socket" << std::endl;
		WSACleanup();
		return 0;
	}

	struct sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(27000);
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//SvrAddr.sin_addr.s_addr = inet_addr("159.203.26.94");
	if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr,
		sizeof(SvrAddr))) == SOCKET_ERROR) {
		std::cout << "Failed to connect to server" << std::endl;
		closesocket(ClientSocket);
		WSACleanup();
		return 0;
	}

	char RxBuffer[128] = {};
	recv(ClientSocket, RxBuffer, sizeof(RxBuffer), 0);
	if (!strcmp(RxBuffer, "Full")) {
		std::cout << "Server full" << std::endl;
		return 0;
	}
	else {
		std::thread(receiveMsg, RxBuffer, ClientSocket).detach();
		std::thread tx(sendMsg, ClientSocket, my_packet);
		tx.join();
	}

	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}