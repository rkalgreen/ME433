#include "encoder.h"

void encoder_init(){
    uint8_t zero_angle_buf[2];
    uint8_t zero_angle_send[3];
    uint8_t max_angle_buf[2];
    uint8_t max_angle_send[3];
    uint8_t md[2]; 
    

    printf("in init function\n");
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

    i2c_write_blocking(i2c_default, AS5600_ADDR, zero_angle_send, 3, true);
    printf("zero pos done\n");
    sleep_ms(12000);
    printf("reading max pos\n");
    read_data(RAW_ANGLE, zero_angle_buf);

    max_angle_send[0] = MPOS;
    max_angle_send[1] = max_angle_buf[0];
    max_angle_send[2] = max_angle_buf[1];
    i2c_write_blocking(i2c_default, AS5600_ADDR, max_angle_send, 3, false);
    printf("max pos done\n");
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
    
    i2c_write_blocking(i2c_default, AS5600_ADDR, addr_buf, 1, true);  // true to keep host control of bus

        i2c_read_blocking(i2c_default, AS5600_ADDR, buf, 2, false);
  
}