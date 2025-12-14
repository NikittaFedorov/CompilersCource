#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>

#include "ast.hpp"
#include "codegen.hpp"
#include "sim.h"

extern int yyparse();
extern FILE* yyin;
extern lifelang::Program* gProgram;

static void usage(const char* argv0) {
    std::cerr
        << "Usage: " << argv0 << " <file.lf> [options]\n"
        << "Options:\n"
        << "  -o <file.ll>     write LLVM IR to file\n"
        << "  --no-run         only compile, do not execute\n"
        << "  --no-sdl         do not call simInit/simExit around entry function\n"
        << "  --entry <name>   entry function to run (default: main if exists else app)\n";
}

int main(int argc, char** argv) {
    if (argc < 2) { usage(argv[0]); return 2; }

    std::string inputPath;
    std::string outLl;
    bool doRun = true;
    bool useSDL = true;
    std::string entryName;

    inputPath = argv[1];
    for (int i = 2; i < argc; i++) {
        std::string a = argv[i];
        if (a == "-o" && i + 1 < argc) {
            outLl = argv[++i];
        } else if (a == "--no-run") {
            doRun = false;
        } else if (a == "--no-sdl") {
            useSDL = false;
        } else if (a == "--entry" && i + 1 < argc) {
            entryName = argv[++i];
        } else {
            usage(argv[0]);
            return 2;
        }
    }

    yyin = std::fopen(inputPath.c_str(), "rb");
    if (!yyin) {
        std::perror(("cannot open " + inputPath).c_str());
        return 2;
    }
    if (yyparse() != 0 || !gProgram) {
        std::cerr << "Parse failed\n";
        return 1;
    }
    std::fclose(yyin);

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    LLVMLinkInMCJIT();
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

    lifelang::CodeGen cg;
    gProgram->codegen(cg);

    if (llvm::verifyModule(*cg.module, &llvm::errs())) {
        llvm::errs() << "[VERIFY] FAIL\n";
        return 1;
    }

    if (!outLl.empty()) {
        std::error_code ec;
        llvm::raw_fd_ostream out(outLl, ec);
        if (ec) {
            llvm::errs() << "Cannot write IR to " << outLl << ": " << ec.message() << "\n";
            return 1;
        }
        cg.module->print(out, nullptr);
    }

    if (!doRun) return 0;

    if (entryName.empty()) {
        if (cg.module->getFunction("main")) entryName = "main";
        else entryName = "app";
    }

    llvm::Function* entry = cg.module->getFunction(entryName);
    if (!entry) {
        llvm::errs() << "Entry function '" << entryName << "' not found\n";
        return 1;
    }
    if (!entry->arg_empty()) {
        llvm::errs() << "Entry function must take 0 args\n";
        return 1;
    }

    std::string errStr;
    llvm::EngineBuilder eb(std::move(cg.module));
    eb.setEngineKind(llvm::EngineKind::JIT);
    eb.setErrorStr(&errStr);
    llvm::ExecutionEngine* ee = eb.create();
    if (!ee) {
        llvm::errs() << "Failed to create ExecutionEngine: " << errStr << "\n";
        return 1;
    }

    ee->InstallLazyFunctionCreator([&](const std::string& fnName) -> void* {
        if (fnName == "simInit") return (void*)simInit;
        if (fnName == "simExit") return (void*)simExit;
        if (fnName == "simFlush") return (void*)simFlush;
        if (fnName == "simPutPixel") return (void*)simPutPixel;
        if (fnName == "simFillRect") return (void*)simFillRect;
        if (fnName == "simRand") return (void*)simRand;
        if (fnName == "simGetTicks") return (void*)simGetTicks;
        if (fnName == "simDelay") return (void*)simDelay;
        if (fnName == "checkFinish") return (void*)checkFinish;
        if (fnName == "simGetMouseX") return (void*)simGetMouseX;
        if (fnName == "simGetMouseY") return (void*)simGetMouseY;
        if (fnName == "simIsMouseButtonDown") return (void*)simIsMouseButtonDown;
        if (fnName == "simIsKeyDown") return (void*)simIsKeyDown;
        return nullptr;
    });

    ee->finalizeObject();

    if (useSDL) simInit();

    std::vector<llvm::GenericValue> noargs;
    llvm::GenericValue rv = ee->runFunction(entry, noargs);

    if (useSDL) simExit();

    if (entry->getReturnType()->isIntegerTy(32)) {
        auto v = rv.IntVal.getSExtValue();
        return (int)v;
    }
    return 0;
}
