#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/logs.hpp"
#include "parser/parser.hpp"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "backend/vm.hpp"
#include "backend/compiler.hpp"
using namespace std;

int main()
{
    VirtualMachine executor;
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
#ifdef REMOTE_CLIENT
        int client = accept(sock,  (sockaddr*)&clientAddr, &len);
        errorCheck(client);
        recv(client, buffer, 400, 0);
#else
        const char* bufferTemp = "CREATE TABLE Workers(name char(34), surname char(34), age INT, id INT, partner_name char(34) );";
        memcpy(buffer, bufferTemp, 96);
#endif
        AstNode* query = parse(buffer);
        InstructionData* byteCode = compile(query);
        executor.exectue(byteCode) ;
    }  
}