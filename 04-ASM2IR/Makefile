CXX = clang++
LLVM_FLAGS = `llvm-config --cppflags --ldflags --system-libs --libs core mcjit native`
SDL_FLAGS = -lSDL2

all: asm2ir_emulator

asm2ir_emulator: isa_definition.cpp asm2ir_emulator.cpp sim.c
	$(CXX) $(LLVM_FLAGS) $(SDL_FLAGS) $^ -o $@

clean:
	rm -f asm2ir_emulator

run: asm2ir_emulator
	./asm2ir_emulator game_of_life.asm