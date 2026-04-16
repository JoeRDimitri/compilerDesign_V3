#include "node.h"

// Shallow-copies all structural and semantic fields from oldnode into newnode,
// overriding the node type with nameOfNewNode. Used when promoting a node to a new type.
void node::copyNode(node* oldnode,node* newnode, std::string nameOfNewNode){
	newnode ->headOfSibling = oldnode ->headOfSibling;
	newnode->leftSibling = oldnode->leftSibling;
	newnode->rightSibling = oldnode->rightSibling;
	newnode->parent = oldnode->parent; // NOTE: likely a bug — should be oldnode->parent
	newnode->children = oldnode->children;
	newnode->nodeType = nameOfNewNode;          // override type with the new name
	newnode->semanticMeaning = oldnode ->semanticMeaning;
	newnode->nodeValue = oldnode->nodeValue;
}
