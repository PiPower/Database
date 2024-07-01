#ifndef VIRTUAL_MACHINE
#define VIRUTAL_MACHINE
#include "database_state.hpp"
#include "compiler.hpp"

class VirtualMachine;
typedef void (VirtualMachine::*Operation)(void*);


class VirtualMachine
{
public:
    VirtualMachine();
    char* execute(const InstructionData* byteCode);
    OpCodes fetchInstruction();
    uint16_t fetchUint16();
    uint16_t fetchUint32();
    ColumnType fetchDataType();
private:
    // ops
    void executeCreateDatase(void*);
    void executeInsertInto(void*);
    void executeSelect(void*);
private:
    static DatabaseState* databaseState;
    char* m_ip;
    static Operation operationTable[(unsigned int)OpCodes::INSTRUCTION_COUNT];
};



#endif