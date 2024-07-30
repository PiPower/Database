#ifndef VIRTUAL_MACHINE
#define VIRUTAL_MACHINE
#include "database_state.hpp"
#include "../parser/compiler.hpp"

class VirtualMachine;
typedef void (VirtualMachine::*Operation)(void*);


class VirtualMachine
{
public:
    VirtualMachine();
    char* execute(const InstructionData* byteCode, int clientFd);
    OpCodes fetchInstruction();
    uint16_t fetchUint16();
    uint16_t fetchUint32();
    ColumnType fetchColumnType();
private:
    // ops
    void executeCreateDatase(void*);
    void executeInsertInto(void*);
    void executeSelect(void*);
    void executeError(void*);
    void executeFilter(void*);

    void sendResponseToClient(IObuffer* data);
private:
    static DatabaseState* databaseState;
    char* m_ip;
    int m_clientFd;
    static Operation operationTable[(unsigned int)OpCodes::DB_OP_COUNT];
    char* m_privateMemory;
};



#endif