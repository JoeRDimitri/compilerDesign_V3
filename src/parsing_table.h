#pragma once
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include "table_entry.h"
#include "first_and_follow.h"


class parsing_table{


	 int static TABLEROWSIZE;
	 int static TABLECOLUMNSIZE;
	 int static UNIT;
	 static std::ifstream inputFileStream;



	//Function to establish connection to the file to read.
	static bool connectFile(std::string);
	//Function to cut the connection to the file to read.
	static bool disconnectFile();

	void setUniqueTerminalSymbols();
	int getTableRowSize();
	int getTableColumnSize();
	void setRowAndColumnSize();
	void setTheTableSize();
	void setTableRow();
	void setTableColumn();
	void createParsingTable();
	void getRule(std::string & rule, const std::string line, int & lineIndex);
	void getAlpha(std::string & alpha, const std::string line, int & lineIndex);
	bool checkFirstOfAlpha(const std::string alpha,std::unordered_set <std::string> & firstOfAlpha,bool & isTerminal);
	bool hasEpsilon(std::unordered_set <std::string> & firstOfAlpha, bool isTerminal);
	void getFollowOfRule(std::unordered_set <std::string> & followOfRule,const std::string rule);
	void updateTable(const std::string rule, std::unordered_set <std::string>  firstOfAlpha,std::string line);
	void fillTableWithErrors();

public:

	void buildTable();
	//Im going to use a 2d array and stor tableInfo structs.
	tableEntry ** table;
	//A set is used in order to have unique values of terminal symbols
    static std::unordered_set<std::string> uniqueTerminalSymbols;

	static std::unordered_map<std::string, int> tableRows;
	static std::unordered_map<std::string, int> tableColumns;  // Ensure values are ints, not references

	first_and_follow* fafPtr = nullptr;
	std::string attributeGrammarFile;
	parsing_table(){};
	~parsing_table(){
		for (int i = 0; i < TABLEROWSIZE; ++i) {
			delete[] table[i];  // Free each row
		}
		delete[] table;  // Free row pointers};
	};

	std::unordered_set <std::string> getUniqueTerminalSymbols(){return this -> uniqueTerminalSymbols;}
};
