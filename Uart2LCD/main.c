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

/* Pre-Processor Defines and Macros */


/* Globals */

/* Subroutines */

/* Main Routine */

int main(void)
{
    initSystemClockTo40Mhz();
    initUart0();
    setUart0BaudRate(115200, 40e6);
    initLcd();

    USER_DATA data;

    while(true)
    {
        getsUart0(&data);
        putsLcd(0,0,data.buff);
    }
}
