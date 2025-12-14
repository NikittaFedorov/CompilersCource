CXX ?= clang++
BISON ?= bison
FLEX ?= flex

LLVM_FLAGS := $(shell llvm-config --cppflags --ldflags --libs)
CXXFLAGS ?= -std=c++17 -O2 -g -Wall -Wextra -Wno-unused-parameter
INCLUDES := -Isrc -Iruntime

SDL_FLAGS := -lSDL2

GEN_DIR := build
SRC_DIR := src
RT_DIR  := runtime

PARSER_C := $(GEN_DIR)/parser.tab.c
PARSER_H := $(GEN_DIR)/parser.tab.h
LEXER_C  := $(GEN_DIR)/lex.yy.c

COMMON_CPP := $(SRC_DIR)/ast.cpp $(SRC_DIR)/codegen.cpp
DRIVER_CPP := $(SRC_DIR)/main.cpp

all: life_sdl

$(GEN_DIR):
	mkdir -p $(GEN_DIR)

$(PARSER_C) $(PARSER_H): $(SRC_DIR)/parser.y | $(GEN_DIR)
	$(BISON) -d -o $(PARSER_C) $(SRC_DIR)/parser.y

$(LEXER_C): $(SRC_DIR)/lexer.l $(PARSER_H) | $(GEN_DIR)
	$(FLEX) -o $(LEXER_C) $(SRC_DIR)/lexer.l

lifec: $(DRIVER_CPP) $(COMMON_CPP) $(PARSER_C) $(LEXER_C) $(RT_DIR)/sim_stub.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LLVM_FLAGS) -o $@

life_sdl: $(DRIVER_CPP) $(COMMON_CPP) $(PARSER_C) $(LEXER_C) $(RT_DIR)/sim_sdl.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LLVM_FLAGS) $(SDL_FLAGS) -o $@

test: lifec
	bash scripts/run_tests.sh

clean:
	rm -rf $(GEN_DIR) lifec life_sdl *.ll

.PHONY: all clean test
