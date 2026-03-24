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
#include "quaternion.h"

/* GLOBALS CONSTS AND MACROS */

#define LED_G           PORTF,3
#define MPU6050_INT     PORTB,0


/* SUB ROUTINE PROTOTYPES */

void initHw(void);
void printData(MpuData m);
void printVec(Vec3f v);
float deltaSeconds(void);
uint32_t start(void);
uint32_t stop(void);
float seconds(uint32_t counts);
Vec3f complimentaryFilter(MpuData m, float dt_s);
Vec3f madgwickFilter(MpuData m, float dt_s);
Vec3f gyroOffsets(void);

/* MAIN ROUTINE */
int main(void) {
    initHw();
    Vec3f g_ofs = gyroOffsets();
    MpuData m;

    uint32_t begin, end;
    char buffer[100];
    for(;;) {
        putsUart0(SAVE_POS);

        m = mpu_read();
        //Correct gyro
        m.g.x -= g_ofs.x;
        m.g.y -= g_ofs.y;
        m.g.z -= g_ofs.z;

        float dt_s = deltaSeconds();
        putsUart0("---------------------Raws-----------------\n");

        printData(m);

        putsUart0("-----------------Complimentary------------\n");

        begin = start();
        Vec3f c_att = complimentaryFilter(m, dt_s);
        end = stop();
        printVec(c_att);
        usprintf(buffer, "Time: %fus | Clocks: %d\n", seconds(end-begin)*1e6, end-begin);
        putsUart0(buffer);

        putsUart0("-------------------Madgwick---------------\n");

        begin = start();
        Vec3f m_att = madgwickFilter(m, dt_s);
        end = stop();
        printVec(m_att);
        usprintf(buffer, "Time: %fus | Clocks: %d\n", seconds(end-begin)*1e6, end-begin);
        putsUart0(buffer);

        putsUart0(RETURN_2_POS);
        waitMicrosecond(1e3*2);
    }
}


/* SUB ROUTINES */

void initHw(void) {
    initSystemClockTo40Mhz();
    initSystemClockTo80Mhz();

    //Status LED
    enablePort(PORTF);
    selectPinPushPullOutput(LED_G);

    //Shell
    initUart0();
    setUart0BaudRate(115200, 80e6);

    //Sensor
    initI2c1();
    mpu_init();

    //Startup Sequence
    putsUart0(CLEAR_SCREEN);
    putsUart0(GOTO_HOME);
    putsUart0(HIDE_CURSOR);
    setPinValue(LED_G, 1);
    waitMicrosecond(500e3*2);
    setPinValue(LED_G, 0);

    //Stopwatch
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;
    _delay_cycles(3);

    WTIMER0_CTL_R  &= ~(TIMER_CTL_TAEN);
    WTIMER0_CFG_R   = 0x4;
    WTIMER0_TAMR_R  = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;
    WTIMER0_TAV_R   = 0;
    WTIMER0_CTL_R  |= TIMER_CTL_TAEN;

    WTIMER0_CTL_R  &= ~(TIMER_CTL_TBEN);
    WTIMER0_CFG_R   = 0x4;
    WTIMER0_TBMR_R  = TIMER_TBMR_TBMR_1_SHOT | TIMER_TBMR_TBCDIR;
    WTIMER0_TBV_R   = 0;
    WTIMER0_CTL_R  |= TIMER_CTL_TBEN;

    //Interrupt Line
//    enablePort(PORTB);
//    selectPinDigitalInput(MPU6050_INT);
//    selectPinInterruptHighLevel(MPU6050_INT);
//    enablePinInterrupt(MPU6050_INT);
//    enableNvicInterrupt(INT_GPIOB);
}

//#define COUNT2SEC .0000000250 //40MHz
#define COUNT2SEC .0000000125 //80MHz
float deltaSeconds(void) {
    uint32_t counts = WTIMER0_TAV_R;
    WTIMER0_TAV_R = 0;
    return ((float)counts) * COUNT2SEC;
}

uint32_t start(void) {
    return WTIMER0_TBV_R;
}

uint32_t stop(void) {
    uint32_t counts = WTIMER0_TBV_R;
    WTIMER0_TBV_R = 0;
    return counts;
}

float seconds(uint32_t counts) {
    return ((float)counts) * COUNT2SEC;
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

void printData(MpuData m) {
    char buffer[100];
    usprintf(buffer, "Ax:%10f|Ay:%10f|Az:%10f|       \n", m.a.x, m.a.y,m.a.z);
    putsUart0(buffer);
    usprintf(buffer, "Gx:%10f|Gy:%10f|Gz:%10f|       \n", m.g.x, m.g.y,m.g.z);
    putsUart0(buffer);
}

void printVec(Vec3f v) {
    char buffer[100];
    usprintf(buffer, "X:%10f|Y:%10f|Z:%10f|          \n", v.x, v.y, v.z);
    putsUart0(buffer);
}

#define C_ALPHA   0.95
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

#define M_BETA       0.0866025404
#define RAD2DEG     57.2957795131
#define DEG2RAD      0.0174532925
Vec3f madgwickFilter(MpuData m, float dt_s) {
    static Quaternion q_est = {1,0,0,0};
    Quaternion q_prev = q_est;

    //Estimate
    Quaternion q_accel = {0, m.a.x, m.a.y, m.a.z};
    q_accel = quaternion_normalize(q_accel);
    Quaternion q_gyro = {0, m.g.x*.5, m.g.y*.5, m.g.z*.5};
    q_gyro = quaternion_scalar(q_gyro, DEG2RAD);
    q_gyro = quaternion_hamilton(q_prev, q_gyro);

    //Objective Function
    float f[3] = {
          2*(q_prev.x*q_prev.z - q_prev.w*q_prev.y) - q_accel.x,
          2*(q_prev.w*q_prev.x + q_prev.y*q_prev.z) - q_accel.y,
          2*(0.5 - q_prev.x*q_prev.x - q_prev.y*q_prev.y) - q_accel.z,
    };

    float j[3][4] =  {
          {-2 * q_prev.y, 2 * q_prev.z, -2 * q_prev.w, 2 * q_prev.x},
          {2 * q_prev.x, 2 * q_prev.w, 2 * q_prev.z, 2 * q_prev.y},
          {0, -4 * q_prev.x, -4 * q_prev.y, 0},
    };

    //Gradiant (J' * F)
    float beta = M_BETA;
    Quaternion q_grad = {
         j[0][0]*f[0] + j[1][0]*f[1] + j[2][0]*f[2],
         j[0][1]*f[0] + j[1][1]*f[1] + j[2][1]*f[2],
         j[0][2]*f[0] + j[1][2]*f[1] + j[2][2]*f[2],
         j[0][3]*f[0] + j[1][3]*f[1] + j[2][3]*f[2]
    };
    q_grad = quaternion_normalize(q_grad);
    q_grad = quaternion_scalar(q_grad, beta);
    q_est = quaternion_add(q_prev, quaternion_scalar(quaternion_sub(q_gyro, q_grad), dt_s));

    // Rotation matrix (body to world, assuming ZYX order)
    float r11 = 1 - 2*(q_est.y*q_est.y + q_est.z*q_est.z);
    float r21 = 2*(q_est.x*q_est.y - q_est.w*q_est.z);
    float r31 = 2*(q_est.x*q_est.z + q_est.w*q_est.y);
    float r32 = 2*(q_est.y*q_est.z - q_est.w*q_est.x);
    float r33 = 1 - 2*(q_est.x*q_est.x + q_est.y*q_est.y);

    return (Vec3f) {
        RAD2DEG * atan2f(r32, r33+ 1e-12),
        RAD2DEG * asinf(-r31),
        RAD2DEG * atan2f(r21, r11)
    };
}
