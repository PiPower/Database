#include "database_state.hpp"
#include <cstring>
#include <algorithm>
using namespace std;
#define PAGE_SIZE 16384

typedef InstructionData IObuffer;

uint32_t copyMachineDataType(char* scratchpad, ColumnType& columnDesc, char* sourceData, MachineDataTypes currentType);
void insertIntoPage(TableState* table, char* data, uint32_t dataSize);
Page* choosePage(TableState* table,  uint32_t requiredSpace);
ColumnType* findColumn(TableState* table, std::string* columnName);
void selectFromPagesFixesEntrySize(IObuffer* buffer, TableState* table, vector<string> requestedColumns);


void createTable(DatabaseState* database, std::string&& tableName, std::vector<ColumnType> &&columns)
{
    TableState* table = new TableState();
    table->columns = move(columns);
    table->maxEntrySize = 0;
    table->flags.variableSizeEntry = false;
    for(auto& col :  table->columns)
    {
        col.offset = table->maxEntrySize;
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

//output of function is binary data in form 
// header + body
// header = col_count(uint16_t) | col_desc_1, ... , col_col_count 
// col_desc_1 = machine_type(uint16_t) | col_name (null terminated string)
// body is just sequence of bytes described by header
IObuffer* selectFromTable(DatabaseState *database, std::string &&tableName, std::vector<std::string> &&colNames)
{
    auto tableIter = database->tables.find( tableName);
    if(tableIter ==  database->tables.end())
    {
        dprintf(2, "table not found");
        exit(-1);
    }
    IObuffer* buffer = createInstructionData();
    if(tableIter->second->flags.variableSizeEntry)
    {
        //currently variable size entries are not supported
    }
    else
    {
        selectFromPagesFixesEntrySize(buffer,  tableIter->second, colNames);
    }
    return buffer;
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
    chosenPage->offsets.push_back(offset );
    memcpy(chosenPage->pageCurrent, data, dataSize);
    chosenPage->pageCurrent += dataSize;

}

Page *choosePage(TableState *table, uint32_t requiredSpace)
{
    return table->pages[0];
}

ColumnType *findColumn(TableState *table, std::string *columnName)
{
    vector<ColumnType>& tableColumns =  table->columns;
    for(int i=0; i < tableColumns.size(); i++)
    {
        if(tableColumns[i].columnName == *columnName )
        {
            return &tableColumns[i];
        }
    }
    return nullptr;
}

void selectFromPagesFixesEntrySize(IObuffer *buffer, TableState *table, vector<string> requestedColumns)
{
    uint16_t maxEntrySize = 0;
    vector<uint16_t> columnOffset;
    vector<ColumnType*> requestedColumnsTypes;
    for(string& name : requestedColumns)
    {
        ColumnType* column =  findColumn(table, &name);
        maxEntrySize += column->size;
        requestedColumnsTypes.push_back(column);
    };


    char* entryBuffer = new char[maxEntrySize];

    for(int i=0; i < table->pages.size(); i++ )
    {
        Page* currentPage = table->pages[i];
        for(int j = 0; j < currentPage->offsets.size(); j++)
        {
            uint16_t currentOffset = currentPage->offsets[j];
            char* currentEntry = currentPage->dataBase + currentOffset;
            char* currentEntryBuffer = entryBuffer;
            for(ColumnType* column : requestedColumnsTypes)
            {
                uint16_t copySize = column->type == MachineDataTypes::STRING ? 
                strlen(currentEntry + column->offset) + 1 :
                column->size;

                memcpy(currentEntryBuffer,  currentEntry + column->offset, copySize);
                currentEntryBuffer+= copySize;
            }

            emitPayload(buffer, entryBuffer, currentEntryBuffer - entryBuffer );
        }
    }

}
