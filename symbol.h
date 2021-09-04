#ifndef   SYMBOL_H_   
#define   SYMBOL_H_   
#include<iostream>
#include<string>
#include<map>
#include<vector>
using namespace std;
class symbol{
public:
    string name;
    string type;
    int scope;
    int offset;
    symbol(string n,string t,int s,int o):name(n),type(t),scope(s),offset(o){};
}; 
#endif
