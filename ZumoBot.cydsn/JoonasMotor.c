#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"

void motor_rotate90_left(void){
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(0);     // set RightMotor forward mode
    PWM_WriteCompare1(100); 
    PWM_WriteCompare2(100); 
    vTaskDelay(500);
}

void motor_rotate90_right(void){
    MotorDirLeft_Write(0);      // set LeftMotor backward mode
    MotorDirRight_Write(1);     // set RightMotor forward mode
    PWM_WriteCompare1(100); 
    PWM_WriteCompare2(100); 
    vTaskDelay(500);
}

void motor_backward_turn(uint8 l_speed, uint8 r_speed, int delay)
{
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
    PWM_WriteCompare1(l_speed); 
    PWM_WriteCompare2(r_speed); 
    vTaskDelay(delay);
}

void motor_turn_cross_left(void)
{
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(0);     // set RightMotor forward mode
    PWM_WriteCompare1(50); 
    PWM_WriteCompare2(150); 
    vTaskDelay(600);
}

void motor_turn_cross_right(void)
{
    MotorDirLeft_Write(0);      // set LeftMotor forward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
    PWM_WriteCompare1(150); 
    PWM_WriteCompare2(50); 
    vTaskDelay(600);
}