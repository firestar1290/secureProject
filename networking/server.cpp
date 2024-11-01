#include "network.h"
#include <WS2tcpip.h>
#include <codecvt>
#include <locale>
using namespace network;

server::server(std::string IPAddr = SELF){
    //Step 1
    std::cout << "Starting WSA\n";
    int WSAError = WSAStartup(WINSOCK_VERSION_NEEDED,&requiredData);
    if(WSAError){
        std::cerr << "Winsock dll not found\n";
        throw std::runtime_error("Winsock dll not found");
    }else{
        std::cout << "Status: " << requiredData.szSystemStatus << std::endl; //print WSA status and flush
    }

    //step 2
    std::cout << "Creating socket\n";
    serverSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(serverSocket==INVALID_SOCKET){
        cleanup();
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        throw std::runtime_error("Error creating socket: " + WSAGetLastError());
    }

    //step 3
    std::cout << "Binding\n";
    socketAddr.sin_port = htons(SERVER_PORT);
    retry:
    socketAddr.sin_family = AF_INET;
    InetPton(AF_INET,IPAddr.c_str(),&socketAddr.sin_addr.s_addr); //there's an eroneous error of "const char* is incompatiable with LPCWSTR", ignore it
    WSAError = bind(serverSocket,(SOCKADDR*)&socketAddr,sizeof(socketAddr));
    if(WSAError){
        if(socketAddr.sin_port != htons(BACKUP_SERVER_PORT)){
            std::cerr << "Binding failed, attempting backup port\n";
            socketAddr.sin_port = htons(BACKUP_SERVER_PORT);
            goto retry;
        }

        cleanup();
        std::cerr << "Error binding socket: " << WSAGetLastError() << std::endl;
        throw std::runtime_error("Error binding socket: " + WSAGetLastError());
    }

    //Step 4
    std::cout << "Listening\n";
    WSAError = listen(serverSocket,MAX_PARTICIPTANTS-1);
    if(WSAError){
        cleanup();
        std::cerr << "Cannot listen: " << WSAGetLastError() << std::endl;
        throw std::runtime_error("Cannot listen: " + WSAGetLastError());
    }
}

//The returned SOCKET should be stored somewhere
SOCKET server::acceptConnection(){
    //Step 5
    SOCKET otherParty;
    std::cout << "Waiting for connection...";

    otherParty = accept(serverSocket,NULL,NULL); //Blocks
    if (otherParty == INVALID_SOCKET){
        std::cout << "\nConnection refused: " << WSAGetLastError() << "\n";
        cleanup();
    }else{
        std::cout << "\nConnection accepted\n";
    }
    return otherParty;
}

server::~server(){
    cleanup();
}

void server::cleanup(){
    if(serverSocket != INVALID_SOCKET) closesocket(this->serverSocket);
    int WSAError = WSACleanup();
    int error = WSAGetLastError();
    if(WSAError && error != 10093){
        std::cerr << "Cleanup error: "<< error <<"\n";
        throw std::runtime_error("Cleanup error: " + error);
    }else if(error == 10093){
        std::cerr << "WSA does not exist/already shutdown\n";
    }
}

server::server(SOCKET connectedSocket){
    int socketAddrLen = sizeof(socketAddr);
    getpeername(connectedSocket,(SOCKADDR*)&socketAddr,&socketAddrLen);
    serverSocket = connectedSocket;
}

std::string server::receiveMessage(){
    int error;
    int num_recv;
    if((num_recv = recv(serverSocket,buffer,BUFFER_SIZE,0)) == SOCKET_ERROR && (error = WSAGetLastError())){
        std::cerr << "Could not receive message: " << error << std::endl;
        return std::string("");
    }else{
        buffer[num_recv] = '\0';
        return std::string(buffer);
    }
}

server::server(const network::server& copyFrom){
    this->requiredData = copyFrom.requiredData;
    this->serverSocket = copyFrom.serverSocket;
    this->socketAddr = copyFrom.socketAddr;
}