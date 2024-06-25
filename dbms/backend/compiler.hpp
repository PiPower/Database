#ifndef COMPILER
#define COMPILER

#include "../parser/parser.hpp"

enum class OpCodes
{
    CREATE_TABLE
};

enum class DataTypes
{
    NONE,
    INT_32,
    CHAR
};

struct InstructionData
{
    char *base;
    char* curr;
    unsigned int size;
};

struct CompilationState
{
    InstructionData* instructionData;
};



InstructionData* createInstructionData();
void emitInstruction(OpCodes opCode, InstructionData* data);
void emitPayload(InstructionData* data, const void* payload, unsigned int payloadSize);
void emitInstructionWithPayload(OpCodes opCode, InstructionData* data,const void* payload, unsigned int payloadSize);


InstructionData* compile(AstNode* query);
void compileCreateTable(CompilationState& state, AstNode *query);
void compileStatement(CompilationState& state, AstNode *query);


void serializeDataType(AstNode* type, InstructionData* byteCode);
#endif