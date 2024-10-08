#include "database_state.hpp"
#include <cstring>
#include <algorithm>
#include <random>
#include "cursor.hpp"
#include "expression.hpp"
using namespace std;
#define PAGE_SIZE 16384

typedef InstructionData IObuffer;
pair<vector<ColumnType>, vector<string>> fetchColumnsFromCursors(const vector<Cursor>& cursors, const vector<string>& columnNames);

IObuffer* createTable(DatabaseState* database, string& tableName, vector<ColumnType>& columns)
{
    database->databaseMutex.lock();

    IObuffer* buffer = createInstructionData();
    auto tableIter = database->tables.find( tableName);
    if(tableIter !=  database->tables.end())
    {
        static const char* const errMsg = "Table with specified name already exists";
        updateStringOutputBuffer(buffer, true, errMsg);
        database->databaseMutex.unlock();
        return buffer;
    }
    database->tables[tableName] = createTable( columns, tableName, false );
    database->locks[tableName] = new mutex();

    static const char* const successMSG = "Table has been created succesfully";
    updateStringOutputBuffer(buffer, false, successMSG);

    database->databaseMutex.unlock();
    return buffer;
}

TableState *createTable(vector<ColumnType>& columns,const string& tableName, bool isSubTable)
{
    TableState* table = new TableState();
    table->columns = columns;
    table->maxEntrySize = 0;
    table->itemCount = 0;
    table->flags.variableSizeEntry = false;
    table->tableName = tableName;
    for(auto& col :  table->columns)
    {
        col.offset = table->maxEntrySize;
        table->maxEntrySize += col.size;
        if(isSubTable)
        {
            // if table is mean to be subtable that means its used within system
            // so it does not need its own avl tree 
            col.tree = nullptr;
        }
    }

    Page* page= createPage();
    table->pages.push_back( page );
    return table;
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
        bool keyErrorFlag = false;
        for(size_t j = 0; j < typeTable.size(); j++)
        {
            MachineDataTypes currentType = (MachineDataTypes)(*(uint16_t*)currentPtr);
            currentPtr+=2;
            int64_t data = fetchAs64BitInt(currentType, currentPtr);
            if(currentType != typeTable[j].machineType)
            {
                typeErrorFlag = true;
            }
            if(typeTable[j].tree != nullptr && typeTable[j].tree->find((char*)&data) )
            {
                keyErrorFlag = true;
            }

            uint32_t offset = copyMachineDataType(currScratchPad, typeTable[j], currentPtr, currentType);
            currentPtr += offset;
            currScratchPad +=  typeTable[j].abstractType == DataTypes::CHAR ?  typeTable[j].size : offset;
            
        }

        bytecodeSize = currentPtr -  args - argOffsets[i];
        bytesWritten += bytecodeSize;
        if(!typeErrorFlag && !keyErrorFlag)
        {
            insertIntoPage(tableIter->second, scratchPad, currScratchPad - scratchPad, true);
            corretValues++;
        }
    }

    snprintf(msgBuffer, bufferSize, "Successfully written %u out of %u items", corretValues, numberOfArgs);
    updateStringOutputBuffer(buffer, false, msgBuffer);
    return buffer;
}

int filterTable(TableState *table, char *byteCode, bool inverseResult)
{
    vector<ExpressionEntry> stack;

    ColumnTypeHashmap columnHash;
    for(int i=0; i< table->columns.size(); i++)
    {
        columnHash[table->columns[i].columnName] = &table->columns[i];
    }

    unordered_map<string, ColumnTypeHashmap> columnsPerTable;
    constexpr const char* name = "temp";
    columnsPerTable[name] =  move(columnHash);

    vector<EntryBase> entry;
    entry.push_back({name, nullptr});
    int deleted = 0;
    for(int i=0; i < table->pages.size(); i++ )
    {
        Page* currentPage = table->pages[i];

        for(int j = 0; j < currentPage->aliveEntries; j++)
        {
            uint16_t currentOffset = GET_OFFSET(currentPage->entries[j] );

            entry[0].ptr = currentPage->dataBase + currentOffset;
            bool expRes = !executeComparison(stack, byteCode, entry, columnsPerTable);
            expRes = inverseResult? !expRes : expRes;
            if(expRes)
            {
                markEntryAsDead(table, currentPage, j);
                deleted++;
                j--;
            }
            stack.clear();
        }
    
    }

    return deleted;
}

TableState* selectAndMerge(DatabaseState *database, const vector<string>& tableNames, 
                            const vector<char*>& byteCode, const std::vector<std::string>& colNames)
{
    // create cursors and bind information about columns and each of tables used in executeComparison
    vector<Cursor> cursors = createListOfCursors(database, tableNames);
    unordered_map<string, ColumnTypeHashmap> columnsPerTable;
    for(int i=0; i < cursors.size(); i++)
    {
        vector<ColumnType>& columns = cursors[i].m_table->columns;
        for(int j =0; j < columns.size(); j++)
        {
            columnsPerTable[cursors[i].m_table->tableName][columns[j].columnName] = &columns[j];
        }
    }

    // create information about used by column in new table, and store parent tables of those columns
    auto colsAndTableNames = fetchColumnsFromCursors(cursors, colNames);
    vector<ColumnType> columns = move(colsAndTableNames.first);
    vector<string> columnParentTables = move(colsAndTableNames.second);
    uint32_t maxSize = 0;
    for(ColumnType& col : columns)
    {
        maxSize+= col.size;
    }

    
    TableState* outTable = createTable(columns, "" );
    //prepare entry data for executeComparison and copy actual data
    vector<EntryBase> entries;
    unordered_map<string, char*> entryMap;
    for(int i = 0; i < cursors.size(); i++)
    {
        entries.push_back( {cursors[i].m_table->tableName, nullptr } );
        entryMap[cursors[i].m_table->tableName] = nullptr;
    }

    char* entryBuffer = new char[maxSize];
    char* entryBufferCurrent = entryBuffer;
    vector<ExpressionEntry> stack;
    while (true)
    {
        //update entry info
        for(int i = 0; i < cursors.size(); i++)
        {
            entries[i].ptr =  cursors[i].getEntry();
            entryMap[cursors[i].m_table->tableName] = entries[i].ptr;
        }

        // check if conditions are met
        bool belongsToTable;
        for(int i=0; i < byteCode.size(); i++)
        {
            belongsToTable = executeComparison( stack, byteCode[i], entries, columnsPerTable);
            if(!belongsToTable)
            {
                break;
            }

        }

        // if met add elem to output table
        if(belongsToTable)
        {
            for(int i = 0; i < columns.size(); i++)
            {
                memcpy(entryBufferCurrent, entryMap[columnParentTables[i]] + columns[i].offset, columns[i].size);
                entryBufferCurrent += columns[i].size;
            }
            insertIntoPage(outTable, entryBuffer, entryBufferCurrent - entryBuffer);
            entryBufferCurrent = entryBuffer;
        }

        stack.clear();
        //increment cursors, if l-th cursor wraps around increment l+1th cursor
        for(int i = 0; i < cursors.size(); i++)
        {
            bool wrapAround = cursors[i].increment();
            if(!wrapAround)
            {
                break;
            }

            if(wrapAround && i == cursors.size() - 1)
            {
                delete[] entryBuffer;
                return outTable;
            }
        }
    }
    

}

// output of function is binary data in form 
// header + body
// header = item_count(uint32_t)  |col_count(uint16_t) | col_desc_1, ... , col_desc_col_count 
// col_desc_1 = machine_type(uint16_t) | max_col_size(uint16) | col_name (null terminated string)
// body is just sequence of bytes described by header in the order following col_desc
IObuffer* selectFromTable(DatabaseState *database, string& tableName, vector<string>& colNames)
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

TableState *createSubtable(DatabaseState *database, std::string& tableName, std::vector<std::string>& colNames)
{
    auto tableIter = database->tables.find( tableName);
    if(tableIter ==  database->tables.end())
    {
        return nullptr;
    }
    TableState* table =  tableIter->second;

    vector<ColumnType> requestedColumnsTypes;
    for(string& name : colNames)
    {
        ColumnType column =  *findColumn(table, &name);
        requestedColumnsTypes.push_back(column);
    };

    TableState* subtable = createTable( requestedColumnsTypes, "" );
    char* buffer = new char[subtable->maxEntrySize];

    for(int i=0; i < table->pages.size(); i++ )
    {
        Page* currentPage = table->pages[i];
        for(int j = 0; j < currentPage->entries.size(); j++)
        {
            //skip dead entries
            if( IS_DEAD( currentPage->entries[j] ) )
            {
                continue;
            }

            uint16_t currentOffset = GET_OFFSET(currentPage->entries[j] );
            char* currentEntry = currentPage->dataBase + currentOffset;
            char* currentBuffer = buffer;


            for(ColumnType& column : requestedColumnsTypes)
            {
                uint16_t copySize = column.size;
                memcpy(currentBuffer,  currentEntry + column.offset, copySize);
                currentBuffer+= copySize;
            }
            insertIntoPage(subtable, buffer, currentBuffer - buffer);
        }
    }

    delete[] buffer;
    return subtable;
}

void freeTable(TableState* table)
{
    for(ColumnType& col : table->columns)
    {
        if(col.tree)
        {
            col.tree->clear();
            delete  col.tree;
        }
    }


    for(Page* page : table->pages)
    {
        freePage(page);
    }
    delete table;
}

IObuffer*  filterTable(DatabaseState *database, std::string& tableName, 
                        char* byteCode, char* msgBuffer, unsigned int bufferSize, bool inverseResult)
{
    IObuffer* buffer = createInstructionData();

    auto tableIter = database->tables.find( tableName);
    if(tableIter ==  database->tables.end())
    {
        static const char* const errMsg = "Table with specified name does not exist";
        updateStringOutputBuffer(buffer, true, errMsg);
        return buffer;
    }

    int deletedRows = filterTable(tableIter->second, byteCode, inverseResult);

    snprintf(msgBuffer, bufferSize, "Deleted %d items", deletedRows);
    updateStringOutputBuffer(buffer, false, msgBuffer);
    return buffer;
}

void freePage(Page* page)
{
    delete[] page->dataBase;
    delete page;
}

IObuffer *serialazeTable(TableState *table)
{
    IObuffer* buffer = createInstructionData();

    uint16_t maxEntrySize = table->maxEntrySize;
    unsigned int headerSize = sizeof(uint32_t) + sizeof(uint16_t);
    for(ColumnType& column : table->columns)
    {
        headerSize += sizeof(uint16_t) * 2;
        headerSize += column.columnName.size() + 1;
    };

    char *header = new char[headerSize];
    header += sizeof(uint32_t);
    *((uint16_t*) header) = (uint16_t) table->columns.size();

    char* headerCurrent = header + sizeof(uint16_t);
    for(int i=0; i < table->columns.size(); i++)
    {
        uint16_t columnMachineType = (uint16_t) table->columns[i].machineType;
        memcpy(headerCurrent, &columnMachineType, sizeof(uint16_t)); // type
        headerCurrent += sizeof(uint16_t);
        memcpy(headerCurrent, &table->columns[i].size, sizeof(uint16_t)); // max size in bytes
        headerCurrent += sizeof(uint16_t);
        memcpy(headerCurrent, table->columns[i].columnName.c_str(),table->columns[i].columnName.size() + 1); // column name
        headerCurrent += table->columns[i].columnName.size() + 1;
    }

    char* skippedPos = skipBytes(buffer, headerSize);
    char* entryBuffer = new char[maxEntrySize];
    uint32_t selectedValues = 0;
    for(int i=0; i < table->pages.size(); i++ )
    {
        Page* currentPage = table->pages[i];
        for(int j = 0; j < currentPage->entries.size(); j++)
        {
            //skip dead entries
            if( IS_DEAD( currentPage->entries[j] ) > 0 )
            {
                continue;
            }

            uint16_t currentOffset = GET_OFFSET(currentPage->entries[j] );
            char* currentEntry = currentPage->dataBase + currentOffset;
            char* currentEntryBuffer = entryBuffer;
            for(ColumnType& column : table->columns)
            {
                uint16_t copySize = column.machineType == MachineDataTypes::STRING ? 
                strlen(currentEntry + column.offset) + 1 :
                column.size;

                if(copySize > column.size)
                {
                    copySize = column.size;
                }

                emitPayload(buffer, currentEntry + column.offset, copySize);
                currentEntryBuffer+= copySize;
            }
            selectedValues++;
        }
    }
    header -= sizeof(uint32_t);
    *(uint32_t*)header = selectedValues;
    fillSkippedBytes(buffer, skippedPos, header, headerSize);

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
        printf("Unsupported data type\n");
        exit(-1);
    }
    return copySize;
}

void insertIntoPage(TableState *table, char *data, uint32_t dataSize, bool insertIntoTrees)
{
    Page* chosenPage = choosePage(table, dataSize);
    uint16_t offset = chosenPage->pageCurrent - chosenPage->dataBase ;
    EntryDescriptor entryDesc = 0;
    SET_ALIVE(entryDesc);
    SET_OFFSET(entryDesc, offset);
    chosenPage->entries.push_back(entryDesc);
    chosenPage->aliveEntries++;
    memcpy(chosenPage->pageCurrent, data, dataSize);
    chosenPage->pageCurrent += dataSize;
    table->itemCount++;

    if(!insertIntoTrees)
    {
        return;
    }
    
    for(int i=0; i < table->columns.size(); i++)
    {
        ColumnType* col = &table->columns[i];

        if(col->tree != nullptr)
        {
            int64_t data = fetchAs64BitInt(col->machineType, chosenPage->dataBase + GET_OFFSET(entryDesc) + col->offset );
            col->tree->insertValue( (char*)&data);
        }
    }

}

Page *choosePage(TableState *table, uint32_t requiredSpace)
{
    int currentPage = table->pages.size() - 1;
    uint16_t remainingSpace =  PAGE_SIZE - (table->pages[currentPage]->pageCurrent - table->pages[currentPage]->dataBase);
    if( remainingSpace >= requiredSpace)
    {
        return table->pages[currentPage];
    }

    Page* page = createPage();
    table->pages.push_back( page );
    return table->pages[currentPage + 1];
}

const ColumnType *findColumn(const TableState *table,const std::string *columnName)
{
    const vector<ColumnType>& tableColumns =  table->columns;
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
    vector<const ColumnType*> requestedColumnsTypes;
    unsigned int headerSize = sizeof(uint32_t) + sizeof(uint16_t);
    for(string& name : requestedColumns)
    {
        const ColumnType* column =  findColumn(table, &name);
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
        for(int j = 0; j < currentPage->entries.size(); j++)
        {
            uint16_t currentOffset = GET_OFFSET(currentPage->entries[j]);
            char* currentEntry = currentPage->dataBase + currentOffset;
            char* currentEntryBuffer = entryBuffer;
            for(const ColumnType* column : requestedColumnsTypes)
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

Page *createPage()
{
    Page* page= new Page();
    page->dataBase = new char[PAGE_SIZE];
    page->pageCurrent = page->dataBase;
    page->aliveEntries = 0;
    return page;
}

void markEntryAsDead(TableState *table, Page *page, int index)
{
    for(ColumnType& column : table->columns)
    {
        if(column.tree)
        {
            int64_t data = fetchAs64BitInt(column.machineType, page->dataBase + GET_OFFSET(page->entries[index]) + column.offset);
            column.tree->removeValue((char*)&data );
        }
    }


    SET_DEAD(page->entries[index]);
    table->itemCount--;
    swap(page->entries[index], page->entries[page->aliveEntries - 1] );
    page->aliveEntries--;
}

int64_t fetchAs64BitInt(const MachineDataTypes& dataType, char *dataPtr)
{
    if( dataType == MachineDataTypes::INT32 )
    {
        return *(int*)dataPtr;
    }
    else  if( dataType == MachineDataTypes::STRING )
    {
        return (int64_t)dataPtr;
    }
    return 0;
}

/*
This function locks single table
Is used iteratevile over multiple tables
deadlocks may arise, so for multiple tables
using  lockTables is adviced
if buffer is NULL request is valid
otherwise request is invalid
*/
IObuffer* lockTable(DatabaseState *database, std::string &tableName)
{
    IObuffer* buffer = checkIfTableExists(database, tableName);
    if(buffer)
    {
        return buffer;
    }
    database->locks[tableName]->lock();

    return nullptr;
}

IObuffer *checkIfTableExists(DatabaseState *database, std::string &tableName)
{
    auto tableIter = database->tables.find( tableName);
    if(tableIter ==  database->tables.end())
    {
        IObuffer* buffer = createInstructionData();
        static const char* const errMsg = "Table with specified name does not exists";
        updateStringOutputBuffer(buffer, true, errMsg);
        return buffer;
    }
    return nullptr;
}

/*
unlocks tables locked using lockTable
if buffer is NULL request is valid
otherwise request is invalid
*/
IObuffer* unlockTable(DatabaseState *database, std::string &tableName)
{
    IObuffer* buffer = checkIfTableExists(database, tableName);
    if(buffer)
    {
        return buffer;
    }
    database->locks[tableName]->unlock();
    return nullptr;
}

/*
This function can be used to lock multiple tables
If one of table locks fails, free all locks and wait 
random number of miliseconds then try again
if buffer is NULL request is valid
otherwise request is invalid
*/
IObuffer* lockTables(DatabaseState *database, std::vector<std::string> &tableNames)
{
    for(int i=0; i < tableNames.size(); i++)
    {
        IObuffer* buffer = checkIfTableExists(database, tableNames[i]);
        if(buffer)
        {
            return buffer;
        }
    }

    while (true)
    {
        int i ;
        bool isLocked;
        for(i = 0; i < tableNames.size(); i++)
        {
            isLocked = database->locks[tableNames[i]]->try_lock();
            if(!isLocked)
            {
                break;
            }
        }

        if(!isLocked)
        {
            for(int j  = 0; j <= i; j++)
            {
                database->locks[tableNames[j]]->unlock();
            }
            random_device rd;
            mt19937_64  gen{rd()};
            uniform_int_distribution<int> dist(0, 100);
            std::chrono::milliseconds duration(dist(gen));
            this_thread::sleep_for( duration );
        }
        else
        {
            return nullptr;
        }
    }

    return nullptr;
}

/*
unlocks tables locked using lockTables
if buffer is NULL request is valid
otherwise request is invalid
*/
IObuffer* unlockTables(DatabaseState *database, std::vector<std::string> &tableNames)
{
    for(int i=0; i < tableNames.size(); i++)
    {
        IObuffer* buffer = checkIfTableExists(database, tableNames[i]);
        if(buffer)
        {
            return buffer;
        }
    }

    for(int i = 0; i < tableNames.size(); i++)
    {
        database->locks[tableNames[i]]->unlock();
    }

    return nullptr;
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


pair<vector<ColumnType>, vector<string>> fetchColumnsFromCursors(const vector<Cursor>& cursors, const vector<string>& columnNames)
{   
    vector<ColumnType> out;
    vector<string> outTables;
    for(const string& colName : columnNames)
    {
        const ColumnType* columnTarget = nullptr;
        int i =0;
        for(; i < cursors.size(); i++)
        {
            // if multiple tables share name of the column
            // pick column from the first table where the name occured in
            columnTarget = findColumn(cursors[i].m_table, &colName);
            if( columnTarget != nullptr)
            {   
                break;
            }
        }
        out.push_back(*columnTarget);
        outTables.push_back(cursors[i].m_table->tableName);
    }

    return make_pair(move(out), move(outTables)) ;
}
