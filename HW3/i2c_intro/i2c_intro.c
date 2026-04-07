#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Sample code changed to I2C1 on GPIO14 (SDA) and GPIO15 (SCL)
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

    // Setting up the MCP23008 I/O expander
    // assigning I/O direction on the expander
    uint8_t ADDR = 0x20; // all adress pins grounded
    char IOreg = 0x00; // IODIR register
    char config = 0x7F; //GP7 out, all others in
    setPin(ADDR, IOreg, config);

    // assigning output pin and values
    char outReg = 0x0A; // OLAT register
    char onValue = 0x80; // GP7 high
    char offValue = 0x00; // GP7 low

    // assigning gpio button input
    char buttonReg = 0x09; // GPIO register
    unsigned int buttonBitpos = 0; // GP0 is the 0 bit in the return
    unsigned int buttonMask = 1 << buttonBitpos; // GP0 is the button input

    // while (true) {
    //     // printf("Hello, world!\n");
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    //     // setPin(ADDR, outReg, onValue); // Testing LED blink
    //     sleep_ms(1000);
        
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    //     //setPin(ADDR, outReg, offValue); Testing LED blink
    //     sleep_ms(1000);
    // }

    while (true) {        
        // onboard heartbeat LED
        static int led_state = 0;
        static int heartbeat_counter = 0;
        heartbeat_counter++;
        if (heartbeat_counter >= 5) {
            led_state = !led_state;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
            heartbeat_counter = 0;
        }
        


        char buttonState = readPin(ADDR, buttonReg);
        // printf("Raw button state: %d\n", buttonState);
        buttonState = (buttonState & buttonMask) >> buttonBitpos;
        printf("Button state: %d\n", buttonState);
        if (buttonState == 0) {
            printf("Button Pressed!\n");
            setPin(ADDR, outReg, onValue); // turn on LED
        } else {
            setPin(ADDR, outReg, offValue); // turn off LED
        }
        buttonState = 1; // reset button state variable
        sleep_ms(10); 
    }
}
