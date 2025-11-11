CXX = clang++
CXXFLAGS = -g -O0 -std=c++17 -I.
LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`
SDL_FLAGS = -lSDL2

all: interpreter asm2ir

interpreter: interpreter.cpp sim.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SDL_FLAGS)

asm2ir: asm2ir.cpp sim.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LLVM_FLAGS) $(SDL_FLAGS)

sim.o: sim.c sim.h
	$(CC) -c -o $@ $< $(SDL_FLAGS)

clean:
	rm -f interpreter asm2ir sim.o *.ll

test_interpreter: interpreter
	./interpreter game_life.asm

test_ir: asm2ir
	./asm2ir game_life.asm game_life.ll

test: test_interpreter test_ir

.PHONY: all clean test test_interpreter test_ir