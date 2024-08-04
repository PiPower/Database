#ifndef DB_STATE
#define DB_STATE

#include "../parser/compiler.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include "entry.hpp"
#include "../algorithms/avl_tree.hpp"
#include <thread>
struct Page
{
    char* dataBase;
    char* pageCurrent; // first unused byte in page
    std::vector<EntryDescriptor> entries;
    unsigned int aliveEntries;
};

typedef uint8_t ColumnFlags;
struct ColumnType
{
    MachineDataTypes machineType;
    DataTypes abstractType;
    uint16_t size;
    uint16_t offset;
    std::string columnName;
    AvlTree* tree;
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
    uint64_t itemCount;
    std::string tableName;

};

struct DatabaseState
{
    std::unordered_map<std::string, TableState*> tables;
    std::unordered_map<std::string, std::mutex*> locks;
    std::mutex databaseMutex;
};


//fused ops
IObuffer* selectFromTable(DatabaseState* database, std::string& tableName, std::vector<std::string>& colNames);

//general ops
IObuffer* createTable(DatabaseState* database, std::string& tableName, std::vector<ColumnType>& columns);
TableState* createTable(std::vector<ColumnType>& columns, const std::string& tableName, bool isSubTable = true);
TableState* createSubtable(DatabaseState *database, std::string& tableName, std::vector<std::string>& colNames);
void freeTable(TableState* table);
void freePage(Page* page);
IObuffer* serialazeTable(TableState* table);
IObuffer* insertIntoTable(DatabaseState* database,const std::string& tableName,
                    const std::vector<std::string>& colNames, const std::vector<uint32_t> argOffsets,
                     char* args, unsigned int& bytesWritten, char* msgBuffer, unsigned int bufferSize);

int filterTable(TableState* table, char* byteCode, bool inverseResult = false);
IObuffer* filterTable(DatabaseState *database, std::string& tableName,
                    char* byteCode, char* msgBuffer, unsigned int bufferSize, bool inverseResult = false);
TableState* selectAndMerge(DatabaseState* database, const std::vector<std::string>& tableNames , 
                            const std::vector<char*>& byteCodes, const std::vector<std::string>& colNames);
// misc
void updateStringOutputBuffer(IObuffer *buffer, bool error, const char *msg);
uint32_t copyMachineDataType(char* scratchpad, ColumnType& columnDesc, char* sourceData, MachineDataTypes currentType);
void insertIntoPage(TableState* table, char* data, uint32_t dataSize, bool insertIntoTrees = false);
Page* choosePage(TableState* table,  uint32_t requiredSpace);
const ColumnType* findColumn(const TableState* table, const std::string* columnName);
void selectFromPagesFixedEntrySize(IObuffer* buffer, TableState* table, std::vector< std::string> requestedColumns);
Page* createPage();
void markEntryAsDead(TableState* table, Page* page, int index);
// table locking mechanism
void lockTable(DatabaseState* database, std::string& tableName);
void unlockTable(DatabaseState* database, std::string& tableName);
void lockTables( DatabaseState* database, std::vector<std::string>& tableName);
void unlockTables(DatabaseState* database, std::vector<std::string>& tableName);
#endif