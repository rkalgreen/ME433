#include <string.h> // for memset
#include "mpu6050.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

unsigned char MPU6050_ADDRESS = 0x68; // 7bit i2c address