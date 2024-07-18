#ifndef CURSOR
#define CURSOR
#include "database_state.hpp"

class Cursor
{
public:
    Cursor(TableState* table);
    bool increment();
public:
    TableState* m_table;
private:
    int m_currentItem;
    char* m_currentItemPtr;
};


std::vector<Cursor> createListOfCursors(DatabaseState* database, const std::vector<std::string>& tableNames);
#endif