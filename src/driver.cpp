#include "lexor.h"
#include "parser.h"
#include <filesystem>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>

int main(int argc, char *argv[])
{

	// Set logging to a file instead of the command line.
	//  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("./outputs/compiler.log", true);
	//  spdlog::set_default_logger(std::make_shared<spdlog::logger>("file_logger", file_sink));
	//  spdlog::set_level(spdlog::level::info);
	spdlog::set_level(spdlog::level::debug);
	spdlog::info("** ** **IN MAIN DRIVER.CPP** ** **");
	lexor *lex = new lexor();
	if (argc >= 2)
		lex->setInputFile(argv[1]);
	spdlog::warn("Current working directory check :{}", std::filesystem::current_path().string());
	std::vector<token *> vectorOfTokens;
	try
	{
		spdlog::warn("Entering the token loop from the driver.cpp file.");
		while (true)
		{
			try
			{
				token *currentToken = lex->getNextToken();
				if (currentToken->getTypeName().find("cmt") == std::string::npos && currentToken->getTypeName().find("comment") == std::string::npos)
					vectorOfTokens.emplace_back(currentToken);
			}
			catch (const std::invalid_argument &e)
			{
				spdlog::warn("Caught exception: {}", e.what());
				return 1;
			}
		}
	}
	catch (const EndOfFileException &e)
	{
		token *lastToken = new token("$", "$", -1, -1);
		vectorOfTokens.emplace_back(lastToken);
		spdlog::warn("Reached the end of the Script file with a total of {} Tokens found in the file.", vectorOfTokens.size());
	}
	catch (const std::exception &e)
	{
		std::cerr << "Caught a generic exception: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Deleted lex objext" << std::endl;
	delete lex;
	std::cout << "------------------------------------------------------------------------------------" << std::endl;
	spdlog::info("Entering Second phase, the parsing phase.");
	parser parser;
	if (argc >= 3)
		parser.faf.setInputFile(argv[2]);
	try
	{
		parser.faf.generateFirstSet();
	}
	catch (const std::invalid_argument &e)
	{
		spdlog::warn("Caught exception: {}", e.what());
		return 1;
	}
	parser.faf.h.writeToFirstSetFile(parser.faf);

	if (argc >= 4)
		parser.faf.setInputFile(argv[3]);
	try
	{
		parser.faf.generateFollowSet();
	}
	catch (const std::invalid_argument &e)
	{
		spdlog::warn("Caught exception: {}", e.what());
		return 1;
	}
	parser.faf.h.writeToFollowSetFile(parser.faf);
	std::cout << "------------------------------------------------------------------------------------" << std::endl;
	if (argc >= 5)
		parser.parsingTable.attributeGrammarFile = argv[4];
	else
		parser.parsingTable.attributeGrammarFile = "inputs/AttributeGrammar.txt";
	parser.parsingTable.buildTable();
	parser.parse(vectorOfTokens);
	std::cout << "Finished Parsing" << std::endl;
	parser.AST.printTree();
	std::cout << "------------------------------------------------------------------------------------" << std::endl;
	parser.AST.treeHead->accept(parser.ref_toTableCreatorVisitor);
	parser.AST.printSymbolTable(parser.AST.treeHead);
	SemanticCheckingVisitor *semanticChecker = new SemanticCheckingVisitor();
	semanticChecker->root = parser.AST.treeHead;
	parser.AST.treeHead->accept(*semanticChecker);
	std::cout << "Finished Building Symbol Table" << std::endl;
	return 0;
}
