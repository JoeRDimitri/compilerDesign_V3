#include "first_and_follow.h"

void first_and_follow::setInputFile(const std::string& path) {
	fileousHandler.connectFile(path);
	fileousHandler.virgin = false;
}
void first_and_follow::resetFileInfo(){
	line ="";
	currentSymbol ="";
	lineIndex = 0;

}
void first_and_follow::getRidOfWhiteSpace(int&tempLineIndex){
	//Function that moves the input stream forward if it reads white space. white space values are 9-10-11-12-13-32
	//Will enter loop if the current character is a whitespace
	while(tempLineIndex<line.size()&&(line.at(tempLineIndex) == 9 || line.at(tempLineIndex) == 11|| line.at(tempLineIndex) == 32)){
		//Move the Input File pointer to the next character
		tempLineIndex++;
	}
}
void first_and_follow::generateFirstSet(){
	spdlog::info("Generating First Set from the properly modified grammar.");
	//Check if we've run this file before. IF we have not then we begin the virgin protocol
	//Virgin protocol is used to get the establish file connection.
	//If the virginf protocol fails then we've already established a connection to the file.
	if(fileousHandler.virgin == true){
			spdlog::info("Within the First Set Generation Step for the first time.");
		//Virgin set to false so that we never enter this code again.
		fileousHandler.virgin = false;
		//If the virgin protocol fails, then we were not able to establish a connection to the file. The program should close
		if(!fileousHandler.virginProtocol())throw std::invalid_argument("Invalid File location");
		else{
		}
	}
	//I need a while loop here to control the flow of change and see if we need to iterate over the file yet again.
	// If change = false then we do not iterate over the file again because there have been no changes.
	//otherwise we iterate again.
	spdlog::info("Going into the main loop of the FirstSet Generation. ");
	change = true;
	while (change){
		change = false;
		resetFileInfo();
		// Reset the file stream to read again
		fileousHandler.resetToBeginning();
		//we should get line until the end of the file.
		while (fileousHandler.readLine(line)){
			lineIndex = 0;
			currentSymbol = "";
			currentWord = "";			
			getRidOfWhiteSpace(lineIndex);
			if(lineIndex == line.size())
				continue;
			//We get the first set and store it in a temp variable.
			std::vector<std::string> * firstSetPtr = findNT(0);
			//We have the NT. Now we move passed the assignment operator.
			if(checkForAssignment(lineIndex)){
				//If we are here it is becaue we've passed the assignment operator.
				//We've gotten rid of the white space
				//We need a loop here for all of the symbols in the line: essentially while the line index != to length of the line
				currentSymbol = "";
				//We check if the first symbol after the assignment symbol is a terminal.
				if(checkT(lineIndex)){
					//A terminal symbol is found.
					//We simply add the terminal symbol to the first set if its not already there.
					//check if the terminal symbol exists in vector
					getSymbol("t",lineIndex);
					//If the terminal symbol is not in the first set.
					if(!inVector(firstSetPtr,currentSymbol)){
						//We add it and move onto the next symbol
						addAndMove( change, firstSetPtr, currentSymbol);
						currentSymbol ="";
						continue;
					}
					//if the terminal symbol is in the first set vector, then we need to reset the currentSymbol and move onto the next symbol.
					else {
						currentSymbol = "";
						continue;
					}
				}
				else if(checkNT(lineIndex)){
					//If we are here it is becaue we've passed the assignment operator.
					//We've gotten rid of the white space
					//We check if the first symbol after the assignment symbol is a non terminal, then we get symbol.
					getSymbol("nt",lineIndex);
					compareSymbolVector(change,firstSetPtr);
				}
				else if(checkE()){
					//If we are here it is becaue we've passed the assignment operator.
					//We've gotten rid of the white space
					//We check if the first symbol after the assignment symbol is a an epsilon symbol
					getSymbol("e",lineIndex);
					if(!inVector(firstSetPtr,currentSymbol)){
						//We add it and move onto the next symbol
						addAndMove( change, firstSetPtr, "EPSILON");
						currentSymbol ="";
						continue;
					}
					//if the terminal symbol is in the first set vector, then we need to reset the currentSymbol and move onto the next symbol.
					else {
						currentSymbol = "";
						continue;
					}
				}
				else{
					spdlog::error("Error in the grammar, neither NT or T found at: {}",currentWord);
					spdlog::error("line is: {}",line);
					spdlog::error("lineIndex is {}",lineIndex);
				}
			}
			else{
				//If we are here then we did not reach the assignment operator after identfying the noterminal symbol there for there is an error in the grammar
				spdlog::error("There is an error here in the grammar. Should have reach an assignment operator for the NT symbol: {}.",currentWord);
				currentWord ="";
				continue;
			}
		}
	}
	fileousHandler.disconnectFile();
	fileousHandler.virgin = true;
}
void first_and_follow:: compareSymbolVector(bool & change,std::vector<std::string>* v){
	//Check if this vector exists.
	bool eps;
	if(firstSet.count(currentSymbol)==0){
		change = true;
		return;
	}
	else{
		//We need to get the vector of the current symbol.
		std::vector<std::string> * firstSetPtrOfSymbol = firstSet.at(currentSymbol);
		//For each element in the firstSetPtrOfSymbol vector, we need check if its in our v ptr vector.
		for(std::string value : (*firstSetPtrOfSymbol)){
			if(inVector(v,value) && value != "EPSILON"){
				continue;
			}
			else if (!inVector(v,value) && value != "EPSILON"){
				addAndMove(change,v,value);
				continue;
			}
			if (value == "EPSILON"){
				eps = true;
			}
		}
		while(eps){
			if(lineIndex == line.size()){
				eps = false;
				if(!inVector(v,"EPSILON")){
					addAndMove(change,v,"EPSILON");
					change = true;
				}
			}
			else{
				checkTheNextSymbol(eps,v);
			}

		}
		if(eps &&lineIndex == line.size()){
			if(!inVector(v,"EPSILON")){
				change = true;
				addAndMove(change,v,"EPSILON");
			}
			return;
		}

	}
}
void first_and_follow::checkTheNextSymbol(bool & eps, std::vector<std::string>*v){
	getRidOfWhiteSpace(lineIndex);
	currentSymbol = "";
	if(checkT(lineIndex)){
		eps = false;
		//A terminal symbol is found.
		//We simply add the terminal symbol to the first set if its not already there.
		//check if the terminal symbol exists in vector
		getSymbol("t",lineIndex);
		//If the terminal symbol is not in the first set.
		if(!inVector(v,currentSymbol)){
			//We add it and move onto the next symbol
			addAndMove( change, v, currentSymbol);
			currentSymbol ="";
			return;
		}
		//if the terminal symbol is in the first set vector, then we need to reset the currentSymbol and move onto the next symbol.
		else {
			currentSymbol = "";
		}
	}
	else if(checkNT(lineIndex)){
		//If we are here it is becaue we've passed the assignment operator.
		//We've gotten rid of the white space
		//We check if the first symbol after the assignment symbol is a non terminal, then we get symbol.
		getSymbol("nt",lineIndex);
		compareSymbolVectorRec(eps,v);
	}
}
void first_and_follow:: compareSymbolVectorRec(bool & eps,std::vector<std::string>* v){
	//Check if this vector exists.
	if(firstSet.count(currentSymbol)==0){
		change = true;
		eps = false;
		return;
	}
	else{
		//We need to get the vector of the current symbol.
		std::vector<std::string> * firstSetPtrOfSymbol = firstSet.at(currentSymbol);
		//For each element in the firstSetPtrOfSymbol vector, we need check if its in our v ptr vector.
		for(std::string value : (*firstSetPtrOfSymbol)){
			if(inVector(v,value) && value != "EPSILON"){
				continue;
			}
			else if (!inVector(v,value) && value != "EPSILON"){
				addAndMove(change,v,value);
				continue;
			}
			if (value == "EPSILON"){
				eps = true;
			}
		}
	}
}
void first_and_follow:: addAndMove(bool& change, std::vector<std::string> * v, std::string s){
	v->emplace_back(s);
	change = true;
//	std::cout<<"Adding: "<<s<<" to the first Set of : "<<currentWord<<std::endl;
}
bool first_and_follow::checkE(){
	if(line.at(lineIndex)=='E'){
		return true;
	}
	return false;
}
void first_and_follow::getSymbol(std::string s,int & tempLineIndex){
	if(s == "t"){while(line.at(tempLineIndex)!= '\''){
		currentSymbol += line.at(tempLineIndex);
		tempLineIndex++;}
	}
	else if(s=="nt"){while(line.at(tempLineIndex)!= '>'){
		currentSymbol += line.at(tempLineIndex);
		tempLineIndex++;}
	}
	else if(s == "e"){while(line.at(tempLineIndex)!=' '){
		currentSymbol += line.at(tempLineIndex);
		tempLineIndex++;}
	}
	tempLineIndex++;
	getRidOfWhiteSpace(tempLineIndex);
}
bool first_and_follow::inVector(std::vector<std::string> * firstSet,std::string s ){
//	std::cout<<(*firstSet)<<std::end
	for (std::string value : (*firstSet)) {
		if(value == s)
			return true;
	}
	return false;
}
bool first_and_follow::checkT(int & tempLinIndex){
	if(line.at(tempLinIndex)=='\''){
		tempLinIndex++;
		return true;
	}
	return false;
}
bool first_and_follow::checkNT(int & tempLinIndex){
	if(line.at(tempLinIndex)=='<'){
		tempLinIndex ++;
		return true;
	}
	return false;
}
std::vector<std::string> * first_and_follow::findNT(int k){
	while(line.at(lineIndex) != '<'){
		lineIndex++;
	}
	lineIndex++;
	//We build the word
	while(line.at(lineIndex) != '>'){
		currentWord += line.at(lineIndex);
		lineIndex++;
	}
	lineIndex++;
	getRidOfWhiteSpace(lineIndex);
	//need to check if this nonterminal already exists.
	if(k == 0){
	    if (firstSet.count(currentWord) == 0) {
	        firstSet[currentWord] = new std::vector<std::string>();  // Allocate memory
	    }
	    return firstSet.at(currentWord);  // Dereference the pointer
	}
	else if(k == 1){
	    if (followSet.count(currentWord) == 0) {
	    	followSet[currentWord] = new std::vector<std::string>();  // Allocate memory
	    }
	    return followSet.at(currentWord);  // Dereference the pointer
	}
    // invalid k value, nothing to return
    return nullptr;
}
bool first_and_follow::checkForAssignment(int & tempIndex){
    if(line.at(tempIndex)==':'){
        tempIndex++;
        if(line.at(tempIndex)==':'){
            tempIndex++;
            if(line.at(tempIndex)=='='){
                lineIndex = tempIndex;
                lineIndex++;
                getRidOfWhiteSpace(lineIndex);
                return true;
            }
        }
    }
    return false;
}
void first_and_follow::generateFollowSet(){
	spdlog::info("Generating Follow Set from the properly modified grammar.");
	//Check if we've run this file before. IF we have not then we begin the virgin protocol
	//Virgin protocol is used to get the establish file connection.
	//If the virginf protocol fails then we've already established a connection to the file.
	if(fileousHandler.virgin == true){
		spdlog::info("Within the First Set Generation Step for the first time.");
		//Virgin set to false so that we never enter this code again.
		fileousHandler.virgin = false;
		//If the virgin protocol fails, then we were not able to establish a connection to the file. The program should close
		if(!fileousHandler.virginProtocol())throw std::invalid_argument("Invalid File location");
		else{
			//We are good to begin the process.
		}
	}
	//I need a while loop here to control the flow of change and see if we need to iterate over the file yet again.
	// If change = false then we do not iterate over the file again because there have been no changes.
	//otherwise we iterate again.

	//Just add  '$' to the start follow start
	//First you have to create the vector list
	std::vector<std::string> *startFollowSet = new std::vector<std::string>;
	startFollowSet->emplace_back("$");
	followSet["START"] = startFollowSet;
	spdlog::info("Going into the main loop of the FollowSet Generation. ");
	change = true;
	while (change){
		change = false;
		line ="";
		currentSymbol ="";
		lineIndex = 0;
	   // Reset the file stream to read again
		fileousHandler.resetToBeginning();
		//we should get line until the end of the file.
		while (fileousHandler.readLine(line)){
//			std::cout << line << std::endl;
			lineIndex = 0;
			currentSymbol = "";
			currentWord = "";
			getRidOfWhiteSpace(lineIndex);
			if(lineIndex == line.size())
				continue;
			//Build the first nt symbol because we might need its follow set.
			std::string nt;
			if(line.at(lineIndex =='<')){
				lineIndex++;
				while(line.at(lineIndex)!='>'){
					nt += line.at(lineIndex);
					lineIndex++;
				}
			}
			lineIndex++;
			getRidOfWhiteSpace(lineIndex);
			checkForAssignment(lineIndex);
			//In the firstset generation we were looking at the first nt symbol
			//In the followset generation we need to look at each symbol in the line.
			//sO we have a while loop that terminates when we reach teh end of the line.
			while(lineIndex != line.size()){
				getRidOfWhiteSpace(lineIndex);
				//We need to getFollowSetSymbol.
				getFollowSetSymbol();
				if(lineIndex == line.size())continue;
				currentWord ="";
				currentSymbol ="";
				//Now currentWord == the symbol that we need to analyze
				std::vector<std::string> * followSetVectorOfWord = findNT(1);
				int tempLineIndex = lineIndex;
				//Now we get the symbol after it.
				//We need to  move the pointer to the location of the next possible
				movePointer(tempLineIndex);
				getRidOfWhiteSpace(tempLineIndex);
				if(lineIndex == line.size()){
					//WE have reached the end of the string.
					// If this is the case then the follow set of of the original nt symbol is part followset of the currentWord
					std::vector<std::string> * followSetOfOriginalNT = findFollowSet(nt);
					compareAndAdd(followSetVectorOfWord,followSetOfOriginalNT);
					currentSymbol ="";
					break;
				}
				else if(checkT(tempLineIndex)){
					//A terminal symbol is found.
					//We simply add the terminal symbol to the first set if its not already there.
					//check if the terminal symbol exists in vector
					getSymbol("t",tempLineIndex);
					//If the terminal symbol is not in the first set.
					if(!inVector(followSetVectorOfWord,currentSymbol)){
						//We add it and move onto the next symbol
						addAndMove( change, followSetVectorOfWord, currentSymbol);
						currentSymbol ="";
						continue;
					}
					//if the terminal symbol is in the first set vector, then we need to reset the currentSymbol and move onto the next symbol.
					else {
						currentSymbol = "";
						continue;
					}
				}
				else if(checkNT(tempLineIndex)){
					//If we are here it is becaue we've passed the assignment operator.
					//We've gotten rid of the white space
					//We check if the first symbol after the assignment symbol is a non terminal, then we get symbol.
					getSymbol("nt",tempLineIndex);
					std::vector<std::string>*firstSetOfSymbol = firstSet[currentSymbol];
					compareAndAdd(followSetVectorOfWord,firstSetOfSymbol);
					if(hasEpsilon(firstSetOfSymbol)){
						while(tempLineIndex<=line.size()){
						getRidOfWhiteSpace(tempLineIndex);
							if(tempLineIndex==line.size()){
								std::vector<std::string> * followSetOfOriginalNT = findFollowSet(nt);
								compareAndAdd(followSetVectorOfWord,followSetOfOriginalNT);
								currentSymbol ="";
								break;
							}
							else if(checkT(tempLineIndex)){
								//A terminal symbol is found.
								//We simply add the terminal symbol to the first set if its not already there.
								//check if the terminal symbol exists in vector
								currentSymbol ="";
								getSymbol("t",tempLineIndex);
								//If the terminal symbol is not in the first set.
								if(!inVector(followSetVectorOfWord,currentSymbol)){
									//We add it and move onto the next symbol
									addAndMove( change, followSetVectorOfWord, currentSymbol);
									currentSymbol ="";
									break;
								}
								//if the terminal symbol is in the first set vector, then we need to reset the currentSymbol and move onto the next symbol.
								else {
									currentSymbol = "";
									break;
								}
							}
							else if(checkNT(tempLineIndex)){
								currentSymbol ="";
								getSymbol("nt",tempLineIndex);
								std::vector<std::string>*firstSetOfSymbol = firstSet[currentSymbol];
								compareAndAdd(followSetVectorOfWord,firstSetOfSymbol);
								if(hasEpsilon(firstSetOfSymbol))continue;
								else break;
							}
						}
						continue;
					}
					else continue;
				}
			}
		}
	}
	fileousHandler.disconnectFile();
}
bool first_and_follow::hasEpsilon(std::vector <std::string> * b){
	for(std::string value : (*b) ){
		if(value == "EPSILON")
			return true;
	}
	return false;
}
void first_and_follow::getToAssignmentOp(){
	while(lineIndex!= line.size() && line.at(lineIndex) != ':')
		lineIndex++;
}
void first_and_follow::getFollowSetSymbol(){
	while(lineIndex!= line.size() && line.at(lineIndex)!='<'){
		lineIndex ++;
	}
	if(lineIndex +1 < line.size()){
		if(line.at(lineIndex+1) == '>'||line.at(lineIndex+1) == '=' || line.at(lineIndex+1) == '\''){
			lineIndex++;
			while(lineIndex!= line.size() && line.at(lineIndex)!='<'){
				lineIndex ++;
			}
		}
	}
}
void first_and_follow::movePointer(int & templineIndex){
	while(templineIndex!=line.size()&&line.at(templineIndex)!='<' &&line.at(templineIndex)!='\''&&line.at(templineIndex)!='e' ){
		templineIndex++;
	}
}
std::vector<std::string> * first_and_follow:: findFollowSet(std::string s){
	if(followSet.count(s)==0){
		followSet[s] = new std::vector<std::string>();  // Allocate memory
    	change = true;
	}
	return followSet[s];
}
void first_and_follow:: compareAndAdd(std::vector<std::string> * followSetOfWord, std::vector<std::string> * followSetOfOriginalNT){
	if(followSetOfOriginalNT->size()==0){
		change = true;
		return;
	}
	for(const std::string& value : *followSetOfOriginalNT){
		if(value == "EPSILON") continue;
		if(!inVector(followSetOfWord, value)){
			change = true;
			followSetOfWord->emplace_back(value);
		}
	}
}