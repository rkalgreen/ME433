#include "hbridge.h"

void init_hbridge(){
    gpio_set_function(IN1, GPIO_FUNC_PWM);       // Set the LED Pin to be PWM
    uint slice_num1 = pwm_gpio_to_slice_num(IN1); // Get PWM slice number
    // the clock frequency is 150MHz divided by a float from 1 to 255
    float div1 = 3;                  // must be between 1-255
    pwm_set_clkdiv(slice_num1, div1); // sets the clock speed
    uint16_t wrap1 = 2500;           // when to rollover, must be less than 65535
    // set the PWM frequency and resolution
    // this sets the PWM frequency to 150MHz / div / wrap
    pwm_set_wrap(slice_num1, wrap1);
    pwm_set_enabled(slice_num1, true); // turn on the PWM
    pwm_set_gpio_level(IN1, wrap1);    // set the duty cycle to 100%

    gpio_set_function(IN2, GPIO_FUNC_PWM);       // Set the LED Pin to be PWM
    uint slice_num2 = pwm_gpio_to_slice_num(IN2); // Get PWM slice number
    // the clock frequency is 150MHz divided by a float from 1 to 255
    float div2 = 3;                  // must be between 1-255
    pwm_set_clkdiv(slice_num2, div2); // sets the clock speed
    uint16_t wrap2 = 2500;           // when to rollover, must be less than 65535
    // set the PWM frequency and resolution
    // this sets the PWM frequency to 150MHz / div / wrap
    pwm_set_wrap(slice_num2, wrap2);
    pwm_set_enabled(slice_num2, true); // turn on the PWM
    pwm_set_gpio_level(IN2, wrap2);    // set the duty cycle to 50%
}

void set_duty(float duty){
    // printf("duty in set: %f ", duty);
    if (duty > 0){
        pwm_set_gpio_level(IN1, 2400*1);
        pwm_set_gpio_level(IN2, 2400 * (100-duty)/100);
        // printf("wrap level: %f ", 2500 * (100-duty)/100);
    }
    else if (duty < 0){
        pwm_set_gpio_level(IN1, 2400 *(100+duty)/ 100);
        pwm_set_gpio_level(IN2, 2400 * 1);
    }
    else if (duty == 0){
        pwm_set_gpio_level(IN1, 2400);
        pwm_set_gpio_level(IN2, 2400);
    }

}