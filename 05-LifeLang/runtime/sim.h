#ifndef LIFELANG_SIM_H
#define LIFELANG_SIM_H

void simInit();
void simExit();
void simFlush();
void simPutPixel(int x, int y, int rgb);
void simFillRect(int x, int y, int w, int h, int rgb);
int  simRand();
int  simGetTicks();
void simDelay(int ms);
int  checkFinish();
int  simGetMouseX();
int  simGetMouseY();
int  simIsMouseButtonDown(int button);
int  simIsKeyDown(int scancode);

#endif
