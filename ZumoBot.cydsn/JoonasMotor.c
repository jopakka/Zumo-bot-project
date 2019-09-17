#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"


/**
* @brief    Moving motors opposite direction
* @details  giveing same speed to each side of PWM to make motors rotate
* @param    uint8 speed : speed value
* @param    uint32 delay : delay time
*/
void rotate90_left(uint8 speed,uint32 delay){
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(0);     // set RightMotor forward mode
    PWM_WriteCompare1(speed); 
    PWM_WriteCompare2(speed); 
    vTaskDelay(delay);
}

/**
* @brief    Moving motors opposite direction
* @details  giveing same speed to each side of PWM to make motors rotate
* @param    uint8 speed : speed value
* @param    uint32 delay : delay time
*/
void rotate90_right(uint8 speed,uint32 delay){
    MotorDirLeft_Write(0);      // set LeftMotor forward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
    PWM_WriteCompare1(speed); 
    PWM_WriteCompare2(speed); 
    vTaskDelay(delay);
}