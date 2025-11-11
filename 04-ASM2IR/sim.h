/*
 * Width and height of the field are set in cells.
 * Changing these values will change the size of the field in cells.
 * The actual size of the simulation window will be CELL_SIZE times larger.
 */
#define FIELD_WIDTH 64
#define FIELD_HEIGHT 64

/*
 * Size of a cell in pixels. Changing this value will change the size of the simulation window.
 * For example, if CELL_SIZE is 2, each cell will be represented by a 2x2 pixel square.
 */
#define CELL_SIZE 4

/*
 * If EDITOR_MODE is set to 1, the simulation starts in editing mode,
 * allowing you to set the initial state of the field using the mouse.
 * Press space to start the simulation.
 */
#define EDITOR_MODE 0

/*
 * If RANDOM_INIT is set to 1, the field is initialized with a random distribution of alive cells.
 */
#define RANDOM_INIT 1

/*
 * Time in milliseconds between generations. Lower values make the simulation run faster.
 */
#define MS_PER_GENERATION 10

/*
 * Probability (in percent) of a cell being alive at the start if RANDOM_INIT is enabled.
 */
#define ALIVE_PROB 30

#define ALIVE_COLOR1 0xC71585
#define ALIVE_COLOR2 0x00FF00
#define DEAD_COLOR 0x000000

#define DEAD 0
#define ALIVE1 1
#define ALIVE2 2

#ifndef __sim__
#define __sim__
void simInit();
void app();
void simExit();
void simFlush();
void simFillRect(int x, int y, int w, int h, int rgb);
int simRand();
int simGetTicks();
void simDelay(int ms);
int checkFinish();
#endif