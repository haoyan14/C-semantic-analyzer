#include "semantic_analyzer.h"
semantic_analyzer::semantic_analyzer(Node *root){
    this->root=root;
    memset(scope_stack,sizeof(scope_stack),0);
}
void semantic_analyzer::createtable(Node *root,int scope){
    //program -> headers programs
    if(root->val=="program"){
        for(auto &a:root->sons){
            createtable(a,scope);
        }
    }
    //headers -> header headers | e
    else if(root->val=="headers"){
        if(root->sons.size()!=0)
        for(auto &a:root->sons){
            createtable(a,scope);
        }
        else return;
    }
    //header -> # include < id > 
    else if(root->val=="header"){
        return;
    }
    //programs -> program1 programs | e | branch programs  | assignment ; programs  | loop programs  | jump ; programs 
    else if(root->val=="programs"){
        if(root->sons.size()!=0)
        for(auto &a:root->sons){
            createtable(a,scope);
        }
        else return;
    }
    //program1 -> type para0 
    else if(root->val=="program1"){
        createtable(root->sons[0],scope);
        root->sons[1]->type=root->sons[0]->type;
        createtable(root->sons[1],scope);
    }
    //type -> int | char | bool | double | float | short | long  
    else if(root->val=="type"){
        root->type=root->sons[0]->val;
    }
    //para0->id para1
    else if(root->val=="para0"){
        root->sons[0]->type=root->type;
        root->sons[1]->name=root->sons[0]->name;
        root->sons[1]->type=root->type;
        createtable(root->sons[1],scope);
    }
    //para1 -> idassignlist2 idlist3 ; | ( paralist ) para2
    else if(root->val=="para1"){
        if(root->sons[0]->val=="idassignlist2"){
            root->sons[0]->type=root->type;
            root->sons[0]->name=root->name;
            createtable(root->sons[0],scope);
            root->sons[1]->type=root->type;
            createtable(root->sons[1],scope);
        }
        else{
            root->sons[1]->type="funcpara";
            createtable(root->sons[1],scope);
            int flag;
            if(root->sons[3]->sons.size()==1)flag=0;
            else flag=1;
            func temp=finsert(root,flag);
            createtable(root->sons[3],scope);
        }
    }
    // para2 -> ; | { sentences }
    else if(root->val=="para2"){
        if(root->sons.size()==1){
            return;
        }
        else {
            scope_stack[top++]=scope;
            scope=++global_scope;
            createtable(root->sons[1],scope);
            scope = scope_stack[--top];
        }
    }
    // paralist  -> type  id para | e
    else if(root->val=="paralist"){
        if(root->type=="funcpara"){
            if(root->sons.size()==0)return;
            else {
                scope_stack[top++]=scope;
                scope=++global_scope;
                createtable(root->sons[0],scope);
                root->sons[1]->type=root->sons[0]->type;
                insert(root->sons[1],scope,0);
                root->sons[2]->type=root->type;
                createtable(root->sons[2],scope);
                scope = scope_stack[--top];  
                global_scope--;
            }
        }
        else if(root->sons.size()!=0){
            createtable(root->sons[0],scope);
            root->sons[1]->type=root->sons[0]->type;
            createtable(root->sons[2],scope);
        }
        else return;
    }
    // para -> , type id para | e
    else if(root->val=="para"){
        if(root->type=="funcpara"){
            if(root->sons.size()==0)return;
            else {
                createtable(root->sons[1],scope);
                root->sons[2]->type=root->sons[1]->type;
                insert(root->sons[2],scope,0);
                root->sons[3]->type=root->type;
                createtable(root->sons[3],scope);
            }
        }
        else if(root->sons.size()==0){
            return;
        }
        else {
            createtable(root->sons[1],scope);
            root->sons[2]->type=root->sons[1]->type;
            createtable(root->sons[3],scope);
        }
    }
    // sentences -> sentences1
    else if(root->val=="sentences"){
        createtable(root->sons[0],scope);
    }
    // sentences1 ->  sentence  sentences1 | e
    else if(root->val=="sentences1"){
        if(root->sons.size()==0){
            return;
        }
        else {
            createtable(root->sons[0],scope);
            createtable(root->sons[1],scope);
        }
    }
    // sentence -> branch | assignment ; | loop | jump ; | statement ;
    else if(root->val=="sentence"){
        createtable(root->sons[0],scope);
    }
    // statement -> type idlist 
    else if(root->val=="statement"){
        createtable(root->sons[0],scope);
        root->sons[1]->type=root->sons[0]->type;
        createtable(root->sons[1],scope);
    }
    // idlist ->  id idassignlist2 idlist3    idassignlist2->name
    else if(root->val=="idlist"){
        root->sons[0]->type=root->type;
        root->sons[1]->type=root->sons[0]->type;
        root->sons[1]->name=root->sons[0]->name;
        createtable(root->sons[1],scope);
        root->sons[2]->type=root->sons[0]->type;
        // root->sons[2]->name=root->sons[2]->name;
        createtable(root->sons[2],scope);
    }
    // idlist3 -> , idlist | e
    else if(root->val=="idlist3"){
        if(root->sons.size()==0){
            return;
        }
        else {
            root->sons[1]->type=root->type;
            createtable(root->sons[1],scope);
        }
    }
    // idassignlist2 -> = expression | e | arrayassign
    else if(root->val=="idassignlist2"){
        if(root->sons.size()==0){
            insert(root,scope,0);
            return;
        }
        else if(root->sons[0]->val=="="){//求expression type 并判断是否相等
            root->sons[1]->name=root->name;
            createtable(root->sons[1],scope);
            string temp=gettype(root->sons[1]->type,root->type);
            if(temp=="wrong"){
                printf("Type mismatch at %s  at rows %d\n",root->name.c_str(),root->line);
                root->type="wrong";
            }
            else if(temp=="int"){
                root->ivalue=root->sons[1]->ivalue;
                if(root->sons[1]->sons[0]->val=="cons"){
                    root->ivalue=root->sons[1]->ivalue;
                    printf("The value of variable %s  at rows %d is %d\n",root->name.c_str(),root->line,root->ivalue);
                }
                insert(root,scope,0);
            }
            else{
                insert(root,scope,0);
            }
        }
        else{
            root->sons[0]->name=root->name;
            root->sons[0]->type=root->type;
            insert(root,scope,0);
            createtable(root->sons[0],scope);
        }
    }
    //expression -> id expression1 | cons opexpression1 | ( opnum opexpression1 )
    else if(root->val=="expression"){
        if(root->sons[0]->val=="id"){
            //正常表达式
            if(root->sons[1]->sons.size()==1){
                createtable(root->sons[0],scope);
                string type1=root->sons[0]->type;
                createtable(root->sons[1],scope);
                string type2=root->sons[1]->type;
                if(gettype(type1,type2)!="wrong"){
                    root->type=gettype(type1,type2);
                }
                else {
                    printf("Type mismatch at %s  at rows%d\n",root->name.c_str(),root->line);
                    root->type="wrong";
                }
            }
            //函数调用
            else {
                createtable(root->sons[1],scope);
                int pnum=0;
                vector<string> ptype;
                Node* temp=root->sons[1]->sons[1];//paralist
                ptype.push_back(temp->sons[0]->type);
                pnum++;
                temp=temp->sons[2];
                while(temp->sons.size()!=0){
                    ptype.push_back(temp->sons[1]->type);
                    pnum++;
                    temp=temp->sons[2];
                }
                if(fhavesymbol(root->sons[0]->name)){
                    string type=findfsymbol(root->sons[0]->name,pnum,ptype);
                    if(type!="wrong"){
                        root->type=type;
                    }
                    else {
                        printf("Type mismatch at %s  at rows%d\n",root->name.c_str(),root->line);
                        root->type="wrong";
                    }
                }
                else {
                    printf("Function  %s not found at rows%d\n",root->sons[0]->name.c_str(),root->line);
                     root->type="wrong";
                }
            }
            
        }
        else if(root->sons[0]->val=="cons"){
            createtable(root->sons[0],scope);
            createtable(root->sons[1],scope);
            string type1=root->sons[0]->type;
            string type2=root->sons[1]->type;
            if(type2=="null"){// opexpression->e
                root->type=type1;
                root->ivalue=root->sons[0]->ivalue;
            }
            else if(gettype(type1,type2)!="wrong"){
                root->type=gettype(type1,type2);
                if(root->sons[1]->sons.size()!=0&&root->type=="int"){//分类讨论
                    if(root->sons[1]->sons[0]->val=="+"){//opexpression=cons + opexpression
                        root->ivalue=root->sons[0]->ivalue+root->sons[1]->sons[1]->ivalue;
                    }
                    else if(root->sons[1]->sons[0]->val=="-"){
                        root->ivalue=root->sons[0]->ivalue+root->sons[1]->sons[1]->ivalue;
                    }
                        else if(root->sons[1]->sons[0]->val=="*"){
                        root->ivalue=root->sons[0]->ivalue*root->sons[1]->sons[1]->ivalue;
                    }
                        else if(root->sons[1]->sons[0]->val=="/"){
                        root->ivalue=root->sons[0]->ivalue/root->sons[1]->sons[1]->ivalue;
                    }
                        else if(root->sons[1]->sons[0]->val=="&"){
                        root->ivalue=root->sons[0]->ivalue & root->sons[1]->sons[1]->ivalue;
                    }
                        else if(root->sons[1]->sons[0]->val=="|"){
                        root->ivalue=root->sons[0]->ivalue | root->sons[1]->sons[1]->ivalue;
                    }
                }
            }
            else{
                cout<<"Opexpression Error:";
                printf("Type mismatch of expression %s  at rows %d\n",root->sons[0]->name.c_str(),root->sons[0]->line);
                root->type="wrong";
            }
        }
        else if(root->sons[1]->val=="opnum") {
            createtable(root->sons[1],scope);
            createtable(root->sons[2],scope);
            string type1=root->sons[0]->type;
            string type2=root->sons[1]->type;
            if(gettype(type1,type2)!="wrong"){
                root->type=gettype(type1,type2);
            }
            else{
                printf("Type mismatch at %s  at rows %d\n",root->sons[1]->name.c_str(),root->sons[1]->line);
                root->type="wrong";
            }
        }
    }
    // expression1 -> ( paralist ) | opexpression1
    else if(root->val=="expression1"){
        if(root->sons.size()==1){
            createtable(root->sons[0],scope);
            root->type=root->sons[0]->type;
            return;
        }
        else{
            createtable(root->sons[1],scope);
            root->type=root->sons[1]->type;
        }
    }
    // opexpression1 -> e | + opexpression | - opexpression | * opexpression | / opexpression | & opexpression | > opexpression | < opexpression | <= opexpression | >= opexpression | == opexpression | != opexpression
    else if(root->val=="opexpression1"){
        if(root->sons.size()==0){
            root->type="null";
            return;
        }
        else{
            createtable(root->sons[1],scope);
            root->type=root->sons[1]->type;
            return;
        }
    }
    //cons
    else if(root->val=="cons"){
        string ans=root->name;
        if(ans=="true"||ans=="false")root->type="bool";
        else {
            root->type="int";
            for(auto a:ans){
                if(a=='.'){
                    root->type="float";
                    break;
                }
            }
            root->ivalue= atoi(root->name.c_str());//获得cons的int表示
        }
    }
    //id
    else if(root->val=="id"){
        if(findsymbol(root->name,scope)!=""){
            string type1=findsymbol(root->name,scope);
            root->type=type1;
        }
        else {
            printf("Variable  %s not found at rows %d\n",root->name.c_str(),root->line);
            root->type="wrong";
            return;
        }
    }
    // opexpression -> cons opexpression1 | ( opnum opexpression1 ) | id opexpression1
    else if(root->val=="opexpression"){
        if(root->sons[0]->val=="cons"){
            createtable(root->sons[0],scope);
            createtable(root->sons[1],scope);
            string type1=root->sons[0]->type;
            string type2=root->sons[1]->type;
            if(type2=="null"){// opexpression->e
                root->type=type1;
                root->ivalue=root->sons[0]->ivalue;
            }
            else if(gettype(type1,type2)!="wrong"){
                root->type=gettype(type1,type2);
                if(root->sons[1]->sons.size()!=0&&root->type=="int"){//分类讨论
                    if(root->sons[1]->sons[0]->val=="+"){//opexpression=cons + opexpression
                        root->ivalue=root->sons[0]->ivalue+root->sons[1]->sons[1]->ivalue;
                    }
                    else if(root->sons[1]->sons[0]->val=="-"){
                        root->ivalue=root->sons[0]->ivalue+root->sons[1]->sons[1]->ivalue;
                    }
                        else if(root->sons[1]->sons[0]->val=="*"){
                        root->ivalue=root->sons[0]->ivalue*root->sons[1]->sons[1]->ivalue;
                    }
                        else if(root->sons[1]->sons[0]->val=="/"){
                        root->ivalue=root->sons[0]->ivalue/root->sons[1]->sons[1]->ivalue;
                    }
                        else if(root->sons[1]->sons[0]->val=="&"){
                        root->ivalue=root->sons[0]->ivalue & root->sons[1]->sons[1]->ivalue;
                    }
                        else if(root->sons[1]->sons[0]->val=="|"){
                        root->ivalue=root->sons[0]->ivalue | root->sons[1]->sons[1]->ivalue;
                    }
                }
            }
            else{
                cout<<"Opexpression";
                printf("Type mismatch of expression %s  at rows %d\n",root->sons[0]->name.c_str(),root->sons[0]->line);
                root->type="wrong";
            }
        }
        else if(root->sons.size()==2){
            createtable(root->sons[0],scope);
            string type1=root->sons[0]->type;
            createtable(root->sons[1],scope);
            string type2=root->sons[1]->type;
            if(type2=="null"){
                root->type=type1;
            }
            else if(gettype(type1,type2)!="wrong"){
                root->type=gettype(type1,type2);
            }
            else {
                printf("Type mismatch at %s  at rows %d\n",root->sons[0]->name.c_str(),root->sons[0]->line);
                root->type="wrong";
            }
        }
        else if(root->sons[1]->val=="opnum") {
            createtable(root->sons[1],scope);
            createtable(root->sons[2],scope);
            string type1=root->sons[0]->type;
            string type2=root->sons[1]->type;
            if(gettype(type1,type2)!="wrong"){
                root->type=gettype(type1,type2);
            }
            else{
                printf("Type mismatch at %s  at rows %d\n",root->sons[1]->name.c_str(),root->sons[1]->line);
                root->type="wrong";
            }

        }
    }
    //opnum -> id | cons
    else if(root->val=="opnum"){
        createtable(root->sons[0],scope);
        root->name=root->sons[0]->name;
    }
    // arrayassign -> [ cons ] arrayassign2
    else if(root->val=="arrayassign"){
        createtable(root->sons[1],scope);
        if(root->sons[1]->type!="int"){
            printf("Incorrect array index  at rows %d\n",root->name.c_str(),root->line);
            return;
        }
        root->sons[3]->type=root->type;//先把之前的type赋值给arrayassign2
        root->sons[3]->name=root->name;//先把之前的type赋值给arrayassign2
        createtable(root->sons[3],scope);
        root->type="Array";//修改当前的type为array
        insert(root,scope,0);

    }
    // arrayassign2 -> = { idlist } | e
    else if(root->val=="arrayassign2"){
        if(root->sons.size()==0){
            return;
        }
        else{
            createtable(root->sons[1],scope);
            if(root->sons[1]->type!=root->type){
                printf("Type mismatch in Array %s  at rows %d\n",root->name.c_str(),root->line);
                return;
            }
            else{

            }
        }
    
    }    
    // assignment -> id assignment1    
    else if(root->val=="assignment"){
        createtable(root->sons[0],scope);
        if(root->sons[1]->sons[0]->val=="++"||root->sons[1]->sons[0]->val=="--"){
            if(root->sons[0]->type!="int"&&root->sons[0]->type!="float")
            printf("Incorrect operation for int at %s  at rows %d\n",root->sons[0]->name.c_str(),root->sons[0]->line);
        }
        else {
            root->sons[1]->type=root->sons[0]->type;
            root->sons[1]->name=root->sons[0]->name;
            createtable(root->sons[1],scope);
        }
    }
    // assignment1 -> ++ | -- | = expression | [ cons ] = expression
    else if(root->val=="assignment1"){
        if(root->sons.size()==2){
            createtable(root->sons[1],scope);
            string type1=root->type;
            string type2=root->sons[1]->type;
            if(gettype(type1,type2)!="wrong"&&type2=="int"){
                root->ivalue=root->sons[1]->ivalue;
                root->type=gettype(type1,type2);
                if(root->sons[1]->sons[0]->val=="cons"){
                    root->ivalue=root->sons[1]->ivalue;
                    printf("The value of variable %s  at rows %d is %d\n",root->name.c_str(),root->line,root->ivalue);
                }
            }
            else if(gettype(type1,type2)!="wrong"){
                root->type=gettype(type1,type2);
            }
            else  {
                printf("Assignment1 error:Type mismatch at %s  at rows %d\n",root->name.c_str(),root->line);
                root->type="wrong";
            }
        }
        else{
            createtable(root->sons[1],scope);
            if(root->sons[1]->type!="int"){
                printf("Incorrect array index  at rows %d\n",root->name.c_str(),root->line);
                return;
            }
            createtable(root->sons[4],scope);
            string type1=root->type;
            string type2=root->sons[4]->type;
            if(gettype(type1,type2)!="wrong"){
            }
            else  printf("Type mismatch at %s  at rows %d\n",root->name.c_str(),root->line);
        }
    }
    // branch -> if ( cmpexpression ) { sentences } branch1
    else if(root->val=="branch"){
        createtable(root->sons[2],scope);
        if(root->sons[2]->type!="bool"){
            printf("Type mismatch in if  at rows %d\n",root->line);
            return;
        }
        else{
            scope_stack[top++]=scope;
            scope=++global_scope;
            createtable(root->sons[5],scope);
            scope = scope_stack[--top];
            createtable(root->sons[7],scope);
        }
    }
    // cmpexpression -> id expression1 | cons 
    else if(root->val=="cmpexpression"){
        if(root->sons.size()==1){
            root->type="bool";
        }
        else if(root->sons.size()==2){
            //正常表达式
            if(root->sons[1]->sons.size()==1){
                createtable(root->sons[0],scope);
                string type1=root->sons[0]->type;
                createtable(root->sons[1],scope);
                string type2=root->sons[1]->type;
                if(type2=="null"){
                    root->type=type1;
                }
                else if(gettype(type1,type2)!="wrong"){
                    root->type="bool";
                }
                else {
                    cout<<"Cmpexpression error:";
                    printf("Type mismatch at %s  at rows %d\n",root->name.c_str(),root->line);
                    root->type="wrong";
                }
            }
            //函数调用
            else {
                createtable(root->sons[1],scope);
                int pnum=0;
                vector<string> ptype;
                Node* temp=root->sons[1]->sons[1];//paralist
                ptype.push_back(temp->sons[0]->type);
                pnum++;
                temp=temp->sons[2];
                while(temp->sons.size()!=0){
                    ptype.push_back(temp->sons[1]->type);
                    pnum++;
                    temp=temp->sons[2];
                }
                if(fhavesymbol(root->sons[0]->name)){
                    string type=findfsymbol(root->sons[0]->name,pnum,ptype);
                    if(type!="wrong"){
                        root->type="bool";
                    }
                    else {
                        printf("Type mismatch at %s  at rows %d\n",root->name.c_str(),root->line);
                        root->type="wrong";
                    }
                }
                else {
                    printf("Function  %s not found at rows %d\n",root->sons[0]->name.c_str(),root->line);
                     root->type="wrong";
                }
            }
            
        }
    }
    // branch1 -> else branch2  | e
    else if(root->val=="branch1"){
        if(root->sons.size()==1){
            return;
        }
        else if(root->sons.size()==2){
            createtable(root->sons[1],scope);
        }
    }
    // branch2 -> { sentences } | sentence
    else if(root->val=="branch2"){
        if(root->sons.size()==1){
            createtable(root->sons[0],scope);
        }
        else {
            scope_stack[top++]=scope;
            scope=++global_scope;
            createtable(root->sons[1],scope);
            scope = scope_stack[--top];
        }
    }
    // loop -> forloop | whileloop
    else if(root->val=="loop"){
        createtable(root->sons[0],scope);
    }
    // forloop -> for ( assignment ; cmpexpression ; opexpression1 ) { sentences }
    else if(root->val=="forloop"){
        createtable(root->sons[2],scope);
        createtable(root->sons[4],scope);
        if(root->sons[4]->type!="bool"){
            printf("Type mismatch in if  at rows %d\n",root->line);
            return;
        }
        createtable(root->sons[6],scope);
        scope_stack[top++]=scope;
        scope=++global_scope;
        createtable(root->sons[9],scope);
        scope = scope_stack[--top];
        
    }
    // whileloop -> while ( cmpexpression ) { sentences  }   
    else if(root->val=="whileloop"){
        createtable(root->sons[2],scope);
        if(root->sons[2]->type!="bool"){
            printf("Type mismatch in while condition  at rows %d\n",root->line);
            return;
        }
        scope_stack[top++]=scope;
        scope=++global_scope;
        createtable(root->sons[5],scope);
        scope = scope_stack[--top];
    }
    // jump -> break | return cmpexpression | continue 
    else if(root->val=="jump"){
        if(root->sons.size()==1){
            return;
        }
        else{
        createtable(root->sons[1],scope);
        }
    }
    
}
void semantic_analyzer::insert(Node *root,int scope,int offset){
    string name=root->name;
    string type=root->type;
    if(havesymbol(name,scope)){
        printf("Variable %s redefined at rows %d\n",name.c_str(),root->line);
    }
    else{
        // cout<<" insert :      "<<name<<" "<<type<<" "<<scope<<endl;
        addsymbol(name,type,scope,offset);
    }
}
string semantic_analyzer::findsymbol(string name,int scope){
    int minn=-1;
    string ans="";
    for(auto &a:table){
        if(a.name==name&&a.scope<=scope){
            if(minn>=a.scope);
            else{
                minn=a.scope;
                ans=a.type;
            }
        }
    }
    return ans;
}

bool semantic_analyzer::havesymbol(string name,int scope){
    for(auto &a:table){
        if(a.name==name&&a.scope==scope){
            return true;
        }
    }
    return false;
}
void semantic_analyzer::addsymbol(string name,string type,int scope,int offset){
    table.push_back(symbol(name,type,scope,offset));
}

func semantic_analyzer::finsert(Node *root,int flag){
    string name=root->name;
    string type=root->type;
    int pnum=0;
    vector<string> ptype;
    Node* temp=root->sons[1];//paralist
    ptype.push_back(temp->sons[0]->type);
    pnum++;
    temp=temp->sons[2];
    while(temp->sons.size()!=0){
        ptype.push_back(temp->sons[1]->type);
        pnum++;
        temp=temp->sons[2];
    }
    if(fhavesymbol(root,flag,pnum,ptype)){
       return func();
    }
    else{
        func temp=faddsymbol(name,type,flag,pnum,ptype);
        return temp;
    }
}
bool semantic_analyzer::fhavesymbol(Node *root,int flag,int pnum,vector<string> ptype){
    for(auto &a:ftable){
        if(flag==0){
            if(a.name==root->name&&a.type==root->type&&a.pnum==pnum&&a.ptype==ptype&&a.flag==0){
                printf("Function %s redeclared at rows %d\n",root->name.c_str(),root->line);
                return true;
            }
        }
        else {
            if(a.name==root->name&&a.type==root->type&&a.pnum==pnum&&a.ptype==ptype&&a.flag==1){
                printf("Function %s redefined at rows %d\n",root->name.c_str(),root->line);
                return true;
            }
        }
    }
    return false;
}
bool semantic_analyzer::fhavesymbol(string name){
    for(auto &a:ftable){
        if(a.name==name)return true;
    }
    return false;
}
string semantic_analyzer::findfsymbol(string name,int pnum,vector<string> ptype){
    for(auto &a:ftable){
        if(a.name==name&&a.pnum==pnum&&a.ptype==ptype){
            return a.type;
        }
    }
    return "wrong";
}
func semantic_analyzer::faddsymbol(string name,string type,int flag,int pnum,vector<string> ptype){
    func temp(name,type,flag,pnum,ptype);
    ftable.push_back(temp);
    return temp;
}
string semantic_analyzer::gettype(string t1,string t2){
    if(t1=="int"&&t2=="int"){
        return "int";
    }
    else if(t1=="float"&&t2=="int"){
        return "float";
    }
    else if(t1=="int"&&t2=="float"){
        return "float";
    }
    else if(t1=="bool"&&t2=="bool"){
        return "bool";
    }
    
    else return "wrong";
}
