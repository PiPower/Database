#ifndef DB_STATE
#define DB_STATE

#include "compiler.hpp"
#include <unordered_map>
#include <string>
#include <vector>

struct Page
{
    char* data;
    std::vector<uint16_t> offsets;
};


struct ColumnType
{
    MachineDataTypes type;
    DataTypes abstractType;
    uint16_t size;
    std::string columnName;
};


struct TableState
{
    std::vector<ColumnType> columns;
    unsigned int maxEntrySize;
    std::vector<Page*> pages;
};

struct DatabaseState
{
    std::unordered_map<std::string, TableState*> tables;
};



void createTable(DatabaseState* database, std::string&& tableName, std::vector<ColumnType>&& columns);
void insertIntoTable(DatabaseState* database,const std::string& tableName,
                    const std::vector<std::string>& colNames, const std::vector<uint32_t> argOffsets, char* args, unsigned int& bytesWritten);
uint32_t copyMachineDataType(char* scratchpad, ColumnType& columnDesc, char* sourceData, MachineDataTypes currentType);
#endif