#include "terminal.h"
#include <vector>
#include <string>
#include <cstring>
using namespace std;

int Terminal::run() {
    while(true) {
        scanf("%s\n", buf);
        int res = parse();
        if (res == -1) break;
    }
    return 0;
}

int Terminal::parse() {
    status = 0;
    vector<string> v;
    getstr(v);
    if (v[0] == "set") {
        if (v.size() < 2) 
            cout << "Correct usage: set server or set client 'IP address' 'port#' 'client name'"<< endl;
        else if ((v[1] == "client") && (v.size() == 5)) {
            client.create_and_connect(v[2].c_str(), v[2].size(), stoi(v[3]));
            client.send_reg(v[4].c_str(), v[4].size());
        } else if (v[1] == "server") {
            server.start();
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
            cout << "Correct usage: block remove/add ... " << endl;
        }
    } else if (v[0] == "broadcast") {
        if (v.size() < 3)
            cout << "Correct usage: broadcast file/msg ..." << endl;
        // get blocked users
        vector<char*> res;
        mblock2raw(res);
        char* p = (char*)malloc(v[2].size()+1);
        int slen = v[1].size();
        if(v.size() >= 3){
            if (v[1] == "file") {
                if (v.size() > 3) 
                    cout << "Only support 1 file 1 time" << endl;
                // send file
                client.send_file(p, slen, true, res);
            } else if ((v[1] == "msg") && (v.size() == 3)) {
                // send msg
                client.send_msg(p, slen, true, res);
            } else {
                cout << "Correct usage: broadcast file/msg ..." << endl;
            }
        }
        free(p);
        for(char* c: res)
            free(c);
    } else if (v[0] == "unicast") {
        if (v.size() != 4) 
            cout << "Correct usage: unicast file/msg 'file name/message' 'receiver name...'" << endl;
        // get dest-users and para v[2]
        vector<char*> res;
        int start = 3;
        str2raw(v, res, start);
        char* p = (char*)malloc(v[2].size()+1);
        int slen = v[1].size();
        if(v.size() == 4){
            if (v[1] == "file")
                client.send_file(p, slen, false, res);
            else if (v[1] == "msg") 
                client.send_msg(p, slen, false, res);
            else 
                cout << "Correct usage: unicast file/msg 'file name/message' 'receiver name...'" << endl;
        }
        free(p);
        for(char* c: res) 
            free(c);
    } else if (v[0] == "blockcast"){
        if (v.size() !=4)
            cout << "Correct usage: blockcast file/msg 'file name/message 'block client name'" << endl;
        mblock_bc.insert(v[3]);
        vector<char*> res;
        mblock_bc2raw(res);
        char* p = (char*)malloc(v[2].size()+1);
        int slen = v[1].size();
        if(v.size() == 4){
            if(v[1] == "file"){
                client.send_file(p, slen, true, res);
            } else if (v[1] == "msg"){
                client.send_msg(p, slen, true, res);
            } else
                cout << "Correct usage: blockcast file/msg 'file name/message 'block client name'" << endl;
        }
        free(p);
        for(char* c: res)
            free(c);
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

void Terminal::str2raw(vector<string> v, vector<char*>& res, int start) {
    for(int i = start; i < v.size(); i++){
        int len = v[i].size();
        char* tmp = (char*) malloc(len+1);
        memcpy(tmp, v[i].c_str(), len);
        tmp[len] = 0;

        res.push_back(tmp);
    }
}

void Terminal::getstr(vector<string>& v) {
    int p = 0;
    while(buf[p]) {
        while(buf[p] && buf[p] == ' ') ++p;
        int q = p + 1;
        while(buf[q] && buf[q] != ' ') ++q;
        v.push_back(string(buf+p, q-p));
        p = q;
    }
    return;
}
