#include "visitor.h"
#include <typeinfo>

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
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind = "parameter";
	ste->name = get("id", head);
	ste->type = get("type", head);
}

void SymTabCreationVisitor::visit(classNode &head)
{
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
					spdlog::debug("[SymTabCreation] Merging impl of '{}' into class declaration.", className);
					for (auto &[fname, fentry] : *child->stEntry.link)
					{
						spdlog::debug("[SymTabCreation]   Checking impl function '{}' (return type='{}').", fname, fentry->type);

						// Check 1: function name exists in class declaration
						if (classEntry->link->count(fname) > 0)
						{
							node::symbolTableEntry *declaredFunc = (*classEntry->link)[fname];
							spdlog::debug("[SymTabCreation]   [OK] '{}::{}' found in class declaration (declared return type='{}').", className, fname, declaredFunc->type);

							// Copy visibility from class declaration
							fentry->visibility = declaredFunc->visibility;
							spdlog::debug("[SymTabCreation]   Copied visibility '{}' to impl entry.", fentry->visibility);

							// Check 2: return type matches
							if (!declaredFunc->type.empty() && declaredFunc->type != fentry->type)
							{
								spdlog::error("[SymTabCreation] Return type mismatch for '{}::{}': declared '{}', implemented '{}'.",
											  className, fname, declaredFunc->type, fentry->type);
							}
							else
							{
								spdlog::debug("[SymTabCreation]   [OK] Return type '{}' matches for '{}::{}'.", fentry->type, className, fname);
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
										spdlog::debug("[SymTabCreation]     Declared param: '{}' type='{}'.", pname, pentry->type);
									}
								}
								// Build impl param map
								std::map<std::string, std::string> implParams;
								for (auto &[pname, pentry] : *fentry->link)
								{
									if (pentry->kind == "parameter")
									{
										implParams[pname] = pentry->type;
										spdlog::debug("[SymTabCreation]     Impl param: '{}' type='{}'.", pname, pentry->type);
									}
								}
								// Check each declared param exists in impl with matching type
								for (auto &[pname, ptype] : declaredParams)
								{
									if (implParams.count(pname) == 0)
										spdlog::error("[SymTabCreation] Parameter '{}' declared in '{}::{}' but missing in implementation.", pname, className, fname);
									else if (implParams[pname] != ptype)
										spdlog::error("[SymTabCreation] Parameter '{}' type mismatch in '{}::{}': declared '{}', implemented '{}'.", pname, className, fname, ptype, implParams[pname]);
									else
										spdlog::debug("[SymTabCreation]     [OK] Param '{}' type '{}' matches.", pname, ptype);
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
								spdlog::debug("[SymTabCreation]   No parameter links to compare for '{}::{}'.", className, fname);
							}
						}
						else
						{
							spdlog::error("[SymTabCreation] Function '{}' implemented in class '{}' but never declared.", fname, className);
						}
						(*classEntry->link)[fname] = fentry;
						spdlog::debug("[SymTabCreation]   Merged '{}::{}' into class symbol table.", className, fname);
					}
				}
				else
				{
					spdlog::debug("[SymTabCreation] No link maps available to merge for impl of '{}'.", className);
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
	node::symbolTableEntry *symbol_table_entry = &head.stEntry;
	spdlog::debug("[SemanticCheck] visit(impldefNode): kind='{}' name='{}'", symbol_table_entry->kind, symbol_table_entry->name);
}

void SemanticCheckingVisitor::visit(implNode &head)
{
}

void SemanticCheckingVisitor::visit(startNode &head)
{
	spdlog::debug("[SemanticCheck] visit(startNode): children count={}", head.children.size());
	for (node *child : head.children)
	{
		spdlog::debug("[SemanticCheck]   child nodeType='{}' semanticMeaning='{}' C++ type={}",
					  child->nodeType, child->semanticMeaning, typeid(*child).name());
	}
}
