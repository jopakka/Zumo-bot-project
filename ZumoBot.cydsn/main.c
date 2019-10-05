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

#if 0
// Hello World!
void zmain(void)
{
    printf("\nHello, World!\n");

    while(true)
    {
        vTaskDelay(100); // sleep (in an infinite loop)
    }
 }   
#endif

#if 0
// Name and age
void zmain(void)
{
    char name[32];
    int age;
    
    
    printf("\n\n");
    
    printf("Enter your name: ");
    //fflush(stdout);
    scanf("%s", name);
    printf("Enter your age: ");
    //fflush(stdout);
    scanf("%d", &age);
    
    printf("You are [%s], age = %d\n", name, age);

    while(true)
    {
        BatteryLed_Write(!SW1_Read());
        vTaskDelay(100);
    }
 }   
#endif


#if 0
//battery level//
void zmain(void)
{
    ADC_Battery_Start();        

    int16 adcresult =0;
    float volts = 0.0;

    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed

    while(true)
    {
        char msg[80];
        ADC_Battery_StartConvert(); // start sampling
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for ADC converted value
            adcresult = ADC_Battery_GetResult16(); // get the ADC value (0 - 4095)
            // convert value to Volts
            // you need to implement the conversion
            
            // Print both ADC results and converted value
            printf("%d %f\r\n",adcresult, volts);
        }
        vTaskDelay(500);
    }
 }   
#endif

#if 0
// button
void zmain(void)
{
    while(true) {
        printf("Press button within 5 seconds!\n");
        int i = 50;
        while(i > 0) {
            if(SW1_Read() == 0) {
                break;
            }
            vTaskDelay(100);
            --i;
        }
        if(i > 0) {
            printf("Good work\n");
            while(SW1_Read() == 0) vTaskDelay(10); // wait until button is released
        }
        else {
            printf("You didn't press the button\n");
        }
    }
}
#endif

#if 0
// button
void zmain(void)
{
    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed
    
    bool led = false;
    
    while(true)
    {
        // toggle led state when button is pressed
        if(SW1_Read() == 0) {
            led = !led;
            BatteryLed_Write(led);
            if(led) printf("Led is ON\n");
            else printf("Led is OFF\n");
            Beep(100, 50);
            while(SW1_Read() == 0) vTaskDelay(10); // wait while button is being pressed
        }        
    }
 }   
#endif


#if 0
//ultrasonic sensor//
void zmain(void)
{
    Ultra_Start();                          // Ultra Sonic Start function
    
    while(true) {
        int d = Ultra_GetDistance();
        // Print the detected distance (centimeters)
        printf("distance = %d\r\n", d);
        vTaskDelay(200);
    }
}   
#endif

#if 0
//IR receiverm - how to wait for IR remote commands
void zmain(void)
{
    IR_Start();
    
    printf("\n\nIR test\n");
    
    IR_flush(); // clear IR receive buffer
    printf("Buffer cleared\n");
    
    bool led = false;
    // Toggle led when IR signal is received
    while(true)
    {
        IR_wait();  // wait for IR command
        led = !led;
        BatteryLed_Write(led);
        if(led) printf("Led is ON\n");
        else printf("Led is OFF\n");
    }    
 }   
#endif



#if 0
//IR receiver - read raw data
void zmain(void)
{
    IR_Start();
    
    uint32_t IR_val; 
    
    printf("\n\nIR test\n");
    
    IR_flush(); // clear IR receive buffer
    printf("Buffer cleared\n");
    
    // print received IR pulses and their lengths
    while(true)
    {
        if(IR_get(&IR_val, portMAX_DELAY)) {
            int l = IR_val & IR_SIGNAL_MASK; // get pulse length
            int b = 0;
            if((IR_val & IR_SIGNAL_HIGH) != 0) b = 1; // get pulse state (0/1)
            printf("%d %d\r\n",b, l);
        }
    }    
 }   
#endif


#if 0
//reflectance
void zmain(void)
{
    struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    

    while(true)
    {
        // read raw sensor values
        reflectance_read(&ref);
        // print out each period of reflectance sensors
        printf("%5d %5d %5d %5d %5d %5d\r\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);       
        
        // read digital values that are based on threshold. 0 = white, 1 = black
        // when blackness value is over threshold the sensors reads 1, otherwise 0
        reflectance_digital(&dig); 
        //print out 0 or 1 according to results of reflectance period
        printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);        
        
        vTaskDelay(200);
    }
}   
#endif


#if 0
//motor
void zmain(void)
{
    motor_start();              // enable motor controller
    motor_forward(0,0);         // set speed to zero to stop motors

    vTaskDelay(3000);
    
    motor_forward(100,2000);     // moving forward
    motor_turn(200,50,2000);     // turn
    motor_turn(50,200,2000);     // turn
    motor_backward(100,2000);    // moving backward
     
    motor_forward(0,0);         // stop motors

    motor_stop();               // disable motor controller
    
    while(true)
    {
        vTaskDelay(100);
    }
}
#endif

#if 0
/* Example of how to use te Accelerometer!!!*/
void zmain(void)
{
    struct accData_ data;
    
    printf("Accelerometer test...\n");

    if(!LSM303D_Start()){
        printf("LSM303D failed to initialize!!! Program is Ending!!!\n");
        vTaskSuspend(NULL);
    }
    else {
        printf("Device Ok...\n");
    }
    
    while(true)
    {
        LSM303D_Read_Acc(&data);
        printf("%8d %8d %8d\n",data.accX, data.accY, data.accZ);
        vTaskDelay(50);
    }
 }   
#endif    

#if 0
// MQTT test
void zmain(void)
{
    int ctr = 0;

    printf("\nBoot\n");
    send_mqtt("Zumo024/debug", "Boot");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 

    while(true)
    {
        printf("Ctr: %d, Button: %d\n", ctr, SW1_Read());
        print_mqtt("Zumo024/debug", "Ctr: %d, Button: %d", ctr, SW1_Read());

        vTaskDelay(1000);
        ctr++;
    }
 }   
#endif


#if 0
void zmain(void)
{    
    struct accData_ data;
    struct sensors_ ref;
    struct sensors_ dig;
    
    printf("MQTT and sensor test...\n");

    if(!LSM303D_Start()){
        printf("LSM303D failed to initialize!!! Program is Ending!!!\n");
        vTaskSuspend(NULL);
    }
    else {
        printf("Accelerometer Ok...\n");
    }
    
    int ctr = 0;
    reflectance_start();
    while(true)
    {
        LSM303D_Read_Acc(&data);
        // send data when we detect a hit and at 10 second intervals
        if(data.accX > 1500 || ++ctr > 1000) {
            printf("Acc: %8d %8d %8d\n",data.accX, data.accY, data.accZ);
            print_mqtt("Zumo01/acc", "%d,%d,%d", data.accX, data.accY, data.accZ);
            reflectance_read(&ref);
            printf("Ref: %8d %8d %8d %8d %8d %8d\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);       
            print_mqtt("Zumo01/ref", "%d,%d,%d,%d,%d,%d", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);
            reflectance_digital(&dig);
            printf("Dig: %8d %8d %8d %8d %8d %8d\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);
            print_mqtt("Zumo01/dig", "%d,%d,%d,%d,%d,%d", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);
            ctr = 0;
        }
        vTaskDelay(10);
    }
 }   

#endif

#if 0
void zmain(void)
{    
    RTC_Start(); // start real time clock
    
    RTC_TIME_DATE now;

    // set current time
    now.Hour = 12;
    now.Min = 34;
    now.Sec = 56;
    now.DayOfMonth = 25;
    now.Month = 9;
    now.Year = 2018;
    RTC_WriteTime(&now); // write the time to real time clock

    while(true)
    {
        if(SW1_Read() == 0) {
            // read the current time
            RTC_DisableInt(); /* Disable Interrupt of RTC Component */
            now = *RTC_ReadTime(); /* copy the current time to a local variable */
            RTC_EnableInt(); /* Enable Interrupt of RTC Component */

            // print the current time
            printf("%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
            
            // wait until button is released
            while(SW1_Read() == 0) vTaskDelay(50);
        }
        vTaskDelay(50);
    }
 }   
#endif

#if 0
//Joonas epic rotation//
void zmain(void)
{
    motor_start();
    rotate90_left(1);
    rotate90_right(1);
    motor_stop();
}   
#endif

#if 0
//Week 2, assignment 1
void zmain(void){
    BatteryLed_Write(0);
    bool led = false;
    int delay[] = {500,500,500,500,500,500,
                   500,1500,500,1500,500,1500,
                   500,500,500,500,500,500,};
    
    while(true)
    {
        // toggle led state when button is pressed
        if(SW1_Read() == 0) {
            led = !led;
            if(led){
                for(int i = 0; i < 18; i++){
                    led = !led;
                    BatteryLed_Write(led);
                    vTaskDelay(delay[i]);
                }
                BatteryLed_Write(0);
            }
        }
    }
}
#endif

#if 0
//Week 2, assignment 2
void zmain(void){
    int age;
    TickType_t aika1;
    TickType_t aika2;
    TickType_t totalAika;
    
    printf("Enter your age: ");
    aika1 = xTaskGetTickCount();
    scanf("%d", &age);
    aika2 = xTaskGetTickCount();
    
    totalAika = aika2 - aika1;
    
    if(age <= 21){
      if(totalAika < 3000){
        printf("Super fast dude!\n");
      }
      else if(totalAika >= 3000 && totalAika <= 5000 ){
        printf("So mediocre.\n");
      }
      else{
        printf("My granny is faster than you!\n");
      }
    }
    else if(age >= 22 && age <= 50){
      if(totalAika < 3000){
        printf("Be quick or be dead\n");
      }
      else if(totalAika >= 3000 && totalAika <= 5000 ){
        printf("You're so average\n");
      }
      else{
        printf("Have you been smoking something illegal?\n");
      }
    }
    else{
      if(totalAika < 3000){
        printf("Still going strong\n");
      }
      else if(totalAika >= 3000 && totalAika <= 5000 ){
        printf("You are doing ok for your age\n");
      }
      else{
        printf("Do they still allow you to drive?\n");
      }
    }
    while(true){
        vTaskDelay(1000);
    }
    
}
#endif

#if 0
//Week 2, assignment 3
void zmain(void){
    ADC_Battery_Start();
    
    BatteryLed_Write(0);

    int16 adcresult =0;
    float volts = 0.0;

    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed

    while(true)
    {
        ADC_Battery_StartConvert();
        
        adcresult = ADC_Battery_GetResult16(); // get the ADC value (0 - 4095)
        volts = adcresult / 4095.0 * 5.0;// convert value to Volts
        volts *= 1.75;
        if(volts < 4.0){
            while(SW1_Read() == 1){
                BatteryLed_Write(1);
                vTaskDelay(500);
                BatteryLed_Write(0);
                vTaskDelay(500);
            }
            BatteryLed_Write(0);
            printf("vmp\n");
            vTaskDelay(2000);
            
        }
        else{
            BatteryLed_Write(0);
            printf("%f\n", volts);
            vTaskDelay(2000);
        }
        vTaskDelay(2000);
    }
}
#endif

#if 0
//Week 3, assignment 1
void zmain(void){
    for(;;){
        if(SW1_Read() == 0){
            motor_start();              // enable motor controller
            motor_forward(0,0);         // set speed to zero to stop motors

            vTaskDelay(1000);

            motor_forward(200,2000);     // moving forward
            motor_rotate90_right(1);
            
            motor_forward(200,1600);     // moving forward
            motor_rotate90_right(1);
            
            motor_forward(200,1800);
            motor_rotate90_right(1);
            
            motor_forward(0,0);
            motor_turn(200,150,2500);

            motor_stop();               // disable motor controller
        }
    }
}
#endif

#if 0
//Week 3, assignment 2
void zmain(void){
    for(;;){
        if(SW1_Read() == 0){
            Ultra_Start();              // Ultra Sonic Start function
            int d = Ultra_GetDistance();
            motor_start();              // enable motor controller
            motor_forward(0,0);         // set speed to zero to stop motors

            vTaskDelay(1000);
            
            /*LOOP:*/for(;;){
                d = Ultra_GetDistance();
                if(d < 10){
                    motor_forward(0,0);
                    Beep(1000, 255);
                    motor_backward_turn(100,200,800);
                    motor_forward(0,0);
                }
                while(d >= 10){
                    motor_forward(100,100);
                    /*goto LOOP;*/
                }
            }
            motor_stop();
        }
    }
}
#endif

#if 0
//Week 3, assingment 3
void zmain(){
    int turns[] = {100,200};
    size_t n = sizeof(turns) / sizeof(turns[0]);
    struct accData_ data;
    LSM303D_Start();
    motor_start();
    
    for(;;){
        if(SW1_Read() == 0){
            vTaskDelay(1000);
            
            while(true){
                srand(xTaskGetTickCount());
                
                while(data.accX < 1750){
                    int x = rand() % n;
                    int y = rand() % n;
                    
                    motor_forward(150,50);
                    LSM303D_Read_Acc(&data);
                
                    motor_turn(turns[x],turns[y],300);
                    LSM303D_Read_Acc(&data);
                }
                
                if(rand() % 2){
                    motor_backward(200,800);
                    motor_rotate90_left();
                    LSM303D_Read_Acc(&data);
                }
                else{
                    motor_backward(200,800);
                    motor_rotate90_right();
                    LSM303D_Read_Acc(&data);
                }
            }
        }
    }
}
#endif

#if 0
//Week 4, assignment 1
void zmain(void){
    int maxLines = 4;
    struct sensors_ dig;
    IR_Start();
    IR_flush();
    
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 9000, 9000, 9000, 9000); // set center sensor threshold to 9000 and others to 9000
    
    for(;;){
        if(SW1_Read() == 0){
            int r = 0, l = 0;
            motor_start();
            vTaskDelay(1000);
            
            while(r < 1 || l < 1){
                motor_forward(100,100);
                reflectance_digital(&dig);
                r = r + dig.r2;
                l = l + dig.l2;
                motor_forward(0,0);
            }
            
            IR_wait();
            
            while(r < maxLines || l < maxLines){
                reflectance_digital(&dig);
                while(dig.l2 == 1 && dig.r2 == 1){
                    motor_forward(100,0);
                    reflectance_digital(&dig);
                }
                motor_forward(100,0);
                r = r + dig.r2;
                l = l + dig.l2;
                motor_forward(0,0);
            }
            motor_stop();
        }
    }
}
#endif

#if 0
//Week 4, assignment 2
void zmain(void){
    struct sensors_ dig;
    IR_Start();
    
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 9000, 9000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    
    
    for(;;){
        if(SW1_Read() == 0){
            vTaskDelay(1000);
            int r = 0, l = 0;
            int wait = 0;
            motor_start();
            IR_flush();
            int maxTime = 125;
            
            while(r < 1 || l < 1){
                motor_forward(100,1);
                reflectance_digital(&dig);
                r = r + dig.r3;
                l = l + dig.l3;
                motor_forward(0,0);
            }
            
            IR_wait();
            
            while(r < 2 || l < 2){
                motor_forward(100,1);
                wait++;
                if(wait >= maxTime){
                    reflectance_digital(&dig);
                    r = r + dig.r3;
                    l = l + dig.l3;
                }
                motor_forward(0,0);
            }
            
            motor_turn_cross_left(60,200,500);
            motor_turn_cross_right(200,60,420);
            
            wait = 0;
            
            while(r < 3 || l < 3){
                motor_forward(100,1);
                wait++;
                if(wait >= maxTime){
                    reflectance_digital(&dig);
                    r = r + dig.r3;
                    l = l + dig.l3;
                }
                motor_forward(0,0);
            }
            
            motor_turn_cross_right(200,60,450);
            
            while(r < 4 || l < 4){
                motor_forward(100,1);
                wait++;
                if(wait >= maxTime){
                    reflectance_digital(&dig);
                    r = r + dig.r3;
                    l = l + dig.l3;
                }
                motor_forward(0,0);
            }
            
            motor_stop();
        }
    }
}
#endif

#if 0
//Week 4, assignment 3
void zmain(void){
    struct sensors_ dig;
    IR_Start();
    
    reflectance_start();
    //reflectance_set_threshold(11000, 9000, 9000, 8000, 11000, 11000); // set center sensor threshold to 11000 and others to 9000
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    
    for(;;){
        if(SW1_Read() == 0){
            vTaskDelay(1000);
            int lines = 0;
            int wait = 0;
            
            IR_flush();
            motor_start();
            
            while(lines == 0){
                motor_forward(100,0);
                reflectance_digital(&dig);
                motor_forward(0,0);
                
                if(dig.l3 == 1 && dig.r3 == 1){
                    lines++;
                }
            }
            
            IR_wait();
            
            while(lines == 1){
                reflectance_digital(&dig);
                
                wait++;
                
                if(dig.l2 == 1 && dig.r2 == 1 && wait >= 100){
                    lines++;
                }
                else if(dig.l1 && dig.r1){
                    //eteenpäin
                    motor_forward(150,1);
                }
                else if(dig.l1 == 1 && dig.l2 == 0 && dig.r1 == 0){
                    //vähän oikeelle ja eteen
                    motor_turn(255,50,1);
                }

                else if(dig.r1 == 1 && dig.r2 == 0 && dig.l1 == 0){
                    //vähän vasemmalle ja eteen
                    motor_turn(50,255,1);
                }

                else if(dig.l1 == 1 && dig.l2 == 1){
                    //oikeelle
                    motor_turn(255,25,1);
                }

                else if(dig.r1 == 1 && dig.r2 == 1){
                    //vasemmalle
                    motor_turn(25,255,1);
                }

                else if(dig.l2 == 1 && dig.l3 == 1){
                    //kovaa oikee
                    motor_turn(255,10,1);
                }

                else if(dig.r2 == 1 && dig.r3 == 1){
                    //kovaa vasen
                    motor_turn(10,255,1);
                }

                else if(dig.r3 == 1){
                    //super kovaa vasen
                    motor_turn(255,0,1);
                }

                else if(dig.l3 == 1){
                    // super kovaa oikee
                    motor_turn(0,255,1);
                }
            }
            
            motor_stop();
        }
    }
}
#endif

#if 0
//Week 5, assingment 1
void zmain(void){

    RTC_Start(); // start real time clock
    RTC_TIME_DATE now;
    // set current time
    now.Sec = 0;
    now.DayOfMonth = 25;
    now.Month = 9;
    now.Year = 2018;

    int h, m;

    vTaskDelay(15000);
    LOOPH:printf("Enter current time(hours only):");
        scanf("%d", &h);
        now.Hour = h;

    if(h < 0 || h > 23){
            printf("Enter correct hours (0-23)\n");
            goto LOOPH;
    }
    LOOPM:printf("Enter current time(minutes only):");
            scanf("%d", &m);
            now.Min = m;

            if(m < 0 || m > 59){
                printf("Enter correct minutes (0-59)\n");
                goto LOOPM;
            }




    RTC_WriteTime(&now); // write the time to real time clock
    while(true){
         if(SW1_Read() == 0) {
             // read the current time
             RTC_DisableInt(); /* Disable Interrupt of RTC Component */
             now = *RTC_ReadTime(); /* copy the current time to a local variable */
             RTC_EnableInt(); /* Enable Interrupt of RTC Component */
             // print the current time
             print_mqtt("Zumo024/", "%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);

             // wait until button is released
             while(SW1_Read() == 0) vTaskDelay(50);
             }
             vTaskDelay(50);
    }


}
#endif

#if 0
//Week 5, assignment 2
void zmain(void){
    for(;;){
        if(SW1_Read() == 0){
            Ultra_Start();              // Ultra Sonic Start function
            int d;
            motor_start();              // enable motor controller
            motor_forward(0,0);         // set speed to zero to stop motors

            vTaskDelay(1000);
            
            for(;;){
                srand(xTaskGetTickCount());
                d = Ultra_GetDistance();
                motor_forward(255,1);
                
                if(d < 15){
                    motor_forward(0,0);
                    Beep(1000, 255);
                    motor_backward(255,500);
                    if(rand() % 2 == 0){
                        motor_rotate90_left();
                        print_mqtt("Zumo024/", "Left");
                    }
                    else{
                        motor_rotate90_right();
                        print_mqtt("Zumo024/", "Right");
                    }
                }
            }
            motor_stop();
        }
    }
}
#endif

#if 1
//255, 241, 227, 214, 202, 191, 180, 170, 160, 151, 143, 135, 127, 119, 113, 106, 100, 94
//A1   A1#  B1   C1   C1#  D1   D1#  E1   F1   F1#  G1   G1#  A2   A2#   B2  C2   C2#  D2
void zmain(void){
    //int notes[18] = {255, 241, 227, 214, 202, 191, 180, 170, 160, 151, 143, 135, 127, 119, 113, 106, 100, 94};
    jaateloauto();
    
    /*for(int i = 0; i < 18; i++){
        Beep(200, notes[i]);
    }*/
    
    
    while(true){
        vTaskDelay(100);
    }
}
#endif
/* [] END OF FILE */
