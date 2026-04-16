#pragma once
#include <string>
#include <vector>
#include <map>

class visitor; // global scope, NOT nested inside node

class node
{
public:
	struct symbolTableEntry
	{
		bool hasLink = false;
		std::string name;
		std::string kind;
		std::string type;
		std::string visibility;
		std::map<std::string, symbolTableEntry *> *link;
	};

	node *headOfSibling;
	node *leftSibling;
	node *rightSibling;
	node *parent;
	std::vector<node *> children;
	std::string nodeType;
	std::string semanticMeaning;
	std::string nodeValue;
	bool isLeaf = false;
	symbolTableEntry stEntry;
	std::map<std::string, symbolTableEntry *> stMap;

	void copyNode(node *oldnode, node *newnode, std::string nameOfNewNode);

	node() {};
	node(std::string type, std::string semanticMeaning) : headOfSibling(nullptr), leftSibling(nullptr), rightSibling(nullptr), parent(nullptr), children(), nodeType(type), semanticMeaning(semanticMeaning) {}
	node(std::string type, std::string semanticMeaning, std::string lastTokenValue) : headOfSibling(nullptr), leftSibling(nullptr), rightSibling(nullptr), parent(nullptr), children(), nodeType(type), semanticMeaning(semanticMeaning), nodeValue(lastTokenValue) {}
	node(node *n, std::string type) : headOfSibling(n->headOfSibling), leftSibling(n->leftSibling), rightSibling(n->rightSibling), parent(n->parent), children(n->children), nodeType(type), semanticMeaning(n->semanticMeaning) {}
	node(node *n, std::string type, std::string value) : headOfSibling(n->headOfSibling), leftSibling(n->leftSibling), rightSibling(n->rightSibling), parent(n->parent), children(n->children), nodeType(type), semanticMeaning(value) {}
	node(std::vector<node *> v, std::string type) : headOfSibling(nullptr), leftSibling(nullptr), rightSibling(nullptr), parent(nullptr), children(v), nodeType(type), semanticMeaning(type) {}
	node(std::vector<node *> v, std::string type, std::string value) : headOfSibling(nullptr), leftSibling(nullptr), rightSibling(nullptr), parent(nullptr), children(v), nodeType(type), semanticMeaning(value) {}

	virtual void accept(visitor &v) = 0;
	virtual ~node() {}
};

class paramNode : public node
{
public:
	void accept(visitor &v) override;
};

class startNode : public node
{
public:
	void accept(visitor &v) override;
};

class intNode : public node
{
public:
	void accept(visitor &v) override;
};

class floatNode : public node
{
public:
	void accept(visitor &v) override;
};

class epsilonNode : public node
{
public:
	void accept(visitor &v) override;
};

class aparamsNode : public node
{
public:
	void accept(visitor &v) override;
};

class aparamstailNode : public node
{
public:
	void accept(visitor &v) override;
};

class addopNode : public node
{
public:
	void accept(visitor &v) override;
};

class arithexprNode : public node
{
public:
	void accept(visitor &v) override;
};

class arraysizeNode : public node
{
public:
	void accept(visitor &v) override;
};

class assignNode : public node
{
public:
	void accept(visitor &v) override;
};

class attributedeclNode : public node
{
public:
	void accept(visitor &v) override;
};

class idNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptclassdecl4Node : public node
{
public:
	void accept(visitor &v) override;
};

class classdeclNode : public node
{
public:
	void accept(visitor &v) override;
};

class classNode : public node
{
public:
	void accept(visitor &v) override;
};

class implNode : public node
{
public:
	void accept(visitor &v) override;
};

class funcNode : public node
{
public:
	void accept(visitor &v) override;
};

class exprNode : public node
{
public:
	void accept(visitor &v) override;
};

class expr2Node : public node
{
public:
	void accept(visitor &v) override;
};

class relopNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptfparams3Node : public node
{
public:
	void accept(visitor &v) override;
};

class reptfparams4Node : public node
{
public:
	void accept(visitor &v) override;
};

class fparamsNode : public node
{
public:
	void accept(visitor &v) override;
};

class factorNode : public node
{
public:
	void accept(visitor &v) override;
};

class factor2Node : public node
{
public:
	void accept(visitor &v) override;
};

class selfNode : public node
{
public:
	void accept(visitor &v) override;
};

class selfandidnest2Node : public node
{
public:
	void accept(visitor &v) override;
};

class dotNode : public node
{
public:
	void accept(visitor &v) override;
};

class idnest2Node : public node
{
public:
	void accept(visitor &v) override;
};

class reptvariable2Node : public node
{
public:
	void accept(visitor &v) override;
};

class aparamsandidNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptvariable2andidNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptfuncbody1Node : public node
{
public:
	void accept(visitor &v) override;
};

class funcbodyNode : public node
{
public:
	void accept(visitor &v) override;
};

class funcdeclNode : public node
{
public:
	void accept(visitor &v) override;
};

class funcdefNode : public node
{
public:
	void accept(visitor &v) override;
};

class funcheadNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptimpldef3Node : public node
{
public:
	void accept(visitor &v) override;
};

class impldefNode : public node
{
public:
	void accept(visitor &v) override;
};

class indiceNode : public node
{
public:
	void accept(visitor &v) override;
};

class localvardeclNode : public node
{
public:
	void accept(visitor &v) override;
};

class localvardeclorstatNode : public node
{
public:
	void accept(visitor &v) override;
};

class multopNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptoptclassdecl22Node : public node
{
public:
	void accept(visitor &v) override;
};

class optclassdecl2Node : public node
{
public:
	void accept(visitor &v) override;
};

class reptprog0Node : public node
{
public:
	void accept(visitor &v) override;
};

class progNode : public node
{
public:
	void accept(visitor &v) override;
};

class relexprNode : public node
{
public:
	void accept(visitor &v) override;
};

class funcdeclfamNode : public node
{
public:
	void accept(visitor &v) override;
};

class attributedeclfamNode : public node
{
public:
	void accept(visitor &v) override;
};

class voidNode : public node
{
public:
	void accept(visitor &v) override;
};

class returntypeNode : public node
{
public:
	void accept(visitor &v) override;
};

class rightrectermNode : public node
{
public:
	void accept(visitor &v) override;
};

class signNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptstatblock1Node : public node
{
public:
	void accept(visitor &v) override;
};

class statblockNode : public node
{
public:
	void accept(visitor &v) override;
};

class ifstatementNode : public node
{
public:
	void accept(visitor &v) override;
};

class ifNode : public node
{
public:
	void accept(visitor &v) override;
};

class conditionNode : public node
{
public:
	void accept(visitor &v) override;
};

class thenNode : public node
{
public:
	void accept(visitor &v) override;
};

class felseNode : public node
{
public:
	void accept(visitor &v) override;
};

class whilestatementNode : public node
{
public:
	void accept(visitor &v) override;
};

class whileNode : public node
{
public:
	void accept(visitor &v) override;
};

class readNode : public node
{
public:
	void accept(visitor &v) override;
};

class readstatementNode : public node
{
public:
	void accept(visitor &v) override;
};

class writeNode : public node
{
public:
	void accept(visitor &v) override;
};

class writestatementNode : public node
{
public:
	void accept(visitor &v) override;
};

class returnNode : public node
{
public:
	void accept(visitor &v) override;
};

class floatnumNode : public node
{
public:
	void accept(visitor &v) override;
};

class freturnstatementNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptstatement4Node : public node
{
public:
	void accept(visitor &v) override;
};

class termNode : public node
{
public:
	void accept(visitor &v) override;
};

class typeNode : public node
{
public:
	void accept(visitor &v) override;
};

class vardeclNode : public node
{
public:
	void accept(visitor &v) override;
};

class visibilityNode : public node
{
public:
	void accept(visitor &v) override;
};

class reptvardecl3Node : public node
{
public:
	void accept(visitor &v) override;
};

class rightrecarithexprNode : public node
{
public:
	void accept(visitor &v) override;
};