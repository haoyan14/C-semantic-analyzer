#ifndef   SEMANTIC_ANALYZER_H_   
#define   SEMANTIC_ANALYZER_H_
#include<iostream>
#include<string>
#include<map>
#include<algorithm>
#include<cstring>
#include<vector>    
#include "symbol.h"
#include "Node.h"
#include "func.h"
using namespace std;
class semantic_analyzer{
    private:
        vector<symbol> table;
        vector<func> ftable;
        Node * root;
        int global_scope=0;
        int top=1;
        int scope_stack[100];
    public:
        semantic_analyzer(Node *root);
        void createtable(Node *root,int scope);
        void insert(Node *root,int scope,int offset);
        bool havesymbol(string name,int scope);
        string findsymbol(string name,int scope);
        void addsymbol(string name,string type,int scope,int offset);
        func finsert(Node *root,int flag);

        bool fhavesymbol(Node *root,int flag,int pnum,vector<string> ptype);
        bool fhavesymbol(string name);
        func faddsymbol(string name,string type,int flag,int pnum,vector<string> ptype);
        string findfsymbol(string name,int pnum,vector<string> ptype);
        string gettype(string t1,string t2);
        Node* getroot(){return root;};
};
#endif
