/*
 * display.h
 *
 * Authors: Luke Kamols, Jarrod Bennett
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "pixel_colour.h"

// display dimensions, these match the size of the board
#define WIDTH  5
#define HEIGHT 5

// offset for the LED matrix, since our board is offset from the edge of the
// LED matrix
#define MATRIX_X_OFFSET 5
#define MATRIX_Y_OFFSET 2

// object definitions
#define EMPTY_SQUARE    0
#define PLAYER_1		1
#define PLAYER_2		2
#define CURSOR			3
#define PREVIOUS		4
#define LEGAL			5

// matrix colour definitions
#define MATRIX_COLOUR_EMPTY		COLOUR_BLACK
#define MATRIX_COLOUR_P1		COLOUR_GREEN
#define MATRIX_COLOUR_P2		COLOUR_RED
#define MATRIX_COLOUR_CURSOR	COLOUR_ORANGE
#define MATRIX_COLOUR_BG		COLOUR_LIGHT_YELLOW 
#define MATRIX_COLOUR_LEGAL		COLOUR_LIGHT_GREEN

// initialise the display for the board, this creates the display
// for an empty board
void initialise_display(void);

// shows a starting display
void start_display(void);

// updates the colour at square (x, y) to be the colour
// of the object 'object'
// 'object' is expected to be EMPTY_SQUARE, PLAYER_1, PLAYER_2 or CURSOR
void update_square_colour(uint8_t x, uint8_t y, uint8_t object);


#endif /* DISPLAY_H_ */