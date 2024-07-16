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
    &VirtualMachine::executeError
};


VirtualMachine::VirtualMachine()
:
m_ip(nullptr), m_clientFd(-1), m_privateMemory(new char[LOCAL_BUFFER_SIZE])
{
    databaseState = new DatabaseState();
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

ColumnType VirtualMachine::fetchDataType()
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

void VirtualMachine::executeCreateDatase(void*)
{
    string tableName = m_ip;
    m_ip += tableName.size() + 1;

    uint16_t argCount = fetchUint16();
    vector<ColumnType> columnTypes;
    for(int i =0; i < argCount; i++)
    {
        ColumnType type = fetchDataType();
        type.columnName =  m_ip;
        m_ip += type.columnName.size() + 1;
        columnTypes.push_back(type);
    }

    IObuffer* buffer = createTable(databaseState, move(tableName), move(columnTypes) );
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
    IObuffer* buffer = insertIntoTable(databaseState, tableName, colNames, argOffset, m_ip, offset, m_privateMemory, LOCAL_BUFFER_SIZE);
    m_ip += offset;
    sendResponseToClient(buffer);
    freeInstructionData(buffer);
}

void VirtualMachine::executeSelect(void *)
{
    string tableName = m_ip;
    m_ip += tableName.size() + 1;

    vector<string> colNames;

    uint16_t colCount = fetchUint16();
    for(int i =0; i < colCount; i++)
    {
        colNames.emplace_back( m_ip );
        m_ip += colNames[i].size() + 1;
    }
    uint32_t bytecodeSize = *(uint32_t*)m_ip;
    m_ip += sizeof(uint32_t);

    TableState* subtable = createSubtable(databaseState, move(tableName), move(colNames));
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

void VirtualMachine::sendResponseToClient(IObuffer *data)
{
    int n = send(m_clientFd, data->base, data->curr - data->base, 0);
}
