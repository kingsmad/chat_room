#include <iostream>
#include <set>
#include <string>
#include "client.h"
#include "server.h"

class Terminal {
private:
    Client client;
    Server server;
    set<string> mblock;
    char buf[1024];
    void getstr(vector<string>& v);
private:
    void parse();
public:
    void run();
};
