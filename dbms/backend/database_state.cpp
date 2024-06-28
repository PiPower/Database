#include "database_state.hpp"
#include <cstring>


using namespace std;
#define PAGE_SIZE 16384

void createTable(DatabaseState* database, std::string&& tableName, std::vector<ColumnType> &&columns)
{
    TableState* table = new TableState();
    table->columns = move(columns);
    table->maxEntrySize = 0;
    for(auto& col :  table->columns)
    {
         table->maxEntrySize += col.size;
    }
    Page* page= new Page();
    page->dataBase = new char[PAGE_SIZE];
    page->pageCurrent = page->dataBase;
    table->pages.push_back( page );
    database->tables[move(tableName)] = table;
}

void insertIntoTable(DatabaseState* database,const std::string& tableName,
                    const std::vector<std::string>& colNames, const std::vector<uint32_t> argOffsets, char* args, unsigned int& bytesWritten)
{
    auto tableIter = database->tables.find( tableName);
    if(tableIter ==  database->tables.end())
    {
        dprintf(2, "table not found");
        exit(-1);
    }

    vector<ColumnType>& typeTable = tableIter->second->columns;

    char* scratchPad = new char[tableIter->second->maxEntrySize];
    char* currScratchPad;
    unsigned int bytecodeSize;
    for(size_t i=0; i < argOffsets.size(); i++)
    {
        char* currentPtr = args + argOffsets[i];
        currScratchPad = scratchPad;
        for(size_t j = 0; j < typeTable.size(); j++)
        {
            MachineDataTypes currentType = (MachineDataTypes)(*(uint16_t*)currentPtr);
            currentPtr+=2;
            if(currentType != typeTable[j].type)
            {
                // TODO handle error
            }
            uint32_t offset = copyMachineDataType(currScratchPad, typeTable[j], currentPtr, currentType);
            currentPtr += offset;
            currScratchPad +=  typeTable[j].abstractType == DataTypes::CHAR ?  typeTable[j].size : offset;
            
        }
        bytecodeSize = currentPtr -  args - argOffsets[i];
        insertIntoPage(tableIter->second, scratchPad, currScratchPad - scratchPad);
    }
    bytesWritten = bytecodeSize + argOffsets[argOffsets.size() - 1 ];
}

uint32_t copyMachineDataType(char *scratchpad, ColumnType& columnDesc, char *sourceData, MachineDataTypes currentType)
{
    uint32_t copySize = 0;
    switch (currentType)
    {
    case MachineDataTypes::INT32 :
        copySize = 4;
        memcpy(scratchpad, sourceData, copySize);
        break;
    case MachineDataTypes::STRING:
        {
            copySize = strlen(sourceData);
            uint32_t include_null = 0;
            if( copySize < columnDesc.size ) {include_null = 1;} 
            memcpy(scratchpad, sourceData, copySize + include_null);
            copySize += 1;
        }break;
    default:
        break;
    }
    return copySize;
}

void insertIntoPage(TableState *table, char *data, uint32_t dataSize)
{
    Page* chosenPage = choosePage(table, dataSize);
    uint32_t offset = chosenPage->pageCurrent - chosenPage->dataBase;
    memcpy(chosenPage->pageCurrent, data, dataSize);
    chosenPage->pageCurrent += dataSize;

}

Page *choosePage(TableState *table, uint32_t requiredSpace)
{
    return table->pages[0];
}
