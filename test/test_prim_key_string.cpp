#include "../client/client.hpp"
#include <string>
using namespace std;


void processQuery(string& msg, Connection* conn, bool showQuery = false)
{
    sendQuery(conn, msg.c_str());
    std::vector<Table*> responses = readResponse(conn);
    for(Table* table : responses)
    {
        if(showQuery)
        {
            printf("Query: %s\n  \n", msg.c_str());
        }
        printTable(table);
        printf("************************\n");
        // TODO add free table
    }
}
int main()
{
    string create_msg = "CREATE TABLE Workers(id INT PRIMARY KEY, name char(34) PRIMARY KEY, surname char(34), age INT, partner_name char(7) );"
                 "INSERT Into Workers VALUES"
                 "(1232445, \'Jan\', \'Kowalski\', 31, \'Janina\' ), "
                 "(32421, \'Jaroslaw\', \'Kryzewski\', 26, \'Kasia\' ),"
                 "(6894, \'TOmasz\', \'Walczewki\', 43, \'Jola\' ),"
                 "(19, \'Ferdynand\', \'Kiepski\', 36, \'Halina\' );"
                 "SeLect id, name, age, partner_name, surname from Workers;";
 
    string simple_insert_msg = "INSERT Into Workers VALUES"
                  "(32421, \'Jarek\', \'Nowak\', 43, \'Jarosia\' ), "
                  "(1232, \'TOmasz\', \'Kora\', 26, \'Basia\' );";
    string select_msg = "SeLect id, name, age, partner_name, surname from Workers;";

    string delete_msg = "Delete From Workers where id > 1232;";

    string last_insert_msg = "INSERT Into Workers VALUES"
                  "(32421, \'XXX\', \'TTTYYY\', 43, \'PPPP\' ),"
                  "(2133, \'Ferdynand\', \'TTTYYY\', 43, \'PPPP\' );";

    Connection* conn = connectToDbms(100, 500);
    processQuery(create_msg, conn);
    processQuery(simple_insert_msg, conn);
    processQuery(select_msg, conn);
    processQuery(delete_msg, conn);
    processQuery(select_msg, conn);
    processQuery(simple_insert_msg, conn);
    processQuery(last_insert_msg, conn);
    processQuery(select_msg, conn);

}