#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "connection/connectionHandler.hpp"
#include <thread>
using namespace std;



void errorCheck(int retVal, int fd = 2, const char* additionalMessage = nullptr);
void handleConnection(ConnectionHandler* handler);

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(sock);

    int val = 1;
    errorCheck(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) );

    sockaddr_in servaddr;
    servaddr.sin_port = htons(6060);
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1",&servaddr.sin_addr.s_addr);

    errorCheck(bind( sock, (sockaddr*) &servaddr, sizeof(sockaddr_in )) );
    errorCheck( listen(sock, 10) );

    while (true)
    {
        sockaddr_in clientAddr;
        socklen_t len;
        int client = accept(sock,  (sockaddr*)&clientAddr, &len);
        errorCheck(client);

        ConnectionHandler* handler = new ConnectionHandler(client, clientAddr, len);
        thread t ( handleConnection, handler );
        t.detach();
    }  
}

void errorCheck(int retVal, int fd, const char* additionalMessage)
{
    if(retVal != -1)
    {
        return;
    }
    dprintf(fd, "Encountered error: %s\n", strerror(errno));
    exit(-1);
}

void handleConnection(ConnectionHandler *handler)
{
    handler->handleClient();
    delete handler;
}

