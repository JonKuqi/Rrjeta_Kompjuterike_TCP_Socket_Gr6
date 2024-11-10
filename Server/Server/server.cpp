#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <ctime>
#include <deque>
#include <queue>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <windows.h>
#include <sstream>
#include <stdio.h>


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define SERVER_NAME ""

#define PORT 8080

#define BUFFER_SIZE 1024

#define MAX_CONNECTED 3

#define TIMEOUT_SECONDS 300  //Ne sekonda


int activeClientCounter = 0;
int clientsInQueue = 0;


struct ClientData {
    SOCKET clientSocket;
    sockaddr_in clientAddress;
    ClientData(SOCKET socket, sockaddr_in address) : clientSocket(socket), clientAddress(address) {}
};

deque<ClientData> waitingClients;


mutex clientMutex;

condition_variable clientCondition;


string fullAccessClientIp = "192.168.3.21";




struct ClientInfo {
    sockaddr_in addr;
    bool wasQueued;
};

queue<ClientInfo> reconnectQueue;
mutex reconnectMutex;


volatile bool serverShutdownRequested = false;


string ListFilesInDirectory(const string& folderPath) {
    stringstream output;

    // Construct the search pattern
    string searchPath = folderPath + "\\*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return "Error: Unable to open directory " + folderPath + "\n";
    }

    output << "Listing files in: " << folderPath << "\n";

    do {
        const char* name = findData.cFileName;

        // Skip "." and ".." entries
        if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
            // Check if it's a directory
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                output << "[Folder] " << name << endl;
            }
            else {
                output << "[File]   " << name << endl;
            }
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);

    return output.str();
}


// si explode() ne php
std::vector<std::string> splitTheString(const std::string& str) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string word;
    while (ss >> word) {
        result.push_back(word);
    }
    return result;
}






void log(const string& message) {

    lock_guard<mutex> lock(clientMutex);
    ofstream logFile("server_log.log", ios::app);
    if (logFile.is_open()) {
        logFile << message << endl;
        logFile.close();
    }
    else {
        cerr << "Unable to open log file." << endl;
    }
}



string getCurrentTime() {
    time_t now = time(0);
    char buf[80];
    struct tm timeInfo;
    localtime_s(&timeInfo, &now); 
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &timeInfo);
    return buf;
}


void setSocketTimeout(SOCKET socket) {
    int timeout = TIMEOUT_SECONDS * 1000;  // Convert to milliseconds
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
}


string readFile(const string& path) {
    ifstream file(path);

    if (!file.is_open()) {
  
        return "Not Found"; 
    }

    stringstream buffer;
    buffer << file.rdbuf();

    file.close(); 
    return buffer.str(); 
}



bool writeToFile(string path, string text, bool append) {
    if (append) {
        ofstream outfile(path, ios::app);
        if (outfile.is_open()) {
            outfile << text;
            outfile.close();
            return true;
        }
        else {
            return false;
        }
    }
    else {
        ofstream outfile(path);
        if (outfile.is_open()) {
            outfile << text;
            outfile.close();
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}


void handleClient(SOCKET clientSocket, sockaddr_in clientAddr, bool wasQueued) {

   
    //Clienti ne timeOut
    setSocketTimeout(clientSocket);

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

    int clientPort = ntohs(clientAddr.sin_port);



    //Kur u log u ruje
    log("Client connected: IP = " + string(clientIP) + ", Port = " + to_string(clientPort) + ", Time = " + getCurrentTime());


    //Nga klienti
    char buffer[BUFFER_SIZE] = { 0 }; // E mushim me zero si fillim

    
    bool isFullAccess = (string(clientIP) == fullAccessClientIp);
    
   
    //PATH
    string path = "ClientFiles/";


    string direct = "\n ______________________\n";
    direct += ListFilesInDirectory(path);
    direct += "\n ______________________\n";

    const char* message;
    string welcome;

    if (isFullAccess) {
        if(wasQueued){
            welcome = "You are now connected, sorry for the wait!\nYou have Full Access! \n\n Commands:  read filename | write filename text | append filename text\n up  (goes up in path folder) | down (goes down in path folder) | execute Stop Server\n\n You are in this directory: \n" + direct;
        }
        else {
            welcome = "You have Full Access! \n\n Commands:  read filename | write filename text | append filename text\n up  (goes up in path folder) | down (goes down in path folder) | execute (Stop Server)\n\n You are in this directory: \n" + direct;
        }
        

    }
    else {
        if (wasQueued) {
            welcome = "You are now connected, sorry for the wait!\You have limited Access! \n\n Commands:  read filename | up  (goes up in path folder) | down folder(goes down in path folder) | execute StopServer\n\n You are in this directory: \n" + direct;
        }
        else {
            welcome = "You have limited Access! \n\n Commands:  read filename | up  (goes up in path folder) | down folder(goes down in path folder)\n\n You are in this directory: \n" + direct;
        }
    }
    message = welcome.c_str();
    send(clientSocket, message, strlen(message), 0);
  
    
    while (true) {

        //Fshije n buffer qka ka
        memset(buffer, 0, BUFFER_SIZE);
        
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        //Wait 50 milisekonda
        //this_thread::sleep_for(chrono::milliseconds(50));

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Received from client: " << buffer << endl;

            //Log mesazhet e klientit
            log("Message from IP " + string(clientIP) + " at " + getCurrentTime() + ": " + buffer);

            string recievedString(buffer);


            vector<string> words = splitTheString(recievedString);



            string response = "";

            if (words[0] == "read") {
                string fromFile = readFile(path + words[1]);
                if (fromFile == "Not Found") {
                    response = "Incorrect path.";
                }
                else {
                    response = fromFile;  // Use std::string
                }
            }
            else if (words[0] == "write" && isFullAccess) {
                string toWrite = "";
                for (int i = 2; i < words.size(); i++ ) {
                    toWrite += words[i] += " ";
                }
                if (writeToFile(path + words[1], toWrite, false)) {
                    response = "Write Successful! \n";
                }
                else {
                    response = "Write NOT Successful! \n";
                }
            }
            else if (words[0] == "append" && isFullAccess) {
                string toWrite = "";
                for (int i = 2; i < words.size(); i++) {
                    toWrite += words[i] += " ";
                }
                if (writeToFile(path + words[1], toWrite, true)) {
                    response = "Write Successful! \n";
                }
                else {
                    response = "Write NOT Successful! \n";
                }
            }
            else if (words[0] == "up") {
                path = "ClientFiles/";
                string direct = ListFilesInDirectory(path);
                direct += "\n ______________________\n";
                send(clientSocket, direct.c_str(), direct.length(), 0);
            }
            else if (words[0] == "down") {
                if (path == "ClientFiles/") {
                    path += words[1] + "/";
            }
                string direct = ListFilesInDirectory(path);
                direct += "\n ______________________\n";
                send(clientSocket, direct.c_str(), direct.length(), 0);
            }
            else if (words[0] == "execute" && isFullAccess && words[1] == "StopServer") {
                response = "Server will shut down [Press any key to confirm]";
                send(clientSocket, response.c_str(), response.length(), 0);
                log("Server shutdown command received from IP " + string(clientIP));
                serverShutdownRequested = true; 
                closesocket(clientSocket);  
                cout << "Shutting down server as requested by a client.\n";
                break;  
            }
            else {
                response = "Invalid Command. Try again.";
            }
            if (!response.empty()) {
                send(clientSocket, response.c_str(), response.length(), 0);
            }
        }
        else if (bytesReceived == 0 || WSAGetLastError() == WSAETIMEDOUT) {
            // Client disconnected ose timed out
            log("Client timed out: IP = " + string(clientIP) + ", Port = " + to_string(clientPort));
            cout << "Client timed out: IP = " + string(clientIP) + ", Port = " + to_string(clientPort) << endl;
            activeClientCounter--;
            {
                lock_guard<mutex> lock(reconnectMutex);
                reconnectQueue.push({ clientAddr, wasQueued });
            }

            closesocket(clientSocket);

            break;
        }
        else {
            cerr << "Receive error for client IP = " << clientIP << ", Port = " << clientPort << endl;
            log("Client disconnected: IP = " + string(clientIP) + ", Port = " + to_string(clientPort) + ", Time = " + getCurrentTime());
            closesocket(clientSocket);

            lock_guard<mutex> lock(clientMutex);  //lock_guard -> klase e mutexave
            activeClientCounter--;

            //E njofton kur te lirohet ni slot
            clientCondition.notify_one();
            break;
        }

    }

    // Log  kur disconnect
    log("Client disconnected: IP = " + string(clientIP) + ", Port = " + to_string(clientPort) + ", Time = " + getCurrentTime());

      closesocket(clientSocket);

    
    lock_guard<mutex> lock(clientMutex);  //lock_guard -> klase e mutexave
    activeClientCounter--;
    
    //E njofton kur te lirohet ni slot
    clientCondition.notify_one();

}











void manageReconnections() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(5));  // Adjust the frequency of checks

        lock_guard<mutex> lock(clientMutex);
        if (activeClientCounter < MAX_CONNECTED) {
            lock_guard<mutex> reconnectLock(reconnectMutex);
            if (!reconnectQueue.empty()) {
                auto clientInfo = reconnectQueue.front();
                reconnectQueue.pop();

                // Try to accept a new connection from the same IP and port
                SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
                if (clientSocket != INVALID_SOCKET && connect(clientSocket, (struct sockaddr*)&clientInfo.addr, sizeof(clientInfo.addr)) == 0) {
                    activeClientCounter++;
                    thread clientThread(handleClient, clientSocket, clientInfo.addr, true);
                    clientThread.detach();
                }
                else {
                    closesocket(clientSocket);
                }
            }
        }
    }
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

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cerr << "Bind failed!\n";
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





    //Thread qe menagjon diconneccted
    thread reconnectionThread(manageReconnections);
    reconnectionThread.detach();
 


    while (true) {

        //Koha qe me ba timeout accepti
        fd_set readfds;
        FD_ZERO(&readfds);  
        FD_SET(serverSocket, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = 0.5;  
        timeout.tv_usec = 0;          

       
        int result = select(0, &readfds, NULL, NULL, &timeout);


        if (result == SOCKET_ERROR) {
            cout << "Select failed with error: " << WSAGetLastError() << endl;
            continue;  
        }

        if (result == 0) {
            // Timeout occurred - Ska connection te reja
            //cout << "No client connected in the timeout period.\n";

            if (!waitingClients.empty() && activeClientCounter < MAX_CONNECTED - 1) {
                SOCKET queuedClient = waitingClients.front().clientSocket;
                
                clientsInQueue--;
                struct sockaddr_in activeQueuedAddress =  waitingClients.front().clientAddress;

                waitingClients.pop_front();
                activeClientCounter+=2;
                int activeQueuedAddressSize = sizeof(activeQueuedAddress);
                

                cout << "Connecting queued client. Active clients: " << activeClientCounter << "\n";

                // Handle queued client
                thread clientThread(handleClient, queuedClient, activeQueuedAddress, true);
                clientThread.detach();
            }

            continue;  
        }


        if (serverShutdownRequested) {
            cout << "Shutting down server as requested by a client.\n";
            break;
        }


        if (FD_ISSET(serverSocket, &readfds)) {
            struct sockaddr_in clientAddress;
            int clientAddressSize = sizeof(clientAddress);

            // Accept clientat e ri
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

            if (clientSocket == INVALID_SOCKET) {
                continue;
            }

            unique_lock<mutex> lock(clientMutex);

            if (activeClientCounter < MAX_CONNECTED) {
                activeClientCounter++;
                cout << "New client processing. Active clients: " << activeClientCounter << endl;

              
                thread clientThread(handleClient, clientSocket, clientAddress, false);
                clientThread.detach();
            }
            else {
                // Nese max shtini ne queue
                clientsInQueue++;
                cout << "Max client number reached, adding to the queue. Number of clients in queue: " << clientsInQueue << endl;

                const char* message = "\nPlease wait till server is free!\n";
                send(clientSocket, message, strlen(message), 0);

                char clientIP[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);

                // Priority bazuar ne IP
                if (string(clientIP) == fullAccessClientIp) {
                    waitingClients.push_front({ clientSocket, clientAddress });
                    cout << "Client waiting with full permission!" << endl;
                }
                else {
                    waitingClients.push_back({ clientSocket, clientAddress });
                    cout << "Client waiting without full permission!" << endl;
                }

                // Unlock mutex for further operations
                lock.unlock();
            }

        
        }


    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}





