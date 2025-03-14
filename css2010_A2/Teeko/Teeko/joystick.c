/*
 * joystick.c
 *
 * Created: 2/06/2022 6:41:22 PM
 *  Author: lerxj
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "joystick.h"
#include "timer0.h"

static uint16_t adc_x, adc_y;
static uint8_t prev_direction = 0;
static uint32_t prev_time;

void init_joystick(void) {
	// Set up ADC - AVCC reference, right adjust
	ADMUX = (1<<REFS0);
	// Turn on the ADC  and choose clock divider of 64
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
}

// Helper function to retrieve and convert ADC values
static void get_adc_values(void) {
	// Read x axis
	ADMUX &= ~1;
	// Start the ADC conversion
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	adc_x = ADC; // Read the value

	// Read y axis
	ADMUX |= 1;
	// Start the ADC conversion
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	adc_y = ADC; // Read the value
}

uint8_t joystick_direction(void) {
	/* 1= up
	 * 2 = down
	 * 3 = left
	 * 4 = right
	 * -1 = middle
	 */
	uint8_t direction;
	uint32_t current_time;

	get_adc_values();
	if(adc_x < 257) {
		direction = 3;
	} else if(adc_x > 768) {
		direction = 4;
	} else if(adc_y > 768) {
		direction = 1;
	} else if(adc_y < 257) {
		direction = 2;
	} else {
		if(prev_direction == 3) {
			direction = (adc_x < 386 ? 3:-1);
		} else if(prev_direction == 4) {
			direction = (adc_x > 641 ? 4:-1);
		} else if(prev_direction == 1) {
			direction = (adc_y > 641 ? 1:-1);
		} else if(prev_direction == 2) {
			direction = (adc_y < 386 ? 2:-1);
		} else {
			direction = -1;
		}
	}

	// Only process if joystick not in middle
	if(direction > 0) {
		// If same direction as previously, check if 500ms has elapsed
		// (auto fire) or else assume at middle
		if(prev_direction == direction) {
			current_time = get_current_time();
			if(current_time < prev_time + 300) {
				return -1;
			}
		}

		prev_direction = direction;
		prev_time = get_current_time();
	}

	return direction;
}