// SPI1 Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// SPI1 Interface:
//   MOSI on PD3 (SSI1Tx)
//   MISO on PD2 (SSI1Rx)
//   ~CS on PD1  (SSI1Fss)
//   SCLK on PD0 (SSI1Clk)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "spi1.h"
#include "gpio.h"

// Pins
#define SSI1TX PORTD,3		// transmt
#define SSI1RX PORTD,2		// receive
#define SSI1FSS PORTD,1		// frame/chip select
#define SSI1CLK PORTD,0		// clock

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize SPI1
void initSpi1(uint32_t pinMask)
{
    // Enable clocks
    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R1;
    _delay_cycles(3);
    enablePort(PORTD);

    // Configure SSI1 pins for SPI configuration
    selectPinPushPullOutput(SSI1TX);				// sets transmit pin to be output
    setPinAuxFunction(SSI1TX, GPIO_PCTL_PD3_SSI1TX);		// set as spi - aux function
    selectPinPushPullOutput(SSI1CLK);				// sets clock pin to be outout
    setPinAuxFunction(SSI1CLK, GPIO_PCTL_PD0_SSI1CLK);		// sets as clock - aux function
    selectPinPushPullOutput(SSI1FSS);				// frame select is output
    if (pinMask & USE_SSI_FSS)					// is the hardware controlling fss
    {
        setPinAuxFunction(SSI1FSS, GPIO_PCTL_PD1_SSI1FSS);	// set fss to be an aux function
    }
    if (pinMask & USE_SSI_RX)					// is there a receiev pin
    {
        selectPinDigitalInput(SSI1RX);				// if so, set it to be an input
        setPinAuxFunction(SSI1RX, GPIO_PCTL_PD2_SSI1RX);	// receive pin is aux function
    }

    // Configure the SSI1 as a SPI master, mode 3, 8bit operation
    SSI1_CR1_R &= ~SSI_CR1_SSE;                        // turn off SSI1 to allow re-configuration
    SSI1_CR1_R = 0;                                    // select master mode in control reg, 3rd bit
    SSI1_CC_R = 0;                                     // select system clock as the clock source
						       // set to 0x05 for PIOSC (precision oscillator)
    SSI1_CR0_R = SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8;     // set SR=0, 8-bit
						       // set Freescale SPI Frame Format &
						       // to 8 bit SSI Data Size 
}

// Set baud rate as function of instruction cycle frequency
void setSpi1BaudRate(uint32_t baudRate, uint32_t fcyc)
{
    uint32_t divisorTimes2 = (fcyc * 2) / baudRate;    // calculate divisor (r) times 2
    SSI1_CR1_R &= ~SSI_CR1_SSE;                        // turn off SSI1 to allow re-configuration
    SSI1_CPSR_R = (divisorTimes2 + 1) >> 1;            // round divisor to nearest integer
    SSI1_CR1_R |= SSI_CR1_SSE;                         // turn on SSI1
}

// Set mode
void setSpi1Mode(uint8_t polarity, uint8_t phase)
{
    SSI1_CR1_R &= ~SSI_CR1_SSE;                        // turn off SSI1 to allow re-configuration
    SSI1_CR0_R &= ~(SSI_CR0_SPH | SSI_CR0_SPO);        // set SPO and SPH as appropriate
						       // set as first edge capture, SPh = 0
						       // set as  steady state Low for clock, SPO = 0
    if (polarity)				// if the polarity is set, set SPO
    {
        SSI1_CR0_R |= SSI_CR0_SPO;		// set SPO = 1
        enablePinPullup(SSI1CLK);		// clock must be pullup if SPO = 1
    }
    else					// if polarity is off then disable pull up to save power
        disablePinPullup(SSI1CLK);		
    if (phase)					// if phase has been set, set SPH
        SSI1_CR0_R |= SSI_CR0_SPH;

    SSI1_CR1_R |= SSI_CR1_SSE;                         // turn on SSI1
}

// Blocking function that writes data and waits until the tx buffer is empty
void writeSpi1Data(uint16_t data)
{
    SSI1_DR_R = data;
    while (SSI1_SR_R & SSI_SR_BSY);
}

// Reads data from the rx buffer after a write
uint16_t readSpi1Data()
{
    return SSI1_DR_R;
}

bool spi1RxNotEmpty(void)
{
    return SSI1_SR_R & SSI_SR_RNE;
}
