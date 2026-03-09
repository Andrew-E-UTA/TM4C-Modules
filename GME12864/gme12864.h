#ifndef _GME_12864_H_
#define _GME_12864_H_

#define GME_ADDR0               0x3C
#define GME_ADDR1               0x3D

/* GME 12864 Command Table*/

/*
 * GME i2c interface:
 *  "Data" bytes does not mean only for GDDRAM
 *      - A data byte can be command (or part of a command) or GDDRAM data
 *  C0:
 *      0: Indicates that no more control bytes needed, this control byte's DC bit
 *          will apply to all remaining Bytes until the stop condition
 *      1:Indicates that the DC bit will only apply to 1 byte, and another
 *          control byte must follow if more data is to be sent
 *  DC:
 *      0: The Byte(s) are a command
 *      1: The Byte(s) are data sent to the GDDRAM
 */
#define GME_DC                  0x40
#define GME_CONT                0x80

#define GME_SCREEN_SIZE         (128*64/8)

//Fundamental Commands
#define GME_CONTRAST_CONTROL    0x81    //n1
#define GME_SHOW_DISPLAY        0xA4    //1n
#define GME_DISPLAY_MODE        0xA6    //1n
#define GME_DISPLAY_ON_OFF      0xAE    //1n

//Scrolling Command Table
// Not implemented yet...

//Addressing Settings
#define GME_START_COL_L         0x00    //4n
#define GME_START_COL_H         0x10    //4n
#define GME_MEM_ADDR_MODE       0x20    //n1
#define GME_COL_ADDR            0x21    //n2
#define GME_PAGE_ADDR           0x22    //n2
#define GME_START_PAGE          0xB0    //3n

//Hardware Configuration
#define GME_DISPLAY_START       0x40    //6n
#define GME_SEG_REMAP           0xA0    //1n
#define GME_MUX_RATIO           0xA8    //n1
#define GME_SCAN_DIR            0xC0    //1n
#define GME_DISPLAY_OFS         0xD3    //n1
#define GME_COM_CONFIG          0xDA    //n1

//Timing Configuration
#define GME_CLK_DIV             0xD5    //n1
#define GME_PRE_CHARGE          0xD9    //n1
#define GME_VCOM_DESEL_LVL      0xDB    //n1
#define GME_NOP                 0xE3    //nn

//Charge Pump
#define GME_CHARGE_PUMP         0x8D    //n1

/* GME 12864 Command BitFields and Command Data Masks */
#define GME_DISPLAY_ON          0x01
#define GME_DISPLAY_OFF         0x00
#define GME_DISPLAY_MODE_NORMAL 0x00
#define GME_DISPLAY_MODE_INV    0x01
#define GME_MEM_ADDR_HORIZ      0x00
#define GME_MEM_ADDR_VERT       0x01
#define GME_MEM_ADDR_PAGE       0x02
#define GME_SEG_REMAP_EN        0x01
#define GME_SCAN_DIR_REMAP      0x08
#define GME_COM_CONFIG_ALT_COM  0x10
#define GME_COM_CONFIG_MASK     0x02
#define GME_CHARGE_PUMP_EN      0x14

void gme_init(void);
void gme_sendCommand(uint8_t cmmd);
void gme_updateDisplay(const uint8_t* databytes);

#endif//_GME_12864_H_
