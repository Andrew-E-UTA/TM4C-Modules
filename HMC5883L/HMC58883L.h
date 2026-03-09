/*
 * HMC58883L.h
 *
 *  Created on: Mar 9, 2026
 *      Author: kyojin
 */

#ifndef HMC58883L_H_
#define HMC58883L_H_


/* INCLUDES */

//C-Std Lib.
#include <stdbool.h>
#include <stdint.h>

/* GLOBALS CONSTS AND MACROS */

//Default I2C Address
#define HMC5883_ADDR                0x1E

//Register Map
#define HMC5883_CONFIG_A_R          0x00
#define HMC5883_CONFIG_B_R          0x01
#define HMC5883_MODE_R              0x02
#define HMC5883_DOUT_X_H_R          0x03
#define HMC5883_DOUT_X_L_R          0x04
#define HMC5883_DOUT_Z_H_R          0x05
#define HMC5883_DOUT_Z_L_R          0x06
#define HMC5883_DOUT_Y_H_R          0x07
#define HMC5883_DOUT_Y_L_R          0x08
#define HMC5883_STAT_R              0x09
#define HMC5883_ID_A_R              0x10
#define HMC5883_ID_B_R              0x11
#define HMC5883_ID_C_R              0x12

//Register Bit Definitions
#define SH_M(v, reg)  (((v)<<(HMC5883_##reg##_SH)&(HMC5883_##reg##_M)))

#define HMC5883_CONFIG_A_MM_NORMAL      0x0 //High-Z across load
#define HMC5883_CONFIG_A_MM_POS         0x1 //Pos. current across load
#define HMC5883_CONFIG_A_MM_NEG         0x2 //Neg. current across load

#define HMC5883_CONFIG_A_DO_M           0x1C
#define HMC5883_CONFIG_A_DO_SH          2
#define HMC5883_CONFIG_A_DO_0P75        SH_M(0x00, CONFIG_A_DO)
#define HMC5883_CONFIG_A_DO_1P5         SH_M(0x01, CONFIG_A_DO)
#define HMC5883_CONFIG_A_DO_3           SH_M(0x02, CONFIG_A_DO)
#define HMC5883_CONFIG_A_DO_7P5         SH_M(0x03, CONFIG_A_DO)
#define HMC5883_CONFIG_A_DO_15          SH_M(0x04, CONFIG_A_DO)
#define HMC5883_CONFIG_A_DO_30          SH_M(0x05, CONFIG_A_DO)
#define HMC5883_CONFIG_A_DO_75          SH_M(0x06, CONFIG_A_DO)

#define HMC5883_CONFIG_A_AVG_M          0x60
#define HMC5883_CONFIG_A_AVG_SH         5
#define HMC5883_CONFIG_A_AVG_1          SH_M(0x00,CONFIG_A_AVG)
#define HMC5883_CONFIG_A_AVG_2          SH_M(0x01,CONFIG_A_AVG)
#define HMC5883_CONFIG_A_AVG_3          SH_M(0x02,CONFIG_A_AVG)
#define HMC5883_CONFIG_A_AVG_8          SH_M(0x03,CONFIG_A_AVG)

#define HMC5883_CONFIG_B_GAIN_M         0xE0
#define HMC5883_CONFIG_B_GAIN_SH        5
#define HMC5883_CONFIG_B_GAIN_0P88      SH_M(0x00, CONFIG_B_GAIN)
#define HMC5883_CONFIG_B_GAIN_1P3       SH_M(0x01, CONFIG_B_GAIN)
#define HMC5883_CONFIG_B_GAIN_1P9       SH_M(0x02, CONFIG_B_GAIN)
#define HMC5883_CONFIG_B_GAIN_2P5       SH_M(0x03, CONFIG_B_GAIN)
#define HMC5883_CONFIG_B_GAIN_4         SH_M(0x04, CONFIG_B_GAIN)
#define HMC5883_CONFIG_B_GAIN_4P7       SH_M(0x05, CONFIG_B_GAIN)
#define HMC5883_CONFIG_B_GAIN_5P6       SH_M(0x06, CONFIG_B_GAIN)
#define HMC5883_CONFIG_B_GAIN_8P1       SH_M(0x07, CONFIG_B_GAIN)

/*Table to convert the RAW Readings in LSB to Gauss
 * (div raw by these values based on the gain)
 *  +---------------------+
 *  |  GAIN  |  LSB/Gauss |
 *  +--------+------------+
 *  | ±0.88  |   1370     |
 *  | ±1.30  |   1090     |
 *  | ±1.90  |    820     |
 *  | ±2.50  |    660     |
 *  | ±4.00  |    440     |
 *  | ±4.70  |    390     |
 *  | ±5.60  |    330     |
 *  | ±8.10  |    230     |
 *  +--------+------------+*/
#define GAIN_P88_CONV       1370
#define GAIN_1P3_CONV       1090
#define GAIN_1P9_CONV       820
#define GAIN_2P5_CONV       660
#define GAIN_4_CONV         440
#define GAIN_4P7_CONV       390
#define GAIN_5P6_CONV       330
#define GAIN_8P1_CONV       230



#define HMC5883_MODE_MODE_M             0x03
#define HMC5883_MODE_MODE_SH            0
#define HMC5883_MODE_MODE_CONT          SH_M(0x00, MODE_MODE)
#define HMC5883_MODE_MODE_SINGLE        SH_M(0x01, MODE_MODE)

#define HMC5883_MODE_HS_M               0x80
#define HMC5883_MODE_HS_SH              8
#define HMC5883_MODE_HS_ON              SH_M(0x01, MODE_HS)

#define HMC5883_STAT_RDY                0x01
#define HMC5883_STAT_LOCK               0x02



/* STRUCTS ENUMS & TYPES*/
typedef struct { uint16_t x,y,z; } u16Vec3;
typedef struct { uint32_t x,y,z; } u32Vec3;

/* SUB ROUTINE PROTOTYPES */

void hmc_init(void);
u16Vec3 hmc_read(void);
u32Vec3 hmc_correct(const u16Vec3* raws);


#endif /* HMC58883L_H_ */
#ifdef HMC5883L_IMPLEMENTATION
#include "i2c1.h"

void hmc_init(void) {
    writeI2c1Register(HMC5883_ADDR, HMC5883_CONFIG_A_R, HMC5883_CONFIG_A_AVG_8 | HMC5883_CONFIG_A_DO_15);
    writeI2c1Register(HMC5883_ADDR, HMC5883_CONFIG_B_R, HMC5883_CONFIG_B_GAIN_4P7);
}

u16Vec3 hmc_read(void) {
    uint8_t i = 0;
    uint8_t buffer[6];
    u16Vec3 raws;
    readI2c1Registers(HMC5883_ADDR, HMC5883_DOUT_X_H_R, buffer, 6);
    raws.x = ((buffer[i++] << 8) | buffer[i++]);
    raws.z = ((buffer[i++] << 8) | buffer[i++]);
    raws.y = ((buffer[i++] << 8) | buffer[i++]);
    return raws;
}

//Returns the values in units of Gauss in Q16 format
#define HMC_CONVERT(r, c) (((uint32_t)(r)<<16)/(c))
u32Vec3 hmc_correct(const u16Vec3* raws) {
    u32Vec3 corrected;
    corrected.x = HMC_CONVERT(raws->x,GAIN_4P7_CONV);
    corrected.y = HMC_CONVERT(raws->y,GAIN_4P7_CONV);
    corrected.z = HMC_CONVERT(raws->z,GAIN_4P7_CONV);
    return corrected;
}
#endif
