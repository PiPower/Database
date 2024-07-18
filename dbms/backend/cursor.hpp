#ifndef CURSOR
#define CURSOR
#include "database_state.hpp"

class Cursor
{
public:
    Cursor(TableState* table);
private:
    TableState* table;
};


std::vector<Cursor> createListOfCursors(DatabaseState* database, const std::vector<std::string>& tableNames);
#endif