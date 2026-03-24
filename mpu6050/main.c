/**
 * main.c
 */


/* INCLUDES */

//C-Std Lib.
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

//TM4C-Core
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "wait.h"
#include "nvic.h"

//TM4C-Peripheral drivers
#include "gpio.h"
#include "i2c1.h"
#include "uart0.h"

//Higher Level Peripheral Access
#include "shell.h"
#include "mpu6050.h"

/* GLOBALS CONSTS AND MACROS */

#define LED_G           PORTF,3
#define MPU6050_INT     PORTB,0


/* SUB ROUTINE PROTOTYPES */

void initHw(void);
void printData(MpuData m);
void printVec(Vec3f v);
float deltaSeconds(void);
Vec3f complimentaryFilter(MpuData m, float dt_s);
Vec3f gyroOffsets(void);

/* MAIN ROUTINE */
int main(void) {
    initHw();
    Vec3f g_ofs = gyroOffsets();

    MpuData m;
    for(;;) {
        putsUart0(SAVE_POS);

        m = mpu_read();
        //Correct gyro
        m.g.x -= g_ofs.x;
        m.g.y -= g_ofs.y;
        m.g.z -= g_ofs.z;

        printData(m);
        putsUart0("-----------------------------------------\n");

        printVec(complimentaryFilter(m, deltaSeconds()));

        putsUart0(RETURN_2_POS);
        waitMicrosecond(1e3);
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
    initI2c1();
    mpu_init();

    //Startup Sequence
    putsUart0(CLEAR_SCREEN);
    putsUart0(GOTO_HOME);
    putsUart0(HIDE_CURSOR);
    setPinValue(LED_G, 1);
    waitMicrosecond(500e3);
    setPinValue(LED_G, 0);

    //Stopwatch
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;
    _delay_cycles(3);

    WTIMER0_CTL_R  &= ~(TIMER_CTL_TAEN);
    WTIMER0_CFG_R   = 0x4;
    WTIMER0_TAMR_R  = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;
    WTIMER0_TAV_R   = 0;
    WTIMER0_CTL_R  |= TIMER_CTL_TAEN;

    //Interrupt Line
//    enablePort(PORTB);
//    selectPinDigitalInput(MPU6050_INT);
//    selectPinInterruptHighLevel(MPU6050_INT);
//    enablePinInterrupt(MPU6050_INT);
//    enableNvicInterrupt(INT_GPIOB);
}

float deltaSeconds(void) {
    uint32_t counts = WTIMER0_TAV_R;
    WTIMER0_TAV_R = 0;
    return ((float)counts) * .000000025;
}

void printData(MpuData m) {
    char buffer[100];
    usprintf(buffer, "Ax:%10f|Ay:%10f|Az:%10f|\n", m.a.x, m.a.y,m.a.z);
    putsUart0(buffer);
    usprintf(buffer, "Gx:%10f|Gy:%10f|Gz:%10f|\n", m.g.x, m.g.y,m.g.z);
    putsUart0(buffer);
}

void printVec(Vec3f v) {
    char buffer[100];
    usprintf(buffer, "X:%10f|Y:%10f|Z:%10f|\n", v.x, v.y, v.z);
    putsUart0(buffer);
}

#define C_ALPHA   0.8
Vec3f complimentaryFilter(MpuData m, float dt_s) {
    static Vec3f gyro_est = {0, 0, 0};
    Vec3f accel_est = {0 ,0, 0};
    //integrate Gyro
    gyro_est.x += m.g.x * dt_s;
    gyro_est.y += m.g.y * dt_s;
    gyro_est.z += m.g.z * dt_s;
    //Euler Angle from Accel
    accel_est.x = atan2f(m.a.y, sqrtf(m.a.x*m.a.x + m.a.z*m.a.z));
    accel_est.y = atan2f(m.a.x, sqrtf(m.a.y*m.a.y + m.a.z*m.a.z));
    accel_est.z = atan2f(sqrtf(m.a.x*m.a.x + m.a.y*m.a.y), m.a.z);
    return (Vec3f){
        .x= (C_ALPHA)*gyro_est.x + (1-C_ALPHA)*accel_est.x,
        .y= (C_ALPHA)*gyro_est.y + (1-C_ALPHA)*accel_est.y,
        .z= (C_ALPHA)*gyro_est.z + (1-C_ALPHA)*accel_est.z
    };
}

Vec3f gyroOffsets(void) {
    uint32_t i;
    MpuData m;
    Vec3f s = {0,0,0};
    for(i = 0; i < 1000; ++i) {
        m = mpu_read();
        s.x += m.g.x;
        s.y += m.g.y;
        s.z += m.g.z;
    }
    s.x /= 1000;
    s.y /= 1000;
    s.z /= 1000;
    return s;
}

