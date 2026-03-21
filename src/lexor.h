#pragma once
#include <unordered_map>
#include <stdexcept>
#include <stack>
#include <vector>
#include <string>
#include "handler.h"


//forward declaration
//The error handler handles the errors

//A token is defined as a data structure that has a lexeme, a type, and a location (line + column)

class token{
private:

	//Type name is either a reserved word, an intnum, a floatnum, an id, inline comment or a block comment.
	std::string typeName;
	//Lexeme is the actual characters that will be defined
	std::string lexeme;
	//line is the line that the lexeme is found on in the file.
	int line;
	//Column is the column that that the lexeme is found on the line in the file.
	int column;
    void updateHandler();

public:
	//Constructor
	token(){};
	token(std::string type, std::string lexeme, int line, int column);
	//Destructor
	~token(){};

	//Getters
	std::string getTypeName(){return this->typeName;};
	std::string getLexeme(){return this->lexeme;};
	int getLine(){return this->line;};
	int getColumn(){return this->column;};

	//Setters
	void setTypeName(const std::string s){this->typeName = s;};
	void setLexeme(const std::string s){this->lexeme = s;};
	void setLine(const int i){this->line = i;};
	void setColumn(const int i){this->column = i;};


};


//Lexor class
class lexor {
private:
    handler h;
    errorHandler erroneousHandler;
    fileHandler fileousHandler;

    std::unordered_map<std::string,std::string> tokenMap;
    std::vector<char> intArray;
    std::vector<char> nonZeroArray;
    std::vector<char> letterArray;
    std::vector<char> alphanumArray;

    int line;
    int column;
    char currentCharacter;
    std::string currentLexeme;
    std::string possibleType;



	//Function to check if the lexeme is a reserved word by looking into tokenMap.
	//Returns true if reserved word is found and false if none are found.
	bool isReservedWord(std::string lexeme);
	//Function that moves the input stream forward if it reads white space. white space values are 9-10-11-12-13-32
	void getRidOfWhiteSpace();
	//checks if current character is whitespace
	bool isWhiteSpace();
	//Given a an array, this function will check that array to compare it to the currentCharacter
	bool isInArray(const std::vector<char>& arr);
	//check dictionnary
	bool isReservedWord();
	//Sets the possible type to determine what the type of the lexeme is.
	void setPossibleType();
	//function that updates the string and advance the pointer
	void addAndMove();
	//function that update the string that is provided. Overloaded function
	int addAndMove(std::string & tempLexeme);
	//Function to handle error
	token* errorProtocol(std::string type);
	//Function returns a valid token
	token* validToken(std::string type,int lineCounter = 0);
	//for the linecomment
	void getLine(std::string & tempLexeme);

	token* id();
	token* num(int decision);
	token* fraction();
	token* flt();
	token* res();
	token* cmt();
	token* invalidChar();


public:

	//Constructor
	lexor() : 
	h(),
	erroneousHandler(&h),
	fileousHandler(&h),
	tokenMap{
	    {"==","=="},{"+","+"},{"or","or"},{"(","("},{";",";"},{"int","int"},{"while","while"},
		{"local","local"},{"<>", "<>"},{"-","-"},{"and","and"},{")",")"},{",",","},
		{"float","float"},{"if","if"},{"constructor","constructor"},{"<","<"},{"*","*"},
		{"not","not"},{"{","{"},{".","."},{"void","void"},{"then","then"},
		{"attribute","attribute"},{">",">"},{"/","/"},{"}","}"},{":",":"},{"class","class"},
		{"else","else"},{"function","function"},{"<=","<="},{":=",":="},{"[","["},{"=>","=>"},
		{"self","self"},{"read","read"},{"public","public"},{">=",">="},{"]","]"},
		{"isa","isa"},{"write","write"},{"private","private"},{"implementation","implementation"},
		{"return","return"}
	},
	intArray{'0','1','2','3','4','5','6','7','8','9'},
	nonZeroArray{'1','2','3','4','5','6','7','8','9'},
	letterArray{'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','_'},
	alphanumArray{'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9','_'},
	line(0),
	column(0),
	currentCharacter(0),
	currentLexeme(""),
	possibleType("")
	{	spdlog::info("Lexor constructor called.");};
	//Destructor
	~lexor(){	spdlog::info("Lexor destructor called.");};
	//Overall flow control. Returns the next token in input file.
	token* getNextToken();
	//Function that runs the first time that we read from a text file. This should only be called a single time in the entire program.
	bool static virginProtocol(std::string filelocation);
	// Bypass stdin prompt: open the file directly and mark as ready.
	void setInputFile(const std::string& path);

};

