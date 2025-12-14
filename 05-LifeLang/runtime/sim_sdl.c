#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <time.h>
#include "sim.h"

#define FIELD_WIDTH 64
#define FIELD_HEIGHT 64
#define CELL_SIZE 4

#define SIM_Y_SIZE (FIELD_HEIGHT * CELL_SIZE)
#define SIM_X_SIZE (FIELD_WIDTH * CELL_SIZE)

static SDL_Renderer *Renderer = NULL;
static SDL_Window *Window = NULL;

void simInit() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SIM_X_SIZE, SIM_Y_SIZE, 0, &Window, &Renderer);
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
    SDL_RenderClear(Renderer);
    srand((unsigned)time(NULL));
    simFlush();
}

void simExit() {
    SDL_Event event;
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        SDL_Delay(10);
    }
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    SDL_Quit();
}

void simFlush() {
    SDL_PumpEvents();
    SDL_RenderPresent(Renderer);
}

void simPutPixel(int x, int y, int rgb) {
    assert(0 <= x && x < SIM_X_SIZE);
    assert(0 <= y && y < SIM_Y_SIZE);
    Uint8 r = (rgb >> 16) & 0xFF;
    Uint8 g = (rgb >> 8) & 0xFF;
    Uint8 b = rgb & 0xFF;
    SDL_SetRenderDrawColor(Renderer, r, g, b, 255);
    SDL_RenderDrawPoint(Renderer, x, y);
}

void simFillRect(int x, int y, int w, int h, int rgb) {
    assert(0 <= x && x + w <= SIM_X_SIZE);
    assert(0 <= y && y + h <= SIM_Y_SIZE);
    Uint8 r = (rgb >> 16) & 0xFF;
    Uint8 g = (rgb >> 8) & 0xFF;
    Uint8 b = rgb & 0xFF;
    SDL_SetRenderDrawColor(Renderer, r, g, b, 255);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(Renderer, &rect);
}

int simRand() { return rand(); }
int simGetTicks() { return (int)SDL_GetTicks(); }
void simDelay(int ms) { SDL_Delay((Uint32)ms); }

int checkFinish() {
    if (SDL_HasEvent(SDL_QUIT) != SDL_TRUE)
        return 0;
    return 1;
}

int simGetMouseX() { int x, y; SDL_GetMouseState(&x, &y); return x; }
int simGetMouseY() { int x, y; SDL_GetMouseState(&x, &y); return y; }
int simIsMouseButtonDown(int button) { int x, y; return SDL_GetMouseState(&x, &y) & SDL_BUTTON(button); }
int simIsKeyDown(int scancode) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    return state[scancode];
}
