#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "parser/parser.hpp"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "backend/vm.hpp"
#include "parser/compiler.hpp"

using namespace std;

void errorCheck(int retVal, int fd = 2, const char* additionalMessage = nullptr);
void sendHandshake(int clientFd, uint8_t tableCount);
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

    /*
    currently serves only one connection dbms-client
    Communication with the client consists of 2 main parts
    The first message from the server is the number of response 
    tables that client can expect. It is followed by the tables
    where each counts as a one.
    */
    sockaddr_in clientAddr;
    socklen_t len;
a:
    int client = accept(sock,  (sockaddr*)&clientAddr, &len);
    errorCheck(client);
    char buffer[10000];
    while (true)
    {
        int n = recv(client, buffer, 10000, 0);
        if(n == 0)
        {
            goto a;
        }
        if( n > 0)
        {
            //TODO fix parser memory leak of full parse tree
            vector<AstNode*> queries = parse(buffer);
            InstructionData* byteCode = compile(queries);
            sendHandshake(client, queries.size());
            executor.execute(byteCode, client);
        }
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

void sendHandshake(int clientFd, uint8_t tableCount)
{
    send(clientFd, &tableCount, 1, 0);
}
