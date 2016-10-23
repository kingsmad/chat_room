#include "terminal.h"
#include <vector>
#include <string>
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
    vector<string> v;
    getstr(v);
    if (v.size() < 1) {

        return -1;
    }

    if (v[0] == "block") {
        if (v.size() < 2)
            cout << "Correct usage: block remove/add ... " << endl;
        if (v[1] ==  "add") {
            for (int i=2; i<v.size(); ++i) {
                blockm.insert(v[i]);
                cout << "successfully blocked " << v[i] << endl;
            }
            
        } else if (v[1] == "remove") {
            for (int i=2; i<v.size(); ++i) {
                if (blockm.count(v[i]) > 0) 
                    blockm.erase(v[i]);
                cout << "successfully removed " << v[i] \
                    << " from blocklists" << endl;
            }
        } else {
            cout << "Correct usage: block remove/add ... " << endl;
        }
    } else if (v[0] == "broadcast") {
        if (v.size() < 2)
            cout << "Correct usage: broadcast file/msg ..." << endl;
        // get blocked users
        vector<char*> res;
        mblock2raw(res);
        char* p = (char*)malloc(v[2].size()+1);
        int slen;
        str2raw(v[2], p, slen);

        if (v[1] == "file") {
            if (v.size() > 3) 
                cout << "Only support 1 file 1 time" << endl;
            // send file
            client.send_file(p, slen, true, res);
        } else if (v[1] == "msg") {
            // send msg
            client.send_msg(p, slen, true, res);
        } else {
            cout << "Correct usage: broadcast file/msg ..." << endl;
        }
        free(p);
    } else if (v[0] == "unicast") {
        if (v.size() < 4 || v[3] != "to") 
            cout << "Correct usage: unicast file/msg xxx to xxx" << endl;
        // get dest-users and para v[2]
        vector<char*> res;
        str2raw(v.begin()+4, v.end(), res);
        char* p = (char*)malloc(v[2].size()+1);
        int slen;
        str2raw(v[2], p, slen);

        if (v[1] == "file")
            client.send_file(p, slen, false, res);
        else if (v[1] == "msg") 
            client.send_msg(p, slen, false, res);
        else 
            cout << "Correct usage: unicast file/msg xxx to xxx" << endl;
    } else if (v[0] == "set") {
        parse_sets(v);
    } else {
        cout << "Correct usage:\n block remove/add ...\nunicast file/msg ...\n\
            broadcast file/msg..." << endl;
    }

    return 0;
}

void Terminal::parse_sets(vector<string>& v) {
    if (v.size() < 3) 
        cout << "Correct usage: set mode client/server" << endl;
    if (v[1] == "client") {

    } else if (v[1] == "server") {

    }
}

void Terminal::mblock2raw(vector<char*>& res) {

}

void Terminal::str2raw(vector<string>::iterator p1, vector<string>::iterator p2, \
        vector<char*>& v) {
    
}

void str2raw(string s1, char* des, int& len) {

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

void Terminal::stop() {

}
