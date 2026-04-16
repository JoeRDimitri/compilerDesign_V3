#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <unordered_set>
#include <stack>
#include <deque>
#include <typeinfo>
#include <spdlog/spdlog.h>
#include "parser.h"
#include "lexor.h"
#include "table_entry.h"
#include "first_and_follow.h"
#include "parsing_table.h"
#include "node.h"
#include "visitor.h"

class parser{

	class abstractSyntaxTree{
		public:
			class symbolTable;
			//Composite Element
			//For each node class, we implement an accept function 
			//The accept function will take a visitor as an argument, then call the accept functino for all of its children. Once all the children are visited by the visitor (through the accept function) then the visitor visits the parent. 

			startNode * treeHead;
			void printSymbolTable(node* head);
			void printSymbolTableRec(std::map<std::string,node::symbolTableEntry*> * link,int & count,std::ofstream & ao);
			void printTree();
			void traverseTree(node* head, std::string prefix, bool isLast, std::ofstream& ao);
	};

	class semanticActions{
		public:
			std::stack<node*>* semanticStackPtr = nullptr;
			abstractSyntaxTree* astPtr = nullptr;
			void makeLeaf(std::string nodeType,std::string lastTokenValue,node* newnode);
			void handleAction(std::string semanticAction,std::string lastTokenValue);
			void makeSubTree(std::string nodeType,std::string semanticMeaning, int amountOfPops,std::vector<std::string> v, node * newnode);
			void passAlong(std::string nodeType,std::string semanticMeaning,node * newnode);
			void passAlong(std::string nodeType,node * newnode);
			void adoptChildren(node* newparent,node* oldparent);
			void makeBinarySubTree(std::string nodeType,int i,node*newnode);
			void makeBinarySubTreeWithHead(std::string nodeType, int numOfPops,node* newnode);
	};

//Need to make all of these variables non-static and instantiate them in the constructor. 
	public:
		first_and_follow faf;
		parsing_table parsingTable;
		std::stack<std::string> parsingStack;
		semanticActions semanticHandler;
		std::stack<node*> semanticStack;
		abstractSyntaxTree AST;
		SymTabCreationVisitor *tableCreatorVisitor = new SymTabCreationVisitor();
		visitor & ref_toTableCreatorVisitor = * tableCreatorVisitor;
		bool searchFirst(std::string lookahead, std::string topOfTheStack);
		bool parse(const  std::vector<token*> &);
		void inverseRHSMultiplePush(tableEntry t,std::vector<std::string>& vec,const int & lineIndex);
		void skipError(token * & currentToken,std::vector<token*>::const_iterator& vectorIterator,std::ofstream &,const std::string & lexeme,const std::string & topOfTheStack,const int & line, const int & column);
		bool search(const std::string & lexeme, const std::string & topOfTheStack);

			parser(){
				parsingTable.fafPtr = &faf;
				semanticHandler.semanticStackPtr = &semanticStack;
				semanticHandler.astPtr = &AST;
				spdlog::info("Parser constructor called.");
			};
			~parser(){spdlog::info("Parser destructor called.");}

};

