#ifndef EXPRESSION
#define EXPRESSION

#include "database_state.hpp"
#include <stack>
#include <unordered_map>

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

struct EntryBase
{
    std::string tableName;
    char* ptr;
};

typedef std::unordered_map<std::string, ColumnType*> ColumnTypeHashmap;

bool executeComparison(std::vector<ExpressionEntry>& stack, char* byteCode,
             std::vector<EntryBase> entriesBasePtr, std::unordered_map<std::string, ColumnTypeHashmap>& entryDesc);

#endif