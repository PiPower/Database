#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/logs.hpp"
#include <string>
#include "../dbms/backend/types.hpp"
#include <vector>
#include <string.h>
using namespace std;


int displayResponse(char* buffer, unsigned int bufferSize)
{
    char* bufferCurrent = buffer;
    uint32_t itemCount = *(uint32_t*) bufferCurrent;
    bufferCurrent += sizeof(uint32_t);
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
    int i =0;
    while (bufferCurrent - buffer < bufferSize && i < itemCount )
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
        i++;
    }

    printf("*********************************\n");
    fflush(stdout);
    return  bufferCurrent - buffer;
}

int main()
{
    
    string msg = "CREATE TABLE Workers(name char(34), surname char(34), age INT, id INT, partner_name char(7) );"
                 "INSERT Into Workers VALUES(\'Jan\', \'Kowalski\', 31, 1232445, \'Janina\' ), "
                 "(\'Jaroslaw\', \'Kryzewski\', 26, 32421, \'ASDFGHJ\' ),"
                 "(\'TOmasz\', \'Walczewki\', 43, 6894, \'HAHAHAH\' ),"
                 "(\'Ferdynand\', \'Kiepski\', 36, 19, \'Halina\' );"
                 "SeLect name, age, id, partner_name, surname from Workers;"
                 "SeLect name, age, id, partner_name, surname from Workers where id > 30;";
                 
   /*
    string msg = "CREATE TABLE Workers(name char(34), surname char(34), age INT, id INT, partner_name char(7) );"
                 "CREATE TABLE Workers(name char(34), surname char(34), age INT, id INT, partner_name char(7) );";
    */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(sock);

    sockaddr_in server;
    server.sin_port = htons(6060);
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1",(char*) &server.sin_addr);
    errorCheck( connect(sock, (sockaddr*)&server, sizeof( sockaddr_in)));
    send(sock, msg.c_str(), msg.size(), 0);
    char buffer[10000];
    while (true)
    {
        int n = recv(sock, buffer, 10000, MSG_DONTWAIT );
        if( n > 0)
        {
            int readBytes = 0;
            while (true)
            {
                readBytes += displayResponse(buffer + readBytes, n);
                if(readBytes >= n)
                {
                    break;
                }
            }
            
        }
    }
    

}