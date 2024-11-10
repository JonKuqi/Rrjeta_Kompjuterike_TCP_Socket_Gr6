#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server_addr;
    char buffer[1024];
    int bytesReceived;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Define server address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Port should match server's port
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);  // Server IP address

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Connection to server failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    cout << "Connected to server." << endl;


    string command;
    while (true) {
        cout << "Enter command (WRITE, READ, EXECUTE, or QUIT): ";
        getline(cin, command);

        if (command == "QUIT") {
            cout << "Disconnecting from server." << endl;
            break;
        }

        // Send command to server
        if (send(sock, command.c_str(), command.length(), 0) == SOCKET_ERROR) {
            cerr << "Failed to send message. Error Code: " << WSAGetLastError() << endl;
            break;
        }

        // Receive server response
        bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Received from server: " << buffer << endl;
        } else if (bytesReceived == 0) {
            cout << "Connection closed by server." << endl;
            break;
        } else {
            cerr << "Failed to receive message. Error Code: " << WSAGetLastError() << endl;
            break;
        }
    }

    // Clean up
    closesocket(sock);
    WSACleanup();
    return 0;
}
