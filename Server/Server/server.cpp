#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define SERVER_NAME = ''
#define PORT 8080

#define BUFFER_SIZE 1024

#define MAX_CONNECTED 3


int activeClientCounter = 0;

queue<SOCKET> waitingClients;


mutex clientMutex;

condition_variable clientCondition;


void handleClient(SOCKET clientSocket) {
    const char* welcomeMessage = "Welcome ... \n";

    send(clientSocket, welcomeMessage, strlen(welcomeMessage), 0);



    //Nga klienti
    char buffer[BUFFER_SIZE] = { 0 }; // E mushim me zero si fillim

    while (true) {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Received from client: " << buffer << endl;



            send(clientSocket, buffer, bytesReceived, 0);
        }
        else {
            break;
        }

    }

    closesocket(clientSocket);

    {
        lock_guard<mutex> lock(clientMutex);  //lock_guard -> klase e mutexave
        activeClientCounter--;

    }
    //E njofton kur te lirohet ni slot
    clientCondition.notify_one();

}


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
    if (listen(serverSocket, MAX_CONNECTED) == SOCKET_ERROR) {
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




        unique_lock<mutex> lock(clientMutex); // Locku per client


        if (activeClientCounter < MAX_CONNECTED) {

            activeClientCounter++;

            cout << "New client processing. Active clients" << activeClientCounter<<endl;


            //E ban handle funksioni nalt
            thread clientThread (handleClient, clientSocket);
            clientThread.detach();

        }
        else {
            //E mushne queue

            cout << "Max client number reached, adding to the queue. \n";
            waitingClients.push(clientSocket);
            lock.unlock();
            
        }

        // bane lock nese activeVlientCounter < MAX_CONNECTED
        clientCondition.wait(lock, [] { return activeClientCounter < MAX_CONNECTED; });

        //Kqyr nese ka ne queue
        if (!waitingClients.empty()) {
            SOCKET clientInQueue = waitingClients.front();
            waitingClients.pop();
            activeClientCounter++;

            cout << "Connecting queued client. Total clients: " << activeClientCounter << "\n";

           
            thread clientThread(handleClient, clientInQueue);
            clientThread.detach();
        }


    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}