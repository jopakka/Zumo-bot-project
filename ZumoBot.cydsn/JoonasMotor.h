#ifndef JOONASMOTOR_H_ 
#define JOONASMOTOR_H_ 
#include "project.h"
    
void motor_rotate90_left(void);

void motor_rotate90_right(void);

void motor_backward_turn(uint8 l_speed, uint8 r_speed, uint32 delay);

void motor_turn_cross_left(uint8 l_speed, uint8 r_speed, int delay);

void motor_turn_cross_right(uint8 l_speed, uint8 r_speed, int delay);

#endif