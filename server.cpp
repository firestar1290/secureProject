#include "network.h"
#include <WS2tcpip.h>
#include <codecvt>
using namespace networking;

server::server(std::wstring IPAddr = SELF){
    //Step 1
    int WSAError = WSAStartup(WINSOCK_VERSION_NEEDED,&wsaData);
    if(WSAError){
        std::cerr << "Winsock dll not found\n";
        throw std::runtime_error("Winsock dll not found");
    }else{
        std::cout << "Status: " << wsaData.szSystemStatus << std::endl; //print WSA status and flush
    }

    //step 2
    serverSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(serverSocket==INVALID_SOCKET){
        cleanup();
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        throw std::exception("Error creating socket: " + WSAGetLastError());
    }

    //step 3
    socketAddr.sin_port = htons(SERVER_PORT);
    retry:
    socketAddr.sin_family = AF_INET;
    InetPton(AF_INET,IPAddr.c_str(),&socketAddr.sin_addr.s_addr);
    WSAError = bind(serverSocket,(SOCKADDR*)&socketAddr,sizeof(socketAddr));
    if(WSAError){
        if(socketAddr.sin_port != htons(BACKUP_SERVER_PORT)){
            std::cerr << "Binding failed, attempting backup port\n";
            socketAddr.sin_port = htons(BACKUP_SERVER_PORT);
            goto retry;
        }

        cleanup();
        std::cerr << "Error binding socket: " << WSAGetLastError() << std::endl;
        throw std::exception("Error binding socket: " + WSAGetLastError());
    }

    //Step 4
    WSAError = listen(serverSocket,MAX_PARTICIPTANTS-1);
    if(WSAError){
        cleanup();
        std::cerr << "Cannot listen: " << WSAGetLastError() << std::endl;
        throw std::exception("Cannot listen: " + WSAGetLastError());
    }
}

server::~server(){
    cleanup();
}

void server::cleanup(){
    if(serverSocket != INVALID_SOCKET) closesocket(this->serverSocket);
    int WSAError = WSACleanup();
    if(WSAError){
        std::cerr << "Cleanup error\n";
        throw std::runtime_error("Cleanup error");
    }
}