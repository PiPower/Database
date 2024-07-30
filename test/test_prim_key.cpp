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
    string msg = "CREATE TABLE Workers(id INT PRIMARY KEY, name char(34), surname char(34), age INT, partner_name char(7) );"
                 "INSERT Into Workers VALUES"
                 "(1232445, \'Jan\', \'Kowalski\', 31, \'Janina\' ), "
                 "(32421, \'Jaroslaw\', \'Kryzewski\', 26, \'Kasia\' ),"
                 "(6894, \'TOmasz\', \'Walczewki\', 43, \'Jola\' ),"
                 "(19, \'Ferdynand\', \'Kiepski\', 36, \'Halina\' );"
                 "SeLect id, name, age, partner_name, surname from Workers;";
 
    string msg2 = "INSERT Into Workers VALUES"
                  "(32421, \'Jarek\', \'Nowak\', 43, \'Jarosia\' ), "
                  "(1232, \'Janel\', \'Kora\', 26, \'Basia\' );";
    string msg3 = "SeLect id, name, age, partner_name, surname from Workers;";

    Connection* conn = connectToDbms(100, 500);
    processQuery(msg, conn);
    processQuery(msg2, conn);
    processQuery(msg3, conn);
}