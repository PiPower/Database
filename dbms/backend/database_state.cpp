#include "database_state.hpp"
using namespace std;

void createTable(DatabaseState& database, std::string&& tableName, std::vector<ColumnType> &&columns)
{
    TableState* table = new TableState();
    table->columns = move(columns);
    table->pages.push_back( new Page() );
    database.tables[move(tableName)] = table;
}