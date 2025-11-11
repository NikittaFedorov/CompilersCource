#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <stdio.h>
#include "sim.h"

#define SIM_Y_SIZE (FIELD_HEIGHT * CELL_SIZE)
#define SIM_X_SIZE (FIELD_WIDTH * CELL_SIZE)

static SDL_Renderer *Renderer = NULL;
static SDL_Window *Window = NULL;
static int should_exit = 0;

void simInit() {
    printf("SDL_Init...\n");
    SDL_Init(SDL_INIT_VIDEO);
    printf("Creating window...\n");
    SDL_CreateWindowAndRenderer(SIM_X_SIZE, SIM_Y_SIZE, 0, &Window, &Renderer);
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
    SDL_RenderClear(Renderer);
    srand(time(NULL));
    printf("Window created: %dx%d\n", SIM_X_SIZE, SIM_Y_SIZE);
    simFlush();
}

void simExit() {
    printf("Exiting...\n");
    should_exit = 1;
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    SDL_Quit();
    exit(0);
}

void simFlush() {
    SDL_PumpEvents();
    SDL_RenderPresent(Renderer);
}

void simFillRect(int x, int y, int w, int h, int rgb) {
    // printf("Drawing at (%d, %d) color: 0x%06X\n", x, y, rgb);
    assert(0 <= x && x + w <= SIM_X_SIZE);
    assert(0 <= y && y + h <= SIM_Y_SIZE);
    Uint8 r = (rgb >> 16) & 0xFF;
    Uint8 g = (rgb >> 8) & 0xFF;
    Uint8 b = rgb & 0xFF;
    SDL_SetRenderDrawColor(Renderer, r, g, b, 255);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(Renderer, &rect);
}

int simRand() {
    return rand();
}

int simGetTicks() {
    return SDL_GetTicks();
}

void simDelay(int ms) {
    SDL_Delay(ms);
}

int checkFinish() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return 1;
        }
    }
    return should_exit;
}