#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/logs.hpp"
#include "parser/parser.hpp"
#include <stdio.h>
#include <iostream>
using namespace std;

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

    char buffer[400];
    while (true)
    {
        sockaddr_in clientAddr;
        socklen_t len;
        int client = accept(sock,  (sockaddr*)&clientAddr, &len);
        errorCheck(client);
        recv(client, buffer, 400, 0);

        parse(buffer);
    }  
}