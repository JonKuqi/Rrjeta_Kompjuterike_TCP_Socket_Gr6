#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main2() {
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
    server_addr.sin_port = htons(8080);  // Server port
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);  // Server IP address

    // Lidhja me serverin
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Connection to server failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    cout << "Connected to server." << endl;


    const char* message = "Hello from client!";
    if (send(sock, message, strlen(message), 0) == SOCKET_ERROR) {
        cerr << "Failed to send message. Error Code: " << WSAGetLastError() << endl;
    }
    else {
        cout << "Message sent to server." << endl;
    }

    // Lexo përgjigjen nga serveri
    char buffer[1024];
    int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        cout << "Received from server: " << buffer << endl;
    }
    else if (bytesReceived == 0) {
        cout << "Connection closed by server." << endl;
    }
    else {
        cerr << "Failed to receive message. Error Code: " << WSAGetLastError() << endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
