#ifndef TYPES
#define TYPES


enum class OpCodes
{
    CREATE_TABLE,
    INSERT,
    EXIT,
    SELECT,
    INSTRUCTION_COUNT
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
    STRING
};

#endif