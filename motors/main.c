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
#include "uart0.h"

//Higher Level Peripheral Access
#define SHELL_IMPLEMENTATION
#include "shell.h"

/* GLOBALS CONSTS AND MACROS */
#define LED_G           PORTF,3

#define M1A PORTB,6     //M0 PWM 0  0A
#define M1B PORTB,4     //M0 PWM 2  1A
#define M1C PORTB,5     //M0 PWM 3  1B

#define AINP             PORTE,1
#define BINP             PORTE,2
#define CINP             PORTE,4

/* SUB ROUTINE PROTOTYPES */
void initHw(void);

void initAdc(void);
void readAdc0Ss0(int16_t* data);
void convert(int16_t* raws, uint16_t* norms);

void initBldc(void);
void setPwms(uint16_t a, uint16_t b, uint16_t c);

/* MAIN ROUTINE */
int main(void)
{
    initHw();

    int16_t raws[3];
    uint16_t pwms[3];


    for(;;) {
        readAdc0Ss0(raws);
        convert(raws, pwms);
        setPwms(pwms[0], 0,0);
        waitMicrosecond(100e3);
}

void initHw(void) {
    initSystemClockTo40Mhz();

    //Status LED
    enablePort(PORTF);
    selectPinPushPullOutput(LED_G);

    initBldc();
    initAdc();

    //Shell
    initUart0();
    setUart0BaudRate(115200, 40e6);

    //Startup Sequence
    putsUart0(CLEAR_SCREEN);
    putsUart0(GOTO_HOME);
    putsUart0(HIDE_CURSOR);
    Print("Motor");
    setPinValue(LED_G, 1);
    waitMicrosecond(500e3);
    setPinValue(LED_G, 0);


}

void initAdc(void) {
    //AINP             PORTE,1
    //BINP             PORTE,2
    //CINP             PORTE,4

    SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;
    enablePort(PORTE);

    selectPinAnalogInput(AINP);
    selectPinAnalogInput(BINP);
    selectPinAnalogInput(CINP);
    setPinAuxFunction(AINP, GPIO_PCTL_PE1_AIN2);
    setPinAuxFunction(BINP, GPIO_PCTL_PE2_AIN1);
    setPinAuxFunction(CINP, GPIO_PCTL_PE4_AIN9);


    //Disable ss0 for programming
    ADC0_ACTSS_R &= ~ ADC_ACTSS_ASEN0;
    //Use PLL Clock
    ADC0_CC_R = ADC_CC_CS_SYSPLL;
    //1Msps sample rate
    ADC0_PC_R = ADC_PC_SR_1M;
    ADC0_EMUX_R = ADC_EMUX_EM0_PROCESSOR;
    //Set HW sampling
    ADC0_SAC_R = ADC_SAC_AVG_16X;
#define ADC_CTL_DITHER 0x00000040
    ADC0_CTL_R |= ADC_CTL_DITHER;
    //Set the sample sequence to read all pins in a burst
    ADC0_SSMUX0_R = (2 << ADC_SSMUX0_MUX0_S) |  // PE1 AIN2 => AINP
                    (1 << ADC_SSMUX0_MUX1_S) |  // PE2 AIN1 => BINP
                    (9 << ADC_SSMUX0_MUX3_S);   // PE4 AIN9 => CINP
    //Mark fourth sample as the end (and trigger interrupt used for dma)
    ADC0_SSCTL0_R = ADC_SSCTL0_END2 | ADC_SSCTL0_IE2;
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN0;
}

void readAdc0Ss0(int16_t* data) {
    ADC0_PSSI_R |= ADC_PSSI_SS0;                    // set start bit
    while (ADC0_ACTSS_R & ADC_ACTSS_BUSY);          // wait until SS3 is not busy
    while (ADC0_SSFSTAT0_R & ADC_SSFSTAT0_EMPTY);
    uint8_t i = 0;
    for(;i < 3; ++i)
        data[i] = (int16_t)ADC0_SSFIFO0_R;          // get single result from the FIFO
}

//  1 << 16 / 4096 * 1024 = 16384
#define min(a,b) ((a)>(b)?b:a)
void convert(int16_t* raws, uint16_t* norms) {
    norms[0] = min(1023,(4096 - raws[0]) * 1024 / 4096);
    norms[1] = min(1023,(4096 - raws[1]) * 1024 / 4096);
    norms[2] = min(1023,(4096 - raws[2]) * 1024 / 4096);
}

void initBldc() {

    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
    enablePort(PORTB);
    selectPinPushPullOutput(M1A);
    setPinAuxFunction(M1A, GPIO_PCTL_PB6_M0PWM0);
    setPinAuxFunction(M1B, GPIO_PCTL_PB4_M0PWM2);
    setPinAuxFunction(M1C, GPIO_PCTL_PB5_M0PWM3);

    // Configure PWM module 1 to drive RGB LED
    //M0 PWM 0 -> pwm0A
    //M0 PWM 2 -> PWM1A
    //M0 PWM 3 -> PWM1B
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;
    SYSCTL_SRPWM_R = 0;

    PWM0_0_CTL_R = 0;
    PWM0_1_CTL_R = 0;

    PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTLOAD_ZERO;
    PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;
    PWM0_1_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;

    PWM0_0_LOAD_R = 1024;
    PWM0_1_LOAD_R = 1024;

    PWM0_0_CMPA_R = 0;
    PWM0_1_CMPB_R = 0;
    PWM0_1_CMPA_R = 0;

    PWM0_0_CTL_R = PWM_0_CTL_ENABLE;
    PWM0_1_CTL_R = PWM_1_CTL_ENABLE;
    PWM0_ENABLE_R = PWM_ENABLE_PWM0EN | PWM_ENABLE_PWM2EN | PWM_ENABLE_PWM3EN;
}

void setPwms(uint16_t a, uint16_t b, uint16_t c) {
    PWM0_0_CMPA_R = a;
    PWM0_1_CMPA_R = b;
    PWM0_1_CMPB_R = c;
}

