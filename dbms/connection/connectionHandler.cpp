#include "connectionHandler.hpp"
#include "../parser/compiler.hpp"
#include <sys/socket.h>
#include <vector>
#include "../parser/parser.hpp"

using namespace std;

ConnectionHandler::ConnectionHandler(int client_fd, sockaddr_in sockAddr, socklen_t len)
:
m_client(client_fd), m_executor(new VirtualMachine() ), m_clientAddr(sockAddr), m_len(len), m_msgBuffer( new char[10000] )
{
}

void ConnectionHandler::handleClient()
{
    while (true)
    {
        int n = recv(m_client, m_msgBuffer, 10000, 0);
        if(n == 0)
        {
            break;
        }
        if( n > 0)
        {
            char* parserBuffer;
            vector<AstNode*> queries = parse(m_msgBuffer, &parserBuffer);
            InstructionData* byteCode = compile(queries);
            freeParserBuffer(parserBuffer);
            sendHandshake(m_client, queries.size());
            
            m_executor->execute(byteCode, m_client);
            freeInstructionData(byteCode);
        }
    }  
}

ConnectionHandler::~ConnectionHandler()
{
    delete[] m_msgBuffer;
    delete m_executor;
}

void ConnectionHandler::sendHandshake(int clientFd, uint8_t tableCount)
{
    send(m_client, &tableCount, 1, 0);
}
