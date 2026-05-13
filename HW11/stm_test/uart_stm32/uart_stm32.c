#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"


// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5

// String buffer for USB input
#define MAX_STRING_LEN 256
char input_string[MAX_STRING_LEN];

// Redirect printf to UART
// int pico_putchar(int ch) {
//     uart_putc(UART_ID, ch);
//     return ch;
// }

void blink_led(int times) {
    for (int i = 0; i < times; i++) {
        gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(100);
        gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(100);
    }
}

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Example to turn on the Pico W LED
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    blink_led(2); 

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    sleep_ms(2000);
    // Startup message
    printf("\r\n=== Pico 2W UART Bidirectional Ready ===\r\n");
    printf("Connected to STM32 over UART1 (pins 4,5)\r\n");
    printf("Type in serial monitor to send to STM32...\r\n");

    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    // uart_puts(UART_ID, " Hello, UART!\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    // Main loop
    while (1) {
        uint8_t rx_char;
        
        // Check if data received from USB serial (stdin)
        if (stdio_usb_connected() && getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {
            rx_char = getchar();
            
            // Send character from USB serial to STM32
            uart_putc(UART_ID, rx_char);
            
            // Echo back to USB serial for user feedback
            printf("%c", rx_char);
            fflush(stdout);
        }
        
        // Check if data received from STM32 over UART
        if (uart_is_readable(UART_ID)) {
            rx_char = uart_getc(UART_ID);
            
            // Print received character to USB serial monitor
            printf("%c", rx_char);
            fflush(stdout);
        }
        
        sleep_ms(10);  // Small delay to prevent CPU spinning
    }
    
    return 0;
}
