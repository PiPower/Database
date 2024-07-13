#ifndef EXPRESSION
#define EXPRESSION

#include "database_state.hpp"
#include <stack>
struct ExpressionEntry
{
    union
    {
        char* string;
        double f_value;
        long long int i_value;
    };
    
    MachineDataTypes type;
};


bool executeComparison(std::vector<ExpressionEntry>& stack, char* byteCode, char* entry, const std::vector<ColumnType>& entryDesc);

#endif