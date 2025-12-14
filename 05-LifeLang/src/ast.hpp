#ifndef LIFELANG_AST_HPP
#define LIFELANG_AST_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace llvm {
class Value;
class BasicBlock;
}

namespace lifelang {

struct CodeGen;

enum class RetType { Void, I32 };

struct Node {
    virtual ~Node() = default;
};

struct Expr : Node {
    virtual llvm::Value* codegen(CodeGen& cg) = 0;
    virtual llvm::Value* codegenPtr(CodeGen& cg) { (void)cg; return nullptr; }
};

struct Stmt : Node {
    virtual void codegen(CodeGen& cg) = 0;
};

struct Decl : Node {
    virtual void codegen(CodeGen& cg) = 0;
};

struct Program : Node {
    std::vector<Decl*> decls;
    std::unordered_map<std::string, int64_t> consts; 

    Program() = default;
    explicit Program(std::vector<Decl*>&& d) : decls(std::move(d)) {}
    ~Program();
    void codegen(CodeGen& cg);
};



struct IntExpr : Expr {
    int64_t value;
    explicit IntExpr(int64_t v) : value(v) {}
    llvm::Value* codegen(CodeGen& cg) override;
};

struct NameExpr : Expr {
    std::string name;
    explicit NameExpr(std::string n) : name(std::move(n)) {}
    llvm::Value* codegen(CodeGen& cg) override;
    llvm::Value* codegenPtr(CodeGen& cg) override;
};

struct UnaryExpr : Expr {
    std::string op;
    Expr* rhs;
    UnaryExpr(std::string o, Expr* r) : op(std::move(o)), rhs(r) {}
    ~UnaryExpr() override;
    llvm::Value* codegen(CodeGen& cg) override;
};

struct BinaryExpr : Expr {
    std::string op;
    Expr* lhs;
    Expr* rhs;
    BinaryExpr(std::string o, Expr* l, Expr* r) : op(std::move(o)), lhs(l), rhs(r) {}
    ~BinaryExpr() override;
    llvm::Value* codegen(CodeGen& cg) override;
};

struct CallExpr : Expr {
    std::string callee;
    std::vector<Expr*> args;
    CallExpr(std::string c, std::vector<Expr*>&& a) : callee(std::move(c)), args(std::move(a)) {}
    ~CallExpr() override;
    llvm::Value* codegen(CodeGen& cg) override;
};

struct IndexExpr : Expr {
    std::string base;
    std::vector<Expr*> indices;
    IndexExpr(std::string b, std::vector<Expr*>&& idx)
        : base(std::move(b)), indices(std::move(idx)) {}
    ~IndexExpr() override;
    llvm::Value* codegen(CodeGen& cg) override; 
    llvm::Value* codegenPtr(CodeGen& cg) override; 
};

struct BlockStmt : Stmt {
    std::vector<Stmt*> stmts;
    explicit BlockStmt(std::vector<Stmt*>&& s) : stmts(std::move(s)) {}
    ~BlockStmt() override;
    void codegen(CodeGen& cg) override;
};

struct VarDeclStmt : Stmt {
    std::string name;
    std::vector<Expr*> dims;
    Expr* init;
    VarDeclStmt(std::string n, std::vector<Expr*>&& d, Expr* i)
        : name(std::move(n)), dims(std::move(d)), init(i) {}
    ~VarDeclStmt() override;
    void codegen(CodeGen& cg) override;
};

struct AssignStmt : Stmt {
    Expr* lhs;
    Expr* rhs;
    AssignStmt(Expr* l, Expr* r) : lhs(l), rhs(r) {}
    ~AssignStmt() override;
    void codegen(CodeGen& cg) override;
};

struct ExprStmt : Stmt {
    Expr* expr;
    explicit ExprStmt(Expr* e) : expr(e) {}
    ~ExprStmt() override;
    void codegen(CodeGen& cg) override;
};

struct IfStmt : Stmt {
    Expr* cond;
    BlockStmt* thenB;
    BlockStmt* elseB; 
    IfStmt(Expr* c, BlockStmt* t, BlockStmt* e) : cond(c), thenB(t), elseB(e) {}
    ~IfStmt() override;
    void codegen(CodeGen& cg) override;
};

struct WhileStmt : Stmt {
    Expr* cond;
    BlockStmt* body;
    WhileStmt(Expr* c, BlockStmt* b) : cond(c), body(b) {}
    ~WhileStmt() override;
    void codegen(CodeGen& cg) override;
};

struct ForStmt : Stmt {
    Stmt* init;  
    Expr* cond;  
    Stmt* step;  
    BlockStmt* body;
    ForStmt(Stmt* i, Expr* c, Stmt* s, BlockStmt* b) : init(i), cond(c), step(s), body(b) {}
    ~ForStmt() override;
    void codegen(CodeGen& cg) override;
};

struct BreakStmt : Stmt {
    void codegen(CodeGen& cg) override;
};

struct ContinueStmt : Stmt {
    void codegen(CodeGen& cg) override;
};

struct ReturnStmt : Stmt {
    Expr* value;
    explicit ReturnStmt(Expr* v) : value(v) {}
    ~ReturnStmt() override;
    void codegen(CodeGen& cg) override;
};


struct ConstDecl : Decl {
    std::string name;
    int64_t value;
    ConstDecl(std::string n, int64_t v) : name(std::move(n)), value(v) {}
    void codegen(CodeGen& cg) override;
};

struct FuncDecl : Decl {
    std::string name;
    RetType retType;
    BlockStmt* body;
    FuncDecl(std::string n, RetType rt, BlockStmt* b) : name(std::move(n)), retType(rt), body(b) {}
    ~FuncDecl() override;
    void codegen(CodeGen& cg) override;
};

} // namespace lifelang

#endif
