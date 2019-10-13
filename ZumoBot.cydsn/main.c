/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This 
    library has basic methods of various sensors and communications so that you can make what you want with them. <br> 
    <br><br>
    </p>
    
    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>
    
    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/

#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "LSM303D.h"
#include "IR.h"
#include "Beep.h"
#include "mqtt_sender.h"
#include <time.h>
#include <sys/time.h>
#include "serial1.h"
#include <unistd.h>
#include "JoonasMotor.h"
#include "notes.h"
#include "songs.h"


/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/


// LINE FOLLOWER DIGITAL
#if 0
void zmain(void){
    struct sensors_ dig;
    IR_Start();
    
    reflectance_start();
    reflectance_set_threshold(15000, 15000, 17000, 17000, 15000, 15000);
    
    while(true){
        if(SW1_Read() == 0){
            while(SW1_Read() == 0){
                vTaskDelay(100);
            }

            vTaskDelay(500);

            int x = 0;
            bool viiva = dig.l3 && dig.l2 && dig.l1 && dig.r1 && dig.r2 && dig.r3;
            TickType_t start, stop;
            
            IR_flush();
            motor_start();
            
            while(!viiva){
                motor_forward(100,0);
                reflectance_digital(&dig);
                viiva = dig.l3 && dig.l2 && dig.l1 && dig.r1 && dig.r2 && dig.r3;
            }
            
            // LINE FOLLOWER CODE
            if(viiva){
                motor_forward(0,0);
                print_mqtt("Zumo024/ready","line");
                IR_wait();

                start = xTaskGetTickCount();
                print_mqtt("Zumo024/start","%d ms", start);
                motor_forward(255,0);

                while(x < 3){
                    reflectance_digital(&dig);
                    viiva = dig.l3 && dig.l2 && dig.l1 && dig.r1 && dig.r2 && dig.r3;

                    if(viiva){
                        x++;

                        while(viiva){
                            reflectance_digital(&dig);
                            viiva = dig.l3 && dig.l2 && dig.l1 && dig.r1 && dig.r2 && dig.r3;
                            motor_forward(255,0);
                        }
                    }

                    //suoraan
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 0 && dig.r3 == 0){
                        motor_forward(255,0);
                    }
                    
                    // normi vasen
                    else if(dig.l3 == 0 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
                        motor_turn(160,255,0);
                    }

                    // normi oikee
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0){
                        motor_turn(255,160,0);
                    }

                    //kovaa vasen
                    else if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
                        motor_turn(90,255,0);
                    }

                    //kovaa oikee
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 1 && dig.r3 == 1){
                        motor_turn(255,90,0);
                    }

                    //super kovaa vasen
                    else if(dig.l3 == 1 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
                        motor_turn(0,255,0);
                    }

                    // super kovaa oikee
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 1){
                        motor_turn(255,0,0);
                    }
                }

                stop = xTaskGetTickCount();
                print_mqtt("Zumo024/stop","%d", stop);
                print_mqtt("Zumo024/time","%d", stop - start);
            }

            motor_stop();
            jaateloauto();
        }
    }
}
#endif


// LINE FOLLOWER ANALOG
#if 1

#define MAXSPEED 255
#define LINETRESHOLD 18000
#define ERROR 1000

void zmain(void){
    struct sensors_ ref;
    struct sensors_ dig;
    bool miss = false;
    bool viiva = false;
    TickType_t start, stop;

    IR_Start();
    reflectance_start();
    reflectance_set_threshold(15000, 15000, 15000, 15000, 15000, 15000);
    
    while(true){
        if(SW1_Read() == 0){
            while(SW1_Read() == 0){
                vTaskDelay(100);
            }

            vTaskDelay(500);

            int x = 0;
            IR_flush();
            motor_start();
            
            // go to first line
            while(!viiva){
                motor_forward(100,0);
                reflectance_digital(&dig);
                viiva = dig.l3 && dig.l2 && dig.l1 && dig.r1 && dig.r2 && dig.r3;
            }
            
            // LINE FOLLOWER CODE
            if(viiva){
                motor_forward(0,0);
                print_mqtt("Zumo024/ready","line");
                IR_wait();

                start = xTaskGetTickCount();
                print_mqtt("Zumo024/start","%d ms", start);
                motor_forward(MAXSPEED,0);

                while(x < 3){
                    reflectance_read(&ref);
                    reflectance_digital(&dig);
                    viiva = dig.l3 && dig.l2 && dig.l1 && dig.r1 && dig.r2 && dig.r3;

                    if(viiva){
                        x++;

                        while(viiva){
                            reflectance_digital(&dig);
                            reflectance_read(&ref);
                            viiva = dig.l3 && dig.l2 && dig.l1 && dig.r1 && dig.r2 && dig.r3;
                            motor_forward(MAXSPEED,0);
                        }
                    }

                    if(ref.l1 > LINETRESHOLD && ref.r1 > LINETRESHOLD){
                        //suoraan
                        if(miss = true){
                            print_mqtt("Zumo024/line","%d", xTaskGetTickCount());
                            miss = false;
                        }
                        motor_forward(MAXSPEED,0);
                    }

                    else if(ref.l3 > LINETRESHOLD - ERROR * 2 || ref.l2 > LINETRESHOLD - ERROR || ref.l1 > LINETRESHOLD){
                        if(ref.l3 > LINETRESHOLD - ERROR * 2 && ref.l2 > LINETRESHOLD - ERROR){
                            //vasen3
                            if(miss = false){
                                print_mqtt("Zumo024/miss","%d", xTaskGetTickCount());
                                miss = true;
                            }
                            motor_turn(ceil(MAXSPEED / 4),MAXSPEED,0);
                        }
                        else if(!(ref.l2 > LINETRESHOLD - ERROR && ref.l1 > LINETRESHOLD)){
                            //vasen4
                            if(miss = false){
                                print_mqtt("Zumo024/miss","%d", xTaskGetTickCount());
                                miss = true;
                            }
                            motor_turn(0,MAXSPEED,0)
                        }
                        else if(ref.l2 > LINETRESHOLD - ERROR && ref.l1 > LINETRESHOLD){
                            //vasen2
                            motor_turn(ceil(MAXSPEED / 2),MAXSPEED,0);
                        }
                        else if(!(ref.l2 > LINETRESHOLD - ERROR) && ref.l1 > LINETRESHOLD){
                            //vasen1
                            motor_turn(ceil(MAXSPEED / 4 * 3),MAXSPEED,0);
                        }
                    }

                    else if(ref.r1 > LINETRESHOLD || ref.r2 > LINETRESHOLD - ERROR || ref.r3 > LINETRESHOLD - ERROR * 2){
                        if(ref.r2 > LINETRESHOLD - ERROR && ref.r3 > LINETRESHOLD - ERROR * 2){
                            //oikea3
                            if(miss = false){
                                print_mqtt("Zumo024/miss","%d", xTaskGetTickCount());
                                miss = true;
                            }
                            motor_turn(MAXSPEED,ceil(MAXSPEED / 4),0);
                        }
                        else if(!(ref.r2 > LINETRESHOLD - ERROR && ref.r1 > LINETRESHOLD)){
                            //oikea4
                            if(miss = false){
                                print_mqtt("Zumo024/miss","%d", xTaskGetTickCount());
                                miss = true;
                            }
                            motor_turn(MAXSPEED,0,0);
                        }
                        else if(ref.r1 > LINETRESHOLD && ref.r2 > LINETRESHOLD - ERROR){
                            //oikea2
                            motor_turn(MAXSPEED,ceil(MAXSPEED / 2),0);
                        }
                        else if(!(ref.r2 > LINETRESHOLD - ERROR) && ref.r1 > LINETRESHOLD){
                            //oikea1
                            motor_turn(MAXSPEED,ceil(MAXSPEED / 4 * 3),0);
                        }
                    }
                }
            }
            motor_stop();
            stop = xTaskGetTickCount();
            print_mqtt("Zumo024/stop","%d", stop);
            print_mqtt("Zumo024/time","%d", stop - start);
            jaateloauto();
        }
    }
}
#endif


// ZUMO
#if 0

#define RAMSPEED 255
#define CASUALSPEED 150
#define BACKTURNDELAY 500
#define ENEMYDISTANCE 15

// checkHit();
void checkHit(void){
    LSM303D_Read_Acc(&data);
    if(((data.accX - calX) > 10000) || (data.accX - calX) < -10000 || (data.accY - calY) > 10000 || (data.accY - calY) < -10000){
        if((data.accX - calX) > 10000 && ((data.accY - calY) > 10000 || (data.accY - calY) < -10000)){
            if((data.accX - calX) > 10000 && (data.accY - calY) > 10000){
                print_mqtt("Zumo024/hit", "%d 225", xTaskGetTickCount());
            }
            else if((data.accX - calX) > 10000 && (data.accY - calY) < -10000){
                print_mqtt("Zumo024/hit", "%d 135", xTaskGetTickCount());
            }
            else{
                print_mqtt("Zumo024/hit", "%d 180", xTaskGetTickCount());
            }
        }
        else if((data.accX - calX) < -10000 && ((data.accY - calY) > 10000 || (data.accY - calY) < -10000)){
            if((data.accX - calX) < -10000 && (data.accY - calY) < -10000){
                print_mqtt("Zumo024/hit", "%d 45", xTaskGetTickCount());
            }
            else if((data.accX - calX) < -10000){
                print_mqtt("Zumo024/hit", "%d 0", xTaskGetTickCount());
            }
            else{
                print_mqtt("Zumo024/hit", "%d 315", xTaskGetTickCount());
            }
        }
        else if((data.accY - calY) > 10000){
            print_mqtt("Zumo024/hit", "%d 270", xTaskGetTickCount());
        }
        else if((data.accY - calY) < -10000){
            print_mqtt("Zumo024/hit", "%d 90", xTaskGetTickCount());
        }
    }
}

// setViiva(&viivaL, &viivaR, &dig);
void setViiva(bool *viivaL, bool *viivaR){
    reflectance_digital(&dig);
    *viivaL = dig.l3 && dig.l2 && dig.l1;
    *viivaR = dig.r3 && dig.r2 && dig.r1;
}

void zmain(void){
    struct sensors_ dig;
    struct accData_ data;
    int calX, calY;
    bool enterRing = true;
    bool viivaL = false;
    bool viivaR = false;
    
    IR_Start();
    LSM303D_Start();
    Ultra_Start();
    reflectance_start();
    reflectance_set_threshold(20000, 20000, 20000, 20000, 20000, 20000);
    
    while(true){
        if(SW1_Read() == 0){
            while(SW1_Read() == 0){
                vTaskDelay(100);
            }
            
            vTaskDelay(500);
            TickType_t start, stop;
            
            IR_flush();
            motor_start();
            
            while(!viivaL && !viivaR){
                setViiva(&viivaL, &viivaR, &dig);
                motor_forward(CASUALSPEED,0);
            }
            
            if(viivaL && viivaR){
                motor_forward(0,0);
                print_mqtt("Zumo024/ready","zumo");
                IR_wait();
                
                LSM303D_Read_Acc(&data);
                calY = data.accY;
                calX = data.accX;
                
                start = xTaskGetTickCount();
                print_mqtt("Zumo024/start","%d", start);
                motor_forward(CASUALSPEED,0);
                
                // ZUMO LOOP
                while(SW1_Read == 1){
                    while(enterRing){
                        while(viivaL || viivaR){
                            setViiva(&viivaL, &viivaR, &dig);
                            motor_forward(CASUALSPEED,0);
                        }
                        enterRing = false;
                    }
                    
                    setViiva(&viivaL, &viivaR, &dig);
                    checkHit();
                    int d = Ultra_GetDistance();
                    
                    // SEARCH ENEMY
                    while(d > ENEMYDISTANCE && !(viivaL || viivaR)){
                        d = Ultra_GetDistance();
                        setViiva(&viivaL, &viivaR, &dig);
                        checkHit();
                        int i = 0;
                        while(i < 50 && d > ENEMYDISTANCE && !(viivaL || viivaR)){
                            d = Ultra_GetDistance();
                            setViiva(&viivaL, &viivaR, &dig);
                            checkHit();
                            motor_turn(CASUALSPEED,CASUALSPEED / 1.5,1);
                            i++;
                            while(i < 25 && d > ENEMYDISTANCE && !(viivaL || viivaR)){
                                d = Ultra_GetDistance();
                                setViiva(&viivaL, &viivaR, &dig);
                                checkHit();
                                motor_turn(CASUALSPEED / 1.5,CASUALSPEED,1);
                                i++;
                            }
                        }
                    }

                    // ATTACK TO BAD GUYS
                    if(d <= ENEMYDISTANCE){
                        motor_forward(RAMSPEED,0);
                    }

                    // TURN BACK
                    if(viivaL){
                        //poistu takavasemmalle
                        for(int i = 0; i < 100; i++){
                            checkHit();
                            motor_backward(RAMSPEED,1);
                        }
                        for(int i = 0; i < BACKTURNDELAY; i++;){
                            checkHit();
                            motor_backward_turn(0,RAMSPEED,1);
                        }
                        motor_forward(0,0);
                    }

                    else if(viivaR){
                        //poistu takaoikealle
                        for(int i = 0; i < 100; i++){
                            checkHit();
                            motor_backward(RAMSPEED,1);
                        }
                        for(int i = 0; i < BACKTURNDELAY; i++;){
                            checkHit();
                            motor_backward_turn(RAMSPEED,0,1);
                        }
                        motor_forward(0,0);
                    }
                }

                stop = xTaskGetTickCount();
                print_mqtt("Zumo024/stop","%d", stop);
                print_mqtt("Zumo024/time","%d", stop - start);
            }

            motor_stop();
        }
    }
}
#endif


// MAZE
#if 0

#define MAXSPEED 75
#define TURNSPEED 25

// setViiva(&viivaL, &viivaR, &dig);
void setViiva(bool *viivaL, bool *viivaR, struct sensors_ *dig){
    struct sensors_ dig;
    reflectance_digital(&dig);
    *viivaL = dig.l3 && dig.l2 && dig.l1;
    *viivaR = dig.r3 && dig.r2 && dig.r1;
}

// followLine(&viivaL, &viivaR, &dig);
void followLine(bool *viivaL, bool *viivaR, struct sensors_ *dig){
    setViiva(&viivaL, &viivaR, &dig);
    while(!viivaL || !viivaR){
        setViiva(&viivaL, &viivaR, &dig);
        if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 0 && dig.r3 == 0){
            //suoraan
            motor_forward(MAXSPEED,0);
        }

        else if(dig.l3 == 0 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
            // vasen
            motor_turn(TURNSPEED,MAXSPEED,0);
        }

        else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0){
            // oikee
            motor_turn(MAXSPEED,TURNSPEED,0);
        }
    }
    if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
        motor_forward(0,0);
    }
}

// reverseToLine(&viivaL, &viivaR, &dig);
void reverseToLine(bool *viivaL, bool *viivaR, struct sensors_ *dig){
    setViiva(&viivaL, &viivaR, &dig);
    while(!viivaL || !viivaR){
        setViiva(&viivaL, &viivaR, &dig);
        motor_backward(MAXSPEED,0);
    }
}

// moveWhileLine(&viivaL, &viivaR, &dig);
void moveWhileLine(bool *viivaL, bool *viivaR, struct sensors_ *dig){
    setViiva(&viivaL, &viivaR, &dig);
    while(viivaL || viivaR){
        // kun viivalla, aja eteenpäin
        setViiva(&viivaL, &viivaR, &dig);
        motor_forward(MAXSPEED,0);
    }
}

// moveAndCountLines(/*maxLines*/, &viivaL, &viivaR, &dig);
void moveAndCountLines(int maxLines, bool *viivaL, bool *viivaR, struct sensors_ *dig){
    int lines = 0;
    while (lines < maxLines) {
        moveWhileLine(&viivaL, &viivaR, &dig);
        followLine(&viivaL, &viivaR, &dig);
        lines++;
    }
}

void zmain(void){
    struct sensors_ dig;
    int x = 0, d, y = 0;
    TickType_t start, stop;
    bool viivaL = false;
    bool viivaR = false;

    IR_Start();
    Ultra_Start();
    IR_flush();
    motor_start();
    
    reflectance_start();
    reflectance_set_threshold(12000, 12000, 12000, 12000, 12000, 12000);
    
    // MAZE CODE
    while(true){
        if(SW1_Read() == 0){
            while(SW1_Read() == 0){
                vTaskDelay(100);
            }
            
            followLine(&viivaL, &viivaR, &dig);
            print_mqtt("Zumo024/ready","maze");
            IR_wait();
            start = xTaskGetTickCount();
            print_mqtt("Zumo024/start","&d", start);
            moveAndCountLines(1, &viivaL, &viivaR, &dig);
            print_mqtt("Zumo024/position", "%d %d", x, y);

            while(y < 11){
                if(x <= 0 && x >= -3){
                    d = Ultra_GetDistance();
                    if(d < 20){
                        if(x == 3){
                            motor_turn_cross_right();
                            moveAndCountLines(2, &viivaL, &viivaR, &dig);
                            x += 2;
                            motor_turn_cross_left();
                        }
                        else{
                            motor_turn_cross_left();
                            d = Ultra_GetDistance();
                            if(d < 20){
                                motor_rotate90_right();
                                motor_rotate90_right();
                                d = Ultra_GetDistance();
                                if(d < 40){
                                    if(d < 20){
                                        motor_turn_cross_right();
                                        moveAndCountLines(2, &viivaL, &viivaR, &dig);
                                        y -= 2;
                                        motor_turn_cross_left();
                                        if(x == 0){
                                            moveAndCountLines(3, &viivaL, &viivaR, &dig);
                                            x += 3;
                                        }
                                        else{
                                            moveAndCountLines(4, &viivaL, &viivaR, &dig);
                                            x += 4;
                                        }
                                        motor_turn_cross_left();
                                    }
                                    else{
                                        moveAndCountLines(1, &viivaL, &viivaR, &dig);
                                        x++;
                                        motor_turn_cross_right();
                                        moveAndCountLines(2, &viivaL, &viivaR, &dig);
                                        y -= 2;
                                        motor_turn_cross_left();
                                        if(y == 0){
                                            moveAndCountLines(3, &viivaL, &viivaR, &dig);
                                            y += 3;
                                        }
                                        else{
                                            moveAndCountLines(4, &viivaL, &viivaR, &dig);
                                            y += 4;
                                        }
                                        motor_turn_cross_left();
                                    }
                                }
                                else{
                                    moveAndCountLines(1, &viivaL, &viivaR, &dig);
                                    x++;
                                    motor_turn_cross_left();
                                }
                            }
                            else{
                                moveAndCountLines(1, &viivaL, &viivaR, &dig);
                                x--;
                                motor_turn_cross_right();
                            }
                        }
                        reverseToLine(&viivaL, &viivaR, &dig);
                    }
                    else{
                        moveAndCountLines(1, &viivaL, &viivaR, &dig);
                        y++;
                    }
                }
                else if(x > 0 && <= 3){
                    d = Ultra_GetDistance();
                    if(d < 20){
                        if( x == 3){
                            motor_turn_cross_left();
                            moveAndCountLines(2, &viivaL, &viivaR, &dig);
                            x -= 2;
                            motor_turn_cross_right();
                        }
                        else{
                            motor_turn_cross_right();
                            d = Ultra_GetDistance();
                            if(d < 20){
                                motor_rotate90_right();
                                motor_rotate90_right();
                                d = Ultra_GetDistance();
                                if(d < 40){
                                    if(d < 20){
                                        motor_turn_cross_left();
                                        moveAndCountLines(2, &viivaL, &viivaR, &dig);
                                        y -= 2;
                                        motor_turn_cross_right();
                                        moveAndCountLines(3, &viivaL, &viivaR, &dig);
                                        x -= 3;
                                        motor_turn_cross_right();
                                    }
                                    else{
                                        moveAndCountLines(1, &viivaL, &viivaR, &dig);
                                        x--;
                                        motor_turn_cross_left();
                                        moveAndCountLines(2, &viivaL, &viivaR, &dig);
                                        y -= 2;
                                        motor_turn_cross_right();
                                        moveAndCountLines(3, &viivaL, &viivaR, &dig);
                                        x -= 3;
                                        motor_turn_cross_right();
                                    }
                                }
                                else{
                                    moveAndCountLines(1, &viivaL, &viivaR, &dig);
                                    x--;
                                    motor_turn_cross_right();
                                }
                            }
                            else{
                                moveAndCountLines(1, &viivaL, &viivaR, &dig);
                                x++;
                                motor_turn_cross_left();
                            }
                        }
                        reverseToLine(&viivaL, &viivaR, &dig);
                    }
                    else{
                        moveAndCountLines(1, &viivaL, &viivaR, &dig);
                        y++;
                    }
                }
            }// end of while y < 11
            while(y == 11){
                if(x == 0){
                    motor_turn_cross_right();
                    while(x < 0){
                        moveAndCountLines(1, &viivaL, &viivaR, &dig);
                        x++;
                    }
                    motor_turn_cross_left();
                }
                else if(x > 0){
                    motor_turn_cross_left();
                    while(x > 0){
                        moveAndCountLines(1, &viivaL, &viivaR, &dig);
                        x++;
                    }
                    motor_turn_cross_left();
                }
                moveAndCountLines(1, &viivaL, &viivaR, &dig);
                y++;
            }//end of while == 11
            while( y <= 13){
                moveAndCountLines(1, &viivaL, &viivaR, &dig);
                y++;
            }
            followLine(&viivaL, &viivaR, &dig);
            motor_stop();
            stop = xTaskGetTickCount();
            print_mqtt("Zumo024/stop", "&d", stop);
            print_mqtt("Zumo024/time", "&d", stop - start);
        }//end of button
    }//end of while
}//end of main
#endif

/* [] END OF FILE */