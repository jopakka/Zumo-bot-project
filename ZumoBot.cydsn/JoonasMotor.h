#ifndef JOONASMOTOR_H_ 
#define JOONASMOTOR_H_ 

#include "project.h"
    
void motor_rotate90_left(int start);

void motor_rotate90_right(int start);

void motor_backward_turn(uint8 l_speed, uint8 r_speed, uint32 delay);

#endif