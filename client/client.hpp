#ifndef CLIENT
#define CLIENT
#include <vector>
#include <sys/time.h>

struct Connection;
struct Table;

Connection* connectToDbms(time_t seconds, suseconds_t microseconds);
void sendQuery(Connection* connection, const char* query);
std::vector<Table*> readResponse(Connection* connection);
void printTable(Table*);
#endif