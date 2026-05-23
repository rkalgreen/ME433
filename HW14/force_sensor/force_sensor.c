#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"

// HX711 pin definitions
#define DT_PIN 17
#define SCK_PIN 16

// Data structure for storing samples
typedef struct {
    int32_t raw;
    int32_t filtered;
    uint32_t timestamp_ms;
} Sample;

// Function prototypes
void HX711_init();
int32_t HX711_read();

int main()
{
    stdio_init_all();
    
    // Wait for USB CDC connection to establish
    sleep_ms(1000);

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    HX711_init();
    printf("HX711 initialized. Enter number of samples to collect: ");
    fflush(stdout);
    
    // Wait for user input
    int num_samples = 0;
    scanf("%d", &num_samples);
    printf("Collecting %d samples...\n", num_samples);
    
    // Allocate memory for samples
    Sample *samples = (Sample *)malloc(num_samples * sizeof(Sample));
    if (samples == NULL) {
        printf("Memory allocation failed\n");
        return -1;
    }
    
    // IIR filter parameters
    // alpha controls cutoff: f_c = (alpha * f_s) / (2*pi)
    // For ~80Hz sampling and 5Hz cutoff: alpha ≈ 0.4
    // Lower alpha = more filtering (smoother), higher alpha = less filtering
    float alpha = 0.4f;  // Adjusted to reduce 25-30Hz noise
    int32_t filtered_value = 0;
    uint32_t start_time = time_us_32();
    
    // Collect samples
    for (int i = 0; i < num_samples; i++) {
        int32_t raw = HX711_read();
        
        // Apply IIR filter: filtered = alpha * raw + (1 - alpha) * prev_filtered
        filtered_value = (int32_t)(alpha * raw + (1.0f - alpha) * filtered_value);
        
        // Calculate timestamp in milliseconds
        uint32_t elapsed_us = time_us_32() - start_time;
        uint32_t timestamp_ms = elapsed_us / 1000;
        
        // Store sample
        samples[i].raw = raw;
        samples[i].filtered = filtered_value;
        samples[i].timestamp_ms = timestamp_ms;
    }
    
    // Print all collected data
    printf("\n=== Collected Data ===\n");
    printf("Sample\tTime(ms)\tRaw\t\tFiltered\n");
    printf("------\t--------\t--------\t--------\n");
    for (int i = 0; i < num_samples; i++) {
        printf("%d\t%u\t%d\t%d\n", i, samples[i].timestamp_ms, samples[i].raw, samples[i].filtered);
    }
    
    // Free allocated memory
    free(samples);
    
    // Heartbeat LED
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
    }
    
    return 0;
}

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