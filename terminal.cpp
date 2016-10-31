#include "terminal.h"
#include "common.h"
#include <vector>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstring>
#include <string.h>

using namespace std;

extern char* str2raw(string& s);

Terminal::Terminal() {
    status = 0;
}

int Terminal::run() {
/*#ifdef ROACH_ONLINE_JUDGE
    freopen("inter_test.txt", "r", stdin);
#endif*/
    while(true) {
        string line;
        getline(cin, line, '\n');

        int res = parse(line);

        if (res == -1) break;
    }
    return 0;
}

int Terminal::parse(string line) {
    stringstream ss(line);
    string tmp; vector<string> v;
    while(ss >> tmp) v.push_back(tmp);

    if (v[0] == "set") {
        if (v.size() < 2) 
            cout << "Correct usage: set server or set client 'IP address' 'port#' 'client name'"<< endl;
        else if ((v[1] == "client") && (v.size() == 5)) {
            client.create_and_connect(v[2].c_str(), v[2].size(), stoi(v[3]));
            client.send_reg(v[4].c_str(), v[4].size());
            status = 2;
        } else if (v[1] == "server") {
            server.start();
            status = 1;
        } else {
            cout << "Correct usage: set server or set client 'IP address' 'port#' 'client name'" << endl;
        }
    } else if(status == 1){
        if(v[0] == "stop"){
            server.stop();
            return -1;
        }
    } else if(status == 2){
        if(v[0] == "stop"){
            return -1;
        }
        parseClient(v);
    } else{
        cout << "wrong input" << endl;
    }
    return 0;
}

void Terminal::parseClient(vector<string>& v) {
    if (v[0] == "block") {
        if (v.size() < 3)
            cout << "Correct usage: block remove/add ... " << endl;
        else if (v[1] ==  "add") {
            for (int i=2; i<v.size(); ++i) {
                mblock.insert(v[i]);
                cout << "successfully blocked " << v[i] << endl;
            } 
        } else if (v[1] == "remove") {
            for (int i=2; i<v.size(); ++i) {
                if (mblock.count(v[i]) > 0) 
                    mblock.erase(v[i]);
                cout << "successfully removed " << v[i]
                    << " from blocklists" << endl;
            }
        } else {
            cout << "Correct usage: block remove/add 'username' " << endl;
        }
    } else if (v[0] == "broadcast") {
        if (v.size() < 3)
            cout << "Correct usage: broadcast file/msg ..." << endl;
        if (v[1] == "file" && v.size() > 3) 
            cout << "Only support 1 file 1 time" << endl;
        if (v[1] == "file" || (v[1] == "msg" && v.size()==3)) {
            if (broadcast_hdl(v) < 0)
                cout << "Failed on broadcast" << endl;
        }
        else 
            cout << "Correct usage: broadcast file/msg ..." << endl;
    } else if (v[0] == "unicast") {
        if (v.size()<4 || (v[1] != "msg" && v[1] != "file")) 
            cout << "Correct usage: unicast file/msg 'file name/message' 'receiver name...'" << endl;
        if (unicast_hdl(v) < 0) 
            cout << "Failed on unicast" << endl;
    } else if (v[0] == "blockcast"){
        if (v.size()<4 || (v[1] !="msg" && v[1]!="file"))
            cout << "Correct usage: blockcast file/msg 'file name/message 'block client name'" << endl;
        if (blockcast_hdl(v) < 0) 
            cout << "Failed on blockcast" << endl;
    }
}

void Terminal::mblock2raw(vector<char*>& res) {
    for (set<string>::iterator itor = mblock.begin(); itor != mblock.end(); ++itor){
        int len = itor->size();
        char* tmp = (char*)malloc(len+1);
        copy(itor->c_str(), itor->c_str()+len, tmp);
        tmp[len] = 0;

        res.push_back(tmp);
    }
}

void Terminal::mblock_bc2raw(vector<char*>& res) {
    for (set<string>::iterator itor = mblock_bc.begin(); itor != mblock.end(); ++itor){
        int len = itor->size();
        char* tmp = (char*)malloc(len+1);
        copy(itor->c_str(), itor->c_str()+len, tmp);
        tmp[len] = 0;

        res.push_back(tmp);
    }
}

int Terminal::unicast_hdl(vector<string>& v) {
    // write msg context / file name 
    char* msg = str2raw(v[2]);

    // write namelists
    vector<char*> nlist;
    for(int i=3; i<v.size(); ++i)
        nlist.push_back(str2raw(v[i])); 
    
    if (v[1] == "file")
        return client.send_file(msg, strlen(msg), false, nlist);  
    else if (v[1] == "msg") 
        return client.send_msg(msg, strlen(msg), false, nlist); 

    return -1;
}

int Terminal::blockcast_hdl(vector<string>& v) {
    // write msg context / file name
    char* msg = str2raw(v[2]);
    
    // write namelists
    vector<char*> nlist;
    for (int i=3; i<v.size(); ++i) 
        nlist.push_back(str2raw(v[i]));

    if (v[1] == "file") 
        return client.send_file(msg, strlen(msg), true, nlist);
    else if (v[1] == "msg")
        return client.send_msg(msg, strlen(msg), true, nlist);

    return -1;
}

int Terminal::broadcast_hdl(vector<string>& v) {
    // get blocked users
    vector<char*> res;
    for(string s: mblock)
        res.push_back(str2raw(s));

    char* ctx = str2raw(v[2]);

    if (v[1] == "file") 
        return client.send_file(ctx, strlen(ctx), true, res);
    else if (v[1] == "msg") 
        return client.send_msg(ctx, strlen(ctx), true, res);

    return -1;
}
