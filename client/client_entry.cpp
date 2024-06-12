#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/logs.hpp"
#include <string>
using namespace std;

int main()
{
    string msg = "CREATE TABE Workers(name char(34), surname char(34), age INT);";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(sock);

    sockaddr_in server;
    server.sin_port = htons(6060);
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1",(char*) &server.sin_addr);
    errorCheck( connect(sock, (sockaddr*)&server, sizeof( sockaddr_in)));
    send(sock, msg.c_str(), msg.size(), 0);

}