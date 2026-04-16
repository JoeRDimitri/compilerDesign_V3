#include "parser.h"

bool parser::parse(const std::vector<token *> &vectorOfTokens)
{
	// File to write the derivation to.
	std::ofstream derivationOut("./outputs/Derivation"); // Open file for writing
														 // i for conditional debugging
	int i = 0;
	// Derivation vector is made to print out the current derivation.
	std::vector<std::string> derivation;
	// LineIndex variable
	int lineIndex = 0;
	// Is the current table entry an error?
	bool error;
	// Initial variables for parsing stack.
	parsingStack.push("$");
	parsingStack.push("START");
	// Initial variables for Derivation
	derivation.emplace_back("START");
	// Iterator to iterate overt the vector of tokens. Just a pointer iterator.
	auto vectorIterator = vectorOfTokens.begin();
	// Dereferencing the token point to get the actual token object.
	token *currentToken = (*vectorIterator);
	// While we havent reached the end of the program.
	while (parsingStack.top() != "$")
	{
		i++;
		// Get the nex parsing object.
		// Basically the top of the stack is determines if the current token is valid by analyzing the first and follow sets
		std::string topOfTheStack = parsingStack.top();

		// Case 1: The top of the stack is a terminal symbol.
		if (parsingTable.uniqueTerminalSymbols.count(topOfTheStack) != 0)
		{
			// If it is a terminal symbol,
			// then we need to match if the terminal symbol matches the same as the current Token
			if (topOfTheStack == (currentToken->getTypeName()))
			{
				// If it does, then success, we can remove this terminal symbol from the parsing stack
				//  We've successfully matched this token from the parsing stack.
				parsingStack.pop();
				// We can successfully remove this terminal symbol from the stack.
				derivation.erase(derivation.begin() + lineIndex);
				derivation.insert(derivation.begin() + lineIndex, currentToken->getLexeme());
				lineIndex++;
				vectorIterator++;
				currentToken = (*vectorIterator);
			}
			// Else we handle the error.
			else
			{
				// Handle error;
				std::string topOfStack;
				topOfStack = parsingStack.top();
				skipError(currentToken, vectorIterator, derivationOut, topOfStack, topOfTheStack, currentToken->getLine(), currentToken->getColumn());
				error = true;
			}
		}
		// Case 2: The top of the stack is a semantic action.
		else if (topOfTheStack.at(0) == '#')
		{
			// last token value need to handle the semanticaction.
			std::string lastTokenValue;
			// Avoid Crashing the program.
			if (vectorIterator != vectorOfTokens.begin())
			{
				lastTokenValue = (*(vectorIterator - 1))->getLexeme();
			}
			parsingStack.pop();
			// semanticFunction is the semantic action without the #
			std::string semanticFunction = topOfTheStack.substr(1);
			// handle the semantic action.
			semanticHandler.handleAction(semanticFunction, lastTokenValue);
			derivation.erase(derivation.begin() + lineIndex);
		}
		// Else case: not a terminal symbol or a semantic action : then it could be a non-terminal symbol or epsilon?
		else
		{
			// Epsilon case
			if (topOfTheStack == "EPSILON")
			{
				// We can simply dismiss it and ignore the epsilon.
				parsingStack.pop();
				derivation.erase(derivation.begin() + lineIndex);
				for (auto const &value : derivation)
				{
					derivationOut << value << " ";
				}
				derivationOut << std::endl;
				continue;
			}
			// Non terminal symbol.
			// Need to find the row and column value for the table
			// row will be whatever the top of the stack is. Columnvalue is whatever the name of the current token is.
			int rowValue = parsingTable.tableRows[topOfTheStack];
			int columnValue = parsingTable.tableColumns[((currentToken)->getTypeName())];
			tableEntry currentTableEntry = parsingTable.table[rowValue][columnValue];
			// The table dictates if we are on this non  terminal symbol with this current token, what rule we need to apply.
			// The rule that we apply is based on the first and follow set, which is integrated when the table is built.
			if (currentTableEntry.derivationRule != "error")
			{
				// Not an error meaning that the current non terminal
				parsingStack.pop();
				// We replace the top of the stack.
				derivation.erase(derivation.begin() + lineIndex);
				// We need to inverse the rule and push it onto the stack.
				inverseRHSMultiplePush(currentTableEntry, derivation, lineIndex);
				for (auto const &value : derivation)
				{
					derivationOut << value << " ";
				}
				derivationOut << std::endl;
			}
			else
			{
				// Need to check the follow set of the current the top of the stack.
				std::string topOfStack;
				topOfStack = parsingStack.top();
				// Handle error;
				skipError(currentToken, vectorIterator, derivationOut, topOfStack, topOfTheStack, currentToken->getLine(), currentToken->getColumn());
				error = true;
			}
		}
	}
	if ((currentToken->getTypeName() != "$") || (error == true))
	{
		return false;
	}
	else
		return true;
}

void parser::inverseRHSMultiplePush(tableEntry currentTableEntry, std::vector<std::string> &derivation, const int &lineIndex)
{
	std::string lineToInverseAndSplitThenPush = currentTableEntry.derivationRule;
	std::stack<std::string> items;
	int tempLineIndex = 0;
	while (lineToInverseAndSplitThenPush.at(tempLineIndex) != '>')
	{
		tempLineIndex++;
	}
	while (lineToInverseAndSplitThenPush.at(tempLineIndex) != '=')
	{
		tempLineIndex++;
	}
	tempLineIndex++;
	while (tempLineIndex < lineToInverseAndSplitThenPush.size())
	{
		std::string currentString;
		while (tempLineIndex != lineToInverseAndSplitThenPush.size() && lineToInverseAndSplitThenPush.at(tempLineIndex) != '\'' && lineToInverseAndSplitThenPush.at(tempLineIndex) != '<' && lineToInverseAndSplitThenPush.at(tempLineIndex) != '#' && lineToInverseAndSplitThenPush.at(tempLineIndex) != 'E')
		{
			tempLineIndex++;
		}
		if (tempLineIndex == lineToInverseAndSplitThenPush.size())
			break;
		if (lineToInverseAndSplitThenPush.at(tempLineIndex) == '\'')
		{
			tempLineIndex++;
			while (lineToInverseAndSplitThenPush.at(tempLineIndex) != '\'')
			{
				currentString += lineToInverseAndSplitThenPush.at(tempLineIndex);
				tempLineIndex++;
			}
			tempLineIndex++;
			items.push(currentString);
			continue;
		}
		else if (lineToInverseAndSplitThenPush.at(tempLineIndex) == '<')
		{
			tempLineIndex++;

			while (lineToInverseAndSplitThenPush.at(tempLineIndex) != '>')
			{
				currentString += lineToInverseAndSplitThenPush.at(tempLineIndex);
				tempLineIndex++;
			}
			tempLineIndex++;
			items.push(currentString);
			continue;
		}
		else if (lineToInverseAndSplitThenPush.at(tempLineIndex) == '#')
		{
			while (tempLineIndex != lineToInverseAndSplitThenPush.size() && lineToInverseAndSplitThenPush.at(tempLineIndex) != ' ')
			{
				currentString += lineToInverseAndSplitThenPush.at(tempLineIndex);
				tempLineIndex++;
			}
			tempLineIndex++;
			items.push(currentString);
			continue;
		}
		else if (lineToInverseAndSplitThenPush.at(tempLineIndex) == 'E')
		{
			while (tempLineIndex != lineToInverseAndSplitThenPush.size() && currentString != "EPSILON")
			{
				currentString += lineToInverseAndSplitThenPush.at(tempLineIndex);
				tempLineIndex++;
			}
			tempLineIndex++;
		}
	}
	while (!items.empty())
	{
		parsingStack.push(items.top());
		derivation.insert(derivation.begin() + lineIndex, items.top());

		items.pop();
	}
}

void parser::skipError(token *&currentToken, std::vector<token *>::const_iterator &vectorIterator, std::ofstream &derivationOut, const std::string &lookahead, const std::string &topOfTheStack, const int &line, const int &column)
{
	derivationOut << "Syntax error at (" << line << ", " << column << ")" << std::endl;
	std::string nextToken = (*(vectorIterator + 1))->getLexeme();
	if (nextToken == "$" || search(nextToken, topOfTheStack))
	{
		parsingStack.pop();
		vectorIterator++;
	}
	else
	{
		while ((!searchFirst(nextToken, topOfTheStack)) && (searchFirst("EPSILON", topOfTheStack) && !search(nextToken, topOfTheStack)))
		{
			vectorIterator++;
			nextToken = (*(vectorIterator + 1))->getLexeme();
			currentToken = (*vectorIterator);
		}
		vectorIterator++;
		currentToken = (*vectorIterator);
	}
}

bool parser::search(const std::string &nextToken, const std::string &topOfTheStack)
{
	if (first_and_follow::inVector(faf.followSet[topOfTheStack], nextToken))
	{
		return true;
	}
	else
		return false;
}

bool parser::searchFirst(std::string nextToken, std::string topOfTheStack)
{
	if (first_and_follow::inVector(faf.firstSet[topOfTheStack], nextToken))
	{
		return true;
	}
	else
		return false;
}

void parser::semanticActions::handleAction(std::string semanticFunction, std::string lastTokenValue)
{

	if (semanticFunction == "start")
	{
		startNode *newnode = new startNode();
		this->passAlong("start", newnode);
	}
	else if (semanticFunction == "aparamstail")
	{
		aparamstailNode *newnode = new aparamstailNode();
		this->passAlong("aparamstail", newnode);
	}
	else if (semanticFunction == "arithexpr")
	{
		arithexprNode *newnode = new arithexprNode();
		this->passAlong("arithexpr", newnode);
	}
	else if (semanticFunction == "arraysize")
	{
		arraysizeNode *newnode = new arraysizeNode();
		this->passAlong("arraysize", newnode);
	}
	else if (semanticFunction == "attributedecl")
	{
		attributedeclNode *newnode = new attributedeclNode();
		this->passAlong("attributedecl", newnode);
	}
	else if (semanticFunction == "class")
	{
		classNode *newnode = new classNode();
		this->passAlong("class", newnode);
	}
	else if (semanticFunction == "impl")
	{
		implNode *newnode = new implNode();
		this->passAlong("impl", newnode);
	}
	else if (semanticFunction == "func")
	{
		funcNode *newnode = new funcNode();
		this->passAlong("func", newnode);
	}
	else if (semanticFunction == "factor")
	{
		factorNode *newnode = new factorNode();
		this->passAlong("factor", newnode);
	}
	else if (semanticFunction == "idnest2")
	{
		idnest2Node *newnode = new idnest2Node();
		this->passAlong("idnest2", newnode);
	}
	else if (semanticFunction == "funcbody")
	{
		funcbodyNode *newnode = new funcbodyNode();
		this->passAlong("funcbody", newnode);
	}
	else if (semanticFunction == "funcdecl")
	{
		funcdeclNode *newnode = new funcdeclNode();
		this->passAlong("funcdecl", newnode);
	}
	else if (semanticFunction == "indice")
	{
		indiceNode *newnode = new indiceNode();
		this->passAlong("indice", newnode);
	}
	else if (semanticFunction == "localvardecl")
	{
		localvardeclNode *newnode = new localvardeclNode();
		this->passAlong("localvardecl", newnode);
	}
	else if (semanticFunction == "localvardeclorstat")
	{
		localvardeclorstatNode *newnode = new localvardeclorstatNode();
		this->passAlong("localvardeclorstat", newnode);
	}
	else if (semanticFunction == "prog")
	{
		progNode *newnode = new progNode();
		this->passAlong("prog", newnode);
	}
	else if (semanticFunction == "condition")
	{
		conditionNode *newnode = new conditionNode();
		this->passAlong("condition", newnode);
	}
	else if (semanticFunction == "then")
	{
		thenNode *newnode = new thenNode();
		this->passAlong("then", newnode);
	}
	else if (semanticFunction == "felse")
	{
		felseNode *newnode = new felseNode();
		this->passAlong("felse", newnode);
	}
	else if (semanticFunction == "statblock")
	{
		statblockNode *newnode = new statblockNode();
		this->passAlong("statblock", newnode);
	}
	else if (semanticFunction == "returntype")
	{
		returntypeNode *newnode = new returntypeNode();
		this->passAlong("returntype", "returntype", newnode);
	}
	else if (semanticFunction == "term")
	{
		termNode *newnode = new termNode();
		this->passAlong("term", newnode);
	}

	else if (semanticFunction == "intnum")
	{
		intNode *newnode = new intNode();
		this->makeLeaf("intnum", lastTokenValue, newnode);
	}
	else if (semanticFunction == "p")
	{
		epsilonNode *newnode = new epsilonNode();
		this->makeLeaf("epsilon", lastTokenValue, newnode);
	}
	else if (semanticFunction == "addop")
	{
		addopNode *newnode = new addopNode();
		this->makeLeaf("addop", lastTokenValue, newnode);
	}
	else if (semanticFunction == "assign")
	{
		assignNode *newnode = new assignNode();
		this->makeLeaf("assign", lastTokenValue, newnode);
	}
	else if (semanticFunction == "id")
	{
		idNode *newnode = new idNode();
		this->makeLeaf("id", lastTokenValue, newnode);
	}
	else if (semanticFunction == "relop")
	{
		relopNode *newnode = new relopNode();
		this->makeLeaf("relop", lastTokenValue, newnode);
	}
	else if (semanticFunction == "self")
	{
		selfNode *newnode = new selfNode();
		this->makeLeaf("self", lastTokenValue, newnode);
	}
	else if (semanticFunction == "multop")
	{
		multopNode *newnode = new multopNode();
		this->makeLeaf("multop", lastTokenValue, newnode);
	}
	else if (semanticFunction == "void")
	{
		voidNode *newnode = new voidNode();
		this->makeLeaf("void", lastTokenValue, newnode);
	}
	else if (semanticFunction == "sign")
	{
		signNode *newnode = new signNode();
		this->makeLeaf("sign", lastTokenValue, newnode);
	}
	else if (semanticFunction == "if")
	{
		ifNode *newnode = new ifNode();
		this->makeLeaf("if", lastTokenValue, newnode);
	}
	else if (semanticFunction == "while")
	{
		whileNode *newnode = new whileNode();
		this->makeLeaf("while", lastTokenValue, newnode);
	}
	else if (semanticFunction == "read")
	{
		readNode *newnode = new readNode();
		this->makeLeaf("read", lastTokenValue, newnode);
	}
	else if (semanticFunction == "write")
	{
		writeNode *newnode = new writeNode();
		this->makeLeaf("write", lastTokenValue, newnode);
	}
	else if (semanticFunction == "return")
	{
		returnNode *newnode = new returnNode();
		this->makeLeaf("return", lastTokenValue, newnode);
	}
	else if (semanticFunction == "type")
	{
		typeNode *newnode = new typeNode();
		this->makeLeaf("type", lastTokenValue, newnode);
	}
	else if (semanticFunction == "visibility")
	{
		visibilityNode *newnode = new visibilityNode();
		this->makeLeaf("visibility", lastTokenValue, newnode);
	}
	else if (semanticFunction == "floatnum")
	{
		floatnumNode *newnode = new floatnumNode();
		this->makeLeaf("floatnum", lastTokenValue, newnode);
	}

	else if (semanticFunction == "reptclassdecl4")
	{
		reptclassdecl4Node *newnode = new reptclassdecl4Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptclassdecl4", "{", 'E', v, newnode);
	}
	else if (semanticFunction == "classdecl")
	{
		classdeclNode *newnode = new classdeclNode();
		std::vector<std::string> v = {"id", "optclassdecl2", "reptclassdecl4"};
		this->makeSubTree("classdecl", "classdecl", 3, v, newnode);
	}
	else if (semanticFunction == "aparams")
	{
		aparamsNode *newnode = new aparamsNode();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("aparams", "aparams", 'E', v, newnode);
	}
	else if (semanticFunction == "funcdef")
	{
		funcdefNode *newnode = new funcdefNode();
		std::vector<std::string> v = {"funchead", "funcbody"};
		this->makeSubTree("funcdef", "funcdef", 2, v, newnode);
	}
	else if (semanticFunction == "expr")
	{
		exprNode *newnode = new exprNode();
		std::vector<std::string> v = {"arithexpr", "expr2", "epsilon"};
		this->makeSubTree("expr", "expr", 2, v, newnode);
	}
	else if (semanticFunction == "expr2")
	{
		expr2Node *newnode = new expr2Node();
		std::vector<std::string> v = {"relop", "arithexpr"};
		this->makeSubTree("expr2", "expr2", 2, v, newnode);
	}
	else if (semanticFunction == "reptfparams3")
	{
		reptfparams3Node *newnode = new reptfparams3Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptfparams3", "dimlist", 'E', v, newnode);
	}
	else if (semanticFunction == "reptfparams4")
	{
		reptfparams4Node *newnode = new reptfparams4Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptfparams4", "param", 'E', v, newnode);
	}
	else if (semanticFunction == "param")
	{
		paramNode *newnode = new paramNode();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("param", "param", 'E', v, newnode);
	}
	else if (semanticFunction == "fparams")
	{
		fparamsNode *newnode = new fparamsNode();
		std::vector<std::string> v = {"param", "reptfparams4"};
		this->makeSubTree("fparams", "funcparams", 'E', v, newnode);
	}
	else if (semanticFunction == "factor2")
	{
		factor2Node *newnode = new factor2Node();
		std::vector<std::string> v = {"not", "sign", "factor"};
		this->makeSubTree("factor2", "factor2", 2, v, newnode);
	}
	else if (semanticFunction == "selfandidnest2")
	{
		selfandidnest2Node *newnode = new selfandidnest2Node();
		std::vector<std::string> v = {"self", "idnest2"};
		this->makeSubTree("selfandidnest2", "selfandidnest2", 2, v, newnode);
	}
	else if (semanticFunction == "reptvariable2")
	{
		reptvariable2Node *newnode = new reptvariable2Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptvariable2", "reptvariable2", 'E', v, newnode);
	}
	else if (semanticFunction == "aparamsandid")
	{
		aparamsandidNode *newnode = new aparamsandidNode();
		std::vector<std::string> v = {"id", "aparams"};
		this->makeSubTree("aparamsandid", "functioncall", 2, v, newnode);
	}
	else if (semanticFunction == "reptvariable2andid")
	{
		reptvariable2andidNode *newnode = new reptvariable2andidNode();
		std::vector<std::string> v = {"id", "reptvariable2"};
		this->makeSubTree("reptvariable2andid", "term", 2, v, newnode);
	}
	else if (semanticFunction == "reptfuncbody1")
	{
		reptfuncbody1Node *newnode = new reptfuncbody1Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptfuncbody1", "funcbody", 'E', v, newnode);
	}
	else if (semanticFunction == "funchead")
	{
		funcheadNode *newnode = new funcheadNode();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("funchead", "funchead", 'E', v, newnode);
	}
	else if (semanticFunction == "reptimpldef3")
	{
		reptimpldef3Node *newnode = new reptimpldef3Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptimpldef3", "reptimpldef3", 'E', v, newnode);
	}
	else if (semanticFunction == "impldef")
	{
		impldefNode *newnode = new impldefNode();
		std::vector<std::string> v = {"funchead", "reptimpldef3"};
		this->makeSubTree("impldef", "impldef", 2, v, newnode);
	}
	else if (semanticFunction == "reptoptclassdecl22")
	{
		reptoptclassdecl22Node *newnode = new reptoptclassdecl22Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptoptclassdecl22", "reptoptclassdecl22", 'E', v, newnode);
	}
	else if (semanticFunction == "optclassdecl2")
	{
		optclassdecl2Node *newnode = new optclassdecl2Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("optclassdecl2", "isalist", 'E', v, newnode);
	}
	else if (semanticFunction == "reptprog0")
	{
		reptprog0Node *newnode = new reptprog0Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptprog0", "prog", 'E', v, newnode);
	}
	else if (semanticFunction == "relexpr")
	{
		relexprNode *newnode = new relexprNode();
		std::vector<std::string> v = {"arithexpr", "relop", "arithexpr"};
		this->makeSubTree("relexpr", "relexpr", 3, v, newnode);
	}
	else if (semanticFunction == "funcdeclfam")
	{
		funcdeclfamNode *newnode = new funcdeclfamNode();
		std::vector<std::string> v = {"visibility", "funcdecl"};
		this->makeSubTree("funcdecl", "funcdecl", 2, v, newnode);
	}
	else if (semanticFunction == "attributedeclfam")
	{
		attributedeclfamNode *newnode = new attributedeclfamNode();
		std::vector<std::string> v = {"visibility", "attributedecl"};
		this->makeSubTree("attributedeclfam", "attributedecl", 2, v, newnode);
	}
	else if (semanticFunction == "reptstatblock1")
	{
		reptstatblock1Node *newnode = new reptstatblock1Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptstatblock1", "reptstatblock1", 'E', v, newnode);
	}
	else if (semanticFunction == "vardecl")
	{
		vardeclNode *newnode = new vardeclNode();
		std::vector<std::string> v = {"id", "type", "reptvardecl3"};
		this->makeSubTree("vardecl", "vardecl", 3, v, newnode);
	}
	else if (semanticFunction == "reptvardecl3")
	{
		reptvardecl3Node *newnode = new reptvardecl3Node();
		std::vector<std::string> v = {"epsilon"};
		this->makeSubTree("reptvardecl3", "dimlist", 'E', v, newnode);
	}

	else if (semanticFunction == "dot")
	{
		dotNode *newnode = new dotNode();
		this->makeBinarySubTree("dot", 0, newnode);
	}
	else if (semanticFunction == "rightrecarithexpr")
	{
		rightrecarithexprNode *newnode = new rightrecarithexprNode();
		this->makeBinarySubTree("rightrecarithexpr", 1, newnode);
	}
	else if (semanticFunction == "rightrecterm")
	{
		rightrectermNode *newnode = new rightrectermNode();
		this->makeBinarySubTree("rightrecterm", 1, newnode);
	}
	else if (semanticFunction == "reptstatement4")
	{
		reptstatement4Node *newnode = new reptstatement4Node();
		this->makeBinarySubTree("reptstatement4", 1, newnode);
	}

	else if (semanticFunction == "whilestatement")
	{
		whilestatementNode *newnode = new whilestatementNode();
		this->makeBinarySubTreeWithHead("whilestatement", 3, newnode);
	}
	else if (semanticFunction == "readstatement")
	{
		readstatementNode *newnode = new readstatementNode();
		this->makeBinarySubTreeWithHead("readstatement", 2, newnode);
	}
	else if (semanticFunction == "writestatement")
	{
		writestatementNode *newnode = new writestatementNode();
		this->makeBinarySubTreeWithHead("writestatement", 2, newnode);
	}
	else if (semanticFunction == "freturnstatement")
	{
		freturnstatementNode *newnode = new freturnstatementNode();
		this->makeBinarySubTreeWithHead("freturnstatement", 2, newnode);
	}
	else if (semanticFunction == "ifstatement")
	{
		ifstatementNode *newnode = new ifstatementNode();
		this->makeBinarySubTreeWithHead("ifstatement", 4, newnode);
	}
}

// Creates a terminal (leaf) AST node with no children, sets its value to the last matched token,
// and pushes it onto the semantic stack.
void parser::semanticActions::makeLeaf(std::string nodeType, std::string lastTokenValue, node *newnode)
{
	newnode->nodeType = nodeType;
	newnode->semanticMeaning = nodeType;
	newnode->nodeValue = lastTokenValue; // store the actual token text (e.g. identifier name, literal)
	newnode->isLeaf = true;

	semanticStackPtr->push(newnode); // push leaf so parent nodes can adopt it later
}

// Pops numOfPops nodes; promotes the LAST popped node as the parent (copying its data into newnode),
// then wires the remaining nodes as its children and pushes the result onto the semantic stack.
void parser::semanticActions::makeBinarySubTreeWithHead(std::string nodeType, int numOfPops, node *newnode)
{
	std::vector<node *> nodesOnStack;
	std::vector<node *> children;
	std::stack<node *> reverseChildren; // used to restore stack-pop order

	// pop numOfPops nodes, preserving left-to-right order via reverseChildren
	for (int i = numOfPops - 1; i >= 0; i--)
	{
		if (semanticStackPtr->empty())
		{
			spdlog::error("[makeBinarySubTreeWithHead] semanticStack empty on pop {} of {} for nodeType='{}'", numOfPops - 1 - i, numOfPops, nodeType);
			return;
		}
		reverseChildren.push(semanticStackPtr->top());
		semanticStackPtr->pop();
	}
	while (!reverseChildren.empty())
	{
		nodesOnStack.emplace_back(reverseChildren.top());
		reverseChildren.pop();
	}

	// the last node in the ordered list becomes the parent; copy its data into newnode
	node *parentNode = nodesOnStack[nodesOnStack.size() - 1];
	newnode->copyNode(parentNode, newnode, nodeType);
	delete parentNode; // original parent shell is no longer needed after copy

	// all nodes except the promoted parent become children (in reverse so leftmost is first)
	for (int k = nodesOnStack.size() - 2; k >= 0; k--)
	{
		children.emplace_back(nodesOnStack.at(k));
	}
	// wire sibling linked-list pointers across all children
	for (int i = 0; i < children.size(); i++)
	{
		if (i == 0)
		{
			if (i + 1 < children.size())
			{
				children.at(i)->headOfSibling = children.at(i); // first child is sibling-list head
				children.at(i)->rightSibling = children.at(i + 1);
			}
		}
		else if (i == children.size() - 1)
		{
			if (i - 1 > -1)
			{
				children.at(i)->headOfSibling = children.at(0);
				children.at(i)->leftSibling = children.at(i - 1); // last child has no right sibling
			}
		}
		else
		{
			children.at(i)->headOfSibling = children.at(0);
			children.at(i)->rightSibling = children.at(i + 1);
			children.at(i)->leftSibling = children.at(i - 1);
		}
	}
	// set parent pointer on every child back up to newnode
	for (node *value : children)
	{
		value->parent = newnode;
	}
	newnode->children = children;
	semanticStackPtr->push(newnode); // push completed subtree
}

// Builds a two-child subtree from the semantic stack.
// i==0: pops 2 nodes [first, second] and makes them direct children of newnode.
// i==1: pops 3 nodes, promotes the middle node (second) as the parent via copyNode,
//        and makes first & third the two children (second is discarded after copy).
void parser::semanticActions::makeBinarySubTree(std::string nodeType, int i, node *newnode)
{
	if (i == 0)
	{
		// simple two-child case: pop two nodes and attach them as left/right children
		if (semanticStackPtr->empty())
		{
			spdlog::error("[makeBinarySubTree i=0] semanticStack empty popping 'first' for nodeType='{}'", nodeType);
			return;
		}
		node *first = semanticStackPtr->top();
		semanticStackPtr->pop();
		if (semanticStackPtr->empty())
		{
			spdlog::error("[makeBinarySubTree i=0] semanticStack empty popping 'second' for nodeType='{}'", nodeType);
			return;
		}
		node *second = semanticStackPtr->top();
		semanticStackPtr->pop();

		std::vector<node *> v = {first, second};
		newnode->children = v;
		newnode->nodeType = nodeType;
		newnode->semanticMeaning = nodeType;

		// wire sibling pointers: first <-> second
		first->headOfSibling = first;
		first->leftSibling = nullptr;
		first->rightSibling = second;
		first->parent = newnode;
		second->headOfSibling = first;
		second->leftSibling = first;
		second->rightSibling = nullptr;
		second->parent = newnode;
		newnode->children = v;
		semanticStackPtr->push(newnode);
	}
	else if (i == 1)
	{
		// three-node pop: middle node becomes the parent, outer two become children
		if (semanticStackPtr->empty())
		{
			spdlog::error("[makeBinarySubTree i=1] semanticStack empty popping 'first' for nodeType='{}'", nodeType);
			return;
		}
		node *first = semanticStackPtr->top();
		semanticStackPtr->pop();
		if (semanticStackPtr->empty())
		{
			spdlog::error("[makeBinarySubTree i=1] semanticStack empty popping 'second' for nodeType='{}'", nodeType);
			return;
		}
		node *second = semanticStackPtr->top(); // promoted to parent
		semanticStackPtr->pop();
		if (semanticStackPtr->empty())
		{
			spdlog::error("[makeBinarySubTree i=1] semanticStack empty popping 'third' for nodeType='{}'", nodeType);
			return;
		}
		node *third = semanticStackPtr->top();
		semanticStackPtr->pop();

		std::vector<node *> v = {first, third};
		newnode->nodeType = nodeType;
		newnode->semanticMeaning = nodeType;
		newnode->copyNode(second, newnode, nodeType); // copy second's data into newnode
		newnode->children = v;
		delete second; // second has been replaced by newnode, free its memory
		// wire sibling pointers: first <-> third
		first->headOfSibling = first;
		first->leftSibling = nullptr;
		first->rightSibling = third;
		first->parent = newnode;
		third->headOfSibling = first;
		third->leftSibling = first;
		third->rightSibling = nullptr;
		third->parent = newnode;
		semanticStackPtr->push(newnode);
	}
}

// Pops nodes from the semantic stack, links them as siblings, attaches them as children
// of newnode, then pushes the resulting subtree back onto the semantic stack.
// amountOfPops == 'E' means: pop until an epsilon sentinel is found.
// match specifies the expected nodeType(s) of nodes to collect.
void parser::semanticActions::makeSubTree(std::string nodeType, std::string semanticMeaning, int amountOfPops, std::vector<std::string> match, node *newnode)
{
	std::vector<node *> nodesFromTheStack; // final ordered list of children
	std::stack<node *> reverseChildren;	   // temp stack used to reverse pop order

	// 'E' (ascii 69) is a sentinel: pop everything down to the nearest epsilon marker
	if (amountOfPops == 'E')
	{
		if (semanticStackPtr->empty())
		{
			spdlog::error("[makeSubTree epsilon] semanticStack empty at start for nodeType='{}'", nodeType);
			return;
		}
		node *n = semanticStackPtr->top();
		// collect nodes until the epsilon boundary node is reached
		while (n->nodeType != "epsilon")
		{
			reverseChildren.push(n);
			semanticStackPtr->pop();
			if (semanticStackPtr->empty())
			{
				spdlog::error("[makeSubTree epsilon] semanticStack exhausted before finding epsilon sentinel for nodeType='{}'", nodeType);
				return;
			}
			n = semanticStackPtr->top();
		}
		semanticStackPtr->pop(); // discard the epsilon sentinel itself
		// reverse the collected nodes back into correct (left-to-right) order
		while (!reverseChildren.empty())
		{
			nodesFromTheStack.emplace_back(reverseChildren.top());
			reverseChildren.pop();
		}
	}
	// When the number of nodes to pop equals the number of types in match, do a strict ordered match
	else if (amountOfPops == match.size())
	{
		bool good = false;
		if (semanticStackPtr->empty())
		{
			spdlog::error("[makeSubTree match.size] semanticStack empty at start for nodeType='{}'", nodeType);
			return;
		}
		// Try matching from the top of the stack downward (reverse order against match)
		if (semanticStackPtr->top()->nodeType == match.at(amountOfPops - 1))
		{
			for (int i = amountOfPops - 1; i >= 0; i--)
			{
				if (semanticStackPtr->empty())
				{
					spdlog::error("[makeSubTree match.size forward-loop] semanticStack empty at i={} for nodeType='{}'", i, nodeType);
					break;
				}
				node *n = semanticStackPtr->top();
				if (n->nodeType == match.at(i))
				{
					reverseChildren.push(n);
					semanticStackPtr->pop();
					// mark success once we've processed past the first element
					if (i == 1)
					{
						good = true;
					}
				}
			}
		}
		else
		{
			// Fallback: try matching in forward order if the reverse attempt didn't align
			if (!good)
				for (int i = 0; i < amountOfPops; i++)
				{
					if (semanticStackPtr->empty())
					{
						spdlog::error("[makeSubTree match.size fallback-loop] semanticStack empty at i={} for nodeType='{}'", i, nodeType);
						break;
					}
					node *n = semanticStackPtr->top();
					if (n->nodeType == match.at(i))
					{
						reverseChildren.push(n);
						semanticStackPtr->pop();
						// mark success once the last expected node is matched
						if (i == amountOfPops - 1)
						{
							good = true;
						}
					}
				}
		}

		// Abort if no valid match was found — indicates a malformed semantic stack
		if (!good)
		{
			std::cout << "Mistake matching in " + nodeType << std::endl;
			exit(1);
		}
		// Drain reverseChildren into the final ordered list
		while (!reverseChildren.empty())
		{
			nodesFromTheStack.emplace_back(reverseChildren.top());
			reverseChildren.pop();
		}
	}
	// amountOfPops != match.size(): pop amountOfPops nodes, each matching any type in match
	else
	{
		for (int i = 0; i < amountOfPops; i++)
		{
			if (semanticStackPtr->empty())
			{
				spdlog::error("[makeSubTree else-loop] semanticStack empty at i={} of {} for nodeType='{}'", i, amountOfPops, nodeType);
				break;
			}
			node *n = semanticStackPtr->top();
			int k = 0;
			// scan the match list to find a type that fits the current top-of-stack node
			for (; k < match.size(); k++)
			{
				if (n->nodeType == match.at(k))
				{
					nodesFromTheStack.emplace_back(n);
					semanticStackPtr->pop();
					break; // found a match, move on to the next pop
				}
				else
				{
					continue;
				}
			}

			// if k reached the end, no match was found — semantic stack is inconsistent
			if (k == match.size())
			{
				std::cout << "problem with stack cant find the matching items  : " + nodeType << std::endl;
				exit(1);
			}
		}
	}

	// Wire sibling pointers so each child knows its neighbours and the head of the sibling list
	for (int i = 0; i < nodesFromTheStack.size(); i++)
	{
		if (i == 0)
		{
			// first child: it is its own sibling head; point right to next sibling
			if (i + 1 < nodesFromTheStack.size())
			{
				nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(i);
				nodesFromTheStack.at(i)->rightSibling = nodesFromTheStack.at(i + 1);
			}
		}
		else if (i == nodesFromTheStack.size() - 1)
		{
			// last child: point back to previous sibling; head is always the first child
			if (i - 1 > -1)
			{
				nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(0);
				nodesFromTheStack.at(i)->leftSibling = nodesFromTheStack.at(i - 1);
			}
		}
		else
		{
			// middle children: link both left and right siblings
			nodesFromTheStack.at(i)->headOfSibling = nodesFromTheStack.at(0);
			nodesFromTheStack.at(i)->rightSibling = nodesFromTheStack.at(i + 1);
			nodesFromTheStack.at(i)->leftSibling = nodesFromTheStack.at(i - 1);
		}
	}

	// Attach the collected nodes as children of the new parent node
	newnode->children = nodesFromTheStack;
	newnode->nodeType = nodeType;
	newnode->semanticMeaning = semanticMeaning;

	// Point every child's parent back up to newnode
	for (node *value : nodesFromTheStack)
	{
		value->parent = newnode;
	}

	// If no children were found, treat this node as an epsilon (empty) production
	if (nodesFromTheStack.size() == 0)
	{
		newnode->semanticMeaning = "epsilon";
	}
	// Push the completed subtree root onto the semantic stack for later composition
	semanticStackPtr->push(newnode);
}

// Pops the top node, copies its data into newnode (re-typing it as nodeType),
// re-parents its children, then either sets the AST root (if "start") or pushes newnode back.
void parser::semanticActions::passAlong(std::string nodeType, node *newnode)
{
	node *topofthestack = semanticStackPtr->top();
	semanticStackPtr->pop();
	newnode->copyNode(topofthestack, newnode, nodeType); // transfer all fields to the typed node
	adoptChildren(newnode, topofthestack);				 // re-point children's parent to newnode

	// if the top was epsilon, propagate that meaning so the tree knows this branch is empty
	if (topofthestack->nodeType == "epsilon" || topofthestack->semanticMeaning == "epsilon")
	{
		newnode->semanticMeaning = "epsilon";
	}
	delete topofthestack; // original generic node replaced by typed newnode
	if (nodeType == "start")
	{
		astPtr->treeHead = dynamic_cast<startNode *>(newnode);
	} // anchor the tree root
	else
	{
		semanticStackPtr->push(newnode);
	}
}

// Overload of passAlong that also overrides the semantic meaning after copying.
// The explicit semanticMeaning can be overridden back to "epsilon" if the source node was epsilon.
void parser::semanticActions::passAlong(std::string nodeType, std::string semanticMeaning, node *newnode)
{
	node *topofthestack = semanticStackPtr->top();
	semanticStackPtr->pop();
	newnode->copyNode(topofthestack, newnode, nodeType);
	newnode->semanticMeaning = semanticMeaning; // apply the caller-provided semantic label
	adoptChildren(newnode, topofthestack);
	// epsilon from source overrides the provided semanticMeaning
	if (topofthestack->nodeType == "epsilon" || topofthestack->semanticMeaning == "epsilon")
	{
		newnode->semanticMeaning = "epsilon";
	}
	delete topofthestack;
	if (nodeType == "start")
	{
		astPtr->treeHead = dynamic_cast<startNode *>(newnode); // anchor the tree root
	}
	else
	{
		semanticStackPtr->push(newnode);
	}
}

// Transfers ownership of oldparent's children to newparent by updating each child's parent pointer.
// If oldparent had no children, marks newparent as a leaf.
void parser::semanticActions::adoptChildren(node *newparent, node *oldparent)
{
	if (oldparent->children.size() == 0)
	{
		newparent->isLeaf = true; // no children means this node is a terminal in the AST
		return;
	}

	else if (oldparent->children.size() > 0)
	{
		// re-point every child to the new parent node
		for (node *child : oldparent->children)
		{
			child->parent = newparent;
		}
	}
}

void parser::abstractSyntaxTree::printTree()
{
	std::ofstream astout("./outputs/AbstractSyntaxTreeViz.ast.outast");
	node *head = this->treeHead;
	astout << "[" << head->semanticMeaning << "]\n";
	std::vector<node *> &v = head->children;
	for (int k = 0; k < (int)v.size(); k++)
	{
		bool isLast = (k == (int)v.size() - 1);
		traverseTree(v.at(k), "", isLast, astout);
	}
}

void parser::abstractSyntaxTree::traverseTree(node *head, std::string prefix, bool isLast, std::ofstream &ao)
{
	if (head->semanticMeaning == "epsilon")
		return;

	std::string connector = isLast ? "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 " : "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 ";
	std::string childPrefix = prefix + (isLast ? "    " : "\xE2\x94\x82   ");

	// Build display label: show semanticMeaning, and for leaves append the value
	std::string label = head->semanticMeaning;
	if (head->isLeaf && !head->nodeValue.empty())
	{
		label += " (" + head->nodeValue + ")";
	}

	ao << prefix << connector << label << "\n";

	// Collect non-epsilon children
	std::vector<node *> visibleChildren;
	for (node *child : head->children)
	{
		if (child->semanticMeaning != "epsilon")
			visibleChildren.push_back(child);
	}

	for (int k = 0; k < (int)visibleChildren.size(); k++)
	{
		bool childIsLast = (k == (int)visibleChildren.size() - 1);
		traverseTree(visibleChildren.at(k), childPrefix, childIsLast, ao);
	}
}

// Prints the global symbol table to symbolTable.txt in a nested box format.
void parser::abstractSyntaxTree::printSymbolTable(node *head)
{
	std::ofstream out("./outputs/symbolTable.txt");
	const int W = 83;

	auto rep = [](char c, int n) -> std::string
	{ return std::string(n, c); };

	// Fit/truncate string to exactly w chars
	auto fit = [](const std::string &s, int w) -> std::string
	{
		if ((int)s.size() >= w)
			return s.substr(0, w);
		return s + std::string(w - (int)s.size(), ' ');
	};

	// Map stored kinds to display labels
	auto dkind = [](const std::string &k) -> std::string
	{
		if (k == "variable")
			return "local";
		if (k == "parameter")
			return "param";
		if (k == "attribute")
			return "data";
		if (k == "function head")
			return "function";
		return k;
	};

	// Build "(p1type, p2type):rettype" from a funchead/funcdef entry
	auto funcSig = [&](const node::symbolTableEntry *e) -> std::string
	{
		std::string sig = "(";
		if (e->hasLink && e->link)
		{
			bool first = true;
			for (auto &[pn, pe] : *e->link)
			{
				if (pe->kind == "parameter")
				{
					if (!first)
						sig += ", ";
					sig += pe->type;
					first = false;
				}
			}
		}
		sig += "):" + e->type;
		return sig;
	};

	// ── Depth-0 (global box) ─────────────────────────────────────────────────
	// Total width = 83
	auto sep0 = [&]() -> std::string
	{ return rep('=', W); };

	auto hdr0 = [&](const std::string &txt) -> std::string
	{
		std::string r = "| " + txt;
		r += rep(' ', W - 1 - (int)r.size());
		r += "|";
		return r;
	};

	// "| kind(12) | name(fill) |"  total = 83
	// 2 + 12 + 2 + name + spaces + 1 = 83  →  name+spaces = 66
	auto row0 = [&](const std::string &kind, const std::string &name) -> std::string
	{
		std::string r = "| " + fit(kind, 12) + "| " + name;
		r += rep(' ', W - 1 - (int)r.size());
		r += "|";
		return r;
	};

	// ── Depth-1 (class / function sub-table) ─────────────────────────────────
	// "|    " (5) + inner(75) + "  |" (3) = 83
	// inner columns: "| " + kind(10) + "| " + name(12) + "| " + type(46) + "|"
	//              =  2  +  10  +  2  +  12  +  2  +  46  + 1 = 75  ✓
	auto sep1 = [&]() -> std::string
	{ return "|    " + rep('=', 75) + "  |"; };

	auto hdr1 = [&](const std::string &txt) -> std::string
	{
		std::string inner = "| " + txt;
		inner += rep(' ', 74 - (int)inner.size());
		inner += "|"; // inner = 75 chars
		return "|    " + inner + "  |";
	};

	auto row1 = [&](const std::string &kind, const std::string &name,
					const std::string &type = "", const std::string &vis = "") -> std::string
	{
		std::string inner;
		if (vis.empty())
		{
			inner = "| " + fit(kind, 10) + "| " + fit(name, 12) + "| " + fit(type, 46) + "|";
		}
		else
		{
			inner = "| " + fit(kind, 10) + "| " + fit(name, 12) + "| " + fit(type, 34) + "| " + fit(vis, 10) + "|";
		}
		return "|    " + inner + "  |";
	};

	// ── Depth-2 (param/local table inside class method) ──────────────────────
	// "|    |     " (11) + inner(66) + "  |  |" (6) = 83
	// inner columns: "| " + kind(10) + "| " + name(12) + "| " + type(37) + "|"
	//              =  2  +  10  +  2  +  12  +  2  +  37  + 1 = 66  ✓
	auto sep2 = [&]() -> std::string
	{ return "|    |     " + rep('=', 66) + "  |  |"; };

	auto hdr2 = [&](const std::string &txt) -> std::string
	{
		std::string inner = "| " + txt;
		inner += rep(' ', 65 - (int)inner.size());
		inner += "|"; // inner = 66 chars
		return "|    |     " + inner + "  |  |";
	};

	auto row2 = [&](const std::string &kind, const std::string &name,
					const std::string &type = "") -> std::string
	{
		std::string inner = "| " + fit(kind, 10) + "| " + fit(name, 12) + "| " + fit(type, 37) + "|";
		return "|    |     " + inner + "  |  |";
	};

	// ── Render ────────────────────────────────────────────────────────────────
	out << sep0() << "\n";
	out << hdr0("table: global") << "\n";
	out << sep0() << "\n";

	for (auto &[ename, entry] : head->stMap)
	{
		if (ename.empty())
			continue;

		if (entry->kind == "class" || entry->kind == "implementation")
		{
			out << row0("class", entry->name) << "\n";
			out << sep1() << "\n";
			out << hdr1("table: " + entry->name) << "\n";
			out << sep1() << "\n";

			// inherit row: type field stores comma-separated parent names (or empty)
			std::string inh = entry->type.empty() ? "none" : entry->type;
			out << row1("inherit", inh) << "\n";

			// members
			if (entry->hasLink && entry->link)
			{
				for (auto &[mname, mentry] : *entry->link)
				{
					if (mname.empty())
						continue;
					if (mentry->kind == "attribute")
					{
						out << row1("data", mentry->name, mentry->type, mentry->visibility) << "\n";
					}
					else
					{
						// function or function head
						out << row1("function", mentry->name, funcSig(mentry), mentry->visibility) << "\n";
						// nested param sub-table
						if (mentry->hasLink && mentry->link && !mentry->link->empty())
						{
							out << sep2() << "\n";
							out << hdr2("table: " + entry->name + "::" + mentry->name) << "\n";
							out << sep2() << "\n";
							for (auto &[pname, pentry] : *mentry->link)
							{
								out << row2(dkind(pentry->kind), pentry->name, pentry->type) << "\n";
							}
							out << sep2() << "\n";
						}
					}
				}
			}
			out << sep1() << "\n";
		}
		else if (entry->kind == "function")
		{
			out << row0("function", entry->name) << "\n";
			out << sep1() << "\n";
			out << hdr1("table: ::" + entry->name) << "\n";
			out << sep1() << "\n";
			if (entry->hasLink && entry->link)
			{
				for (auto &[vname, ventry] : *entry->link)
				{
					if (vname.empty())
						continue;
					out << row1(dkind(ventry->kind), ventry->name, ventry->type) << "\n";
				}
			}
			out << sep1() << "\n";
		}
	}
	out << sep0() << "\n";
}

// All logic is now in printSymbolTable; kept for ABI compatibility.
void parser::abstractSyntaxTree::printSymbolTableRec(std::map<std::string, node::symbolTableEntry *> *link, int &depth, std::ofstream &ao)
{
}
