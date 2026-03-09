/**
 * main.c
 */


/* INCLUDES */

//C-Std Lib.
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

//TM4C-Core
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "wait.h"
#include "nvic.h"

//TM4C-Peripheral drivers
#include "gpio.h"
#include "i2c0.h"
#include "uart0.h"

//Higher Level Peripheral Access
#define SHELL_IMPLEMENTATION
#include "shell.h"
#define MPU6050_IMPLEMENTATION
#include "mpu6050.h"

/* GLOBALS CONSTS AND MACROS */

#define LED_G           PORTF,3
#define MPU6050_INT     PORTB,0

typedef struct { float x, y, z; } Vec3f;

Vec3 gyroOffset;
Vec3 a_l;
Vec3 a_h;
//AccelCalibrations accelCal;

/* SUB ROUTINE PROTOTYPES */

#define uprintf(s, ...) \
    do {\
    char buffer[100];\
    snprintf(buffer, sizeof(buffer), s, __VA_ARGS__);\
    putsUart0(buffer);\
    putcUart0('\n');\
    } while(0);

void initHw(void);

void calibrate();

/* MAIN ROUTINE */
int main(void) {
    initHw();

    calibrate();

    float ax, ay, az;
    float gx, gy, gz;
    for(;;) {
        putsUart0(SAVE_POS);
        ax = mpu_convert(mpu_accel.x);
        ay = mpu_convert(mpu_accel.y);
        az = mpu_convert(mpu_accel.z);
        uprintf("ax: %10g", ax);
        uprintf("ay: %10g", ay);
        uprintf("az: %10g", az);

        gx = mpu_convert(mpu_gyro.x - gyroOffset.x);
        gy = mpu_convert(mpu_gyro.y - gyroOffset.y);
        gz = mpu_convert(mpu_gyro.z - gyroOffset.z);
        uprintf("gx: %10g", gx);
        uprintf("gy: %10g", gy);
        uprintf("gz: %10g", gz);

        putsUart0(RETURN_2_POS);
        waitMicrosecond(100e3);
    }
}


/* SUB ROUTINES */

void initHw(void) {
    initSystemClockTo40Mhz();

    //Status LED
    enablePort(PORTF);
    selectPinPushPullOutput(LED_G);

    //Shell
    initUart0();
    setUart0BaudRate(115200, 40e6);

    //Sensor
    initI2c0();
    mpu_init();

    //Startup Sequence
    putsUart0(CLEAR_SCREEN);
    putsUart0(GOTO_HOME);
    putsUart0(HIDE_CURSOR);
    setPinValue(LED_G, 1);
    waitMicrosecond(500e3);
    setPinValue(LED_G, 0);

    //Interrupt Line
    enablePort(PORTB);
    selectPinDigitalInput(MPU6050_INT);


    selectPinInterruptHighLevel(MPU6050_INT);
    enablePinInterrupt(MPU6050_INT);
    enableNvicInterrupt(INT_GPIOB);
}

void calibrate() {
    disablePinInterrupt(MPU6050_INT);
    uprintf("Calibrating Gyroscopes%c", ' ');
    uprintf("<Enter> To Calibrate Gyro\nLeave Resting%c",' ');
    while(1) {
        char c = getcUart0();
        if(13 == c || 10 == c) break;
    }
    mpu_read_avg_gyro(&gyroOffset);
    enablePinInterrupt(MPU6050_INT);
    //    size_t i = 0;
    //    char* labels[] = {"+Z", "-Z", "+Y", "-Y", "+X", "-X"};
    //    char* desc[] = {"Leave Resting", "Upside Down", "Nose Up", "Nose Down", "Right Up", "Left Up"};
    //
    //    uprintf("Calibrating %s", "Accelerometer")
    //    for(;i < 6; ++i) {
    //        Vec3 tmp;
    //        uprintf("<Enter> To Calibrate %s", labels[i]);
    //        uprintf("%s", desc[i]);
    //        while(1) {
    //            char c = getcUart0();
    //            if(13 == c || 10 == c) break;
    //        }
    //        mpu_read_avg_acc(&tmp);
    //        switch (i) {
    //        case 0: a_h.z = tmp.z; break;
    //        case 1: a_l.z = tmp.z; break;
    //        case 2: a_h.y = tmp.y; break;
    //        case 3: a_l.y = tmp.y; break;
    //        case 4: a_h.x = tmp.x; break;
    //        case 5: a_l.x = tmp.x; break;
    //        }
    //    }
    //    mpu_calibrate(&a_l, &a_h, &accelCal);
}

void risingEdgeIsr(void) {
    mpu_callback();
    clearPinInterrupt(MPU6050_INT);
}

void delayMs(uint16_t m) {
    waitMicrosecond(m * 1000);
}
