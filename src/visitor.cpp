#include "visitor.h"
#include <typeinfo>
#include <algorithm>

namespace
{
	// Small AST/symbol-table helpers used by the semantic and symbol-table passes below.
	std::string findTypeInEntryLink(node::symbolTableEntry *entry, const std::string &name, std::unordered_set<node::symbolTableEntry *> &visited)
	{
		// Walk a linked symbol-table chain recursively, guarding against cycles.
		if (!entry || !entry->hasLink || !entry->link || visited.count(entry) > 0)
			return "";
		visited.insert(entry);

		auto direct = entry->link->find(name);
		if (direct != entry->link->end() && direct->second)
		{
			return direct->second->type;
		}

		for (auto &[_, nested] : *entry->link)
		{
			std::string nestedType = findTypeInEntryLink(nested, name, visited);
			if (!nestedType.empty())
				return nestedType;
		}

		return "";
	}

	std::string findTypeFromRoot(node *root, const std::string &name)
	{
		// Resolve a symbol by first checking the root table and then any nested links.
		if (!root)
			return "";

		auto direct = root->stMap.find(name);
		if (direct != root->stMap.end() && direct->second)
		{
			return direct->second->type;
		}

		std::unordered_set<node::symbolTableEntry *> visited;
		for (auto &[_, entry] : root->stMap)
		{
			std::string nestedType = findTypeInEntryLink(entry, name, visited);
			if (!nestedType.empty())
				return nestedType;
		}

		return "";
	}

	std::string getClassDeclName(node *classDeclNode)
	{
		// classdecl nodes store the class identifier as a direct id child.
		if (!classDeclNode)
			return "";

		for (node *child : classDeclNode->children)
		{
			if (child && child->semanticMeaning == "id")
			{
				return child->nodeValue;
			}
		}

		return "";
	}

	node *findClassDeclarationNode(node *root, const std::string &className)
	{
		// Depth-first search for the class declaration node with the matching class name.
		if (!root)
			return nullptr;

		if (root->semanticMeaning == "classdecl" && getClassDeclName(root) == className)
		{
			return root;
		}

		for (node *child : root->children)
		{
			if (node *found = findClassDeclarationNode(child, className); found != nullptr)
			{
				return found;
			}
		}

		return nullptr;
	}

	bool classDeclaresFunction(node *classDeclNode, const std::string &funcName)
	{
		// Check whether a class declaration contains a matching function declaration anywhere inside it.
		if (!classDeclNode)
			return false;

		auto matchesFunctionNode = [&](node *candidate) -> bool
		{
			return candidate &&
				   (candidate->semanticMeaning == "funcdecl" || candidate->semanticMeaning == "funchead" || candidate->semanticMeaning == "funcdeclfam") &&
				   candidate->stEntry.name == funcName;
		};

		auto scanDescendants = [&](auto &&self, node *current) -> bool
		{
			if (!current)
				return false;

			if (matchesFunctionNode(current))
			{
				return true;
			}

			for (node *child : current->children)
			{
				if (self(self, child))
				{
					return true;
				}
			}

			return false;
		};

		for (node *child : classDeclNode->children)
		{
			if (!child)
				continue;

			if (child->semanticMeaning == "{")
			{
				for (node *grandchild : child->children)
				{
					if (scanDescendants(scanDescendants, grandchild))
					{
						return true;
					}
				}
			}
			else if (matchesFunctionNode(child))
			{
				return true;
			}
			else if (scanDescendants(scanDescendants, child))
			{
				return true;
			}
		}

		return false;
	}

	node *findDeclaredFunctionNode(node *classDeclNode, const std::string &funcName)
	{
		// Return the declaration node itself so the semantic pass can compare its parameters.
		if (!classDeclNode)
			return nullptr;

		const std::string className = classDeclNode->stEntry.name;

		auto matchesFunctionNode = [&](node *candidate) -> bool
		{
			return candidate &&
				   (candidate->semanticMeaning == "funcdecl" || candidate->semanticMeaning == "funchead" || candidate->semanticMeaning == "funcdeclfam") &&
				   (candidate->stEntry.name == funcName || (funcName == "constructor" && candidate->stEntry.name == className));
		};

		auto scanDescendants = [&](auto &&self, node *current) -> node *
		{
			if (!current)
				return nullptr;

			if (matchesFunctionNode(current))
			{
				return current;
			}

			for (node *child : current->children)
			{
				if (node *found = self(self, child); found != nullptr)
				{
					return found;
				}
			}

			return nullptr;
		};

		return scanDescendants(scanDescendants, classDeclNode);
	}

	std::map<std::string, std::string> collectParamTypes(node *funcheadNode)
	{
		// Collect parameter name/type pairs from a funchead subtree.
		std::map<std::string, std::string> params;
		if (!funcheadNode)
			return params;

		auto scan = [&](auto &&self, node *current) -> void
		{
			if (!current)
				return;

			if (current->semanticMeaning == "param")
			{
				std::string paramName;
				std::string paramType;
				for (node *child : current->children)
				{
					if (!child)
						continue;
					if (child->semanticMeaning == "id")
					{
						paramName = child->nodeValue;
					}
					else if (child->semanticMeaning == "type")
					{
						paramType = child->nodeValue;
					}
				}
				if (!paramName.empty())
				{
					params[paramName] = paramType;
				}
			}

			for (node *child : current->children)
			{
				self(self, child);
			}
		};

		scan(scan, funcheadNode);
		return params;
	}

	std::vector<std::string> collectOrderedParamTypes(node *functionNode)
	{
		// Collect parameter types in source order to enforce full signature matching.
		std::vector<std::string> paramTypes;
		if (!functionNode)
			return paramTypes;

		auto scan = [&](auto &&self, node *current) -> void
		{
			if (!current)
				return;

			if (current->semanticMeaning == "param")
			{
				for (node *child : current->children)
				{
					if (child && child->semanticMeaning == "type")
					{
						paramTypes.push_back(child->nodeValue);
						break;
					}
				}
			}

			for (node *child : current->children)
			{
				self(self, child);
			}
		};

		scan(scan, functionNode);
		return paramTypes;
	}

	std::string collectDeclaredReturnType(node *functionNode)
	{
		if (!functionNode)
			return "";

		auto scan = [&](auto &&self, node *current) -> std::string
		{
			if (!current)
				return "";

			if (current->semanticMeaning == "returntype")
			{
				if (!current->nodeValue.empty())
					return current->nodeValue;
				if (!current->stEntry.type.empty())
					return current->stEntry.type;
			}

			for (node *child : current->children)
			{
				std::string found = self(self, child);
				if (!found.empty())
					return found;
			}

			return "";
		};

		std::string declaredType = scan(scan, functionNode);
		if (!declaredType.empty())
			return declaredType;

		return functionNode->stEntry.type;
	}

	std::unordered_set<std::string> collectClassAttributes(node *classDeclNode)
	{
		// Gather every class member name declared inside the class body.
		std::unordered_set<std::string> attributes;
		if (!classDeclNode)
			return attributes;

		auto scan = [&](auto &&self, node *current) -> void
		{
			if (!current)
				return;

			if (current->semanticMeaning == "vardecl")
			{
				if (!current->stEntry.name.empty())
				{
					attributes.insert(current->stEntry.name);
				}
				else
				{
					for (node *child : current->children)
					{
						if (child && child->nodeType == "id")
						{
							attributes.insert(child->nodeValue);
							break;
						}
					}
				}
			}

			for (node *child : current->children)
			{
				self(self, child);
			}
		};

		scan(scan, classDeclNode);
		return attributes;
	}

	std::unordered_set<std::string> collectConstructorTargets(node *implBodyNode)
	{
		// Gather the left-hand targets written by constructor-style self.member := ... assignments.
		std::unordered_set<std::string> targets;
		if (!implBodyNode)
			return targets;

		auto scan = [&](auto &&self, node *current) -> void
		{
			if (!current)
				return;

			if (current->semanticMeaning == "dot" && !current->children.empty() && current->children[0])
			{
				node *assignNode = current->children[0];
				if ((assignNode->semanticMeaning == "assign" || assignNode->semanticMeaning == "reptstatement4") && !assignNode->children.empty() && assignNode->children[0])
				{
					node *lhsNode = assignNode->children[0];
					if (!lhsNode->stEntry.name.empty())
					{
						targets.insert(lhsNode->stEntry.name);
					}
					else
					{
						for (node *child : lhsNode->children)
						{
							if (child && child->nodeType == "id")
							{
								targets.insert(child->nodeValue);
								break;
							}
						}
					}
				}
			}

			for (node *child : current->children)
			{
				self(self, child);
			}
		};

		scan(scan, implBodyNode);
		return targets;
	}
}

std::string SymTabCreationVisitor::get(std::string search, node &head)
{
	for (node *child : head.children)
	{
		if (child->semanticMeaning == search)
		{
			return child->nodeValue;
		}
	}
	if (head.children.empty())
	{
		return "";
	}
	return get(search, (*head.children[0]));
}
void SymTabCreationVisitor::visit(funcdeclNode &head)
{
	// Build the function declaration entry and point it at the parameter map.
	node::symbolTableEntry *ste = &head.stEntry;

	// kind
	ste->kind = "function";
	// haslink
	ste->hasLink = true;
	// name
	ste->name = get("id", head);
	ste->type = get("returntype", head);

	for (node *children : head.children)
	{

		if (children->nodeType == "fparams")
		{
			// link
			ste->link = &children->stMap;
		}
	}
}

void SymTabCreationVisitor::visit(fparamsNode &head)
{
	// Store parameter declarations in the function's local symbol table.
	std::map<std::string, node::symbolTableEntry *> *mp = &head.stMap;
	for (node *child : head.children)
	{
		if (child->nodeType == "param")
		{
			if ((*mp).count(child->stEntry.name) > 0)
			{
				spdlog::error("Multiple Parameters with the same name for: " + child->stEntry.name + ".");
			}
			else
			{
				(*mp)[child->stEntry.name] = &child->stEntry;
			}
		}
	}
}

void SymTabCreationVisitor::visit(paramNode &head)
{
	// A param node only needs its identifier and declared type copied into the entry.
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "parameter";
	ste->name = get("id", head);
	ste->type = get("type", head);
}

void SymTabCreationVisitor::visit(classNode &head)
{
	// Build the class entry and collect attributes/function declarations from the class body.
	std::map<std::string, node::symbolTableEntry *> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "class";
	ste->name = get("id", head);
	ste->hasLink = true;
	ste->link = mp;
	for (node *child : head.children)
	{
		if (child->semanticMeaning == "isalist")
		{
			for (node *children : child->children)
			{
				ste->type += children->nodeValue;
			}
		}
		else if (child->semanticMeaning == "{")
		{
			// Walk the class body and collect member declarations into the class symbol table.
			for (node *children : child->children)
			{
				if (children->semanticMeaning == "attributedecl")
				{
					if ((*mp).count(children->stEntry.name) > 0)
					{
						spdlog::error("Multiple Attribute Declarations for: " + children->stEntry.name + ".");
					}
					else
					{
						(*mp)[children->stEntry.name] = &children->stEntry;
					}
				}
				else if (children->semanticMeaning == "funcdecl")
				{
					// funcdeclfamNode wraps visibility + funchead
					if (!children->stEntry.name.empty())
					{
						if ((*mp).count(children->stEntry.name) > 0)
						{
							spdlog::error("Multiple Function Declarations for: " + children->stEntry.name + ".");
						}
						else
						{
							(*mp)[children->stEntry.name] = &children->stEntry;
						}
					}
					else
					{
						// fallback: dig into grandchildren for funchead
						for (node *grandchildren : children->children)
						{
							if (grandchildren->semanticMeaning == "funchead")
							{
								if ((*mp).count(grandchildren->stEntry.name) > 0)
								{
									spdlog::error("Multiple Function Declarations for: " + grandchildren->stEntry.name + ".");
								}
								else
								{
									(*mp)[grandchildren->stEntry.name] = &grandchildren->stEntry;
								}
							}
						}
					}
				}
			}
		}
	}
}

void SymTabCreationVisitor::visit(startNode &head)
{
	// Top-level pass: collect classes and free functions, then merge implementations into classes.
	std::map<std::string, node::symbolTableEntry *> *mp = &head.stMap;
	std::unordered_set<std::string> implementedClasses;
	for (node *child : head.children)
	{
		if (child->semanticMeaning == "classdecl")
		{
			// link
			if ((*mp).count(child->stEntry.name) > 0)
			{
				spdlog::error("Multiple Class Declaration for: " + child->stEntry.name + ".");
			}
			else
			{
				(*mp)[child->stEntry.name] = &child->stEntry;
			}
		}
		else if (child->semanticMeaning == "funcdef")
		{
			(*mp)[child->stEntry.name] = &child->stEntry;
		}
	}
	for (node *child : head.children)
	{

		if (child->semanticMeaning == "impldef")
		{
			// gets the class name
			std::string className = child->stEntry.name;
			// Checks if the current implementation was
			if ((*mp).count(className) > 0 && (*mp)[className]->kind == "class")
			{
				// Found the class the function was declared in.
				if (implementedClasses.find(className) == implementedClasses.end())
				{
					spdlog::info("Class Declaration found for " + className + ". Updating implementation.");
				}
				implementedClasses.insert(className);
				auto *classEntry = (*mp)[className];

				// Merge function entries from this impldef into the class's stMap
				if (child->stEntry.hasLink && child->stEntry.link && classEntry->hasLink && classEntry->link)
				{
					// spdlog::debug("[SymTabCreation] Merging impl of '{}' into class declaration.", className);
					for (auto &[fname, fentry] : *child->stEntry.link)
					{
						// spdlog::debug("[SymTabCreation]   Checking impl function '{}' (return type='{}').", fname, fentry->type);

						// Check 1: function name exists in class declaration
						if (classEntry->link->count(fname) > 0)
						{
							node::symbolTableEntry *declaredFunc = (*classEntry->link)[fname];
							// spdlog::debug("[SymTabCreation]   [OK] '{}::{}' found in class declaration (declared return type='{}').", className, fname, declaredFunc->type);

							// Copy visibility from class declaration
							fentry->visibility = declaredFunc->visibility;
							// spdlog::debug("[SymTabCreation]   Copied visibility '{}' to impl entry.", fentry->visibility);

							// Check 2: return type matches
							if (!declaredFunc->type.empty() && declaredFunc->type != fentry->type)
							{
								spdlog::error("[SymTabCreation] Return type mismatch for '{}::{}': declared '{}', implemented '{}'.",
											  className, fname, declaredFunc->type, fentry->type);
							}
							else
							{
								// spdlog::debug("[SymTabCreation]   [OK] Return type '{}' matches for '{}::{}'.", fentry->type, className, fname);
							}

							// Check 3: parameters match
							if (declaredFunc->hasLink && declaredFunc->link && fentry->hasLink && fentry->link)
							{
								// Build declared param map
								std::map<std::string, std::string> declaredParams;
								for (auto &[pname, pentry] : *declaredFunc->link)
								{
									if (pentry->kind == "parameter")
									{
										declaredParams[pname] = pentry->type;
										// spdlog::debug("[SymTabCreation]     Declared param: '{}' type='{}'.", pname, pentry->type);
									}
								}
								// Build impl param map
								std::map<std::string, std::string> implParams;
								for (auto &[pname, pentry] : *fentry->link)
								{
									if (pentry->kind == "parameter")
									{
										implParams[pname] = pentry->type;
										// spdlog::debug("[SymTabCreation]     Impl param: '{}' type='{}'.", pname, pentry->type);
									}
								}
								// Check each declared param exists in impl with matching type
								for (auto &[pname, ptype] : declaredParams)
								{
									if (implParams.count(pname) == 0)
										spdlog::error("[SymTabCreation] Parameter '{}' declared in '{}::{}' but missing in implementation.", pname, className, fname);
									else if (implParams[pname] != ptype)
										spdlog::error("[SymTabCreation] Parameter '{}' type mismatch in '{}::{}': declared '{}', implemented '{}'.", pname, className, fname, ptype, implParams[pname]);
									// else
									// spdlog::debug("[SymTabCreation]     [OK] Param '{}' type '{}' matches.", pname, ptype);
								}
								// Check for extra params in impl not in declaration
								for (auto &[pname, ptype] : implParams)
								{
									if (declaredParams.count(pname) == 0)
										spdlog::error("[SymTabCreation] Parameter '{}' in implementation of '{}::{}' not declared.", pname, className, fname);
								}
							}
							else
							{
								// spdlog::debug("[SymTabCreation]   No parameter links to compare for '{}::{}'.", className, fname);
							}
						}
						else
						{
							spdlog::error("[SymTabCreation] Function '{}' implemented in class '{}' but never declared.", fname, className);
						}
						(*classEntry->link)[fname] = fentry;
						// spdlog::debug("[SymTabCreation]   Merged '{}::{}' into class symbol table.", className, fname);
					}
				}
				else
				{
					// spdlog::debug("[SymTabCreation] No link maps available to merge for impl of '{}'.", className);
				}
			}
			else
			{
				spdlog::error("Missing Class Declaration for: " + child->stEntry.name + ".");
			}
		}
	}
	for (const auto &[key, value] : (*mp))
	{
		if (value->kind == "class" && implementedClasses.find(key) == implementedClasses.end())
		{
			spdlog::error("There is a class declaration: " + value->name + " without implementation");
		}
	}
}

void SymTabCreationVisitor::visit(attributedeclNode &head)
{
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "variable";
	ste->name = get("id", head);
	ste->type = get("type", head);
}

void SymTabCreationVisitor::visit(attributedeclfamNode &head)
{
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "attribute";
	for (node *child : head.children)
	{
		if (child->semanticMeaning == "vardecl")
		{
			ste->name = child->stEntry.name;
			ste->type = child->stEntry.type;
		}
		else if (child->semanticMeaning == "visibility")
		{
			ste->visibility = child->nodeValue;
		}
	}
}
void SymTabCreationVisitor::visit(funcdeclfamNode &head)
{
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "function head";
	for (node *child : head.children)
	{
		if (child->nodeType == "funcdecl")
		{
			// funcdecl inherits funchead's children via passAlong
			// Mirror funcheadNode logic for name/type/link
			for (node *gc : child->children)
			{
				if (gc->semanticMeaning == "id" && ste->name.empty())
				{
					ste->name = gc->nodeValue;
				}
				else if (gc->semanticMeaning == "returntype")
				{
					ste->type = gc->nodeValue;
				}
				else if (gc->semanticMeaning == "funcparams")
				{
					ste->link = &gc->stMap;
					ste->hasLink = true;
				}
			}
			if (ste->name.empty())
				ste->name = "constructor";
		}
		else if (child->semanticMeaning == "visibility")
		{
			ste->visibility = child->nodeValue;
		}
	}
}
void SymTabCreationVisitor::visit(implNode &head)
{
	std::map<std::string, node::symbolTableEntry *> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "implementation";
	ste->hasLink = true;
	ste->link = mp;

	std::string className;
	std::string funcName;
	node *funcheadChild = nullptr;

	for (node *child : head.children)
	{
		if (child->semanticMeaning == "funchead")
		{
			funcheadChild = child;
			bool foundFirst = false;
			for (node *fc : child->children)
			{
				if (fc->semanticMeaning == "id")
				{
					if (!foundFirst)
					{
						className = fc->nodeValue;
						foundFirst = true;
					}
					else
					{
						funcName = fc->nodeValue;
					}
				}
			}
			if (funcName.empty())
				funcName = "constructor";
		}
	}

	ste->name = className;

	// Build a function entry from the funchead
	if (funcheadChild)
	{
		node::symbolTableEntry *funcEntry = &funcheadChild->stEntry;
		funcEntry->kind = "function";
		funcEntry->name = funcName;
		funcEntry->hasLink = true;
		// If funchead didn't have a link, use its own stMap
		if (!funcEntry->link)
		{
			funcEntry->link = &funcheadChild->stMap;
		}
		// Merge local vardecls from reptimpldef3 into the function's param/local map
		for (node *child : head.children)
		{
			if (child->semanticMeaning == "reptimpldef3")
			{
				for (node *bodyChild : child->children)
				{
					if (bodyChild->semanticMeaning == "vardecl")
					{
						if (funcEntry->link->count(bodyChild->stEntry.name) == 0)
						{
							(*funcEntry->link)[bodyChild->stEntry.name] = &bodyChild->stEntry;
						}
					}
				}
			}
		}
		// Store in implNode's stMap keyed by function name
		(*mp)[funcName] = funcEntry;
	}
}

void SymTabCreationVisitor::visit(funcdefNode &head)
{
	std::map<std::string, node::symbolTableEntry *> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "function";
	ste->hasLink = true;
	ste->link = mp;

	for (node *child : head.children)
	{
		if (child->semanticMeaning == "funchead")
		{
			mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			ste->name = child->stEntry.name;
			ste->type = child->stEntry.type;
		}
		else if (child->semanticMeaning == "funcbody")
		{
			mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			child->stEntry.name = ste->name;
		}
	}
}
void SymTabCreationVisitor::visit(funcheadNode &head)
{
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "function head";
	// Get function name from direct id children only (don't recurse into params)
	ste->name = "";
	for (node *child : head.children)
	{
		if (child->semanticMeaning == "id")
		{
			ste->name = child->nodeValue;
			break;
		}
	}
	if (ste->name.empty())
		ste->name = "constructor";
	for (node *child : head.children)
	{
		if (child->semanticMeaning == "returntype")
		{
			ste->type = child->nodeValue;
		}
		else if (child->semanticMeaning == "funcparams")
		{
			// link
			ste->link = &child->stMap;
		}
	}
}
void SymTabCreationVisitor::visit(funcbodyNode &head)
{
	std::map<std::string, node::symbolTableEntry *> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->hasLink = true;
	ste->link = mp;
	ste->kind = "function body";
	for (node *child : head.children)
	{
		if (child->semanticMeaning == "vardecl")
		{
			if ((*mp).count(child->stEntry.name) > 0)
			{
				spdlog::error("Multiple Variable Declarations for: " + child->stEntry.name + ".");
			}
			else
			{
				(*mp)[child->stEntry.name] = &child->stEntry;
			}
		}
	}
}

void SymTabCreationVisitor::visit(localvardeclNode &head)
{
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "variable";
	ste->name = get("id", head);
	ste->type = get("type", head);
}

void SymTabCreationVisitor::visit(funcNode &head)
{
	std::map<std::string, node::symbolTableEntry *> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "function";
	ste->hasLink = true;
	ste->link = mp;

	for (node *child : head.children)
	{
		if (child->semanticMeaning == "funchead")
		{
			if (!mp->empty())
				mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			ste->name = child->stEntry.name;
			ste->type = child->stEntry.type;
		}
		else if (child->semanticMeaning == "funcbody")
		{
			mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			child->stEntry.name = ste->name;
		}
	}
}
// Not actually reaching this node as it is not used.
void SemanticCheckingVisitor::visit(impldefNode &head)
{
	// impldef is currently validated via implNode and symbol table merge checks.
}

void SemanticCheckingVisitor::visit(implNode &head)
{
	int childrenCount = head.children.size();

	if (childrenCount == 2)
	{
		std::vector<std::string> children_types = {"funchead", "reptimpldef3"};
		// Check that left and right type of children
		if (checkChildren(2, childrenCount, children_types, head))
		{
			node *funcHeadNode = head.children[0];
			node *funcBodyNode = head.children[1];
			std::string className = funcHeadNode->children.size() > 0 ? funcHeadNode->children[0]->nodeValue : "<unknown-class>";
			std::string funcName = "<unknown-function>";
			if (funcHeadNode->children.size() >= 2 && funcHeadNode->children[1]->nodeType == "id")
			{
				funcName = funcHeadNode->children[1]->nodeValue;
			}
			else if (funcHeadNode->children.size() >= 1)
			{
				funcName = "constructor";
			}

			std::string function_return_type = funcHeadNode->stEntry.type;
			std::string actual_return_type = funcBodyNode->stEntry.type;

			node *classDeclNode = findClassDeclarationNode(root, className);
			node *declFunctionNode = classDeclNode ? findDeclaredFunctionNode(classDeclNode, funcName) : nullptr;
			if (!classDeclNode)
			{
				canGenerateMachineCode = false;
				spdlog::error("[SemanticCheck][FAIL][Declaration] '{}::{}' has no matching class declaration.", className, funcName);
			}
			else if (declFunctionNode != nullptr)
			{
				spdlog::info("[SemanticCheck][PASS][Declaration] '{}::{}' is declared in class '{}'.", className, funcName, className);
			}
			else
			{
				canGenerateMachineCode = false;
				spdlog::error("[SemanticCheck][FAIL][Declaration] '{}::{}' is implemented but not declared in class '{}'.", className, funcName, className);
			}

			auto hasHeadShape = [](node *fh, const std::vector<std::string> &shape) -> bool
			{
				if (!fh || fh->children.size() != shape.size())
					return false;
				for (size_t i = 0; i < shape.size(); ++i)
				{
					if (!fh->children[i] || fh->children[i]->nodeType != shape[i])
						return false;
				}
				return true;
			};

			bool isMethodHead = hasHeadShape(funcHeadNode, {"id", "id", "fparams", "returntype"});
			bool isCtorHead = !isMethodHead && hasHeadShape(funcHeadNode, {"id", "fparams", "returntype"});
			bool headerShapeOk = isMethodHead || isCtorHead;

			if (function_return_type == actual_return_type)
			{
				if (headerShapeOk)
					spdlog::info("[SemanticCheck][PASS][ReturnType] '{}::{}' declared='{}' actual='{}'.", className, funcName, function_return_type, actual_return_type);
				else
					spdlog::info("[SemanticCheck][PASS][ReturnType] '{}::{}' declared='{}' actual='{}' (non-canonical funchead shape).", className, funcName, function_return_type, actual_return_type);
			}
			else
			{
				canGenerateMachineCode = false;
				if (headerShapeOk)
					spdlog::warn("[SemanticCheck][FAIL][ReturnType] '{}::{}' declared='{}' actual='{}'.", className, funcName, function_return_type, actual_return_type);
				else
				{
					spdlog::warn("[SemanticCheck][FAIL][ReturnType] '{}::{}' declared='{}' actual='{}' (unexpected funchead shape).", className, funcName, function_return_type, actual_return_type);
				}
			}

			if (declFunctionNode != nullptr)
			{
				std::string declaredReturnType = collectDeclaredReturnType(declFunctionNode);
				const std::string implReturnType = collectDeclaredReturnType(funcHeadNode);
				const auto declaredParamTypes = collectOrderedParamTypes(declFunctionNode);
				const auto implParamTypes = collectOrderedParamTypes(funcHeadNode);
				if (funcName == "constructor" && declaredReturnType.empty())
				{
					declaredReturnType = "void";
				}

				if (declaredReturnType == implReturnType && declaredParamTypes == implParamTypes)
				{
					spdlog::info("[SemanticCheck][PASS][Signature] '{}::{}' implementation signature matches declaration (name, ordered parameter types, return type).", className, funcName);
				}
				else
				{
					canGenerateMachineCode = false;
					spdlog::error("[SemanticCheck][FAIL][Signature] '{}::{}' implementation signature does not match declaration.", className, funcName);
				}
			}

			if (classDeclNode != nullptr && funcName == "constructor")
			{
				const auto declaredAttributes = collectClassAttributes(classDeclNode);
				const auto constructorTargets = collectConstructorTargets(funcBodyNode);
				bool allAttributesAssigned = true;
				for (const std::string &attributeName : declaredAttributes)
				{
					if (constructorTargets.find(attributeName) == constructorTargets.end())
					{
						allAttributesAssigned = false;
						canGenerateMachineCode = false;
						spdlog::error("[SemanticCheck][FAIL][ClassMember] '{}::{}' does not assign declared class member '{}'.", className, funcName, attributeName);
					}
				}
				if (allAttributesAssigned)
				{
					spdlog::info("[SemanticCheck][PASS][ClassMember] '{}::{}' assigns all declared class members.", className, funcName);
				}
			}
		}
		else
		{
			canGenerateMachineCode = false;
		}
	}
	else
	{
		spdlog::error("[SemanticCheckingVisitor] visit(implNode): expected 2 children, got {}.", childrenCount);
		canGenerateMachineCode = false;
	}
}

void SemanticCheckingVisitor::visit(reptimpldef3Node &n)
{
	// I dont care about the amount of children i am just looking the "freturnstatemenetNode" child
	if (node *child = find_child("freturnstatement", n.children); child != nullptr)
	{
		n.stEntry.type = child->stEntry.type;
		spdlog::trace("[SemanticCheck][Trace] reptimpldef3 resolved return statement type='{}'.", child->stEntry.type);
		n.stEntry.type = child->stEntry.type;
	}

	else
	{
		n.stEntry.type = "void";
		spdlog::debug("[SemanticCheck][Flow] reptimpldef3 has no return statement; defaulting body type to 'void'.");
	}
}

void SemanticCheckingVisitor::visit(startNode &head)
{
	for (node *child : head.children)
	{
		// spdlog::debug("[SemanticCheck]   child nodeType='{}' semanticMeaning='{}' C++ type={}",
		//				  child->nodeType, child->semanticMeaning, typeid(*child).name());
	}
}

void SemanticCheckingVisitor::visit(reptstatement4Node &n)
{
	// Need to check the left and right side of the assignnode to validate if the assignment is valid
	// Get left child
	// assignnode is supposed to be a binary
	if (n.children.size() != 2 || n.children[0] == nullptr || n.children[1] == nullptr)
	{
		spdlog::error("[SemanticCheck] AssignNode ERROR : NOT BINARY TREE");
		return;
	}

	node *leftchild = n.children.at(0); // LHS variable (reptvariable2andidNode)
										// quick checks
	// spdlog::debug("[SemanticCheck]   Left child nodeType='{}' semanticMeaning='{}' C++ type={}",
	//				  leftchild->nodeType, leftchild->semanticMeaning, typeid(*leftchild).name());
	node *rightchild = n.children.at(1); // RHS expression (exprNode)
}

void SemanticCheckingVisitor::visit(floatnumNode &n)
{
	spdlog::debug("[SemanticCheck] floatnumNode: nodeValue='{}' nodeType='{}' semanticMeaning='{}'",
				  n.nodeValue, n.nodeType, n.semanticMeaning);
	n.stEntry.type = "float";
}

void SemanticCheckingVisitor::visit(paramNode &n)
{
	int childrenCount = n.children.size();

	// param is built with an epsilon-delimited pop, so it may include extra children
	// (for example dimlist/reptfparams3), not just id+type.
	if (childrenCount < 2)
	{
		spdlog::error("[SemanticCheckingVisitor] visit(paramNode): expected at least id and type, got childrenCount={}", childrenCount);
		return;
	}

	node *typeChild = find_child("type", n.children);
	if (typeChild)
	{
		n.stEntry.type = typeChild->nodeValue;
	}
	else
	{
		spdlog::error("[SemanticCheckingVisitor] visit(paramNode): missing type child.");
	}

	node *idChild = find_child("id", n.children);
	if (idChild)
	{
		n.stEntry.name = idChild->nodeValue;
	}
}

void SemanticCheckingVisitor::visit(freturnstatementNode &n)
{
	int childrenCount = n.children.size();

	if (childrenCount == 1)
	{
		spdlog::trace("[SemanticCheck][Trace] freturnstatement shape ok: nodeType='{}'.", n.nodeType);

		std::vector<std::string> children_types = {"expr"};
		node *onlyChild = n.children[0];

		// Check that left and right type of children
		if (checkChildren(1, childrenCount, children_types, n))
		{
			n.stEntry.type = onlyChild->stEntry.type;
			spdlog::trace("[SemanticCheck][Trace] freturnstatement propagated type='{}' from child nodeType='{}'.", onlyChild->stEntry.type, onlyChild->nodeType);
		}
		else
		{
			spdlog::error("WRONG child type: {}. This is not the correct child expected for the return statement, was expecting an exprNode.", onlyChild->nodeType);
		}
	}
	else
	{
		spdlog::error("[SemanticCheckingVisitor] visit(freturnstatementNode): expected 1 child, got {}.", childrenCount);
	}
}

void SemanticCheckingVisitor::visit(exprNode &n)
{
	// The parser always produces 2 children: [expr2/epsilon, arithexpr]
	// child[0] is expr2 (relational) or epsilon (simple), child[1] is arithexpr.
	int childrenCount = n.children.size();
	if (childrenCount == 2)
	{
		node *firstChild = n.children[0];
		node *arithChild = n.children[1];

		if (firstChild->nodeType == "epsilon" && arithChild->nodeType == "arithexpr")
		{
			// Simple expression — just an arithexpr with no relational operator
			n.stEntry.type = arithChild->stEntry.type;
		}
		else if (firstChild->nodeType == "expr2")
		{
			// Relational expression (arithexpr relop arithexpr) — result is boolean/integer
			n.stEntry.type = "integer";
		}
		else
		{
			spdlog::warn("[SemanticCheckingVisitor] visit(exprNode): unexpected child[0] nodeType='{}', propagating arithexpr type.", firstChild->nodeType);
			n.stEntry.type = arithChild->stEntry.type;
			spdlog::debug("exprNode :: n.stEntry.type = {}", n.stEntry.type);
		}
	}

	else
	{
		spdlog::error("[SemanticCheckingVisitor] visit(exprNode): unexpected childrenCount={} for node name: '{}', node type: '{}'.", childrenCount, n.stEntry.name, n.stEntry.type);
	}
}
void SemanticCheckingVisitor::visit(arithexprNode &n)
{
	// if (remove_node(n))
	// 	return;
	int childrenCount = static_cast<int>(n.children.size());
	bool isSingleTermShape = (childrenCount == 1 && n.children[0] && (n.children[0]->nodeType == "term" || n.children[0]->nodeType == "id" || n.semanticMeaning == "term"));
	bool isCommonTermShape = (childrenCount == 2 && n.semanticMeaning == "term" && n.children[0] && n.children[0]->nodeType == "id");
	if (isSingleTermShape || isCommonTermShape)
	{
		node *termChild = (n.children[0]->nodeType == "term") ? n.children[0] : &n;
		node *idChild = nullptr;
		if (n.children[0]->nodeType == "id")
		{
			idChild = n.children[0];
		}
		else if (termChild->nodeType == "id")
		{
			idChild = termChild;
		}
		else
		{
			idChild = find_child("id", termChild->children);
		}

		std::string termName = idChild ? idChild->nodeValue : "<unknown-term>";
		std::string resolvedType = idChild ? idChild->stEntry.type : "";

		if (resolvedType.empty() && !termName.empty() && termName != "<unknown-term>")
		{
			for (node *scope = &n; scope != nullptr; scope = scope->parent)
			{
				auto it = scope->stMap.find(termName);
				if (it != scope->stMap.end() && it->second)
				{
					resolvedType = it->second->type;
					break;
				}
				if (scope->stEntry.hasLink && scope->stEntry.link)
				{
					auto linkedIt = scope->stEntry.link->find(termName);
					if (linkedIt != scope->stEntry.link->end() && linkedIt->second)
					{
						resolvedType = linkedIt->second->type;
						break;
					}
				}
			}
		}
		if (resolvedType.empty())
		{
			resolvedType = findTypeFromRoot(root, termName);
		}

		if (!resolvedType.empty())
		{
			termChild->stEntry.name = termName;
			termChild->stEntry.type = resolvedType;
			if (idChild)
			{
				idChild->stEntry.type = resolvedType;
			}
			n.stEntry.type = resolvedType;
			spdlog::trace("[SemanticCheck][Trace] arithexpr single-term resolved: name='{}', type='{}'.", termName, resolvedType);
		}
		else
		{
			spdlog::warn("[SemanticCheck] arithexpr single-term unresolved type for term='{}'.", termName);
		}
		return;
	}

	if (n.isLeaf)
	{
		// collapsed literal — read type from semanticMeaning
		if (n.semanticMeaning == "floatnum")
			n.stEntry.type = "float";
		else if (n.semanticMeaning == "intnum")
			n.stEntry.type = "integer";
	}
	else
	{
		// normal case — propagate type from child
		if (!n.children.empty() && n.children[0])
		{
			n.stEntry.type = n.children[0]->stEntry.type;
		}
		else
		{
			spdlog::error("[SemanticCheckingVisitor] visit(arithexprNode): missing child for non-leaf node.");
		}
	}
}

void SemanticCheckingVisitor::visit(termNode &n)
{
	if (n.children.size() == 1 && n.children[0] && n.children[0]->nodeType == "id")
	{
		node *idChild = n.children[0];
		std::string termName = idChild->nodeValue;
		std::string resolvedType = idChild->stEntry.type;

		if (resolvedType.empty() && !termName.empty())
		{
			for (node *scope = &n; scope != nullptr; scope = scope->parent)
			{
				auto it = scope->stMap.find(termName);
				if (it != scope->stMap.end() && it->second)
				{
					resolvedType = it->second->type;
					break;
				}
				if (scope->stEntry.hasLink && scope->stEntry.link)
				{
					auto linkedIt = scope->stEntry.link->find(termName);
					if (linkedIt != scope->stEntry.link->end() && linkedIt->second)
					{
						resolvedType = linkedIt->second->type;
						break;
					}
				}
			}
		}
		if (resolvedType.empty())
		{
			resolvedType = findTypeFromRoot(root, termName);
		}

		if (!resolvedType.empty())
		{
			n.stEntry.name = termName;
			n.stEntry.type = resolvedType;
			idChild->stEntry.type = resolvedType;
			spdlog::trace("[SemanticCheck][Trace] term resolved: name='{}', type='{}'.", termName, resolvedType);
		}
		else
		{
			spdlog::warn("[SemanticCheck] term unresolved type for id='{}'.", termName);
		}
	}
}

void SemanticCheckingVisitor::visit(assignNode &n)
{
	if (n.children.size() != 2 || n.children[0] == nullptr || n.children[1] == nullptr)
	{
		spdlog::error("[SemanticCheckingVisitor] visit(assignNode): expected binary assign node, got {} children.", n.children.size());
		return;
	}

	// The assignment expression type is the RHS expression type.
	n.stEntry.type = n.children[1]->stEntry.type;
}

bool SemanticCheckingVisitor::remove_node(node &n)
{
	std::string &node_name = n.get_name();
	std::string &node_type = n.get_type();
	std::string &node_visibility = n.get_visibility();
	std::string &node_kind = n.get_kind();

	if ((node_name.empty() && node_type.empty() && node_visibility.empty() && node_kind.empty()) && (n.isLeaf || n.children.empty()))
	{
		spdlog::warn("[SemanticCheckingVisitor] Semantic Checking remove_node :: About to remove the node: : {}, node type: {} and .", n.stEntry.name, n.stEntry.type);

		// Bridge the doubly-linked sibling chain
		if (n.leftSibling)
			n.leftSibling->rightSibling = n.rightSibling;
		if (n.rightSibling)
			n.rightSibling->leftSibling = n.leftSibling;

		// If this node is the head of the sibling list, advance the head
		if (n.headOfSibling == &n)
		{
			if (n.rightSibling)
				n.rightSibling->headOfSibling = n.rightSibling;
		}

		// Remove from parent's children vector
		if (n.parent)
		{
			auto &siblings = n.parent->children;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), &n), siblings.end());
		}

		return true;
	}
	return false;
}

bool SemanticCheckingVisitor::checkChildren(int expected_num_of_children, int actual_num_of_children, const std::vector<std::string> &expected_children_types, node &n)
{
	if (expected_num_of_children != actual_num_of_children)
	{
		spdlog::error("SemanticCheckingVisitor :: Unrecoverable error");
		return false;
		// throw std::runtime_error("Expected_num_of_children does not equal to the actual number of children");
	}

	if (expected_num_of_children == 2)
	{
		// Order in children_names matters, the first is supposed to be the left child and the second is the right child
		return (match_without_order(expected_num_of_children, expected_children_types, n.children));
	}
	else if (expected_num_of_children == 1)
	{
		return (match_without_order(expected_num_of_children, expected_children_types, n.children));
	}
	return false;
}

bool SemanticCheckingVisitor::match_without_order(int num_of_children, std::vector<std::string> children_names, std::vector<node *> &children)
{
	// Need to compare the names in the children_names vector and compare them to the children.
	if (num_of_children == 2)
	{
		if (children[0]->nodeType == children_names[0] && children[1]->nodeType == children_names[1])
		{
			return true;
		}
		else
		{
			if (children[1]->nodeType == children_names[0] && children[0]->nodeType == children_names[1])
			{
				spdlog::debug("match_without_order :: incorect child order but the number of children is correct. Swapping the child vector pointers values around so that they point to the right child");
				std::swap(children[0], children[1]);
				return true;
			}
			spdlog::debug("match_without_order :: incorect children types.");
			return false;
		}
	}
	else if (num_of_children == 1)
	{
		if (children[0]->nodeType == children_names[0])
		{
			return true;
		}
		else
		{
			spdlog::debug("match_without_order :: incorect children types.");
			return false;
		}
	}
	return false;
}

node *visitor::find_child(std::string missing_child, std::vector<node *> vector_of_children)
{
	for (auto child_it = vector_of_children.begin(); child_it != vector_of_children.end(); ++child_it)
	{
		if ((*child_it)->nodeType == missing_child)
		{
			return (*child_it);
		}
		continue;
	}
	return nullptr;
}

void SemanticCheckingVisitor::visit(funcheadNode &n)
{
	// I dont care about the amount of children i am just looking the "freturnstatemenetNode" child
	if (node *child = find_child("returntype", n.children); child != nullptr)
	{
		// bring the type up
		spdlog::trace("[SemanticCheck][Trace] funchead return type discovered: '{}'", child->stEntry.type);
		n.stEntry.type = child->stEntry.type;
	}
}

void SemanticCheckingVisitor::visit(returntypeNode &n)
{
	if (!n.nodeValue.empty())
	{
		n.stEntry.type = n.nodeValue;
	}
}
