#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "encoder.h"
#include "hx711.h"
#include "hbridge.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>



// =========================
// === INA219 DEFINITIONS ==
// =========================
#define INA219_ADDR 0b1000000
#define INA219_REG_CONFIG 0x00
#define INA219_REG_CURRENT 0x04
#define INA219_REG_CALIBRATION 0x05

// =========================
// === PID + CONTROL VARS ==
// =========================
volatile float kp = 0.001;
volatile float ki = 0.3125;

volatile float Kp_position = 0.05;
volatile float Kd_position = 0;

#define EINTMAX 200
#define EINTMIN -200

volatile int state = 0;
volatile float pid_out;

float desired_current=0;
float saved_current=0;

volatile uint16_t* desired_position;

volatile int last_desired_position = 0; 
volatile int last_pos = 0; 
volatile int last_error = 0;

int flag = 0;

// =========================
// === POSITION CONTROL ====
// =========================
#define N 4000
#define TRI_AMP 35.0f
#define NUM_TRI 6
#define ZEROS 50

int16_t curve1[N];
int16_t curve2[N];

volatile int angle;
volatile float des_current_out;

static struct repeating_timer timer1;
static struct repeating_timer timer2;
// =========================
// === I2C PORTS ===========
// =========================
#define I2C_PORT0 i2c0
#define I2C_SDA0 8
#define I2C_SCL0 9

#define I2C_PORT1 i2c1
#define I2C_SDA1 14
#define I2C_SCL1 15

// =====================================================
// =============== INA219 FUNCTIONS =====================
// =====================================================
void writeINA219(uint8_t reg, uint16_t value) {
    uint8_t buf[3] = {reg, value >> 8, value & 0xFF};
    i2c_write_blocking(I2C_PORT1, INA219_ADDR, buf, 3, false);
}

int16_t readINA219(uint8_t reg) {
    uint8_t buffer[2];
    i2c_write_blocking(I2C_PORT1, INA219_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT1, INA219_ADDR, buffer, 2, false);
    return (buffer[0] << 8) | buffer[1];
}

void init_ina219() {
    uint16_t cal = 1024;
    uint16_t config = 0b0011000010001111;
    writeINA219(INA219_REG_CALIBRATION, cal);
    writeINA219(INA219_REG_CONFIG, config);
}

float read_ina219() {
    return readINA219(INA219_REG_CURRENT) / 3.0f;
}

// =====================================================
// =============== PWM OUTPUT ===========================
// =====================================================


// =====================================================
// =============== DESIRED CURRENT CURVES ===============
// =====================================================
void desired_current_init() {
    for (int i = 0; i < 100; i++) {
        curve1[i] = -255;
        curve2[i] = -255;
    }

    for (int i = 100; i < N - 100; i++) {
        curve1[i] = 0;
    }

    for (int i = N - 100; i < N; i++) {
        curve1[i] = 255;
        curve2[i] = 255;
    }

    for (int i = 100; i < N - 100; i++) {
        float t = (float)(i - 100) / (float)(N - 200);
        curve2[i] = (int16_t)(t * 255.0f);
    }
}

// =====================================================
// =============== MAIN CONTROL LOOP ====================
// =====================================================

bool current_control(__unused struct repeating_timer *t){
    static volatile int counter = 0;

    	uint32_t position = read_angle();
        desired_current = des_current_out;
//    	desired_current = read_pico();

    	if (position > 3900 || position < 20){
    					  pwm_set_gpio_level(IN1, 2400);
                          pwm_set_gpio_level(IN2, 2400);
    				 }
    	if (state) {
            flag = 1; 
			  // 3955 is zero degrees, 552 is 180 degrees,

    		// if (counter == 400){
    		// 	 __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 2400);
    		// 	 __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 2400);
    		// 	 state = 0;
    		// 	 counter = 0;
    		// }
    		saved_current = read_ina219();

            static float eint = 0;
            float error = desired_current - saved_current;


            
            eint = eint + error;

            if (eint > EINTMAX){
                eint = EINTMAX;
            }
            else if (eint < EINTMIN){
                eint = EINTMIN;
            }
            float upercent = kp*error + ki*eint;
            // upercent += 50;

             if(upercent > 100){
                    upercent = 100.0;
                }
            else if(upercent < -100){
                    upercent = -100;
                }
             pid_out = upercent;
            set_duty(upercent);


		 }
        }

bool position_control(__unused struct repeating_timer *t) {
    angle = read_angle();
    if (angle > 4000){
        angle = 4000; 
    }
    if (angle < 20){
        angle = 20; 
    }
    int32_t force = HX711_read(); 
    
    if (angle - last_pos > 0){
        desired_position = curve2; 
    }
    else{
        desired_position = curve1; 
    }
            
    static float eder = 0;
    float error = (int)desired_position[angle] - read_ina219();

    eder = error - last_error;         

    des_current_out = Kp_position*error + Kd_position*eder;
     if (angle > 3900){
        des_current_out = -des_current_out;
    }
    
    last_desired_position = (int)desired_position[angle];
    last_pos = angle; 
    last_error = error;
   

    return true;
}

// =====================================================
// ======================= MAIN =========================
// =====================================================
int main() {
    stdio_init_all();

    // I2C0 → Encoder
    i2c_init(I2C_PORT0, 400 * 1000);
    gpio_set_function(I2C_SDA0, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL0, GPIO_FUNC_I2C);

    // I2C1 → INA219
    i2c_init(I2C_PORT1, 100 * 1000);
    gpio_set_function(I2C_SDA1, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL1, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA1);
    gpio_pull_up(I2C_SCL1);

    HX711_init();
    encoder_init();
    init_ina219();
    desired_current_init();
    init_hbridge();

    // PWM setup
    // gpio_set_function(0, GPIO_FUNC_PWM);
    // gpio_set_function(1, GPIO_FUNC_PWM);
    // uint slice = pwm_gpio_to_slice_num(0);
    // pwm_set_wrap(slice, 2400);
    // pwm_set_enabled(slice, true);

    add_repeating_timer_ms(1, position_control, NULL, &timer1);
    add_repeating_timer_ms(1, current_control, NULL, &timer2);
    state = 1;
    while (true) {
        printf("pos: %d\n", angle);
        printf("sent: %f\n", des_current_out);
        printf("duty: %f\n", pid_out);
        printf("read: %f\n", saved_current);
        printf("flag: %d\n", flag);

        sleep_ms(100);
    }
}
