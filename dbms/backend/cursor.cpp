#include "cursor.hpp"
using namespace std;

Cursor::Cursor(TableState *table)
:
table(table)
{

}

std::vector<Cursor> createListOfCursors(DatabaseState* database, const vector<string>& tableNames)
{
    vector<Cursor> cursors;
    for(int i=0; i < tableNames.size(); i++)
    {
        auto tableIter = database->tables.find( tableNames[i]);
        if(tableIter ==  database->tables.end())
        {
            printf( "Table with specified name does not exist");
            exit(-1);
        }
        cursors.push_back( Cursor{tableIter->second} );
    }
    return move(cursors);
}
