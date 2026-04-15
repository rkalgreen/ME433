#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "ssd1306.h"
#include "font.h"
#include "mpu6050.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

// MPU6050 defines
#define MPU6050_ADDRESS 0x68 // 7bit i2c address

// General read/write functions for I2C
void setPin(char address, char reg, char value) {
    char data[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, data, 2, false);
}

char readPin(char address, char reg) {
    char value;
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);  // true to keep host control of bus
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);  // false - finished with bus
    return value;
}


int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    // Set up the OLED display
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    // Check communication with the mpu6050
    char IMU_init =readPin(MPU6050_ADDRESS, WHO_AM_I);
    if (IMU_init == 0x68) {
        printf("MPU6050 recognized\n");
    } else {
        printf("MPU6050 not recognized, WHO_AM_I register returned: 0x%X\n", IMU_init);
        while (true) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(100);
        }
    }
    
    // Set up the mpu6050
    mpu6050_setup();

    while (true) {
        // onboard heartbeat LED
        static int led_state = 0;
        static int heartbeat_counter = 0;
        heartbeat_counter++;
        if (heartbeat_counter >= 10) {
            led_state = !led_state;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
            heartbeat_counter = 0;
        }





        sleep_ms(100);
    }
}
