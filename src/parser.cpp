#include "parser.h"




/*
 *
 *
 *
 * PARSING TABLE SECTION BELOW
 *
 *
 *
 */

int parser::parsing_table:: TABLEROWSIZE;
int parser::parsing_table:: TABLECOLUMNSIZE;
int parser::parsing_table:: UNIT;
std::ifstream parser::parsing_table::inputFileStream;
std::unordered_set<std::string> parser::parsing_table::uniqueTerminalSymbols;
std::unordered_map<std::string, int> parser::parsing_table::tableRows;
std::unordered_map<std::string, int> parser::parsing_table::tableColumns;  // Ensure values are ints, not references

bool parser::parsing_table::connectFile(std::string fileName){

	if(inputFileStream.is_open()){
			std::cout<<"The file is connected... Closing it first..."<<std::endl;
			inputFileStream.close();
		}

	inputFileStream.open(fileName,std::ios::in);

	if(inputFileStream){
		std::cout<<"Successfully opened file: "<<fileName<<"."<<std::endl;
		return true;
	}
	else{
		std::cout<<"Failed to open file: "<<fileName<<"."<<std::endl;
		return false;
	}
}

bool parser::parsing_table::disconnectFile(){

	if(!inputFileStream.is_open()){
			std::cout<<"The file is not even connected. No need to close."<<std::endl;
			return true;
		}
	inputFileStream.close();

	if(!inputFileStream.is_open()){
		std::cout<<"Successfully closed the inputFileStream."<<std::endl;
		return true;
	}
	else{
		std::cout<<"Failed to close the inputFileStream."<<std::endl;
		return false;
	}
}

void parser::parsing_table:: buildTable(){
	setUniqueTerminalSymbols();
	setRowAndColumnSize();
	setTheTableSize();
	setTableRow();
	setTableColumn();
	fillTableWithErrors();
	createParsingTable();
}

void parser::parsing_table::fillTableWithErrors(){
	for (int i = 0; i < TABLEROWSIZE; i++) {
	    for (int j = 0; j < TABLECOLUMNSIZE; j++) {
			tableEntry currentEntry = {i,j,"error"};
	    	table[i][j] = currentEntry;
	    }
	}
    spdlog::info("Successfully filled Table with error symbols.");

}

void parser::parsing_table:: setUniqueTerminalSymbols(){
	std::cout<<"hello"<<std::endl;
    for (const auto& pair : fafPtr->firstSet) {
        for (const auto& value : (*pair.second)) {
        	if(value == "EPSILON"){
        		continue;
        	}
        	uniqueTerminalSymbols.insert(value);  // insert automatically handles duplicates
        }
    }
    for (const auto& pair : fafPtr->followSet) {
        for (const auto& value : (*pair.second)) {
        	uniqueTerminalSymbols.insert(value);  // insert automatically handles duplicates
        }
    }

    uniqueTerminalSymbols.insert(":");
    uniqueTerminalSymbols.insert("=>");
    uniqueTerminalSymbols.insert(":=");
    uniqueTerminalSymbols.insert("then");
    uniqueTerminalSymbols.insert("invalidnum");


    spdlog::info("Successfully set unique Terminal Symbols set.");

}
//THE TABLESROWS REPRESENT THE VALUE OF THE FIRST ARRAY
int parser::parsing_table:: getTableRowSize(){
	return fafPtr->firstSet.size();
}
//TABLE COLUMNS REPRESENT THE VALUE OF THE SECOND ARRAY
int parser::parsing_table:: getTableColumnSize(){
	return (uniqueTerminalSymbols.size());
}

void parser::parsing_table:: setRowAndColumnSize(){
	TABLEROWSIZE = getTableRowSize();
	TABLECOLUMNSIZE = getTableColumnSize();
	UNIT = sizeof(struct tableEntry);
    spdlog::info("Successfully set Parsing Table row and column size.");

}

void parser::parsing_table:: setTheTableSize(){
	table = new tableEntry*[TABLEROWSIZE]; // Allocate row pointers
	for (int i = 0; i < TABLEROWSIZE; ++i) {
	    table[i] = new tableEntry[TABLECOLUMNSIZE]; // Allocate each row
	}
    spdlog::info("Successfully created table with appropriate size {} x {}.",TABLEROWSIZE,TABLECOLUMNSIZE);
}

void parser::parsing_table::setTableRow(){
	int i = 0;
//	std::cout<< "FOR THE ROWS"<<std::endl;
	for(const auto& value : fafPtr->firstSet){
		tableRows[value.first] = i;
//		std::cout<<"Row = "<< value.first <<", value = "<<i<<std::endl;
		i++;
	}
    spdlog::info("Successfully filled first Table Row.");

}

void parser::parsing_table::setTableColumn(){
	int i = 0;
//	std::cout<< "FOR THE COLUMNS"<<std::endl;
	for(const auto& value: uniqueTerminalSymbols){
		tableColumns[value]=i;
//		std::cout<<"Column = "<< value<<", value = "<<i<<std::endl;

		i++;
	}
    spdlog::info("Successfully filled first Table Column.");

}

void parser::parsing_table::getRule(std::string & rule, std::string line, int & lineIndex){
	 if(lineIndex == line.size())
		 return;

	 while(line.at(lineIndex)!='<'){
		 if(lineIndex == line.size())
			 return;

		 lineIndex++;
	 }
	 lineIndex++;
	 while(line.at(lineIndex)!='>'){
		 rule+=line.at(lineIndex);
		 lineIndex++;
	 }
	 while(line.at(lineIndex)!='='){
			 lineIndex++;
	 }

	 lineIndex++;
}

void parser::parsing_table::getAlpha(std::string & alpha, const std::string line, int & lineIndex){
	while(lineIndex!=line.size()){
		alpha+=line.at(lineIndex);
		lineIndex++;
	}
}

bool parser::parsing_table::checkFirstOfAlpha(const std::string alpha,std::unordered_set<std::string> & firstOfAlpha,bool & isTerminal){
//	std::cout<<"hello"<<std::endl;
	int tempLineIndex = 0;
	while(alpha.at(tempLineIndex)!= '\'' && alpha.at(tempLineIndex)!='<' && alpha.at(tempLineIndex)!= 'E'){
		tempLineIndex++;
	}
	if (alpha.at(tempLineIndex)=='\''){
		isTerminal = true;
		tempLineIndex++;
		std::string terminal;
		while(alpha.at(tempLineIndex)!='\''){
			terminal +=alpha.at(tempLineIndex);
			tempLineIndex++;
		}
		firstOfAlpha.insert(terminal);
		return false;
	}

	else if(alpha.at(tempLineIndex)== '<'){
		isTerminal = false;
		bool containsEpsilon = false;
		tempLineIndex++;
		std::string NonTerminal;
		while(alpha.at(tempLineIndex)!='>'){
			NonTerminal += alpha.at(tempLineIndex);
			tempLineIndex++;
		}

		std::vector<std::string> * NonTerminalFirstSet = fafPtr->firstSet[NonTerminal];
		for(const auto & value:(*NonTerminalFirstSet)){
			firstOfAlpha.insert(value);
			if(value == "EPSILON"){
				containsEpsilon = true;
			}
		}
		if(containsEpsilon){
			while(containsEpsilon){
				containsEpsilon = false;
				firstOfAlpha.erase("EPSILON");
				std::string nextSymbol;

				while(tempLineIndex != alpha.size() && alpha.at(tempLineIndex)!= '\'' && alpha.at(tempLineIndex)!='<' && alpha.at(tempLineIndex)!= 'E'){
					tempLineIndex++;
				}

				if(tempLineIndex == alpha.size()){
					containsEpsilon = true;
					return true;
				}

				if (alpha.at(tempLineIndex)=='\''){
						tempLineIndex++;
						while(alpha.at(tempLineIndex)!='\''){
							nextSymbol +=alpha.at(tempLineIndex);
							tempLineIndex++;
						}
						firstOfAlpha.insert(nextSymbol);
						return false;
					}
				else if(alpha.at(tempLineIndex)== '<'){
					tempLineIndex++;
					while(alpha.at(tempLineIndex)!='>'){
						nextSymbol += alpha.at(tempLineIndex);
						tempLineIndex++;
					}
					if(nextSymbol =="rept-statement4"){

					}

					std::vector<std::string> * NonTerminalFirstSetSecond = fafPtr->firstSet[nextSymbol];
					for(const auto & value:(*NonTerminalFirstSetSecond)){
						if(value == "EPSILON"){
							containsEpsilon = true;
						}
						firstOfAlpha.insert(value);
					}


				}


			}
			if(!containsEpsilon)return true;

		}
		else return false;
//		for (const auto& val : firstOfAlpha) {
//		        std::cout << val << std::endl;
//		    }

	}
	else if(alpha.at(tempLineIndex)=='E'){
		firstOfAlpha.insert("EPSILON");
		return true;
	}
	else{
		std::cout<<"Code should not reach here. You are in parser::parsing_table::checkFirstOfAlpha"<<std::endl;
	}
    // ensure function always returns a bool
    return false;
}
void parser::parsing_table::updateTable(const std::string rule, std::unordered_set <std::string> firstOfAlpha,std::string line){
//
	for (const auto& value : firstOfAlpha){
		int rowValue = tableRows[rule];
		int columnValue = tableColumns[value];
//		std::cout<<value<<rowValue<<","<<columnValue<<std::endl;
		tableEntry currentEntry = {rowValue,columnValue,line};
		table[rowValue][columnValue] = currentEntry;
	}
}

bool parser::parsing_table::hasEpsilon(std::unordered_set <std::string> & firstOfAlpha, bool isTerminal){
	for (auto value : firstOfAlpha){
		if(value == "EPSILON")
			return true;
	}
	return false;

}

void parser::parsing_table::getFollowOfRule(std::unordered_set <std::string> & followOfRule,const std::string rule ){
	std::vector<std::string> * ruleFollowVector = fafPtr->followSet[rule];
	for (auto value : (*ruleFollowVector)){
		followOfRule.insert(value);
	}
}

void parser::parsing_table::createParsingTable(){
	std::cout<<"Enter the location of the Attribute Grammar file: "<<std::endl;
	std::string fileLocation;
	std::getline(std::cin, fileLocation);

	if(connectFile(fileLocation)){
		//We've properly connected to the file now we need to get rid of the white space and then hand control back over getNextToken().
	}
	else throw std::invalid_argument("Invalid File location");
	std::string line;

	while (std::getline(inputFileStream, line)){
		int lineIndex=0;
		bool hasEpsilon;
		std::string rule;
		std::string alpha;
		std::unordered_set <std::string> firstOfAlpha;
		std::unordered_set <std::string> followOfRule;
		bool isTerminal;

		getRule(rule,line,lineIndex);
		 if(lineIndex == line.size())
			 continue;
//		 std::cout<<"THE RULE IS: "<<rule<<std::endl<<std::endl;
		getAlpha(alpha,line,lineIndex);
//		std::cout<<"THE ALPHA IS: "<<alpha<<std::endl;
		hasEpsilon = checkFirstOfAlpha(alpha,firstOfAlpha,isTerminal);
//		 for (const auto& elem : firstOfAlpha) {
//		        std::cout << elem << " ";
//
//		    }
//		 std::cout<<std::endl;

//		for (const auto& val : firstOfAlpha) {
//		        std::cout << val << std::endl;
//		    }

		updateTable(rule,firstOfAlpha,line);


		if(hasEpsilon){
//			std::cout<<"ALPHA HAS EPSILON NEED TO ADD FOLLOW SET OF "<<rule<<std::endl;
			getFollowOfRule(followOfRule,rule);
			updateTable(rule,followOfRule,line);
		}

//		insertErrors();

	}
    std::ofstream file("./outputs/SYMBOLTABLEOUTPUT.txt");
	auto rowit = tableRows.begin();

    for (int i = 0; i < TABLEROWSIZE; i++) {
    	auto columnit = tableColumns.begin();

        for (int j = 0; j < TABLECOLUMNSIZE; j++) {

            file << "ROW: " << rowit ->first<< ", at value: " << rowit->second<<std::endl
                      << "COLUMN : " << columnit -> first<<", at value: "<<columnit -> second <<std::endl
                      << "Table entry: " << table[rowit->second][columnit -> second].derivationRule << std::endl;
            columnit++;
        }
        rowit++;
    }
    file.close();

	disconnectFile();


}

/*
 *
 *
 *
 * MAIN PARSING SECTION BELOW
 *
 *
 *
 */
//Forward Declaration
bool parser::parse(const std::vector<token*>  & vectorOfTokens){
    std::ofstream derivationOut("./outputs/Derivation"); // Open file for writing
    int i =0;
	std::vector<std::string> derivation;
	int lineIndex =0;
	bool error;
	parsingStack.push("$");
	parsingStack.push("START");
	derivation.emplace_back("START");
	auto vectorIterator = vectorOfTokens.begin();
	token* currentToken = (*vectorIterator);

	while(parsingStack.top()!="$"){
		if(i >= 2650)
		{
			std::cout<<i<<std::endl;

		}
		i++;
		std::string topOfTheStack = parsingStack.top();
		if(parsingTable.uniqueTerminalSymbols.count(topOfTheStack)!=0){
			if(topOfTheStack==(currentToken->getTypeName())){
				parsingStack.pop();
				derivation.erase(derivation.begin()+lineIndex);
				derivation.insert(derivation.begin()+lineIndex,currentToken->getLexeme());
				lineIndex++;
				vectorIterator++;
				currentToken = (*vectorIterator);
//				for(auto const & value:derivation){
//					derivationOut<<value<<" ";
//				}
//				derivationOut<<std::endl;
			}
			else{
				//Handle error;
				std::string topOfStack;
				topOfStack = parsingStack.top();
				skipError(currentToken,vectorIterator,derivationOut,topOfStack,topOfTheStack,currentToken->getLine(),currentToken->getColumn());
				error = true;
			}

		}

		else if(topOfTheStack.at(0) == '#'){
			std::string lastTokenValue;
			if(vectorIterator!= vectorOfTokens.begin()){
				lastTokenValue = (*(vectorIterator - 1))->getLexeme();
			}
			std::string semanticFunction;
			parsingStack.pop();
			for(int i = 1; i<topOfTheStack.size();i++){
				semanticFunction +=topOfTheStack[i];
			}
			if(semanticFunction=="factor2"){
				std::cout<<"hello"<<std::endl;
			}
			semanticHandler.handleAction(semanticFunction,lastTokenValue);
			derivation.erase(derivation.begin()+lineIndex);

		}
		else {
			if(topOfTheStack == "EPSILON"){
				parsingStack.pop();
				derivation.erase(derivation.begin()+lineIndex);
				for(auto const & value:derivation){
					derivationOut<<value<<" ";
				}
				derivationOut<<std::endl;
				continue;
			}

			//Need to find the row and column value for the table
			//row will be whatever the top of the stack is.
			int rowValue = parsingTable.tableRows[topOfTheStack];
			int columnValue = parsingTable.tableColumns[((currentToken)->getTypeName())];
			tableEntry currentTableEntry = parsingTable.table[rowValue][columnValue];
			if(currentTableEntry.derivationRule!="error"){
				parsingStack.pop();
				derivation.erase(derivation.begin()+lineIndex);
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

			//items.push("EPSILON");
//			break;
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

void parser::semanticActions:: makeLeaf(std::string nodeType,std::string lastTokenValue,abstractSyntaxTree::node * newnode){
	newnode->nodeType = nodeType;
	newnode->semanticMeaning = nodeType;
	newnode->nodeValue = lastTokenValue;
	newnode->isLeaf = true;

	semanticStackPtr->push(newnode);
}

void parser::semanticActions::makeBinarySubTreeWithHead(std::string nodeType,int numOfPops,abstractSyntaxTree::node* newnode){
	std::vector <abstractSyntaxTree::node *>nodesOnStack;
	std::vector <abstractSyntaxTree::node *>children;
	std::stack <abstractSyntaxTree::node*> reverseChildren;

	for(int i = numOfPops -1; i>=0; i--){
		reverseChildren.push(semanticStackPtr->top());
		semanticStackPtr->pop();
	}
	while(!reverseChildren.empty()){
		nodesOnStack.emplace_back(reverseChildren.top());
		reverseChildren.pop();
	}

	abstractSyntaxTree::node * parentNode = nodesOnStack[nodesOnStack.size()-1];
	newnode->copyNode(parentNode, newnode, nodeType);
	delete parentNode;

	for(int k = nodesOnStack.size()-2; k>=0; k--){
		children.emplace_back(nodesOnStack.at(k));
	}
	for(int i = 0; i<children.size();i++){
		if(i ==0){
			if(i + 1<children.size()){
				children.at(i)->headOfSibling = children.at(i);
				children.at(i)->rightSibling = children.at(i+1);
			}
		}
		else if(i==children.size()-1){
			if(i -1 > -1 ){
				children.at(i)->headOfSibling = children.at(0);
				children.at(i)->leftSibling = children.at(i-1);
			}
		}
		else{
			children.at(i)->headOfSibling = children.at(0);
			children.at(i)->rightSibling = children.at(i+1);
			children.at(i)->leftSibling = children.at(i-1);

		}

	}
	for(abstractSyntaxTree::node * value : children){
		value->parent = newnode;
	}
	newnode->children = children;
	semanticStackPtr->push(newnode);


}

void parser::semanticActions::makeBinarySubTree(std::string nodeType,int i,abstractSyntaxTree::node*newnode){
	if(i ==0){
		abstractSyntaxTree::node * first = semanticStackPtr->top();
		semanticStackPtr->pop();
		abstractSyntaxTree::node * second = semanticStackPtr->top();
		semanticStackPtr->pop();

		std::vector<abstractSyntaxTree::node* > v = {first,second};
		newnode->children=v;
		newnode->nodeType=nodeType;
		newnode->semanticMeaning=nodeType;

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
		abstractSyntaxTree::node * first = semanticStackPtr->top();
		semanticStackPtr->pop();
		abstractSyntaxTree::node * second = semanticStackPtr->top();
		semanticStackPtr->pop();
		abstractSyntaxTree::node * third  = semanticStackPtr->top();
		semanticStackPtr->pop();



		std::vector<abstractSyntaxTree::node* > v = {first,third};
		newnode->nodeType=nodeType;
		newnode->semanticMeaning=nodeType;
		newnode->copyNode(second, newnode, nodeType);
		newnode->children=v;
		delete second;
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

void parser::semanticActions:: makeSubTree(std::string nodeType, std::string semanticMeaning,int amountOfPops,std::vector<std::string> match, abstractSyntaxTree::node* newnode){
	std::vector<abstractSyntaxTree::node *> nodesFromTheStack;
	std::stack <abstractSyntaxTree::node*> reverseChildren;

	if(amountOfPops == 'E'){
		abstractSyntaxTree::node * n = semanticStackPtr->top();
		while(n->nodeType!="epsilon"){
			reverseChildren.push(n);
			semanticStackPtr->pop();
			n = semanticStackPtr->top();
		}
		semanticStackPtr->pop();
		while(!reverseChildren.empty()){
			nodesFromTheStack.emplace_back(reverseChildren.top());
			reverseChildren.pop();
		}
	}
	//First if statement handles if the amount that we are about to pop matches the amount we need to match.
	else if(amountOfPops == match.size()){
		bool good = false;
		if(semanticStackPtr->top()->nodeType==match.at(amountOfPops-1)){
			for(int i = amountOfPops-1; i>=0;i--){
				abstractSyntaxTree::node * n = semanticStackPtr->top();
				if(n->nodeType==match.at(i)){
					reverseChildren.push(n);
					semanticStackPtr->pop();
					if(i == 1){
						good = true;
					}
				}
			}
		}
		else{
		if(!good)
			for(int i = 0; i<amountOfPops;i++){
				abstractSyntaxTree::node * n = semanticStackPtr->top();
				if(n->nodeType==match.at(i)){
					reverseChildren.push(n);
					semanticStackPtr->pop();
					if(i == amountOfPops-1){
						good = true;
					}
				}
			}
		}


		if(!good){
			std::cout<<"Mistake matching in " + nodeType <<std::endl;
			exit(1);
		}
		while(!reverseChildren.empty()){
			nodesFromTheStack.emplace_back(reverseChildren.top());
			reverseChildren.pop();
		}

	}
	//If it does not match then we need to match just one of them
	else{
		for(int i = 0; i<amountOfPops;i++){
			abstractSyntaxTree::node * n = semanticStackPtr->top();
			int k = 0;
			for(; k<match.size(); k++){
				if(n->nodeType==match.at(k)){
					nodesFromTheStack.emplace_back(n);
					semanticStackPtr->pop();
					break;
				}
				else{
					continue;
				}
			}

			if(k == match.size()){
				std::cout<<"problem with stack cant find the matching items  : " + nodeType<<std::endl;
				exit(1);
			}
		}
	}


	for(int i = 0; i<nodesFromTheStack.size();i++){
		if(i ==0){
			if(i + 1<nodesFromTheStack.size()){
				nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(i);
				nodesFromTheStack.at(i)->rightSibling = nodesFromTheStack.at(i+1);
			}
		}
		else if(i==nodesFromTheStack.size()-1){
			if(i -1 > -1 ){
				nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(0);
				nodesFromTheStack.at(i)->leftSibling = nodesFromTheStack.at(i-1);
			}
		}
		else{
			nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(0);
			nodesFromTheStack.at(i)->rightSibling = nodesFromTheStack.at(i+1);
			nodesFromTheStack.at(i)->leftSibling = nodesFromTheStack.at(i-1);

		}

	}


	newnode->children = nodesFromTheStack;
	newnode->nodeType = nodeType;
	newnode->semanticMeaning = semanticMeaning;

	for(abstractSyntaxTree::node * value : nodesFromTheStack){
		value->parent = newnode;
	}

	if(nodesFromTheStack.size()==0){
		newnode->semanticMeaning="epsilon";
		}
	semanticStackPtr->push(newnode);
}

void parser::abstractSyntaxTree::node::copyNode(node* oldnode,node* newnode, std::string nameOfNewNode){
	newnode ->headOfSibling = oldnode ->headOfSibling;
	newnode->leftSibling = oldnode->leftSibling;
	newnode->rightSibling = oldnode->rightSibling;
	newnode->parent = oldnode->rightSibling;
	newnode->children = oldnode->children;
	newnode->nodeType = nameOfNewNode;
	newnode->semanticMeaning = oldnode ->semanticMeaning;
	newnode->nodeValue = oldnode->nodeValue;
}

void parser::semanticActions:: passAlong(std::string nodeType,abstractSyntaxTree::node * newnode){
	abstractSyntaxTree::node * topofthestack = semanticStackPtr->top();
	semanticStackPtr->pop();
	newnode->copyNode(topofthestack,newnode,nodeType);
	adoptChildren(newnode,topofthestack);

	if(topofthestack->nodeType =="epsilon" || topofthestack->semanticMeaning == "epsilon"){
		newnode->semanticMeaning ="epsilon";
	}
	delete topofthestack;
	if(nodeType == "start"){astPtr->treeHead = dynamic_cast<abstractSyntaxTree::startNode*>(newnode);}
	else {semanticStackPtr->push(newnode);}

}

void parser::semanticActions:: passAlong(std::string nodeType,std::string semanticMeaning,abstractSyntaxTree::node * newnode){
	abstractSyntaxTree::node * topofthestack = semanticStackPtr->top();
	semanticStackPtr->pop();
	newnode->copyNode(topofthestack,newnode,nodeType);
	newnode->semanticMeaning = semanticMeaning;
	adoptChildren(newnode,topofthestack);
	if(topofthestack->nodeType =="epsilon" || topofthestack->semanticMeaning == "epsilon"){
		newnode->semanticMeaning ="epsilon";
	}
	delete topofthestack;
	if(nodeType == "start"){
		astPtr->treeHead = dynamic_cast<abstractSyntaxTree::startNode*>(newnode);

	}
	else {
		semanticStackPtr->push(newnode);
	}

}

void parser::semanticActions::adoptChildren(abstractSyntaxTree::node* newparent,abstractSyntaxTree::node* oldparent){
	if(oldparent->children.size()==0){
		newparent->isLeaf = true;
		return;
	}

	else if(oldparent->children.size()>0){
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

void parser::abstractSyntaxTree::printSymbolTable(node* head){
	std::vector<node*>headChildren = head->children;
    std::ofstream symtabout("./outputs/symbolTable.txt"); // Open file for writing

	symtabout<<"Global Table"<<std::endl<<"========================="<<std::endl;
	for (auto it = head->stMap.begin(); it != head->stMap.end(); ++it) {
	    symtabout << it->first << ": " << it->second->kind + " | "<< it->second->type+ " | "<< std::endl;
	    if(it->second->hasLink){
	    	symtabout<<"========================="<<std::endl<<"\ttable for "<<it->second->name<<std::endl<<std::endl;
	    	int count = 0;
	    	printSymbolTableRec(it->second->link,count,symtabout);
	    }
	}
}
void parser::abstractSyntaxTree::printSymbolTableRec(std::map<std::string,parser::abstractSyntaxTree::node::symbolTableEntry*> * link,int & count,std::ofstream & ao){
count ++;
	for (auto it = link->begin(); it != link->end(); ++it) {
		if(it->first!="")
		for (int i = 0; i < count; i++) {
	        ao << "\t";
	    }
	    if(it->first!="")ao << it->first << ": " << it->second->kind + " | " + it->second->type + " | " << std::endl;
	    if(it->second->hasLink){
		    for (int i = 0; i < count; i++) {
		        ao << "\t";
		    }

	    	ao<<"========================="<<std::endl<<"\ttable for "<<it->second->name<<std::endl<<std::endl;
	    	printSymbolTableRec(it->second->link,count,ao);
		    for (int i = 0; i < count+1; i++) {
		        ao << "\t";
		    }

	    	ao<<"========================="<<std::endl;
	    }
	}

count --;
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

