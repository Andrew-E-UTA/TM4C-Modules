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
#include "i2c1.h"
#include "uart0.h"

//Higher Level Peripheral Access
#define SHELL_IMPLEMENTATION
#include "shell.h"
#define HMC5883L_IMPLEMENTATION
#include "HMC58883L.h"


/* GLOBALS CONSTS AND MACROS */
#define LED_G           PORTF,3
#define HMC_IRQ         PORTB,5

/* SUB ROUTINE PROTOTYPES */

void initHw(void);
void hmcIsr(void);

/* MAIN ROUTINE */
int main(void) {
    initHw();

    for(;;) {
    }
}

void hmcIsr(void) {
    togglePinValue(LED_G);
    clearPinInterrupt(HMC_IRQ);
}

void initHw(void) {
    initSystemClockTo40Mhz();

    //Status LED
    enablePort(PORTF);
    selectPinPushPullOutput(LED_G);

    enablePort(PORTB);
    selectPinDigitalInput(HMC_IRQ);
    selectPinInterruptRisingEdge(HMC_IRQ);
    enablePinInterrupt(HMC_IRQ);
    enableNvicInterrupt(INT_GPIOB);

    //Shell
    initUart0();
    setUart0BaudRate(115200, 40e6);

    //Magnetometer
    initI2c1();

    //Startup Sequence
    putsUart0(CLEAR_SCREEN);
    putsUart0(GOTO_HOME);
    putsUart0(HIDE_CURSOR);
    Print("Magno");
    setPinValue(LED_G, 1);
    waitMicrosecond(500e3);
    setPinValue(LED_G, 0);
}

