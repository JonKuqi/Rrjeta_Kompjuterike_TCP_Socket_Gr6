#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define SERVER_NAME = ''
#define PORT 8080

#define BUFFER_SIZE 1024


int main() {


    //Inicializimi i WinStock

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }



    //Inicializimi i Socket

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed! \n";
        WSACleanup();
        return 1;
    }



    //Bind i Socket
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    int successfulBind = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if (successfulBind == SOCKET_ERROR) {
        cerr << "Bind Failed \n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }


    


    //Listen serveri
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Server is listening on port " << PORT << "...\n";

    struct sockaddr_in clientAddress;
    SOCKET clientSocket;
    int clientAddressSize = sizeof(clientAddress);
    char buffer[BUFFER_SIZE] = { 0 }; //Inicializim me zero si fillim


    while (true) {

        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

        if (clientSocket == INVALID_SOCKET) {
            continue; //Merri tjert
        }

        cout << "Client connected.\n";

        const char* welcome = "Welcome to the server! /\n";

        send(clientSocket, welcome, strlen(welcome), 0);

        //Me ba handle qetu duhet


        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Mi hjek zerot me lan veq mesazhin
            cout << "Received from client: " << buffer << endl;
        }
        else if (bytesReceived == 0) {
            cout << "Client disconnected.\n";
        }
       

        closesocket(clientSocket);
        

    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}