#ifndef NRF24L01_H_
#define NRF24L01_H_

/* REGISTER MAP */

#define NRF_CONFIG      0x00
#define NRF_EN_AA       0x01
#define NRF_EN_RXADDR   0x02
#define NRF_SETUP_AW    0x03
#define NRF_SETUP_RETR  0x04
#define NRF_RF_CH       0x05
#define NRF_RF_SETUP    0x06
#define NRF_STATUS      0x07
#define NRF_OBSERVE_TX  0x08
#define NRF_CD          0x09
#define NRF_RX_ADDR_P0  0x0A
#define NRF_RX_ADDR_P1  0x0B
#define NRF_RX_ADDR_P2  0x0C
#define NRF_RX_ADDR_P3  0x0D
#define NRF_RX_ADDR_P4  0x0E
#define NRF_RX_ADDR_P5  0x0F
#define NRF_TX_ADDR     0x10
#define NRF_RX_PW_0     0x11
#define NRF_RX_PW_1     0x12
#define NRF_RX_PW_2     0x13
#define NRF_RX_PW_3     0x14
#define NRF_RX_PW_4     0x15
#define NRF_RX_PW_5     0x16
#define NRF_FIFO_STATUS 0x17
#define NRF_DYNPD       0x1C
#define NRF_FEATURE     0x1D

/* REGISTER BIT DESCRIPTIONS */

#define CONFIG_MASK_RX_DR   0x40    //Interrupt not reflected in IRQB pin
#define CONFIG_MASK_TX_DS   0x20    //Interrupt not reflected in IRQB pin
#define CONFIG_MASK_MAX_RT  0x10    //Interrupt not reflected in IRQB pin
#define CONFIG_EN_CRC       0x8     //Enable CRC
#define CONFIG_CRCO         0x4     //Set 2-Byte CRC o/w 1-Byte
#define CONFIG_PWR_UP       0x2     //Set to Power up o/w Power down
#define CONFIG_PRIM_RX      0x1     //Set mode to Receive o/w Transmit

#define ENAA_P5             0x20    //Enable Auto Ack on data pipe x
#define ENAA_P4             0x10
#define ENAA_P3             0x08
#define ENAA_P2             0x04
#define ENAA_P1             0x02
#define ENAA_P0             0x01
#define ENAA_NO_AA          0x00

#define ERX_P5              0x20    //Enable data pipe x
#define ERX_P4              0x10
#define ERX_P3              0x08
#define ERX_P2              0x04
#define ERX_P1              0x02
#define ERX_P0              0x01
#define ERX_NO_PIPES        0x00

#define AW_5BYTE            0x03    //RX/TX Address field width
#define AW_4BYTE            0x02
#define AW_3BYTE            0x01

#define RETR_NO_RETR        0x00    //Disable Auto Re-Transmission

#define RFSETUP_PLL_LOCK    0x10
#define RFSETUP_RF_DR       0x08    //Set 2Mbps Air Data Rate o/w 1Mbps

#define RFSETUP_RF_PWR18    0x00    //Set RF power output in TX mode
#define RFSETUP_RF_PWR12    0x02
#define RFSETUP_RF_PWR6     0x04
#define RFSETUP_RF_PWR0     0x06
#define RFSETUP_LNA_HCURR   0x01

#define STATUS_RX_DR        0x40    //Interrupt when RX is received and ready
#define STATUS_TX_DS        0x20    //Interrupt when TX data is sent
                                    //if auto-ack: only set when ACK is received
#define STATUS_MAX_RT       0x10    //Interrupt for max number of re-transmits
#define STATUS_RX_P_NO      0x
#define STATUS_TX_FULL      0x00    // On if TX FIFO is full

#define FIFO_STAT_REUSE     0x40
#define FIFO_STAT_TX_FULL   0x20
#define FIFO_STAT_TX_EMPTY  0x10
#define FIFO_STAT_CHECK     0x0C
#define FIFO_STAT_RX_FULL   0x02
#define FIFO_STAT_RX_EMPTY  0x01


// .... other addresses to be added if needed ...


/* SPI - COMMAND MNEUMONICS */

#define R_RX_PAYLOAD    0x61
#define W_TX_PAYLOAD    0xA0
#define FLUSH_TX        0xE1
#define FLUSH_RX        0xE2
#define REUSE_TX_PL     0xE3

/* Not used */
#define ACTIVATE        0x50
#define ACT_2           0x73
#define R_RX_PL_WID     0x60
#define NOP             0xFF

#define MAX_TX_LEN      32

void initNrfGpio(bool interrupts);
void configDefaultNrf24(void);
void setNrfTxMode(uint8_t *pipeAddr, uint8_t channel);
void setNrfRxMode(void);//setup later
bool nrf24Transmit(uint8_t *data);
void nrf24WriteReg(uint8_t reg, uint8_t data);
void nrf24WriteRegMulti(uint8_t reg, uint8_t *data, uint8_t len);
uint8_t nrf24ReadReg(uint8_t reg);
void nrf24SendCommand(uint8_t cmmd);


#endif /* NRF24L01_H_ */
