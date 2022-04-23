#include <iostream>
#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Could not start DLLs" << std::endl;
        return 0;
    }


    //create server socket
    SOCKET ServerSocket;
    ServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ServerSocket == INVALID_SOCKET) {
        std::cout << "Could not create socket" << std::endl;
        WSACleanup();
        return 0;
    }

    struct sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_addr.s_addr = INADDR_ANY;
    SvrAddr.sin_port = htons(27020);
    if (bind(ServerSocket, (struct sockaddr*)&SvrAddr,
        sizeof(SvrAddr)) == SOCKET_ERROR) {
        std::cout << "Could not bind socket to port" << std::endl;
        closesocket(ServerSocket);
        WSACleanup();
        return 0;
    }

    char RxBuffer[128] = {};
    do {
        char* begin = RxBuffer;
        char* end = begin + sizeof(RxBuffer);
        std::fill(begin, end, 0);

        struct sockaddr_in CltAddr;
        int addr_len = sizeof(CltAddr);
        std::cout << "Ready to receive messages." << std::endl;
        recvfrom(ServerSocket, RxBuffer, sizeof(RxBuffer), 0,
            (struct sockaddr*)&CltAddr, &addr_len);
        std::cout << "Msg Rx: " << RxBuffer << std::endl;

        if (!strcmp(RxBuffer, "[q]") || !strcmp(RxBuffer, "[x]")) {
            char TxBuffer[128] = { "Goodbye!" };
            sendto(ServerSocket, TxBuffer, sizeof(TxBuffer), 0,
                (struct sockaddr*)&CltAddr, sizeof(CltAddr));
        }
        else {
            char TxBuffer[128] = { "Thanks for your message!" };
            sendto(ServerSocket, TxBuffer, sizeof(TxBuffer), 0,
                (struct sockaddr*)&CltAddr, sizeof(CltAddr));
        }
    } while (strcmp(RxBuffer, "[x]") != 0);

    std::cout << "Terminated based on client's request" << std::endl;

    closesocket(ServerSocket);
    WSACleanup();
    return 0;
}