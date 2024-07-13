#ifndef DB_STATE
#define DB_STATE

#include "../parser/compiler.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include "entry.hpp"

struct Page
{
    char* dataBase;
    char* pageCurrent; // first unused byte in page
    std::vector<EntryDescriptor> entries;
};


struct ColumnType
{
    MachineDataTypes machineType;
    DataTypes abstractType;
    uint16_t size;
    uint16_t offset;
    std::string columnName;
};

struct TableFags
{
    bool variableSizeEntry;
};


struct TableState
{
    std::vector<ColumnType> columns;
    unsigned int maxEntrySize;
    TableFags flags;
    std::vector<Page*> pages;

};

struct DatabaseState
{
    std::unordered_map<std::string, TableState*> tables;
};


//fused ops
IObuffer* selectFromTable(DatabaseState* database, std::string&& tableName, std::vector<std::string>&& colNames);

//general ops
IObuffer* createTable(DatabaseState* database, std::string&& tableName, std::vector<ColumnType>&& columns);
TableState* createTable(std::vector<ColumnType>& columns);
TableState* createSubtable(DatabaseState *database, std::string &&tableName, std::vector<std::string> &&colNames);
IObuffer* serialazeTable(TableState* table);
IObuffer* insertIntoTable(DatabaseState* database,const std::string& tableName,
                    const std::vector<std::string>& colNames, const std::vector<uint32_t> argOffsets,
                     char* args, unsigned int& bytesWritten, char* msgBuffer, unsigned int bufferSize);

void filterTable(TableState* table, char* byteCode);
// misc
void updateStringOutputBuffer(IObuffer *buffer, bool error, const char *msg);
#endif