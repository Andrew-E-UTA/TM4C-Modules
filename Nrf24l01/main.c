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

/* Pre-Processor Defines and Macros */

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
#include "spi1.h"

//Device - libraries
#include "nrf24l01.h"

//GPIO Defines
#define GREEN       PORTF,3

/* Globals */

uint8_t addr[] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};

/* Subroutines */

void initGpio(void)
{
    enablePort(PORTF);
    selectPinPushPullOutput(GREEN);
    setPinValue(GREEN, 0);
    return;
}
/* Main Routine */

int main(void)
{
    USER_DATA data;
    const char* str;
    char buffer[50];
    uint8_t reg, byte;

    //HW setup
    initSystemClockTo40Mhz();
    initGpio();

    //Spi setup
    initSpi1(USE_SSI_RX);
    setSpi1BaudRate(1e6, 40e6);
    setSpi1Mode(0,0);

    //Uart setup
    initUart0();
    setUart0BaudRate(19200, 40e6);
    initScreen();

    //Nrf24 setup
    initNrfGpio(false);
    configDefaultNrf24();
    setNrfTxMode(addr, 10);

	while(1)
	{
	    if(!kbhitUart0()) continue;
	    getsUart0(&data);
	    parseFields(&data);
	    if(isCommand(&data, "read", 1) && data.fieldType[1] == 'n')
	    {
	        byte = getFieldInt(&data, 1);

	        byte = nrf24ReadReg(byte);

            sprintf(buffer, "Data: %hu", byte);
            putsUart0(buffer);
            newline();
	    }
	    else if(isCommand(&data, "write", 2))
	    {
	        reg = getFieldInt(&data, 1);
            byte = getFieldInt(&data, 2);

            nrf24WriteReg(reg, byte);
	    }
	    else if(isCommand(&data, "send", 1))
	    {
	        str = getFieldString(&data, 1);
	        if(nrf24Transmit((uint8_t*)str))
	        {
	            setPinValue(GREEN, 1);
	            waitMicrosecond(500e3);
                setPinValue(GREEN, 0);
                waitMicrosecond(500e3);
	        }
	    }
	}
}
