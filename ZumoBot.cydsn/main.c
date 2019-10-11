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


// LINE FOLLOWER
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

            vTaskDelay(1000);

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


// ZUMO
#if 0
void zmain(void){
    struct sensors_ dig;
    struct accData_ data;
    int calX, calY;
    bool launch = true;
    bool viivaL = dig.l3 && dig.l2 && dig.l1;
    bool viivaR = dig.r3 && dig.r2 && dig.r1;
    
    IR_Start();
    LSM303D_Start();
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
                reflectance_digital(&dig);
                viivaL = dig.l3 && dig.l2 && dig.l1;
                viivaR = dig.r3 && dig.r2 && dig.r1;
                motor_forward(100,0);
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
                motor_forward(255,0);
                
                while(true){
                    while(launch){
                        motor_forward(255,100);
                        launch = false;
                    }
                    
                    reflectance_digital(&dig);
                    LSM303D_Read_Acc(&data);
                    
                    if(((data.accX - calX) > 10000) || (data.accX - calX) < -10000 || (data.accY - calY) > 10000 || (data.accY - calY) < -10000){
                        print_mqtt("Zumo024/hit", "%d", xTaskGetTickCount());
                    }
                    
                    if(!viivaL && !viivaR){
                        //rynnäkkö
                        motor_forward(255,0);
                    }

                    else if(viivaL){
                        //poistu takavasemmalle
                        motor_backward_turn(0,255,500);
                        motor_forward(0,0);
                    }

                    else if(viivaR){
                        //poistu takaoikealle
                        motor_backward_turn(255,0,500);
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

#if 1
// followLine();
void followLine(void){
    struct sensors_ dig;
    reflectance_digital(&dig);
    bool viivaL = dig.l3 && dig.l2 && dig.l1;
    bool viivaR = dig.r1 && dig.r2 && dig.r3;
    while(!viivaL || !viivaR){
        if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 0 && dig.r3 == 0){
            //suoraan
            reflectance_digital(&dig);
            viivaL = dig.l3 && dig.l2 && dig.l1;
            viivaR = dig.r1 && dig.r2 && dig.r3;
            motor_forward(75,0);
        }

        else if(dig.l3 == 0 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
            // vasen
            reflectance_digital(&dig);
            viivaL = dig.l3 && dig.l2 && dig.l1;
            viivaR = dig.r1 && dig.r2 && dig.r3;
            motor_turn(25,75,0);
        }

        else if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0){
            // oikee
            reflectance_digital(&dig);
            viivaL = dig.l3 && dig.l2 && dig.l1;
            viivaR = dig.r1 && dig.r2 && dig.r3;
            motor_turn(75,25,0);
        }
    }
    if(dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0){
        motor_forward(0,0);
    }
}

// reverseToLine();
void reverseToLine(void){
    struct sensors_ dig;
    reflectance_digital(&dig);
    bool viivaL = dig.l3 && dig.l2 && dig.l1;
    bool viivaR = dig.r1 && dig.r2 && dig.r3;
    while(!viivaL || !viivaR){
        reflectance_digital(&dig);
        viivaL = dig.l3 && dig.l2 && dig.l1;
        viivaR = dig.r1 && dig.r2 && dig.r3;
        motor_backward(75,0);
    }
}

// moveWhileLine();
void moveWhileLine(void){
    struct sensors_ dig;
    reflectance_digital(&dig);
    bool viivaL = dig.l3 && dig.l2 && dig.l1;
    bool viivaR = dig.r1 && dig.r2 && dig.r3;
    while(viivaL || viivaR){
        // kun viivalla, aja eteenpäin
        reflectance_digital(&dig);
        viivaL = dig.l3 && dig.l2 && dig.l1;
        viivaR = dig.r1 && dig.r2 && dig.r3;
        motor_forward(75,0);
    }
}

// moveAndCountLines(/*maxLines*/);
void moveAndCountLines(int maxLines){
    int lines = 0;
    while (lines < maxLines) {
        moveWhileLine();
        followLine();
        lines++;
    }
}

void zmain(void){
    struct sensors_ dig;
    int x = 0, d, y = 0;
    TickType_t start, stop;
    bool viivaL = dig.l3 && dig.l2 && dig.l1;
    bool viivaR = dig.r1 && dig.r2 && dig.r3;

    IR_Start();
    Ultra_Start();
    IR_flush();
    motor_start();
    
    reflectance_start();
    reflectance_set_threshold(12000, 12000, 12000, 12000, 12000, 12000);
    
    while(true){
        if(SW1_Read() == 0){
            while(SW1_Read() == 0){
                vTaskDelay(100);
            }
            
            followLine();
            print_mqtt("Zumo024/ready","maze");
            IR_wait();
            start = xTaskGetTickCount();
            print_mqtt("Zumo024/start","&d", start);
            moveAndCountLines(1);
            print_mqtt("Zumo024/position", "%d %d", x, y);

            while(y < 11){
                if(x <= 0 && x >= -3){
                    d = Ultra_GetDistance();
                    if(d < 20){
                        if(x == 3){
                            motor_turn_cross_right();
                            moveAndCountLines(2);
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
                                        moveAndCountLines(2);
                                        y -= 2;
                                        motor_turn_cross_left();
                                        if(x == 0){
                                            moveAndCountLines(3);
                                            x += 3;
                                        }
                                        else{
                                            moveAndCountLines(4);
                                            x += 4;
                                        }
                                        motor_turn_cross_left();
                                    }
                                    else{
                                        moveAndCountLines(1);
                                        x++;
                                        motor_turn_cross_right();
                                        moveAndCountLines(2);
                                        y -= 2;
                                        motor_turn_cross_left();
                                        if(y == 0){
                                            moveAndCountLines(3);
                                            y += 3;
                                        }
                                        else{
                                            moveAndCountLines(4);
                                            y += 4;
                                        }
                                        motor_turn_cross_left();
                                    }
                                }
                                else{
                                    moveAndCountLines(1);
                                    x++;
                                    motor_turn_cross_left();
                                }
                            }
                            else{
                                moveAndCountLines(1);
                                x--;
                                motor_turn_cross_right();
                            }
                        }
                        reverseToLine();
                    }
                    else{
                        moveAndCountLines(1);
                        y++;
                    }
                }
                else if(x > 0 && <= 3){
                    d = Ultra_GetDistance();
                    if(d < 20){
                        if( x == 3){
                            motor_turn_cross_left();
                            moveAndCountLines(2);
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
                                        moveAndCountLines(2);
                                        y -= 2;
                                        motor_turn_cross_right();
                                        moveAndCountLines(3);
                                        x -= 3;
                                        motor_turn_cross_right();
                                    }
                                    else{
                                        moveAndCountLines(1);
                                        x--;
                                        motor_turn_cross_left();
                                        moveAndCountLines(2);
                                        y -= 2;
                                        motor_turn_cross_right();
                                        moveAndCountLines(3);
                                        x -= 3;
                                        motor_turn_cross_right();
                                    }
                                }
                                else{
                                    moveAndCountLines(1);
                                    x--;
                                    motor_turn_cross_right();
                                }
                            }
                            else{
                                moveAndCountLines(1);
                                x++;
                                motor_turn_cross_left();
                            }
                        }
                        reverseToLine();
                    }
                    else{
                        moveAndCountLines(1);
                        y++;
                    }
                }
            }// end of while y < 11
            while(y == 11){
                if(x == 0){
                    motor_turn_cross_right();
                    while(x < 0){
                        moveAndCountLines(1);
                        x++;
                    }
                    motor_turn_cross_left();
                }
                else if(x > 0){
                    motor_turn_cross_left();
                    while(x > 0){
                        moveAndCountLines(1);
                        x++;
                    }
                    motor_turn_cross_left();
                }
                moveAndCountLines(1);
                y++;
            }//end of while == 11
            while( y <= 13){
                moveAndCountLines(1);
                y++;
            }
            followLine();
            motor_stop();
            stop = xTaskGetTickCount();
            print_mqtt("Zumo024/stop", "&d", stop);
            print_mqtt("Zumo024/time", "&d", stop - start);
        }//end of button
    }//end of while
}//end of main
#endif

/* [] END OF FILE */
