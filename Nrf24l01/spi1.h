// SPI1 library
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

#ifndef SPI1_H_
#define SPI1_H_

#define USE_SSI_FSS 1		// do we want the fss, either software or hardware controlled
#define USE_SSI_RX  2		// spi receive or not receieve

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initSpi1(uint32_t pinMask);
void setSpi1BaudRate(uint32_t clockRate, uint32_t fcyc);
void setSpi1Mode(uint8_t polarity, uint8_t phase);
void writeSpi1Data(uint16_t data);
uint16_t readSpi1Data();
bool spi1RxNotEmpty(void);

#endif
