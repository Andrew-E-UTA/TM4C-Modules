/*
 * QMC5883L.h
 *
 *  Created on: Mar 14, 2026
 *      Author: kyojin
 */

#ifndef QMC5883L_H_
#define QMC5883L_H_

//=============================
/* INCLUDES */
//=============================

//C-Std Lib.
#include <stdbool.h>
#include <stdint.h>

//=============================
/* GLOBALS CONSTS AND MACROS */
//=============================

//Default I2C Address
#define QMC5883_ADDR        0x0D

//Register Map
#define QMC5883_DOUT_XL_R   0x00
#define QMC5883_DOUT_XH_R   0x01
#define QMC5883_DOUT_YL_R   0x02
#define QMC5883_DOUT_YH_R   0x03
#define QMC5883_DOUT_ZL_R   0x04
#define QMC5883_DOUT_ZH_R   0x05
#define QMC5883_STAT_R      0x06
#define QMC5883_DOUT_TL_R   0x07
#define QMC5883_DOUT_TH_R   0x08
#define QMC5883_CTRLA_R     0x09
#define QMC5883_CTRLB_R     0x0A
#define QMC5883_SR_R        0x0B
#define QMC5883_RESERVED    0x0C
#define QMC5883_WHO_AM_I_R  0x0D

//Register Bit Definitions
#define SH_M(v, reg)  (((v)<<(QMC5883_##reg##_SH)&(QMC5883_##reg##_M)))

#define QMC5883_STAT_DRDY   0x01    // R/W1C
#define QMC5883_STAT_OVF    0x02    //RO
#define QMC5883_STAT_DOR    0x04    //RO

//Sample Mode
#define QMC5883_CTRLA_MODE_SH       0
#define QMC5883_CTRLA_MODE_M        0x03
#define QMC5883_CTRLA_MODE_STANDBY  SH_M(0x0, CTRLA_MODE)
#define QMC5883_CTRLA_MODE_CONT     SH_M(0x1, CTRLA_MODE)

//Output Data Rate
#define QMC5883_CTRLA_ODR_SH        2
#define QMC5883_CTRLA_ODR_M         0x0C
#define QMC5883_CTRLA_ODR_10HZ      SH_M(0x0, CTRLA_ODR)
#define QMC5883_CTRLA_ODR_50HZ      SH_M(0x1, CTRLA_ODR)
#define QMC5883_CTRLA_ODR_100HZ     SH_M(0x2, CTRLA_ODR)
#define QMC5883_CTRLA_ODR_200HZ     SH_M(0x3, CTRLA_ODR)

//Range
#define QMC5883_CTRLA_RNG_SH        4
#define QMC5883_CTRLA_RNG_M         0x30
#define QMC5883_CTRLA_RNG_2G        SH_M(0x0, CTRLA_RNG)
#define QMC5883_CTRLA_RNG_8G        SH_M(0x1, CTRLA_RNG)

//Oversample Ratio
#define QMC5883_CTRLA_OSR_SH        6
#define QMC5883_CTRLA_OSR_M         0xC0
#define QMC5883_CTRLA_OSR_512       SH_M(0x0, CTRLA_OSR)
#define QMC5883_CTRLA_OSR_256       SH_M(0x1, CTRLA_OSR)
#define QMC5883_CTRLA_OSR_128       SH_M(0x2, CTRLA_OSR)
#define QMC5883_CTRLA_OSR_64        SH_M(0x3, CTRLA_OSR)

//Interrupt enable
#define QMC5883_CTRLB_INTEN_SH      0
#define QMC5883_CTRLB_INTEN_M       0x01
#define QMC5883_CTRLB_INTEN_ON      SH_M(0x0, CTRLB_INTEN)
#define QMC5883_CTRLB_INTEN_OFF     SH_M(0x1, CTRLB_INTEN)

//Pointer Rollover
#define QMC5883_CTRLB_PTROL_SH      6
#define QMC5883_CTRLB_PTROL_M       0x40
#define QMC5883_CTRLB_PTROL_ON      SH_M(0x1, CTRLB_PTROL)
#define QMC5883_CTRLB_PTROL_OFF     SH_M(0x0, CTRLB_PTROL)

//Soft Reset
#define QMC5883_CTRLB_SRST_SH       7
#define QMC5883_CTRLB_SRST_M        0x80
#define QMC5883_CTRLB_SRST_EN       SH_M(0x1, CTRLB_SRST)


//=============================
/* STRUCTS ENUMS & TYPES*/
//=============================
//typedef union {uint32_t w; struct{uint16_t h,l;}; } h_w;
typedef struct { int16_t x,y,z; } i16Vec3;

//=============================
/* SUB ROUTINE PROTOTYPES */
//=============================

void qmc_init(void);
i16Vec3 qmc_read(void);



#endif /* QMC58883L_H_ */
//=============================
//=============================
#ifdef QMC5883L_IMPLEMENTATION
#include "i2c1.h"

void qmc_init(void) {
    //Soft reset
    writeI2c1Register(QMC5883_ADDR, QMC5883_CTRLB_R, QMC5883_CTRLB_SRST_EN);
    waitMicrosecond(10e3);

    writeI2c1Register(QMC5883_ADDR, QMC5883_CTRLA_R,
                      QMC5883_CTRLA_MODE_CONT |
                      QMC5883_CTRLA_ODR_50HZ |
                      QMC5883_CTRLA_OSR_512 |
                      QMC5883_CTRLA_RNG_8G);
    writeI2c1Register(QMC5883_ADDR, QMC5883_CTRLB_R,
                      QMC5883_CTRLB_INTEN_ON |
                      QMC5883_CTRLB_PTROL_ON);

    writeI2c1Register(QMC5883_ADDR, QMC5883_SR_R, 0x01);
}

i16Vec3 qmc_read(void) {
    uint8_t bytes[6];
    uint8_t i = 0;
    i16Vec3 raws;
    readI2c1Registers(QMC5883_ADDR, QMC5883_DOUT_XL_R, bytes, 6);
    raws.x = (int16_t)((bytes[i++] << 8) | bytes[i++]);
    raws.z = (int16_t)((bytes[i++] << 8) | bytes[i++]);
    raws.y = (int16_t)((bytes[i++] << 8) | bytes[i++]);
    return raws;
}

#endif
