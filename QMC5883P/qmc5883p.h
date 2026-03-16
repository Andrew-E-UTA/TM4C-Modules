/*
 * QMC5883P.h
 *
 *  Created on: Mar 14, 2026
 *      Author: kyojin
 */

#ifndef QMC5883P_H_
#define QMC5883P_H_

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
#define QMC5883P_ADDR           0x2C

//Register Map
#define QMC5883P_CHIPID_R       0x00
#define QMC5883P_DOUT_XL_R      0x01
#define QMC5883P_DOUT_XH_R      0x02
#define QMC5883P_DOUT_YL_R      0x03
#define QMC5883P_DOUT_YH_R      0x04
#define QMC5883P_DOUT_ZL_R      0x05
#define QMC5883P_DOUT_ZH_R      0x06

#define QMC5883P_STAT_R         0x09
#define QMC5883P_CTRLA_R        0x0A
#define QMC5883P_CTRLB_R        0x0B

#define QMC5883P_SIGN_R         0x29

//Register Bit Definitions

#define QMC5883P_CHIPID_ID      0x80

#define QMC5883P_STAT_DRDY      0x01
#define QMC5883P_STAT_OVFL      0x02

#define QMC5883P_CTRLA_MODE_SH          0
#define QMC5883P_CTRLA_MODE_M           0x03
#define QMC5883P_CTRLA_MODE_SUSPEND     0x00
#define QMC5883P_CTRLA_MODE_NORMAL      0x01
#define QMC5883P_CTRLA_MODE_SINGLE      0x02
#define QMC5883P_CTRLA_MODE_CONTINUOUS  0x03

#define QMC5883P_CTRLA_ODR_SH           2
#define QMC5883P_CTRLA_ODR_M            0xC0
#define QMC5883P_CTRLA_ODR_10HZ         0x00
#define QMC5883P_CTRLA_ODR_50HZ         0x04
#define QMC5883P_CTRLA_ODR_100HZ        0x08
#define QMC5883P_CTRLA_ODR_200HZ        0x0C

#define QMC5883P_CTRLA_OSR1_SH          4
#define QMC5883P_CTRLA_OSR1_M           0x30
#define QMC5883P_CTRLA_OSR1_8           0x00
#define QMC5883P_CTRLA_OSR1_4           0x10
#define QMC5883P_CTRLA_OSR1_2           0x20
#define QMC5883P_CTRLA_OSR1_1           0x30

#define QMC5883P_CTRLA_OSR2_SH          6
#define QMC5883P_CTRLA_OSR2_M           0xC0
#define QMC5883P_CTRLA_OSR2_1           0x00
#define QMC5883P_CTRLA_OSR2_2           0x40
#define QMC5883P_CTRLA_OSR2_4           0x80
#define QMC5883P_CTRLA_OSR2_8           0xC0

#define QMC5883P_CTRLB_SR_SH            0
#define QMC5883P_CTRLB_SR_M             0x03
#define QMC5883P_CTRLB_SR_SR            0x00
#define QMC5883P_CTRLB_SR_S             0x01
#define QMC5883P_CTRLB_SR_OFF           0x02

#define QMC5883P_CTRLB_RNG_SH           2
#define QMC5883P_CTRLB_RNG_M            0x0C
#define QMC5883P_CTRLB_RNG_30GA         0x00
#define QMC5883P_CTRLB_RNG_12GA         0x04
#define QMC5883P_CTRLB_RNG_8GA          0x08
#define QMC5883P_CTRLB_RNG_2GA          0x0C

#define QMC5883P_CTRLB_SELF_TEST        0x40
#define QMC5883P_CTRLB_SOFT_RESET       0x80


//=============================
/* STRUCTS ENUMS & TYPES*/
//=============================
//typedef union {uint32_t w; struct{uint16_t h,l;}; } h_w;
typedef struct { int16_t x,y,z; } i16Vec3;

//=============================
/* SUB ROUTINE PROTOTYPES */
//=============================

bool qmc_verify(void);
void qmc_clear_int(void);

void qmc_init(void);
i16Vec3 qmc_read(void);

#endif /* QMC58883P_H_ */
//=============================
//=============================
#ifdef QMC5883P_IMPLEMENTATION
#include "i2c1.h"

bool qmc_verify(void) {
    return QMC5883P_CHIPID_ID == readI2c1Register(QMC5883P_ADDR, QMC5883P_CHIPID_R);
}

void qmc_clear_int(void) {
    readI2c1Register(QMC5883P_ADDR, QMC5883P_STAT_R);
}

void qmc_init(void) {
    //return all registers to default state
    writeI2c1Register(QMC5883P_ADDR, QMC5883P_CTRLB_R, QMC5883P_CTRLB_SOFT_RESET);

    //In documentation for defining the sign
    writeI2c1Register(QMC5883P_ADDR, QMC5883P_SIGN_R, 0x06);

    //Set reset on. 8 gauss range
    writeI2c1Register(QMC5883P_ADDR, QMC5883P_CTRLB_R,
                      QMC5883P_CTRLB_SR_SR |
                      QMC5883P_CTRLB_RNG_30GA);

    // continuous, data rate 50hz, over sample ratio 4, down sampling rate 1
    writeI2c1Register(QMC5883P_ADDR, QMC5883P_CTRLA_R,
                      QMC5883P_CTRLA_MODE_CONTINUOUS |
                      QMC5883P_CTRLA_ODR_50HZ |
                      QMC5883P_CTRLA_OSR1_4 |
                      QMC5883P_CTRLA_OSR2_1);
}

i16Vec3 qmc_read(void) {
    uint8_t bytes[6];
    uint8_t i = 0;
    i16Vec3 raws;
    readI2c1Registers(QMC5883P_ADDR, QMC5883P_DOUT_XL_R, bytes, 6);
    raws.x = (int16_t)((bytes[i++] << 8) | bytes[i++]);
    raws.z = (int16_t)((bytes[i++] << 8) | bytes[i++]);
    raws.y = (int16_t)((bytes[i++] << 8) | bytes[i++]);
    return raws;
}

#endif
