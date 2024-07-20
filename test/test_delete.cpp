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
    string create_t1 = "CREATE TABLE Clients(clients_id INT, name char(50), surname char(50), purchase_count INT, total_profit INT);";
    string insert_clients = "INSERT INTO Clients VALUES "
                            "(0, 'Breanna', 'Walker', 40, 1739)," 
                            "(1, 'Maizie', 'Jones', 24, 9198)," 
                            "(2, 'Emerson', 'Jackson', 36, 4486)," 
                            "(3, 'Preston', 'Walker', 35, 3482)," 
                            "(4, 'Audrey', 'Cooper', 21, 3287)," 
                            "(5, 'Harrison', 'Hunt', 5, 5658)," 
                            "(6, 'Edward', 'Cooper', 0, 8748)," 
                            "(7, 'Jackson', 'Clarke', 33, 9654)," 
                            "(8, 'Tatum', 'Johnson', 1, 9536)," 
                            "(9, 'Emerson', 'Richards', 8, 3393)," 
                            "(10, 'Jackson', 'Parker', 28, 7166)," 
                            "(11, 'Edward', 'Young', 27, 4993)," 
                            "(12, 'Braxton', 'Hughes', 32, 3201)," 
                            "(13, 'Emerson', 'Price', 4, 9429)," 
                            "(14, 'Archie', 'Patel', 21, 5847)," 
                            "(15, 'Hailee', 'Cooper', 2, 8532)," 
                            "(16, 'Colton', 'Thomas', 12, 9457)," 
                            "(17, 'Hailee', 'Walker', 36, 7998)," 
                            "(18, 'Ivy', 'James', 8, 9437)," 
                            "(19, 'Edward', 'Ellis', 16, 8938)," 
                            "(20, 'Willow', 'Rogers', 17, 6776)," 
                            "(21, 'Ember', 'Thomas', 3, 4550)," 
                            "(22, 'Willow', 'Edwards', 6, 5970)," 
                            "(23, 'Dean', 'Wright', 27, 6899)," 
                            "(24, 'Tatum', 'Simpson', 14, 7120)," 
                            "(25, 'Harrison', 'Ali', 37, 2857)," 
                            "(26, 'Preston', 'Wilson', 23, 8347)," 
                            "(27, 'Jenny', 'Singh', 32, 9637)," 
                            "(28, 'Archie', 'Bennett', 15, 2611)," 
                            "(29, 'Heather', 'Gray', 39, 2360);"; 

    string select_query = "Select clients_id, name, surname, purchase_count, total_profit FROM Clients;";
    
    string select_query_2 = "Select clients_id, name, surname, purchase_count, total_profit FROM Clients Where purchase_count > 20;";
    string delete_query = "Delete FrOm Clients Where purchase_count <= 20;";
    string select_query_3 = "Select clients_id, name, surname, purchase_count, total_profit FROM Clients;";

    Connection* conn = connectToDbms(1000, 0);
    processQuery(create_t1, conn);
    processQuery(insert_clients, conn);
    processQuery(select_query, conn);
    processQuery(select_query_2, conn);
    processQuery(delete_query, conn);
    processQuery(select_query_3, conn);
}