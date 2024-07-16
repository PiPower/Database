#include "../client/client.hpp"
#include <string>
using namespace std;

int main()
{
    string msg =  "CREATE TABLE Workers(name char(34), surname char(34), age INT, id INT, partner_name char(7) );"
                 "INSERT Into Workers VALUES(\'Jan\', \'Kowalski\', 31, 1232445, \'Janina\' ), "
                 "(\'Jaroslaw\', \'Kryzewski\', 26, 32421, \'Kasia\' ),"
                 "(\'TOmasz\', \'Walczewki\', 43, 6894, \'Jola\' ),"
                 "(\'Ferdynand\', \'Kiepski\', 36, 19, \'Halina\' );"
                 "SeLect name, age, id, partner_name, surname from Workers;"
                 "SeLect name, age, id, partner_name, surname from Workers where id > 30;";

    Connection* conn = connectToDbms(0, 500);
    sendQuery(conn, msg.c_str());
    unsigned int readSize;
    std::vector<Table*> responses = readResponse(conn);
    for(Table* table : responses)
    {
        printTable(table);
        printf("************************\n");
    }
}