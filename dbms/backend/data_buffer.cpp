#include "data_buffer.hpp"
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <cstdint>
/*
    TO DO !!!
    add proper memory management for InstructionData
*/

using namespace std;
InstructionData *createInstructionData()
{
    InstructionData* data = new InstructionData();
    data->base = new char[30000];
    data->curr = data->base;
    data->size = 30000;

    return data;
}


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

char* skipBytes(InstructionData *data, unsigned int size)
{
    PlaceHolder placeHolder;
    placeHolder.pos = data->curr;
    placeHolder.size = size;
    
    data->curr += size;
    data->skippedPos.push_back( placeHolder);
    return placeHolder.pos;
}

void fillSkippedBytes(InstructionData *data, char *pos, void* payload, unsigned int size)
{
    auto elem = find_if(data->skippedPos.cbegin(), data->skippedPos.cend(),
    [pos](const PlaceHolder& placeholder)
    {
        return pos == placeholder.pos;
    } );

    if(elem == data->skippedPos.end() )
    {
        dprintf(2, "error element not found \n");
        exit(-1);
    }

    memcpy(pos, payload, size );
}   


void freeInstructionData(InstructionData *instructionData)
{
    delete[] instructionData->base;
    delete instructionData;
}

void freeInstructionDataExceptBuffer(InstructionData *instructionData)
{
    delete instructionData;
}