#include "parsing_table.h"

int parsing_table:: TABLEROWSIZE;
int parsing_table:: TABLECOLUMNSIZE;
int parsing_table:: UNIT;
std::ifstream parsing_table::inputFileStream;
std::unordered_set<std::string> parsing_table::uniqueTerminalSymbols;
std::unordered_map<std::string, int> parsing_table::tableRows;
std::unordered_map<std::string, int> parsing_table::tableColumns;  // Ensure values are ints, not references

bool parsing_table::connectFile(std::string fileName){

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

bool parsing_table::disconnectFile(){

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

void parsing_table:: buildTable(){
	setUniqueTerminalSymbols();
	setRowAndColumnSize();
	setTheTableSize();
	setTableRow();
	setTableColumn();
	fillTableWithErrors();
	createParsingTable();
}

void parsing_table::fillTableWithErrors(){
	for (int i = 0; i < TABLEROWSIZE; i++) {
	    for (int j = 0; j < TABLECOLUMNSIZE; j++) {
			tableEntry currentEntry = {i,j,"error"};
	    	table[i][j] = currentEntry;
	    }
	}
    spdlog::info("Successfully filled Table with error symbols.");

}

void parsing_table:: setUniqueTerminalSymbols(){
    spdlog::info("Setting Unique Terminal Symbols");
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
    uniqueTerminalSymbols.insert("::");
    uniqueTerminalSymbols.insert(":");
    uniqueTerminalSymbols.insert("=>");
    uniqueTerminalSymbols.insert(":=");
    uniqueTerminalSymbols.insert("then");
    uniqueTerminalSymbols.insert("invalidnum");


    spdlog::info("Successfully set unique Terminal Symbols set.");

}
//THE TABLESROWS REPRESENT THE VALUE OF THE FIRST ARRAY
int parsing_table:: getTableRowSize(){
	return fafPtr->firstSet.size();
}
//TABLE COLUMNS REPRESENT THE VALUE OF THE SECOND ARRAY
int parsing_table:: getTableColumnSize(){
	return (uniqueTerminalSymbols.size());
}

void parsing_table:: setRowAndColumnSize(){
	TABLEROWSIZE = getTableRowSize();
	TABLECOLUMNSIZE = getTableColumnSize();
	UNIT = sizeof(struct tableEntry);
    spdlog::info("Successfully set Parsing Table row and column size.");

}

void parsing_table:: setTheTableSize(){
	table = new tableEntry*[TABLEROWSIZE]; // Allocate row pointers
	for (int i = 0; i < TABLEROWSIZE; ++i) {
	    table[i] = new tableEntry[TABLECOLUMNSIZE]; // Allocate each row
	}
    spdlog::info("Successfully created table with appropriate size {} x {}.",TABLEROWSIZE,TABLECOLUMNSIZE);
}

void parsing_table::setTableRow(){
	int i = 0;
	for(const auto& value : fafPtr->firstSet){
		tableRows[value.first] = i;
		i++;
	}
    spdlog::info("Successfully filled first Table Row.");

}

void parsing_table::setTableColumn(){
	int i = 0;
	for(const auto& value: uniqueTerminalSymbols){
		tableColumns[value]=i;

		i++;
	}
    spdlog::info("Successfully filled first Table Column.");

}

void parsing_table::getRule(std::string & rule, std::string line, int & lineIndex){
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

void parsing_table::getAlpha(std::string & alpha, const std::string line, int & lineIndex){
	while(lineIndex!=line.size()){
		alpha+=line.at(lineIndex);
		lineIndex++;
	}
}

bool parsing_table::checkFirstOfAlpha(const std::string alpha,std::unordered_set<std::string> & firstOfAlpha,bool & isTerminal){
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
void parsing_table::updateTable(const std::string rule, std::unordered_set <std::string> firstOfAlpha,std::string line){
//
	for (const auto& value : firstOfAlpha){
		int rowValue = tableRows[rule];
		int columnValue = tableColumns[value];
		tableEntry currentEntry = {rowValue,columnValue,line};
		table[rowValue][columnValue] = currentEntry;
	}
}

bool parsing_table::hasEpsilon(std::unordered_set <std::string> & firstOfAlpha, bool isTerminal){
	for (auto value : firstOfAlpha){
		if(value == "EPSILON")
			return true;
	}
	return false;

}

void parsing_table::getFollowOfRule(std::unordered_set <std::string> & followOfRule,const std::string rule ){
	std::vector<std::string> * ruleFollowVector = fafPtr->followSet[rule];
	for (auto value : (*ruleFollowVector)){
		followOfRule.insert(value);
	}
}

void parsing_table::createParsingTable(){
	std::string fileLocation = attributeGrammarFile.empty() ? "inputs/AttributeGrammar.txt" : attributeGrammarFile;

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
		getAlpha(alpha,line,lineIndex);
		hasEpsilon = checkFirstOfAlpha(alpha,firstOfAlpha,isTerminal);
		updateTable(rule,firstOfAlpha,line);
		if(hasEpsilon){
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
