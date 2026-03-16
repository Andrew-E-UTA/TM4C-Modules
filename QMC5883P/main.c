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
#define QMC5883P_IMPLEMENTATION
#include "qmc5883p.h"


/* GLOBALS CONSTS AND MACROS */
#define LED_G           PORTF,3
#define QMC_IRQ         PORTB,5

bool data_ready = false;

/* SUB ROUTINE PROTOTYPES */

void initHw(void);
void qmcIsr(void);

/* MAIN ROUTINE */
int main(void) {
    initHw();

    char buffer[100];
    i16Vec3 raws;

    uint8_t addrs[] = {9, 10, 11};
    uint8_t i = 0;
    for(;i < 3; ++i) {
        usprintf(buffer, "Reg[%x]: %x\n", addrs[i], readI2c1Register(QMC5883P_ADDR, addrs[i]));
        putsUart0(buffer);
    }


    for(;;) {
        //Ran every Cycle


        if(!data_ready) continue;
        data_ready = false;
        qmc_clear_int();    //clear device interrupt
        setPinValue(LED_G, 1);
        raws = qmc_read();

        putsUart0(SAVE_POS);

        usprintf(buffer, "x: %d\n", raws.x);
        putsUart0(buffer);
        usprintf(buffer, "y: %d\n", raws.y);
        putsUart0(buffer);
        usprintf(buffer, "z: %d\n", raws.z);
        putsUart0(buffer);

        putsUart0(RETURN_2_POS);
        setPinValue(LED_G, 0);
        waitMicrosecond(100e3);
    }
}

void qmcIsr(void) {
    data_ready = true;
    clearPinInterrupt(QMC_IRQ);
}

void initHw(void) {
    initSystemClockTo40Mhz();

    //Status LED
    enablePort(PORTF);
    selectPinPushPullOutput(LED_G);

    enablePort(PORTB);
    selectPinDigitalInput(QMC_IRQ);
    selectPinInterruptRisingEdge(QMC_IRQ);
    enablePinInterrupt(QMC_IRQ);
    enableNvicInterrupt(INT_GPIOB);

    //Shell
    initUart0();
    setUart0BaudRate(115200, 40e6);

    //Magnetometers
    initI2c1();

    if(!pollI2c1Address(QMC5883P_ADDR)) {
        Print("[Error]: Magno not detected...", .fg=Red);
        for(;;);
    }
    if(!qmc_verify()) {
        Print("[Error]: Invalid Chip ID", .fg=Red);
        for(;;);
    }
    qmc_init();

    //Startup Sequence
    putsUart0(CLEAR_SCREEN);
    putsUart0(GOTO_HOME);
    putsUart0(HIDE_CURSOR);
    Print("Magno");
    setPinValue(LED_G, 1);
    waitMicrosecond(500e3);
    setPinValue(LED_G, 0);
}

