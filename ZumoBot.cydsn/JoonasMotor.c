#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"


/**
* @brief    Moving motors opposite direction
* @details  giveing same speed to each side of PWM to make motors rotate
* @param    uint8 speed : speed value
* @param    uint32 delay : delay time
*/
void motor_rotate90_left(){
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(0);     // set RightMotor forward mode
    PWM_WriteCompare1(100); 
    PWM_WriteCompare2(100); 
    vTaskDelay(500);
}

/**
* @brief    Moving motors opposite direction
* @details  giveing same speed to each side of PWM to make motors rotate
* @param    uint8 speed : speed value
* @param    uint32 delay : delay time
*/
void motor_rotate90_right(){
    MotorDirLeft_Write(0);      // set LeftMotor backward mode
    MotorDirRight_Write(1);     // set RightMotor forward mode
    PWM_WriteCompare1(100); 
    PWM_WriteCompare2(100); 
    vTaskDelay(500);
}

void motor_backward_turn(uint8 l_speed, uint8 r_speed, uint32 delay)
{
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
    PWM_WriteCompare1(l_speed); 
    PWM_WriteCompare2(r_speed); 
    vTaskDelay(delay);
}

void motor_turn_cross_left(uint8 l_speed, uint8 r_speed, uint32 delay)
{
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(0);     // set RightMotor forward mode
    PWM_WriteCompare1(l_speed); 
    PWM_WriteCompare2(r_speed); 
    vTaskDelay(delay);
}

void motor_turn_cross_right(uint8 l_speed, uint8 r_speed, uint32 delay)
{
    MotorDirLeft_Write(0);      // set LeftMotor forward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
    PWM_WriteCompare1(l_speed); 
    PWM_WriteCompare2(r_speed); 
    vTaskDelay(delay);
}