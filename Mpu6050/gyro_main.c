/*
 * Author:
 *  - Andrew Espinoza
 *
 * Dependencies:
 *  - Jason Losh's tm4c123gh6pm related config and module libraries
 *
 * Target:
 *  - Tiva C TM4C123GH6PM Launchpad Evaluation Board
 *
 */


//C-std libraries
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//TM4 - config libraries
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "wait.h"
#include "uart0.h"
//TM4 - module libraries
#include "gpio.h"
#include "nvic.h"
#include "shell.h"
#include "i2c0.h"
//Device - libraries
#include "i2c0_lcd.h"
#include "mpu6050.h"


/* Pre-Processor Defines and Macros */


/* Globals */

MpuData data = {}, offset = {};
Vec3   accelP = {}, gyroP = {};

uint16_t pitch, roll, yaw;

/* Subroutines */


void initHw(void)
{
    initSystemClockTo40Mhz();

    //periodic timer to get gyro data at known delta t

    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;
    _delay_cycles(3);

    WTIMER0_CTL_R   &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);

    WTIMER0_CFG_R    = 0x4;

    WTIMER0_TAMR_R   = TIMER_TAMR_TAMR_1_SHOT
                     | TIMER_TAMR_TACDIR;
    WTIMER0_TBMR_R   = TIMER_TBMR_TBMR_PERIOD;

    WTIMER0_TBILR_R   = 40e6;

    WTIMER0_IMR_R    = TIMER_IMR_TBTOIM;

    WTIMER0_TAV_R    = 0;
    WTIMER0_TBV_R    = 0;

    WTIMER0_ICR_R |= TIMER_ICR_TBTOCINT;
    NVIC_EN2_R = (uint32_t)1 << (INT_WTIMER0B - 16 - 64);
}

void startTimer(double *dt)
{
    WTIMER0_TAV_R = 0;
    WTIMER0_CTL_R |= TIMER_CTL_TAEN;
}

void endTimer(double *dt)
{
    *dt = WTIMER0_TAV_R / 40e6;
    WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;
}

void displayIsr(void)
{

    char num[20];
    char buffer[21];

    itoa32(pitch, num);
    strcpyFill(buffer, num, 21, ' ');
    putsLcd(0,0, buffer);
    putsLcd(1, 0, "Pitch               ");
    putsLcd(2, 0, "Roll                ");
    putsLcd(3, 0, "Yaw                 ");
    WTIMER0_ICR_R |= TIMER_ICR_TBTOCINT;
}

void main(void)
{
    initHw();
    initShell(115200, 40e6);
    initI2c0();
    initLcd();
    initMpu6050(MPU_DEF_ADDR);

    double dt = 0, alpha = 0.9f;

    calculateOffset(&offset);

    putsUart0(CLEAR_SCREEN);
    putsUart0(HIDE_CURSOR);
    while(true)
    {
        //end timer
        endTimer(&dt);
        //getdata
        getMpuData(&data, &offset);
        //start the timer
        startTimer(&dt);
        //calculate gyro/accel positions
        getAccelPos(&data, &accelP);
        getGyroPos(&data, &gyroP, dt);
        WTIMER0_CTL_R |= TIMER_CTL_TBEN;
        //complementary filter output
        //ANGLE_n = (ANGLE_acc_n * ALPHA) + (ANGLE_n-1 + ANGLE_gyro_n)
        pitch = (alpha * accelP.x) + (1.0f - alpha) * (pitch + gyroP.x);
        roll  = (alpha * accelP.y) + (1.0f - alpha) * (roll  + gyroP.y);
        yaw   = (alpha * accelP.z) + (1.0f - alpha) * (roll  + gyroP.z);
        WTIMER0_CTL_R &= ~TIMER_CTL_TBEN;
    }
}
