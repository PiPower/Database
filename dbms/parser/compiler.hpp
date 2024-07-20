#ifndef COMPILER
#define COMPILER

#include "../parser/parser.hpp"
#include "../backend/types.hpp"
#include "../backend/data_buffer.hpp"

struct CompilationState
{
    InstructionData* instructionData;
};


InstructionData* compile(const std::vector<AstNode*>& queries);
void compileCreateTable(CompilationState& state, AstNode *query);
void compileInsert(CompilationState& state, AstNode *query);
void compileSelect(CompilationState& state, AstNode *query);
void compileStatement(CompilationState& state, AstNode *query);
void compileError(CompilationState& state, AstNode *query); 
void compileDelete(CompilationState& state, AstNode *query);

void compileExpression(CompilationState& state, AstNode *query);
void compileOps(CompilationState& state, AstNode *query);

std::vector<MachineDataTypes> inferMachineDataTypes(AstNode* args);
void serializeDataType(AstNode* type, InstructionData* byteCode);
unsigned int serializeMachineDataType(const MachineDataTypes& type, AstNode* value, InstructionData* byteCode);
bool typesPossiblyCastable(const MachineDataTypes& type1,const MachineDataTypes& type2);
#endif