/*
 * qmc5883p.c
 *
 *  Created on: Mar 24, 2026
 *      Author: kyojin
 */
#include "qmc5883p.h"
#include <stdint.h>
#include <stdbool.h>
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

    //In documentation for defining the sign (flips Y and Z)
//    writeI2c1Register(QMC5883P_ADDR, QMC5883P_SIGN_R, 0x06);

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

//      QMC5883P Magnetometer Conversion
/* +-------------+-----------------+ *
 * |     FSR     |     LSB Sens.   | *
 * +--------------------+----------+ *
 * |   ± 30 G    |  1000  LSB/G    | *
 * |   ± 12 G    |  2500  LSB/G    | *
 * |   ± 8  G    |  3750  LSB/G    | *
 * |   ± 2  G    |  15000 LSB/G    | *
 * +-------------+-----------------+ */
#define MAG_SENS    1000

Vec3f qmc_read(void) {
    uint8_t bytes[6];
    uint8_t i = 0;
    Vec3f raws;
    readI2c1Registers(QMC5883P_ADDR, QMC5883P_DOUT_XL_R, bytes, 6);
    raws.x = ((float)(int16_t)((bytes[i++] << 8) | bytes[i++])) / MAG_SENS;
    raws.y = ((float)(int16_t)((bytes[i++] << 8) | bytes[i++])) / MAG_SENS;
    raws.z = ((float)(int16_t)((bytes[i++] << 8) | bytes[i++])) / MAG_SENS;
    return raws;
}
