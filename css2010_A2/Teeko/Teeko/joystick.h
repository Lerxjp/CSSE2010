/*
 * joystick.h
 *
 * Created: 2/06/2022 6:41:22 PM
 *  Author: lerxj
 */ 
#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <stdint.h>

// Setup ADC
void init_joystick(void);

// Get current direction of joystick
uint8_t joystick_direction(void);

#endif /* JOYSTICK_H_ */