#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define AS5600_ADDR 0x36
#define RAW_ANGLE 0x0C
#define ANGLE 0x0E
#define ZPOS 0x01
#define MPOS 0x03
#define MAGNET 0x0B

void encoder_init();
int read_angle();
void read_data(uint8_t addr, uint8_t* buf);

int main()
{
    stdio_init_all();


    sleep_ms(4000);
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c
    int angle = 0;
    encoder_init();
    while (true) {
       angle = read_angle();
       printf("%d\n", angle);
       sleep_ms(100);
    }
}

// i2c_write_blocking(i2c_default, SSD1306_ADDRESS, buf, 2, false);
// i2c_read_blocking(i2c_default, IMU_ADDR, buf, 14, false);

void encoder_init(){
    uint8_t zero_angle_buf[2];
    uint8_t zero_angle_send[3];
    uint8_t max_angle_buf[2];
    uint8_t max_angle_send[3];
    uint8_t md[2]; 
    

    // printf("in init function\n");
    // write address to read from 
    read_data(MAGNET, md);


    if(md[0] != 32){
        // printf("NO MAGNET!!!\n");
    }
    else{
        // printf("magnet detected :D\n");
    }

    read_data(RAW_ANGLE, zero_angle_buf);
    zero_angle_send[0] = ZPOS;
    zero_angle_send[1] = zero_angle_buf[0];
    zero_angle_send[2] = zero_angle_buf[1];

    i2c_write_blocking(I2C_PORT, AS5600_ADDR, zero_angle_send, 3, true);
    // printf("zero pos done\n");
    sleep_ms(12000);
    // printf("reading max pos\n");
    read_data(RAW_ANGLE, zero_angle_buf);

    max_angle_send[0] = MPOS;
    max_angle_send[1] = max_angle_buf[0];
    max_angle_send[2] = max_angle_buf[1];
    i2c_write_blocking(I2C_PORT, AS5600_ADDR, max_angle_send, 3, false);
    // printf("max pos done\n");
    // read raw angle from two consecutive 
    // write raw angle to ZPOS
    // wait 5 s
    // rotate to stop position
    // read raw angle
    // write to MPOS

}

int read_angle(){
    uint8_t buf[2];
    uint16_t angle = 0;
    read_data(ANGLE, buf);
    angle = (buf[0]<<8)|buf[1];
    return angle; 
    //i2c read
    //concatenate bits
    //success
}

void read_data(uint8_t addr, uint8_t* buf){ 
    uint8_t addr_buf[1];
    addr_buf[0] = addr;
    
    i2c_write_blocking(I2C_PORT, AS5600_ADDR, addr_buf, 1, true);  // true to keep host control of bus

        i2c_read_blocking(I2C_PORT, AS5600_ADDR, buf, 2, false);
  
}