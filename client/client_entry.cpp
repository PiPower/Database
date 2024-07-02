#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/logs.hpp"
#include <string>
#include "../dbms/backend/types.hpp"
#include <vector>
#include <string.h>
using namespace std;


void displayResponse(char* buffer, unsigned int bufferSize)
{
    char* bufferCurrent = buffer;
    uint16_t colCount =  *(uint16_t*) bufferCurrent;
    bufferCurrent+= sizeof(uint16_t);
    vector<MachineDataTypes> types;
    vector<uint16_t> maxSizes;
    printf("Column names: ");
    for(int i=0; i < colCount; i++)
    {
        uint16_t colType = *(uint16_t*) bufferCurrent;
        bufferCurrent += sizeof(uint16_t);
        types.push_back((MachineDataTypes)colType );
        maxSizes.push_back( *(uint16_t*) bufferCurrent);
        bufferCurrent += sizeof(uint16_t);

        printf("%s", bufferCurrent);
        if(i < colCount - 1) 
        {
            printf(", ");
        }
        bufferCurrent += strlen(bufferCurrent) + 1;
    }
    printf("\n");
    while (bufferCurrent - buffer < bufferSize)
    {
        for(int i = 0; i < colCount; i++)
        {

            switch (types[i])
            {
            case MachineDataTypes::INT32 :
            {
                int val =  *(int*) bufferCurrent;
                bufferCurrent += sizeof(int);
                printf("%d", val);
            }break;
            case MachineDataTypes::STRING :
            {
                int stringLen = strlen(bufferCurrent) + 1;
                if(stringLen > maxSizes[i])
                {
                    stringLen = maxSizes[i];
                }
                printf("%.*s", stringLen, bufferCurrent);
                bufferCurrent += stringLen;
            } break;
            default:
                break;
            }

            if(i < colCount -1)
            {
                printf(", ");
            }

        }
        printf("\n");
    }

    fflush(stdout);
}

int main()
{
    string msg = "CREATE TABLE Workers(name char(34), surname char(34), age INT, id INT, partner_name char(7) );"
                 "INSERT Into Workers VALUES(\'Jan\', \'Kowalski\', 31, 1232445, \'Janina\' ), "
                 "(\'Jaroslaw\', \'Kryzewski\', 26, 32421, \'ASDFGHJ\' ), (\'TOmasz\', \'Walczewki\', 43, 6894, \'HAHAHAH\' );"
                 "SeLect name, age, id, partner_name from Workers ; ";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(sock);

    sockaddr_in server;
    server.sin_port = htons(6060);
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1",(char*) &server.sin_addr);
    errorCheck( connect(sock, (sockaddr*)&server, sizeof( sockaddr_in)));
    send(sock, msg.c_str(), msg.size(), 0);
    char buffer[10000];
    int n = recv(sock, buffer, 10000, 0);

    displayResponse(buffer, n);
}