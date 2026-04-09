#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "ssd1306.h"
#include "font.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

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

    // Loop delay
    uint16_t sleep_time_ms = 500;

    
    while (true) {
        // Start FPS counter time
        absolute_time_t t1, t2;
        t1 = get_absolute_time();
        ssd1306_clear();
        // onboard heartbeat LED
        static int led_state = 0;
        static int heartbeat_counter = 0;
        heartbeat_counter++;
        if (heartbeat_counter >= 500 / sleep_time_ms) {
            led_state = !led_state;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
            heartbeat_counter = 0;
        }
        
        char message[26];
        sprintf(message, "It was the best of times,");
        ssd1306_drawString(0, 0, message);
        sprintf(message, "it was the worst of times,");
        ssd1306_drawString(0, 8, message);
        sprintf(message, "it was the age of wisdom,");
        ssd1306_drawString(0, 16, message);

        t2 = get_absolute_time();
        uint64_t ta;
        ta = to_us_since_boot(t2) - to_ms_since_boot(t1);
        char speed[12];
        sprintf(speed, "FPS= %6.3f", 1000000.0/ta);
        ssd1306_drawString(0, 24, speed);

        char adc0_V[12];
        sprintf(adc0_V, "V_adc0= %.2f", readPin(0b1000000, 0x42)*0.00488);
        ssd1306_drawString(13, 24, adc0_V);

        // ssd1306_drawChar(10, 10, 0x21);
        ssd1306_update();
        sleep_ms(sleep_time_ms);

        // ssd1306_clear();
        // ssd1306_update();
        sleep_ms(sleep_time_ms);
    }
}
