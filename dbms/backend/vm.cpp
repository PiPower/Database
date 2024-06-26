#include "vm.hpp"
using namespace std;

DatabaseState* VirtualMachine::databaseState = nullptr;
Operation VirtualMachine::operationTable[(unsigned int)OpCodes::INSTRUCTION_COUNT] = 
{
    &VirtualMachine::executeCreateDatase,
    &VirtualMachine::executeInsertInto,
    nullptr, // exit is handled in execute function
    &VirtualMachine::executeSelect
};


VirtualMachine::VirtualMachine()
:
m_ip(nullptr)
{
    databaseState = new DatabaseState();
}

char *VirtualMachine::execute(const InstructionData *byteCode)
{
    m_ip = byteCode->base;
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
            return ColumnType{MachineDataTypes::STRING, type,  size};
        }
    case DataTypes::INT:
        return ColumnType{MachineDataTypes::INT32, type, 4};
        
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

    createTable(databaseState, move(tableName), move(columnTypes) );
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
    insertIntoTable(databaseState, tableName, colNames, argOffset, m_ip, offset);
    m_ip += offset;

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
    IObuffer* buffer = selectFromTable(databaseState, move(tableName), move(colNames));
}
