#include <iostream>
#include <windows.networking.sockets.h>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
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
    SvrAddr.sin_port = htons(27010);
    SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
        std::cout << "Failed to connect to server" << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 0;
    }

    std::string TxBuffer = {};

    while (TxBuffer != "[q]") {

        std::cout << "Enter a String to transmit or \"[q]\" to quit" << std::endl;
        std::getline(std::cin, TxBuffer);

        if (TxBuffer.size() > 128) {
            while (TxBuffer.size() > 128) {
                std::cout << "Max chars allowed are 128. Enter the string again: ";
                std::getline(std::cin, TxBuffer);
            }
        }

        send(ClientSocket, TxBuffer.c_str(), TxBuffer.size(), 0);

        char RxBuffer[128] = {};
        if (recv(ClientSocket, RxBuffer, sizeof(RxBuffer), 0) == SOCKET_ERROR) {
            std::cout << "Connection Timed Out!\n";
            break;
        }
        if (TxBuffer == "[q]") {
            std::cout << "The server will now be shut down\n";
        }
        std::cout << "Response: " << RxBuffer << std::endl;
    }

    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}