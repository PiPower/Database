#include "../client/client.hpp"
#include <string>
using namespace std;

int main()
{
    string msg =  "CREATE TABLE Workers(name char(34), surname char(34), id INT, age INT, partner_name char(7) );"
                 "INSERT Into Workers VALUES(\'Jan\', \'Kowalski\', 31, 1232445, \'Janina\' ), "
                 "(\'Jaroslaw\', \'Kryzewski\', 26, 32421, \'Kasia\' ),"
                 "(\'TOmasz\', \'Walczewki\', 43, 6894, \'Jola\' ),"
                 "(\'Ferdynand\', \'Kiepski\', 36, 19, \'Halina\' );"
                 "SeLect name, age, id, partner_name, surname from Workers;";

    Connection* conn = connectToDbms(2, 500);
    sendQuery(conn, msg.c_str());
    std::vector<Table*> responses = readResponse(conn);
    for(Table* table : responses)
    {
        printTable(table);
        printf("************************\n");
        // TODO add free table
    }
}