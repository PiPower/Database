#include "expression.hpp"

using namespace std;

bool executeComparison(std::vector<ExpressionEntry>& stack, char* byteCode, 
                vector<EntryBase>& entriesBasePtr, unordered_map<string, ColumnTypeHashmap>& entryDesc)
{
    if(!byteCode)
    {
        return true;
    }
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
            char *item = nullptr;
            ColumnType* column = nullptr;

            size_t pos = name.find(".");
            if(pos == string::npos)
            {
                for(int i =0; i < entriesBasePtr.size(); i++)
                {
                    auto iter = entryDesc[entriesBasePtr[i].tableName].find(name);
                    if(iter != entryDesc[entriesBasePtr[i].tableName].end())
                    {
                        column = iter->second;
                        item = entriesBasePtr[i].ptr + column->offset;
                        break;
                    }
                }
            }
            else
            {
                string tableName = name.substr(0, pos);
                string columnName = name.substr(pos + 1, string::npos);
                column = entryDesc[tableName][columnName];
                int j =0;
                while (j < entryDesc.size())
                {
                    if( entriesBasePtr[j].tableName == tableName)
                    {
                        break;
                    }
                    j++;
                }
                item = entriesBasePtr[j].ptr + column->offset;
            }

            ExpressionEntry term;
            if(column->machineType == MachineDataTypes::INT32)
            {
                term.i_value = *(int*)item;
                term.type = MachineDataTypes::INT64;
            }
            else if(column->machineType == MachineDataTypes::STRING)
            {
                term.string = item;
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
        case OpCodes::GREATER_EQUAL:
        case OpCodes::EQUAL:
        case OpCodes::LESS:
        case OpCodes::LESS_EQUAL:
        {
            ExpressionEntry l, r;
            r = stack.back();
            stack.pop_back();
            l = stack.back();
            l.type = MachineDataTypes::INT64;
            if(opcode == OpCodes::GREATER ) { l.i_value = l.i_value > r.i_value;}
            else if(opcode == OpCodes::GREATER ) { l.i_value = l.i_value > r.i_value;}
            else if(opcode == OpCodes::GREATER_EQUAL ) { l.i_value = l.i_value >= r.i_value;}
            else if(opcode == OpCodes::EQUAL ) { l.i_value = l.i_value == r.i_value;}
            else if(opcode == OpCodes::LESS ) { l.i_value = l.i_value < r.i_value;}
            else if(opcode == OpCodes::LESS_EQUAL ) { l.i_value = l.i_value <= r.i_value;}
            stack.push_back(l);
        }break;
        case OpCodes::EXIT_EXPRESSION:
        {
            ExpressionEntry term;
            term = stack.back();
            return term.i_value > 0;
        }
        default:
            printf("usupported exepression op\n");
            exit(-1);
            break;
        }
    }
}