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
void selectFromPagesFixedEntrySize(IObuffer* buffer, TableState* table, vector<string> requestedColumns);
void updateStringOutputBuffer( IObuffer* buffer, bool error, const char* errMsg);

IObuffer* createTable(DatabaseState* database, std::string&& tableName, std::vector<ColumnType> &&columns)
{
    IObuffer* buffer = createInstructionData();
    auto tableIter = database->tables.find( tableName);
    if(tableIter !=  database->tables.end())
    {
        static const char* const errMsg = "Table with specified name already exists";
        updateStringOutputBuffer(buffer, true, errMsg);
        return buffer;
    }


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


    static const char* const successMSG = "Table has been created succesfully";
    updateStringOutputBuffer(buffer, false, successMSG);
    return buffer;
}

IObuffer* insertIntoTable(DatabaseState* database,const std::string& tableName,
                    const std::vector<std::string>& colNames, const std::vector<uint32_t> argOffsets,
                     char* args, unsigned int& bytesWritten, char* msgBuffer, unsigned int bufferSize)
{
    IObuffer* buffer = createInstructionData();

    auto tableIter = database->tables.find( tableName);
    if(tableIter ==  database->tables.end())
    {
        static const char* const errMsg = "Table with specified name does not exist";
        updateStringOutputBuffer(buffer, true, errMsg);
        return buffer;
    }

    vector<ColumnType>& typeTable = tableIter->second->columns;
    char* scratchPad = new char[tableIter->second->maxEntrySize];
    char* currScratchPad;
    unsigned int bytecodeSize;
    unsigned int corretValues = 0;
    unsigned int numberOfArgs = static_cast<unsigned int>( argOffsets.size() );

    for(size_t i=0; i <numberOfArgs; i++)
    {
        char* currentPtr = args + argOffsets[i];
        currScratchPad = scratchPad;
        bool typeErrorFlag = false;
        for(size_t j = 0; j < typeTable.size(); j++)
        {
            MachineDataTypes currentType = (MachineDataTypes)(*(uint16_t*)currentPtr);
            currentPtr+=2;
            if(currentType != typeTable[j].machineType)
            {
                typeErrorFlag = true;
            }
            uint32_t offset = copyMachineDataType(currScratchPad, typeTable[j], currentPtr, currentType);
            currentPtr += offset;
            currScratchPad +=  typeTable[j].abstractType == DataTypes::CHAR ?  typeTable[j].size : offset;
            
        }

        bytecodeSize = currentPtr -  args - argOffsets[i];
        bytesWritten += bytecodeSize;
        if(!typeErrorFlag)
        {
            insertIntoPage(tableIter->second, scratchPad, currScratchPad - scratchPad);
            corretValues++;
        }
    }

    snprintf(msgBuffer, bufferSize, "Successfully writtent %u out of %u items", corretValues, numberOfArgs);
    updateStringOutputBuffer(buffer, false, msgBuffer);
    return buffer;
}

//output of function is binary data in form 
// header + body
// header =  item_count(uint32_t)  |col_count(uint16_t) | col_desc_1, ... , col_col_count 
// col_desc_1 = machine_type(uint16_t) | max_col_size(uint16) | col_name (null terminated string)
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
        selectFromPagesFixedEntrySize(buffer,  tableIter->second, colNames);
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
            if( copySize > columnDesc.size) {copySize = columnDesc.size;}
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
    int currentPage = table->pages.size() - 1;
    uint16_t remainingSpace =  PAGE_SIZE - (table->pages[currentPage]->pageCurrent - table->pages[currentPage]->dataBase);
    if( remainingSpace >= requiredSpace)
    {
        return table->pages[currentPage];
    }

    Page* page= new Page();
    page->dataBase = new char[PAGE_SIZE];
    page->pageCurrent = page->dataBase;
    table->pages.push_back( page );
    return table->pages[currentPage + 1];
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

void selectFromPagesFixedEntrySize(IObuffer *buffer, TableState *table, vector<string> requestedColumns)
{
    uint16_t maxEntrySize = 0;
    vector<ColumnType*> requestedColumnsTypes;
    unsigned int headerSize = sizeof(uint32_t) + sizeof(uint16_t);
    for(string& name : requestedColumns)
    {
        ColumnType* column =  findColumn(table, &name);
        maxEntrySize += column->size;
        requestedColumnsTypes.push_back(column);
        headerSize += sizeof(uint16_t) * 2;
        headerSize += column->columnName.size() + 1;
    };

    char *header = new char[headerSize];
    header += sizeof(uint32_t);
    *((uint16_t*) header) = (uint16_t) requestedColumnsTypes.size();

    char* headerCurrent = header + sizeof(uint16_t);
    for(int i=0; i < requestedColumnsTypes.size(); i++)
    {
        uint16_t columnMachineType = (uint16_t) requestedColumnsTypes[i]->machineType;
        memcpy(headerCurrent, &columnMachineType, sizeof(uint16_t)); // type
        headerCurrent += sizeof(uint16_t);
        memcpy(headerCurrent, &requestedColumnsTypes[i]->size, sizeof(uint16_t)); // max size in bytes
        headerCurrent += sizeof(uint16_t);
        memcpy(headerCurrent, requestedColumnsTypes[i]->columnName.c_str(), requestedColumnsTypes[i]->columnName.size() + 1); // column name
        headerCurrent += requestedColumnsTypes[i]->columnName.size() + 1;
    }

    char* skippedPos = skipBytes(buffer, headerSize);

    char* entryBuffer = new char[maxEntrySize];
    uint32_t selectedValues = 0;
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
                uint16_t copySize = column->machineType == MachineDataTypes::STRING ? 
                strlen(currentEntry + column->offset) + 1 :
                column->size;

                if(copySize > column->size)
                {
                    copySize = column->size;
                }

                memcpy(currentEntryBuffer,  currentEntry + column->offset, copySize);
                currentEntryBuffer+= copySize;
            }
            selectedValues++;
            emitPayload(buffer, entryBuffer, currentEntryBuffer - entryBuffer );
        }
    }
    header -= sizeof(uint32_t);
    *(uint32_t*)header = selectedValues;
    fillSkippedBytes(buffer, skippedPos, header, headerSize);
}

// creating buffer for error message
void updateStringOutputBuffer(IObuffer *buffer, bool error, const char *msg)
{
    static const char* const colNameError = "Error";
    static const char* const  colNameMessage = "Message";
    const char* outputType;
    if(error)
    {
        outputType = colNameError;
    }
    else
    {
        outputType = colNameMessage;
    }
    uint32_t itemCount = 1;
    uint16_t msgSize = strlen(msg) + 1;
    uint16_t colCount = 1;
    uint16_t machineType = (uint16_t)MachineDataTypes::STRING;

    emitPayload(buffer, &itemCount, 4);
    emitPayload(buffer, &colCount, 2);
    emitPayload(buffer, &machineType, 2);
    emitPayload(buffer, &msgSize, 2);
    emitPayload(buffer, outputType, strlen(outputType) + 1);
    emitPayload(buffer, msg, strlen(msg) + 1);
}
