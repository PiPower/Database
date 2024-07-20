#ifndef TYPES
#define TYPES


enum class OpCodes
{
    // database related ops
    CREATE_TABLE, INSERT, EXIT, SELECT, ERROR, 
    DB_OP_COUNT, INNER_JOIN, FILTER,

    // stack ops
    PUSH_IDENTIFIER, PUSH_STRING, PUSH_CONSTANT,

    //comparisons
    GREATER_EQUAL, GREATER, LESS_EQUAL, LESS, 
    EQUAL,

    // expressions flow control
    EXIT_EXPRESSION
};

enum class DataTypes
{
    NONE,
    INT,
    CHAR,
};

enum class MachineDataTypes
{
    NONE,
    INT32,
    INT64,
    STRING
};

#endif