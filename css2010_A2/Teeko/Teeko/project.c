/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett
 * Modified by <YOUR NAME HERE>
 */ 

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#define F_CPU 8000000L
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "joystick.h"

long cycles = 15624;
// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	start_screen();
	
	// Loop forever,
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	init_joystick();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	init_timer0();
	DDRD = (1<<7);
	OCR2A = cycles;
	// Turn on global interrupts
	sei();
}

void start_screen(void) {
	// Clear terminal screen and output a message
	clear_terminal();
	move_terminal_cursor(10,10);
	printf_P(PSTR("Teeko"));
	move_terminal_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 project by Jean-Pierre Le Roux 45839573"));
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	start_display();
	
	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1) {
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S') {
			break;
		}
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED) {
			break;
		}
	}
}

void new_game(void) {
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the game and display
	initialise_game();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void) {
	uint32_t last_flash_time, current_time;
	uint8_t btn, joystick; //the button pushed
	move_terminal_cursor(30,7);
	printf_P(PSTR("Current Player: 1 (green)"));
	char serial_input;
	last_flash_time = get_current_time();
	
	// We play the game until it's over
	while(!is_game_over()) {
		joystick = joystick_direction();
		if (is_game_over() == 1) {
			handle_game_over();
		}
		int count = 0;
		serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		// We need to check if any button has been pushed, this will be
		// NO_BUTTON_PUSHED if no button has been pushed
		btn = button_pushed();
		if (serial_input == 's' || serial_input == 'S' || btn == BUTTON0_PUSHED || joystick == 3) {
			// If button 0 is pushed, move down.
			move_display_cursor(0, -1);
			if (Check_LEGAL() == true) {
				PORTD = 0xFF;
			} else if (Check_LEGAL() == false) {
				PORTD = 0x00; // turns off all LEDs
			}
			if (is_game_over() == 1) {
				handle_game_over();
			}
		}
		if (serial_input == 'w' || serial_input == 'W' || btn == BUTTON1_PUSHED || joystick == 4) {
			// If button 0 is pushed, move up.
			move_display_cursor(0, 1);
			if (Check_LEGAL() == true) {
				PORTD = 0xFF;
			} else if (Check_LEGAL() == false) {
				PORTD = 0x00; // turns off all LEDs
			}
			if (is_game_over() == 1) {
				handle_game_over();
			}
		}
		if (serial_input == 'd' || serial_input == 'D' || btn == BUTTON2_PUSHED || joystick == 2) {
			// If button 0 is pushed, move right.
			move_display_cursor(1, 0);
			if (Check_LEGAL() == true) {
				PORTD = 0xFF;
			} else if (Check_LEGAL() == false) {
				PORTD = 0x00; // turns off all LEDs
			}
			if (is_game_over() == 1) {
				handle_game_over();
			}
		}
		if (serial_input == 'a' || serial_input == 'A' || btn == BUTTON3_PUSHED || joystick == 1) {
			// If button 0 is pushed, move left.
			move_display_cursor(-1, 0);
			if (Check_LEGAL() == true) {
				PORTD = 0xFF;
			} else if (Check_LEGAL() == false) {
				PORTD = 0x00; // turns off all LEDs
			}
			if (is_game_over() == 1) {
				handle_game_over();
			}
		}
		if (serial_input == ' ') {
			if (count < 8) {
				place_piece();
				PORTD = 0x00;
			}
			if (is_game_over() == 1) {
				handle_game_over();
			}
		}
		if (serial_input == 'P' || serial_input == 'p') {
			// Pause game (display, controls and timers)
			move_terminal_cursor(30,6);
			printf_P(PSTR("Game Paused"));
			while(1) {
				serial_input = fgetc(stdin);
				if(serial_input == 'p' || serial_input == 'P') {
					move_terminal_cursor(30,6);
					printf_P(PSTR("           "));
					move_terminal_cursor(37, 8);
					break;
				}
			}
			
		}
	
		current_time = get_current_time();
		if(current_time >= last_flash_time + 500) {
			// 500ms (0.5 second) has passed since the last time we
			// flashed the cursor, so flash the cursor
			flash_cursor();
			
			// Update the most recent time the cursor was flashed
			last_flash_time = current_time;
		}
	}
	// We get here if the game is over.
}

void handle_game_over() {
	
	move_terminal_cursor(10,13);
	printf_P(PSTR("player %d is the winner"), get_current_player());
	move_terminal_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf_P(PSTR("Press any IOboard button twice"));
	
	while(button_pushed() == NO_BUTTON_PUSHED) {
		;
	}
	
}
