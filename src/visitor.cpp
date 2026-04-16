#include "visitor.h"

std::string SymTabCreationVisitor::get(std::string search, node& head){
	for(node * child : head.children){
		if(child->semanticMeaning == search){
			return child->nodeValue;
		}
	}
	if(head.children.empty()){
		return "";
	}
	return get(search,(*head.children[0]));


}
void SymTabCreationVisitor:: visit(funcdeclNode & head){
	node::symbolTableEntry *ste = &head.stEntry;

	//kind
	ste->kind = "function";
	//haslink
	ste->hasLink =true;
	//name
	ste->name=get("id",head);
	ste->type = get("returntype",head);

	for(node*children:head.children){

		 if(children->nodeType=="fparams"){
			//link
			 ste->link =  &children->stMap;
		}
	}
}

void SymTabCreationVisitor:: visit(fparamsNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	for(node*child:head.children){
		if(child->nodeType=="param"){
			 if((*mp).count(child->stEntry.name)>0){
				 spdlog::error("Multiple Parameters with the same name for: "+child->stEntry.name +".");
			 }
			 else{
					(*mp)[child->stEntry.name] =& child->stEntry;
			 }
		}
	}
}

void SymTabCreationVisitor:: visit(paramNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="parameter";
	ste->name = get("id",head);
	ste->type = get("type",head);


}

void SymTabCreationVisitor:: visit(classNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="class";
	ste->name = get("id",head);
	ste->hasLink =true;
	ste->link=mp;
	for(node*child:head.children){
		if(child->semanticMeaning=="isalist"){
			for(node*children:child->children){
				ste->type+=children->nodeValue;

			}
		}
		else if(child->semanticMeaning=="{"){
			for(node*children:child->children){
				if(children->semanticMeaning=="attributedecl"){
					 if((*mp).count(children->stEntry.name)>0){
						 spdlog::error("Multiple Attribute Declarations for: "+children->stEntry.name +".");
					 }
					 else{
							(*mp)[children->stEntry.name] =& children->stEntry;

					 }

				}
				else if(children->semanticMeaning == "funcdecl"){
					// funcdeclfamNode wraps visibility + funchead
					if(!children->stEntry.name.empty()){
						if((*mp).count(children->stEntry.name)>0){
							spdlog::error("Multiple Function Declarations for: "+children->stEntry.name +".");
						}
						else{
							(*mp)[children->stEntry.name] = &children->stEntry;
						}
					}
					else{
						// fallback: dig into grandchildren for funchead
						for(node*grandchildren:children->children){
							if(grandchildren->semanticMeaning=="funchead"){
								if((*mp).count(grandchildren->stEntry.name)>0){
									spdlog::error("Multiple Function Declarations for: "+grandchildren->stEntry.name +".");
								}
								else{
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

void SymTabCreationVisitor:: visit(startNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	std::unordered_set<std::string> implementedClasses;
	for(node*child:head.children){
		 if(child->semanticMeaning=="classdecl"){
			//link
			 if((*mp).count(child->stEntry.name)>0){
				 spdlog::error("Multiple Class Declaration for: "+child->stEntry.name +".");

			 }
			 else{
				 (*mp)[child->stEntry.name] = &child->stEntry;

			 }
		}
		 else if(child->semanticMeaning=="funcdef"){
			 (*mp)[child->stEntry.name] = &child->stEntry;
		 }
		 else if(child->semanticMeaning=="impldef"){
			 std::string className = child->stEntry.name;
			 if((*mp).count(className)>0 && (*mp)[className]->kind=="class"){
				 if(implementedClasses.find(className)==implementedClasses.end()){
					 spdlog::error("Class Declaration found for " + className + ". Updating implementation.");
				 }
				 implementedClasses.insert(className);
				 auto* classEntry = (*mp)[className];
				 // Merge function entries from this impldef into the class's stMap
				 if(child->stEntry.hasLink && child->stEntry.link && classEntry->hasLink && classEntry->link){
					 for(auto& [fname, fentry] : *child->stEntry.link){
						 // Copy visibility from class declaration's matching function head
						 if(classEntry->link->count(fname)>0){
							 fentry->visibility = (*classEntry->link)[fname]->visibility;
						 }
						 (*classEntry->link)[fname] = fentry;
					 }
				 }
			 }
			 else{
				 spdlog::error("Missing Class Declaration for: "+child->stEntry.name +".");
			 }

		 }

	}
    for (const auto& [key, value] : (*mp)) {
    	if(value->kind=="class" && implementedClasses.find(key)==implementedClasses.end()){
    		spdlog::error("There is a class declaration: "+value->name+" without implementation");
    	}
    }

}

void SymTabCreationVisitor:: visit(attributedeclNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="variable";
	ste->name = get("id",head);
	ste->type = get("type",head);

}

void SymTabCreationVisitor:: visit(attributedeclfamNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="attribute";
	for(node*child:head.children){
		if(child->semanticMeaning =="vardecl"){
			ste->name=child->stEntry.name;
			ste->type=child->stEntry.type;
		}
		else if(child->semanticMeaning =="visibility"){
			ste->visibility=child->nodeValue;
		}
	}
}
void SymTabCreationVisitor:: visit(funcdeclfamNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="function head";
	for(node*child:head.children){
		if(child->nodeType =="funcdecl"){
			// funcdecl inherits funchead's children via passAlong
			// Mirror funcheadNode logic for name/type/link
			for(node* gc : child->children){
				if(gc->semanticMeaning=="id" && ste->name.empty()){
					ste->name = gc->nodeValue;
				}
				else if(gc->semanticMeaning=="returntype"){
					ste->type = gc->nodeValue;
				}
				else if(gc->semanticMeaning=="funcparams"){
					ste->link = &gc->stMap;
					ste->hasLink = true;
				}
			}
			if(ste->name.empty()) ste->name = "constructor";
		}
		else if(child->semanticMeaning =="visibility"){
			ste->visibility=child->nodeValue;
		}
	}
}
void SymTabCreationVisitor:: visit(implNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="implementation";
	ste->hasLink =true;
	ste->link=mp;

	std::string className;
	std::string funcName;
	node* funcheadChild = nullptr;

	for(node*child:head.children){
		if(child->semanticMeaning=="funchead"){
			funcheadChild = child;
			bool foundFirst = false;
			for(node* fc : child->children){
				if(fc->semanticMeaning=="id"){
					if(!foundFirst){ className = fc->nodeValue; foundFirst = true; }
					else { funcName = fc->nodeValue; }
				}
			}
			if(funcName.empty()) funcName = "constructor";
		}
	}

	ste->name = className;

	// Build a function entry from the funchead
	if(funcheadChild){
		node::symbolTableEntry* funcEntry = &funcheadChild->stEntry;
		funcEntry->kind = "function";
		funcEntry->name = funcName;
		funcEntry->hasLink = true;
		// If funchead didn't have a link, use its own stMap
		if(!funcEntry->link){
			funcEntry->link = &funcheadChild->stMap;
		}
		// Merge local vardecls from reptimpldef3 into the function's param/local map
		for(node*child:head.children){
			if(child->semanticMeaning=="reptimpldef3"){
				for(node*bodyChild:child->children){
					if(bodyChild->semanticMeaning=="vardecl"){
						if(funcEntry->link->count(bodyChild->stEntry.name)==0){
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

void SymTabCreationVisitor:: visit(funcdefNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="function";
	ste->hasLink =true;
	ste->link=mp;

	for(node*child:head.children){
		 if(child->semanticMeaning=="funchead"){
			 mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			 ste->name = child->stEntry.name;
			 ste->type = child->stEntry.type;
		 }
		 else if(child->semanticMeaning=="funcbody"){
			 mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			 child->stEntry.name=ste->name;

		 }

	}
}
void SymTabCreationVisitor:: visit(funcheadNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="function head";
	// Get function name from direct id children only (don't recurse into params)
	ste->name = "";
	for(node* child : head.children){
		if(child->semanticMeaning=="id"){
			ste->name = child->nodeValue;
			break;
		}
	}
	if(ste->name.empty()) ste->name = "constructor";
	for(node*child:head.children){
		 if(child->semanticMeaning=="returntype"){
			 ste->type=child->nodeValue;
		}
		 else if(child->semanticMeaning =="funcparams" ){
			//link
			ste->link =  &child->stMap;

		 }
	}
}
void SymTabCreationVisitor:: visit(funcbodyNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->hasLink =true;
	ste->link=mp;
	ste->kind ="function body";
	for(node*child:head.children){
		 if(child->semanticMeaning=="vardecl"){
			 if((*mp).count(child->stEntry.name)>0){
				 spdlog::error("Multiple Variable Declarations for: "+child->stEntry.name +".");
			 }
			 else{
					(*mp)[child->stEntry.name] =& child->stEntry;
			 }
		}
	}
}

void SymTabCreationVisitor:: visit(localvardeclNode & head){
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="variable";
	ste->name = get("id",head);
	ste->type = get("type",head);
}

void SymTabCreationVisitor:: visit(funcNode & head){
	std::map<std::string, node::symbolTableEntry*> *mp = &head.stMap;
	node::symbolTableEntry *ste = &head.stEntry;
	ste->kind ="function";
	ste->hasLink =true;
	ste->link=mp;

	for(node*child:head.children){
		 if(child->semanticMeaning=="funchead"){
			 if(!mp->empty())mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			 ste->name = child->stEntry.name;
			 ste->type = child->stEntry.type;
		 }
		 else if(child->semanticMeaning=="funcbody"){
			 mp->insert((*child->stEntry.link).begin(), (*child->stEntry.link).end());
			 child->stEntry.name=ste->name;
		 }
	}
}

