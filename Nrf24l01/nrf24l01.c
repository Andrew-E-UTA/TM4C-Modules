/*
 *
 *
 *
 *
 *
 */


/* Pre-Processor Defines and Macros */

#include <stdint.h>
#include <stdbool.h>

#include "tm4c123gh6pm.h"
#include "wait.h"
#include "gpio.h"
#include "nvic.h"
#include "nrf24l01.h"
#include "spi1.h"

//SPI Chip select
#define CS          PORTD,1
//Interrupts configurable on status of FIFOS
#define IRQ         PORTE,1
//Aloows to exit from standby
#define CE          PORTE,2


/* Globals */

/* Subroutines */

void irqIsr(void)
{
    //handle irq
    clearPinInterrupt(IRQ);
}


void initNrfGpio(bool interrupts)
{
    enablePort(PORTE);
    selectPinPushPullOutput(CE);
    setPinValue(CE, 0);

    enablePort(PORTD);
    selectPinPushPullOutput(CS);

    if(interrupts)
    {
        selectPinDigitalInput(IRQ);
        selectPinInterruptFallingEdge(IRQ);
        enablePinInterrupt(IRQ);
        clearPinInterrupt(IRQ);
        enableNvicInterrupt(INT_GPIOE);
    }
}

void configDefaultNrf24(void)
{
    //disable chip for programming
    setPinValue(CE, 0);

    //config will be setup for specific TX/RX mode
    //Disbale CRC and Interrupts
    nrf24WriteReg(NRF_CONFIG,       0x00);

    nrf24WriteReg(NRF_EN_AA ,       ENAA_NO_AA);
    nrf24WriteReg(NRF_EN_RXADDR,    ERX_NO_PIPES);

    nrf24WriteReg(NRF_SETUP_AW,     AW_5BYTE);
    nrf24WriteReg(NRF_SETUP_RETR,   RETR_NO_RETR);
    nrf24WriteReg(NRF_RF_CH,        0x00);
    nrf24WriteReg(NRF_RF_SETUP,     RFSETUP_RF_DR | RFSETUP_RF_PWR0 | RFSETUP_LNA_HCURR);

    setPinValue(CE, 1);
}

void setNrfTxMode(uint8_t *pipeAddr, uint8_t channel)
{
    uint8_t config;

    //disable chip for programming
    setPinValue(CE, 0);

    nrf24WriteReg(NRF_RF_CH, channel);
    nrf24WriteRegMulti(NRF_TX_ADDR, pipeAddr, 5);

    config  = nrf24ReadReg(NRF_CONFIG);
    config |= CONFIG_PWR_UP;
    nrf24WriteReg(NRF_CONFIG, config);

    setPinValue(CE, 1);
}

//TODO: setup
void setNrfRxMode(void)
{

}

bool nrf24Transmit(uint8_t *data)
{
    uint8_t i;
    uint8_t status;

    setPinValue(CS, 0);

    writeSpi1Data(W_TX_PAYLOAD);
    for(i = 0; i < MAX_TX_LEN; i++)
        writeSpi1Data(data[i]);

    setPinValue(CS, 1);

    //Allow data to settle
    waitMicrosecond(1e3);

    //Ensure that the data was successfully transmitted
    status = nrf24ReadReg(NRF_FIFO_STATUS);
    if((status & FIFO_STAT_TX_EMPTY) && !(status & FIFO_STAT_CHECK))
    {
        nrf24SendCommand(FLUSH_TX);
        return true;
    }
    return false;
}


void nrf24WriteReg(uint8_t reg, uint8_t data)
{
    reg &= ~0xE0;
    reg |= 1 << 5;
    setPinValue(CS, 0);

    writeSpi1Data(reg);
    writeSpi1Data(data);

    setPinValue(CS,1);
}

void nrf24WriteRegMulti(uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t i;

    setPinValue(CS, 0);

    writeSpi1Data(reg);
    for(i = 0; i < len; i++)
        writeSpi1Data(data[i]);

    setPinValue(CS, 1);
}


uint8_t nrf24ReadReg(uint8_t reg)
{
    uint8_t data;

    reg &= ~0xE0;

    setPinValue(CS,0);

    while(spi1RxNotEmpty())
        readSpi1Data();

    writeSpi1Data(reg);
    readSpi1Data();
    writeSpi1Data(0x00);
    data = readSpi1Data();

    setPinValue(CS,1);

    return data;
}

void nrf24SendCommand(uint8_t cmmd)
{
    setPinValue(CS, 0);

    writeSpi1Data(cmmd);

    setPinValue(CS, 1);


}
