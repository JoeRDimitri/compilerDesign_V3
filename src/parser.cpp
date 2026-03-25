#include "parser.h"

bool parser::parse(const std::vector<token*>  & vectorOfTokens){
	//File to write the derivation to.
    std::ofstream derivationOut("./outputs/Derivation"); // Open file for writing
    //i for conditional debugging
	int i =0;
	//Derivation vector is made to print out the current derivation. 
	std::vector<std::string> derivation;
	//LineIndex variable
	int lineIndex =0;
	//Is the current table entry an error?
	bool error;
	//Initial variables for parsing stack.
	parsingStack.push("$");
	parsingStack.push("START");
	//Initial variables for Derivation 
	derivation.emplace_back("START");
	//Iterator to iterate overt the vector of tokens. Just a pointer iterator. 
	auto vectorIterator = vectorOfTokens.begin();
	//Dereferencing the token point to get the actual token object. 
	token* currentToken = (*vectorIterator);
	//While we havent reached the end of the program. 
	while(parsingStack.top()!="$"){
		i++;
		//Get the nex parsing object. 
		//Basically the top of the stack is determines if the current token is valid by analyzing the first and follow sets
		std::string topOfTheStack = parsingStack.top();

		//Case 1: The top of the stack is a terminal symbol.
		if(parsingTable.uniqueTerminalSymbols.count(topOfTheStack)!=0){
			//If it is a terminal symbol, 
			//then we need to match if the terminal symbol matches the same as the current Token 
			if(topOfTheStack==(currentToken->getTypeName())){
				//If it does, then success, we can remove this terminal symbol from the parsing stack
				// We've successfully matched this token from the parsing stack. 
				parsingStack.pop();
				//We can successfully remove this terminal symbol from the stack. 
				derivation.erase(derivation.begin()+lineIndex);
				derivation.insert(derivation.begin()+lineIndex,currentToken->getLexeme());
				lineIndex++;
				vectorIterator++;
				currentToken = (*vectorIterator);
			}
			//Else we handle the error.			
			else{
				//Handle error;
				std::string topOfStack;
				topOfStack = parsingStack.top();
				skipError(currentToken,vectorIterator,derivationOut,topOfStack,topOfTheStack,currentToken->getLine(),currentToken->getColumn());
				error = true;
			}
		}
		//Case 2: The top of the stack is a semantic action. 
		else if(topOfTheStack.at(0) == '#'){
			//last token value need to handle the semanticaction.
			std::string lastTokenValue;
			//Avoid Crashing the program.
			if(vectorIterator!= vectorOfTokens.begin()){
				lastTokenValue = (*(vectorIterator - 1))->getLexeme();
			}
			parsingStack.pop();
			//semanticFunction is the semantic action without the #
			std::string semanticFunction = topOfTheStack.substr(1);
			//handle the semantic action.
			semanticHandler.handleAction(semanticFunction,lastTokenValue);
			derivation.erase(derivation.begin()+lineIndex);

		}
		//Else case: not a terminal symbol or a semantic action : then it could be a non-terminal symbol or epsilon?
		else {
			//Epsilon case
			if(topOfTheStack == "EPSILON"){
				//We can simply dismiss it and ignore the epsilon.
				parsingStack.pop();
				derivation.erase(derivation.begin()+lineIndex);
				for(auto const & value:derivation){
					derivationOut<<value<<" ";
				}
				derivationOut<<std::endl;
				continue;
			}
			//Non terminal symbol. 
			//Need to find the row and column value for the table
			//row will be whatever the top of the stack is. Columnvalue is whatever the name of the current token is.
			int rowValue = parsingTable.tableRows[topOfTheStack];
			int columnValue = parsingTable.tableColumns[((currentToken)->getTypeName())];
			tableEntry currentTableEntry = parsingTable.table[rowValue][columnValue];
			//The table dictates if we are on this non  terminal symbol with this current token, what rule we need to apply.
			//The rule that we apply is based on the first and follow set, which is integrated when the table is built. 
			if(currentTableEntry.derivationRule!="error"){
				//Not an error meaning that the current non terminal 
				parsingStack.pop();
				//We replace the top of the stack. 
				derivation.erase(derivation.begin()+lineIndex);
				//We need to inverse the rule and push it onto the stack. 
				inverseRHSMultiplePush(currentTableEntry,derivation,lineIndex);
				for(auto const & value:derivation){
					derivationOut<<value<<" ";
				}
				derivationOut<<std::endl;
			}
			else{
				//Need to check the follow set of the current the top of the stack.
				std::string topOfStack;
				topOfStack = parsingStack.top();
				//Handle error;
				skipError(currentToken,vectorIterator,derivationOut,topOfStack,topOfTheStack,currentToken->getLine(),currentToken->getColumn());
				error = true;
			}
		}


	}
	if((currentToken->getTypeName()!="$")||(error ==true)){
		return false;
	}
	else return true;
}

void parser::inverseRHSMultiplePush(tableEntry currentTableEntry,std::vector<std::string>&derivation,const int & lineIndex){
	std::string lineToInverseAndSplitThenPush = currentTableEntry.derivationRule;
	std::stack<std::string> items;
	int tempLineIndex =0;
	while(lineToInverseAndSplitThenPush.at(tempLineIndex)!= '>'){tempLineIndex++;}
	while(lineToInverseAndSplitThenPush.at(tempLineIndex)!= '='){tempLineIndex++;}
	tempLineIndex++;
	while(tempLineIndex<lineToInverseAndSplitThenPush.size()){
		std::string currentString;
		while(tempLineIndex != lineToInverseAndSplitThenPush.size() && lineToInverseAndSplitThenPush.at(tempLineIndex)!= '\'' && lineToInverseAndSplitThenPush.at(tempLineIndex)!='<' &&lineToInverseAndSplitThenPush.at(tempLineIndex)!='#'  && lineToInverseAndSplitThenPush.at(tempLineIndex)!= 'E'){
			tempLineIndex++;
		}
		if(tempLineIndex==lineToInverseAndSplitThenPush.size())break;
		if (lineToInverseAndSplitThenPush.at(tempLineIndex)=='\''){
			tempLineIndex++;
			while(lineToInverseAndSplitThenPush.at(tempLineIndex)!='\''){
				currentString +=lineToInverseAndSplitThenPush.at(tempLineIndex);
				tempLineIndex++;
			}
			tempLineIndex++;
			items.push(currentString);
			continue;
		}
		else if(lineToInverseAndSplitThenPush.at(tempLineIndex)=='<'){
			tempLineIndex++;

			while(lineToInverseAndSplitThenPush.at(tempLineIndex)!='>'){
				currentString +=lineToInverseAndSplitThenPush.at(tempLineIndex);
				tempLineIndex++;
			}
			tempLineIndex++;
			items.push(currentString);
			continue;
		}
		else if(lineToInverseAndSplitThenPush.at(tempLineIndex)=='#' ){
					while(tempLineIndex!=lineToInverseAndSplitThenPush.size()&&lineToInverseAndSplitThenPush.at(tempLineIndex)!=' '){
						currentString +=lineToInverseAndSplitThenPush.at(tempLineIndex);
						tempLineIndex++;
					}
					tempLineIndex++;
					items.push(currentString);
					continue;
		}
		else if(lineToInverseAndSplitThenPush.at(tempLineIndex)=='E'){
			while(tempLineIndex!=lineToInverseAndSplitThenPush.size()&&currentString != "EPSILON"){
				currentString +=lineToInverseAndSplitThenPush.at(tempLineIndex);
				tempLineIndex++;

			}
			tempLineIndex++;
		}
	}
	while(!items.empty()){
		 parsingStack.push(items.top());
			derivation.insert(derivation.begin()+lineIndex,items.top());

		 items.pop();
	}
}

void parser::skipError(token * & currentToken,std::vector<token*>::const_iterator& vectorIterator,std::ofstream & derivationOut,const std::string &lookahead, const std::string &topOfTheStack,const int & line, const int & column){
	derivationOut<<"Syntax error at ("<< line<<", "<<column<<")"<<std::endl;
	std::string nextToken = (*(vectorIterator +1))->getLexeme();
	if(nextToken == "$"|| search(nextToken,topOfTheStack)){
		parsingStack.pop();
		vectorIterator++;
	}
	else{
		while((!searchFirst(nextToken,topOfTheStack))&&(searchFirst("EPSILON",topOfTheStack)&&!search(nextToken,topOfTheStack))){
			vectorIterator++;
			nextToken = (*(vectorIterator+1))->getLexeme();
			currentToken = (*vectorIterator);
		}
		vectorIterator++;
		currentToken = (*vectorIterator);

	}

}

bool parser::search(const std::string & nextToken, const std::string & topOfTheStack){
	if(first_and_follow::inVector(faf.followSet[topOfTheStack],nextToken)){
		return true;
	}
	else return false;
}

bool parser::searchFirst(std::string nextToken, std::string topOfTheStack){
	if(first_and_follow::inVector(faf.firstSet[topOfTheStack],nextToken)){
		return true;
	}
	else return false;
}

void parser::semanticActions::handleAction(std::string semanticFunction,std::string lastTokenValue){


	if(semanticFunction == "start"){abstractSyntaxTree::startNode * newnode = new abstractSyntaxTree::startNode(); this->passAlong("start",newnode);}
	else if(semanticFunction == "aparamstail"){abstractSyntaxTree::aparamstailNode * newnode = new abstractSyntaxTree::aparamstailNode(); this->passAlong("aparamstail",newnode);}
	else if(semanticFunction == "arithexpr"){abstractSyntaxTree::arithexprNode * newnode = new abstractSyntaxTree::arithexprNode(); this->passAlong("arithexpr",newnode);}
	else if(semanticFunction == "arraysize"){abstractSyntaxTree::arraysizeNode * newnode = new abstractSyntaxTree::arraysizeNode(); this->passAlong("arraysize",newnode);}
	else if(semanticFunction == "attributedecl"){abstractSyntaxTree::attributedeclNode * newnode = new abstractSyntaxTree::attributedeclNode(); this->passAlong("attributedecl",newnode);}
	else if(semanticFunction == "class"){abstractSyntaxTree::classNode * newnode = new abstractSyntaxTree::classNode(); this->passAlong("class",newnode);}
	else if(semanticFunction == "impl"){abstractSyntaxTree::implNode * newnode = new abstractSyntaxTree::implNode(); this->passAlong("impl",newnode);}
	else if(semanticFunction == "func"){abstractSyntaxTree::funcNode * newnode = new abstractSyntaxTree::funcNode(); this->passAlong("func",newnode);}
	else if(semanticFunction == "factor"){abstractSyntaxTree::factorNode * newnode = new abstractSyntaxTree::factorNode(); this->passAlong("factor",newnode);}
	else if(semanticFunction == "idnest2"){abstractSyntaxTree::idnest2Node * newnode = new abstractSyntaxTree::idnest2Node(); this->passAlong("idnest2",newnode);}
	else if(semanticFunction == "funcbody"){abstractSyntaxTree::funcbodyNode * newnode = new abstractSyntaxTree::funcbodyNode(); this->passAlong("funcbody",newnode);}
	else if(semanticFunction == "funcdecl"){abstractSyntaxTree::funcdeclNode * newnode = new abstractSyntaxTree::funcdeclNode(); this->passAlong("funcdecl",newnode);}
	else if(semanticFunction == "indice"){abstractSyntaxTree::indiceNode * newnode = new abstractSyntaxTree::indiceNode(); this->passAlong("indice",newnode);}
	else if(semanticFunction == "localvardecl"){abstractSyntaxTree::localvardeclNode * newnode = new abstractSyntaxTree::localvardeclNode(); this->passAlong("localvardecl",newnode);}
	else if(semanticFunction == "localvardeclorstat"){abstractSyntaxTree::localvardeclorstatNode * newnode = new abstractSyntaxTree::localvardeclorstatNode(); this->passAlong("localvardeclorstat",newnode);}
	else if(semanticFunction == "prog"){abstractSyntaxTree::progNode * newnode = new abstractSyntaxTree::progNode(); this->passAlong("prog",newnode);}
	else if(semanticFunction == "condition"){abstractSyntaxTree::conditionNode * newnode = new abstractSyntaxTree::conditionNode(); this->passAlong("condition",newnode);}
	else if(semanticFunction == "then"){abstractSyntaxTree::thenNode * newnode = new abstractSyntaxTree::thenNode(); this->passAlong("then",newnode);}
	else if(semanticFunction == "felse"){abstractSyntaxTree::felseNode * newnode = new abstractSyntaxTree::felseNode(); this->passAlong("felse",newnode);}
	else if(semanticFunction == "statblock"){abstractSyntaxTree::statblockNode * newnode = new abstractSyntaxTree::statblockNode(); this->passAlong("statblock",newnode);}
	else if(semanticFunction == "returntype"){abstractSyntaxTree::returntypeNode * newnode = new abstractSyntaxTree::returntypeNode(); this->passAlong("returntype","returntype",newnode);}
	else if(semanticFunction == "term"){abstractSyntaxTree::termNode * newnode = new abstractSyntaxTree::termNode(); this->passAlong("term",newnode);}


	else if(semanticFunction == "intnum"){abstractSyntaxTree::intNode * newnode = new abstractSyntaxTree::intNode();this->makeLeaf("intnum",lastTokenValue,newnode);}
	else if(semanticFunction == "p"){abstractSyntaxTree::epsilonNode * newnode = new abstractSyntaxTree::epsilonNode();this->makeLeaf("epsilon",lastTokenValue,newnode);}
	else if(semanticFunction == "addop"){abstractSyntaxTree::addopNode * newnode = new abstractSyntaxTree::addopNode();this->makeLeaf("addop",lastTokenValue,newnode);}
	else if(semanticFunction == "assign"){abstractSyntaxTree::assignNode * newnode = new abstractSyntaxTree::assignNode();this->makeLeaf("assign",lastTokenValue,newnode);}
	else if(semanticFunction == "id"){abstractSyntaxTree::idNode * newnode = new abstractSyntaxTree::idNode();this->makeLeaf("id",lastTokenValue,newnode);}
	else if(semanticFunction == "relop"){abstractSyntaxTree::relopNode * newnode = new abstractSyntaxTree::relopNode();this->makeLeaf("relop",lastTokenValue,newnode);}
	else if(semanticFunction == "self"){abstractSyntaxTree::selfNode * newnode = new abstractSyntaxTree::selfNode();this->makeLeaf("self",lastTokenValue,newnode);}
	else if(semanticFunction == "multop"){abstractSyntaxTree::multopNode * newnode = new abstractSyntaxTree::multopNode();this->makeLeaf("multop",lastTokenValue,newnode);}
	else if(semanticFunction == "void"){abstractSyntaxTree::voidNode * newnode = new abstractSyntaxTree::voidNode();this->makeLeaf("void",lastTokenValue,newnode);}
	else if(semanticFunction == "sign"){abstractSyntaxTree::signNode * newnode = new abstractSyntaxTree::signNode();this->makeLeaf("sign",lastTokenValue,newnode);}
	else if(semanticFunction == "if"){abstractSyntaxTree::ifNode * newnode = new abstractSyntaxTree::ifNode();this->makeLeaf("if",lastTokenValue,newnode);}
	else if(semanticFunction == "while"){abstractSyntaxTree::whileNode * newnode = new abstractSyntaxTree::whileNode();this->makeLeaf("while",lastTokenValue,newnode);}
	else if(semanticFunction == "read"){abstractSyntaxTree::readNode * newnode = new abstractSyntaxTree::readNode();this->makeLeaf("read",lastTokenValue,newnode);}
	else if(semanticFunction == "write"){abstractSyntaxTree::writeNode * newnode = new abstractSyntaxTree::writeNode();this->makeLeaf("write",lastTokenValue,newnode);}
	else if(semanticFunction == "return"){abstractSyntaxTree::returnNode * newnode = new abstractSyntaxTree::returnNode();this->makeLeaf("return",lastTokenValue,newnode);}
	else if(semanticFunction == "type"){abstractSyntaxTree::typeNode * newnode = new abstractSyntaxTree::typeNode();this->makeLeaf("type",lastTokenValue,newnode);}
	else if(semanticFunction == "visibility"){abstractSyntaxTree::visibilityNode * newnode = new abstractSyntaxTree::visibilityNode();this->makeLeaf("visibility",lastTokenValue,newnode);}
	else if(semanticFunction == "floatnum"){abstractSyntaxTree::floatnumNode * newnode = new abstractSyntaxTree::floatnumNode();this->makeLeaf("floatnum",lastTokenValue,newnode);}


	else if(semanticFunction == "reptclassdecl4"){abstractSyntaxTree::reptclassdecl4Node * newnode = new abstractSyntaxTree::reptclassdecl4Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptclassdecl4","{",'E',v,newnode);}
	else if(semanticFunction == "classdecl"){abstractSyntaxTree::classdeclNode * newnode = new abstractSyntaxTree::classdeclNode();std::vector<std::string> v = {"id","optclassdecl2","reptclassdecl4"}; this->makeSubTree("classdecl","classdecl",3,v,newnode);}
	else if(semanticFunction == "aparams"){abstractSyntaxTree::aparamsNode * newnode = new abstractSyntaxTree::aparamsNode(); std::vector<std::string> v = {"epsilon"}; this->makeSubTree("aparams","aparams",'E',v,newnode);}
	else if(semanticFunction == "funcdef"){abstractSyntaxTree::funcdefNode * newnode = new abstractSyntaxTree::funcdefNode();std::vector<std::string> v = {"funchead","funcbody"}; this->makeSubTree("funcdef","funcdef",2,v,newnode);}
	else if(semanticFunction == "expr"){abstractSyntaxTree::exprNode*newnode = new abstractSyntaxTree::exprNode(); std::vector<std::string> v = {"arithexpr","expr2","epsilon"}; this->makeSubTree("expr","expr",2,v,newnode);}
	else if(semanticFunction == "expr2"){abstractSyntaxTree::expr2Node*newnode = new abstractSyntaxTree::expr2Node();std::vector<std::string> v = {"relop","arithexpr"}; this->makeSubTree("expr2","expr2",2,v,newnode);}
	else if(semanticFunction == "reptfparams3"){abstractSyntaxTree::reptfparams3Node*newnode = new abstractSyntaxTree::reptfparams3Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptfparams3","dimlist",'E',v,newnode);}
	else if(semanticFunction == "reptfparams4"){abstractSyntaxTree::reptfparams4Node*newnode = new abstractSyntaxTree::reptfparams4Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptfparams4","param",'E',v,newnode);}
	else if(semanticFunction == "param"){abstractSyntaxTree::paramNode*newnode = new abstractSyntaxTree::paramNode();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("param","param",'E',v,newnode);}
	else if(semanticFunction == "fparams"){abstractSyntaxTree::fparamsNode*newnode = new abstractSyntaxTree::fparamsNode();std::vector<std::string> v = {"param","reptfparams4"}; this->makeSubTree("fparams","funcparams",'E',v,newnode);}
	else if(semanticFunction == "factor2"){abstractSyntaxTree::factor2Node*newnode = new abstractSyntaxTree::factor2Node();std::vector<std::string> v = {"not","sign","factor"}; this->makeSubTree("factor2","factor2",2,v,newnode);}
	else if(semanticFunction == "selfandidnest2"){abstractSyntaxTree::selfandidnest2Node*newnode = new abstractSyntaxTree::selfandidnest2Node();std::vector<std::string> v = {"self","idnest2"}; this->makeSubTree("selfandidnest2","selfandidnest2",2,v,newnode);}
	else if(semanticFunction == "reptvariable2"){abstractSyntaxTree::reptvariable2Node*newnode = new abstractSyntaxTree::reptvariable2Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptvariable2","reptvariable2",'E',v,newnode);}
	else if(semanticFunction == "aparamsandid"){abstractSyntaxTree::aparamsandidNode*newnode = new abstractSyntaxTree::aparamsandidNode();std::vector<std::string> v = {"id","aparams"}; this->makeSubTree("aparamsandid","functioncall",2,v,newnode);}
	else if(semanticFunction == "reptvariable2andid"){abstractSyntaxTree::reptvariable2andidNode*newnode = new abstractSyntaxTree::reptvariable2andidNode();std::vector<std::string> v = {"id","reptvariable2"}; this->makeSubTree("reptvariable2andid","term",2,v,newnode);}
	else if(semanticFunction == "reptfuncbody1"){abstractSyntaxTree::reptfuncbody1Node*newnode = new abstractSyntaxTree::reptfuncbody1Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptfuncbody1","funcbody",'E',v,newnode);}
	else if(semanticFunction == "funchead"){abstractSyntaxTree::funcheadNode*newnode = new abstractSyntaxTree::funcheadNode();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("funchead","funchead",'E',v,newnode);}
	else if(semanticFunction == "reptimpldef3"){abstractSyntaxTree::reptimpldef3Node*newnode = new abstractSyntaxTree::reptimpldef3Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptimpldef3","reptimpldef3",'E',v,newnode);}
	else if(semanticFunction == "impldef"){abstractSyntaxTree::impldefNode*newnode = new abstractSyntaxTree::impldefNode();std::vector<std::string> v = {"id","reptimpldef3"}; this->makeSubTree("impldef","impldef",2,v,newnode);}
	else if(semanticFunction == "reptoptclassdecl22"){abstractSyntaxTree::reptoptclassdecl22Node*newnode = new abstractSyntaxTree::reptoptclassdecl22Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptoptclassdecl22","reptoptclassdecl22",'E',v,newnode);}
	else if(semanticFunction == "optclassdecl2"){abstractSyntaxTree::optclassdecl2Node*newnode = new abstractSyntaxTree::optclassdecl2Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("optclassdecl2","isalist",'E',v,newnode);}
	else if(semanticFunction == "reptprog0"){abstractSyntaxTree::reptprog0Node*newnode = new abstractSyntaxTree::reptprog0Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptprog0","prog",'E',v,newnode);}
	else if(semanticFunction == "relexpr"){abstractSyntaxTree::relexprNode*newnode = new abstractSyntaxTree::relexprNode();std::vector<std::string> v = {"arithexpr","relop","arithexpr"}; this->makeSubTree("relexpr","relexpr",3,v,newnode);}
	else if(semanticFunction == "funcdeclfam"){abstractSyntaxTree::funcdeclfamNode*newnode = new abstractSyntaxTree::funcdeclfamNode();std::vector<std::string> v = {"visibility","funcdecl"}; this->makeSubTree("funcdecl","funcdecl",2,v,newnode);}
	else if(semanticFunction == "attributedeclfam"){abstractSyntaxTree::attributedeclfamNode*newnode = new abstractSyntaxTree::attributedeclfamNode();std::vector<std::string> v = {"visibility","attributedecl"}; this->makeSubTree("attributedeclfam","attributedecl",2,v,newnode);}
	else if(semanticFunction == "reptstatblock1"){abstractSyntaxTree::reptstatblock1Node*newnode = new abstractSyntaxTree::reptstatblock1Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptstatblock1","reptstatblock1",'E',v,newnode);}
	else if(semanticFunction == "vardecl"){abstractSyntaxTree::vardeclNode*newnode = new abstractSyntaxTree::vardeclNode();std::vector<std::string> v = {"id","type","reptvardecl3"}; this->makeSubTree("vardecl","vardecl",3,v,newnode);}
	else if(semanticFunction == "reptvardecl3"){abstractSyntaxTree::reptvardecl3Node*newnode = new abstractSyntaxTree::reptvardecl3Node();std::vector<std::string> v = {"epsilon"}; this->makeSubTree("reptvardecl3","dimlist",'E',v,newnode);}


	else if(semanticFunction == "dot"){abstractSyntaxTree::dotNode*newnode = new abstractSyntaxTree::dotNode();this->makeBinarySubTree("dot",0,newnode);}
	else if(semanticFunction == "rightrecarithexpr"){abstractSyntaxTree::rightrecarithexprNode*newnode = new abstractSyntaxTree::rightrecarithexprNode();this->makeBinarySubTree("rightrecarithexpr",1,newnode);}
	else if(semanticFunction == "rightrecterm"){abstractSyntaxTree::rightrectermNode*newnode = new abstractSyntaxTree::rightrectermNode();this->makeBinarySubTree("rightrecterm",1,newnode);}
	else if(semanticFunction == "reptstatement4"){abstractSyntaxTree::reptstatement4Node*newnode = new abstractSyntaxTree::reptstatement4Node();this->makeBinarySubTree("reptstatement4",1,newnode);}


	else if(semanticFunction == "whilestatement"){abstractSyntaxTree::whilestatementNode*newnode = new abstractSyntaxTree::whilestatementNode();this->makeBinarySubTreeWithHead("whilestatement",3,newnode);}
	else if(semanticFunction == "readstatement"){abstractSyntaxTree::readstatementNode*newnode = new abstractSyntaxTree::readstatementNode();this->makeBinarySubTreeWithHead("readstatement",2,newnode);}
	else if(semanticFunction == "writestatement"){abstractSyntaxTree::writestatementNode*newnode = new abstractSyntaxTree::writestatementNode();this->makeBinarySubTreeWithHead("writestatement",2,newnode);}
	else if(semanticFunction == "freturnstatement"){abstractSyntaxTree::freturnstatementNode*newnode = new abstractSyntaxTree::freturnstatementNode();this->makeBinarySubTreeWithHead("freturnstatement",2,newnode);}
	else if(semanticFunction == "ifstatement"){abstractSyntaxTree::ifstatementNode*newnode = new abstractSyntaxTree::ifstatementNode();this->makeBinarySubTreeWithHead("ifstatement",4,newnode);}


}

// Creates a terminal (leaf) AST node with no children, sets its value to the last matched token,
// and pushes it onto the semantic stack.
void parser::semanticActions:: makeLeaf(std::string nodeType,std::string lastTokenValue,abstractSyntaxTree::node * newnode){
	newnode->nodeType = nodeType;
	newnode->semanticMeaning = nodeType;
	newnode->nodeValue = lastTokenValue; // store the actual token text (e.g. identifier name, literal)
	newnode->isLeaf = true;

	semanticStackPtr->push(newnode); // push leaf so parent nodes can adopt it later
}

// Pops numOfPops nodes; promotes the LAST popped node as the parent (copying its data into newnode),
// then wires the remaining nodes as its children and pushes the result onto the semantic stack.
void parser::semanticActions::makeBinarySubTreeWithHead(std::string nodeType,int numOfPops,abstractSyntaxTree::node* newnode){
	std::vector <abstractSyntaxTree::node *>nodesOnStack;
	std::vector <abstractSyntaxTree::node *>children;
	std::stack <abstractSyntaxTree::node*> reverseChildren; // used to restore stack-pop order

	// pop numOfPops nodes, preserving left-to-right order via reverseChildren
	for(int i = numOfPops -1; i>=0; i--){
		reverseChildren.push(semanticStackPtr->top());
		semanticStackPtr->pop();
	}
	while(!reverseChildren.empty()){
		nodesOnStack.emplace_back(reverseChildren.top());
		reverseChildren.pop();
	}

	// the last node in the ordered list becomes the parent; copy its data into newnode
	abstractSyntaxTree::node * parentNode = nodesOnStack[nodesOnStack.size()-1];
	newnode->copyNode(parentNode, newnode, nodeType);
	delete parentNode; // original parent shell is no longer needed after copy

	// all nodes except the promoted parent become children (in reverse so leftmost is first)
	for(int k = nodesOnStack.size()-2; k>=0; k--){
		children.emplace_back(nodesOnStack.at(k));
	}
	// wire sibling linked-list pointers across all children
	for(int i = 0; i<children.size();i++){
		if(i ==0){
			if(i + 1<children.size()){
				children.at(i)->headOfSibling = children.at(i); // first child is sibling-list head
				children.at(i)->rightSibling = children.at(i+1);
			}
		}
		else if(i==children.size()-1){
			if(i -1 > -1 ){
				children.at(i)->headOfSibling = children.at(0);
				children.at(i)->leftSibling = children.at(i-1); // last child has no right sibling
			}
		}
		else{
			children.at(i)->headOfSibling = children.at(0);
			children.at(i)->rightSibling = children.at(i+1);
			children.at(i)->leftSibling = children.at(i-1);

		}

	}
	// set parent pointer on every child back up to newnode
	for(abstractSyntaxTree::node * value : children){
		value->parent = newnode;
	}
	newnode->children = children;
	semanticStackPtr->push(newnode); // push completed subtree


}

// Builds a two-child subtree from the semantic stack.
// i==0: pops 2 nodes [first, second] and makes them direct children of newnode.
// i==1: pops 3 nodes, promotes the middle node (second) as the parent via copyNode,
//        and makes first & third the two children (second is discarded after copy).
void parser::semanticActions::makeBinarySubTree(std::string nodeType,int i,abstractSyntaxTree::node*newnode){
	if(i ==0){
		// simple two-child case: pop two nodes and attach them as left/right children
		abstractSyntaxTree::node * first = semanticStackPtr->top();
		semanticStackPtr->pop();
		abstractSyntaxTree::node * second = semanticStackPtr->top();
		semanticStackPtr->pop();

		std::vector<abstractSyntaxTree::node* > v = {first,second};
		newnode->children=v;
		newnode->nodeType=nodeType;
		newnode->semanticMeaning=nodeType;

		// wire sibling pointers: first <-> second
		first->headOfSibling = first;
		first->leftSibling = nullptr;
		first->rightSibling=second;
		first->parent = newnode;
		second ->headOfSibling = first;
		second ->leftSibling = first;
		second -> rightSibling = nullptr;
		second ->parent = newnode;
		newnode ->children =v;
		semanticStackPtr->push(newnode);

	}
	else if(i == 1){
		// three-node pop: middle node becomes the parent, outer two become children
		abstractSyntaxTree::node * first = semanticStackPtr->top();
		semanticStackPtr->pop();
		abstractSyntaxTree::node * second = semanticStackPtr->top(); // promoted to parent
		semanticStackPtr->pop();
		abstractSyntaxTree::node * third  = semanticStackPtr->top();
		semanticStackPtr->pop();



		std::vector<abstractSyntaxTree::node* > v = {first,third};
		newnode->nodeType=nodeType;
		newnode->semanticMeaning=nodeType;
		newnode->copyNode(second, newnode, nodeType); // copy second's data into newnode
		newnode->children=v;
		delete second; // second has been replaced by newnode, free its memory
		// wire sibling pointers: first <-> third
		first->headOfSibling = first;
		first->leftSibling = nullptr;
		first->rightSibling=third;
		first->parent = newnode;
		third ->headOfSibling = first;
		third ->leftSibling = first;
		third -> rightSibling = nullptr;
		third ->parent = newnode;
		semanticStackPtr->push(newnode);

	}

}

// Pops nodes from the semantic stack, links them as siblings, attaches them as children
// of newnode, then pushes the resulting subtree back onto the semantic stack.
// amountOfPops == 'E' means: pop until an epsilon sentinel is found.
// match specifies the expected nodeType(s) of nodes to collect.
void parser::semanticActions:: makeSubTree(std::string nodeType, std::string semanticMeaning,int amountOfPops,std::vector<std::string> match, abstractSyntaxTree::node* newnode){
	std::vector<abstractSyntaxTree::node *> nodesFromTheStack; // final ordered list of children
	std::stack <abstractSyntaxTree::node*> reverseChildren;    // temp stack used to reverse pop order

	// 'E' (ascii 69) is a sentinel: pop everything down to the nearest epsilon marker
	if(amountOfPops == 'E'){
		abstractSyntaxTree::node * n = semanticStackPtr->top();
		// collect nodes until the epsilon boundary node is reached
		while(n->nodeType!="epsilon"){
			reverseChildren.push(n);
			semanticStackPtr->pop();
			n = semanticStackPtr->top();
		}
		semanticStackPtr->pop(); // discard the epsilon sentinel itself
		// reverse the collected nodes back into correct (left-to-right) order
		while(!reverseChildren.empty()){
			nodesFromTheStack.emplace_back(reverseChildren.top());
			reverseChildren.pop();
		}
	}
	// When the number of nodes to pop equals the number of types in match, do a strict ordered match
	else if(amountOfPops == match.size()){
		bool good = false;
		// Try matching from the top of the stack downward (reverse order against match)
		if(semanticStackPtr->top()->nodeType==match.at(amountOfPops-1)){
			for(int i = amountOfPops-1; i>=0;i--){
				abstractSyntaxTree::node * n = semanticStackPtr->top();
				if(n->nodeType==match.at(i)){
					reverseChildren.push(n);
					semanticStackPtr->pop();
					// mark success once we've processed past the first element
					if(i == 1){
						good = true;
					}
				}
			}
		}
		else{
		// Fallback: try matching in forward order if the reverse attempt didn't align
		if(!good)
			for(int i = 0; i<amountOfPops;i++){
				abstractSyntaxTree::node * n = semanticStackPtr->top();
				if(n->nodeType==match.at(i)){
					reverseChildren.push(n);
					semanticStackPtr->pop();
					// mark success once the last expected node is matched
					if(i == amountOfPops-1){
						good = true;
					}
				}
			}
		}

		// Abort if no valid match was found — indicates a malformed semantic stack
		if(!good){
			std::cout<<"Mistake matching in " + nodeType <<std::endl;
			exit(1);
		}
		// Drain reverseChildren into the final ordered list
		while(!reverseChildren.empty()){
			nodesFromTheStack.emplace_back(reverseChildren.top());
			reverseChildren.pop();
		}

	}
	// amountOfPops != match.size(): pop amountOfPops nodes, each matching any type in match
	else{
		for(int i = 0; i<amountOfPops;i++){
			abstractSyntaxTree::node * n = semanticStackPtr->top();
			int k = 0;
			// scan the match list to find a type that fits the current top-of-stack node
			for(; k<match.size(); k++){
				if(n->nodeType==match.at(k)){
					nodesFromTheStack.emplace_back(n);
					semanticStackPtr->pop();
					break; // found a match, move on to the next pop
				}
				else{
					continue;
				}
			}

			// if k reached the end, no match was found — semantic stack is inconsistent
			if(k == match.size()){
				std::cout<<"problem with stack cant find the matching items  : " + nodeType<<std::endl;
				exit(1);
			}
		}
	}

	// Wire sibling pointers so each child knows its neighbours and the head of the sibling list
	for(int i = 0; i<nodesFromTheStack.size();i++){
		if(i ==0){
			// first child: it is its own sibling head; point right to next sibling
			if(i + 1<nodesFromTheStack.size()){
				nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(i);
				nodesFromTheStack.at(i)->rightSibling = nodesFromTheStack.at(i+1);
			}
		}
		else if(i==nodesFromTheStack.size()-1){
			// last child: point back to previous sibling; head is always the first child
			if(i -1 > -1 ){
				nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(0);
				nodesFromTheStack.at(i)->leftSibling = nodesFromTheStack.at(i-1);
			}
		}
		else{
			// middle children: link both left and right siblings
			nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(0);
			nodesFromTheStack.at(i)->rightSibling = nodesFromTheStack.at(i+1);
			nodesFromTheStack.at(i)->leftSibling = nodesFromTheStack.at(i-1);

		}

	}

	// Attach the collected nodes as children of the new parent node
	newnode->children = nodesFromTheStack;
	newnode->nodeType = nodeType;
	newnode->semanticMeaning = semanticMeaning;

	// Point every child's parent back up to newnode
	for(abstractSyntaxTree::node * value : nodesFromTheStack){
		value->parent = newnode;
	}

	// If no children were found, treat this node as an epsilon (empty) production
	if(nodesFromTheStack.size()==0){
		newnode->semanticMeaning="epsilon";
		}
	// Push the completed subtree root onto the semantic stack for later composition
	semanticStackPtr->push(newnode);
}

// Shallow-copies all structural and semantic fields from oldnode into newnode,
// overriding the node type with nameOfNewNode. Used when promoting a node to a new type.
void parser::abstractSyntaxTree::node::copyNode(node* oldnode,node* newnode, std::string nameOfNewNode){
	newnode ->headOfSibling = oldnode ->headOfSibling;
	newnode->leftSibling = oldnode->leftSibling;
	newnode->rightSibling = oldnode->rightSibling;
	newnode->parent = oldnode->parent; // NOTE: likely a bug — should be oldnode->parent
	newnode->children = oldnode->children;
	newnode->nodeType = nameOfNewNode;          // override type with the new name
	newnode->semanticMeaning = oldnode ->semanticMeaning;
	newnode->nodeValue = oldnode->nodeValue;
}

// Pops the top node, copies its data into newnode (re-typing it as nodeType),
// re-parents its children, then either sets the AST root (if "start") or pushes newnode back.
void parser::semanticActions:: passAlong(std::string nodeType,abstractSyntaxTree::node * newnode){
	abstractSyntaxTree::node * topofthestack = semanticStackPtr->top();
	semanticStackPtr->pop();
	newnode->copyNode(topofthestack,newnode,nodeType); // transfer all fields to the typed node
	adoptChildren(newnode,topofthestack);             // re-point children's parent to newnode

	// if the top was epsilon, propagate that meaning so the tree knows this branch is empty
	if(topofthestack->nodeType =="epsilon" || topofthestack->semanticMeaning == "epsilon"){
		newnode->semanticMeaning ="epsilon";
	}
	delete topofthestack; // original generic node replaced by typed newnode
	if(nodeType == "start"){astPtr->treeHead = dynamic_cast<abstractSyntaxTree::startNode*>(newnode);} // anchor the tree root
	else {semanticStackPtr->push(newnode);}

}

// Overload of passAlong that also overrides the semantic meaning after copying.
// The explicit semanticMeaning can be overridden back to "epsilon" if the source node was epsilon.
void parser::semanticActions:: passAlong(std::string nodeType,std::string semanticMeaning,abstractSyntaxTree::node * newnode){
	abstractSyntaxTree::node * topofthestack = semanticStackPtr->top();
	semanticStackPtr->pop();
	newnode->copyNode(topofthestack,newnode,nodeType);
	newnode->semanticMeaning = semanticMeaning; // apply the caller-provided semantic label
	adoptChildren(newnode,topofthestack);
	// epsilon from source overrides the provided semanticMeaning
	if(topofthestack->nodeType =="epsilon" || topofthestack->semanticMeaning == "epsilon"){
		newnode->semanticMeaning ="epsilon";
	}
	delete topofthestack;
	if(nodeType == "start"){
		astPtr->treeHead = dynamic_cast<abstractSyntaxTree::startNode*>(newnode); // anchor the tree root

	}
	else {
		semanticStackPtr->push(newnode);
	}

}

// Transfers ownership of oldparent's children to newparent by updating each child's parent pointer.
// If oldparent had no children, marks newparent as a leaf.
void parser::semanticActions::adoptChildren(abstractSyntaxTree::node* newparent,abstractSyntaxTree::node* oldparent){
	if(oldparent->children.size()==0){
		newparent->isLeaf = true; // no children means this node is a terminal in the AST
		return;
	}

	else if(oldparent->children.size()>0){
		// re-point every child to the new parent node
		for(abstractSyntaxTree::node* child : oldparent->children){
			child->parent = newparent;
		}
	}
}

void parser::abstractSyntaxTree::printTree(){
    std::ofstream astout("./outputs/AbstractSyntaxTreeViz.ast.outast"); // Open file for writing
	abstractSyntaxTree::node * head = this->treeHead;
	std::vector<abstractSyntaxTree::node*> v = head->children;
	int counter = 0;
	astout<<head->semanticMeaning<<std::endl;
	for(int k = 0;k<v.size();k++){
//		if(v.at(k)->semanticMeaning=="epsilon"){
//			delete v.at(k);
//		    v.erase(v.begin()+k);
//		    continue;
//		}
		traverseTree(v.at(k),counter,astout);
	}
}

void parser::abstractSyntaxTree::traverseTree(abstractSyntaxTree::node* head, int& counter,std::ofstream & ao){
	counter++;
	if(head->children.size()!= 0){
		for(int i = 0; i<counter; i++){
			ao<<"| ";
		}

		if(head->semanticMeaning !="epsilon")ao<<head->semanticMeaning<<std::endl;
		std::vector<abstractSyntaxTree::node*> v = head->children;
		for(int k = 0;k<v.size();k++){
//			if(v.at(k)->semanticMeaning=="epsilon"){
//				delete v.at(k);
//			    v.erase(v.begin()+k);
//			    continue;
//			}
			traverseTree(v.at(k),counter,ao);
		}
		counter--;
	}
	else if(head->children.size() == 0){
		if(head->semanticMeaning =="epsilon"){
			std::cout<<"EPSILON"<<std::endl;
			node* parent = head->parent;


		}

		if(head->semanticMeaning !="epsilon"){


		for(int i = 0; i<counter; i++){
					ao<<"| ";
				}
		ao<<head->semanticMeaning<<std::endl;
		std::cout << typeid(*head).name() << std::endl; // Dereference to get dynamic type

		}
		counter--;
	}

}

// Prints the global symbol table to symbolTable.txt with clean indented nested sub-tables.
void parser::abstractSyntaxTree::printSymbolTable(node* head){
    std::ofstream out("./outputs/symbolTable.txt");

    out << "==============================\n"
        << "      GLOBAL SYMBOL TABLE     \n"
        << "==============================\n\n";

    for (auto& [name, entry] : head->stMap) {
        if(name.empty()) continue;
        // top-level entry: name  [kind]  :  type
        out << name << "  [" << entry->kind << "]  :  "
            << (entry->type.empty() ? "-" : entry->type) << "\n";
        if(entry->hasLink){
            out << "  +-- " << name << " --------------------\n";
            int depth = 0;
            printSymbolTableRec(entry->link, depth, out);
            out << "  +------------------------------\n\n";
        }
    }
    out << "==============================\n";
}

// Recursively prints symbol sub-tables, indenting 4 spaces per nesting level.
void parser::abstractSyntaxTree::printSymbolTableRec(std::map<std::string,parser::abstractSyntaxTree::node::symbolTableEntry*> * link,int & depth,std::ofstream & ao){
    depth++;
    std::string pad(depth * 4, ' '); // 4 spaces per depth level

    for (auto& [name, entry] : *link) {
        if(name.empty()) continue;
        // indented entry: name  [kind]  :  type
        ao << pad << name << "  [" << entry->kind << "]  :  "
           << (entry->type.empty() ? "-" : entry->type) << "\n";
        if(entry->hasLink){
            ao << pad << "+-- " << entry->name << " --------------------\n";
            printSymbolTableRec(entry->link, depth, ao); // recurse into nested table
            ao << pad << "+------------------------------\n";
        }
    }
    depth--;
}




std::string parser::abstractSyntaxTree::SymTabCreationVisitor::get(std::string search, node& head){
	for(node * child : head.children){
		if(child->semanticMeaning == search){
			return child->nodeValue;
		}
	}
	return get(search,(*head.children[0]));


}
void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(funcdeclNode & head){
	node::symbolTableEntry *ste = &head.stEntry;

	//kind
	ste->kind = "function";
	//haslink
	ste->hasLink =true;
	//name
	ste->name=get("id",head);
	ste->type = get("returntype",head);

	for(node*children:head.children){

		 if(children->nodeType=="fparams"){
			//link
			 ste->link =  &children->stMap;
		}
	}
}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(fparamsNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	for(node*child:head.children){
		if(child->nodeType=="param"){
			 if((*mp).count(child->stEntry.name)>0){
				 std::cerr<<"Multiple Parameters with the same name for: "+child->stEntry.name +"."<<std::endl;
			 }
			 else{
					(*mp)[child->stEntry.name] =& child->stEntry;
			 }
		}
	}
}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(paramNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="parameter";
	ste->name = get("id",head);
	ste->type = get("type",head);


}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(classNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="class";
	ste->name = get("id",head);
	ste->hasLink =true;
	ste->link=mp;
	for(node*child:head.children){
		if(child->semanticMeaning=="isalist"){
			for(node*children:child->children){
				ste->type+=children->nodeValue;

			}
		}
		else if(child->semanticMeaning=="{"){
			for(node*children:child->children){
				if(children->semanticMeaning=="attributedecl"){
					 if((*mp).count(children->stEntry.name)>0){
						 std::cerr<<"Multiple Attribute Declarations for: "+children->stEntry.name +"."<<std::endl;
					 }
					 else{
							(*mp)[children->stEntry.name] =& children->stEntry;

					 }

				}
				else if(children->semanticMeaning == "funcdecl"){
					for(node*grandchildren:children->children){
						if(grandchildren->semanticMeaning=="funchead"){
							 if((*mp).count(grandchildren->stEntry.name)>0){
								 std::cerr<<"Multiple Function Declarations for: "+grandchildren->stEntry.name +"."<<std::endl;
							 }
							 else{
									(*mp)[grandchildren->stEntry.name] = &grandchildren->stEntry;

							 }
						}
					}
				}
			}
		}
	}

}
int uniquenamecount = 0;

std::string generateUniqueName(std::string baseName) {
    std::string newName = baseName +std::to_string(uniquenamecount);
    uniquenamecount++;
    return newName;
}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(startNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	for(node*child:head.children){
		 if(child->semanticMeaning=="classdecl"){
			//link
			 if((*mp).count(child->stEntry.name)>0){
				 std::cerr<<"Multiple Class Declaration for: "+child->stEntry.name +"."<<std::endl;

			 }
			 else{
				 (*mp)[child->stEntry.name] = &child->stEntry;

			 }
		}
		 else if(child->semanticMeaning=="funcdef"){
			 (*mp)[child->stEntry.name] = &child->stEntry;
		 }
		 else if(child->semanticMeaning=="impldef"){
			 if((*mp).count(child->stEntry.name)>0){
				 if((*mp)[child->stEntry.name]->kind=="class"){
					 std::cout<<"Class Declaration found for " + child->stEntry.name + ". Updating implementation."<<std::endl;
					 (*mp)[child->stEntry.name] = &child->stEntry;
				 }

			 }
			 else{
				 std::cerr<<"Missing Class Declaration for: "+child->stEntry.name +"."<<std::endl;
			 }

		 }

	}
    for (const auto& [key, value] : (*mp)) {
    	if(value->kind=="class"){
    		std::cerr<<"There is a class declaration: "+value->name+" without implementation"<<std::endl;
    	}
    }

}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(attributedeclNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="variable";
	ste->name = get("id",head);
	ste->type = get("type",head);

}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(attributedeclfamNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="attribute";
	for(node*child:head.children){
		if(child->semanticMeaning =="vardecl"){
			ste->name=child->stEntry.name;
			ste->type=child->stEntry.type;

		}
	}
}
void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(implNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="implementation";
	ste->name = get("id",head);
	ste->hasLink =true;
	ste->link=mp;
	for(node*child:head.children){
		 if(child->semanticMeaning=="reptimpldef3"){
			for(node*children:child->children){
				if(children->semanticMeaning=="funcdef"){
					 if((*mp).count(children->stEntry.name)>0){
						 std::cerr<<"Multiple Function Definitions for: "+children->stEntry.name +"."<<std::endl;
					 }
					 else{
							(*mp)[children->stEntry.name] =& children->stEntry;
					 }
				}
			}
		}
	}
}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(funcdefNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="function";
	ste->hasLink =true;
	ste->link=mp;

	for(node*child:head.children){
		 if(child->semanticMeaning=="funchead"){
			 mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			 ste->name = child->stEntry.name;
			 ste->type = child->stEntry.type;
		 }
		 else if(child->semanticMeaning=="funcbody"){
			 mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			 child->stEntry.name=ste->name;

		 }

	}
}
void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(funcheadNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="function head";
	ste->name = get("id",head);
//	ste->name = generateUniqueName(ste->name);
	for(node*child:head.children){
		 if(child->semanticMeaning=="returntype"){
			 ste->type=child->nodeValue;
		}
		 else if(child->semanticMeaning =="funcparams" ){
			//link
			ste->link =  &child->stMap;

		 }
	}
}
void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(funcbodyNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->hasLink =true;
	ste->link=mp;
	ste->kind ="function body";
	for(node*child:head.children){
		 if(child->semanticMeaning=="vardecl"){
			 if((*mp).count(child->stEntry.name)>0){
				 std::cerr<<"Multiple Variable Declarations for: "+child->stEntry.name +"."<<std::endl;
			 }
			 else{
					(*mp)[child->stEntry.name] =& child->stEntry;
			 }
		}
	}
}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(localvardeclNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="variable";
	ste->name = get("id",head);
	ste->type = get("type",head);
}

void parser::abstractSyntaxTree::SymTabCreationVisitor:: visit(funcNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="function";
	ste->hasLink =true;
	ste->link=mp;

	for(node*child:head.children){
		 if(child->semanticMeaning=="funchead"){
			 if(!mp->empty())mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			 ste->name = child->stEntry.name;
			 ste->type = child->stEntry.type;
		 }
		 else if(child->semanticMeaning=="funcbody"){
			 mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			 child->stEntry.name=ste->name;
		 }
	}
}

