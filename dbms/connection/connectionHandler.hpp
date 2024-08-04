#ifndef CONNECTION_HANDLER
#define CONNECTION_HANDLER
#include "../backend/vm.hpp"
#include <netinet/in.h>

class ConnectionHandler
{

public:
    ConnectionHandler(int client_fd, sockaddr_in sockAddr, socklen_t len);
    void handleClient();
    ~ConnectionHandler();
private:
    void sendHandshake(int clientFd, uint8_t tableCount);
private:
    char* m_msgBuffer;
    const int m_client;
    VirtualMachine* m_executor;
    sockaddr_in m_clientAddr;
    socklen_t m_len;
};




#endif