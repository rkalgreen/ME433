#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/cyw43_arch.h"
#include <math.h>
#include "hardware/gpio.h"

// SPI Defines
// We are going to use SPI 1, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi1
#define PIN_CS_RAM 11
#define PIN_MISO 12
#define PIN_CS_DAC   13
#define PIN_SCK  14
#define PIN_MOSI 15

// Forward declarations
void setDac(int channel, float v);
void spi_ram_write(uint16_t addr, uint8_t * data, int len);
void spi_ram_read(uint16_t addr, uint8_t * data, int len);

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // SPI initialisation. This example will use SPI at 12kHz.
    spi_init(SPI_PORT, 12*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_DAC,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_CS_RAM,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // Set RAM chip to continuous mode
    uint8_t status[2];
    status[0] = 0b000001;
    status[1] = 0b01000000;
    gpio_put(PIN_CS_RAM, 0); 
    spi_write_blocking(SPI_PORT, status, 2);
    gpio_put(PIN_CS_RAM, 1);
    
    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    uint16_t div = 1024;

    union FloatInt {
        float f;
        uint16_t i;
    };
    sleep_ms(2000);

    // Generate 2hz sine wave values for DAC output 1
    union FloatInt vA[div];
    for (uint16_t i = 0; i<div; i++) {
        vA[i].f = (sinf(i / (float)div * 8 * M_PI) + 1) / 2 * 3.3;
        
        int channel = 0;
        uint8_t data[2];

        data[0] = 0b01110000;
        data[0] = data[0] | ((channel & 0b1) << 7);
        vA[i].i = vA[i].f/3.3*1023;
        data[0] = data[0] | (vA[i].i >> 6);
        data[1] = (vA[i].i << 2) & 0xFF;

        spi_ram_write(i*2, data, 2);
        
        if (i == 0) {
            printf("Sample 0: vA[0].f=%.2f V, vA[0].i=%u, data[0]=0x%02x data[1]=0x%02x\n", vA[i].f, vA[i].i, data[0], data[1]);
        }
    }

    // // Generate 1hz triangle wave values for DAC output 2
    // float vB[div];
    // for (int i = 0; i<div; i++) {
    //     if (i < div/2) {
    //         vB[i] = (i / (float)(div/2)) * 3.3;
    //     } else {
    //         vB[i] = (1 - (i - div/2) / (float)(div/2)) * 3.3;
    //     }
    // }

    uint16_t time = 0;

    while (true) {
        // onboard heartbeat LED
        static int led_state = 0;
        static int heartbeat_counter = 0;
        heartbeat_counter++;
        if (heartbeat_counter >= 256) {
            led_state = !led_state;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
            heartbeat_counter = 0;
        }

        uint8_t data[2];
        spi_ram_read(time*2, data, 2);
        
        if (time == 0) {
            printf("Read from RAM at time=0: data[0]=0x%02x data[1]=0x%02x\n", data[0], data[1]);
        }
        
        gpio_put(PIN_CS_DAC, 0); 
        spi_write_blocking(SPI_PORT, data, 2);
        gpio_put(PIN_CS_DAC, 1);

        // // update DAC
        // setDac(0, vA[time]);
        // setDac(1, vA[time].f);
        
        // printf("DAC output: %f V\n", vA[time]);

        time = (time + 1) % div;
        // Sleep for 10ms
        sleep_us(100000/div);
    }
}

void setDac(int channel, float v){

    uint8_t data[2];

    data[0] = 0b01110000;
    data[0] = data[0] | ((channel & 0b1) << 7);
    uint16_t theV = v/3.3*1023;
    data[0] = data[0] | (theV >> 6);
    data[1] = (theV << 2) & 0xFF;

    gpio_put(PIN_CS_DAC, 0); 
    spi_write_blocking(SPI_PORT, data, 2);
    gpio_put(PIN_CS_DAC, 1);
}

void spi_ram_write(uint16_t addr, uint8_t * data, int len) {
    uint8_t packet[5];
    packet[0] = 0b00000010; // write
    packet[1] = addr>>8;
    packet[2] = addr&0xFF;
    packet[3] = data[0];
    packet[4] = data[1];

    gpio_put(PIN_CS_RAM, 0);
    // sleep_us(10);
    spi_write_blocking(SPI_PORT, packet, 5);
    // sleep_us(10);
    gpio_put(PIN_CS_RAM, 1);
    
    printf("RAM WRITE: addr=0x%04x, data=0x%02x 0x%02x\n", addr, data[0], data[1]);
}

void spi_ram_read(uint16_t addr, uint8_t * data, int len) {
    uint8_t packet[5];
    packet[0] = 0b00000011; // read
    packet[1] = addr>>8;
    packet[2] = addr&0xFF;
    packet[3] = 0;
    packet[4] = 0;

    uint8_t dst[5];
    gpio_put(PIN_CS_RAM, 0);
    // sleep_us(10);
    spi_write_read_blocking(SPI_PORT, packet, dst, 5);
    // sleep_us(10);
    gpio_put(PIN_CS_RAM, 1);
    data[0] = dst[3];
    data[1] = dst[4];
    
    printf("RAM READ: addr=0x%04x, received=0x%02x 0x%02x (raw dst: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x)\n", 
           addr, data[0], data[1], dst[0], dst[1], dst[2], dst[3], dst[4]);
}

// void spi_ram_read(uint16_t addr, uint8_t * data, int len) {
//     uint8_t cmd[3];
//     cmd[0] = 0b00000011; // read opcode
//     cmd[1] = addr>>8;
//     cmd[2] = addr&0xFF;
    
//     uint8_t dummy[3];
//     uint8_t read_data[2];
    
//     gpio_put(PIN_CS_RAM, 0);
    
//     // Send command and address
//     spi_write_blocking(SPI_PORT, cmd, 3);
    
//     // Read the data bytes
//     spi_read_blocking(SPI_PORT, 0, read_data, 2);
    
//     gpio_put(PIN_CS_RAM, 1);
    
//     data[0] = read_data[0];
//     data[1] = read_data[1];
    
//     printf("RAM READ: addr=0x%04x, received=0x%02x 0x%02x\n", addr, data[0], data[1]);
// }