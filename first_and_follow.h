#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "handler.h"


class first_and_follow {
private:
	//unordered map for the first set
	//Key will be the name of the nonterminal symbol
	//The value will be a vector containing the actual set.
    handler h;
    errorHandler erroneousHandler;
    fileHandler fileousHandler;
	std::string currentSymbol;
	std::string currentWord;
	std::string line;
	int lineIndex;
	bool change;



	void resetFileInfo();
	//Function that moves the input stream forward if it reads white space. white space values are 9-10-11-12-13-32
	void getRidOfWhiteSpace(int & templineIndex);
	void getRidOfWhiteSpace();

	//Finds the first non terminal symbol which will the symbol we are generating a first set for
	std::vector<std::string> * findNT(int k);
	//Updates the currentCharacter
	bool checkForAssignment(int & tempLineIndex);
	//Checks if it is a terminal.
	bool checkT(int & tempLineIndex);
	bool checkT();

	bool checkNT(int & tempLineIndex);

	bool checkE();
	//Function that checks if the current symbol is in the vector that is provided as an argument
	void getSymbol(std::string t,int & tempLineIndex);
	void addAndMove(bool & change, std::vector<std::string>* v, std::string s);
	void compareSymbolVector(bool & change, std::vector<std::string>* v);
	void checkTheNextSymbol(bool& eps, std::vector<std::string>*v);
	void compareSymbolVectorRec(bool & eps, std::vector<std::string> *v);
	void getToAssignmentOp();
	void getFollowSetSymbol();
	void movePointer(int & templineIndex);
	std::vector<std::string> * findFollowSet(std::string);
	void compareAndAdd(std::vector<std::string>* v1, std::vector<std::string>* v2);
	bool hasEpsilon(std::vector<std::string> * v);
public:
	static bool inVector(std::vector<std::string> * v, const std::string s);
	std::unordered_map<std::string, std::vector<std::string>*> firstSet;
	std::unordered_map<std::string, std::vector<std::string>*> followSet;

	bool writeToFirstSetFile();
	bool writeToFollowSetFile();
	void generateFirstSet();
	void generateFollowSet();

	first_and_follow():
	h(),
	erroneousHandler(&h),
	fileousHandler(&h),
	lineIndex(0),
	change(true),
	line(""),
	currentSymbol(""),
	currentWord("")
	{spdlog::info("In the first_and_follow() Construction");};

	~first_and_follow(){
		spdlog::info("In the first_and_follow() Destruction");
		for (auto& pair : firstSet) {
		        delete pair.second;  // Deallocate the vector pointer
		    }
		for (auto& pair : followSet) {
		        delete pair.second;  // Deallocate the vector pointer
		    }
	}
};
