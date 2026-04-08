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
#include "qmc5883p.h"
#include "quaternion.h"

/* GLOBALS CONSTS AND MACROS */

#define LED_G           PORTF,3
#define MPU6050_INT     PORTB,0


/* SUB ROUTINE PROTOTYPES */

void initHw(void);
void printData(MpuData m, Vec3f mag);
void printVec(Vec3f v);
float deltaSeconds(void);
uint32_t start(void);
uint32_t stop(void);
float seconds(uint32_t counts);
Vec3f complimentaryFilter(MpuData m, float dt_s);
Vec3f madgwickFilter(MpuData m, Vec3f mag, float dt_s);
Vec3f gyroOffsets(void);
//TODO: magOffsets (calculate hard-iron bias by capturing measurements in all positions:
//  offset = 1/2(max + min)

/* MAIN ROUTINE */
int main(void) {
    initHw();
    Vec3f g_ofs = gyroOffsets();
    MpuData m;
    Vec3f mag;

    uint32_t begin, end;
    char buffer[100];
    int count = 100;

    for(;;) {
        putsUart0(SAVE_POS);

        m = mpu_read();

        if(count++ >= 20) {
            count = 0;
            mag = qmc_read();
        }

        //Correct gyro
        m.g.x -= g_ofs.x;
        m.g.y -= g_ofs.y;
        m.g.z -= g_ofs.z;

        float dt_s = deltaSeconds();
        Print("---------------------Raws-----------------", .bg=Gray);

        printData(m, mag);

        Print("-----------------Complimentary------------", .bg=Gray);

        begin = start();
        Vec3f c_att = complimentaryFilter(m, dt_s);
        end = stop();
        printVec(c_att);
        usprintf(buffer, "Time: %fus | Clocks: %d\n", seconds(end-begin)*1e6, end-begin);
        putsUart0(buffer);

        Print("-------------------Madgwick---------------", .bg=Gray);

        begin = start();
        Vec3f m_att = madgwickFilter(m, mag, dt_s);
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
    qmc_init();


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
        waitMicrosecond(1e3*2);
    }
    s.x /= 1000;
    s.y /= 1000;
    s.z /= 1000;
    return s;
}

void printData(MpuData m, Vec3f mag) {
    char buffer[100];
    usprintf(buffer, "Ax:%10f|Ay:%10f|Az:%10f|\n", m.a.x, m.a.y,m.a.z);
    putsUart0(buffer);
    usprintf(buffer, "Gx:%10f|Gy:%10f|Gz:%10f|\n", m.g.x, m.g.y,m.g.z);
    putsUart0(buffer);
    usprintf(buffer, "Mx:%10f|My:%10f|Mz:%10f|\n", mag.x, mag.y,mag.z);
    putsUart0(buffer);
}

void printVec(Vec3f v) {
    char buffer[100];
    usprintf(buffer, "X:%10f|Y:%10f|Z:%10f   |\n", v.x, v.y, v.z);
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

#define BETA_MADGWICK   1.5f
#define RAD2DEG         57.2957795131f
#define DEG2RAD         0.0174532925f

Vec3f madgwickFilter(MpuData mpu, Vec3f mag, float dt_s) {
//    static Quaternion q_est = {1.0f, 0.0f, 0.0f, 0.0f};   // w, x, y, z
//    Quaternion q_prev = q_est;
//
//    // Sensor data
//    Quaternion a = {1, mpu.a.x, mpu.a.y, mpu.a.z};
//    Quaternion g = {1, mpu.g.x * DEG2RAD, mpu.g.y * DEG2RAD, mpu.g.z * DEG2RAD};
//    Quaternion m = {1, mag.x, mag.y, mag.z};
//
//    // Normalise Accel and Mag Sensor Data
//    a = quaternion_normalize(a);
//    m = quaternion_normalize(m);
//
//    // Auxiliary products (direct access to q_prev components)
//    float q1 = q_prev.w, q2 = q_prev.x, q3 = q_prev.y, q4 = q_prev.z;
//    float _2q1 = 2.0f * q1, _2q2 = 2.0f * q2, _2q3 = 2.0f * q3, _2q4 = 2.0f * q4;
//    float _2q1q3 = 2.0f * q1 * q3;
//    float _2q3q4 = 2.0f * q3 * q4;
//    float q1q1 = q1*q1, q1q2 = q1*q2, q1q3 = q1*q3, q1q4 = q1*q4;
//    float q2q2 = q2*q2, q2q3 = q2*q3, q2q4 = q2*q4;
//    float q3q3 = q3*q3, q3q4 = q3*q4, q4q4 = q4*q4;
//
//    // Reference direction of Earth's magnetic field
//    float _2q1mx = 2.0f * q1 * m.x;
//    float _2q1my = 2.0f * q1 * m.y;
//    float _2q1mz = 2.0f * q1 * m.z;
//    float _2q2mx = 2.0f * q2 * m.x;
//    float hx = m.x*q1q1 - _2q1my*q4 + _2q1mz*q3 + m.x*q2q2 + _2q2*m.y*q3 + _2q2*m.z*q4 - m.x*q3q3 - m.x*q4q4;
//    float hy = _2q1mx*q4 + m.y*q1q1 - _2q1mz*q2 + _2q2mx*q3 - m.y*q2q2 + m.y*q3q3 + _2q3*m.z*q4 - m.y*q4q4;
//    float _2bx = sqrtf(hx*hx + hy*hy);
//    float _2bz = -_2q1mx*q3 + _2q1my*q2 + m.z*q1q1 + _2q2mx*q4 - m.z*q2q2 + _2q3*m.y*q4 - m.z*q3q3 + m.z*q4q4;
//    float _4bx = 2.0f * _2bx;
//    float _4bz = 2.0f * _2bz;
//
//    // Gradient of the objective function (s1..s4)
//    Quaternion grad = {
//                    -_2q3 * (2.0f*q2q4 - _2q1q3 - a.x)
//                    + _2q2 * (2.0f*q1q2 + _2q3q4 - a.y)
//                    - _2bz*q3 * (_2bx*(0.5f - q3q3 - q4q4) + _2bz*(q2q4 - q1q3) - m.x)
//                    + (-_2bx*q4 + _2bz*q2) * (_2bx*(q2q3 - q1q4) + _2bz*(q1q2 + q3q4) - m.y)
//                    + _2bx*q3 * (_2bx*(q1q3 + q2q4) + _2bz*(0.5f - q2q2 - q3q3) - m.z),
//
//                    _2q4 * (2.0f*q2q4 - _2q1q3 - a.x)
//                    + _2q1 * (2.0f*q1q2 + _2q3q4 - a.y)
//                    - 4.0f*q2 * (1.0f - 2.0f*q2q2 - 2.0f*q3q3 - a.z)
//                    + _2bz*q4 * (_2bx*(0.5f - q3q3 - q4q4) + _2bz*(q2q4 - q1q3) - m.x)
//                    + (_2bx*q3 + _2bz*q1) * (_2bx*(q2q3 - q1q4) + _2bz*(q1q2 + q3q4) - m.y)
//                    + (_2bx*q4 - _4bz*q2) * (_2bx*(q1q3 + q2q4) + _2bz*(0.5f - q2q2 - q3q3) - m.z),
//
//                    -_2q1 * (2.0f*q2q4 - _2q1q3 - a.x)
//                    + _2q4 * (2.0f*q1q2 + _2q3q4 - a.y)
//                    - 4.0f*q3 * (1.0f - 2.0f*q2q2 - 2.0f*q3q3 - a.z)
//                    + (-_4bx*q3 - _2bz*q1) * (_2bx*(0.5f - q3q3 - q4q4) + _2bz*(q2q4 - q1q3) - m.x)
//                    + (_2bx*q2 + _2bz*q4) * (_2bx*(q2q3 - q1q4) + _2bz*(q1q2 + q3q4) - m.y)
//                    + (_2bx*q1 - _4bz*q3) * (_2bx*(q1q3 + q2q4) + _2bz*(0.5f - q2q2 - q3q3) - m.z),
//
//                    _2q2 * (2.0f*q2q4 - _2q1q3 - a.x)
//                    + _2q3 * (2.0f*q1q2 + _2q3q4 - a.y)
//                    + (-_4bx*q4 + _2bz*q2) * (_2bx*(0.5f - q3q3 - q4q4) + _2bz*(q2q4 - q1q3) - m.x)
//                    + (-_2bx*q1 + _2bz*q3) * (_2bx*(q2q3 - q1q4) + _2bz*(q1q2 + q3q4) - m.y)
//                    + _2bx*q2 * (_2bx*(q1q3 + q2q4) + _2bz*(0.5f - q2q2 - q3q3) - m.z)
//    };
//
//    grad = quaternion_normalize(grad);
//
//    Quaternion omega = {0.0f, g.x, g.y, g.z};
//
//    // qDot = 0.5 * q * omega - beta * grad
//    Quaternion qDot = quaternion_sub(
//        quaternion_scalar(quaternion_hamilton(q_prev, omega), 0.5f),
//        quaternion_scalar(grad, BETA_MADGWICK)
//    );
//
//    // Integrate and normalise
//    Quaternion q_new = quaternion_add(q_prev, quaternion_scalar(qDot, dt_s));
//    q_est = quaternion_normalize(q_new);

//    static Quaternion q = {1.0f, 0.0f, 0.0f, 0.0f};
    static float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float ax = mpu.a.x, ay = mpu.a.y, az = mpu.a.z;
    float gx = mpu.g.x, gy = mpu.g.y, gz = mpu.g.z;
    float mx = mag.x, my = mag.y, mz = mag.z;

    float beta = 1.5;
    float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
    float norm;
    float hx, hy, _2bx, _2bz;
    float s1, s2, s3, s4;
    float qDot1, qDot2, qDot3, qDot4;

    // Auxiliary variables to avoid repeated arithmetic
    float _2q1mx;
    float _2q1my;
    float _2q1mz;
    float _2q2mx;
    float _4bx;
    float _4bz;
    float _2q1 = 2.0f * q1;
    float _2q2 = 2.0f * q2;
    float _2q3 = 2.0f * q3;
    float _2q4 = 2.0f * q4;
    float _2q1q3 = 2.0f * q1 * q3;
    float _2q3q4 = 2.0f * q3 * q4;
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q1q4 = q1 * q4;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q2q4 = q2 * q4;
    float q3q3 = q3 * q3;
    float q3q4 = q3 * q4;
    float q4q4 = q4 * q4;

    // Normalise accelerometer measurement
    norm = 1.0f / sqrtf(ax * ax + ay * ay + az * az);
    ax *= norm;
    ay *= norm;
    az *= norm;

    // Normalise magnetometer measurement
    norm = 1.0f / sqrtf(mx * mx + my * my + mz * mz);
    mx *= norm;
    my *= norm;
    mz *= norm;

    // Reference direction of Earth's magnetic field
    _2q1mx = 2.0f * q1 * mx;
    _2q1my = 2.0f * q1 * my;
    _2q1mz = 2.0f * q1 * mz;
    _2q2mx = 2.0f * q2 * mx;
    hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
    hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
    _2bx = sqrtf(hx * hx + hy * hy);
    _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;

    // Gradient decent algorithm corrective step
    s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    norm = sqrtf(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
    norm = 1.0f/norm;
    s1 *= norm;
    s2 *= norm;
    s3 *= norm;
    s4 *= norm;

    // Compute rate of change of quaternion
    qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
    qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
    qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
    qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

    // Integrate to yield quaternion
    q1 += qDot1 * dt_s;
    q2 += qDot2 * dt_s;
    q3 += qDot3 * dt_s;
    q4 += qDot4 * dt_s;
    norm = sqrtf(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
    norm = 1.0f/norm;
    q[0] = q1 * norm;
    q[1] = q2 * norm;
    q[2] = q3 * norm;
    q[3] = q4 * norm;

    Quaternion q_est = {q[0], q[1], q[2], q[3]};

    // Compute Euler angles (ZYX order)
    float r11 = 1 - 2*(q_est.y*q_est.y + q_est.z*q_est.z);
    float r21 = 2*(q_est.x*q_est.y - q_est.w*q_est.z);
    float r31 = 2*(q_est.x*q_est.z + q_est.w*q_est.y);
    float r32 = 2*(q_est.y*q_est.z - q_est.w*q_est.x);
    float r33 = 1 - 2*(q_est.x*q_est.x + q_est.y*q_est.y);

    return (Vec3f) {
        RAD2DEG * atan2f(r32, r33 + 1e-12f),
        RAD2DEG * asinf(-r31),
        RAD2DEG * atan2f(r21, r11)
    };
}
