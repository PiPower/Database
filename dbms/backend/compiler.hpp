#ifndef COMPILER
#define COMPILER

#include "../parser/parser.hpp"

enum class OpCodes
{
    CREATE_TABLE,
    INSERT,
    EXIT,
    SELECT,
    INSTRUCTION_COUNT
};

enum class DataTypes
{
    NONE,
    INT,
    CHAR,
};

enum class MachineDataTypes
{
    NONE,
    INT32,
    STRING
};

struct PlaceHolder
{
    char* pos;
    unsigned int size;
};

struct InstructionData
{
    char *base;
    char* curr;
    unsigned int size;
    std::vector<PlaceHolder> skippedPos;
};

struct CompilationState
{
    InstructionData* instructionData;
};

InstructionData* createInstructionData();
void emitInstruction(OpCodes opCode, InstructionData* data);
void emitPayload(InstructionData* data, const void* payload, unsigned int payloadSize);
void emitInstructionWithPayload(OpCodes opCode, InstructionData* data,const void* payload, unsigned int payloadSize);
char* skipBytes(InstructionData* data, unsigned int size);
void fillSkippedBytes(InstructionData* data, char* pos, void* payload, unsigned int size);

InstructionData* compile(const std::vector<AstNode*>& queries);
void compileCreateTable(CompilationState& state, AstNode *query);
void compileInsert(CompilationState& state, AstNode *query);
void compileSelect(CompilationState& state, AstNode *query);
void compileStatement(CompilationState& state, AstNode *query);

std::vector<MachineDataTypes> inferMachineDataTypes(AstNode* args);
void serializeDataType(AstNode* type, InstructionData* byteCode);
unsigned int serializeMachineDataType(const MachineDataTypes& type, AstNode* value, InstructionData* byteCode);
bool typesPossiblyCastable(const MachineDataTypes& type1,const MachineDataTypes& type2);
#endif