#include <iostream>
#include <cstring>
#include <string>
#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Could not start DLLs" << std::endl;
		return 0;
	}

	SOCKET ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "Could not create socket" << std::endl;
		WSACleanup();
		return 0;
	}

	struct sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(27010);
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	std::string TxBuffer = {};
	do {
		TxBuffer = {};

		std::cout << "Enter a string to transmit:" << std::endl;
		std::getline(std::cin, TxBuffer);
		sendto(ClientSocket, TxBuffer.c_str(), sizeof(TxBuffer), 0,
			(struct sockaddr*)&SvrAddr, sizeof(SvrAddr));

		char RxBuffer[128] = {};
		int addr_len = sizeof(SvrAddr);
		recvfrom(ClientSocket, RxBuffer, sizeof(RxBuffer), 0,
			(struct sockaddr*)&SvrAddr, &addr_len);
		std::cout << "Msg Rx: " << RxBuffer << std::endl;
	} while (TxBuffer != "[q]" && TxBuffer != "[x]");

	closesocket(ClientSocket);
	WSACleanup();
}