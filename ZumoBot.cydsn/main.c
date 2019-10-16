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
    reflectance_set_threshold(15000, 15000, 15000, 15000, 15000, 15000);
    
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
                print_mqtt("Zumo024/start","%d", start);
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
#define TURNDELAY 500
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
// turnUntilLine(/*0 left or 1 right*/);
/*0 turn left, 1 turn right*/
void turnUntilLine(int direction){
    struct sensors_ dig;
    reflectance_digital(&dig);
    bool viiva = dig.l1 && dig.r1;
    if(direction == 0){
        motor_turn_cross_left(50,150,400);
        while(!viiva){
            motor_turn_cross_left(50,150,0);
            reflectance_digital(&dig);
            viiva = dig.l1 && dig.r1;
        }
    }
    else if(direction == 1){
        motor_turn_cross_right(150,50,400);
        while(!viiva){
            motor_turn_cross_right(150,50,0);
            reflectance_digital(&dig);
            viiva = dig.l1 && dig.r1;
        }
    }
    motor_forward(0,0);
}
// checkViiva(&viivaL, &viivaR);
void checkViiva(bool *viivaL, bool *viivaR){
    struct sensors_ dig;
    reflectance_digital(&dig);
    *viivaL = dig.l3 && dig.l2 && dig.l1;
    *viivaR = dig.r3 && dig.r2 && dig.r1; 
}
// followLine();
void followLine(void){
    struct sensors_ dig;
    reflectance_digital(&dig);
    bool viivaL = dig.l3 && dig.l2 && dig.l1;
    bool viivaR = dig.r3 && dig.r2 && dig.r1;
    while(!viivaL || !viivaR){
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

        else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0){
            // oikee
            motor_turn(75,0,0);
        }

        else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
            // jos kaikki valkosella niin stop
            motor_forward(0,0);
            viivaL = true;
        }
    }
    motor_forward(0,0);
}
// reverseToLine(viivaL, viivaR);
void reverseToLine(int viivaL, int viivaR){
    struct sensors_ dig;
    while(!viivaL || !viivaR){
        reflectance_digital(&dig);
        viivaL = dig.l3 && dig.l2;
        viivaR = dig.r2 && dig.r3;
        motor_backward(75,0);
    }
    motor_forward(0,0);
}
// moveWhileLine(viivaL, viivaR);
void moveWhileLine(int viivaL, int viivaR){
    struct sensors_ dig;
    while(viivaL || viivaR){
        // kun viivalla, aja eteenpäin
        reflectance_digital(&dig);
        viivaL = dig.l3 && dig.l2 && dig.l1;
        viivaR = dig.r3 && dig.r2 && dig.r1;
        motor_forward(75,0);
    }
    motor_forward(0,0);
}
void zmain(void){
    struct sensors_ dig;
    IR_Start();
    Ultra_Start();
    
    int suunta = 0;
    int x = 0, d, y = 0, z = 0;
    
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
            
            while(!viivaL || !viivaR){
                followLine();
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
                followLine();
                
                if(viivaL ^ viivaR){
                    if(viivaR && suunta == 0 && d < 20 && x == -3){
                        turnUntilLine(1);
                        checkViiva(&viivaL, &viivaR);
                        reverseToLine(viivaL, viivaR);
                        checkViiva(&viivaL, &viivaR);
                        d = Ultra_GetDistance();
                        print_mqtt("Zumo024/x,y", "%d %d", x, y);
                        moveWhileLine(viivaL, viivaR);
                        followLine();
                        x++;
                        d = Ultra_GetDistance();

                        while(x != 1){
                            checkViiva(&viivaL, &viivaR);
                            followLine();
                            checkViiva(&viivaL, &viivaR);
                            moveWhileLine(viivaL, viivaR);
                            x++;
                        }
                        checkViiva(&viivaL, &viivaR);
                        suunta = 0;
                        turnUntilLine(0);
                        reverseToLine(viivaL, viivaR);
                        d = Ultra_GetDistance();
                    }
                    else if(viivaL && suunta == 0 && d < 20 && x == 3){
                        turnUntilLine(0);
                        checkViiva(&viivaL, &viivaR);
                        reverseToLine(viivaL, viivaR);
                        d = Ultra_GetDistance();
                        checkViiva(&viivaL, &viivaR);
                        moveWhileLine(viivaL, viivaR);
                        checkViiva(&viivaL, &viivaR);
                        followLine();
                        x--;
                        print_mqtt("Zumo024/x,y", "%d %d", x, y);
                        d = Ultra_GetDistance();
                        checkViiva(&viivaL, &viivaR);

                        while(x != -1){
                            moveWhileLine(viivaL, viivaR);
                            checkViiva(&viivaL, &viivaR);
                            followLine();
                        }
                        turnUntilLine(1);
                        suunta = 0;
                        reverseToLine(viivaL, viivaR);
                        d = Ultra_GetDistance();
                    }
                }
                else if(viivaL || viivaR){
                    if(suunta == 0){
                        if(d < 20){
                            if(x <= 0 && x >= -2){
                                turnUntilLine(0);
                                checkViiva(&viivaL, &viivaR);
                                suunta = -1;
                                reverseToLine(viivaL, viivaR);
                                checkViiva(&viivaL, &viivaR);
                                d = Ultra_GetDistance();

                                if(d < 20){
                                    motor_rotate90_right();
                                    motor_rotate90_right();
                                    suunta = 1;
                                    checkViiva(&viivaL, &viivaR);
                                    d = Ultra_GetDistance();
                                    reverseToLine(viivaL, viivaR);
                                    
                                    if((viivaL || viivaR) && d < 40 && suunta == 1){
                                    turnUntilLine(1);//suunta alaspäin
                                    reverseToLine(viivaL, viivaR);
                                    followLine(); //followLine tokalle viivalle asti
                                    z = 0;

                                    while(z != -2){
                                        followLine();
                                        checkViiva(&viivaL, &viivaR);

                                        if(viivaL || viivaR){
                                            y--;
                                            z--;
                                            print_mqtt("Zumo024/x,y,z", "%d %d %d", x, y, z);
                                            moveWhileLine(viivaL, viivaR);
                                            checkViiva(&viivaL, &viivaR);
                                        }
                                    }
                                    d = Ultra_GetDistance();

                                    if(z == -2){
                                        turnUntilLine(1);
                                        suunta = -1;
                                    }
                                    reverseToLine(viivaL, viivaR);
                                    d = Ultra_GetDistance();

                                    while(x != -3){
                                        checkViiva(&viivaL, &viivaR);
                                        x--;
                                        print_mqtt("Zumo024/x,y", "%d %d", x, y);
                                        moveWhileLine(viivaL, viivaR);
                                        followLine();
                                    }

                                    if(x == -3){
                                        turnUntilLine(1);
                                        checkViiva(&viivaL, &viivaR);
                                        reverseToLine(viivaL, viivaR);
                                        checkViiva(&viivaL, &viivaR);
                                        suunta = 0;
                                    }
                                    print_mqtt("Zumo024/x,y,z", "%d %d %d", x, y, z);
                                    d = Ultra_GetDistance();
                                    }
                                }
                            }
                            else if(x == 1 || x == 2){
                                turnUntilLine(1);
                                checkViiva(&viivaL, &viivaR);
                                suunta = 1;
                                reverseToLine(viivaL, viivaR);
                                d = Ultra_GetDistance();
                                
                                if((viivaL || viivaR) && d < 20 && suunta == 1){
                                    motor_rotate90_left();
                                    motor_rotate90_left();
                                    suunta = -1;
                                    checkViiva(&viivaL, &viivaR);
                                    reverseToLine(viivaL, viivaR);
                                    d = Ultra_GetDistance();
                                    
                                    if((viivaL || viivaR) && d < 40 && suunta == -1){
                                        turnUntilLine(0);//suunta alaspäin
                                        checkViiva(&viivaL, &viivaR);
                                        moveWhileLine(viivaL, viivaR);
                                        checkViiva(&viivaL, &viivaR);
                                        followLine(); //followLine tokalle viivalle asti

                                        while(z != -2){
                                            followLine();
                                            checkViiva(&viivaL, &viivaR);

                                            if(viivaL || viivaR){
                                                y--;
                                                z--;
                                                print_mqtt("Zumo024/x,y,z", "%d %d %d", x, y, z);
                                                moveWhileLine(viivaL, viivaR);
                                            }
                                        }
                                        d = Ultra_GetDistance();

                                        if(z == -2){
                                            turnUntilLine(0);
                                            suunta = -1;
                                            reverseToLine(viivaL, viivaR);
                                        }
                                        d = Ultra_GetDistance();
                                        checkViiva(&viivaL, &viivaR);
                                        moveWhileLine(viivaL, viivaR);
                                        checkViiva(&viivaL, &viivaR);
                                        followLine();

                                        while(x != 3){
                                            checkViiva(&viivaL, &viivaR);
                                            print_mqtt("Zumo024/x,y", "%d %d", x, y);
                                            moveWhileLine(viivaL, viivaR);
                                            checkViiva(&viivaL, &viivaR);
                                            followLine();
                                            x++;
                                        }
                                        turnUntilLine(0);
                                        reverseToLine(viivaL, viivaR);
                                        suunta = 0;
                                        z = 0;
                                        print_mqtt("Zumo024/x,y,z", "%d %d %d", x, y, z);
                                        d = Ultra_GetDistance();
                                    }
                                }
                            }
                        }
                        else if(d >= 20){
                            checkViiva(&viivaL, &viivaR);
                            moveWhileLine(viivaL, viivaR);
                            checkViiva(&viivaL, &viivaR);
                            followLine();
                            y++;
                            print_mqtt("Zumo024/x,y", "%d %d", x, y);
                            d = Ultra_GetDistance();
                        }
                    }
                    else if(suunta == -1){
                        if(x == -3){
                            turnUntilLine(1);
                            reverseToLine(viivaL, viivaR);
                            suunta = 0;
                        }

                        if(d < 20){
                            
                        }
                        else if(d >= 20){
                            if(x == 1 || x == 2){
                                checkViiva(&viivaL, &viivaR);
                                moveWhileLine(viivaL, viivaR);
                                checkViiva(&viivaL, &viivaR);
                                followLine();
                                x--;
                                print_mqtt("Zumo024/x,y", "%d %d", x, y);
                                turnUntilLine(1);
                                suunta = 0;
                                checkViiva(&viivaL, &viivaR);
                                reverseToLine(viivaL, viivaR);
                                motor_forward(0,0);
                                d = Ultra_GetDistance();
                            }
                            else if(x <= 0 && x >= -2){
                                checkViiva(&viivaL, &viivaR);
                                moveWhileLine(viivaL, viivaR);
                                checkViiva(&viivaL, &viivaR);
                                followLine();
                                x--;
                                print_mqtt("Zumo024/x,y", "%d %d", x, y);
                                turnUntilLine(1);
                                suunta = 0;
                                checkViiva(&viivaL, &viivaR);
                                reverseToLine(viivaL, viivaR);
                                checkViiva(&viivaL, &viivaR);
                                motor_forward(0,0);
                                d = Ultra_GetDistance();
                            }
                        }
                    }
                    else if(suunta == 1){
                        if(x == 3){
                            turnUntilLine(0);
                            reverseToLine(viivaL, viivaR);
                            suunta = 0;
                        }

                        if(d >= 20){
                            if(x <= 0 && x >= -2){
                                checkViiva(&viivaL, &viivaR);
                                moveWhileLine(viivaL, viivaR);
                                checkViiva(&viivaL, &viivaR);
                                followLine();
                                x++;
                                print_mqtt("Zumo024/x,y", "%d %d", x, y);
                                turnUntilLine(0);
                                suunta = 0;
                                checkViiva(&viivaL, &viivaR);
                                reverseToLine(viivaL, viivaR);
                                motor_forward(0,0);
                                d = Ultra_GetDistance();
                            }
                            else if(x == 1 || x == 2){
                                checkViiva(&viivaL, &viivaR);
                                moveWhileLine(viivaL, viivaR);
                                checkViiva(&viivaL, &viivaR);
                                followLine();
                                x++;
                                print_mqtt("Zumo024/x,y", "%d %d", x, y);
                                turnUntilLine(0);
                                suunta = 0;
                                checkViiva(&viivaL, &viivaR);
                                reverseToLine(viivaL, viivaR);
                                checkViiva(&viivaL, &viivaR);
                                motor_forward(0,0);
                                d = Ultra_GetDistance();
                            }
                        }
                    }
                }
            }//end of while y < 11
            while(y == 11){//while y is 11
                if(x < 0){
                    turnUntilLine(1);
                    checkViiva(&viivaL, &viivaR);
                    suunta = 1;
                    reverseToLine(viivaL, viivaR);
                    
                    while(x < 0){
                        checkViiva(&viivaL, &viivaR);
                        moveWhileLine(viivaL, viivaR);
                        checkViiva(&viivaL, &viivaR);
                        followLine();
                        x++;
                    }
                    turnUntilLine(0);
                }
                else if(x > 0){
                    turnUntilLine(0);
                    checkViiva(&viivaL, &viivaR);
                    suunta = -1;
                    reverseToLine(viivaL, viivaR);
                    
                    while(x > 0){
                        checkViiva(&viivaL, &viivaR);
                        moveWhileLine(viivaL, viivaR);
                        checkViiva(&viivaL, &viivaR);
                        followLine();
                        x--;
                    }
                    turnUntilLine(1);
                }
                checkViiva(&viivaL, &viivaR);
                moveWhileLine(viivaL, viivaR);
                checkViiva(&viivaL, &viivaR);
                followLine();
                y++;
            }
            while(y <= 13){//while y is 13
                checkViiva(&viivaL, &viivaR);
                moveWhileLine(viivaL, viivaR);
                checkViiva(&viivaL, &viivaR);
                followLine();
                y++;
            }
            checkViiva(&viivaL, &viivaR);
            moveWhileLine(viivaL, viivaR);
            checkViiva(&viivaL, &viivaR);
            followLine();
            stop = xTaskGetTickCount();
            motor_stop();
            print_mqtt("Zumo024/stop", "&d", stop);
            print_mqtt("Zumo024/time", "%d", stop -start);
            }//end of button
    }//end of for
}//end of main
#endif

/* [] END OF FILE */