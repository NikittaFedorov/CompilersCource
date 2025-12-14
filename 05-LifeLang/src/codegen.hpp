#ifndef LIFELANG_CODEGEN_HPP
#define LIFELANG_CODEGEN_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>

#include "ast.hpp"

namespace lifelang {

struct VarInfo {
    llvm::AllocaInst* alloca = nullptr;
    std::vector<int64_t> dims; 
};

struct LoopTarget {
    llvm::BasicBlock* breakBB = nullptr;
    llvm::BasicBlock* continueBB = nullptr;
};

struct CodeGen {
    llvm::LLVMContext ctx;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    lifelang::Program* program = nullptr;
    std::unordered_map<std::string, int64_t>* programConsts = nullptr;

    std::vector<std::unordered_map<std::string, VarInfo>> scopes;
    std::vector<LoopTarget> loopStack;

    llvm::FunctionCallee fn_simFillRect;
    llvm::FunctionCallee fn_simFlush;
    llvm::FunctionCallee fn_simDelay;
    llvm::FunctionCallee fn_simRand;
    llvm::FunctionCallee fn_simGetTicks;
    llvm::FunctionCallee fn_checkFinish;
    llvm::FunctionCallee fn_simPutPixel;
    llvm::FunctionCallee fn_simGetMouseX;
    llvm::FunctionCallee fn_simGetMouseY;
    llvm::FunctionCallee fn_simIsMouseButtonDown;
    llvm::FunctionCallee fn_simIsKeyDown;

    CodeGen();

    llvm::Type* i32Ty();
    llvm::Type* voidTy();
    llvm::Value* constI32(int64_t v);

    void addConst(const std::string& name, int64_t value);
    std::optional<int64_t> getConst(const std::string& name) const;

    void pushScope();
    void popScope();
    VarInfo* findVar(const std::string& name);

    llvm::AllocaInst* createEntryAlloca(llvm::Function* fn, llvm::Type* ty, const std::string& name);
    llvm::Value* toI64(llvm::Value* v);
    llvm::Value* truthy(llvm::Value* v);
    int64_t evalConstDim(Expr* e);

    void declareBuiltins();

    void declareFunctionPrototype(const FuncDecl& f);
    void codegenFunction(const FuncDecl& f);

    llvm::Value* addrOfName(const std::string& name);
    llvm::Value* loadName(const std::string& name);

    llvm::Type* arrayTypeFromDims(const std::vector<int64_t>& dims);
    llvm::Value* addrOfIndex(const std::string& base, const std::vector<Expr*>& indices);

    llvm::Value* codegenUnary(const std::string& op, Expr* rhs);
    llvm::Value* codegenBinary(const std::string& op, Expr* lhs, Expr* rhs);
    llvm::Value* codegenCall(const std::string& callee, const std::vector<Expr*>& args);

    void codegenBlock(BlockStmt& blk);
    void codegenVarDecl(const std::string& name, const std::vector<Expr*>& dims, Expr* init);
    void codegenAssign(Expr* lhs, Expr* rhs);
    void codegenIf(Expr* cond, BlockStmt* thenB, BlockStmt* elseB);
    void codegenWhile(Expr* cond, BlockStmt* body);
    void codegenFor(Stmt* init, Expr* cond, Stmt* step, BlockStmt* body);
    void codegenBreak();
    void codegenContinue();
    void codegenReturn(Expr* value);
};

} // namespace lifelang

#endif
