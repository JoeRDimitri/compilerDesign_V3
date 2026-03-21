#include "lexor.h"


void lexor::getRidOfWhiteSpace(){
	//Function that moves the input stream forward if it reads white space. white space values are 9-10-11-12-13-32
	//Will enter loop if the current character is a whitespace
	while((currentCharacter >= 9 && currentCharacter <=13) || currentCharacter == 32)
	{
		//Increase the column where the next character will be.
		column++;
		//If we go to the next line then we increase the line variable and set the column to 0
		if (currentCharacter == 10)
		{
			line ++;
			column = 0;
		}
		//Move the Input File pointer to the next character
		currentCharacter = fileousHandler.move();
	}
}

bool lexor::isWhiteSpace(){
	if((currentCharacter >= 9 && currentCharacter <=13)||currentCharacter==32)
		return true;
	else return false;
}

bool lexor::isInArray(const std::vector<char>& arr){
	for (char ch : arr){
		if(currentCharacter == ch) return true;
	}
	return false;

}

bool lexor::isReservedWord(std::string s){
	if(tokenMap.count(s)>0) return true;
	else return false;
}

bool lexor::isReservedWord(){
	std::string charAsString {currentCharacter};
	if(tokenMap.count(charAsString)>0||currentCharacter == '=') return true;
	else return false;
}

void lexor::setPossibleType(){
	//Calling std::string constructor overloaded to take a character as a parameter, will convert this character to a string
	std::string charAsString {currentCharacter};
	if(isInArray(letterArray))possibleType = "id";
	else if(isInArray(intArray))possibleType = "num";
	else if(tokenMap.count(charAsString)>0||currentCharacter == '=') {possibleType ="res";}
	else if(currentCharacter == '/')possibleType="cmt";
	else {possibleType="inv";}
}

void lexor::addAndMove(){
	currentLexeme+=currentCharacter;
	currentCharacter = fileousHandler.move();
	column++;
}

int lexor::addAndMove(std::string & templexeme){
	int tempLineCounter = 0;
	if(currentCharacter=='\r'){templexeme += "\\n";column = 0;}
	else if(currentCharacter == '\n'){tempLineCounter++;}
	else templexeme+=currentCharacter;
	currentCharacter = fileousHandler.move();
	column++;
	return tempLineCounter;
}

token* lexor::errorProtocol(std::string type){

	while(!isWhiteSpace()&&!isReservedWord()&&currentCharacter != '=')addAndMove();
	std::string tempLexeme = currentLexeme;
	currentLexeme = "";

	if(type.compare("id")==0){
		token * t = new token("invalidid",tempLexeme,line,column);
		erroneousHandler.handleError("Lexical error", "Invalid Identifier",tempLexeme, line, column, t);
		return t;
	}
	else if(type.compare("intnum")==0){
		if(currentCharacter == '.'){
			addAndMove(tempLexeme);
			while(!isWhiteSpace()&&!isReservedWord()&&currentCharacter != '=')addAndMove(tempLexeme);
		}
		token * t = new token("invalidnum",tempLexeme,line,column);
		erroneousHandler.handleError("Lexical error", "Invalid Number",tempLexeme, line, column,t);
		return t;
	}
	else if(type.compare("frac")==0){
		token * t = new token("invalidnum",tempLexeme,line,column);
		erroneousHandler.handleError("Lexical error", "Invalid Number",tempLexeme, line, column,t);
		return t;
	}
	else if(type.compare("invChar")==0){
		token * t = new token("invalidchar",tempLexeme,line,column);
		erroneousHandler.handleError("Lexical error", "Invalid Character",tempLexeme, line, column,t);
		return t;
	}
    // fallback to avoid warning: should never happen
    return nullptr;
}

// restored function signature for validToken
token* lexor::validToken(std::string type, int lineCounter){
    std::string tempLexeme = currentLexeme;
    currentLexeme = "";

	if(type.compare("id")==0){
		if(isReservedWord(tempLexeme)) {
			token* t = new token(tokenMap.at(tempLexeme),tempLexeme,line,column);
			fileousHandler.writeToken(t);
			return t;
		}
		else{
			token* t = new token("id",tempLexeme,line,column);
			fileousHandler.writeToken(t);
			return t;
		}
	}
	else if (type.compare("intnum")==0){
		token* t = new token("intnum",tempLexeme,line,column);
		fileousHandler.writeToken(t);
		return t;
	}
	else if (type.compare("frac")==0){
		token* t = new token("floatnum",tempLexeme,line,column);
		fileousHandler.writeToken(t);
		return t;
	}
	else if(type.compare("res")==0){
		std::string value = tokenMap.at(tempLexeme);
		token* t =  new token(value,tempLexeme,line,column);
		fileousHandler.writeToken(t);
		return t;
	}
	else if(type.compare("linecomment")==0){
		getLine(tempLexeme);
		token* t = new token("linecomment",tempLexeme,line, column);
		fileousHandler.writeToken(t);
		column = 0;
		return t;
	}

	else if(type.compare("blockcomment")==0){
		token* t = new token("blockcmt",tempLexeme,line,column);
		fileousHandler.writeToken(t);
		line+=lineCounter;
		return t;
	}
    // should never reach here
    return nullptr;
}

void lexor::getLine(std::string &tempLexeme){
    while(currentCharacter != '\r' && currentCharacter!='\n' && !fileousHandler.checkEndOfStream()){
        addAndMove(tempLexeme);
    }
}

token* lexor::id(){

	//lexeme is still empty at this point
	//Invalid Id case
	if(currentCharacter == '_'){return errorProtocol("id");}

	//We add our first character
	addAndMove();

	//While the current character is an alpha numeric value keep adding this to the current lexeme.
	while(isInArray(alphanumArray)){addAndMove();}

	/*There are numerous resons why we might be here.
	 * 1. We might've reached a whitespace. In that case we can simply return from this function since we have found a proper lexeme
	 * 2. We might've reached a special character, in that case we can simply return from thins functino since we have found a proper lexeme
	 * 3. else*/

	if(fileousHandler.checkEndOfStream()||isWhiteSpace()||isReservedWord()){}
	else{return errorProtocol("id");}
	return validToken("id");
}

token* lexor::num(int decision){

	//lexeme is still empty at this point
	//Check if it is 0
	if(currentCharacter == '0'){
		//We add our first character
		addAndMove();

		/*We need to make sure that the next character is either:
		 *1. A white space. in which case we are done successfully
		 *2. a '.' in which case we go to the float function.
		 *3. anything else is an invalid num
		 */
		if(decision == 0){
			if(currentCharacter =='.')return fraction();
			if(fileousHandler.checkEndOfStream()||isWhiteSpace()||isReservedWord()){return validToken("intnum");}
			else{return errorProtocol("intnum");}
		}
		else if(decision == 1){
			if(fileousHandler.checkEndOfStream()||isWhiteSpace()||isReservedWord()){return validToken("frac");}
			else{return errorProtocol("frac");}
		}


	}
	//Check if its nonzero
	else if(isInArray(nonZeroArray)){
		addAndMove();

		while(isInArray(intArray)){addAndMove();}

		/*
		 * At this point there are cases we need to consider
		 * 1. White space
		 * 2. anything else is an error
		 */
		if(decision == 0){
			if(currentCharacter =='.')return fraction();
			if(fileousHandler.checkEndOfStream()||isWhiteSpace()||isReservedWord()){return validToken("intnum");}
			else{return errorProtocol("intnum");}
		}
		else if(decision == 1){
			if(fileousHandler.checkEndOfStream()||isWhiteSpace()||isReservedWord()){return validToken("frac");}
			else{return errorProtocol("frac");}
		}

	}

	std::cout<<"CODE SHOULD NOT REACH HERE :: TOKEN* LEXOR::NUM()\n returning an empty token.";
	return new token();

}

token* lexor::fraction(){
	//Double check that the currentCharacter == '.'
	if(currentCharacter == '.'){addAndMove();}
	else{
		std::cout<<"INCORRECT CHARACTER, EXPECTED '.' in FRACTION() \n RETURN EMPTY TOKEN";
		return new token();
	}

	//Continuing after confirming'.'
	if(isInArray(intArray)){
		//a digit was confirmed so we move on
		//We keep adding digits
		//When we've reached the end of the lexeme we need to check the final digit.
		while(isInArray(intArray)){addAndMove();}
		//Need to verify the last character of the lexeme is not a 0
		if(currentCharacter == 'e') return flt();
		else if(currentLexeme == "0.0"){return validToken("frac");}
		else if(currentLexeme.back() == '0' && currentLexeme.size()==3 ){return validToken("frac");}
		else if(currentLexeme.back() == '0'){return errorProtocol("frac");}

		//If we reach a white space or a reserve word we've reached the end of the lexeme
		else if(isWhiteSpace()||isReservedWord()||fileousHandler.checkEndOfStream()){return validToken("frac");}
		else{return errorProtocol("frac");}
	}
	//Anything that is not a number is an error.
	return errorProtocol("frac");
}

token* lexor::flt(){
	//Double check currentCharacter == 'e'
	if(currentCharacter == 'e'){addAndMove();}
	else{
		std::cout<<"INCORRECT CHARACTER, EXPECTED 'e' in FLT()\n RETURN EMPTY TOKEN";
		return new token();
	}
	//Continuing after confirming'e'
	/*
	 *There are a few cases that we need to consider
	 *1. we have a 0 after the e
	 *2. we have a nonzero number after the e
	 *3. we have a +/- after e
	 */
	//If we habe a digit just use num () to verify
	if(isInArray(intArray)){
		return num(1);
	}

	//If we have a + or a - we add it to the lexeme and then use num() to verify
	else if(currentCharacter == '+' || currentCharacter == '-'){
		addAndMove();
		if(isInArray(intArray)){
			return num(1);
		}
	}

	//If we have anything else we have an error.
	return errorProtocol("frac");
}

token* lexor::res(){
	std::string charAsString1 {currentCharacter};
	//confirming the decision
	if(tokenMap.count(charAsString1)>0||currentCharacter == '='){addAndMove();}
	else{
		std::cout<<"INCORRECT CHARACTER, EXPECTED a map item in RES()\n RETURN EMPTY TOKEN";
		return new token();
	}
	//Check if it is possibly a comment
	if(charAsString1.compare("/")==0){
		//Could be a comment but need to confirm
		std::string charAsString2 = charAsString1 + currentCharacter;
		if(charAsString2.compare("//")==0||charAsString2.compare("/*")==0){
			addAndMove();
			return cmt();
		}
	}
	//Continue natural flow
	std::string charAsString2 = charAsString1 + currentCharacter;
	//
	if(tokenMap.count(charAsString2)>0){
		addAndMove();
	}

	else{}

	return validToken("res");
}

token* lexor::cmt(){
	if(currentLexeme.compare("//")==0){
		return validToken("linecomment");
	}
	else if(currentLexeme.compare("/*")==0){
		int lineCounter = 0;
		std::stack<int> commentStackCounter;
		commentStackCounter.push(0);

		while(true){
			if(commentStackCounter.empty()){
				return validToken("blockcomment",lineCounter);
			}

			if(fileousHandler.checkEndOfStream())
				return errorProtocol("eof");

			while(currentCharacter != '*'&&currentCharacter != '/'&&!fileousHandler.checkEndOfStream()){
				lineCounter += addAndMove(currentLexeme);
			}
			if(fileousHandler.checkEndOfStream())
				continue;
			else if(currentCharacter == '*'){
				addAndMove(currentLexeme);
				if(currentCharacter == '/'){
					addAndMove(currentLexeme);
					commentStackCounter.pop();
					continue;
				}
				continue;
			}
			else if(currentCharacter == '/'){
				addAndMove(currentLexeme);
				if(currentCharacter == '*'){
					addAndMove(currentLexeme);
					commentStackCounter.push(0);
					continue;
					}
			}

			else{
				addAndMove(currentLexeme);
				continue;
			}
		}
	}

	else{
		std::cout<<"ERROR THE CODE SHOULD NOT REACH HERE, END OF CMT() FUNCTION"<<std::endl;
		return nullptr; // defensive return
	}
}

token* lexor::invalidChar(){
	return errorProtocol("invChar");
}

token* lexor:: getNextToken(){
	//Check if we've run this file before. IF we have not then we begin the virgin protocol
	//Virgin protocol is used to get the establish file connection.
	//If the virginf protocol fails then we've already established a connection to the file.
	if(fileousHandler.virgin == true){
			spdlog::info("Within the getNextToken function for the first time.");

		//Virgin set to false so that we never enter this code again.
		fileousHandler.virgin = false;

		//If the virgin protocol fails, then we were not able to establish a connection to the file. The program should close
		if(!fileousHandler.virginProtocol())throw std::invalid_argument("Invalid File location");
		else{
			//We are good to begin the process.
			//Get rid of white spaces
			//set line and column to 0
			line = 1;
			column = 0;
			currentCharacter = fileousHandler.move();
			currentLexeme = "";
		}
	}
	//Get rid of all white space.
	getRidOfWhiteSpace();

	if(fileousHandler.checkEndOfStream()){
		spdlog::warn("Reached end of Stream.");        // warn level
		throw EndOfFileException();
	}

	//We've set the possible type for the first character of our brand new lexeme.
	setPossibleType();

	//Check what the currentCharacter and go into the corresponding function
	if(possibleType == "id")return id();
	else if(possibleType == "num")return num(0);
	else if(possibleType == "res")return res();
	else if(possibleType =="cmt")return cmt();
	else if(possibleType =="inv")return invalidChar();    // if we get here something went wrong
    return nullptr;
}

token::token(std::string type, std::string lexeme, int line, int column){
	this->typeName = type;
	this->lexeme = lexeme;
	this->line = line;
	this->column = column;
}
