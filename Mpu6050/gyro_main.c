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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
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
#include "mpu6050.h"


/* Pre-Processor Defines and Macros */

#define SYS_CLOCK_FREQ  40e6
#define SAMPLE_RATE     500.0f


/* Globals */

MpuData data = {}, offset = {};
Pos3d   accelPos = {}, gyroPos = {};

/* Subroutines */


void initHw(void)
{
    initSystemClockTo40Mhz();

    //periodic timer to get gyro data at known delta t

    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;
    _delay_cycles(3);

    WTIMER0_CTL_R &= ~ TIMER_CTL_TAEN;
    WTIMER0_CFG_R = 0x4;
    WTIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD | TIMER_TAMR_TACDIR;
    WTIMER0_IMR_R = TIMER_IMR_TATOIM;
    //mpu is configured for 2kHz sampling rate : be safe and sample at 1.6kHz
    WTIMER0_TAILR_R = SYS_CLOCK_FREQ / SAMPLE_RATE;
    WTIMER0_TAV_R = 0;
    WTIMER0_CTL_R = TIMER_CTL_TAEN;

}
void pollValidAddresses(void)
{
    uint8_t addr;
    char buffer[20];

    for(addr = 0x8; addr <= 0x77; addr++)
    {
        if(pollI2c0Address(addr))
        {
            htoa(addr, buffer);
            putsUart0("Addr: ");
            putsUart0(buffer);
            putsUart0(" is valid\n");
        }
    }
}

void printData(MpuData *data)
{
    char buffer[80];

    putsUart0(SAVE_POS);
    putsUart0(GOTO_HOME);
    //Print to Uart the readings
    putsUart0("Accelerometer Readings:\n");
    snprintf(buffer, 50, "AX = %10.2lf", data->accel.x);
    putsUart0(buffer);
    putsUart0("\n");
    snprintf(buffer, 50, "AY = %10.2lf", data->accel.y);
    putsUart0(buffer);
    putsUart0("\n");
    snprintf(buffer, 50, "AZ = %10.2lf", data->accel.z);
    putsUart0(buffer);
    putsUart0("\n");


    putsUart0("Gyroscope Readings:\n");
    snprintf(buffer, 50, "GX = %10.2lf", data->gyro.x);
    putsUart0(buffer);
    putsUart0("\n");
    snprintf(buffer, 50, "GY = %10.2lf", data->gyro.y);
    putsUart0(buffer);
    putsUart0("\n");
    snprintf(buffer, 50, "GZ = %10.2lf", data->gyro.z);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0("Temperature Reading:\n");
    snprintf(buffer, 50, "Temp (C) = %10.2lf", data->temp.c);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(RETURN_2_POS);

}

void printPos3d(Pos3d *position)
{
    char buffer[80];

    putsUart0(SAVE_POS);
    putsUart0(GOTO_HOME);
    //Print to Uart the readings
    putsUart0("Position Readings:\n");
    snprintf(buffer, 50, "Pitch = %10.2lf", position->pitch);
    putsUart0(buffer);
    putsUart0("\n");
    snprintf(buffer, 50, "Roll = %10.2f", (double)position->roll);
    putsUart0(buffer);
    putsUart0("\n");
    snprintf(buffer, 50, "Yaw = %10.2lf", position->yaw);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(RETURN_2_POS);
}

void readMpuDataIsr(void )
{
    WTIMER0_ICR_R = TIMER_ICR_TATOCINT;

    //read mpu
    getMpuData(&data, &offset);
    //calculate GyroPos
    getGyroPos(&data, &gyroPos,(double) 1 / SAMPLE_RATE);
}

void main(void)
{
    initHw();
    initShell(115200, 40e6);
    initI2c0();

    if(!pollI2c0Address(0x68))
    {
        putsUart0("Couldn't communicate to mpu6050...\n");
        while(true);
    }

    initMpu6050(MPU_DEF_ADDR);

    putsUart0(CLEAR_SCREEN);
    putsUart0(GOTO_HOME);
    putsUart0(HIDE_CURSOR);

    calculateOffset(&offset);
    waitMicrosecond(1e6);

    //start reading data from mpu at regular interval
    enableNvicInterrupt(INT_WTIMER0A);

    while(true)
    {
        printPos3d(&gyroPos);
        waitMicrosecond(200e3);
    }
}

