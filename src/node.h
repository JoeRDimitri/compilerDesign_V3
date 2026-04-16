#pragma once

#include <string>
#include <vector>
#include <map>
#include "visitor.h"


class node{
	public:

		struct symbolTableEntry{
			bool hasLink = false;
			std::string name;
			std::string kind;
			std::string type;
			std::string visibility;
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
	class paramNode : public node {
	public:
	    void accept(visitor& v) override {
	    	for(node * child : this->children){
				child->accept(v);
			}
	    	v.visit(*this);
	    }
	};
	//Composite Element
	class startNode : public node {
	public:
	    void accept(visitor& v) override {
	    	if(this->isLeaf ==true){
				v.visit(*this);
			}
	    	else{
				for(node * child : this->children){
					child->accept(v);
				}
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
	    	for(node * child : this->children){
	    		child->accept(v);}
	    	v.visit(*this);
	    	}
	};