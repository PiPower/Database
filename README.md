It is sql based database that can support many connections in parallel.
Client api is exposed, as a way to transform returned blob into usefull information.

Example of creating connection to dabase
```c++
     Connection* conn = connectToDbms(100, 500);
```
Let's define four queries

```c++
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
```
In order to execture them on databe all it takes is to call
```c++
    sendQuery(conn, create_t1.c_str());
    responses = readResponse(conn);
    sendQuery(conn, create_t2.c_str());
    responses = readResponse(conn);
    sendQuery(conn, insert_orders.c_str());
    responses = readResponse(conn);
    sendQuery(conn, insert_clients.c_str());
    responses = readResponse(conn);
```

Database support also more complex operations like joins
Belowe image represents result of query

```sql
Query: SELECT order_id, client_id, product_name, client_name, client_surname FROM Orders INNER JOIN Clients ON Orders.client_id = Clients.client_id WHERE order_id > 5000; 
```

<img width="300"  src="https://github.com/PiPower/Database/blob/master/db.png">