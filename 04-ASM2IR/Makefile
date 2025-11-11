CXX = clang++
CXXFLAGS = -g -O0 -std=c++17 -I.
LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core executionengine mcjit interpreter analysis native`
SDL_FLAGS = -lSDL2

all: asm2ir_emulator

asm2ir_emulator: asm2ir_emulator.cpp sim.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LLVM_FLAGS) $(SDL_FLAGS)

sim.o: sim.c sim.h
	$(CC) -c -o $@ $< $(SDL_FLAGS)

clean:
	rm -f asm2ir_emulator sim.o *.ll

test: asm2ir_emulator
	./asm2ir_emulator game_of_life.asm game_of_life.ll

run: asm2ir_emulator
	./asm2ir_emulator game_of_life.asm game_of_life.ll

.PHONY: all clean test run