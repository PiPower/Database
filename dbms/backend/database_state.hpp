#ifndef DB_STATE
#define DB_STATE

#include "compiler.hpp"
#include <unordered_map>
#include <string>
#include <vector>

struct Page
{
    void* data;
    unsigned int elemenetsOffset[32];
};


struct ColumnType
{
    DataTypes type;
    uint16_t size;
    std::string columnName;
};


struct TableState
{
    std::vector<ColumnType> columns;

    std::vector<Page*> pages;
};

struct DatabaseState
{
    std::unordered_map<std::string, TableState*> tables;
};



void createTable(DatabaseState& database, std::string&& tableName, std::vector<ColumnType>&& columns);
void insertIntoTable(DatabaseState& database, std::string&& tableName);

#endif