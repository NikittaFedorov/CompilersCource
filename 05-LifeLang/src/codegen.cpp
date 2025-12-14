#include "codegen.hpp"

#include <cassert>
#include <iostream>

#include <llvm/IR/Intrinsics.h>
#include <llvm/Support/raw_ostream.h>

using namespace lifelang;
using namespace llvm;

static void cgError(const std::string& msg) {
    std::cerr << "[LifeLang codegen] ERROR: " << msg << std::endl;
    std::exit(1);
}

CodeGen::CodeGen()
    : module(std::make_unique<Module>("lifelang", ctx)),
      builder(ctx) {
    declareBuiltins();
}

Type* CodeGen::i32Ty() { return Type::getInt32Ty(ctx); }
Type* CodeGen::voidTy() { return Type::getVoidTy(ctx); }

Value* CodeGen::constI32(int64_t v) {
    return ConstantInt::get(i32Ty(), (uint64_t)v, true);
}

void CodeGen::addConst(const std::string& name, int64_t value) {
    if (!programConsts) 
        return;
    (*programConsts)[name] = value;
}

std::optional<int64_t> CodeGen::getConst(const std::string& name) const {
    if (!programConsts) return std::nullopt;
    auto it = programConsts->find(name);

    if (it == programConsts->end()) 
        return std::nullopt;
    return it->second;
}

void CodeGen::pushScope() { scopes.emplace_back(); }
void CodeGen::popScope()  { scopes.pop_back(); }

VarInfo* CodeGen::findVar(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto f = it->find(name);
        if (f != it->end()) 
            return &f->second;
    }
    return nullptr;
}

AllocaInst* CodeGen::createEntryAlloca(Function* fn, Type* ty, const std::string& name) {
    IRBuilder<> tmp(&fn->getEntryBlock(), fn->getEntryBlock().begin());
    return tmp.CreateAlloca(ty, nullptr, name);
}

Value* CodeGen::toI64(Value* v) {
    if (v->getType()->isIntegerTy(64)) 
        return v;

    cgError("toI64: unsupported type");
    return nullptr;
}

Value* CodeGen::truthy(Value* v) {
    if (v->getType()->isIntegerTy(1)) 
        return v;
    if (!v->getType()->isIntegerTy(32)) 
        cgError("truthy expects i32/i1");
    return builder.CreateICmpNE(v, constI32(0), "truthy");
}

int64_t CodeGen::evalConstDim(Expr* e) {
    if (auto* ie = dynamic_cast<IntExpr*>(e)) {
        return ie->value;
    }

    if (auto* ne = dynamic_cast<NameExpr*>(e)) {
        auto cv = getConst(ne->name);
        if (!cv) 
            cgError("array dim '" + ne->name + "' is not a const");
        return *cv;
    }

    cgError("array dim must be int literal or const identifier");
    return 0;
}

void CodeGen::declareBuiltins() {
    fn_simFillRect = module->getOrInsertFunction(
        "simFillRect",
        FunctionType::get(voidTy(), {i32Ty(), i32Ty(), i32Ty(), i32Ty(), i32Ty()}, false));

    fn_simFlush = module->getOrInsertFunction(
        "simFlush",
        FunctionType::get(voidTy(), {}, false));

    fn_simDelay = module->getOrInsertFunction(
        "simDelay",
        FunctionType::get(voidTy(), {i32Ty()}, false));

    fn_simRand = module->getOrInsertFunction(
        "simRand",
        FunctionType::get(i32Ty(), {}, false));

    fn_simGetTicks = module->getOrInsertFunction(
        "simGetTicks",
        FunctionType::get(i32Ty(), {}, false));

    fn_checkFinish = module->getOrInsertFunction(
        "checkFinish",
        FunctionType::get(i32Ty(), {}, false));

    fn_simPutPixel = module->getOrInsertFunction(
        "simPutPixel",
        FunctionType::get(voidTy(), {i32Ty(), i32Ty(), i32Ty()}, false));

    fn_simGetMouseX = module->getOrInsertFunction(
        "simGetMouseX",
        FunctionType::get(i32Ty(), {}, false));

    fn_simGetMouseY = module->getOrInsertFunction(
        "simGetMouseY",
        FunctionType::get(i32Ty(), {}, false));

    fn_simIsMouseButtonDown = module->getOrInsertFunction(
        "simIsMouseButtonDown",
        FunctionType::get(i32Ty(), {i32Ty()}, false));

    fn_simIsKeyDown = module->getOrInsertFunction(
        "simIsKeyDown",
        FunctionType::get(i32Ty(), {i32Ty()}, false));
}

void CodeGen::declareFunctionPrototype(const FuncDecl& f) {
    if (module->getFunction(f.name)) 
        return;

    Type* retTy = (f.retType == RetType::I32) ? i32Ty() : voidTy();
    auto* fTy = FunctionType::get(retTy, {}, false);
    Function::Create(fTy, Function::ExternalLinkage, f.name, module.get());
}

void CodeGen::codegenFunction(const FuncDecl& f) {
    if (!f.body) 
        cgError("function has no body: " + f.name);
    Function* fn = module->getFunction(f.name);
    if (!fn) 
        cgError("internal: prototype missing for function " + f.name);

    if (!fn->empty())
        cgError("function redefinition: " + f.name);

    BasicBlock* entry = BasicBlock::Create(ctx, "entry", fn);
    builder.SetInsertPoint(entry);

    pushScope();

    f.body->codegen(*this);

    if (!builder.GetInsertBlock()->getTerminator()) {
        if (f.retType == RetType::I32) {
            builder.CreateRet(constI32(0));
        } else {
            builder.CreateRetVoid();
        }
    }

    popScope();
}

Value* CodeGen::addrOfName(const std::string& name) {
    if (auto* vi = findVar(name)) {
        if (vi->dims.empty()) 
            return vi->alloca;
        cgError("cannot take scalar address of array variable '" + name + "' (use indexing)");
    }

    cgError("unknown variable '" + name + "'");
    return nullptr;
}

Value* CodeGen::loadName(const std::string& name) {
    if (auto* vi = findVar(name)) {
        if (!vi->dims.empty()) 
            cgError("array variable '" + name + "' used as scalar (missing indices?)");
        return builder.CreateLoad(i32Ty(), vi->alloca, name + ".ld");
    }
    auto cv = getConst(name);
    if (cv) 
        return constI32(*cv);

    cgError("unknown identifier '" + name + "'");
    return nullptr;
}

Type* CodeGen::arrayTypeFromDims(const std::vector<int64_t>& dims) {
    Type* t = i32Ty();
    for (auto it = dims.rbegin(); it != dims.rend(); ++it) {
        if (*it <= 0) 
            cgError("array dimension must be > 0");
        t = ArrayType::get(t, (uint64_t)*it);
    }
    return t;
}

Value* CodeGen::addrOfIndex(const std::string& base, const std::vector<Expr*>& indices) {
    auto* vi = findVar(base);
    if (!vi) 
        cgError("unknown array '" + base + "'");
    if (vi->dims.empty()) 
        cgError("scalar '" + base + "' used with indices");

    if (indices.size() != vi->dims.size()) {
        cgError("array '" + base + "' expects " + std::to_string(vi->dims.size())
            + " indices, got " + std::to_string(indices.size()));
    }

    std::vector<Value*> gepIdx;
    gepIdx.push_back(constI32(0)); 
    for (auto* e : indices) {
        Value* idx32 = e->codegen(*this);
        if (!idx32->getType()->isIntegerTy(32)) 
            cgError("index must be i32");
        gepIdx.push_back(idx32);
    }

    Type* arrTy = arrayTypeFromDims(vi->dims);
    return builder.CreateInBoundsGEP(arrTy, vi->alloca, gepIdx, base + ".elt");
}

Value* CodeGen::codegenUnary(const std::string& op, Expr* rhsE) {
    Value* rhs = rhsE->codegen(*this);
    if (op == "-") {
        return builder.CreateNeg(rhs, "neg");
    }
    if (op == "!") {
        Value* c = truthy(rhs);
        Value* inv = builder.CreateNot(c, "not");
        return builder.CreateZExt(inv, i32Ty(), "not.i32");
    }
    cgError("unknown unary op: " + op);
    return nullptr;
}

Value* CodeGen::codegenBinary(const std::string& op, Expr* lhsE, Expr* rhsE) {
    Value* a = lhsE->codegen(*this);
    Value* b = rhsE->codegen(*this);

    auto zextBool = [&](Value* i1) -> Value* {
        return builder.CreateZExt(i1, i32Ty(), "bool.i32");
    };

    if (op == "+") return builder.CreateAdd(a, b, "add");
    if (op == "-") return builder.CreateSub(a, b, "sub");
    if (op == "*") return builder.CreateMul(a, b, "mul");
    if (op == "/") return builder.CreateSDiv(a, b, "sdiv");
    if (op == "%") return builder.CreateSRem(a, b, "srem");

    if (op == "==") return zextBool(builder.CreateICmpEQ(a, b, "eq"));
    if (op == "!=") return zextBool(builder.CreateICmpNE(a, b, "ne"));
    if (op == "<")  return zextBool(builder.CreateICmpSLT(a, b, "lt"));
    if (op == "<=") return zextBool(builder.CreateICmpSLE(a, b, "le"));
    if (op == ">")  return zextBool(builder.CreateICmpSGT(a, b, "gt"));
    if (op == ">=") return zextBool(builder.CreateICmpSGE(a, b, "ge"));

    if (op == "&&") {
        Value* ai1 = truthy(a);
        Value* bi1 = truthy(b);
        return zextBool(builder.CreateAnd(ai1, bi1, "and"));
    }
    if (op == "||") {
        Value* ai1 = truthy(a);
        Value* bi1 = truthy(b);
        return zextBool(builder.CreateOr(ai1, bi1, "or"));
    }

    cgError("unknown binary op: " + op);
    return nullptr;
}

Value* CodeGen::codegenCall(const std::string& callee, const std::vector<Expr*>& argsE) {
    auto argVals = [&](size_t n) {
        if (argsE.size() != n) 
            cgError("call '" + callee + "' expects " + std::to_string(n) + " args");

        std::vector<Value*> v;
        v.reserve(n);
        for (auto* e : argsE) 
            v.push_back(e->codegen(*this));
        return v;
    };

    if (callee == "fill_rect") {
        auto v = argVals(5);
        builder.CreateCall(fn_simFillRect, v);
        return Constant::getNullValue(i32Ty());
    }
    if (callee == "flush") {
        auto v = argVals(0);
        builder.CreateCall(fn_simFlush, v);
        return Constant::getNullValue(i32Ty());
    }
    if (callee == "delay") {
        auto v = argVals(1);
        builder.CreateCall(fn_simDelay, v);
        return Constant::getNullValue(i32Ty());
    }
    if (callee == "rand") {
        auto v = argVals(0);
        return builder.CreateCall(fn_simRand, v, "rand");
    }
    if (callee == "ticks") {
        auto v = argVals(0);
        return builder.CreateCall(fn_simGetTicks, v, "ticks");
    }
    if (callee == "finish") {
        auto v = argVals(0);
        return builder.CreateCall(fn_checkFinish, v, "finish");
    }
    if (callee == "put_pixel") {
        auto v = argVals(3);
        builder.CreateCall(fn_simPutPixel, v);
        return Constant::getNullValue(i32Ty());
    }
    if (callee == "mouse_x") {
        auto v = argVals(0);
        return builder.CreateCall(fn_simGetMouseX, v, "mx");
    }
    if (callee == "mouse_y") {
        auto v = argVals(0);
        return builder.CreateCall(fn_simGetMouseY, v, "my");
    }
    if (callee == "mouse_btn") {
        auto v = argVals(1);
        return builder.CreateCall(fn_simIsMouseButtonDown, v, "mb");
    }
    if (callee == "key_down") {
        auto v = argVals(1);
        return builder.CreateCall(fn_simIsKeyDown, v, "kd");
    }

    if (!argsE.empty()) 
        cgError("user function '" + callee + "' currently must be called with 0 args");

    Function* fn = module->getFunction(callee);
    if (!fn) 
        cgError("unknown function '" + callee + "'");
    return builder.CreateCall(fn, {}, callee + ".call");
}

void CodeGen::codegenBlock(BlockStmt& blk) {
    pushScope();
    for (auto* s : blk.stmts) {
        if (builder.GetInsertBlock()->getTerminator()) 
            break;
        s->codegen(*this);
    }
    popScope();
}

void CodeGen::codegenVarDecl(const std::string& name, const std::vector<Expr*>& dimsE, Expr* initE) {
    Function* fn = builder.GetInsertBlock()->getParent();
    if (!fn) 
        cgError("var decl outside function");

    if (scopes.empty()) 
        pushScope();

    auto& cur = scopes.back();
    if (cur.find(name) != cur.end()) 
        cgError("redeclared variable '" + name + "' in same scope");

    VarInfo vi;

    if (dimsE.empty()) {
        vi.alloca = createEntryAlloca(fn, i32Ty(), name);
        vi.dims.clear();
        cur[name] = vi;

        if (initE) {
            Value* v = initE->codegen(*this);
            builder.CreateStore(v, vi.alloca);
        } else {
            builder.CreateStore(constI32(0), vi.alloca);
        }
        return;
    }

    std::vector<int64_t> dims;
    dims.reserve(dimsE.size());
    int64_t totalElems = 1;
    for (auto* de : dimsE) {
        int64_t d = evalConstDim(de);
        dims.push_back(d);
        totalElems *= d;
    }

    Type* arrTy = arrayTypeFromDims(dims);
    vi.alloca = createEntryAlloca(fn, arrTy, name);
    vi.dims = dims;
    cur[name] = vi;

    int64_t bytes = totalElems * 4;
    Value* i8ptr = builder.CreateBitCast(vi.alloca, Type::getInt8PtrTy(ctx), name + ".i8");
    builder.CreateMemSet(i8ptr, builder.getInt8(0), (uint64_t)bytes, MaybeAlign(16));

    if (initE)
        cgError("array initialization is not supported (init on var a[...])");
}

void CodeGen::codegenAssign(Expr* lhsE, Expr* rhsE) {
    Value* ptr = lhsE->codegenPtr(*this);

    if (!ptr) 
        cgError("left side of assignment is not an lvalue");

    Value* v = rhsE->codegen(*this);
    builder.CreateStore(v, ptr);
}

void CodeGen::codegenIf(Expr* condE, BlockStmt* thenB, BlockStmt* elseB) {
    Function* fn = builder.GetInsertBlock()->getParent();
    Value* cond = truthy(condE->codegen(*this));

    BasicBlock* thenBB = BasicBlock::Create(ctx, "if.then", fn);
    BasicBlock* mergeBB = BasicBlock::Create(ctx, "if.end", fn);
    BasicBlock* elseBB = elseB ? BasicBlock::Create(ctx, "if.else", fn) : mergeBB;

    builder.CreateCondBr(cond, thenBB, elseBB);

    builder.SetInsertPoint(thenBB);
    thenB->codegen(*this);
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(mergeBB);

    if (elseB) {
        builder.SetInsertPoint(elseBB);
        elseB->codegen(*this);
        if (!builder.GetInsertBlock()->getTerminator())
            builder.CreateBr(mergeBB);
    }

    builder.SetInsertPoint(mergeBB);
}

void CodeGen::codegenWhile(Expr* condE, BlockStmt* bodyB) {
    Function* fn = builder.GetInsertBlock()->getParent();

    BasicBlock* condBB = BasicBlock::Create(ctx, "while.cond", fn);
    BasicBlock* bodyBB = BasicBlock::Create(ctx, "while.body", fn);
    BasicBlock* endBB  = BasicBlock::Create(ctx, "while.end", fn);

    builder.CreateBr(condBB);

    builder.SetInsertPoint(condBB);
    Value* cond = truthy(condE->codegen(*this));
    builder.CreateCondBr(cond, bodyBB, endBB);

    builder.SetInsertPoint(bodyBB);
    loopStack.push_back({endBB, condBB});
    bodyB->codegen(*this);
    loopStack.pop_back();

    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(condBB);

    builder.SetInsertPoint(endBB);
}

void CodeGen::codegenFor(Stmt* initS, Expr* condE, Stmt* stepS, BlockStmt* bodyB) {
    Function* fn = builder.GetInsertBlock()->getParent();

    pushScope(); 
    if (initS) initS->codegen(*this);

    BasicBlock* condBB = BasicBlock::Create(ctx, "for.cond", fn);
    BasicBlock* bodyBB = BasicBlock::Create(ctx, "for.body", fn);
    BasicBlock* stepBB = BasicBlock::Create(ctx, "for.step", fn);
    BasicBlock* endBB  = BasicBlock::Create(ctx, "for.end",  fn);

    builder.CreateBr(condBB);

    builder.SetInsertPoint(condBB);
    Value* cond = nullptr;
    if (condE) cond = truthy(condE->codegen(*this));
    else cond = builder.getTrue();
    builder.CreateCondBr(cond, bodyBB, endBB);

    builder.SetInsertPoint(bodyBB);
    loopStack.push_back({endBB, stepBB});
    bodyB->codegen(*this);
    loopStack.pop_back();
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(stepBB);

    builder.SetInsertPoint(stepBB);
    if (stepS) stepS->codegen(*this);
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(condBB);

    builder.SetInsertPoint(endBB);
    popScope();
}

void CodeGen::codegenBreak() {
    if (loopStack.empty()) cgError("break outside loop");
    builder.CreateBr(loopStack.back().breakBB);

    Function* fn = builder.GetInsertBlock()->getParent();
    BasicBlock* after = BasicBlock::Create(ctx, "after.break", fn);
    builder.SetInsertPoint(after);
}

void CodeGen::codegenContinue() {
    if (loopStack.empty()) cgError("continue outside loop");
    builder.CreateBr(loopStack.back().continueBB);
    Function* fn = builder.GetInsertBlock()->getParent();
    BasicBlock* after = BasicBlock::Create(ctx, "after.continue", fn);
    builder.SetInsertPoint(after);
}

void CodeGen::codegenReturn(Expr* valueE) {
    Function* fn = builder.GetInsertBlock()->getParent();
    if (!fn) 
        cgError("return outside function");
    Type* retTy = fn->getReturnType();

    if (retTy->isVoidTy()) {
        if (valueE) 
            cgError("return with value in void function");
        builder.CreateRetVoid();
    } else {
        if (!valueE) 
            cgError("return without value in i32 function");
        Value* v = valueE->codegen(*this);
        builder.CreateRet(v);
    }

    BasicBlock* after = BasicBlock::Create(ctx, "after.return", fn);
    builder.SetInsertPoint(after);
}
