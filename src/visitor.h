#pragma once

#include "node.h"
#include <unordered_map>
#include <unordered_set>
#include <spdlog/spdlog.h>

class visitor
{
public:
    virtual void visit(paramNode &n) {};
    virtual void visit(startNode &n) = 0;
    virtual void visit(intNode &n) {};
    virtual void visit(floatNode &n) {};
    virtual void visit(epsilonNode &n) {};
    virtual void visit(aparamsNode &n) {};
    virtual void visit(aparamstailNode &n) {};
    virtual void visit(addopNode &n) {};
    virtual void visit(arithexprNode &n) {};
    virtual void visit(arraysizeNode &n) {};
    virtual void visit(assignNode &n) {};
    virtual void visit(attributedeclNode &n) {};
    virtual void visit(idNode &n) {};
    virtual void visit(reptclassdecl4Node &n) {};
    virtual void visit(classdeclNode &n) {};
    virtual void visit(classNode &n) {};
    virtual void visit(implNode &n) {};
    virtual void visit(funcNode &n) {};
    virtual void visit(exprNode &n) {};
    virtual void visit(expr2Node &n) {};
    virtual void visit(relopNode &n) {};
    virtual void visit(reptfparams3Node &n) {};
    virtual void visit(reptfparams4Node &n) {};
    virtual void visit(fparamsNode &n) {};
    virtual void visit(factorNode &n) {};
    virtual void visit(factor2Node &n) {};
    virtual void visit(selfNode &n) {};
    virtual void visit(selfandidnest2Node &n) {};
    virtual void visit(dotNode &n) {};
    virtual void visit(idnest2Node &n) {};
    virtual void visit(reptvariable2Node &n) {};
    virtual void visit(aparamsandidNode &n) {};
    virtual void visit(reptvariable2andidNode &n) {};
    virtual void visit(reptfuncbody1Node &n) {};
    virtual void visit(funcbodyNode &n) {};
    virtual void visit(funcdeclNode &n) {};
    virtual void visit(funcdefNode &n) {};
    virtual void visit(funcheadNode &n) {};
    virtual void visit(reptimpldef3Node &n) {};
    virtual void visit(impldefNode &n) {};
    virtual void visit(indiceNode &n) {};
    virtual void visit(localvardeclNode &n) {};
    virtual void visit(localvardeclorstatNode &n) {};
    virtual void visit(multopNode &n) {};
    virtual void visit(reptoptclassdecl22Node &n) {};
    virtual void visit(optclassdecl2Node &n) {};
    virtual void visit(reptprog0Node &n) {};
    virtual void visit(progNode &n) {};
    virtual void visit(relexprNode &n) {};
    virtual void visit(funcdeclfamNode &n) {};
    virtual void visit(attributedeclfamNode &n) {};
    virtual void visit(voidNode &n) {};
    virtual void visit(returntypeNode &n) {};
    virtual void visit(rightrectermNode &n) {};
    virtual void visit(signNode &n) {};
    virtual void visit(reptstatblock1Node &n) {};
    virtual void visit(statblockNode &n) {};
    virtual void visit(ifstatementNode &n) {};
    virtual void visit(ifNode &n) {};
    virtual void visit(conditionNode &n) {};
    virtual void visit(thenNode &n) {};
    virtual void visit(felseNode &n) {};
    virtual void visit(whilestatementNode &n) {};
    virtual void visit(whileNode &n) {};
    virtual void visit(readNode &n) {};
    virtual void visit(readstatementNode &n) {};
    virtual void visit(writeNode &n) {};
    virtual void visit(writestatementNode &n) {};
    virtual void visit(returnNode &n) {};
    virtual void visit(freturnstatementNode &n) {};
    virtual void visit(reptstatement4Node &n) {};
    virtual void visit(termNode &n) {};
    virtual void visit(typeNode &n) {};
    virtual void visit(vardeclNode &n) {};
    virtual void visit(visibilityNode &n) {};
    virtual void visit(reptvardecl3Node &n) {};
    virtual void visit(rightrecarithexprNode &n) {};
    virtual void visit(floatnumNode &n) {};
    virtual ~visitor() {} // Virtual destructor for proper cleanup
};

class SymTabCreationVisitor : public visitor
{
public:
    enum type
    {
        CLASSDECL,
        CLASSIMPL,
        FUNCDECL,
        FUNCIMPL
    };
    std::unordered_map<std::string, type> semanticTracking;
    virtual ~SymTabCreationVisitor() {}
    using visitor::visit;
    void visit(node *head) {};
    void visit(funcNode &head);
    void visit(localvardeclNode &head);
    void visit(funcheadNode &head);
    void visit(funcdefNode &head);
    void visit(startNode &head);
    void visit(implNode &head);
    void visit(funcdeclNode &head);
    void visit(fparamsNode &head);
    void visit(paramNode &head);
    void visit(classNode &head);
    void visit(attributedeclNode &head);
    void visit(attributedeclfamNode &head);
    void visit(funcdeclfamNode &head);
    void visit(funcbodyNode &head);
    std::string get(std::string, node &head);
};

class SemanticCheckingVisitor : public visitor
{
public:
    node *root = nullptr; // set to parser.AST.treeHead before the pass
    virtual ~SemanticCheckingVisitor() {}
    // Need to implement semnatic checking with the semantic checking visitor and not the sym tab creation visitor.
    void visit(startNode &head);
    void visit(implNode &head);
    void visit(impldefNode &head);
};
