#include <iostream>

#include "lexical_analyzer.h"
#include "grammer_analyzer.h"
#include "semantic_analyzer.h"
#include<vector>
#include <string> 
#include <fstream> 
using namespace std;
int main(){
	lexical_analyzer la;
	la.lexical_analyzer_do("in.txt");
	grammer_analyzer ga("grammer.txt");
	// ga.printpro();
	ga.getfirst();
	ga.getfollow();
	ga.gettable();
	ga.analyse("lexical.txt",la);
	 ga.printtree();
	semantic_analyzer sa(ga.getroot());
	sa.createtable(sa.getroot(),0);
	return 0;
}
