#include "client.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include <vector>
#include <string>
#include "../dbms/backend/types.hpp"
#include <stdio.h>

using namespace std;

struct Connection
{
    int sockFd;
};

struct ColumnDescriptor
{
    string name;
    uint16_t maxSize;
    MachineDataTypes type;
};

struct Table
{
    //table of pointers
    //each pointer in the table corresponds to one column
    char** columnTable;
    vector<ColumnDescriptor> columnDescriptors;
    unsigned int itemCount;
};


static Table* readTable(char* buffer, const unsigned int totalSize,  unsigned int& readSize);
static void errorCheck(int retVal, int fd = 2, const char* additionalMessage = nullptr);

Connection* connectToDbms(time_t seconds, suseconds_t microseconds)
{

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(sock);

    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = microseconds;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    sockaddr_in server;
    server.sin_port = htons(6060);
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1",(char*) &server.sin_addr);
    errorCheck( connect(sock, (sockaddr*)&server, sizeof( sockaddr_in)));

    Connection* connection = new Connection();
    connection->sockFd = sock;
    return connection;
}

void sendQuery(Connection* connection, const char *query)
{
    send(connection->sockFd, query, strlen(query) + 1, 0);
}

std::vector<Table*> readResponse(Connection *connection)
{
    int buffSize = 10000;
    int size = 0;
    char* buffer = new char[buffSize];
    while (true)
    {
        int n = recv(connection->sockFd, buffer + size, buffSize, 0);

        if( n < 1 )
        {
            break;
        }
        size += n;
        if(size < buffSize)
        {
            break;
        }
        else if( size == buffSize)
        {
            buffSize *= 2;
            char* temp = new char[buffSize];
            memcpy(temp, buffer, size );
            delete[] buffer;
            buffer = temp;
        }
    }
    vector<Table*> tables;
    unsigned int readDataSize = 0;
    while (readDataSize < size)
    {
        unsigned int offset = 0;
        tables.push_back( readTable(buffer + readDataSize, size - readDataSize,offset) );
        readDataSize += offset;
    }
    
    return tables;
}

void printTable(Table* table)
{
    printf("Column names: ");
    for(int i=0; i < table->columnDescriptors.size(); i++)
    {
        printf("%s",  table->columnDescriptors[i].name.c_str());
        if( i <  table->columnDescriptors.size() - 1)
        {
            printf(", ");
        }
    }

    printf("\n-------------------------\n");
    for(int i=0; i < table->itemCount; i++)
    {
        for(int column = 0; column < table->columnDescriptors.size(); column++)
        {
            unsigned int offset = i * table->columnDescriptors[column].maxSize;
            char* data = table->columnTable[column] + offset;
            switch (table->columnDescriptors[column].type)
            {
            case MachineDataTypes::INT32:
                printf("%d", *(int*)data );
                break;
            case MachineDataTypes::STRING:
            {
                int charCount = min(strlen(data) + 1,(unsigned long) table->columnDescriptors[column].maxSize);
                printf("%.*s", charCount, data );
            }break;
            }
            if(column < table->columnDescriptors.size() -1)
            {
                printf(", ");
            }
        }
        printf("\n");

    }
    fflush(stdout);
}

static Table* readTable(char* buffer, const unsigned int totalSize, unsigned int& readSize)
{
    readSize = 0;
    Table* table = new Table();
    char* bufferCurrent = buffer;

    uint32_t itemCount = *(uint32_t*) bufferCurrent;
    bufferCurrent += sizeof(uint32_t);
    uint16_t colCount =  *(uint16_t*) bufferCurrent;
    bufferCurrent+= sizeof(uint16_t);

    for(int i=0; i < colCount; i++)
    {
        uint16_t colType = *(uint16_t*) bufferCurrent;
        bufferCurrent += sizeof(uint16_t);
        uint16_t maxSize = *(uint16_t*) bufferCurrent;
        bufferCurrent += sizeof(uint16_t);
        char* colName = bufferCurrent;
        bufferCurrent += strlen(bufferCurrent) + 1;

        ColumnDescriptor desc;
        desc.type = (MachineDataTypes)colType;
        desc.maxSize = maxSize;
        desc.name = colName;
        table->columnDescriptors.push_back(move(desc));
    }

    table->itemCount = itemCount;
    table->columnTable = new char*[colCount];
    for(int i= 0; i < colCount; i++)
    {
        table->columnTable[i] = new char[itemCount * table->columnDescriptors[i].maxSize];
    }

    int item = 0;
    while (bufferCurrent - buffer < totalSize && item < itemCount )
    {
        for(int i = 0; i < colCount; i++)
        {
            char* currentColumn = table->columnTable[i];
            switch (table->columnDescriptors[i].type)
            {
            case MachineDataTypes::INT32:
            {
                memcpy(currentColumn + item * table->columnDescriptors[i].maxSize,  bufferCurrent, sizeof(int));
                bufferCurrent += sizeof(int);
            }break;
            case MachineDataTypes::STRING:
            {
                int stringLen = strlen(bufferCurrent) + 1;
                if(stringLen > table->columnDescriptors[i].maxSize)
                {
                    stringLen = table->columnDescriptors[i].maxSize;
                }
                memcpy(currentColumn + item * table->columnDescriptors[i].maxSize,  bufferCurrent, stringLen);
                bufferCurrent += stringLen;
            } break;
            default:
                break;
            }

        }
        item++;
    }
    readSize = bufferCurrent - buffer;
    return table;
}

static void errorCheck(int retVal, int fd, const char* additionalMessage)
{
    if(retVal != -1)
    {
        return;
    }
    dprintf(fd, "Encountered error: %s\n", strerror(errno));
    exit(-1);
}