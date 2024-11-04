
#include <math.h>

#include "mpu6050.h"
#include "i2c0.h"


/* Pre-Processor Defines and Macros */

#define PI 3.141592f
#define RAD_2_DEG   57.2957f

#define AVERAGE_COUNT 255

#define ACCX_OFF    -.01f
#define ACCY_OFF     .01f
#define ACCZ_OFF    -.02f

/* Globals */

/* Function Implementation */

void initMpu6050(uint8_t addr)
{
    //SampleRate Divider
    //gyroscope  rate = 8k for dlpf off and 1k for dlpf on
    //Sampling Rate = 8e3 / (1 + SMPLRT_DIV) = 2 kHz
    writeI2c0Register(addr, SMPLRT_DIV, 0x03);

    // CLKSEL = 1
    // Recommended by datasheet to use one of the gyroscope reference sources for improved stability
    writeI2c0Register(addr, PWR_MGMT_1, 0x0A);

    //External Frame Sync
    //Digital Low Pass Filter
    writeI2c0Register(addr, CONFIG, 0x00);

    //No selfTests
    //Gyro Full Scale 1
    writeI2c0Register(addr, GYRO_CONFIG, 0x08);

    //No selfTests
    //Accel Full Scale 2
    writeI2c0Register(addr, ACCEL_CONFIG, 0x10);


    //Data Ready Enable = 1
    writeI2c0Register(addr, INT_ENABLE, 0x01);
    //Interrupt status can be read in I2C_MST_STATUS: bit6
}

void getMpuData(MpuData *data, MpuData *offset)
{

    //Byte array
    uint8_t buffer[14] = {};
    int16_t accX, accY, accZ, gyrX, gyrY, gyrZ;
    int16_t temp;
    //Read 14 bytes starting at accelerometer xout high
    //Accelerometer xyz H/L (6 bytes)
    //Temp H/L (2 bytes)
    //Gyroscope xyz H/L (6 bytes)
    readI2c0Registers(MPU_DEF_ADDR, ACCEL_XOUT_H, buffer, 14);

    accX = (buffer[0] << 8)  | buffer[1];
    accY = (buffer[2] << 8)  | buffer[3];
    accZ = (buffer[4] << 8)  | buffer[5];

    temp = (buffer[6] << 8)  | buffer[7];

    gyrX = (buffer[8] << 8)  | buffer[9];
    gyrY = (buffer[10] << 8) | buffer[11];
    gyrZ = (buffer[12] << 8) | buffer[13];


    //Based on configuration, divide result by the fullscale to get the reading
    data->accel.x = (double)   accX / ACC_FS_2_DIV + ACCX_OFF;
    data->accel.y = (double)   accY / ACC_FS_2_DIV + ACCY_OFF;
    data->accel.z = (double)   accZ / ACC_FS_2_DIV + ACCZ_OFF;

    data->gyro.x = (double)   gyrX / GYRO_FS_1_DIV;
    data->gyro.y = (double)   gyrY / GYRO_FS_1_DIV;
    data->gyro.z = (double)   gyrZ / GYRO_FS_1_DIV;

    if(offset)
    {
        data->gyro.x -= offset->gyro.x;
        data->gyro.y -= offset->gyro.y;
        data->gyro.z -= offset->gyro.z;
    }

    data->temp.c =  ((double)temp / 340.0f) + 36.53f;

}

void calculateOffset(MpuData *offset)
{
    MpuData data, cumulate;
    uint8_t iter;

    for(iter = 0; iter < AVERAGE_COUNT; iter++)
    {
        getMpuData(&data, 0);
        cumulate.accel.x += data.accel.x;
        cumulate.accel.y += data.accel.y;
        cumulate.accel.z += data.accel.z;

        cumulate.gyro.x += data.gyro.x;
        cumulate.gyro.y += data.gyro.y;
        cumulate.gyro.z += data.gyro.z;
    }

    offset->accel.x = cumulate.accel.x / AVERAGE_COUNT;
    offset->accel.y = cumulate.accel.y / AVERAGE_COUNT;
    offset->accel.z = cumulate.accel.z / AVERAGE_COUNT;

    offset->gyro.x = cumulate.gyro.x / AVERAGE_COUNT;
    offset->gyro.y = cumulate.gyro.y / AVERAGE_COUNT;
    offset->gyro.z = cumulate.gyro.z / AVERAGE_COUNT;
}

void getAccelPos(const MpuData *data, Pos3d *accelPos)
{
    accelPos->pitch = atan((data->accel.y) / (sqrt(data->accel.x * data->accel.x + data->accel.z * data->accel.z))) * RAD_2_DEG;
    accelPos->roll = atan((-1 * data->accel.x) / (sqrt(data->accel.y * data->accel.y + data->accel.z * data->accel.z))) * RAD_2_DEG;
    accelPos->yaw = 0;
}

void getGyroPos(const MpuData *data, Pos3d *gyroPos, double dt)
{
    //gyroPos stores the previous angle for pitch roll and yaw
    //dt is the time in seconds between measurements

    //PitchAngle += PitchRate * dt
    gyroPos->pitch  += data->gyro.x * dt;

    //RollAngle += RollRate * dt
    gyroPos->roll   += data->gyro.y * dt;

    //YawAngle += YawRate * dt
    gyroPos->yaw    += data->gyro.z * dt;
}
