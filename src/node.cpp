#include "node.h"
#include "visitor.h"

// Shallow-copies all structural and semantic fields from oldnode into newnode,
// overriding the node type with nameOfNewNode. Used when promoting a node to a new type.
void node::copyNode(node *oldnode, node *newnode, std::string nameOfNewNode)
{
	newnode->headOfSibling = oldnode->headOfSibling;
	newnode->leftSibling = oldnode->leftSibling;
	newnode->rightSibling = oldnode->rightSibling;
	newnode->parent = oldnode->parent;
	newnode->children = oldnode->children;
	newnode->nodeType = nameOfNewNode;
	newnode->semanticMeaning = oldnode->semanticMeaning;
	newnode->nodeValue = oldnode->nodeValue;
}

// ── accept() implementations ──────────────────────────────────
// These must live in a .cpp that includes visitor.h so that
// the compiler knows visitor's full definition (all visit() methods).

void paramNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void startNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void intNode::accept(visitor &v) { v.visit(*this); }
void floatNode::accept(visitor &v) { v.visit(*this); }
void epsilonNode::accept(visitor &v) {}

void aparamsNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void aparamstailNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void addopNode::accept(visitor &v) { v.visit(*this); }

void arithexprNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void arraysizeNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void assignNode::accept(visitor &v) { v.visit(*this); }

void attributedeclNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void idNode::accept(visitor &v) { v.visit(*this); }

void reptclassdecl4Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void classdeclNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void classNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			(*child).accept(v);
		}
		v.visit(*this);
	}
}

void implNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void funcNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void exprNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void expr2Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void relopNode::accept(visitor &v) { v.visit(*this); }

void reptfparams3Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void reptfparams4Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void fparamsNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void factorNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void factor2Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void selfNode::accept(visitor &v) { v.visit(*this); }

void selfandidnest2Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void dotNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void idnest2Node::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void reptvariable2Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void aparamsandidNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void reptvariable2andidNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void reptfuncbody1Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void funcbodyNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void funcdeclNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void funcdefNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void funcheadNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void reptimpldef3Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void impldefNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void indiceNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void localvardeclNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void localvardeclorstatNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void multopNode::accept(visitor &v) { v.visit(*this); }

void reptoptclassdecl22Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void optclassdecl2Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void reptprog0Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void progNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void relexprNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void funcdeclfamNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void attributedeclfamNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void voidNode::accept(visitor &v) { v.visit(*this); }

void returntypeNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void rightrectermNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void signNode::accept(visitor &v) { v.visit(*this); }

void reptstatblock1Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void statblockNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void ifstatementNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void ifNode::accept(visitor &v) { v.visit(*this); }

void conditionNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void thenNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void felseNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void whilestatementNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void whileNode::accept(visitor &v) { v.visit(*this); }
void readNode::accept(visitor &v) { v.visit(*this); }

void readstatementNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void writeNode::accept(visitor &v) { v.visit(*this); }

void writestatementNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void returnNode::accept(visitor &v) { v.visit(*this); }
void floatnumNode::accept(visitor &v) { v.visit(*this); }

void freturnstatementNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void reptstatement4Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void termNode::accept(visitor &v)
{
	if (this->isLeaf == true)
	{
		v.visit(*this);
	}
	else
	{
		for (node *child : this->children)
		{
			child->accept(v);
		}
		v.visit(*this);
	}
}

void typeNode::accept(visitor &v) { v.visit(*this); }

void vardeclNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void visibilityNode::accept(visitor &v) { v.visit(*this); }

void reptvardecl3Node::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}

void rightrecarithexprNode::accept(visitor &v)
{
	for (node *child : this->children)
	{
		child->accept(v);
	}
	v.visit(*this);
}
