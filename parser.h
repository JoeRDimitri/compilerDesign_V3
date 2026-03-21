#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <unordered_set>
#include <stack>
#include "lexor.h"
#include "first_and_follow.h"
#include <deque>
#include <typeinfo>
#include <spdlog/spdlog.h>
class parser{

class abstractSyntaxTree{


public:

	class symbolTable;
	class visitor;
	class node{
	public:
		struct symbolTableEntry{
			bool hasLink = false;
			std::string name;
			std::string kind;
			std::string type;
			std::map<std::string, symbolTableEntry*> * link;
		};


		node * headOfSibling;
		node * leftSibling;
		node * rightSibling;
		node * parent;
		std::vector<node *> children;
		std::string nodeType;
		std::string semanticMeaning;
		std::string nodeValue;
		bool isLeaf = false;
		symbolTableEntry stEntry;
		std::map<std::string, symbolTableEntry*> stMap;


		void copyNode(node* oldnode,node* newnode, std::string nameOfNewNode);

		node(){};
		node(std::string type,std::string semanticMeaning) : headOfSibling(nullptr),leftSibling(nullptr),rightSibling(nullptr),parent(nullptr),children(),nodeType(type),semanticMeaning(semanticMeaning){}
		node(std::string type,std::string semanticMeaning,std::string lastTokenValue) : headOfSibling(nullptr),leftSibling(nullptr),rightSibling(nullptr),parent(nullptr),children(),nodeType(type),semanticMeaning(semanticMeaning),nodeValue(lastTokenValue){}
		node(node * n, std::string type) : headOfSibling(n->headOfSibling),leftSibling(n->leftSibling),rightSibling(n->rightSibling),parent(n->parent),children(n->children),nodeType(type),semanticMeaning(n->semanticMeaning){}
		node(node * n, std::string type,std::string value) : headOfSibling(n->headOfSibling),leftSibling(n->leftSibling),rightSibling(n->rightSibling),parent(n->parent),children(n->children),nodeType(type),semanticMeaning(value){}
		node(std::vector<node*> v,std::string type) : headOfSibling(nullptr),leftSibling(nullptr),rightSibling(nullptr),parent(nullptr),children(v),nodeType(type),semanticMeaning(type){}
		node(std::vector<node*> v,std::string type,std::string value) : headOfSibling(nullptr),leftSibling(nullptr),rightSibling(nullptr),parent(nullptr),children(v),nodeType(type),semanticMeaning(value){}

	    virtual void accept(visitor& visitor)=0; // Pure virtual function
	    virtual ~node() {} // Virtual destructor
	};
	//Composite Element
	class paramNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};
	//Composite Element
	class startNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class intNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class floatNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class epsilonNode : public node {
	public:
	    void accept(visitor& v) override {
	    	}
	};
	//Composite Element
	class aparamsNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class aparamstailNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};
	class addopNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class arithexprNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit((*this));}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class arraysizeNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class assignNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class attributedeclNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class idNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class reptclassdecl4Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class classdeclNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class classNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){(*child).accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class implNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class funcNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class exprNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){
	    		child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class expr2Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class relopNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class reptfparams3Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class reptfparams4Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class fparamsNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class factorNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class factor2Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class selfNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class selfandidnest2Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class dotNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class idnest2Node : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class reptvariable2Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class aparamsandidNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class reptvariable2andidNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class reptfuncbody1Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class funcbodyNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class funcdeclNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class funcdefNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class funcheadNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class reptimpldef3Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class impldefNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class indiceNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class localvardeclNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class localvardeclorstatNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class multopNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class reptoptclassdecl22Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class optclassdecl2Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class reptprog0Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class progNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class relexprNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class funcdeclfamNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class attributedeclfamNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class voidNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class returntypeNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class rightrectermNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class signNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class reptstatblock1Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class statblockNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class ifstatementNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class ifNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class conditionNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class thenNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class felseNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class whilestatementNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class whileNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class readNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class readstatementNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class writeNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class writestatementNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class returnNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class floatnumNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class freturnstatementNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class reptstatement4Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class termNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){v.visit(*this);}
	    	else{for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	    	}
	};

	class typeNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class vardeclNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class visibilityNode : public node {
	public:
	    void accept(visitor& v) override { v.visit(*this); }
	};

	class reptvardecl3Node : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class rightrecarithexprNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(parser::abstractSyntaxTree::node * child : this->children){
	    		child->accept(v);}
	    	v.visit(*this);
	    	}
	};

	class visitor{
	public:
	    virtual void visit(abstractSyntaxTree::paramNode& n) {};
	    virtual void visit(abstractSyntaxTree::startNode& n) = 0;
	    virtual void visit(abstractSyntaxTree::intNode& n) {};
	    virtual void visit(abstractSyntaxTree::floatNode& n) {};
	    virtual void visit(abstractSyntaxTree::epsilonNode& n) {};
	    virtual void visit(abstractSyntaxTree::aparamsNode& n) {};
	    virtual void visit(abstractSyntaxTree::aparamstailNode& n) {};
	    virtual void visit(abstractSyntaxTree::addopNode& n) {};
	    virtual void visit(abstractSyntaxTree::arithexprNode& n) {};
	    virtual void visit(abstractSyntaxTree::arraysizeNode& n) {};
	    virtual void visit(abstractSyntaxTree::assignNode& n) {};
	    virtual void visit(abstractSyntaxTree::attributedeclNode& n) {};
	    virtual void visit(abstractSyntaxTree::idNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptclassdecl4Node& n) {};
	    virtual void visit(abstractSyntaxTree::classdeclNode& n) {};
	    virtual void visit(abstractSyntaxTree::classNode& n) {};
	    virtual void visit(abstractSyntaxTree::implNode& n) {};
	    virtual void visit(abstractSyntaxTree::funcNode& n) {};
	    virtual void visit(abstractSyntaxTree::exprNode& n) {};
	    virtual void visit(abstractSyntaxTree::expr2Node& n) {};
	    virtual void visit(abstractSyntaxTree::relopNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptfparams3Node& n) {};
	    virtual void visit(abstractSyntaxTree::reptfparams4Node& n) {};
	    virtual void visit(abstractSyntaxTree::fparamsNode& n) {};
	    virtual void visit(abstractSyntaxTree::factorNode& n) {};
	    virtual void visit(abstractSyntaxTree::factor2Node& n) {};
	    virtual void visit(abstractSyntaxTree::selfNode& n) {};
	    virtual void visit(abstractSyntaxTree::selfandidnest2Node& n) {};
	    virtual void visit(abstractSyntaxTree::dotNode& n) {};
	    virtual void visit(abstractSyntaxTree::idnest2Node& n) {};
	    virtual void visit(abstractSyntaxTree::reptvariable2Node& n) {};
	    virtual void visit(abstractSyntaxTree::aparamsandidNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptvariable2andidNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptfuncbody1Node& n) {};
	    virtual void visit(abstractSyntaxTree::funcbodyNode& n) {};
	    virtual void visit(abstractSyntaxTree::funcdeclNode& n) {};
	    virtual void visit(abstractSyntaxTree::funcdefNode& n) {};
	    virtual void visit(abstractSyntaxTree::funcheadNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptimpldef3Node& n) {};
	    virtual void visit(abstractSyntaxTree::impldefNode& n) {};
	    virtual void visit(abstractSyntaxTree::indiceNode& n) {};
	    virtual void visit(abstractSyntaxTree::localvardeclNode& n) {};
	    virtual void visit(abstractSyntaxTree::localvardeclorstatNode& n) {};
	    virtual void visit(abstractSyntaxTree::multopNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptoptclassdecl22Node& n) {};
	    virtual void visit(abstractSyntaxTree::optclassdecl2Node& n) {};
	    virtual void visit(abstractSyntaxTree::reptprog0Node& n) {};
	    virtual void visit(abstractSyntaxTree::progNode& n) {};
	    virtual void visit(abstractSyntaxTree::relexprNode& n) {};
	    virtual void visit(abstractSyntaxTree::funcdeclfamNode& n) {};
	    virtual void visit(abstractSyntaxTree::attributedeclfamNode& n) {};
	    virtual void visit(abstractSyntaxTree::voidNode& n) {};
	    virtual void visit(abstractSyntaxTree::returntypeNode& n) {};
	    virtual void visit(abstractSyntaxTree::rightrectermNode& n) {};
	    virtual void visit(abstractSyntaxTree::signNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptstatblock1Node& n) {};
	    virtual void visit(abstractSyntaxTree::statblockNode& n) {};
	    virtual void visit(abstractSyntaxTree::ifstatementNode& n) {};
	    virtual void visit(abstractSyntaxTree::ifNode& n) {};
	    virtual void visit(abstractSyntaxTree::conditionNode& n) {};
	    virtual void visit(abstractSyntaxTree::thenNode& n) {};
	    virtual void visit(abstractSyntaxTree::felseNode& n) {};
	    virtual void visit(abstractSyntaxTree::whilestatementNode& n) {};
	    virtual void visit(abstractSyntaxTree::whileNode& n) {};
	    virtual void visit(abstractSyntaxTree::readNode& n) {};
	    virtual void visit(abstractSyntaxTree::readstatementNode& n) {};
	    virtual void visit(abstractSyntaxTree::writeNode& n) {};
	    virtual void visit(abstractSyntaxTree::writestatementNode& n) {};
	    virtual void visit(abstractSyntaxTree::returnNode& n) {};
	    virtual void visit(abstractSyntaxTree::freturnstatementNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptstatement4Node& n) {};
	    virtual void visit(abstractSyntaxTree::termNode& n) {};
	    virtual void visit(abstractSyntaxTree::typeNode& n) {};
	    virtual void visit(abstractSyntaxTree::vardeclNode& n) {};
	    virtual void visit(abstractSyntaxTree::visibilityNode& n) {};
	    virtual void visit(abstractSyntaxTree::reptvardecl3Node& n) {};
	    virtual void visit(abstractSyntaxTree::rightrecarithexprNode& n) {};
	    virtual void visit(abstractSyntaxTree::floatnumNode& n) {};
		    virtual ~visitor() {} // Virtual destructor for proper cleanup

	};

	class SymTabCreationVisitor : public visitor{
	public:
		enum type{
			CLASSDECL,
			CLASSIMPL,
			FUNCDECL,
			FUNCIMPL
		};
		std::unordered_map<std::string, type  > semanticTracking;
	    virtual ~SymTabCreationVisitor() {}
		using visitor::visit;
		void visit(node * head){};
		void visit(funcNode& head);
		void visit(localvardeclNode& head);
		void visit(funcheadNode& head);
		void visit(funcdefNode& head);
		void visit(startNode& head);
		void visit(implNode& head);
		void visit(funcdeclNode & head);
		void visit(fparamsNode & head);
		void visit(paramNode & head);
		void visit(classNode & head);
		void visit(attributedeclNode & head);
		void visit(attributedeclfamNode & head);
		void visit(funcbodyNode & head);
		std::string get(std::string, node& head);


	};

	class SemanticCheckingVisitor : public visitor{
	    virtual ~SemanticCheckingVisitor() {}

		void visit(startNode& head);

	};



	startNode * treeHead;
	void printSymbolTable(node* head);
	void printSymbolTableRec(std::map<std::string,parser::abstractSyntaxTree::node::symbolTableEntry*> * link,int & count,std::ofstream & ao);

	void printTree();
	void traverseTree(abstractSyntaxTree::node* head, int& counter,std::ofstream & ao);

};
//struct symbolTableEntry{
//	std::string name;
//	std::string kind;
//	std::string type;
//	symbolTable * link;
//};

class semanticActions{
public:
	void makeLeaf(std::string nodeType,std::string lastTokenValue,abstractSyntaxTree::node* newnode);
	void handleAction(std::string semanticAction,std::string lastTokenValue);
	void makeSubTree(std::string nodeType,std::string semanticMeaning, int amountOfPops,std::vector<std::string> v,abstractSyntaxTree::node * newnode);
	void passAlong(std::string nodeType,std::string semanticMeaning,abstractSyntaxTree::node * newnode);
	void passAlong(std::string nodeType,abstractSyntaxTree::node * newnode);
	void adoptChildren(abstractSyntaxTree:: node* newparent,abstractSyntaxTree:: node* oldparent);
	void makeBinarySubTree(std::string nodeType,int i,abstractSyntaxTree::node*newnode);
	void makeBinarySubTreeWithHead(std::string nodeType, int numOfPops,abstractSyntaxTree::node* newnode);



};

struct tableEntry{
		int row;
		int column;
		std::string derivationRule;
	};



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

	parsing_table(){};
	~parsing_table(){
		for (int i = 0; i < TABLEROWSIZE; ++i) {
			delete[] table[i];  // Free each row
		}
		delete[] table;  // Free row pointers};
	};

	std::unordered_set <std::string> getUniqueTerminalSymbols(){return this -> uniqueTerminalSymbols;}
};

class symbolTable{

// When is there a new symbol table?
	/*
	 * - One is made at the beginning for the global scope.
	 * - One is maded for Each class gets a symbol table
	 * - Each class gets an entyr
	 * - Each variable gets an entry
	 * - each function gets an entry. This entry is a link to another symbol table. The symbol table of the function
	 * Report errors like if a variable with the same name has already been declared. That means whenever we want to add a new entry we need to check if it exists already.
	 *
	 *
	 */

};


//Need to make all of these variables non-static and instantiate them in the constructor. 
public:
	static first_and_follow faf;
	static parsing_table parsingTable;
	static std::stack<std::string> parsingStack;
	static semanticActions semanticHandler;
	static std::stack<abstractSyntaxTree::node *> semanticStack;
	static abstractSyntaxTree AST;

	parser::abstractSyntaxTree::SymTabCreationVisitor *firstVisitor = new parser::abstractSyntaxTree::SymTabCreationVisitor();
	parser::abstractSyntaxTree::visitor & first = *firstVisitor;
	bool searchFirst(std::string lookahead, std::string topOfTheStack);
	bool parse(const  std::vector<token*> &);
	void inverseRHSMultiplePush(tableEntry t,std::vector<std::string>& vec,const int & lineIndex);
	void skipError(token * & currentToken,std::vector<token*>::const_iterator& vectorIterator,std::ofstream &,const std::string & lexeme,const std::string & topOfTheStack,const int & line, const int & column);
	bool search(const std::string & lexeme, const std::string & topOfTheStack);

		parser(){spdlog::info("Parser constructor called.");};
		~parser(){spdlog::info("Parser destructor called.");}

};

