#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/cyw43_arch.h"
#include <math.h>
#include "hardware/gpio.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi1
#define PIN_MISO 12
#define PIN_CS   13
#define PIN_SCK  14
#define PIN_MOSI 15

// Forward declarations
void setDac(int channel, float v);
// void cs_select(uint pin);
// void cs_deselect(uint pin);

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // SPI initialisation. This example will use SPI at 12kHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    int div = 1000;
    
    // Generate 2hz sine wave values for DAC output 1
    float vA[div];
    for (int i = 0; i<div; i++) {
        vA[i] = (sinf(i / (float)div * 4 * M_PI) + 1) / 2 * 3.3;
    }

    // Generate 1hz triangle wave values for DAC output 2
    float vB[div];
    for (int i = 0; i<div; i++) {
        if (i < div/2) {
            vB[i] = (i / (float)(div/2)) * 3.3;
        } else {
            vB[i] = (1 - (i - div/2) / (float)(div/2)) * 3.3;
        }
    }

    int time = 0;

    while (true) {
        // onboard heartbeat LED
        static int led_state = 0;
        static int heartbeat_counter = 0;
        heartbeat_counter++;
        if (heartbeat_counter >= 50) {
            led_state = !led_state;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
            heartbeat_counter = 0;
        }

        // update DAC
        setDac(0, vA[time]);
        setDac(1, vB[time]);
        time = (time + 1) % div;
        // printf("DAC output: %f V\n", vA[time]);

        // Sleep for 10ms
        sleep_ms(1000/div);
    }
}

void setDac(int channel, float v){

    uint8_t data[2];

    data[0] = 0b01110000;
    data[0] = data[0] | ((channel & 0b1) << 7);
    uint16_t theV = v/3.3*1023;
    data[0] = data[0] | (theV >> 6);
    data[1] = (theV << 2) & 0xFF;

    gpio_put(PIN_CS, 0); 
    spi_write_blocking(SPI_PORT, data, 2);
    gpio_put(PIN_CS, 1);
}
