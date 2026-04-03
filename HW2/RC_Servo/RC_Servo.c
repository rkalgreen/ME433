#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Servo output pin (adjust as needed)
#define SERVO_GPIO 15

// Standard 50Hz for analog RC servos
#define SERVO_FREQ_HZ 50

// Pulse width range in microseconds (common 0.5ms - 2.5ms for 0..180 deg)
#define SERVO_MIN_US 750
#define SERVO_MAX_US 2250

// Initialize servo PWM on a given GPIO
static void servo_pwm_init(uint gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);

    // Use 1 MHz PWM clock tick (125 MHz / 125)
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);

    // 50Hz period => 20ms => 20000 ticks
    pwm_config_set_wrap(&config, 20000 - 1);
    pwm_init(slice, &config, true);
}

// Set servo angle in degrees (0 to 180)
void servo_set_angle(uint gpio, float angle_deg)
{
    if (angle_deg < 0.0f) angle_deg = 0.0f;
    if (angle_deg > 180.0f) angle_deg = 180.0f;

    float pulse_us = SERVO_MIN_US + (angle_deg / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US);
    uint16_t level = (uint16_t)(pulse_us); // 1 tick = 1 us with this setup

    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_gpio_level(gpio, level);
    pwm_set_enabled(slice, true);
}

int main()
{
    stdio_init_all();

    servo_pwm_init(SERVO_GPIO);

    while (true) {
        // Sweep servo 0..180
        for (float a = 0.0f; a <= 180.0f; a += 1.0f) {
            servo_set_angle(SERVO_GPIO, a);
            sleep_ms(15);
        }
        for (float a = 180.0f; a >= 0.0f; a -= 1.0f) {
            servo_set_angle(SERVO_GPIO, a);
            sleep_ms(15);
        }
    }
}
