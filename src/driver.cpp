#include "lexor.h"
#include "parser.h"
#include <filesystem>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>

void initialize_argument_files(int argc, char *argv[], lexor *main_lex, parser *parser)
{
	if (argc >= 2)
	{
		main_lex->setInputFile(argv[1]);
		spdlog::info("Virgin Protocol set to FALSE because arguments specified in the executable command.");
	}
	if (argc >= 3)
		parser->set_firstSet_inputFile(argv[2]);
	if (argc >= 4)
		parser->set_followSet_inputFile(argv[2]);
};

int main(int argc, char *argv[])
{

	// Set logging to a file instead of the command line.
	//  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("./outputs/compiler.log", true);
	//  spdlog::set_default_logger(std::make_shared<spdlog::logger>("file_logger", file_sink));
	//  spdlog::set_level(spdlog::level::info);
	spdlog::set_level(spdlog::level::debug);
	spdlog::info("Initialized logging");

	lexor lex;
	parser parser;

	initialize_argument_files(argc, argv, &lex, &parser);

	// spdlog::warn("Current working directory check :{}", std::filesystem::current_path().string());
	std::vector<std::unique_ptr<token>> vectorOfTokens;
	try
	{
		spdlog::debug("Entering the token loop from the driver.cpp file.");
		while (true)
		{
			try
			{
				std::unique_ptr<token> currentToken = lex.getNextToken();
				// We specifically place the tokens that are not comments in the vector
				// Comment tokens including line comments and block comments, the pointer gets destroyed here
				// Whereas the pointer for the other tokens live on in the vector
				if (currentToken->getTypeName().find("cmt") == std::string::npos && currentToken->getTypeName().find("comment") == std::string::npos)
					vectorOfTokens.emplace_back(std::move(currentToken));
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
		vectorOfTokens.emplace_back(std::make_unique<token>("$", "$", -1, -1));
		spdlog::debug("Reached the end of the Script file with a total of {} Tokens found in the file.", vectorOfTokens.size());
	}
	catch (const std::exception &e)
	{
		std::cerr << "Caught a generic exception: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "------------------------------------------------------------------------------------" << std::endl;
	spdlog::info("Entering Second phase, the parsing phase.");
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

	parser.build_parsing_table();

	parser.parse(vectorOfTokens);

	std::cout << "Finished Parsing" << std::endl;

	parser.AST.printTree();
	std::cout << "------------------------------------------------------------------------------------" << std::endl;

	parser.AST.treeHead->accept(parser.ref_toTableCreatorVisitor);
	parser.AST.printSymbolTable(parser.AST.treeHead);
	SemanticCheckingVisitor *semanticChecker = new SemanticCheckingVisitor();
	semanticChecker->root = parser.AST.treeHead;
	parser.AST.treeHead->accept(*semanticChecker);
	if (!semanticChecker->canGenerateMachineCode)
	{
		spdlog::error("Semantic checks failed. Machine code generation is disabled.");
		return 1;
	}

	spdlog::info("Semantic checks passed. Machine code generation is enabled.");
	std::cout << "Finished Building Symbol Table" << std::endl;
	return 0;
}
