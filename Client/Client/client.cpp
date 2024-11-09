#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server_addr;

    // Inicializo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Winsock initialized." << endl;

    // Krijo socket-in
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    cout << "Socket created successfully." << endl;

    // Percakto IP dhe portin e serverit
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Porti i serverit
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // IP-ja e serverit
