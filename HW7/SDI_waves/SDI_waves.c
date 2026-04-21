#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/cyw43_arch.h"
#include <math.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19




int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // SPI initialisation. This example will use SPI at 1MHz.
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

    uint8_t div = 100;
    
    float vA[div];
    for (int i = 0; i<div; i++) {
        vA[i] = (sinf(i / div * 2 * M_PI) + 1) / 2 * 3.3;
    }
    
    uint8_t time = 0;

    while (true) {
        // update DAC
        setDac(0, vA[time]);
        time = (time + 1) % div;

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

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}