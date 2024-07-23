#include "compiler.hpp"
#include <algorithm>
#include <math.h>
#include <string.h>
#define BYTE_SIZE 8
using namespace std;

std::vector<MachineDataTypes> inferMachineDataTypes(AstNode* args)
{
    std::vector<MachineDataTypes> types;
    for(AstNode* arg : args->child)
    {
        switch (arg->type)
        {
        case AstNodeType::CONSTANT:
            types.push_back(MachineDataTypes::INT32);
            break;
        case AstNodeType::STRING:
            types.push_back(MachineDataTypes::STRING);
            break;
        default:
            break;
        }
    }
    return types;
}

InstructionData* compile(const vector<AstNode*>& queries)
{
    CompilationState state;
    state.instructionData = createInstructionData();
    for(AstNode* query : queries)
    {
        compileStatement(state, query);
    }
    emitInstruction(OpCodes::EXIT, state.instructionData);
    return state.instructionData;
}

void compileStatement(CompilationState &state, AstNode *query)
{
    switch (query->type)
    {
    case AstNodeType::CREATE_TABLE:
        compileCreateTable(state, query);
        return;
    case AstNodeType::INSERT:
        compileInsert(state, query);
        return;
    case AstNodeType::SELECT:
        compileSelect(state, query);
        return;
    case AstNodeType::ERROR:
        compileError(state, query);
        return;
    case AstNodeType::DELETE:
        compileDelete(state, query);
        return;
    default:
        break;
    };
}

void compileError(CompilationState &state, AstNode *query)
{
    string* errorMsg = (string*)query->data;
    emitInstructionWithPayload(OpCodes::ERROR, state.instructionData, errorMsg->c_str(), errorMsg->size() + 1 );
}

void compileDelete(CompilationState &state, AstNode *query)
{
    string* tableName = (string*) query->child[0]->data;
    emitInstructionWithPayload(OpCodes::FILTER, state.instructionData, tableName->c_str(), tableName->size() + 1);

      // compile where expresison
    auto whereIter = find_if(query->child.begin(), query->child.end(), [](const AstNode* node){
        return node->type == AstNodeType::WHERE;
    });

    if(whereIter != query->child.end())
    {
        compileExpression(state, (*whereIter)->child[0]);
    }
    else
    {
        uint32_t size = 0;
        emitPayload(state.instructionData, &size, sizeof(uint32_t));
    }
}

void compileExpression(CompilationState &state, AstNode *query)
{
    char* pos = skipBytes(state.instructionData, sizeof(uint32_t));
    char* oldPos = state.instructionData->curr;

    compileOps(state, query);
    emitInstruction(OpCodes::EXIT_EXPRESSION, state.instructionData);
    
    uint32_t size = state.instructionData->curr - oldPos;
    fillSkippedBytes(state.instructionData, pos, &size, sizeof(uint32_t));
}

void compileOps(CompilationState &state, AstNode *query)
{
    OpCodes code;
    switch (query->type)
    {
    case AstNodeType::GREATER:
    case AstNodeType::GREATER_EQUAL:
    case AstNodeType::LESS:
    case AstNodeType::LESS_EQUAL:
    case AstNodeType::EQUAL:
        compileOps(state, query->child[0]);
        compileOps(state, query->child[1]);
        code = (OpCodes)( (uint16_t)query->type -  (uint16_t)AstNodeType::GREATER_EQUAL + (uint16_t)OpCodes::GREATER_EQUAL) ;
        emitInstruction(code , state.instructionData );
        break;
    case AstNodeType::CONSTANT:
    {
        long long int x = stoi(*(string*)query->data );
        emitInstructionWithPayload(OpCodes::PUSH_CONSTANT, state.instructionData, &x, sizeof(long long int));
    }break;
    case AstNodeType::STRING:
    case AstNodeType::IDENTIFIER:
    {
        code = query->type == AstNodeType::STRING? OpCodes::PUSH_STRING : OpCodes::PUSH_IDENTIFIER;
        string* literal = (string*) query->data;
        emitInstructionWithPayload(code, state.instructionData, literal->c_str(), literal->size() + 1);
    }break;
    case AstNodeType::TABLE_SPEC:
    {
        code = query->type == AstNodeType::STRING? OpCodes::PUSH_STRING : OpCodes::PUSH_IDENTIFIER;
        string literal = *(string*) query->data + "." + *(string*)query->child[0]->data;
        emitInstructionWithPayload(code, state.instructionData, literal.c_str(), literal.size() + 1);
    }break;
    default:
        break;
    }
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
        constexpr uint16_t typeID = (uint16_t)DataTypes::INT;
        emitPayload(byteCode, &typeID, sizeof(uint16_t));
        emitPayload(byteCode, name->c_str(), name->size() + 1);
    } return;
    default:
        break;
    }
}

unsigned int serializeMachineDataType(const MachineDataTypes &type, AstNode* value, InstructionData *byteCode)
{
    int outSize = 0 ;
    switch (type)
    {
    case MachineDataTypes::INT32 :
        {
            int valueBinary = stoi( *(string*)(value->data) );
            uint16_t type = (uint16_t) MachineDataTypes::INT32;
            emitPayload(byteCode, &type, sizeof(uint16_t));
            emitPayload(byteCode, &valueBinary, sizeof(int));
            outSize = sizeof(int) + sizeof(uint16_t);
        }return outSize;
    case MachineDataTypes::STRING :
        {
            string*  storedString = (string*)(value->data) ;
            outSize = storedString->size() + 1 + sizeof(uint16_t);
            uint16_t type = (uint16_t) MachineDataTypes::STRING;
            emitPayload(byteCode, &type, sizeof(uint16_t));
            emitPayload(byteCode, storedString->c_str(), storedString->size() + 1);
        }return outSize;
    default:
        break;
    }

    return outSize;
}

bool typesPossiblyCastable(const MachineDataTypes &type1, const MachineDataTypes &type2)
{
    if (type1 == type2)
    {
        return true;
    }
    return false;
}

void compileCreateTable(CompilationState& state, AstNode *query)
{
    emitInstruction(OpCodes::CREATE_TABLE, state.instructionData);
    string* tableName = (string*)query->child[0]->data;
    emitPayload(state.instructionData, tableName->c_str(), tableName->size() + 1);

    uint16_t argCount = query->child.size() - 1;
    emitPayload(state.instructionData, &argCount, sizeof(uint16_t));


    const unsigned int byteCount = argCount/BYTE_SIZE + 1;
    char* primKeySetters = new char[ byteCount ];
    memset(primKeySetters, 0, byteCount);
    for(int i = 1; i < query->child.size(); i++)
    {   
        AstNode* type = query->child[i];
        serializeDataType(type, state.instructionData);
        
        if(type->child.size() > 1 && (type->child[1]->type == AstNodeType::PRIMARY_KEY) )
        {
            unsigned int byteIndex = (i-1)/BYTE_SIZE;
            uint8_t bit_shift = (i-1)% BYTE_SIZE;
            primKeySetters[byteIndex] |= (char)0x01 << bit_shift;
        }
    }
    emitPayload(state.instructionData, primKeySetters, byteCount);
}

void compileInsert(CompilationState &state, AstNode *query)
{
    vector<MachineDataTypes> machineTypes =  inferMachineDataTypes(query->child[2]);
    if(query->child[1])
    {
        // add support for custom column ordering
    }
    string* tableName = (string*)query->child[0]->data;
    emitInstructionWithPayload(OpCodes::INSERT, state.instructionData, tableName->c_str(), tableName->size() + 1);
    // header
    unsigned int memorySize = sizeof(uint16_t) + sizeof(uint16_t) + (query->child.size()-2) * sizeof(uint32_t);
    uint16_t* header = (uint16_t*) new uint8_t[memorySize];
    uint16_t* header_curr = header;
    *header_curr = 0;
    header_curr++;
    *header_curr = query->child.size()-2;
    header_curr++;

    char* placeholder = skipBytes(state.instructionData, memorySize);
    uint32_t offset = 0;
    for(int i = 2; i < query->child.size(); i++)
    {
        AstNode* row =  query->child[i];
        vector<MachineDataTypes> rowMachineTypes = inferMachineDataTypes( row);

        memcpy(header_curr, &offset, sizeof(uint32_t));
        header_curr+=2;
        for(int j =0; j < row->child.size(); j++)
        {
            AstNode* arg = row->child[j];
            if(!typesPossiblyCastable(rowMachineTypes[j], machineTypes[j]) )
            {
                // trigger error
            }
            offset += serializeMachineDataType(rowMachineTypes[j], arg, state.instructionData);

        }
    }

    fillSkippedBytes(state.instructionData, placeholder, header, memorySize);
}

void compileSelect(CompilationState &state, AstNode *query)
{
    uint16_t tableCount = query->child[1]->child.size();
    emitInstructionWithPayload(OpCodes::SELECT, state.instructionData, &tableCount, sizeof(uint16_t));
    for (size_t i = 0; i < tableCount; i++)
    {
        string* tableName = (string*)query->child[1]->child[i]->data;
        emitPayload( state.instructionData, tableName->c_str(), tableName->size() + 1);
    }
    
    uint16_t colCount = query->child[0]->child.size();
    emitPayload(state.instructionData, &colCount, sizeof(uint16_t));
    for(AstNode* column: query->child[0]->child)
    {
        string* colName = (string*)column->data;
        emitPayload(state.instructionData, colName->c_str(), colName->size() + 1);
    }
    // compile inner Join expresison
    auto innerJoinIter = find_if(query->child.begin(), query->child.end(), [](const AstNode* node){
        return node->type == AstNodeType::INNER_JOIN_ON;
    });

    if(innerJoinIter != query->child.end())
    {
        compileExpression(state, (*innerJoinIter)->child[0]);
    }

    // compile where expresison
    auto whereIter = find_if(query->child.begin(), query->child.end(), [](const AstNode* node){
        return node->type == AstNodeType::WHERE;
    });

    if(whereIter != query->child.end())
    {
        compileExpression(state, (*whereIter)->child[0]);
    }
    else
    {
        uint32_t size = 0;
        emitPayload(state.instructionData, &size, sizeof(uint32_t));
    }
}
