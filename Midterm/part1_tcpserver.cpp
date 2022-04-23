#include <iostream>
#include <cstring>
#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {

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
	SvrAddr.sin_port = htons(27010);
	if (bind(ListenSocket, (struct sockaddr*)&SvrAddr,
		sizeof(SvrAddr)) == SOCKET_ERROR) {
		std::cout << "Could not bind socket to address" << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	SOCKET UDPSocket;
	UDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (UDPSocket == INVALID_SOCKET) {
		std::cout << "Could not create UDP socket" << std::endl;
		WSACleanup();
		return 0;
	}

	struct sockaddr_in Svr;
	Svr.sin_family = AF_INET;
	Svr.sin_addr.s_addr = inet_addr("127.0.0.1");
	Svr.sin_port = htons(27010);
	if (bind(UDPSocket, (struct sockaddr*)&Svr,
		sizeof(Svr)) == SOCKET_ERROR) {
		std::cout << "Could not bind UDP socket to address" << std::endl;
		closesocket(UDPSocket);
		WSACleanup();
		return 0;
	}

	if (listen(ListenSocket, 1) == SOCKET_ERROR) {
		std::cout << "Could not start to listen" << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	SOCKET ClientSocket;
	while (true) {
		std::cout << "Waiting for client connection" << std::endl;
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "Failed to accept connection" << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}

		std::cout << "Connection Established" << std::endl;

		char RxBuffer[128] = {};

		do {
			char TxBuffer[128] = {};
			char* begin = RxBuffer;
			char* end = begin + sizeof(RxBuffer);
			std::fill(begin, end, 0);

			recv(ClientSocket, RxBuffer, sizeof(RxBuffer), 0);

			if (!strcmp(RxBuffer, "[q]")) {
				std::cout << "Client purposely wants to end this connection!\n\n";
				strcpy(TxBuffer, "As per your request this is a final message from server! Take Care!");
			}
			else {
				std::cout << "Msg Rx: " << RxBuffer << std::endl;
				strcpy(TxBuffer, "Thank you for your message!");
			}

			send(ClientSocket, TxBuffer, sizeof(TxBuffer), 0);
			send(UDPSocket, RxBuffer, sizeof(RxBuffer), 0);

		} while (strcmp(RxBuffer, "[q]"));
	}

	closesocket(ListenSocket);

	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}