#include "../client/client.hpp"
#include <string>
using namespace std;

void processQuery(string& msg, Connection* conn)
{
    sendQuery(conn, msg.c_str());
    std::vector<Table*> responses = readResponse(conn);
    for(Table* table : responses)
    {
        printTable(table);
        printf("************************\n");
        // TODO add free table
    }

}

int main()
{
    string create_t1 = "CREATE TABLE Orders(order_id INT, client_id INT, product_name char(40));";
    string create_t2 = "CREATE TABLE Clients(client_id INT, client_name char(25), client_surname char(30));";
    string insert_orders = "INSERT INTO Orders VALUES"
                           "(10308, 1, \'smartphone samsung\'),"
                           "(42381, 5, \'LG tv\'),"
                           "(3472, 3, \'Playstation 5\'),"
                           "(3, 1, \'Personal Computer\');";
    string insert_clients = "INSERT INTO Clients VALUES"
                            "(1, \'Jan\', \'Kowalski\'),"
                            "(2, \'Monika\', \'Nowak\'),"
                            "(3, \'Thomas\', \'Jenkins\'),"
                            "(4, \'Peter\', \'Reltih\'),"
                            "(5, \'Angelina\', \'Nilats\');";
    string select_1 = "Select product_name, order_id, product_name, client_id from Orders;";
    string select_2 = "select client_id, client_name, client_surname from Clients;";
    string inner_join = "SELECT order_id, client_id, product_name, client_name, client_surname FROM Orders "
                    "INNER JOIN Clients ON Orders.client_id = Clients.client_id;";
    string inner_join_where = "SELECT order_id, client_id, product_name, client_name, client_surname FROM Orders "
                    "INNER JOIN Clients ON Orders.client_id = Clients.client_id WHERE order_id > 5000; ";


    Connection* conn = connectToDbms(1000, 0);
    processQuery(create_t1, conn);
    processQuery(create_t2, conn);
    processQuery(insert_orders, conn);
    processQuery(insert_clients, conn);
    processQuery(select_1, conn);
    processQuery(select_2, conn);
    processQuery(inner_join, conn);
    processQuery(inner_join_where, conn);
}