#ifndef DATA_BUFFER
#define DATA_BUFFER

#include "types.hpp"
#include <vector>

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

typedef InstructionData IObuffer;

InstructionData* createInstructionData();
void freeInstructionData(InstructionData* instructionData);
void freeInstructionDataExceptBuffer(InstructionData* instructionData);
void emitInstruction(OpCodes opCode, InstructionData* data);
void emitPayload(InstructionData* data, const void* payload, unsigned int payloadSize);
void emitInstructionWithPayload(OpCodes opCode, InstructionData* data,const void* payload, unsigned int payloadSize);
char* skipBytes(InstructionData* data, unsigned int size);
void fillSkippedBytes(InstructionData* data, char* pos, void* payload, unsigned int size);

#endif