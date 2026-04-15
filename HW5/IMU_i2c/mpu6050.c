#include <string.h> // for memset
#include "mpu6050.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

unsigned char MPU6050_ADDRESS = 0x68; // 7bit i2c address

void mpu6050_setup() {
    // small delay for power up
    sleep_ms(20);
    mpu6050_command(PWR_MGMT_1, 0x00); // turn on chip
    mpu6050_command(ACCEL_CONFIG, 0x07); // set accelerometer to +/- 2g, run self test
    mpu6050_command(GYRO_CONFIG, 0xF8); // set gyro to +/- 2000 dps, run self test

}

signed short mpu6050_readData(char reg) {
    char data[2];
    i2c_write_blocking(I2C_PORT, MPU6050_ADDRESS, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDRESS, data, 2, false);
    return (signed short)((data[0] << 8) | data[1]);
    }

void mpu6050_readAll(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z, int16_t *temp,
     int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z) {
    char data[14];
    char reg = ACCEL_XOUT_H;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDRESS, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDRESS, data, 14, false);
    *accel_x = (int16_t)((data[0] << 8) | data[1]);
    *accel_y = (int16_t)((data[2] << 8) | data[3]);
    *accel_z = (int16_t)((data[4] << 8) | data[5]);
    *temp = (int16_t)((data[6] << 8) | data[7]);
    *gyro_x = (int16_t)((data[8] << 8) | data[9]);
    *gyro_y = (int16_t)((data[10] << 8) | data[11]);
    *gyro_z = (int16_t)((data[12] << 8) | data[13]);
}

void mpu6050_command(char reg, char value) {
    char data[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDRESS, data, 2, false);
}