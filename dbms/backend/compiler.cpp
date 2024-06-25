#include "compiler.hpp"
#include <string.h>
using namespace std;
/*
    TO DO !!!
    add proper memory management for InstructionData
*/

void emitInstruction(OpCodes opCode, InstructionData *data)
{
    uint16_t opValue = (uint16_t) opCode;
    *(uint16_t*)data->curr = opValue;
    data->curr += 2;
}

void emitPayload(InstructionData *data, const void *payload, unsigned int payloadSize)
{
    memcpy(data->curr, payload, payloadSize);
    data->curr += payloadSize;
}

void emitInstructionWithPayload(OpCodes opCode, InstructionData *data, const void *payload, unsigned int payloadSize)
{
    emitInstruction(opCode, data);
    memcpy(data->curr, payload, payloadSize);
    data->curr += payloadSize;
}

InstructionData *compile(AstNode *query)
{
    CompilationState state;
    state.instructionData = createInstructionData();
    compileStatement(state, query);

    return state.instructionData;
}

InstructionData *createInstructionData()
{
    InstructionData* data = new InstructionData();
    data->base = new char[30000];
    data->curr = data->base;
    data->size = 30000;

    return data;
}

void compileStatement(CompilationState &state, AstNode *query)
{
    switch (query->type)
    {
    case AstNodeType::CREATE_TABLE:
        compileCreateTable(state, query);
        return;
    default:
        break;
    };
}

void serializeDataType(AstNode *type, InstructionData* byteCode)
{
    switch (type->type)
    {
    case AstNodeType::CHAR:
    {
        string* charSize = (string*)(type->child[0]->data);
        uint16_t sizeAsNumber = stoi(*charSize);
        string* name = (string*)type->child[1]->data;
        constexpr uint16_t typeID = (uint16_t)DataTypes::CHAR;
        
        emitPayload(byteCode, &typeID, sizeof(uint16_t));
        emitPayload(byteCode, &sizeAsNumber, sizeof(uint16_t));
        emitPayload(byteCode, name->c_str(), name->size() + 1);
    } return;
    case AstNodeType::INT_32 :
    {
        string* name = (string*)type->child[0]->data;
        constexpr uint16_t typeID = (uint16_t)DataTypes::INT_32;
        emitPayload(byteCode, &typeID, sizeof(uint16_t));
        emitPayload(byteCode, name->c_str(), name->size() + 1);
    } return;
    default:
        break;
    }
}

void compileCreateTable(CompilationState& state, AstNode *query)
{
    emitInstruction(OpCodes::CREATE_TABLE, state.instructionData);
    string* tableName = (string*)query->child[0]->data;
    emitPayload(state.instructionData, tableName->c_str(), tableName->size() + 1);

    uint16_t argCount = query->child.size() - 1;
    emitPayload(state.instructionData, &argCount, sizeof(uint16_t));

    for(int i = 1; i < query->child.size(); i++)
    {   
        AstNode* type = query->child[i];
        serializeDataType(type, state.instructionData);
    }
}