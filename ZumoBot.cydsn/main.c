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


// LINE FOLLOWER WORKING
#if 0
void zmain(void){
    struct sensors_ dig;
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
            reflectance_digital(&dig);
            bool viiva = dig.l3 && dig.l2 && dig.l1 && dig.r1 && dig.r2 && dig.r3;
            TickType_t start, stop;
            bool lost = false;
            
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
                print_mqtt("Zumo024/start","%d", start);
                motor_forward(255,0);

                while(x < 3){ // Runs until x is 3
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
                        if(lost){
                            print_mqtt("Zumo024/miss", "%d", xTaskGetTickCount());
                            lost = false;
                        }
                        motor_forward(255,0);
                    }
                    
                    //kevyt vasen
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
                        motor_forward(255,0);;
                    }
                    
                    //kevyt oikee
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 0 && dig.r3 == 0){
                        motor_forward(255,0);
                    }
                    
                    // normi vasen
                    else if(dig.l3 == 0 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
                        motor_turn(200,255,0);
                    }

                    // normi oikee
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0){
                        motor_turn(255,200,0);
                    }

                    //kovaa vasen
                    else if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
                        motor_turn(55,255,0);
                        if(!lost){
                            print_mqtt("Zumo024/line", "%d", xTaskGetTickCount());
                            lost = true;
                        }
                    }

                    //kovaa oikee
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 1 && dig.r3 == 1){
                        motor_turn(255,55,0);
                        if(!lost){
                            print_mqtt("Zumo024/line", "%d", xTaskGetTickCount());
                            lost = true;
                        }
                    }

                    //super kovaa vasen
                    else if(dig.l3 == 1 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
                        motor_turn(0,255,1);
                        if(!lost){
                            print_mqtt("Zumo024/line", "%d", xTaskGetTickCount());
                            lost = true;
                        }
                    }

                    // super kovaa oikee
                    else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 1){
                        motor_turn(255,0,1);
                        if(!lost){
                            print_mqtt("Zumo024/line", "%d", xTaskGetTickCount());
                            lost = true;
                        }
                    }
                }

                stop = xTaskGetTickCount();
                print_mqtt("Zumo024/stop","%d", stop);
                print_mqtt("Zumo024/time","%d", stop - start);
            }

            motor_stop();
            //jaateloauto();
        }
    }
}
#endif


// ZUMO
#if 0

#define RAMSPEED 255
#define CASUALSPEED 150
#define BACKTURNDELAY 300
#define ENEMYDISTANCE 20
#define TURNDELAY 1000
#define HITVALUE 15000

//  checkHit(calX, calY);
void checkHit(int calX, int calY){
    struct accData_ data;
    LSM303D_Read_Acc(&data);
    if(((data.accX - calX) > HITVALUE) || (data.accX - calX) < -HITVALUE || (data.accY - calY) > HITVALUE || (data.accY - calY) < -HITVALUE){
        if((data.accX - calX) > HITVALUE && ((data.accY - calY) > HITVALUE || (data.accY - calY) < -HITVALUE)){
            if((data.accX - calX) > HITVALUE && (data.accY - calY) > HITVALUE){
                print_mqtt("Zumo024/hit", "%d 225", xTaskGetTickCount());
            }
            else if((data.accX - calX) > HITVALUE && (data.accY - calY) < -HITVALUE){
                print_mqtt("Zumo024/hit", "%d 135", xTaskGetTickCount());
            }
            else{
                print_mqtt("Zumo024/hit", "%d 180", xTaskGetTickCount());
            }
        }
        else if((data.accX - calX) < -HITVALUE && ((data.accY - calY) > HITVALUE || (data.accY - calY) < -HITVALUE)){
            if((data.accX - calX) < -HITVALUE && (data.accY - calY) < -HITVALUE){
                print_mqtt("Zumo024/hit", "%d 45", xTaskGetTickCount());
            }
            else if((data.accX - calX) < -15000){
                print_mqtt("Zumo024/hit", "%d 0", xTaskGetTickCount());
            }
            else{
                print_mqtt("Zumo024/hit", "%d 315", xTaskGetTickCount());
            }
        }
        else if((data.accY - calY) > 15000){
            print_mqtt("Zumo024/hit", "%d 270", xTaskGetTickCount());
        }
        else if((data.accY - calY) < -15000){
            print_mqtt("Zumo024/hit", "%d 90", xTaskGetTickCount());
        }
    }
}

// setViiva(&viivaL, &viivaR);
void setViiva(bool *viivaL, bool *viivaR){
    struct sensors_ dig;
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
    reflectance_set_threshold(15000, 15000, 15000, 15000, 15000, 15000);
    
    while(true){
        if(SW1_Read() == 0){
            while(SW1_Read() == 0){
                vTaskDelay(100);
            }
            
            vTaskDelay(500);
            TickType_t start, stop;
            
            IR_flush();
            motor_start();
            while(!viivaL || !viivaR){
                setViiva(&viivaL, &viivaR);
                motor_forward(CASUALSPEED,0);
            }
            
            if(viivaL || viivaR){
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
                while(SW1_Read() == 1){
                    while(enterRing){
                        while(viivaL || viivaR){
                            setViiva(&viivaL, &viivaR);
                            motor_forward(CASUALSPEED,0);
                        }
                        enterRing = false;
                    }
                    
                    reflectance_digital(&dig);
                    viivaL = dig.l3 && dig.l2 && dig.l1;
                    viivaR = dig.r3 && dig.r2 && dig.r1;
                    checkHit(calX, calY);
                    int d = Ultra_GetDistance();
                    
                    // SEARCH ENEMY
                    while(d > ENEMYDISTANCE && !(viivaL || viivaR)){
                        d = Ultra_GetDistance();
                        setViiva(&viivaL, &viivaR);
                        checkHit(calX, calY);
                        int i = 0;
                        while(i < TURNDELAY * 2 && d > ENEMYDISTANCE && !(viivaL || viivaR)){
                            d = Ultra_GetDistance();
                            setViiva(&viivaL, &viivaR);
                            checkHit(calX, calY);
                            motor_turn(CASUALSPEED,CASUALSPEED / 3,1);
                            i++;
                            while(i < TURNDELAY && d > ENEMYDISTANCE && !(viivaL || viivaR)){
                                d = Ultra_GetDistance();
                                setViiva(&viivaL, &viivaR);
                                checkHit(calX, calY);
                                motor_turn(CASUALSPEED / 3,CASUALSPEED,1);
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
                            checkHit(calX, calY);
                            motor_backward(RAMSPEED,1);
                        }
                        int i = 0;
                        while(i < BACKTURNDELAY && (!viivaL || !viivaR)){
                            checkHit(calX, calY);
                            setViiva(&viivaL, &viivaR);
                            motor_backward_turn(0,RAMSPEED,1);
                            i++;
                        }
                        motor_forward(0,0);
                    }

                    else if(viivaR){
                        //poistu takaoikealle
                        for(int i = 0; i < 100; i++){
                            checkHit(calX, calY);
                            motor_backward(RAMSPEED,1);
                        }
                        int i = 0;
                        while(i < BACKTURNDELAY && (!viivaL || !viivaR)){
                            checkHit(calX, calY);
                            setViiva(&viivaL, &viivaR);
                            motor_backward_turn(RAMSPEED,0,1);
                            i++;
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
#if 1
    
#define ROTATETIME 550

// line follower function
void followLine(struct sensors_ dig){
    reflectance_digital(&dig);
    bool viivaL = dig.l3 && dig.l2 && dig.l1;
    bool viivaR = dig.r3 && dig.r2 && dig.r1;
    while(!viivaL && !viivaR){
        reflectance_digital(&dig);
        viivaL = dig.l3 && dig.l2 && dig.l1;
        viivaR = dig.r3 && dig.r2 && dig.r1;
        if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 0 && dig.r3 == 0){
            //suoraan
            motor_forward(75,0);
        }

        else if(dig.l3 == 0 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
            // vasen
            motor_turn(0,75,0);
        }
        
        else if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
            // vasen kova
            motor_turn_cross_left(35,75,0);
        }

        else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0){
            // oikee
            motor_turn(75,0,0);
        }
        
        else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 1 && dig.r3 == 1){
            // oikee kova
            motor_turn_cross_right(75,35,0);
        }

        else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
            // jos kaikki valkosella niin stop
            motor_forward(0,0);
            break;
        }
    }
    motor_forward(0,0);
}
// reverseToLine(viivaL, viivaR, dig);
/*
while(viivaL || viivaR){
    reflectance_digital(&dig);
    viivaL = dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1;
    viivaR = dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1;
    motor_forward(80,0);
}*/
/*while(!viivaL || !viivaR){
    reflectance_digital(&dig);
    viivaL = dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1;
    viivaR = dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1;
    motor_backward(80,0);
}*/
// checkViiva(&viivaL, &viivaR);
void checkViiva(bool *viivaL, bool *viivaR){
    struct sensors_ dig;
    reflectance_digital(&dig);
    *viivaL = dig.l3 && dig.l2 && dig.l1;
    *viivaR = dig.r1 && dig.r2 && dig.r3;
}
// reverse back to line
void reverseToLine(bool viivaL, bool viivaR, struct sensors_ dig){
    while(!viivaL && !viivaR){
        reflectance_digital(&dig);
        viivaL = dig.l3 && dig.l2;
        viivaR = dig.r2 && dig.r3;
        motor_backward(75,0);
    }
    motor_forward(0,0);
}
void reverseToLineLeftside(bool viivaR, struct sensors_ dig){
    while(!viivaR){
        reflectance_digital(&dig);
        viivaR = dig.r2 && dig.r3;
        motor_backward(75,0);
    }
    motor_forward(0,0);
}
void reverseToLineRightside(bool viivaL, struct sensors_ dig){
    while(!viivaL){
        reflectance_digital(&dig);
        viivaL = dig.l2 && dig.l3;
        motor_backward(75,0);
    }
    motor_forward(0,0);
}
// moveWhileLine(viivaL, viivaR, dig);
void moveWhileLine(bool viivaL, bool viivaR, struct sensors_ dig){
    while(viivaL || viivaR){
        // kun viivalla, aja eteenpäin
        reflectance_digital(&dig);
        viivaL = dig.l3 && dig.l2 && dig.l1;
        viivaR = dig.r3 && dig.r2 && dig.r1;
        motor_forward(75,0);
    }
}
void zmain(void){
    struct sensors_ dig;
    IR_Start();
    Ultra_Start();
    
    int suunta = 0;
    int x = 0, d, y = -1, z = 0;
    // d = distance
    // z = how many crossroads it has come back
    
    reflectance_start();
    //reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    reflectance_set_threshold(15000, 15000, 15000, 15000, 15000, 15000); // set center sensor threshold to 11000 and others to 9000
    
    while(true){
        if(SW1_Read() == 0){
            while(SW1_Read() == 0){
                vTaskDelay(100);
            }
            
            bool viivaL = false;
            bool viivaR = false;
            TickType_t start, stop;
            
            IR_flush();
            motor_start();
            checkViiva(&viivaL, &viivaR);
            motor_forward(100,0);
            
            while(!viivaL && !viivaR){
                followLine(dig);
                checkViiva(&viivaL, &viivaR);
            }
            
            if(viivaL || viivaR){
                motor_forward(0,0);
                print_mqtt("Zumo024/ready","maze");
                IR_wait();
                
                start = xTaskGetTickCount();
                print_mqtt("Zumo024/start","%d", start);
                motor_forward(100,0);
            }   
            while(y < 11){//kun y pienempi kuin 11
                d = Ultra_GetDistance();
                checkViiva(&viivaL, &viivaR);
                followLine(dig);
                
                if((viivaL || viivaR) && suunta == 0 && d < 20 && (x == 0 || x == -1 || x == -2)){
                    motor_turn_cross_left(50,150,ROTATETIME);
                    checkViiva(&viivaL, &viivaR);
                    suunta = -1;
                    reverseToLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    motor_forward(0,0);
                    d = Ultra_GetDistance();
                    if((viivaL || viivaR) && d < 20 && suunta == -1){
                        motor_rotate90_right();
                        motor_rotate90_right();
                        suunta = 1;
                        checkViiva(&viivaL, &viivaR);
                        reverseToLine(viivaL, viivaR, dig);
                        d = Ultra_GetDistance();
                        checkViiva(&viivaL, &viivaR);
                        
                        if((viivaL || viivaR) && d < 40 && suunta == 1){
                        motor_turn_cross_right(150,50,ROTATETIME);//suunta alaspäin
                        reverseToLine(viivaL, viivaR, dig);
                        moveWhileLine(viivaL, viivaR, dig);
                        followLine(dig); //followLine tokalle viivalle asti
                        z = 0;
                            while(z != -2){
                                followLine(dig);
                                checkViiva(&viivaL, &viivaR);
                                if(viivaL || viivaR){
                                    y--;
                                    z--;
                                    print_mqtt("Zumo024/position", "%d %d", x, y);
                                    moveWhileLine(viivaL, viivaR, dig);
                                    checkViiva(&viivaL, &viivaR);
                                }
                            }
                            d = Ultra_GetDistance();
                            if(z == -2){
                                motor_turn_cross_right(150,50,ROTATETIME);
                                suunta = -1;
                                //y = y - 2;
                            }
                            reverseToLine(viivaL, viivaR, dig);
                            d = Ultra_GetDistance();
                            while(x != -3){
                                checkViiva(&viivaL, &viivaR);
                                x--;
                                print_mqtt("Zumo024/position", "%d %d", x, y);
                                moveWhileLine(viivaL, viivaR, dig);
                                followLine(dig);
                            }
                            if(x == -3){//x=-3 viivalla motor_turn_cross_left(150,50,700);
                                motor_turn_cross_right(150,50,ROTATETIME);
                                checkViiva(&viivaL, &viivaR);
                                reverseToLine(viivaL, viivaR, dig);
                                reflectance_digital(&dig);
                                checkViiva(&viivaL, &viivaR);
                                suunta = 0;
                            }
                            print_mqtt("Zumo024/position", "%d %d", x, y);
                            d = Ultra_GetDistance();
                        }
                    }
                }
                else if(viivaR && suunta == 0 && d < 20 && x == -3){
                    motor_turn_cross_right(150,50,ROTATETIME);
                    checkViiva(&viivaL, &viivaR);
                    reverseToLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    
                    bool ekaEste = true;
                    while(x != 1){
                        d = Ultra_GetDistance();
                        if(ekaEste && d < 20){
                            motor_turn_cross_right(150,50,ROTATETIME);
                            checkViiva(&viivaL, &viivaR);
                            moveWhileLine(viivaL, viivaR, dig);
                            checkViiva(&viivaL, &viivaR);
                            followLine(dig);
                            y--;
                            print_mqtt("Zumo024/position", "%d %d", x, y);
                            checkViiva(&viivaL, &viivaR);
                            moveWhileLine(viivaL, viivaR, dig);
                            checkViiva(&viivaL, &viivaR);
                            followLine(dig);
                            y--;
                            print_mqtt("Zumo024/position", "%d %d", x, y);
                            motor_turn_cross_left(50,150,ROTATETIME);
                            ekaEste = false;
                        }
                        checkViiva(&viivaL, &viivaR);
                        moveWhileLine(viivaL, viivaR, dig);
                        checkViiva(&viivaL, &viivaR);
                        followLine(dig);
                        x++;
                        print_mqtt("Zumo024/position", "%d %d", x, y);
                        d = Ultra_GetDistance();
                        
                        if(d < 20){
                            motor_turn_cross_left(50,150,ROTATETIME);
                            checkViiva(&viivaL, &viivaR);
                            moveWhileLine(viivaL, viivaR, dig);
                            checkViiva(&viivaL, &viivaR);
                            followLine(dig);
                            y++;
                            print_mqtt("Zumo024/position", "%d %d", x, y);
                            motor_turn_cross_right(150,50,ROTATETIME);
                        }
                        checkViiva(&viivaL, &viivaR);
                        moveWhileLine(viivaL, viivaR, dig);
                        checkViiva(&viivaL, &viivaR);
                        followLine(dig);
                        x++;
                        print_mqtt("Zumo024/position", "%d %d", x, y);
                    }
                    motor_turn_cross_left(50,150,ROTATETIME);
                    suunta = 0;
                    checkViiva(&viivaL, &viivaR);
                    reverseToLine(viivaL, viivaR, dig);
                    d = Ultra_GetDistance();
                }
                else if((viivaL || viivaR) && suunta == 0 && d < 20 && (x==1 || x==2)){
                    motor_turn_cross_right(150,50,ROTATETIME);
                    checkViiva(&viivaL, &viivaR);
                    suunta = 1;
                    reverseToLine(viivaL, viivaR, dig);
                    d = Ultra_GetDistance();
                    
                    if((viivaL || viivaR) && d < 20 && suunta == 1){
                        motor_rotate90_left();
                        motor_rotate90_left();
                        suunta = -1;
                        checkViiva(&viivaL, &viivaR);
                        reverseToLine(viivaL, viivaR, dig);
                        motor_forward(0,0);
                        d = Ultra_GetDistance();
                        
                        if((viivaL || viivaR) && d < 40 && suunta == -1){
                            motor_turn_cross_left(50,150,ROTATETIME);//suunta alaspäin
                            checkViiva(&viivaL, &viivaR);
                            moveWhileLine(viivaL, viivaR, dig);
                            checkViiva(&viivaL, &viivaR);
                            followLine(dig); //followLine tokalle viivalle asti
                            while(z != -2){
                                followLine(dig);
                                checkViiva(&viivaL, &viivaR);
                                if(viivaL || viivaR){
                                    y--;
                                    z--;
                                    print_mqtt("Zumo024/position", "%d %d", x, y);
                                    moveWhileLine(viivaL, viivaR, dig);
                                }
                            }
                                d = Ultra_GetDistance();
                            if(z == -2){
                                motor_turn_cross_left(50,150,ROTATETIME);
                                suunta = -1;
                                reverseToLine(viivaL, viivaR, dig);
                                //y = y - 2;
                            }
                            
                            d = Ultra_GetDistance();
                            checkViiva(&viivaL, &viivaR);
                            moveWhileLine(viivaL, viivaR, dig);
                            checkViiva(&viivaL, &viivaR);
                            followLine(dig);
                            while(x != 3){
                                checkViiva(&viivaL, &viivaR);
                                print_mqtt("Zumo024/position", "%d %d", x, y);
                                moveWhileLine(viivaL, viivaR, dig);
                                checkViiva(&viivaL, &viivaR);
                                followLine(dig);
                                x++;
                            }
                            motor_turn_cross_left(50,150,ROTATETIME);
                            reverseToLine(viivaL, viivaR, dig);
                            suunta = 0;
                            z = 0;
                            print_mqtt("Zumo024/position", "%d %d", x, y);
                            d = Ultra_GetDistance();
                            
                        }
                    }
                }
                else if(viivaL && suunta == 0 && d < 20 && x == 3){
                    motor_turn_cross_left(50,150,ROTATETIME);
                    checkViiva(&viivaL, &viivaR);
                    reverseToLineRightside(viivaL, dig);
                    checkViiva(&viivaL, &viivaR);
                    motor_forward(0,0);
                    d = Ultra_GetDistance();
                    moveWhileLine(viivaL, viivaR, dig);
                    followLine(dig);
                    x--;
                    print_mqtt("Zumo024/position", "%d %d", x, y);
                    d = Ultra_GetDistance();
                    while(x != -1){
                        checkViiva(&viivaL, &viivaR);
                        moveWhileLine(viivaL, viivaR, dig);
                        checkViiva(&viivaL, &viivaR);
                        followLine(dig);
                        x--;
                        print_mqtt("Zumo024/position", "%d %d", x, y);
                    }
                    motor_turn_cross_right(150,50,ROTATETIME);
                    suunta = 0;
                    checkViiva(&viivaL, &viivaR);
                    reverseToLine(viivaL, viivaR, dig);
                    d = Ultra_GetDistance();
                }
                else if((viivaL || viivaR) && suunta == 0 && d >= 20){
                    checkViiva(&viivaL, &viivaR);
                    moveWhileLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    followLine(dig);
                    y++;
                    print_mqtt("Zumo024/position", "%d %d", x, y);
                    d = Ultra_GetDistance();
                }
                else if((viivaL || viivaR) && suunta == -1 && d >=20 && (x == 1 || x == 2)){
                    checkViiva(&viivaL, &viivaR);
                    moveWhileLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    followLine(dig);
                    x--;
                    print_mqtt("Zumo024/position", "%d %d", x, y);
                    motor_turn_cross_right(150,50,ROTATETIME);
                    suunta = 0;
                    checkViiva(&viivaL, &viivaR);
                    reverseToLine(viivaL, viivaR, dig);
                    motor_forward(0,0);
                    d = Ultra_GetDistance();
                }
                else if((viivaL || viivaR) && suunta == 1 && d >=20 && (x == 0 || x == -1 || x == -2)){
                    checkViiva(&viivaL, &viivaR);
                    moveWhileLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    followLine(dig);
                    x++;
                    print_mqtt("Zumo024/position", "%d %d", x, y);
                    motor_turn_cross_left(50,150,ROTATETIME);
                    suunta = 0;
                    checkViiva(&viivaL, &viivaR);
                    reverseToLine(viivaL, viivaR, dig);
                    motor_forward(0,0);
                    d = Ultra_GetDistance();
                }
                else if((viivaL || viivaR) && suunta == -1 && d >=20 && (x == 0 || x == -1 || x == -2)){
                    checkViiva(&viivaL, &viivaR);
                    moveWhileLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    followLine(dig);
                    x--;
                    print_mqtt("Zumo024/position", "%d %d", x, y);
                    motor_turn_cross_right(150,50,ROTATETIME);
                    suunta = 0;
                    checkViiva(&viivaL, &viivaR);
                    reverseToLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    motor_forward(0,0);
                    d = Ultra_GetDistance();
                }
                else if((viivaL || viivaR) && suunta == 1 && d >=20 && (x == 1 || x == 2)){
                    checkViiva(&viivaL, &viivaR);
                    moveWhileLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    followLine(dig);
                    x++;
                    print_mqtt("Zumo024/position", "%d %d", x, y);
                    motor_turn_cross_left(50,150,ROTATETIME);
                    suunta = 0;
                    checkViiva(&viivaL, &viivaR);
                    reverseToLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    motor_forward(0,0);
                    d = Ultra_GetDistance();
                }
                else if((viivaL || viivaR) && suunta == -1 && x == -3){
                    motor_turn_cross_right(150,50,ROTATETIME);
                    reverseToLine(viivaL, viivaR, dig);
                    suunta = 0;
                }
                else if((viivaL || viivaR) && suunta == 1 && x == 3){
                    motor_turn_cross_left(50,150,ROTATETIME);
                    reverseToLine(viivaL, viivaR, dig);
                    suunta = 0;
                }
            }//end of while y < 11
            while(y == 11){//while y is 11 or 12
                checkViiva(&viivaL, &viivaR);
                if((viivaL || viivaR) && x < 0){
                    motor_turn_cross_right(150,50,ROTATETIME);
                    checkViiva(&viivaL, &viivaR);
                    suunta = 1;
                    reverseToLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    
                    while(x < 0){
                        moveWhileLine(viivaL, viivaR, dig);
                        checkViiva(&viivaL, &viivaR);
                        followLine(dig);
                        checkViiva(&viivaL, &viivaR);
                        x++;
                        print_mqtt("Zumo024/position", "%d %d", x, y);
                    }
                    motor_turn_cross_left(50,150,ROTATETIME);
                }
                else if((viivaL || viivaR) && x > 0){
                    motor_turn_cross_left(50,150,ROTATETIME);
                    checkViiva(&viivaL, &viivaR);
                    suunta = -1;
                    reverseToLine(viivaL, viivaR, dig);
                    checkViiva(&viivaL, &viivaR);
                    
                    while(x > 0){
                        moveWhileLine(viivaL, viivaR, dig);
                        checkViiva(&viivaL, &viivaR);
                        followLine(dig);
                        checkViiva(&viivaL, &viivaR);
                        x--;
                        print_mqtt("Zumo024/position", "%d %d", x, y);
                    }
                    motor_turn_cross_right(150,50,ROTATETIME);
                }
                checkViiva(&viivaL, &viivaR);
                moveWhileLine(viivaL, viivaR, dig);
                checkViiva(&viivaL, &viivaR);
                followLine(dig);
                y++;
                print_mqtt("Zumo024/position", "%d %d", x, y);
            }
            while(y <= 13){//while y is 13
                checkViiva(&viivaL, &viivaR);
                moveWhileLine(viivaL, viivaR, dig);
                checkViiva(&viivaL, &viivaR);
                followLine(dig);
                y++;
                print_mqtt("Zumo024/position", "%d %d", x, y);
            }
            checkViiva(&viivaL, &viivaR);
            moveWhileLine(viivaL, viivaR, dig);
            checkViiva(&viivaL, &viivaR);
            followLine(dig);
            stop = xTaskGetTickCount();
            motor_stop();
            print_mqtt("Zumo024/stop", "%d", stop);
            print_mqtt("Zumo024/time", "%d", stop - start);
            }//end of button
    }//end of for
}//end of main
#endif

/* [] END OF FILE */