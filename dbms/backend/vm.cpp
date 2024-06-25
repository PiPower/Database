#include "vm.hpp"
using namespace std;

DatabaseState* VirtualMachine::dbState = nullptr;

VirtualMachine::VirtualMachine()
:
m_ip(nullptr)
{
    dbState = new DatabaseState();
}

char *VirtualMachine::exectue(const InstructionData *byteCode)
{
    m_ip = byteCode->base;
    OpCodes op = fetchInstruction();
    switch (op)
    {
    case OpCodes::CREATE_TABLE:
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

            createTable(*dbState, move(tableName), move(columnTypes) );
        }
        break;
    
    default:
        break;
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
    m_ip += 2;
    return bytes;
}

ColumnType VirtualMachine::fetchDataType()
{
    DataTypes type = (DataTypes)fetchUint16();
    switch (type)
    {
    case DataTypes::CHAR:
        {
            uint16_t size = fetchUint16();
            return ColumnType{DataTypes::CHAR, size};
        }
    case DataTypes::INT_32:
        return ColumnType{DataTypes::INT_32, 2};
        
    default:
        break;
    }
    return ColumnType{DataTypes::NONE, 0};
}
