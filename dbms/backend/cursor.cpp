#include "cursor.hpp"
using namespace std;

Cursor::Cursor(TableState *table)
:
m_table(table), m_currentItem(0), m_currentItemPtr(0)
{

}

bool Cursor::increment()
{
    bool wrapAround = false;
    m_currentItem++;

    if(m_currentItem >= m_table->itemCount)
    {
        wrapAround = true;
        m_currentItem = 0;
    }

    return wrapAround;
}

char *Cursor::getEntry()
{
    int i = 0;
    int j = 0;
    while (i <= m_currentItem)
    {
        i += m_table->pages[j]->entries.size(); 
        j++;
    }
    j--;
    Page* page = m_table->pages[j];

    i-= page->entries.size();
    int index = m_currentItem - i;
    return page->dataBase + GET_OFFSET(page->entries[index]);
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
