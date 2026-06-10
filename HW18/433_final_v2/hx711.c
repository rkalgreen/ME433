#include "hx711.h"

void HX711_init() {
    // This function should set up the GPIO pins and any necessary configurations for the HX711
    gpio_init(DT_PIN);
    gpio_set_dir(DT_PIN, GPIO_IN);
    gpio_init(SCK_PIN);
    gpio_set_dir(SCK_PIN, GPIO_OUT);

    printf("HX711 initialized on DT pin %d and SCK pin %d\n", DT_PIN, SCK_PIN);
}

int32_t HX711_read() {
    // This function should read the analog value from the HX711 and return it
    int32_t raw = 0;
    
    // Wait for DT pin to go low
    while (gpio_get(DT_PIN)) {
        // DT is high, keep waiting
    }
    
    // Read 24 bits
    for (int i = 0; i < 24; i++) {
        // Set SCK high
        gpio_put(SCK_PIN, 1);
        sleep_us(1);  // Small delay for timing
        
        // Read DT
        int bit = gpio_get(DT_PIN);
        raw = (raw << 1) | bit;
        
        // Set SCK low
        gpio_put(SCK_PIN, 0);
        sleep_us(1);  // Small delay for timing
    }
    
    // 25th clock pulse (doesn't read data, sets gain for next read)
    gpio_put(SCK_PIN, 1);
    sleep_us(1);
    gpio_put(SCK_PIN, 0);
    sleep_us(1);
    
    // sign-extend 24-bit two's complement to 32-bit signed int
    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }
    
    // // Print the converted value
    // printf("HX711 value: %d\n", raw);
    return raw;
}