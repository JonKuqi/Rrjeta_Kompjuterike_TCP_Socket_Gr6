#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <ws2tcpip.h>
#include <vector>
#include <fstream>
#include <direct.h>
#include <thread>
#include <string>


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

enum Privileges { FULL_ACCESS, READ_ONLY };

// Struktura per te mbajtur informacionin e klientev dhe privilegjet
struct ClientInfo {
    SOCKET sock;
    Privileges privilege;
};

// Lista e klienteve qe jane lidh
vector<ClientInfo> clients;

// Funksioni qe e trajton secilin klient sipas privilegjeve
void handleClient(ClientInfo client) {
    char buffer[1024];
    int bytesReceived;

    while (true) {
        // Prano kërkesën e klientit
        bytesReceived = recv(client.sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            cout << "Client disconnected." << endl;
            closesocket(client.sock);
            break;
        }

        buffer[bytesReceived] = '\0';
        string request(buffer);

//Kontrollo privelegjet e klientit
if (client.privilege == FULL_ACCESS) {
            if (request.rfind("WRITE ", 0) == 0) { // Komanda WRITE
                ofstream file("server_data.txt", ios::app);
                if (file.is_open()) {
                    file << request.substr(6) << endl;
                    file.close();
                    send(client.sock, "Data written successfully.\n", 26, 0);
                }
                else {
                    send(client.sock, "Failed to write data.\n", 22, 0);
                }
            }


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
