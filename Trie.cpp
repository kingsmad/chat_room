/*************************************************************************
 *     > File Name: D.cpp
 ************************************************************************/
#include <cstdio>
#include <iostream>
#include <cmath>
#include <stack>
#include "Trie.h"
#include <queue>
#include <climits>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cstring>
#include <algorithm>
using namespace std;
#define per(i,a,n) for(int i=n-1;i>=a;--i)
#define rep(i,a,n) for(int i=a;i<n;++i)
#define erep(i,a,n) for(int i=a;i<=n;++i)
#define fi first
#define se second
#define all(x) (x).begin(), (x).end()
#define pb push_back
#define mp make_pair
typedef long long ll;
typedef vector<int> VI;
typedef pair<int, int> PII;
#define lc(o) (o<<1)
#define rc(o) (o<<1|1)
ll powmod(ll a,ll b, ll MOD) {ll res=1;a%=MOD;for(;b;b>>=1){if(b&1)res=res*a%MOD;a=a*a%MOD;}return res;}
/*----------- head-----------*/
const int maxq = 2e3 + 10;
const int maxnode = maxq * 32;
const int sigma_size = 26;

void bprintf(char* s, int x) {
    fill(s, s+32, '0');
    char* p = s+31;      
    while(x) { *p-- = (x&1) ? '1':'0'; x=x>>1;}
}

struct Trie {
    int ch[maxnode][sigma_size];
    int val[maxnode], fds[maxnode], sz;
    void clear() { sz = 1; memset(ch[0], 0, sizeof(ch[0]));}
    int idx(char c) { return c - '0';}

    void insert(char* s, int len, int fd) {
        int u = 0;
        rep(i, 0, len) {
            int c = idx(s[i]);
            if (!ch[u][c]) {
                memset(ch[sz], 0, sizeof(ch[sz]));
                ch[u][c] = sz++;
            }
            u = ch[u][c];
            ++val[u];
        }
        fds[u] = fd; 
    }

    int find(char* s, int len) {
        int t = 0, u = 0;
        rep(i, 0, 32) {
            int c = idx(s[i]);
            if (!ch[u][c]) return -1;
            u = ch[u][c];
        }
        return fds[u];
    }

    void del(char* s, int len) {
        int u = 0;
        rep(i, 0, len) {
            int c = idx(s[i]);
            int tu = ch[u][c]; 
            if (--val[ch[u][c]]<=0) ch[u][c]=0;
            u = tu;
        }
        fds[u] = -1;
    }
};

