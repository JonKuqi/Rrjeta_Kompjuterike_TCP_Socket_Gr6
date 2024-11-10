#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define SERVER_IP "127.0.0.1" // Server IP address
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }


    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed!\n";
        WSACleanup();
        return 1;
    }

    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

 

    if (inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr) <= 0) {
        cerr << "Invalid IP address format: " << SERVER_IP << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cerr << "Connection to server failed! Error: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to the server.\n";

    string userInput;
    char buffer[BUFFER_SIZE];

    // Main loop for sending and receiving messages
    cout << "Enter message (type 'exit' to disconnect): ";

    while (true) {

       
        
        getline(cin, userInput);

        if (userInput == "exit") {
            cout << "Disconnecting from the server...\n";
            break;
        }

        // Send the message to the server
        int sendResult = send(clientSocket, userInput.c_str(), userInput.length(), 0);
        if (sendResult == SOCKET_ERROR) {
            cerr << "Send failed: " << WSAGetLastError() << "\n";
        }
        else {
            // Add a small delay after sending the command to the server
            Sleep(50); // Delay of 50 milliseconds

            memset(buffer, 0, BUFFER_SIZE); // Clear the buffer before receiving
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0'; // Null-terminate the received data
                cout << "Server: " << buffer << endl;
            }
            else if (bytesReceived == 0) {
                cout << "Server disconnected.\n";
                break;
            }
            else {
                cerr << "Receive failed: " << WSAGetLastError() << "\n";
                break;
            }
        }

       
    }


    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
