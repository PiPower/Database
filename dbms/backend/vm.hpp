#ifndef VIRTUAL_MACHINE
#define VIRUTAL_MACHINE
#include "database_state.hpp"
#include "compiler.hpp"

class VirtualMachine
{
public:
    VirtualMachine();
    char* exectue(const InstructionData* byteCode);
    OpCodes fetchInstruction();
    uint16_t fetchUint16();
    ColumnType fetchDataType();
private:
    static DatabaseState* dbState;
    char* m_ip;
};



#endif