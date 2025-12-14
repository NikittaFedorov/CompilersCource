#include "ast.hpp"
#include "codegen.hpp"

#include <llvm/IR/Value.h>

using namespace lifelang;

Program::~Program() {
    for (auto* d : decls) delete d;
}

void Program::codegen(CodeGen& cg) {
    for (auto* d : decls) {
        if (auto* c = dynamic_cast<ConstDecl*>(d)) {
            consts[c->name] = c->value;
        }
    }
    cg.programConsts = &consts;

    for (auto* d : decls) {
        if (auto* f = dynamic_cast<FuncDecl*>(d)) {
            cg.declareFunctionPrototype(*f);
        }
    }

    for (auto* d : decls) {
        d->codegen(cg);
    }
}

UnaryExpr::~UnaryExpr() { delete rhs; }
BinaryExpr::~BinaryExpr() { delete lhs; delete rhs; }
CallExpr::~CallExpr() { for (auto* a : args) delete a; }
IndexExpr::~IndexExpr() { for (auto* e : indices) delete e; }

BlockStmt::~BlockStmt() { for (auto* s : stmts) delete s; }
VarDeclStmt::~VarDeclStmt() { for (auto* e : dims) delete e; delete init; }
AssignStmt::~AssignStmt() { delete lhs; delete rhs; }
ExprStmt::~ExprStmt() { delete expr; }
IfStmt::~IfStmt() { delete cond; delete thenB; delete elseB; }
WhileStmt::~WhileStmt() { delete cond; delete body; }
ForStmt::~ForStmt() { delete init; delete cond; delete step; delete body; }
ReturnStmt::~ReturnStmt() { delete value; }

FuncDecl::~FuncDecl() { delete body; }

void ConstDecl::codegen(CodeGen& cg) { cg.addConst(name, value); }
void FuncDecl::codegen(CodeGen& cg) { cg.codegenFunction(*this); }

llvm::Value* IntExpr::codegen(CodeGen& cg) { return cg.constI32(value); }

llvm::Value* NameExpr::codegen(CodeGen& cg) { return cg.loadName(name); }

llvm::Value* NameExpr::codegenPtr(CodeGen& cg) { return cg.addrOfName(name); }

llvm::Value* UnaryExpr::codegen(CodeGen& cg) { return cg.codegenUnary(op, rhs); }

llvm::Value* BinaryExpr::codegen(CodeGen& cg) { return cg.codegenBinary(op, lhs, rhs); }

llvm::Value* CallExpr::codegen(CodeGen& cg) { return cg.codegenCall(callee, args); }

llvm::Value* IndexExpr::codegen(CodeGen& cg) {
    auto* ptr = codegenPtr(cg);
    return cg.builder.CreateLoad(cg.i32Ty(), ptr, "ldidx");
}

llvm::Value* IndexExpr::codegenPtr(CodeGen& cg) { return cg.addrOfIndex(base, indices); }

void BlockStmt::codegen(CodeGen& cg) { cg.codegenBlock(*this); }

void VarDeclStmt::codegen(CodeGen& cg) { cg.codegenVarDecl(name, dims, init); }

void AssignStmt::codegen(CodeGen& cg) { cg.codegenAssign(lhs, rhs); }

void ExprStmt::codegen(CodeGen& cg) { (void)expr->codegen(cg); }

void IfStmt::codegen(CodeGen& cg) { cg.codegenIf(cond, thenB, elseB); }

void WhileStmt::codegen(CodeGen& cg) { cg.codegenWhile(cond, body); }

void ForStmt::codegen(CodeGen& cg) { cg.codegenFor(init, cond, step, body); }

void BreakStmt::codegen(CodeGen& cg) { cg.codegenBreak(); }

void ContinueStmt::codegen(CodeGen& cg) { cg.codegenContinue(); }

void ReturnStmt::codegen(CodeGen& cg) { cg.codegenReturn(value); }
