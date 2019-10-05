/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "notes.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Beep.h"


void jaateloauto(){
    Beep(150,D1);
    Beep(150,B2);
    Beep(175,D2);
    Beep(150,B2);
    Beep(500,G1);
    
    Beep(150,D1);
    Beep(150,B2);
    Beep(175,D2);
    Beep(150,B2);
    Beep(500,G1);
    
    Beep(350,C2);
    vTaskDelay(30);
    Beep(90,C2);
    
    Beep(350,A2);
    vTaskDelay(30);
    Beep(90,A2);
    
    Beep(500,G1);
}

/* [] END OF FILE */
