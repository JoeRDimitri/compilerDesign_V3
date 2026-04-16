#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <spdlog/spdlog.h>

class first_and_follow; // forward declaration — full type needed only in handler.cpp

// Forward Declaration of the token class
class token;
class lexor;   // Add this forward declaration
class handler; // Forward declare handler so fileHandler/errorHandler can refer to it

std::ostream &operator<<(std::ostream &os, token &t);

class fileHandler
{
	handler *parent;

public:
	fileHandler(handler *parent_ptr = nullptr) : parent(parent_ptr),
												 inputFileStream() {};
	~fileHandler() {};

	friend class errorHandler;
	char move();
	void writeToken(token *t);
	bool checkEndOfStream();
	bool virgin = true;
	bool virginProtocol();
	void clear();
	void resetToBeginning();
	bool readLine(std::string &line);

	// A boolean to check if this is the first time reading the txt file.
	// inputFileStream to read the file.
	std::ifstream inputFileStream;
	// Function to establish connection to the file to read.
	bool connectFile(std::string);
	// Function to cut the connection to the file to read.
	bool disconnectFile();
	// function that checks for the end of the input stream
};

class errorHandler
{
	handler *parent;

public:
	friend class fileHandler;
	~errorHandler() {};
	errorHandler(handler *parent_ptr = nullptr) : parent(parent_ptr) {};
	void handleError(const std::string &errorType, const std::string &invalidType, const std::string &lexeme, const int &line, const int &column, token *t);

	void setParent(handler *p) { parent = p; }
};

class EndOfFileException : public std::exception
{
public:
	const char *what() const noexcept override
	{
		return "End of file reached";
	}
};

class handler
{
public:
	std::string tokenFileName;
	std::string errorFileName;
	fileHandler fileousHandler;
	errorHandler errorFileHandler;

	bool writeToFirstSetFile(first_and_follow &faf);
	bool writeToFollowSetFile(first_and_follow &faf);

	handler() : tokenFileName(""),
				errorFileName(""),
				fileousHandler(this),
				errorFileHandler(this) {};
	void setFileName(std::string s);
	std::vector<std::string> splitString(const std::string &str, char delimiter);
};