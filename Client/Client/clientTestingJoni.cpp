#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1" // Use IP address instead of "localhost"
#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE] = { 0 };

    // Step 1: Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    // Step 2: Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Step 3: Set up the server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        cerr << "Invalid address/Address not supported.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Step 4: Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection to server failed: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    cout << "Connected to the server.\n";

    // Step 5: Send a message to the server
    const char* message = "Hello, Server!";
    if (send(clientSocket, message, strlen(message), 0) == SOCKET_ERROR) {
        cerr << "Send failed: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    cout << "Message sent to server: " << message << "\n";

    // Step 6: Receive a response from the server
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // Null-terminate the received data
        cout << "Received from server: " << buffer << "\n";
    }
    else if (bytesReceived == 0) {
        cout << "Server closed the connection.\n";
    }
    else {
        cerr << "Receive failed: " << WSAGetLastError() << "\n";
    }

    // Step 7: Clean up and close the socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
