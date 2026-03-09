#include <stdint.h>
#include <stdbool.h>

#include "wait.h"
#include "i2c0.h"
#include "gme12864.h"

#define nullptr (void*)0
#define byteArray(...) (uint8_t[]){__VA_ARGS__}

void gme_init(void)
{
    writeI2c0Registers(GME_ADDR0, 0x00, byteArray(
            GME_DISPLAY_ON_OFF,
            GME_CLK_DIV, 0x80,
            GME_MUX_RATIO, 0x3F,
            GME_DISPLAY_OFS, 0x00,
            GME_DISPLAY_START,
            GME_CHARGE_PUMP, GME_CHARGE_PUMP_EN,
            GME_MEM_ADDR_MODE, GME_MEM_ADDR_HORIZ,
            GME_SEG_REMAP | GME_SEG_REMAP_EN,
            GME_SCAN_DIR | GME_SCAN_DIR_REMAP,
            GME_COM_CONFIG, GME_COM_CONFIG_MASK | GME_COM_CONFIG_ALT_COM,
            GME_CONTRAST_CONTROL, 0xCF,
            GME_PRE_CHARGE, 0xF1,
            GME_VCOM_DESEL_LVL, 0x40,
            GME_SHOW_DISPLAY,
            GME_DISPLAY_MODE,
            GME_DISPLAY_ON_OFF | GME_DISPLAY_ON), 25);
}

/*
 *  How this is intended to be used:
 *      - Outside of this library, define the screen buffer
 *      - write to that buffer however you want
 *      - Call this to write that buffer to the oled display
 */
void gme_updateDisplay(const uint8_t* dataBytes)
{
    writeI2c0Registers(GME_ADDR0, 0x00, byteArray(0x21, 0x00, 0x7F, 0x22, 0x00, 0x07), 6);
    writeI2c0Registers(GME_ADDR0, 0x40, dataBytes, GME_SCREEN_SIZE);
}
