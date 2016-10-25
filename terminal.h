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
    set<string> mblock_bc;
    char buf[1024];
    void getstr(vector<string>& v);
    void mblock2raw(vector<char*>& res);
    void mblock2raw(vector<char*>& res);
    void str2raw(vector<string>::iterator p1, vector<string>::iterator p2, vector<char*>& v);

private:
    int parse();
    void parseServer();
    void parseClient();
public:
    int run();

};
