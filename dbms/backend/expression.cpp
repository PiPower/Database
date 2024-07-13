#include "expression.hpp"

using namespace std;

bool executeComparison(std::vector<ExpressionEntry>& stack, char* byteCode, char* entry, const std::vector<ColumnType>& entryDesc)
{
    while(true)
    {
        OpCodes opcode = (OpCodes) *(uint16_t*)byteCode;
        byteCode += sizeof(uint16_t);
        switch (opcode)
        {
        case OpCodes::PUSH_IDENTIFIER :
        {
            string name = byteCode; 
            byteCode += name.size() + 1;
            const ColumnType* column = nullptr;
            for(int i = 0; i < entryDesc.size(); i++)
            {
                if(name == entryDesc[i].columnName)
                {
                    column = &entryDesc[i];
                    break;
                }
            }

            ExpressionEntry term;
            if(column->machineType == MachineDataTypes::INT32)
            {
                term.i_value = *(int*)(entry + column->offset);
                term.type = MachineDataTypes::INT64;
            }
            else if(column->machineType == MachineDataTypes::STRING)
            {
                term.string = entry + column->offset;
                term.type = MachineDataTypes::STRING;
            }
            stack.push_back(move(term));

        }break;
        case OpCodes::PUSH_CONSTANT :
        {
            ExpressionEntry term;
            term.type = MachineDataTypes::INT64;
            term.i_value =  *(long long int*)byteCode; 
            byteCode += sizeof(long long int);
            stack.push_back( move(term) );
        }break;
        case OpCodes::GREATER:
        {
            ExpressionEntry l, r;
            r = stack.back();
            stack.pop_back();
            l = stack.back();
            l.i_value = l.i_value > r.i_value;
            l.type = MachineDataTypes::INT64;
            stack.push_back(l);
        }break;
        case OpCodes::EXIT_EXPRESSION:
        {
            ExpressionEntry term;
            term = stack.back();
            return term.i_value > 0;
        }
        default:
            break;
        }
    }
}