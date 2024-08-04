#include "vm.hpp"
#include <sys/socket.h>
#include <string.h>
#define LOCAL_BUFFER_SIZE 10000
using namespace std;

DatabaseState* VirtualMachine::databaseState = nullptr;
Operation VirtualMachine::operationTable[(unsigned int)OpCodes::DB_OP_COUNT] = 
{
    &VirtualMachine::executeCreateDatase,
    &VirtualMachine::executeInsertInto,
    nullptr, // exit is handled in execute function
    &VirtualMachine::executeSelect,
    &VirtualMachine::executeError,
    &VirtualMachine::executeFilter
};


VirtualMachine::VirtualMachine()
:
m_ip(nullptr), m_privateMemory(new char[LOCAL_BUFFER_SIZE])
{
    if(!databaseState)
    {
        databaseState = new DatabaseState();
    }
}

char *VirtualMachine::execute(const InstructionData *byteCode, int clientFd)
{
    m_ip = byteCode->base;
    m_clientFd = clientFd;
    while (true)
    {
        OpCodes op = fetchInstruction();
        if(op == OpCodes::EXIT)
        {
            return nullptr;
        }
        
        (this->*operationTable[(unsigned int) op] )( nullptr);

    }
    return nullptr;
}

OpCodes VirtualMachine::fetchInstruction()
{
    OpCodes op = (OpCodes) (*(uint16_t*) m_ip);
    m_ip += 2;
    return op;
}

uint16_t VirtualMachine::fetchUint16()
{
    uint16_t bytes = *(uint16_t*) m_ip;
    m_ip += sizeof(uint16_t);
    return bytes;
}

uint16_t VirtualMachine::fetchUint32()
{
    m_ip+=sizeof(uint32_t);
    return *(uint32_t*)(m_ip - sizeof(uint32_t));
}

ColumnType VirtualMachine::fetchColumnType()
{
    DataTypes type = (DataTypes)fetchUint16();
    switch (type)
    {
    case DataTypes::CHAR:
        {
            uint16_t size = fetchUint16();
            return ColumnType{MachineDataTypes::STRING, type,  size,  0, m_ip};
        }
    case DataTypes::INT:
        return ColumnType{MachineDataTypes::INT32, type, 4, 0, m_ip};
        
    default:
        break;
    }
    return ColumnType{MachineDataTypes::NONE, type, 0};
}

VirtualMachine::~VirtualMachine()
{
    m_ip = nullptr;
    delete[] m_privateMemory;
}

void VirtualMachine::executeCreateDatase(void*)
{
    string tableName = m_ip;
    m_ip += tableName.size() + 1;

    uint16_t argCount = fetchUint16();
    vector<ColumnType> columnTypes;
    for(int i =0; i < argCount; i++)
    {
        ColumnType type = fetchColumnType();
        type.columnName =  m_ip;
        type.tree = nullptr;
        m_ip += type.columnName.size() + 1;
        columnTypes.push_back(type);
    }

    unsigned int settersCount = columnTypes.size()/8 + 1;

    for(int i =0; i < argCount; i++)
    {
        unsigned int byteIndex = i/8;
        uint8_t bit_shift = i% 8;
        if( m_ip[byteIndex] & ( 1 << bit_shift)  )
        {
            columnTypes[i].tree = new AvlTree( columnTypes[i].machineType, columnTypes[i].size);
        }
    }
    m_ip += settersCount;

    IObuffer* buffer = createTable(databaseState,tableName, columnTypes );
    sendResponseToClient(buffer);
    freeInstructionData(buffer);
}

void VirtualMachine::executeInsertInto(void *)
{
    string tableName = m_ip;
    m_ip += tableName.size() + 1;

    uint16_t columnCount = fetchUint16();
    uint16_t argumentCount = fetchUint16();

    std::vector<string> colNames;
    std::vector<uint32_t> argOffset;
    for(uint16_t i = 0; i < argumentCount; i++)
    {
        argOffset.push_back(fetchUint32());
    }
    unsigned int offset = 0;
    lockTable(databaseState, tableName);
    IObuffer* buffer = insertIntoTable(databaseState, tableName, colNames, argOffset, m_ip, offset, m_privateMemory, LOCAL_BUFFER_SIZE);
    unlockTable(databaseState, tableName);
    m_ip += offset;
    sendResponseToClient(buffer);
    freeInstructionData(buffer);
}

void VirtualMachine::executeSelect(void *)
{
    uint16_t tableCount = *(uint16_t*)m_ip;
    m_ip += sizeof(uint16_t);

    vector<string> tableNames;
    for(int i=0; i < tableCount; i++)
    {
        string tableName = m_ip;
        m_ip += tableName.size() + 1;
        tableNames.push_back( move(tableName) );
    }

    vector<string> colNames;

    uint16_t colCount = fetchUint16();
    for(int i =0; i < colCount; i++)
    {
        colNames.emplace_back( m_ip );
        m_ip += colNames[i].size() + 1;
    }

    vector<char*> joinCodes;
    for(int i=0; i < tableCount - 1; i++)
    {
        uint32_t bytecodeSize = *(uint32_t*)m_ip;
        m_ip += sizeof(uint32_t);
        //if byte_code_size  == 1 then INNER_JOIN_ON clause is not used
        char* joinCode = bytecodeSize > 1 ? m_ip : nullptr;
        joinCodes.push_back(joinCode);
        m_ip += bytecodeSize;
    }

    TableState* subtable;

    
    if(tableCount == 1)
    {
        lockTable(databaseState, tableNames[0] );
        subtable = createSubtable(databaseState, tableNames[0], colNames);
        unlockTable(databaseState, tableNames[0] );
    }
    else
    {
        lockTables(databaseState, tableNames);
        subtable = selectAndMerge(databaseState, tableNames,  joinCodes, colNames);
        unlockTables(databaseState, tableNames);
    }

    uint32_t bytecodeSize = *(uint32_t*)m_ip;
    m_ip += sizeof(uint32_t);
    if(bytecodeSize > 0)
    {
        filterTable(subtable, m_ip);
    }
    IObuffer* buffer = serialazeTable(subtable);
    freeTable(subtable);
    sendResponseToClient(buffer);
    freeInstructionData(buffer);

    m_ip += bytecodeSize;
}

void VirtualMachine::executeError(void *)
{
    IObuffer* buffer = createInstructionData();
    updateStringOutputBuffer(buffer, true, m_ip);
    sendResponseToClient(buffer);
    freeInstructionData(buffer);

    m_ip += strlen(m_ip) + 1;
}

void VirtualMachine::executeFilter(void *)
{
    string tableName = m_ip;
    m_ip += tableName.size() + 1;

    uint32_t byteCodeSize = *(uint32_t*)m_ip;
    m_ip += sizeof(uint32_t);

    char* expression = byteCodeSize > 0 ? m_ip : nullptr;
    m_ip += byteCodeSize;

    lockTable( databaseState, tableName);
    IObuffer* buffer = filterTable(databaseState, tableName, expression, m_privateMemory, LOCAL_BUFFER_SIZE, true);
    unlockTable( databaseState, tableName);

    sendResponseToClient(buffer);
    freeInstructionData(buffer);
}

void VirtualMachine::sendResponseToClient(IObuffer *data)
{
    int n = send(m_clientFd, data->base, data->curr - data->base, 0);
}
