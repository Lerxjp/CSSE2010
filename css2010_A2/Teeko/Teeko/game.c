/*
 * game.c
 *
 * Contains functions relating to the play of the game Teeko
 *
 * Authors: Luke Kamols, Jarrod Bennett
 */ 

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "display.h"
#include "terminalio.h"
#include "serialio.h"
#include "terminalio.h"

// Start pieces in the middle of the board
#define CURSOR_X_START ((int)(WIDTH/2))
#define CURSOR_Y_START ((int)(HEIGHT/2))
int green_count = 0;
int red_count = 0;
int row;
int column;
uint8_t board[WIDTH][HEIGHT];
// cursor coordinates should be /* SIGNED */ to allow left and down movement.
// All other positions should be unsigned as there are no negative coordinates.
int8_t cursor_x;
int8_t cursor_y;
uint8_t cursor_visible;
uint8_t current_player;
uint8_t count = 0;
uint8_t x = 8;
uint8_t y = 8;
int picked = 0;
void initialise_game(void) {
	
	// initialise the display we are using
	initialise_display();
	
	// initialise the board to be all empty
	for (uint8_t x = 0; x < WIDTH; x++) {
		for (uint8_t y = 0; y < HEIGHT; y++) {
			board[x][y] = EMPTY_SQUARE;
		}
	}
	
	// set the starting player
	current_player = PLAYER_1;

	// also set where the cursor starts
	cursor_x = CURSOR_X_START;
	cursor_y = CURSOR_Y_START;
	cursor_visible = 0;
	count = 0;
}

uint8_t get_piece_at(uint8_t x, uint8_t y) {
	// check the bounds, anything outside the bounds
	// will be considered empty
	if (x < 0 || x >= WIDTH || y < 0 || y >= WIDTH) {
		return EMPTY_SQUARE;
	} else {
		//if in the bounds, just index into the array
		return board[x][y];
	}
}

void flash_cursor(void) {
	if (cursor_visible) {
		// we need to flash the cursor off, it should be replaced by
		// the colour of the piece which is at that location
		uint8_t piece_at_cursor = get_piece_at(cursor_x, cursor_y);
		update_square_colour(cursor_x, cursor_y, piece_at_cursor);
		
	} else {
		update_square_colour(cursor_x, cursor_y, CURSOR);
	}
	cursor_visible = 1 - cursor_visible; //alternate between 0 and 1
}
uint8_t get_picked(void) {
	return picked;
}
void change_player(void) {
	if (current_player == PLAYER_1) {
		move_terminal_cursor(30,7);
		printf_P(PSTR("                          "));
		move_terminal_cursor(30,7);
		printf_P(PSTR("Current Player: 2 (red)"));
		current_player = PLAYER_2;
	} else {
		move_terminal_cursor(30,7);
		printf_P(PSTR("                          "));
		move_terminal_cursor(30,7);
		printf_P(PSTR("Current Player: 1 (green)"));
		current_player = PLAYER_1;
	}
}

 bool check_empty(void) {
	if (board[cursor_x][cursor_y] == EMPTY_SQUARE) {
		return true;
	} else if (board[cursor_x][cursor_y] == board[x][y]) {
		return false;
	}
	return false;
}

void make_legal(void) {
	if (board[x+1][y] == EMPTY_SQUARE && x+1 < 5) {
		board[x+1][y] = LEGAL;
		update_square_colour(x+1, y, LEGAL);
	}
	if (board[x-1][y] == EMPTY_SQUARE && x-1 > -1) {
		board[x-1][y] = LEGAL;
		update_square_colour(x-1, y, LEGAL);
	}
	if (board[x+1][y+1] == EMPTY_SQUARE && x+1 < 5 && y+1 < 5) {
		board[x+1][y+1] = LEGAL;
		update_square_colour(x+1, y+1, LEGAL);
	}
	if (board[x+1][y-1] == EMPTY_SQUARE && x+1 < 5 && y-1 > -1) {
		board[x+1][y-1] = LEGAL;
		update_square_colour(x+1, y-1, LEGAL);
	}
	if (board[x][y+1] == EMPTY_SQUARE && y+1 < 5) {
		board[x][y+1] = LEGAL;
		update_square_colour(x, y+1, LEGAL);
	}
	if (board[x][y-1] == EMPTY_SQUARE && y-1 > -1) {
		board[x][y-1] = LEGAL;
		update_square_colour(x, y-1, LEGAL);
	}
	if (board[x-1][y+1] == EMPTY_SQUARE && x-1 > -1 && y+1 < 5) {
		board[x-1][y+1] = LEGAL;
		update_square_colour(x-1, y+1, LEGAL);
	}
	if (board[x-1][y-1] == EMPTY_SQUARE && x-1 > -1 && y-1 > -1) {
		board[x-1][y-1] = LEGAL;
		update_square_colour(x-1, y-1, LEGAL);
	}	
}
void remove_legal(void) {
	if (board[x+1][y] == LEGAL) {
		board[x+1][y] = EMPTY_SQUARE;
		update_square_colour(x+1, y, EMPTY_SQUARE);
	}
	if (board[x-1][y] == LEGAL) {
		board[x-1][y] = EMPTY_SQUARE;
		update_square_colour(x-1, y, EMPTY_SQUARE);
	}
	if (board[x+1][y+1] == LEGAL) {
		board[x+1][y+1] = EMPTY_SQUARE;
		update_square_colour(x+1, y+1, EMPTY_SQUARE);
	}
	if (board[x+1][y-1] == LEGAL) {
		board[x+1][y-1] = EMPTY_SQUARE;
		update_square_colour(x+1, y-1, EMPTY_SQUARE);
	}
	if (board[x][y+1] == LEGAL) {
		board[x][y+1] = EMPTY_SQUARE;
		update_square_colour(x, y+1, EMPTY_SQUARE);
	}
	if (board[x][y-1] == LEGAL) {
		board[x][y-1] = EMPTY_SQUARE;
		update_square_colour(x, y-1, EMPTY_SQUARE);
	}
	if (board[x-1][y+1] == LEGAL) {
		board[x-1][y+1] = EMPTY_SQUARE;
		update_square_colour(x-1, y+1, EMPTY_SQUARE);
	}
	if (board[x-1][y-1] == LEGAL) {
		board[x-1][y-1] = EMPTY_SQUARE;
		update_square_colour(x-1, y-1, EMPTY_SQUARE);
	}
}

bool Check_LEGAL(void) {
	if(board[cursor_x][cursor_y] == LEGAL) {
		return true;
	} else {
		return false;
	}
	return false;
}

void place_piece(void) {
	if (check_empty() == true && count < 8 && board[cursor_x][cursor_y] != PREVIOUS) {
		board[cursor_x][cursor_y] = current_player;
		update_square_colour(cursor_x, cursor_y, current_player);
		change_player();
		board[x][y] = EMPTY_SQUARE;
		count+=1;
	}
	if (count >= 8 && board[cursor_x][cursor_y] == current_player && picked == 0) {
		x = cursor_x;
		y = cursor_y;
		board[cursor_x][cursor_y] = PREVIOUS;
		picked = 1;
		make_legal();
	}
	if (board[cursor_x][cursor_y] == LEGAL) {
		board[cursor_x][cursor_y] = current_player;
		picked = 0;
		update_square_colour(cursor_x, cursor_y, current_player);
		change_player();
		board[x][y] = EMPTY_SQUARE;
		remove_legal();
	}
}

//check the header file game.h for a description of what this function should do
// (it may contain some hints as to how to move the cursor)
void move_display_cursor(int8_t dx, int8_t dy) {
	//YOUR CODE HERE
	/*suggestions for implementation:
	 * 1: remove the display of the cursor at the current location
	 *		(and replace it with whatever piece is at that location)
	 * 2: update the positional knowledge of the cursor, this will include
	 *		variables cursor_x, cursor_y and cursor_visible. Make sure you
	 *		consider what should happen if the cursor moves off the board.
	 * 3: display the cursor at the new location
	 * 4: reset the cursor flashing cycle. See project.c for how the cursor
	 *		is flashed.
	 */
	uint8_t piece_at_cursor = get_piece_at(cursor_x, cursor_y);
	update_square_colour(cursor_x, cursor_y, piece_at_cursor);
	cursor_x += dx;
	cursor_y += dy;
	if (cursor_x == 5) {
		cursor_x = 0;
	} else if (cursor_y == 5) {
		cursor_y = 0;
	} else if (cursor_x == -1) {
		cursor_x = 4;
	} else if (cursor_y == -1) {
		cursor_y = 4;
	}
	update_square_colour(cursor_x, cursor_y, CURSOR);
}
// check player 1
bool check_horizontal(void) {
		// from left
		if(board[0][0] == PLAYER_1) {
			if (board[1][0] == PLAYER_1 && board[2][0] == PLAYER_1 && board[3][0] == PLAYER_1) {
				return true;
			}
		}
		if(board[0][1] == PLAYER_1) {
			if (board[1][1] == PLAYER_1 && board[2][1] == PLAYER_1 && board[3][1] == PLAYER_1) {
				return true;
			}
		}
		if(board[0][2] == PLAYER_1) {
			if (board[1][2] == PLAYER_1 && board[2][2] == PLAYER_1 && board[3][2] == PLAYER_1) {
				return true;
			}
		}
		if(board[0][3] == PLAYER_1) {
			if (board[1][3] == PLAYER_1 && board[2][3] == PLAYER_1 && board[3][3] == PLAYER_1) {
				return true;
			}
		}
		if(board[0][4] == PLAYER_1) {
			if (board[1][4] == PLAYER_1 && board[2][4] == PLAYER_1 && board[3][4] == PLAYER_1) {
				return true;
			}
		}
		//from middle left and middle right
		if(board[1][0] == PLAYER_1) {
			if (board[2][0] == PLAYER_1 && board[3][0] == PLAYER_1 && board[4][0] == PLAYER_1) {
				return true;
			}
		}
		if(board[1][1] == PLAYER_1) {
			if (board[2][1] == PLAYER_1 && board[3][1] == PLAYER_1 && board[4][1] == PLAYER_1) {
				return true;
			}
		}
		if(board[1][2] == PLAYER_1) {
			if (board[2][2] == PLAYER_1 && board[3][2] == PLAYER_1 && board[4][2] == PLAYER_1) {
				return true;
			}
		}
		if(board[1][3] == PLAYER_1) {
			if (board[2][3] == PLAYER_1 && board[3][3] == PLAYER_1 && board[4][3] == PLAYER_1) {
				return true;
			}
		}
		if(board[1][4] == PLAYER_1) {
			if (board[2][4] == PLAYER_1 && board[3][4] == PLAYER_1 && board[4][4] == PLAYER_1) {
				return true;
			}
		}
		// from right
		if(board[4][0] == PLAYER_1) {
			if (board[3][0] == PLAYER_1 && board[2][0] == PLAYER_1 && board[1][0] == PLAYER_1) {
				return true;
			}
		}
		if(board[1][1] == PLAYER_1) {
			if (board[3][1] == PLAYER_1 && board[2][1] == PLAYER_1 && board[1][1] == PLAYER_1) {
				return true;
			}
		}
		if(board[1][2] == PLAYER_1) {
			if (board[3][2] == PLAYER_1 && board[2][2] == PLAYER_1 && board[1][2] == PLAYER_1) {
				return true;
			}
		}
		if(board[1][3] == PLAYER_1) {
			if (board[3][3] == PLAYER_1 && board[2][3] == PLAYER_1 && board[1][3] == PLAYER_1) {
				return true;
			}
		}
		if(board[1][4] == PLAYER_1) {
			if (board[3][4] == PLAYER_1 && board[2][4] == PLAYER_1 && board[1][4] == PLAYER_1) {
				return true;
			}
		}

	return false;
}

bool check_horizontal_red(void) {
	// from left
	if(board[0][0] == PLAYER_2) {
		if (board[1][0] == PLAYER_2 && board[2][0] == PLAYER_2 && board[3][0] == PLAYER_2) {
			return true;
		}
	}
	if(board[0][1] == PLAYER_2) {
		if (board[1][1] == PLAYER_2 && board[2][1] == PLAYER_2 && board[3][1] == PLAYER_2) {
			return true;
		}
	}
	if(board[0][2] == PLAYER_2) {
		if (board[1][2] == PLAYER_2 && board[2][2] == PLAYER_2 && board[3][2] == PLAYER_2) {
			return true;
		}
	}
	if(board[0][3] == PLAYER_2) {
		if (board[1][3] == PLAYER_2 && board[2][3] == PLAYER_2 && board[3][3] == PLAYER_2) {
			return true;
		}
	}
	if(board[0][4] == PLAYER_2) {
		if (board[1][4] == PLAYER_2 && board[2][4] == PLAYER_2 && board[3][4] == PLAYER_2) {
			return true;
		}
	}
	//from middle left and middle right
	if(board[1][0] == PLAYER_2) {
		if (board[2][0] == PLAYER_2 && board[3][0] == PLAYER_2 && board[4][0] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][1] == PLAYER_2) {
		if (board[2][1] == PLAYER_2 && board[3][1] == PLAYER_2 && board[4][1] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][2] == PLAYER_2) {
		if (board[2][2] == PLAYER_2 && board[3][2] == PLAYER_2 && board[4][2] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][3] == PLAYER_2) {
		if (board[2][3] == PLAYER_2 && board[3][3] == PLAYER_2 && board[4][3] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][4] == PLAYER_2) {
		if (board[2][4] == PLAYER_2 && board[3][4] == PLAYER_2 && board[4][4] == PLAYER_2) {
			return true;
		}
	}
	// from right
	if(board[4][0] == PLAYER_2) {
		if (board[3][0] == PLAYER_2 && board[2][0] == PLAYER_2 && board[1][0] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][1] == PLAYER_2) {
		if (board[3][1] == PLAYER_2 && board[2][1] == PLAYER_2 && board[1][1] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][2] == PLAYER_2) {
		if (board[3][2] == PLAYER_2 && board[2][2] == PLAYER_2 && board[1][2] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][3] == PLAYER_2) {
		if (board[3][3] == PLAYER_2 && board[2][3] == PLAYER_2 && board[1][3] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][4] == PLAYER_2) {
		if (board[3][4] == PLAYER_2 && board[2][4] == PLAYER_2 && board[1][4] == PLAYER_2) {
			return true;
		}
	}
	return false;
}

bool check_vertical(void) {
	// from left
	if(board[0][0] == PLAYER_1) {
		if (board[0][1] == PLAYER_1 && board[0][2] == PLAYER_1 && board[0][3] == PLAYER_1) {
			return true;
		}
	}
	if(board[1][0] == PLAYER_1) {
		if (board[1][1] == PLAYER_1 && board[1][2] == PLAYER_1 && board[1][3] == PLAYER_1) {
			return true;
		}
	}
	if(board[2][0] == PLAYER_1) {
		if (board[2][1] == PLAYER_1 && board[2][2] == PLAYER_1 && board[2][3] == PLAYER_1) {
			return true;
		}
	}
	if(board[3][0] == PLAYER_1) {
		if (board[3][1] == PLAYER_1 && board[3][2] == PLAYER_1 && board[3][3] == PLAYER_1) {
			return true;
		}
	}
	if(board[4][0] == PLAYER_1) {
		if (board[4][1] == PLAYER_1 && board[4][2] == PLAYER_1 && board[4][3] == PLAYER_1) {
			return true;
		}
	}
	//from middle left and middle right
	if(board[0][1] == PLAYER_1) {
		if (board[0][2] == PLAYER_1 && board[0][3] == PLAYER_1 && board[0][4] == PLAYER_1) {
			return true;
		}
	}
	if(board[1][1] == PLAYER_1) {
		if (board[1][2] == PLAYER_1 && board[1][3] == PLAYER_1 && board[1][4] == PLAYER_1) {
			return true;
		}
	}
	if(board[2][1] == PLAYER_1) {
		if (board[2][2] == PLAYER_1 && board[2][3] == PLAYER_1 && board[2][4] == PLAYER_1) {
			return true;
		}
	}
	if(board[3][1] == PLAYER_1) {
		if (board[3][2] == PLAYER_1 && board[3][3] == PLAYER_1 && board[3][4] == PLAYER_1) {
			return true;
		}
	}
	if(board[4][1] == PLAYER_1) {
		if (board[4][2] == PLAYER_1 && board[4][3] == PLAYER_1 && board[4][4] == PLAYER_1) {
			return true;
		}
	}
	// from right
	if(board[0][4] == PLAYER_1) {
		if (board[0][3] == PLAYER_1 && board[0][2] == PLAYER_1 && board[0][1] == PLAYER_1) {
			return true;
		}
	}
	if(board[1][4] == PLAYER_1) {
		if (board[1][3] == PLAYER_1 && board[2][2] == PLAYER_1 && board[1][1] == PLAYER_1) {
			return true;
		}
	}
	if(board[2][4] == PLAYER_1) {
		if (board[2][3] == PLAYER_1 && board[2][2] == PLAYER_1 && board[2][1] == PLAYER_1) {
			return true;
		}
	}
	if(board[3][4] == PLAYER_1) {
		if (board[3][3] == PLAYER_1 && board[2][2] == PLAYER_1 && board[3][1] == PLAYER_1) {
			return true;
		}
	}
	if(board[4][4] == PLAYER_1) {
		if (board[4][3] == PLAYER_1 && board[2][2] == PLAYER_1 && board[4][1] == PLAYER_1) {
			return true;
		}
	}

	return false;
}

bool check_vertical_red(void) {
	// from left
	if(board[0][0] == PLAYER_2) {
		if (board[0][1] == PLAYER_2 && board[0][2] == PLAYER_2 && board[0][3] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][0] == PLAYER_2) {
		if (board[1][1] == PLAYER_2 && board[1][2] == PLAYER_2 && board[1][3] == PLAYER_2) {
			return true;
		}
	}
	if(board[2][0] == PLAYER_2) {
		if (board[2][1] == PLAYER_2 && board[2][2] == PLAYER_2 && board[2][3] == PLAYER_2) {
			return true;
		}
	}
	if(board[3][0] == PLAYER_2) {
		if (board[3][1] == PLAYER_2 && board[3][2] == PLAYER_2 && board[3][3] == PLAYER_2) {
			return true;
		}
	}
	if(board[4][0] == PLAYER_2) {
		if (board[4][1] == PLAYER_2 && board[4][2] == PLAYER_2 && board[4][3] == PLAYER_2) {
			return true;
		}
	}
	//from middle left and middle right
	if(board[0][1] == PLAYER_2) {
		if (board[0][2] == PLAYER_2 && board[0][3] == PLAYER_2 && board[0][4] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][1] == PLAYER_2) {
		if (board[1][2] == PLAYER_2 && board[1][3] == PLAYER_2 && board[1][4] == PLAYER_2) {
			return true;
		}
	}
	if(board[2][1] == PLAYER_2) {
		if (board[2][2] == PLAYER_2 && board[2][3] == PLAYER_2 && board[2][4] == PLAYER_2) {
			return true;
		}
	}
	if(board[3][1] == PLAYER_2) {
		if (board[3][2] == PLAYER_2 && board[3][3] == PLAYER_2 && board[3][4] == PLAYER_2) {
			return true;
		}
	}
	if(board[4][1] == PLAYER_2) {
		if (board[4][2] == PLAYER_2 && board[4][3] == PLAYER_2 && board[4][4] == PLAYER_2) {
			return true;
		}
	}
	// from right
	if(board[0][4] == PLAYER_2) {
		if (board[0][3] == PLAYER_2 && board[0][2] == PLAYER_2 && board[0][1] == PLAYER_2) {
			return true;
		}
	}
	if(board[1][4] == PLAYER_2) {
		if (board[1][3] == PLAYER_2 && board[2][2] == PLAYER_2 && board[1][1] == PLAYER_2) {
			return true;
		}
	}
	if(board[2][4] == PLAYER_2) {
		if (board[2][3] == PLAYER_2 && board[2][2] == PLAYER_2 && board[2][1] == PLAYER_2) {
			return true;
		}
	}
	if(board[3][4] == PLAYER_2) {
		if (board[3][3] == PLAYER_2 && board[2][2] == PLAYER_2 && board[3][1] == PLAYER_2) {
			return true;
		}
	}
	if(board[4][4] == PLAYER_2) {
		if (board[4][3] == PLAYER_2 && board[2][2] == PLAYER_2 && board[4][1] == PLAYER_2) {
			return true;
		}
	}

	return false;
}

bool check_diagonal_down(void) {
	if (board[0][0] == PLAYER_1 && board[1][1] == PLAYER_1 && board[2][2] == PLAYER_1 && board[3][3] == PLAYER_1) {
		return true;
	} else if (board[0][0] == PLAYER_2 && board[1][1] == PLAYER_2 && board[2][2] == PLAYER_2 && board[3][3] == PLAYER_2) {
		return true;
	} else if (board[4][4] == PLAYER_2 && board[3][3] == PLAYER_2 && board[2][2] == PLAYER_2 && board[1][1] == PLAYER_2) {
		return true;
	} else if (board[4][4] == PLAYER_1 && board[3][3] == PLAYER_1 && board[2][2] == PLAYER_1 && board[1][1] == PLAYER_1) {
		return true;
	}
	return false;
	
}

bool check_diagonal_up(void) {
	if (board[0][4] == PLAYER_1 && board[1][3] == PLAYER_1 && board[2][2] == PLAYER_1 && board[3][1] == PLAYER_1) {
		return true;
	} else if (board[4][0] == PLAYER_2 && board[3][1] == PLAYER_2 && board[2][2] == PLAYER_2 && board[1][3] == PLAYER_2) {
		return true;
	} else if (board[0][4] == PLAYER_2 && board[1][3] == PLAYER_2 && board[2][2] == PLAYER_2 && board[3][1] == PLAYER_2) {
		return true;
	} else if (board[4][0] == PLAYER_1 && board[3][1] == PLAYER_1 && board[2][2] == PLAYER_1 && board[1][3] == PLAYER_1) {
		return true;
	}
	return false;
}
uint8_t get_current_player(void) {
	if (current_player == PLAYER_1) {
		return PLAYER_2;
	} else if (current_player == PLAYER_2) {
		return PLAYER_1;
	}
	return current_player;
}
uint8_t is_game_over(void) {
	// YOUR CODE HERE
	if (check_vertical() == true || check_horizontal() == true || check_diagonal_down() == true || check_horizontal_red() == true || check_vertical_red() == true || check_diagonal_up() == true) {
		
		return 1;
	}
	// Detect if the game is over i.e. if a player has won.
	return 0;
}