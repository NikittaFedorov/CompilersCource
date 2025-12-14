#include "sim.h"
#include <stdlib.h>
#include <time.h>

void simInit() { srand((unsigned)time(NULL)); }
void simExit() { }
void simFlush() { }
void simPutPixel(int x, int y, int rgb) { (void)x; (void)y; (void)rgb; }
void simFillRect(int x, int y, int w, int h, int rgb) { (void)x; (void)y; (void)w; (void)h; (void)rgb; }
int  simRand() { return rand(); }
int  simGetTicks() { return 0; }
void simDelay(int ms) { (void)ms; }
int  checkFinish() { return 1; }
int  simGetMouseX() { return 0; }
int  simGetMouseY() { return 0; }
int  simIsMouseButtonDown(int button) { (void)button; return 0; }
int  simIsKeyDown(int scancode) { (void)scancode; return 0; }
